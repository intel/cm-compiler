//===-- llgen.cpp - Implement the LLVM INTEL Graphics GEN Code Generator --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the llgen Intel Graphics GEN code generator driver. It provides a
// convenient command-line interface for generating vISA or native GEN assembly
// language code, given LLVM bitcode or spir-v input.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/CodeGen/CommandFlags.def"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/SPIRV.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetMachine.h"

#include <memory>
#include <fstream>

using namespace llvm;

// General options for llgen.  Other pass-specific options are specified
// within the corresponding llgen passes, and target-specific options
// and back-end code generation options are specified with the target machine.
//
static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input bitcode>"), cl::init("-"));

static cl::opt<std::string>
InputLanguage("x", cl::desc("Input language ('ir' or 'spirv')"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Output filename"), cl::value_desc("filename"));

static cl::opt<std::string>
TargetTriple("mtriple", cl::desc("Override target triple for module"));

static cl::opt<bool> NoVerify("disable-verify", cl::Hidden,
                              cl::desc("Do not verify input module"));

struct LLGenDiagHandler : public DiagnosticHandler {
  bool *HasError;
  LLGenDiagHandler(bool *HasErrorPtr) : HasError(HasErrorPtr) {}
  bool handleDiagnostics(const DiagnosticInfo &DI) override {
    if (DI.getSeverity() == DS_Error)
      *HasError = true;

    if (auto *Remark = dyn_cast<DiagnosticInfoOptimizationBase>(&DI))
      if (!Remark->isEnabled())
        return true;

    DiagnosticPrinterRawOStream DP(errs());
    errs() << LLVMContext::getDiagnosticMessagePrefix(DI.getSeverity()) << ": ";
    DI.print(DP);
    errs() << "\n";
    return true;
  }
};

static int compileModule(char **argv, LLVMContext &Context);

// main - Entry point for the llgen compiler.
//
int main(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);

  // Enable debug stream buffering.
  EnableDebugBuffering = true;

  LLVMContext Context;
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.

  // Initialize targets first.
  InitializeAllTargets();
  InitializeAllTargetMCs();

  // Initialize codegen and IR passes used by llgen so that the -print-after,
  // -print-before, and -stop-after options work.
  PassRegistry *Registry = PassRegistry::getPassRegistry();
  initializeCore(*Registry);
  initializeLoopStrengthReducePass(*Registry);
  initializeUnreachableBlockElimLegacyPassPass(*Registry);
  initializeConstantHoistingLegacyPassPass(*Registry);
  initializeScalarOpts(*Registry);

  // Register the target printer for --version.
  cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);
  cl::ParseCommandLineOptions(argc, argv, "llvm system compiler\n");

  bool HasError = false;
  Context.setDiagnosticHandler(llvm::make_unique<LLGenDiagHandler>(&HasError));

  if (InputLanguage != "" && InputLanguage != "ir" &&
      InputLanguage != "spv") {
    errs() << argv[0] << "Input language must be '', 'llvm IR' or 'spirv IR'\n";
    return 1;
  }

  return compileModule(argv, Context);
}

static std::unique_ptr<ToolOutputFile> GetOutputStream(const char *TargetName,
                                                       Triple::OSType OS,
                                                       const char *ProgName) {
  // If we don't yet have an output filename, make one.
  if (OutputFilename.empty()) {
    if (InputFilename == "-")
      OutputFilename = "-";
    else {
      // If InputFilename ends in .bc or .ll, remove it.
      StringRef IFN = InputFilename;
      if (IFN.endswith(".bc") || IFN.endswith(".ll"))
        OutputFilename = IFN.drop_back(3);
      else if (IFN.endswith(".spv"))
        OutputFilename = IFN.drop_back(4);
      else
        OutputFilename = IFN;
      OutputFilename += ".isa";
    }
  }

  // Open the file.
  std::error_code EC;
  sys::fs::OpenFlags OpenFlags = sys::fs::F_None;
  auto FDOut = llvm::make_unique<ToolOutputFile>(OutputFilename, EC, OpenFlags);
  if (EC) {
    errs() << EC.message() << '\n';
    return nullptr;
  }

  return FDOut;
}

static int compileModule(char **argv, LLVMContext &Context) {
  // Load the module to be compiled...
  SMDiagnostic Err;
  std::unique_ptr<Module> M;
  Triple TheTriple;

  bool SkipModule = MCPU == "help" ||
      (!MAttrs.empty() && MAttrs.front() == "help");

  // If user just wants to list available options, skip module loading
  if (!SkipModule) {
    if (InputLanguage == "spv" ||
        (InputLanguage == "" && StringRef(InputFilename).endswith(".spv"))) {
      std::ifstream IFS(InputFilename, std::ios::binary);
      llvm::Module *SpirM = nullptr;
      std::string ErrMsg;
      readSPIRV(Context, IFS, SpirM, ErrMsg);
      Err = SMDiagnostic(InputFilename, SourceMgr::DK_Error,
                         "Could not open input file: " + ErrMsg);
      M.reset(SpirM);
    } else
      M = parseIRFile(InputFilename, Err, Context);

    if (!M) {
      Err.print(argv[0], errs());
      return 1;
    }

    // Verify module immediately to catch problems before doInitialization() is
    // called on any passes.
    if (!NoVerify && verifyModule(*M, &errs())) {
      errs() << argv[0] << ": " << InputFilename
             << ": error: input module is broken!\n";
      return 1;
    }

    // If we are supposed to override the target triple, do so now.
    if (!TargetTriple.empty())
      M->setTargetTriple(Triple::normalize(TargetTriple));
    TheTriple = Triple(M->getTargetTriple());
  } else
    TheTriple = Triple(Triple::normalize(TargetTriple));

  if (TheTriple.getTriple().empty())
    TheTriple.setTriple(sys::getDefaultTargetTriple());

  // Update to gen32 or gen64 arch.
  TheTriple.isArch32Bit() ? TheTriple.setArch(Triple::genx32)
                          : TheTriple.setArch(Triple::genx64);
  M->setTargetTriple(TheTriple.getTriple());

  // Get the target specific parser.
  std::string Error;
  auto TheTarget = TargetRegistry::lookupTarget(MArch, TheTriple, Error);
  if (!TheTarget) {
    errs() << argv[0] << ": " << Error;
    return 1;
  }

  std::string CPUStr = getCPUStr();
  std::string FeaturesStr = getFeaturesStr();
  CodeGenOpt::Level OLvl = CodeGenOpt::Default;
  TargetOptions Options = InitTargetOptionsFromCodeGenFlags();

  std::unique_ptr<TargetMachine> Target(TheTarget->createTargetMachine(
      TheTriple.getTriple(), CPUStr, FeaturesStr, Options, getRelocModel(),
      getCodeModel(), OLvl));

  assert(Target && "Could not allocate target machine!");

  // If we don't have a module then just exit now. We do this down
  // here since the CPU/Feature help is underneath the target machine
  // creation.
  if (SkipModule)
    return 0;

  // Figure out where we are going to send the output.
  std::unique_ptr<ToolOutputFile> Out =
      GetOutputStream(TheTarget->getName(), TheTriple.getOS(), argv[0]);
  if (!Out)
    return 1;

  // Build up all of the passes that we want to do to the module.
  legacy::PassManager PM;

  // Add an appropriate TargetLibraryInfo pass for the module's triple.
  TargetLibraryInfoImpl TLII(Triple(M->getTargetTriple()));
  PM.add(new TargetLibraryInfoWrapperPass(TLII));

  // Add the target data from the target machine, if it exists, or the module.
  M->setDataLayout(Target->createDataLayout());

  // Override function attributes based on CPUStr, FeaturesStr, and command line
  // flags.
  setFunctionAttributes(CPUStr, FeaturesStr, *M);

  LLVMTargetMachine &LLVMTM = static_cast<LLVMTargetMachine &>(*Target);
  MachineModuleInfo *MMI = new MachineModuleInfo(&LLVMTM);
  raw_pwrite_stream *OS = &Out->os();

  if (Target->addPassesToEmitFile(PM, *OS, FileType, NoVerify, MMI)) {
    const char *argv0 = argv[0];
    errs() << argv0 << ": target does not support generation of this"
           << " file type!\n";
    return 1;
  }

  // Before executing passes, print the final values of the LLVM options.
  cl::PrintOptionValues();

  PM.run(*M);
  auto HasError = ((const LLGenDiagHandler *)(Context.getDiagHandlerPtr()))->HasError;
  if (*HasError)
    return 1;

  // Declare success.
  Out->keep();
  return 0;
}