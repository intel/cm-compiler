/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/


#include "Common.h"

#include "clang/FrontendWrapper/Frontend.h"

#include <llvm/Support/Errc.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>

#include <llvm/ADT/StringExtras.h>

#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <fstream>
#include <string>

const char* DbgStr = std::getenv("IGC_CMOC_DEBUG");
const bool DebugEnabled = DbgStr ? (strcmp(DbgStr, "1") == 0) : false;

bool isCmocDebugEnabled() {
  return DebugEnabled;
}

using StringStorage = std::unordered_set<std::string>;
const char *getStableCStr(llvm::StringRef S, StringStorage &StableStrings) {
  return StableStrings.insert(S.str()).first->c_str();
}

template<typename InputIter>
std::vector<const char*> convertToCStrings(InputIter Begin, InputIter End,
                                           StringStorage &SavedStrings) {
  std::vector<const char*> CStrings;
  using InputType = typename std::iterator_traits<InputIter>::value_type;
  std::transform(Begin, End, std::back_inserter(CStrings),
                 [&SavedStrings](const InputType &S) {
                   return getStableCStr(S, SavedStrings);
                 });
  return CStrings;
}

enum class OutputKind {
  LLVM_IR_BC,
  LLVM_IR,
  SPIRV,
  VISA,
  PREPROC,
  Invalid
};

using BinaryData = std::vector<char>;

using IDriverInvocation = IGC::AdaptorCM::Frontend::IDriverInvocation;
using IDriverInvocationPtr = IGC::AdaptorCM::Frontend::IDriverInvocationPtr;

std::string FindCmHeaders(const char *argv0) {
  auto ExecPath =
      llvm::sys::fs::getMainExecutable(argv0, (void *)&FindCmHeaders);
  auto ExecRoot = llvm::sys::path::parent_path(ExecPath);
#if defined(__linux__)
  auto IncRoot = llvm::sys::path::parent_path(ExecRoot);
#else
  auto IncRoot = ExecRoot;
#endif
  llvm::SmallVector<char, 1024> PathData(IncRoot.begin(), IncRoot.end());
  llvm::sys::path::append(PathData, "include", "cm", "cm.h");
  llvm::StringRef CMHeaderPath(PathData.begin(), PathData.size());
  if (llvm::sys::fs::exists(CMHeaderPath)) {
    return llvm::sys::path::parent_path(
               llvm::sys::path::parent_path(CMHeaderPath))
        .str();
  }
  return {};
}

class CmocContext {
  using InputArgs = IGC::AdaptorCM::Frontend::InputArgs;
  using FEWrapper =
      IGC::AdaptorCM::Frontend::FEWrapper<void (*)(const std::string &)>;

  FEWrapper FE;
  IDriverInvocationPtr DriverInvocation = {nullptr, [](IDriverInvocation *) {}};
  std::unordered_set<std::string> StableStrings;
  std::vector<std::string> OriginalArgs;

  IDriverInvocationPtr createInvocation(std::vector<const char *> &Args);

  // Maps input file types reported by Frontend Wrapper
  // to the internal type supported by offline compiler
  static InputKind translateInputType(IDriverInvocation::InputTypeT InputType);

  // Maps output type reported by Frontend Wrapper
  // to the internal type supported by offline compiler
  static OutputKind
  translateOutputType(IDriverInvocation::OutputTypeT OutputType);

  BinaryData runFeForInvocation(const IDriverInvocation &Invocation);

public:
  bool isHelp() const {
    assert(DriverInvocation);
    return DriverInvocation->isHelp();
  }
  const std::string& getInputFilename() const {
    assert(DriverInvocation);
    return DriverInvocation->getInputFilename();
  }
  const std::string& getOutputFilename() const {
    assert(DriverInvocation);
    return DriverInvocation->getOutputFilename();
  }
  InputKind getInputKind() const {
    assert(DriverInvocation);
    return translateInputType(DriverInvocation->getInputType());
  }
  OutputKind getOutputKind() const {
    assert(DriverInvocation);
    return translateOutputType(DriverInvocation->getOutputType());
  }

  CmocContext(int argc, const char **argv);

  BinaryData runFE(llvm::StringRef Adjuster);
  void runVCOpt(const BinaryData &Input, InputKind IK,
                ILTranslationResult &Result);
};

CmocContext::CmocContext(int argc, const char **argv)
    : FE{IGC::AdaptorCM::Frontend::makeFEWrapper(FatalError).getValue()} {

  auto IncludeDirOpt = llvm::sys::Process::GetEnv("CM_INCLUDE_DIR");
  if (!IncludeDirOpt)
    IncludeDirOpt = llvm::sys::Process::GetEnv("CMOC_SUPPORT_DIR");

  llvm::StringRef CmIncludesDir;
  if (IncludeDirOpt) {
    CmIncludesDir = getStableCStr(IncludeDirOpt.getValue(), StableStrings);

    if (DebugEnabled)
      llvm::errs() << "CM includes path is taken from environment variable\n";

  } else {
    auto IncludePath = FindCmHeaders(argv[0]);
    CmIncludesDir = getStableCStr(IncludePath, StableStrings);

    if (DebugEnabled && !CmIncludesDir.empty())
      llvm::errs() << "CM includes path is auto-detected\n";
  }

  if (!CmIncludesDir.empty()) {

    OriginalArgs.push_back("-isystem");
    OriginalArgs.push_back(CmIncludesDir.str());

    if (DebugEnabled) {
      llvm::errs() << "original arguments we adjusted to include support " <<
          "directory" << llvm::join(OriginalArgs, " ") << "\n---\n";
    }
  }
  std::copy(argv, argv + argc, std::back_inserter(OriginalArgs));

  if (DebugEnabled) {
    llvm::errs() << "creating initial invocation : " <<
      llvm::join(OriginalArgs, " ") << "\n---\n";
  }

  StringStorage SavedStrings;
  auto Args = convertToCStrings(OriginalArgs.begin(), OriginalArgs.end(),
                                SavedStrings);

  DriverInvocation = createInvocation(Args);
  if (!DriverInvocation) {
    FatalError("could not create compiler invocaton\n");
  }

  if (DebugEnabled) {
    const auto& FeArgs = DriverInvocation->getFEArgs();
    const auto& BeArgs = DriverInvocation->getBEArgs();
    llvm::errs() << "Original FE Args: " << llvm::join(FeArgs, " ") << "\n";
    llvm::errs() << "Original BE Args: " << llvm::join(BeArgs, ", ") << "\n";
    llvm::errs() << "\n---\n";
  }
}

IDriverInvocationPtr
CmocContext::createInvocation(std::vector<const char *> &Args) {
  return FE.buildDriverInvocation(Args);
}

// Maps input file types reported by Frontend Wrapper
// to the internal type supported by offline compiler
InputKind
CmocContext::translateInputType(IDriverInvocation::InputTypeT InputType) {

  switch(InputType) {
  case IDriverInvocation::InputTypeT::SourceCM:
    return InputKind::TEXT;
  case IDriverInvocation::InputTypeT::LLVM_IR:
    return InputKind::IR;
  case IDriverInvocation::InputTypeT::SPIRV:
    return InputKind::SPIRV;
  // we may accept NONE input someday
  case IDriverInvocation::InputTypeT::NONE:
  case IDriverInvocation::InputTypeT::OTHER:
  default:
    return InputKind::Unsupported;
  }
}

// Maps output type reported by Frontend Wrapper
// to the internal type supported by offline compiler
OutputKind
CmocContext::translateOutputType(IDriverInvocation::OutputTypeT OutputType) {

  switch(OutputType) {
  case IDriverInvocation::OutputTypeT::S:
    return OutputKind::VISA;
  case IDriverInvocation::OutputTypeT::SPIRV:
    return OutputKind::SPIRV;
  case IDriverInvocation::OutputTypeT::LLVM_IR:
    return OutputKind::LLVM_IR;
  case IDriverInvocation::OutputTypeT::LLVM_BC:
    return OutputKind::LLVM_IR_BC;
  case IDriverInvocation::OutputTypeT::PREPROC:
    return OutputKind::PREPROC;
  case IDriverInvocation::OutputTypeT::OTHER:
  default:
    return OutputKind::Invalid;
  }
}

BinaryData
CmocContext::runFeForInvocation(const IDriverInvocation &Invocation) {
  std::ifstream InputFile(Invocation.getInputFilename());
  if (!InputFile.is_open())
    FatalError("could not open input file\n");

  std::string InputText{std::istreambuf_iterator<char>(InputFile),
                        std::istreambuf_iterator<char>()};

  InputArgs InputArgs;
  InputArgs.InputText = InputText;
  InputArgs.CompilationOpts = Invocation.getFEArgs();

  auto FEOutput = FE.translate(InputArgs);

  if (!FEOutput)
    FatalError("runFE returns null!\n");

  const auto &ErrLog = FEOutput->getLog();
  if (!ErrLog.empty()) {
    llvm::errs() << FEOutput->getLog();
  }

  if (FEOutput->getStatus() ==
      IGC::AdaptorCM::Frontend::IOutputArgs::ErrT::COMPILE_PROGRAM_FAILURE) {
    FatalError("Frontend detected a fatal error!\n");
  }
  return FEOutput->getIR();
}

BinaryData CmocContext::runFE(llvm::StringRef Adjuster) {

  if (Adjuster.empty()) {

    if (DebugEnabled)
      llvm::errs() << "Running original FE invocation" << "\n---\n";

    return runFeForInvocation(*DriverInvocation);
  }

  std::vector<const char*> NewArgs = convertToCStrings(OriginalArgs.begin(),
                                                       OriginalArgs.end(),
                                                       StableStrings);
  auto DashDashIt = std::find_if(NewArgs.begin(), NewArgs.end(),
                    [](const char* Arg) { return strcmp(Arg, "--") == 0; });
  NewArgs.insert(DashDashIt, getStableCStr(Adjuster, StableStrings));

  if (DebugEnabled) {
    std::vector<std::string> DebugStr;
    std::copy(NewArgs.begin(), NewArgs.end(), std::back_inserter(DebugStr));
    llvm::errs() << "Adjusting FE args with user args: " <<
      llvm::join(DebugStr, " ") << "\n---\n";
  }

  auto ProxyInvocation = createInvocation(NewArgs);
  if (!ProxyInvocation)
    FatalError("could not create frontend invocation!\n");

  if (DebugEnabled) {
    llvm::errs() << "Adjusted FE args: " <<
        llvm::join(ProxyInvocation->getFEArgs(), " ") << "\n---\n";
  }
  return runFeForInvocation(*ProxyInvocation);
}

void CmocContext::runVCOpt(const BinaryData &In, InputKind IK,
                           ILTranslationResult &Result) {

  assert(DriverInvocation);

  std::string CPU = DriverInvocation->getTargetArch();
  if (CPU.empty())
    CPU = "SKL"; // TODO: consider reporting an error

  std::string BinFormat;
  switch(DriverInvocation->getBinaryFormat()) {
  case IDriverInvocation::BinaryFormatT::CM:
    BinFormat = "cm";
    break;
  case IDriverInvocation::BinaryFormatT::OCL:
    BinFormat = "ocl";
    break;
  case IDriverInvocation::BinaryFormatT::ZE:
    BinFormat = "ze";
    break;
  }
  assert(!BinFormat.empty());

  std::vector<std::string> VcOpts =
      IGC::AdaptorCM::Frontend::convertBackendArgsToVcOpts(DriverInvocation->getBEArgs());

  const auto &TargetFeatures = DriverInvocation->getTargetFeaturesStr();

  translateIL(CPU, BinFormat, TargetFeatures, VcOpts, In, IK,
              DriverInvocation->getTimePasses(), Result);
}

static std::string makeDefaultFilename(OutputKind Kind) {
  switch(Kind) {
    case OutputKind::LLVM_IR_BC: return "a.bc";
    case OutputKind::LLVM_IR: return "a.ll";
    case OutputKind::SPIRV: return "a.spv";
    case OutputKind::VISA: return "a.bin";
    default: return "a.out"; //Should not happen
  }
}

static void checkInputOutputCompatibility(InputKind IK, OutputKind OK) {

  if (OK == OutputKind::Invalid)
    FatalError("unsupported output\n");

  if (IK == InputKind::Unsupported)
    FatalError("unsupported input\n");

  if (IK == InputKind::TEXT)
    return;

  if (IK == InputKind::Unsupported)
    FatalError("could not determine input type (fatal)\n");

  if (OK != OutputKind::VISA)
    FatalError("current input/output combination is not supported!\n");
}

static void printCmocHelp() {
  llvm::outs() << "---\nCMOC-specific help:\n";
  llvm::outs() << "Environment variables:\n";
  llvm::outs() << "   CM_INCLUDE_DIR - directory with the include files";
  llvm::outs() << "\n";
}
static std::error_code WriteBinaryToFile(llvm::StringRef Filename,
                                         const BinaryData &BinData) {
  std::ofstream Output(Filename, std::ios::binary | std::ios::out);
  if (!Output.is_open())
    return std::make_error_code(std::errc::io_error);
  Output.write(BinData.data(), BinData.size());
  // we want to check for an errors  if any, so buffer must be flushed
  Output.close();
  if (!Output)
    return std::make_error_code(std::errc::no_stream_resources);
  return {};
}
int main(int argc, const char **argv) {
  if (argc > 1) {
    // skip program name
    ++argv;
    --argc;
  }

  CmocContext Ctx(argc, argv);

  if (Ctx.isHelp()) {
    printCmocHelp();
    return EXIT_SUCCESS;
  }

  checkInputOutputCompatibility(Ctx.getInputKind(), Ctx.getOutputKind());

  BinaryData VCOptInput;
  // If input is text, run CM Frontend
  if (Ctx.getInputKind() == InputKind::TEXT) {
    VCOptInput = Ctx.runFE(
        (Ctx.getOutputKind() == OutputKind::VISA) ? "-emit-spirv" : "");
  } else {
    std::ifstream InputFile(Ctx.getInputFilename(), std::ios::binary);
    if (!InputFile.is_open())
      FatalError("could not open input file\n");
    VCOptInput = BinaryData(std::istreambuf_iterator<char>(InputFile), {});
  }

  if (Ctx.getOutputKind() == OutputKind::PREPROC) {
    // Dependency file (-MM) is generated directly by FE invocation
    // Compilation does not produce a direct output
    if (VCOptInput.empty())
      return EXIT_SUCCESS;
    FatalError("unsupported output detected");
  }

  BinaryData PrimaryOutput;
  BinaryData DebugInfoOutput;
  if (Ctx.getOutputKind() == OutputKind::VISA) {
    ILTranslationResult TranslatedResult;
    Ctx.runVCOpt(VCOptInput, Ctx.getInputKind(), TranslatedResult);
    PrimaryOutput = std::move(TranslatedResult.KernelBinary);
    DebugInfoOutput = std::move(TranslatedResult.DebugInfo);
  } else {
    PrimaryOutput = std::move(VCOptInput);
  }

  auto OutputFilename = Ctx.getOutputFilename();
  if (OutputFilename.empty())
    OutputFilename = makeDefaultFilename(Ctx.getOutputKind());

  if (auto Err = WriteBinaryToFile(OutputFilename, PrimaryOutput))
    FatalError("error during writing output file: " + Err.message());

  if (!DebugInfoOutput.empty()) {
    auto Err = WriteBinaryToFile(OutputFilename + ".dbg", DebugInfoOutput);
    if (Err)
      FatalError("error during writing debug info file" + Err.message());
  }

  return EXIT_SUCCESS;
}
