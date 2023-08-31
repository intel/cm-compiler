/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "clang/FrontendWrapper/Interface.h"

#include "ArgsManagement.h"
#include "HeaderStorage.h"
#include "Utils.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "clang/Driver/ToolChain.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/FrontendTool/Utils.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/VirtualFileSystem.h"

#include <string>

namespace {

const char* DbgStr = std::getenv("IGC_CMFE_DEBUG");
const bool DebugEnabled = DbgStr ? (strcmp(DbgStr, "1") == 0) : false;

// Helper to create array ref for header descriptors.
static llvm::ArrayRef<CmHeaderDesc> getCmHeaderDescs() {
  return {CmDescBegin, CmDescEnd};
}

template <typename InputIter>
std::vector<const char*> makeCOpts(InputIter Begin, InputIter End,
                                   llvm::StringSaver& Saver) {
  std::vector<const char*> COpts;
  std::transform(Begin, End, std::back_inserter(COpts),
                 [&Saver](llvm::StringRef S) {
                   return Saver.save(S).data();
                 });
  return COpts;
}

void insertTargetAndModeArgs(const clang::driver::ParsedClangName &NameParts,
                             llvm::SmallVectorImpl<const char *> &ArgVector,
                             llvm::StringSaver &Saver) {
  // Put target and mode arguments at the start of argument list so that
  // arguments specified in command line could override them. Avoid putting
  // them at index 0, as an option like '-cc1' must remain the first.
  int InsertionPoint = 0;
  if (ArgVector.size() > 0)
    ++InsertionPoint;

  if (NameParts.DriverMode) {
    // Add the mode flag to the arguments.
    ArgVector.insert(ArgVector.begin() + InsertionPoint,
                     Saver.save(NameParts.DriverMode).data());
  }

  if (NameParts.TargetIsValid) {
    const char *arr[] = {"-target", Saver.save(NameParts.TargetPrefix).data() };
    ArgVector.insert(ArgVector.begin() + InsertionPoint,
                     std::begin(arr), std::end(arr));
  }
}

// Use this virtual absolute location to store our headers.
// This will be added later as system headers path.
// Use UNC convention for windows and simple root path for linux.
#ifdef _WIN32
static const auto BuiltinHeadersRoot = "//cm/builtin";
#else
static const auto BuiltinHeadersRoot = "/cm/builtin";
#endif

llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem>
createBuiltinMemoryFileSystem() {
  auto MemFS = wrapper::MakeIntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem>();

  MemFS->setCurrentWorkingDirectory(BuiltinHeadersRoot);

  for (const CmHeaderDesc &D : getCmHeaderDescs()) {
    llvm::StringRef Name{D.Name};
    llvm::StringRef Data{D.Begin, D.Size};
    auto MemBuf = llvm::MemoryBuffer::getMemBuffer(Data);
    MemFS->addFile(Name, 0, std::move(MemBuf),
                   /*User=*/llvm::None, /*Group=*/llvm::None,
                   llvm::sys::fs::file_type::regular_file,
                   llvm::sys::fs::all_read);
  }

  return MemFS;
}

llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> getHeadersFileSystem() {
  static auto MemFS = createBuiltinMemoryFileSystem();
  return MemFS;
}

// These code is deceptive. It extracts filename from command line
// but uses InArgs->InputText as its content. So actually if someone (FCL)
// just generated real file but has not copied it to input, compiler
// will compile empty source.
void createInputsFileSystem(
    const wrapper::IInputArgs *InArgs, llvm::StringRef InputFileName,
    const llvm::IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem>
        &OverlayMemFS) {
  auto MemFS = wrapper::MakeIntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem>();
  OverlayMemFS->pushOverlay(MemFS);

  auto Src = wrapper::getSrc<llvm::StringRef>(InArgs);
  MemFS->addFile(InputFileName, 0,
                 llvm::MemoryBuffer::getMemBuffer(Src, InputFileName));

  for (auto File : wrapper::getExtraFiles<llvm::StringRef>(InArgs)) {
    MemFS->addFile(File.Name, 0,
                   llvm::MemoryBuffer::getMemBuffer(File.Src, File.Name));
  }
}

llvm::IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem>
createFileSystem(const wrapper::IInputArgs *InArgs,
                 llvm::StringRef InputFileName) {
  auto OverlayMemFS =
      wrapper::MakeIntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem>(
          llvm::vfs::getRealFileSystem());
  OverlayMemFS->pushOverlay(getHeadersFileSystem());

  // User can specify source file to be compiled in 2 ways:
  // 1) by passing file name in real FS (InputFileName gets it;
  // in this case wrapper::getSrc<llvm::StringRef>(InArgs)
  // aka InArgs->InputText is empty string);
  //
  // 2) by passing content of source file in memory buffer
  // (wrapper::getSrc<llvm::StringRef>(InArgs)
  // aka InArgs->InputText gets it),
  // optionally providing file name for debug info through cm-src option
  // (in this case InputFileName gets it, or default "src.cm");
  //
  // Check which of these 2 variants is in this case, and if 2-nd,
  // then createInputsFileSystem() adds source file from memory FS
  // to overlay
  if (!wrapper::getSrc<llvm::StringRef>(InArgs).empty())
    createInputsFileSystem(InArgs, InputFileName, OverlayMemFS);

  return OverlayMemFS;
}

llvm::StringRef getTheOnlyInputFileName(const clang::CompilerInstance &Clang) {
  auto &Inputs = Clang.getFrontendOpts().Inputs;
  assert(Inputs.size() == 1 && "must have only one input file");
  return Inputs[0].getFile();
}

struct DiagnosticSubsystem {
  llvm::raw_ostream& DiagStream;
  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID;
  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts;
  clang::TextDiagnosticPrinter DiagsPrinter;

  llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> Diags;

  DiagnosticSubsystem(llvm::raw_ostream& DS)
      : DiagStream(DS),
        DiagID(wrapper::MakeIntrusiveRefCntPtr<clang::DiagnosticIDs>()),
        DiagOpts(wrapper::MakeIntrusiveRefCntPtr<clang::DiagnosticOptions>()),
        DiagsPrinter(DiagStream, &*DiagOpts),
        Diags(wrapper::MakeIntrusiveRefCntPtr<clang::DiagnosticsEngine>(
            DiagID, &*DiagOpts, &DiagsPrinter, /* ShouldOwnClient = */ false)) {
  }
};

using Intel::CM::ClangFE::IDriverInvocation;
using OutputTypeT = IDriverInvocation::OutputTypeT;
using InputTypeT = IDriverInvocation::InputTypeT;

OutputTypeT deriveOutputFromActionKind(clang::frontend::ActionKind Action) {
  switch(Action) {
  case clang::frontend::ActionKind::EmitAssembly:
    return OutputTypeT::S;
  case clang::frontend::ActionKind::EmitBC:
    return OutputTypeT::LLVM_BC;
  case clang::frontend::ActionKind::EmitLLVM:
    return OutputTypeT::LLVM_IR;
  case clang::frontend::ActionKind::EmitSPIRV:
    return OutputTypeT::SPIRV;
  case clang::frontend::PrintPreprocessedInput:
  case clang::frontend::RunPreprocessorOnly:
    return OutputTypeT::PREPROC;
  // PREPROC output type should probably be renamed type to something like
  // "FEOnlyOutput" (alternatively a new type could be introduced).
  // However, given the current interface should be preserved - both approaches
  // are impractical (as such modifications break ABI compatibility)
  // Such changes were undesired and thus we piggy-back on existing
  // functionality - PREPROC action implies that FE handles output by itself.
  case clang::frontend::ASTDeclList:
  case clang::frontend::ASTDump:
  case clang::frontend::ASTPrint:
  case clang::frontend::ASTView:
    return OutputTypeT::PREPROC;
  default:
    return OutputTypeT::OTHER;
  }
}
InputTypeT deriveInputTypeFromInputLanguage(clang::InputKind::Language T) {
  switch(T) {
  case clang::InputKind::CM:
    return InputTypeT::SourceCM;
  case clang::InputKind::LLVM_IR:
    return InputTypeT::LLVM_IR;
  case clang::InputKind::SPIRV:
    return InputTypeT::SPIRV;
  default:
    return InputTypeT::OTHER;
  }
}

static llvm::opt::InputArgList
parseOptions(llvm::ArrayRef<const char *> CArgs, unsigned Flag,
             llvm::opt::OptTable *Opts) {
  unsigned MissingArgIndex, MissingArgCount;
  llvm::opt::InputArgList Args = Opts->ParseArgs(CArgs,
      MissingArgIndex, MissingArgCount, Flag);
  assert(MissingArgCount == 0 &&
         "must have been covered in CompilerInvocation::CreateFromArgs");
  return Args;
}

static llvm::opt::InputArgList
parseCompilerOptions(llvm::ArrayRef<const char *> CArgs,
                     llvm::opt::OptTable *Opts) {
  return parseOptions(CArgs, clang::driver::options::CC1Option, Opts);
}

static int getRevId(const llvm::opt::InputArgList &Args) {
  return clang::getLastArgIntValue(Args, clang::driver::options::OPT_Qxcm_revid,
                                   0);
}

static IDriverInvocation::BinaryFormatT
getBinaryFormat(const llvm::opt::InputArgList &Args) {
  auto *Arg = Args.getLastArg(clang::driver::options::OPT_binary_format);
  if (!Arg)
    return IDriverInvocation::BinaryFormatT::DEFAULT;
  return llvm::StringSwitch<IDriverInvocation::BinaryFormatT>(Arg->getValue())
      .Case("ocl", IDriverInvocation::BinaryFormatT::OCL)
      .Case("ze", IDriverInvocation::BinaryFormatT::ZE)
      .Default(IDriverInvocation::BinaryFormatT::DEFAULT);
}

static bool getTimePasses(const llvm::opt::InputArgList &Args) {
  return Args.hasArg(clang::driver::options::OPT_ftime_report);
}

static bool getPrintStats(const llvm::opt::InputArgList &Args) {
  return Args.hasArg(clang::driver::options::OPT_show_stats);
}

static std::string getStatsFile(const llvm::opt::InputArgList &Args) {
  return Args.getLastArgValue(clang::driver::options::OPT_dump_stats);
}

static std::string getVCApiOptions(const llvm::opt::InputArgList &Args,
                                   clang::CompilerInstance &Clang) {
  std::string VCApiOptions = "";
  if (Args.hasArg(clang::driver::options::OPT_mCM_optimize_none))
    VCApiOptions += " -optimize=none";

  if (Args.hasArg(clang::driver::options::OPT_vc_use_plain_2d_images))
    VCApiOptions += " -vc-use-plain-2d-images";
  if (Args.hasArg(clang::driver::options::OPT_mCM_disable_jmpi))
    VCApiOptions += " -fno-jump-tables";
  if (Args.hasArg(clang::driver::options::OPT_mCM_no_vector_decomposition))
    VCApiOptions += " -ze-no-vector-decomposition";
  if (Args.hasArg(clang::driver::options::OPT_mCM_translate_legacy))
    VCApiOptions += " -ftranslate-legacy-memory-intrinsics";

  if (auto *Arg = Args.getLastArg(
          clang::driver::options::OPT_Qxcm_register_file_size)) {
    llvm::StringRef Val = Arg->getValue();
    if (Val == "auto") {
      VCApiOptions += " -ze-intel-enable-auto-large-GRF-mode";
    } else {
      VCApiOptions += " -ze-exp-register-file-size=";
      VCApiOptions += Val;
    }
  }

  // pass -fp-contract option
  if (const llvm::opt::Arg *A =
          Args.getLastArg(clang::driver::options::OPT_ffp_contract)) {
    llvm::StringRef Val = A->getValue();
    VCApiOptions += " -ffp-contract=";
    VCApiOptions += Val;
  }
  const auto &Invocation = Clang.getInvocation();
  if (Invocation.getCodeGenOpts().getDebugInfo() !=
      clang::codegenoptions::NoDebugInfo)
    VCApiOptions += " -g";

  return VCApiOptions;
}

struct OptionInfoT {
  IDriverInvocation::BinaryFormatT BinaryFormat;
  bool TimePasses;
  bool PrintStats;
  std::string StatsFile;
  std::string VCApiOptions;
  int RevId;
};

static OptionInfoT
makeAdditionalOptionParsing(const std::vector<const char *> &CArgs,
                            clang::CompilerInstance &Clang) {
  // Opts create info table which will be set up in parseCompilerOptions and
  // used in getBinaryFormat.
  std::unique_ptr<llvm::opt::OptTable> Opts =
      clang::driver::createDriverOptTable();

  auto ParsedArgs = parseCompilerOptions(CArgs, Opts.get());
  OptionInfoT Info;
  Info.BinaryFormat = getBinaryFormat(ParsedArgs);
  Info.TimePasses = getTimePasses(ParsedArgs);
  Info.PrintStats = getPrintStats(ParsedArgs);
  Info.StatsFile = getStatsFile(ParsedArgs);
  Info.VCApiOptions = getVCApiOptions(ParsedArgs, Clang);
  Info.RevId = getRevId(ParsedArgs);
  return Info;
}

class Cc1ExtraOptionInfoT {
  bool OPT_mCM_enforce_disable_free;
public:
  Cc1ExtraOptionInfoT(llvm::ArrayRef<const char *> Args) {
    std::unique_ptr<llvm::opt::OptTable> Opts =
        clang::driver::createDriverOptTable();
    auto ParsedArgs = parseCompilerOptions(Args, Opts.get());
    OPT_mCM_enforce_disable_free =
      ParsedArgs.hasArg(clang::driver::options::OPT_mCM_enforce_disable_free);
  }
  bool hasOPT_mCM_enforce_disable_free() {
    return OPT_mCM_enforce_disable_free;
  }
  void applyTo(clang::CompilerInstance &Clang) {
    if (!hasOPT_mCM_enforce_disable_free())
      Clang.getFrontendOpts().DisableFree = 0;
  }
};

// TODO: all non-debug llvm::errs() output should be moved to diagnostics
wrapper::IDriverInvocationImpl*
createDriverInvocationFromCCArgs(const std::vector<const char*> &CArgs,
                                 DiagnosticSubsystem &DS) {
  clang::CompilerInstance Clang;

  if (!clang::CompilerInvocation::CreateFromArgs(
        Clang.getInvocation(), CArgs.data(), CArgs.data() + CArgs.size(),
        *DS.Diags)) {
    llvm::errs() << "FEWrapper fatal error: could not create compilerInvocation\n";
    return nullptr;
  }
  if (DS.Diags->hasErrorOccurred()) {
    llvm::errs() << "Fatal error was encountered\n";
    return nullptr;
  }

  auto OptionInfo = makeAdditionalOptionParsing(CArgs, Clang);

  OutputTypeT OutputType =
    deriveOutputFromActionKind(Clang.getFrontendOpts().ProgramAction);

  auto &Inputs = Clang.getFrontendOpts().Inputs;
  InputTypeT InputType = Inputs.empty() ? InputTypeT::NONE :
      deriveInputTypeFromInputLanguage(Inputs[0].getKind().getLanguage());

  std::string InputFilename;
  if (!Inputs.empty()) {
    if (Inputs.size() != 1) {
      llvm::errs() << "FEWrapper fatal error: we support only one input (for now)\n";
      return nullptr;
    }
    InputFilename = Inputs[0].getFile();
  } else {
    llvm::errs() << "FEWrapper fatal error: no inputs were detected\n";
    return nullptr;
  }
  auto OutputFilename = Clang.getFrontendOpts().OutputFile;

  const auto& TO = Clang.getTargetOpts();

  using TargetRuntimeT = IDriverInvocation::TargetRuntimeT;
  TargetRuntimeT TargetRuntime = TargetRuntimeT::L0;
  if (std::any_of(TO.FeaturesAsWritten.begin(), TO.FeaturesAsWritten.end(),
      [](const std::string& Feature) { return Feature == "+ocl_runtime"; }))
    TargetRuntime = TargetRuntimeT::OCL;
  if (std::any_of(TO.FeaturesAsWritten.begin(), TO.FeaturesAsWritten.end(),
      [](const std::string& Feature) { return Feature == "+ze_runtime"; }))
    TargetRuntime = TargetRuntimeT::L0;

  auto TargetArch = TO.CPU;

  if (DebugEnabled) {
    llvm::errs() << "-----\n";
    llvm::errs() << "requested arch: <" << TargetArch << ">\n";
    llvm::errs() << "requested runtime: " << static_cast<int>(TargetRuntime) << "\n";
    llvm::errs() << "-----\n";
  }

  IDriverInvocation::SeqStrT FrontendArgs;
  IDriverInvocation::SeqStrT BackendArgs;
  // extract "-mllvm ..." options
  size_t CArgsNum = CArgs.size();
  for (size_t i = 0; i < CArgsNum; ++i) {
    // if we have an abrupt end to -mllvm option - it shall go to FeArgs
    if ((strcmp(CArgs[i], "-mllvm") == 0) && ((i + 1) < CArgsNum)) {
      ++i;
      BackendArgs.emplace_back(CArgs[i]);
    }
    else {
      FrontendArgs.emplace_back(CArgs[i]);
    }
  }
  auto *Result = wrapper::IDriverInvocationImpl::createInvocation(
      std::move(FrontendArgs), std::move(BackendArgs));
  Result->setIOParams(std::move(InputFilename), InputType,
                      std::move(OutputFilename), OutputType);
  Result->setTargetParams(OptionInfo.BinaryFormat, TargetRuntime,
                          std::move(TargetArch), TO.FeaturesAsWritten,
                          OptionInfo.TimePasses, OptionInfo.PrintStats,
                          OptionInfo.StatsFile);
  Result->setVCApiOptions(OptionInfo.VCApiOptions);
  Result->setRevId(OptionInfo.RevId);
  return Result;
}

wrapper::IDriverInvocationImpl *
makeDriverInvocationFromCompilation(clang::driver::Compilation &C,
                                    llvm::SmallVectorImpl<const char *> &Args,
                                    DiagnosticSubsystem &DS) {

  const auto &Jobs = C.getJobs();

  if (DebugEnabled)
    Jobs.Print(llvm::errs(), "\n", true);

  if (Jobs.empty()) {
    if (C.getArgs().hasArg(clang::driver::options::OPT_help))
      return wrapper::IDriverInvocationImpl::createHelp();

    if (C.getArgs().hasArg(clang::driver::options::OPT__version))
      return wrapper::IDriverInvocationImpl::createShowVersion();

    llvm::errs() << "FEWrapper fatal error: no Jobs created\n";
    return nullptr;
  }
  if (Jobs.size() != 1) {
    llvm::errs() << "FEWrapper unexpected number of jobs were created\n";
    return nullptr;
  }
  const auto &Cmd = llvm::cast<clang::driver::Command>(*Jobs.begin());
  const auto &CCArgs = Cmd.getArguments();

  if (DebugEnabled)
    std::for_each(CCArgs.begin(), CCArgs.end(), [](const char* p) {
        llvm::errs() << "CompilationArg: " << p << "\n";
      });

  llvm::BumpPtrAllocator Alloc;
  llvm::StringSaver Saver(Alloc);
  auto COpts = makeCOpts(CCArgs.begin(), CCArgs.end(), Saver);
  return createDriverInvocationFromCCArgs(COpts, DS);
}

} // namespace

extern "C" INTEL_CM_CLANGFE_DLL_DECL int IntelCMClangFEGetRevId(
    Intel::CM::ClangFE::IDriverInvocation const *DriverInvocPtr) {
  return static_cast<wrapper::IDriverInvocationImpl const *>(DriverInvocPtr)
      ->getRevId();
}

extern "C" INTEL_CM_CLANGFE_DLL_DECL bool IntelCMClangFEIsShowVersionInvocation(
    Intel::CM::ClangFE::IDriverInvocation const *DriverInvocPtr) {
  return static_cast<wrapper::IDriverInvocationImpl const *>(DriverInvocPtr)
      ->isShowVersion();
}

extern "C" INTEL_CM_CLANGFE_DLL_DECL bool IntelCMClangFEGetPrintStats(
    Intel::CM::ClangFE::IDriverInvocation const *DriverInvocPtr) {
  return static_cast<wrapper::IDriverInvocationImpl const *>(DriverInvocPtr)
      ->getPrintStats();
}

extern "C" INTEL_CM_CLANGFE_DLL_DECL const char *IntelCMClangFEGetStatsFile(
    Intel::CM::ClangFE::IDriverInvocation const *DriverInvocPtr) {
  return static_cast<wrapper::IDriverInvocationImpl const *>(DriverInvocPtr)
      ->getStatsFile().c_str();
}

extern "C" INTEL_CM_CLANGFE_DLL_DECL const char *IntelCMClangFEGetVCApiOptions(
    Intel::CM::ClangFE::IDriverInvocation const *DriverInvocPtr) {
  return static_cast<wrapper::IDriverInvocationImpl const *>(DriverInvocPtr)
      ->getVCApiOptions().c_str();
}

extern "C" INTEL_CM_CLANGFE_DLL_DECL Intel::CM::ClangFE::IDriverInvocation *
IntelCMClangFEBuildDriverInvocation(int argc, const char * argv[]) {

  llvm::BumpPtrAllocator Alloc;
  llvm::StringSaver Saver(Alloc);

  DiagnosticSubsystem DS(llvm::errs());

  auto TargetAndMode =
      clang::driver::ToolChain::getTargetAndModeFromProgramName("./cmc");

  clang::driver::Driver TheDriver("./cmc", "genx64", *DS.Diags);
  TheDriver.setCheckInputsExist(false);
  TheDriver.setTargetAndMode(TargetAndMode);

  if (DebugEnabled)
    std::for_each(argv, argv + argc, [](const char* p) {
        llvm::errs() << "InputArg: " << p << "\n";
      });

  llvm::SmallVector<const char *, 16> Args;
  Args.push_back("cmc");
  insertTargetAndModeArgs(TargetAndMode, Args, Saver);
  Args.insert(Args.end(), argv, argv + argc);

  if (DebugEnabled)
    std::for_each(Args.begin(), Args.end(), [](const char* p) {
      llvm::errs() << "ProcessedArg: " << p << "\n";
    });

  std::unique_ptr<clang::driver::Compilation> C(TheDriver.BuildCompilation(Args));
  if (!C) {
    llvm::errs() << "FEWrapper fatal error: empty compilation :(\n";
    return nullptr;
  }

  return makeDriverInvocationFromCompilation(*C, Args, DS);
}

extern "C" INTEL_CM_CLANGFE_DLL_DECL Intel::CM::ClangFE::IOutputArgs *
IntelCMClangFECompile(const Intel::CM::ClangFE::IInputArgs *InArgs) {

  wrapper::OutputArgsBuilder OutArgsBuilder;

  auto error_stream = OutArgsBuilder.getLogStream();
  DiagnosticSubsystem DS(*error_stream);

  auto CStrCompOpts = wrapper::getCompOpts<const char *>(InArgs);
  // TODO: consider moving O0 enforcement to AdaptorCM
  CStrCompOpts.push_back("-O0");

  // Facilities to pass extra -cc1 options for debug purposes.
  // Options are expected to be separated by ';'.
  // Example: "-mllvm;-debug-pass=Structure"
  llvm::BumpPtrAllocator Alloc;
  llvm::StringSaver Saver(Alloc);
  auto Cc1Extra = llvm::sys::Process::GetEnv("IGC_CMFE_CC1_EXTRA");
  if (Cc1Extra) {

    if (DebugEnabled)
      llvm::errs() << "Extra CC1 options passed: " << Cc1Extra.getValue() << "\n";

    llvm::SmallVector<llvm::StringRef, 4> ExtraArgs;
    llvm::StringRef(Cc1Extra.getValue()).split(ExtraArgs, ';');
    std::transform(ExtraArgs.begin(), ExtraArgs.end(),
                   std::back_inserter(CStrCompOpts),
                   [&Saver](const llvm::StringRef& S) {
                     return Saver.save(S).data();
                   });
  }

  if (DebugEnabled)
    std::for_each(CStrCompOpts.begin(), CStrCompOpts.end(), [](const char* p) {
        llvm::errs() << "FeArg: " << p << "\n";
      });

  clang::CompilerInstance Clang;

  clang::HeaderSearchOptions &HS = Clang.getHeaderSearchOpts();
  // CM_INCLUDE_DIR has highest priority in system header search process.
  if (auto IncludeDirOpt = llvm::sys::Process::GetEnv("CM_INCLUDE_DIR"))
    HS.AddPath(IncludeDirOpt.getValue(), clang::frontend::System, false, true);

  if (!clang::CompilerInvocation::CreateFromArgs(
        Clang.getInvocation(), CStrCompOpts.data(),
        CStrCompOpts.data() + CStrCompOpts.size(), *DS.Diags)) {
    llvm::errs() << "FEWrapper fatal error: could not create compilerInvocation\n";
    return nullptr;
  }

  // At this time header search options were filled with arguments so
  // embedded headers will have lowest priority in search.
  HS.AddPath(BuiltinHeadersRoot, clang::frontend::System, false, true);
  // Do not use any of standard paths.
  HS.UseStandardSystemIncludes = 0;
  HS.UseBuiltinIncludes = 0;

  auto IRStream = OutArgsBuilder.getIRStream();
  Clang.setOutputStream(std::move(IRStream));

  auto MemFS = createFileSystem(InArgs, getTheOnlyInputFileName(Clang));
  Clang.setVirtualFileSystem(MemFS);

  auto Cc1ExtraOptionInfo = Cc1ExtraOptionInfoT{
    llvm::makeArrayRef(CStrCompOpts.data(), CStrCompOpts.size())};
  Cc1ExtraOptionInfo.applyTo(Clang);

  clang::TextDiagnosticPrinter Consumer(*error_stream,
                                        &Clang.getDiagnosticOpts(), false);
  Clang.createDiagnostics(&Consumer, false);
  if (!Clang.hasDiagnostics()) {
    llvm::errs() << "FEWrapper fatal error: could not create diagnostics\n";
    return nullptr;
  }

  llvm::cl::ResetAllOptionOccurrences();
  Clang.setVerboseOutputStream(*error_stream);
  auto success = clang::ExecuteCompilerInvocation(&Clang);
  OutArgsBuilder.setStatus(success);
  // stream is buffered, so update underlying string
  error_stream->flush();
  auto OutArgs = wrapper::OutputArgsImpl::create(OutArgsBuilder);

  return OutArgs;
}

int IntelCMClangFEGetInterfaceVersion() {
  return Intel::CM::ClangFE::InterfaceVersion;
}
