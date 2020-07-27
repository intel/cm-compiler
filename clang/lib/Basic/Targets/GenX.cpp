//===--- GenX.cpp - Implement GenX target feature support -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements Intel GenX TargetInfo objects.
//
//===----------------------------------------------------------------------===//

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
  // OCL runtime specific headers support
  OCLRuntime = std::any_of(
      Features.begin(), Features.end(),
      [](const std::string &Feature) { return Feature == "+ocl_runtime"; });
  return true;
}
