/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

#include "GenX.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/Utils.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Option/ArgList.h"

#include <sstream>
#include <utility>

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

int GenX::getGenXRevId(const std::string &CPU,
                       const ArgList &Args,
                       const Driver *Drv) {
  int RevId = 0;
  if (Arg *A = Args.getLastArg(options::OPT_Qxcm_revid)) {
    if (StringRef(A->getValue()).getAsInteger(0, RevId) && Drv)
      Drv->Diag(diag::err_drv_invalid_int_value)
          << A->getAsString(Args) << A->getValue();
    return RevId;
  }

  // if no option, try to deduce from CPU
  RevId = llvm::StringSwitch<int>(CPU)
            .Case("PVC", 0)
            .Case("PVCXT", 5)
            .Default(0);

  return RevId;
}


static void reportUnsupportedStepping(const std::string &CPU,
                                      const std::string &Stepping) {
  std::string Err = std::string(
      (Twine("stepping <") + Stepping + "> is not supported for <" + CPU + ">")
          .str());
  llvm::report_fatal_error(Err);
}

static std::string deriveFinalCpuNameFromStepping(const std::string &CPU,
                                                  const std::string &Stepping) {
  if (Stepping.empty()) {
    return CPU;
  }

  if (Stepping == "A" && CPU == "PVC")
    return "PVC";

  if (Stepping == "B" && CPU == "PVC")
    return "PVCXT";


  if (CPU == "DG2")
    return CPU;

  reportUnsupportedStepping(CPU, Stepping);
  return CPU;
}

static bool isDeprecatedTarget(StringRef CanonicalCPU) {
  return llvm::StringSwitch<bool>(CanonicalCPU)
      .Case("BDW", true)
      .Case("BXT", true)
      .Case("GLK", true)
      .Default(false);
}

static std::string getCanonicalGenXTargetCPU(const std::string &CPU,
                                             const ArgList &Args,
                                             const Driver *Drv) {
  // As side-effect of the way we accept the CM command line options for
  // backwards compatiblity, the CPU string may be prefixed by '=' or ':'.
  // If so, remove the prefix character.
  size_t CPUNameStart = CPU.find_first_not_of("=:");
  if (CPUNameStart == std::string::npos)
    CPUNameStart = 0;
  std::string CPUName = CPU.substr(CPUNameStart);
  std::transform(CPUName.begin(), CPUName.end(), CPUName.begin(), ::toupper);

  // Ensure the CPU name is in canonical form
  auto *CanonicalCPU = llvm::StringSwitch<const char *>(CPUName)
                           .Cases("GEN7_5", "HSW", "HSW")
                           .Cases("GEN8", "BDW", "BDW")
                           .Cases("GEN8LP", "GEN8_5", "CHV", "CHV")
                           .Cases("GEN9", "CFL", "SKL", "SKL")
                           .Cases("GEN9LP", "BXT", "BXT")
                           .Cases("GEN9_5", "GEN9P5", "KBL", "KBL")
                           .Cases("GEN9_5LP", "GEN9P5LP", "GLK", "GLK")
                           .Cases("GEN11", "ICL", "ICL")
                           .Cases("GEN11LP", "ICLLP", "ICLLP")
                           .Cases("GEN12LP", "TGLLP", "TGLLP")
                           .Case("RKL", "RKL")
                           .Case("DG1", "DG1")
                           .Cases("XEHP", "XEHP_SDV", "XEHP_SDV")
                           .Case("DG2", "DG2")
                           .Case("ADLP", "ADLP")
                           .Case("ADLS", "ADLS")
                           .Case("ADLN", "ADLN")
                           .Case("MTL", "MTL")
                           .Case("PVC", "PVC")
                           .Case("PVCXT", "PVCXT")
                           .Default("");

  // Check canonical name but report original argument.
  if (Drv && isDeprecatedTarget(CanonicalCPU))
    Drv->Diag(clang::diag::warn_cm_deprecated_target) << CPU;

  int RevId = GenX::getGenXRevId(CPU, Args, Drv);
  if (CPUName == "PVC" && RevId >= 3)
    return "PVCXT";
  return CanonicalCPU;
}

std::string GenX::getGenXTargetCPU(const ArgList &Args, const Driver *Drv) {
  // GenX target CPU may be specified using one of /Qxcm_jit_target=xxx,
  // -mcpu=xxx, or -march=xxx.
  if (const Arg *A = Args.getLastArg(options::OPT_Qxcm_jit_target)) {
    auto Jit_CPU = getCanonicalGenXTargetCPU(A->getValue(), Args, Drv);
    if (!Jit_CPU.empty())
      return std::move(Jit_CPU);
  }
  if (const Arg *A = Args.getLastArg(options::OPT_mcpu_EQ)) {
    auto Mcpu_CPU = getCanonicalGenXTargetCPU(A->getValue(), Args, Drv);
    if (!Mcpu_CPU.empty())
      return std::move(Mcpu_CPU);
  }
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ)) {
    auto March_CPU = getCanonicalGenXTargetCPU(A->getValue(), Args, Drv);
    if (!March_CPU.empty())
      return std::move(March_CPU);
  }
  // no GenX target CPU specified
  if (Drv)
    Drv->Diag(clang::diag::err_cm_unknown_arch);
  return "";
}

bool GenX::isCMBinaryFormat(const Driver &Drv, const ArgList &Args) {
  auto *Arg = Args.getLastArg(options::OPT_binary_format);
  if (!Arg || std::string(Arg->getValue()) == "cm") {
    // CMRT binary is default
    Drv.Diag(clang::diag::warn_cm_deprecated_cmrt);
    return true;
  }
  return false;
}

void GenX::getGenXTargetFeatures(const Driver &D, const llvm::Triple &Triple,
                                 const ArgList &Args,
                                 std::vector<StringRef> &Features) {
  if (Args.getLastArg(options::OPT_mCM_disable_jmpi))
    Features.push_back("+disable_jmpi");
  if (Args.getLastArg(options::OPT_mCM_warn_callable))
    Features.push_back("+warn_callable");
  if (Args.getLastArg(options::OPT_mCM_no_vector_decomposition))
    Features.push_back("+disable_vec_decomp");
  if (Args.getLastArg(options::OPT_mCM_translate_legacy))
    Features.push_back("+translate_legacy_message");

  if (!isCMBinaryFormat(D, Args))
    Features.push_back("+ocl_runtime");
}
