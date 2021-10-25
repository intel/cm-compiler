/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

// This file implements Intel GenX TargetInfo objects.

#include "GenX.h"

using namespace clang;
using namespace clang::targets;

// There's another copy of this in llvm/lib/Target/GenX/GenXTargetMachine.cpp
static std::string getDL(bool Is64Bit) {
  return Is64Bit ? "e-p:64:64-i64:64-n8:16:32" : "e-p:32:32-i64:64-n8:16:32";
}

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

/// handleTargetFeatures - Perform initialization based on the user
/// configured set of features.
bool GenXTargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                          DiagnosticsEngine &Diags) {

  NativeI64Support = llvm::StringSwitch<bool>(CPU)
                         .Case("ICLLP", false)
                         .Case("RKL", false)
                         .Case("TGLLP", false)
                         .Case("DG1", false)
                         .Default(true);

  NativeDoubleSupport = llvm::StringSwitch<bool>(CPU)
                            .Case("ICLLP", false)
                            .Case("TGLLP", false)
                            .Case("RKL", false)
                            .Case("DG1", false)
                            .Default(true);

  // OCL runtime specific headers support
  OCLRuntime = std::any_of(
      Features.begin(), Features.end(),
      [](const std::string &Feature) { return Feature == "+ocl_runtime"; });
  return true;
}

bool GenXTargetInfo::setCPU(const std::string &Name) {
  bool CPUKnown = llvm::StringSwitch<bool>(Name)
                      .Case("HSW", true)
                      .Case("BDW", true)
                      .Case("CHV", true)
                      .Case("SKL", true)
                      .Case("BXT", true)
                      .Case("GLK", true)
                      .Case("KBL", true)
                      .Case("ICL", true)
                      .Case("ICLLP", true)
                      .Case("TGLLP", true)
                      .Case("RKL", true)
                      .Case("DG1", true)
                      .Case("XEHP_SDV", true)
                      .Default(false);

  if (CPUKnown)
    CPU = Name;

  return CPUKnown;
}
void GenXTargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  Builder.defineMacro("_WIN32");
  Builder.defineMacro("_WINNT");
  Builder.defineMacro("_X86_");
  Builder.defineMacro("__MSVCRT__");
  Builder.defineMacro("_HAS_EXCEPTIONS", "0");

  bool I64Emulation = !NativeI64Support && Opts.CMEmulateI64;
  if (I64Emulation)
    Builder.defineMacro("__CM_INTEGER_EMULATION_ENABLED__");

  if (NativeI64Support || I64Emulation)
    Builder.defineMacro("CM_HAS_LONG_LONG", "1");

  if (NativeDoubleSupport)
    Builder.defineMacro("CM_HAS_DOUBLE", "1");

  // OCL runtime specific headers support
  if (OCLRuntime)
    Builder.defineMacro("__CM_OCL_RUNTIME");
  if (Opts.CMUseOCLSpecPrintf)
    Builder.defineMacro("__CM_USE_OCL_SPEC_PRINTF");
}
bool GenXTargetInfo::hasFeature(StringRef Feature) const {
  return llvm::StringSwitch<bool>(Feature)
      .Case("longlong", NativeI64Support)
      .Case("double", NativeDoubleSupport)
      .Default(true);
}
