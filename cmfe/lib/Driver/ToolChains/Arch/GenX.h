//===--- GenX.h - GenX-specific Tool Helpers --------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_GENX_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_GENX_H

#include "clang/Driver/Driver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include <string>
#include <vector>

namespace clang {
namespace driver {
namespace tools {
namespace GenX {

// get CPU from args
std::string getGenXTargetCPU(const llvm::opt::ArgList &Args);


// get features from args and triple
void getGenXTargetFeatures(const Driver &D, const llvm::Triple &Triple,
                           const llvm::opt::ArgList &Args,
                           std::vector<llvm::StringRef> &Features);

// binary format for CMRT is required according to options
bool isCMBinaryFormat(const llvm::opt::ArgList &Args);

} // end namespace GenX
} // namespace tools
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_GENX_H
