/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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

class CmocContext {
  using InputArgs = IGC::AdaptorCM::Frontend::InputArgs;
  using FEWrapper =
      IGC::AdaptorCM::Frontend::FEWrapper<void (*)(const std::string &)>;

  FEWrapper FE;
  IDriverInvocationPtr DriverInvocation = {nullptr, [](IDriverInvocation *) {}};

  IDriverInvocationPtr createInvocation(int Argc, const char **Argv);

  // Maps input file types reported by Frontend Wrapper
  // to the internal type supported by offline compiler
  static InputKind translateInputType(IDriverInvocation::InputTypeT InputType);

  // Maps output type reported by Frontend Wrapper
  // to the internal type supported by offline compiler
  static OutputKind
  translateOutputType(IDriverInvocation::OutputTypeT OutputType);

  BinaryData runFeForInvocation(const std::vector<std::string> &FEArgs);

public:
  bool isHelp() const {
    assert(DriverInvocation);
    return DriverInvocation->isHelp();
  }
  bool isShowVersion() {
    assert(DriverInvocation);
    return FE.isDriverShowVersionInvocation(DriverInvocation.get());
  }
  bool getPrintStats() {
    assert(DriverInvocation);
    return FE.getPrintStats(DriverInvocation.get());
  }
  std::string getStatsFile() {
    assert(DriverInvocation);
    return FE.getStatsFile(DriverInvocation.get());
  }
  std::string getVCApiOptions() {
    assert(DriverInvocation);
    return FE.getVCApiOptions(DriverInvocation.get());
  }
  int getRevId() {
    assert(DriverInvocation);
    return FE.getRevIdSymInvocation(DriverInvocation.get());
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

CmocContext::CmocContext(int Argc, const char **Argv)
    : FE{IGC::AdaptorCM::Frontend::makeFEWrapper(FatalError).getValue()} {

  if (DebugEnabled) {
    std::vector<llvm::StringRef> Args(Argv, Argv + Argc);
    llvm::errs() << "creating initial invocation : " << llvm::join(Args, " ")
                 << "\n---\n";
  }

  DriverInvocation = createInvocation(Argc, Argv);
  if (!DriverInvocation)
    FatalError("could not create compiler invocaton\n");

  if (DebugEnabled) {
    const auto& FeArgs = DriverInvocation->getFEArgs();
    const auto& BeArgs = DriverInvocation->getBEArgs();
    llvm::errs() << "Original FE Args: " << llvm::join(FeArgs, " ") << "\n";
    llvm::errs() << "Original BE Args: " << llvm::join(BeArgs, ", ") << "\n";
    llvm::errs() << "\n---\n";
  }
}

IDriverInvocationPtr CmocContext::createInvocation(int Argc,
                                                   const char **Argv) {
  return FE.buildDriverInvocation(Argc, Argv);
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
CmocContext::runFeForInvocation(const std::vector<std::string> &FEArgs) {
  InputArgs InputArgs;
  InputArgs.CompilationOpts = FEArgs;

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
  auto &OriginalCC1Args = DriverInvocation->getFEArgs();

  if (Adjuster.empty()) {
    if (DebugEnabled)
      llvm::errs() << "Running original FE invocation" << "\n---\n";

    return runFeForInvocation(OriginalCC1Args);
  }

  std::vector<std::string> NewArgs = OriginalCC1Args;
  NewArgs.push_back(Adjuster.str());

  if (DebugEnabled)
    llvm::errs() << "Adjusting CC1 FE args with user args: "
                 << llvm::join(NewArgs, " ") << "\n---\n";

  return runFeForInvocation(NewArgs);
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

  std::string APIOptions = getVCApiOptions();
  int RevId = getRevId();

  bool PrintStats = getPrintStats();
  std::string StatsFile = getStatsFile();

  translateIL(CPU, RevId, BinFormat, TargetFeatures, APIOptions, VcOpts, In, IK,
              DriverInvocation->getTimePasses(), PrintStats, StatsFile, Result);
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

  if (IK == InputKind::IR && OK == OutputKind::SPIRV) {
    return;
  }

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

static void printBackendVersion() {
  llvm::outs() << "Ocloc version: " << getOclocDriverVersion() << ' '
               << getOclocRevision() << "\n";
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

  if (Ctx.isShowVersion()) {
    printBackendVersion();
    return EXIT_SUCCESS;
  }
  checkInputOutputCompatibility(Ctx.getInputKind(), Ctx.getOutputKind());

  BinaryData VCOptInput;
  // If input is text or LLVM IR, run CM Frontend
  if (Ctx.getInputKind() == InputKind::TEXT ||
      Ctx.getInputKind() == InputKind::IR) {
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
  if (Ctx.getOutputKind() == OutputKind::VISA) {
    ILTranslationResult TranslatedResult;
    Ctx.runVCOpt(VCOptInput, Ctx.getInputKind(), TranslatedResult);
    PrimaryOutput = std::move(TranslatedResult.KernelBinary);
  } else {
    PrimaryOutput = std::move(VCOptInput);
  }

  auto OutputFilename = Ctx.getOutputFilename();
  if (OutputFilename.empty())
    OutputFilename = makeDefaultFilename(Ctx.getOutputKind());

  if (auto Err = WriteBinaryToFile(OutputFilename, PrimaryOutput))
    FatalError("error during writing output file: " + Err.message());

  return EXIT_SUCCESS;
}
