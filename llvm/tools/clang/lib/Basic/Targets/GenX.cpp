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

/// handleTargetFeatures - Perform initialization based on the user
/// configured set of features.
bool GenXTargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                          DiagnosticsEngine &Diags) {
  return true;
}
