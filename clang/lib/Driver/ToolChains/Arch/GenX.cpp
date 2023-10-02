/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2023 Intel Corporation

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

static uint32_t getGenXTargetCPUId(const std::string &CPU, int RevId) {
  // As side-effect of the way we accept the CM command line options for
  // backwards compatiblity, the CPU string may be prefixed by '=' or ':'.
  // If so, remove the prefix character.
  size_t CPUNameStart = CPU.find_first_not_of("=:");
  if (CPUNameStart == std::string::npos)
    CPUNameStart = 0;
  std::string CPUName = CPU.substr(CPUNameStart);
  std::transform(CPUName.begin(), CPUName.end(), CPUName.begin(), ::tolower);

  CPUName = llvm::StringSwitch<std::string>(CPUName)
                .Case("gen9lp", "bxt")
                .Cases("gen9_5", "gen9p5", "kbl")
                .Cases("gen9_5lp", "gen9p5lp", "glk")
                .Case("gen11lp", "gen11")
                .Case("tgl", "tgllp")
                .Case("adls", "adl-s")
                .Case("adlp", "adl-p")
                .Case("adln", "adl-n")
                .Cases("xehp", "xehp_sdv", "xe_hp_sdv", "xehp-sdv", "xe-hp-sdv",
                       "xe-hp")
                .Case("dg2", "acm-g10")
                .Case("mtl", "mtl-p")
                .Case("pvcxt", "pvc")
                .Default(CPUName);

  uint32_t CPUId = GenX::getDeviceId(CPUName);

  if (CPUId != 0 && RevId >= 0) {
    CPUId &= ~0x3f;
    CPUId |= RevId & 0x3f;
  }

  return CPUId;
}

uint32_t GenX::getGenXTargetCPU(const ArgList &Args, const Driver *Drv) {
  // GenX target CPU may be specified using one of /Qxcm_jit_target=xxx,
  // -mcpu=xxx or -march=xxx.
  const Arg *ArchArg =
      Args.getLastArg(options::OPT_Qxcm_jit_target, options::OPT_mcpu_EQ,
                      options::OPT_march_EQ);
  const char *CPUName = ArchArg ? ArchArg->getValue() : "unknown";

  const Arg *RevArg = Args.getLastArg(options::OPT_Qxcm_revid);
  int RevId = -1;
  if (RevArg && StringRef(RevArg->getValue()).getAsInteger(0, RevId) && Drv)
    Drv->Diag(diag::err_drv_invalid_int_value)
        << RevArg->getAsString(Args) << RevArg->getValue();

  uint32_t CPUId = getGenXTargetCPUId(CPUName, RevId);
  if (Drv && CPUId == 0)
    Drv->Diag(clang::diag::err_drv_invalid_arch_name) << CPUName;

  return CPUId;
}

void GenX::getGenXTargetFeatures(const Driver &D, const llvm::Triple &Triple,
                                 const ArgList &Args,
                                 std::vector<StringRef> &Features) {
  Features.push_back("+ocl_runtime");

  if (Args.getLastArg(options::OPT_mCM_warn_callable))
    Features.push_back("+warn_callable");
}
