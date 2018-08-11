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
#include "llvm/Support/Path.h"
#include <cstdlib> // ::getenv

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

const char *getFinalizerPlatform(const char *CPU) {
  // Unfortunately, the finalizer doesn't support all platforms, so we map
  // any unsupported platforms to the most appropriate supported one.
  const char *FinalizerPlatform = llvm::StringSwitch<const char *>(CPU)
                                      .Case("KBL", "SKL")
                                      .Case("GLK", "BXT")
                                      .Default(CPU);

  return FinalizerPlatform;
}

static bool mayDisableIGA(std::string CPU) {
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
                        .Case("CNL", true)
                        .Default(false);

  return mayDisable;
}


/// GenX Tools
// GenX assembly is performed by the GenX Finalizer.
void tools::GenX::Assemble::ConstructJob(Compilation &C, const JobAction &JA,
                                         const InputInfo &Output,
                                         const InputInfoList &Inputs,
                                         const ArgList &Args,
                                         const char *LinkingOutput) const {
  // We only call the GenX Finalizer if the -Qxcm_jit_target option and a valid
  // genx variant were specified.
  const char *Platform = tools::GenX::getGenXTargetCPU(Args);
  if (strlen(Platform) && Args.hasArg(options::OPT_Qxcm_jit_target)) {
    ArgStringList FinalizerArgs;
    // Discard any '=' or ':' delimeters from the filename
    const char *Filename = Output.getFilename();
    while (*Filename == '=' || *Filename == ':')
      ++Filename;
    if (strlen(Filename) == 0)
      return;
    FinalizerArgs.push_back(Filename);
    FinalizerArgs.push_back("-outputCisaBinaryName");
    FinalizerArgs.push_back(Filename);
    FinalizerArgs.push_back("-output");
    FinalizerArgs.push_back("-binary");
    FinalizerArgs.push_back("-dumpcommonisa");
    FinalizerArgs.push_back("-dumpvisa");
    if (Args.hasArg(options::OPT_Qxcm_noschedule))
      FinalizerArgs.push_back("-noschedule");
    if (Args.hasArg(options::OPT_Qxcm_print_asm_count))
      FinalizerArgs.push_back("-printasmcount");
    if (Args.hasArg(options::OPT_mCM_printregusage)) {
      FinalizerArgs.push_back("-noroundrobin");
      FinalizerArgs.push_back("-printregusage");
    }
    if (Args.hasArg(options::OPT_Qxcm_opt_report))
      FinalizerArgs.push_back("-optreport");
    if (Args.hasArg(options::OPT_Qxcm_release))
      FinalizerArgs.push_back("-stripcomments");
    if (Arg *A = Args.getLastArg(options::OPT_mCM_unique_labels)) {
      const char *LabelName = A->getValue();
      if ((LabelName[0] == '=') || (LabelName[0] == ':'))
        LabelName = &LabelName[1];
      if (strlen(LabelName)) {
        FinalizerArgs.push_back("-uniqueLabels");
        FinalizerArgs.push_back(LabelName);
      }
    }

    // Add any finalizer options specified using -mCM_jit_option.
    // Options may be single options or multiple options within quotes.
    // There may be any number of instances of -mCM_jit_option.
    for (auto A : Args.filtered(options::OPT_mCM_jit_option)) {
      std::string JitOpt = A->getValue();
      size_t OptStart = JitOpt.find_first_not_of("=:");
      if (OptStart == std::string::npos)
        OptStart = 0;
      for (unsigned i = OptStart, size = JitOpt.size(); i < size; ++i) {
        if (JitOpt[i] == ' ') {
          if (i > OptStart)
            FinalizerArgs.push_back(
                Args.MakeArgString(JitOpt.substr(OptStart, i - OptStart)));
          OptStart = i + 1;
        }
      }
      if (OptStart < JitOpt.size())
        FinalizerArgs.push_back(Args.MakeArgString(JitOpt.substr(OptStart)));
    }

    FinalizerArgs.push_back("-platform");
    FinalizerArgs.push_back(getFinalizerPlatform(Platform));

    // For GenX variants below Gen11 we disable IGA by default, by passing the
    // -disableIGASyntax option to the finalizer.
    // IGA syntax may be enabled (or more accurately not disabled) either by
    // the -cm_enableiga option, or by the ENABLE_IGA environment variable
    // having a non-zero value. If IGA is enabled by the environment variable
    // we issue a warning to advise the user of this.
    if (!Args.hasArg(options::OPT_menableiga) &&
        !Args.hasArg(options::OPT_mCM_enableiga)) {
      const char *enableIGA = getenv("ENABLE_IGA");
      if (enableIGA && (atol(enableIGA)) > 0) {
        if (mayDisableIGA(Platform)) {
          const Driver &D = getToolChain().getDriver();
          D.Diag(diag::warn_cm_iga_enabled);
        }
      } else if (mayDisableIGA(Platform))
        FinalizerArgs.push_back("-disableIGASyntax");
    }

    // Scalar jmp instructions will be translated into goto's
    if (Args.hasArg(options::OPT_mCM_disable_jmpi)) {
      FinalizerArgs.push_back("-noScalarJmp");
      FinalizerArgs.push_back("-disableStructurizer");
    }

    // Determine the CM Finalizer executable - by default it is assumed to be
    // in the same directory as the cmc executable.
    StringRef P = C.getDriver().Dir;
    SmallString<128> FPath(P);
#ifdef LLVM_ON_WIN32
    llvm::sys::path::append(FPath, "GenX_IR.exe");
#else
    llvm::sys::path::append(FPath, "GenX_IR");
#endif
    FinalizerExecutable = FPath.c_str();

    // The default Finalizer path can be overidden by the -mCM_genx_assembler
    // option.
    if (const Arg *A = Args.getLastArg(options::OPT_mCM_genx_assembler)) {
      FinalizerExecutable = A->getValue();
      // remove any ':' or '=' that may prefix the executable path that may
      // be present due to the way we parse the -mCM_genx_assembler option to
      // allow for backwards compatibility
      size_t FinalizerStart = FinalizerExecutable.find_first_not_of("=:");
      if (FinalizerStart == std::string::npos)
        FinalizerExecutable.clear();
      else
        FinalizerExecutable = FinalizerExecutable.substr(FinalizerStart);
    }
    if (FinalizerExecutable.size())
      C.addCommand(llvm::make_unique<Command>(
          JA, *this, FinalizerExecutable.c_str(), FinalizerArgs, Inputs));
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

void GenX::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                     ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  // For MDF CM compilations we only want the CM include files in the include
  // search path.
  SmallString<128> CmIncludeDir(getDriver().InstalledDir);
  llvm::sys::path::append(CmIncludeDir, "../include_llvm");
  addSystemInclude(DriverArgs, CC1Args, CmIncludeDir.str());
}
