/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

// This file implements Intel GenX TargetInfo objects.

#include "GenX.h"

#include "clang/Basic/CodeGenOptions.h"

using namespace clang;
using namespace clang::targets;

// There's another copy of this in llvm/lib/Target/GenX/GenXTargetMachine.cpp
static std::string getDL(bool Is64Bit) {
  return Is64Bit ? "e-p:64:64-i64:64-n8:16:32" : "e-p:32:32-i64:64-n8:16:32";
}

static const unsigned GenXAddrSpaceMap[] = {
    0, // Default
    1, // opencl_global
    3, // opencl_local
    2, // opencl_constant
    0, // opencl_private
    4, // opencl_generic
    0, // cuda_device
    0, // cuda_constant
    0  // cuda_shared
};

GenXTargetInfo::GenXTargetInfo(const llvm::Triple &Triple,
                               unsigned TargetPointerWidth)
    : TargetInfo(Triple) {
  assert((TargetPointerWidth == 32 || TargetPointerWidth == 64) &&
         "only supports 32- and 64-bit modes.");
  resetDataLayout(getDL(TargetPointerWidth == 64));
  // GenX target always uses Itanium C++ ABI.
  TheCXXABI.set(TargetCXXABI::GenericItanium);
  LongWidth = LongAlign = TargetPointerWidth;
  PointerWidth = PointerAlign = TargetPointerWidth;
  AddrSpaceMap = &GenXAddrSpaceMap;
  switch (TargetPointerWidth) {
  case 32:
    SizeType = TargetInfo::UnsignedInt;
    PtrDiffType = TargetInfo::SignedInt;
    IntPtrType = TargetInfo::SignedInt;
    break;
  case 64:
    SizeType = TargetInfo::UnsignedLong;
    PtrDiffType = TargetInfo::SignedLong;
    IntPtrType = TargetInfo::SignedLong;
    break;
  default:
    llvm_unreachable("TargetPointerWidth must be 32 or 64");
  }
}

void GenXTargetInfo::adjustTargetOptions(const CodeGenOptions &CGOpts,
                                         TargetOptions &TargetOpts) const {
  TargetOpts.CMMaxSLMSize =
      CGOpts.MaxSLMSize == 0 ? MaxSLMSize : CGOpts.MaxSLMSize;
  TargetOpts.CMMaxOWordBlock =
      CGOpts.MaxOBRWSize == 0 ? MaxOWordBlock : CGOpts.MaxOBRWSize;
  TargetOpts.CMIEFByPass = CGOpts.IEFByPass || HasIEFByPass;
}

/// handleTargetFeatures - Perform initialization based on the user
/// configured set of features.
bool GenXTargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                          DiagnosticsEngine &Diags) {
  return true;
}

bool GenXTargetInfo::setCPU(const std::string &Name) {
  CPU = Name;
  if (std::sscanf(CPU.c_str(), "%u.%u.%u", &Major, &Minor, &Revision) != 3)
    return false;
  setCPUProperties();

  // FIXME: get rid of this stuff
  if (Major < 12)
    HasIEFByPass = false;
  if (Major >= 12)
    MaxOWordBlock = 16;

  return true;
}

void GenXTargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  Builder.defineMacro("_WIN32");
  Builder.defineMacro("_WINNT");
  Builder.defineMacro("_X86_");
  Builder.defineMacro("__MSVCRT__");
  Builder.defineMacro("_HAS_EXCEPTIONS", "0");

  Builder.defineMacro("__CLANG_CM");
  Builder.defineMacro("__CM");
  Builder.defineMacro("__CMC");
  Builder.defineMacro("__VARIADIC_TEMPLATES");

  Builder.defineMacro("__CM_INTEL_TARGET_MAJOR", std::to_string(Major));
  Builder.defineMacro("__CM_INTEL_TARGET_MINOR", std::to_string(Minor));
  Builder.defineMacro("__CM_INTEL_TARGET_REVISION", std::to_string(Revision));

  if (HasFP64)
    Builder.defineMacro("CM_HAS_DOUBLE", "1");

  Builder.defineMacro("CM_MAX_SLM_SIZE", std::to_string(MaxSLMSize));
}
bool GenXTargetInfo::hasFeature(StringRef Feature) const {
  return llvm::StringSwitch<bool>(Feature)
      .Case("double", HasFP64)
      .Default(true);
}
