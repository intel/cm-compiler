//===--- GenX.cpp - GenX ToolChain Implementations --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "GenX.h"
#include "Arch/GenX.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include <cstdlib>

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

namespace {

StringRef fixupComplexArgument(StringRef Input) {
  if (Input.consume_front("="))
    return Input;
  Input.consume_front(":");
  return Input;
}

std::string getFinalizerPlatform(StringRef CPU) {
  // Unfortunately, the finalizer doesn't support all platforms, so we map
  // any unsupported platforms to the most appropriate supported one.
  auto FinalizerPlatform = llvm::StringSwitch<StringRef>(CPU)
                               .Case("KBL", "SKL")
                               .Case("GLK", "BXT")
                               .Case("", "SKL")
                               .Default(CPU);

  return FinalizerPlatform.str();
}

bool mayDisableIGA(const std::string &CPU) {
  // IGA may be disabled for targets before ICL.
  // CPU is expected to be a canonical Genx target name.
  bool mayDisable = llvm::StringSwitch<bool>(CPU)
                        .Case("HSW", true)
                        .Case("BDW", true)
                        .Case("CHV", true)
                        .Case("SKL", true)
                        .Case("BXT", true)
                        .Case("KBL", true)
                        .Case("GLK", true)
                        .Default(false);

  return mayDisable;
}

ArgStringList constructCompatibilityFinalizerOptions(const ArgList &Args,
                                                     const Driver &DR) {
  ArgStringList CompatibilityArgs;

  if (Args.getLastArg(options::OPT_mdump_asm) ||
      llvm::sys::Process::GetEnv("CM_FORCE_ASSEMBLY_DUMP")) {
    CompatibilityArgs.push_back("-dumpcommonisa");
    CompatibilityArgs.push_back("-output");
    CompatibilityArgs.push_back("-binary");
  }
  if (Args.hasArg(options::OPT_Qxcm_noschedule))
    CompatibilityArgs.push_back("-noschedule");
  if (Args.hasArg(options::OPT_Qxcm_print_asm_count))
    CompatibilityArgs.push_back("-printasmcount");
  if (Args.hasArg(options::OPT_mCM_printregusage)) {
    CompatibilityArgs.push_back("-printregusage");
  }
  if (Args.hasArg(options::OPT_Qxcm_opt_report))
    CompatibilityArgs.push_back("-optreport");
  if (Args.hasArg(options::OPT_Qxcm_release))
    CompatibilityArgs.push_back("-stripcomments");
  if (Arg *A = Args.getLastArg(options::OPT_mCM_unique_labels)) {
    auto LabelName = fixupComplexArgument(A->getValue());
    if (LabelName.size()) {
      CompatibilityArgs.push_back("-uniqueLabels");
      CompatibilityArgs.push_back(LabelName.data());
    }
  }

  // Add any finalizer options specified using -mCM_jit_option.
  // Options may be single options or multiple options within quotes.
  // There may be any number of instances of -mCM_jit_option.
  for (auto A : Args.filtered(options::OPT_mCM_jit_option)) {
    SmallVector<StringRef, 4> TmpArgs;
    // This construct allows user to specify several options with a single
    // Qxcm argument. Like this:
    //  Qxcm_jit_option="-SWSBTokenNum 32 -TotalGRFNum 256"
    fixupComplexArgument(A->getValue()).trim().split(TmpArgs, ' ', -1, false);
    for (const StringRef &a: TmpArgs) {
      CompatibilityArgs.push_back(Args.MakeArgString(a));
    }
  }

  auto Platform = getFinalizerPlatform(tools::GenX::getGenXTargetCPU(Args));
  CompatibilityArgs.push_back("-platform");
  CompatibilityArgs.push_back(Args.MakeArgString(Platform));


  // For GenX variants below Gen11 we disable IGA by default, by passing the
  // -disableIGASyntax option to the finalizer.
  // IGA syntax may be enabled (or more accurately not disabled) either by
  // the -cm_enableiga option, or by the ENABLE_IGA environment variable
  // having a non-zero value. If IGA is enabled by the environment variable
  // we issue a warning to advise the user of this.
  if (!Args.hasArg(options::OPT_menableiga) &&
      !Args.hasArg(options::OPT_mCM_enableiga)) {

    auto enableIGA = llvm::sys::Process::GetEnv("ENABLE_IGA");
    if (enableIGA && (std::atol(enableIGA.getValue().c_str()) > 0)) {
      if (mayDisableIGA(Platform)) {
        DR.Diag(diag::warn_cm_iga_enabled);
      }
    }
    else if (mayDisableIGA(Platform))
      CompatibilityArgs.push_back("-disableIGASyntax");
  }

  // Scalar jmp instructions will be translated into goto's
  if (Args.hasArg(options::OPT_mCM_disable_jmpi)) {
    CompatibilityArgs.push_back("-noScalarJmp");
    CompatibilityArgs.push_back("-disableStructurizer");
  }

  // preRA scheduler options.
  if (Args.hasArg(options::OPT_Qxcm_preschedule)) {
    CompatibilityArgs.push_back("-presched");
    if (Arg *A = Args.getLastArg(options::OPT_Qxcm_preschedule_ctrl)) {
      CompatibilityArgs.push_back("-presched-ctrl");
      CompatibilityArgs.push_back(fixupComplexArgument(A->getValue()).data());
    }
    if (Arg *A = Args.getLastArg(options::OPT_Qxcm_preschedule_rp)) {
      CompatibilityArgs.push_back("-presched-rp");
      CompatibilityArgs.push_back(fixupComplexArgument(A->getValue()).data());
    }
  }
  return CompatibilityArgs;
}
}

/// GenX tool chain
GenX::GenX(const Driver &D, const llvm::Triple &Triple, const ArgList &Args)
    : ToolChain(D, Triple, Args) {
  // ProgramPaths are found relative to compiler executable.
}

Tool *GenX::buildAssembler() const { return new tools::GenX::Assemble(*this); }

bool GenX::isPICDefaultForced() const { return false; }

bool GenX::isPICDefault() const { return false; }

bool GenX::isPIEDefault() const { return false; }

void GenX::addClangTargetOptions(const llvm::opt::ArgList &  DriverArgs,
                                 llvm::opt::ArgStringList &  CC1Args,
                                 Action::OffloadKind DeviceOffloadKind) const {

  // Enforce backend to print parameters used to invoke finalizer
  if (DriverArgs.getLastArg(options::OPT_mCM_printfargs) ||
      DriverArgs.getLastArg(options::OPT_v)) {
    CC1Args.push_back("-mllvm");
    CC1Args.push_back("-cg-print-finalizer-args");
  }

  // Emit asm files with old style
  if (DriverArgs.getLastArg(options::OPT_mCM_old_asm_name)||
      llvm::sys::Process::GetEnv("CM_FORCE_ASSEMBLY_DUMP")) {
    // AsmName := <base-filename> + '_' + <index> + ".(visa)asm"
    auto Input = DriverArgs.getLastArg(options::OPT_INPUT);
    StringRef BaseName = llvm::sys::path::stem(Input->getValue());
    std::string AsmName = BaseName.str();
    auto ArgStr = "-asm-name=" + AsmName;
    const char *AsmNameC = DriverArgs.MakeArgString(ArgStr);
    CC1Args.push_back("-mllvm");
    CC1Args.push_back(AsmNameC);
  }

  auto AsmNameArg = DriverArgs.getLastArg(options::OPT__SLASH_Fa);
  if (!AsmNameArg)
    AsmNameArg = DriverArgs.getLastArg(options::OPT_Qxcm_asm_output);
  if (AsmNameArg) {
    // Push desired asm name to GenXCisaBuilder
    auto AsmNameL = fixupComplexArgument(AsmNameArg->getValue());
    if (AsmNameL.size()) {
      auto ArgStr = "-asm-name=" + AsmNameL;
      const char *AsmNameC = DriverArgs.MakeArgString(ArgStr);
      CC1Args.push_back("-mllvm");
      CC1Args.push_back(AsmNameC);
    }
  }

  // Reverse kernels list order if user asm name is presented
  if (DriverArgs.getLastArg(options::OPT_mCM_reverse_kernels)) {
      CC1Args.push_back("-mllvm");
      CC1Args.push_back("-reverse-kernels");
  }

  // if the assembler is overridden, then we enforce running GenX_IR
  // and thus llvm should emit visa
  if (DriverArgs.getLastArg(options::OPT_mCM_genx_assembler)) {
    llvm::errs() << "warning: assembler override is deprecated\n";
    // construction for GenX_IR invocation is handled in GenX::Assemble::ConstructJob
    // here we request emitting visa and bail out
    CC1Args.push_back("-mllvm");
    CC1Args.push_back("-emit-visa");
    return;
  }

  ArgStringList CompatibilityArgs =
    constructCompatibilityFinalizerOptions(DriverArgs, getDriver());
  if (CompatibilityArgs.empty())
    return;

  std::vector<std::string> Args;
  Args.assign(CompatibilityArgs.begin(), CompatibilityArgs.end());
  auto ArgsStr = "-finalizer-opts=" + llvm::join(Args, " ");
  const char* FOpts = DriverArgs.MakeArgString(ArgsStr);

  CC1Args.push_back("-mllvm");
  CC1Args.push_back(FOpts);
}
