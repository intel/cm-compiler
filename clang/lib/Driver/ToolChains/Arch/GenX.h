/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

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
uint32_t getGenXTargetCPU(const llvm::opt::ArgList &Args,
                          const Driver *Drv = nullptr);

// get features from args and triple
void getGenXTargetFeatures(const Driver &D, const llvm::Triple &Triple,
                           const llvm::opt::ArgList &Args,
                           std::vector<llvm::StringRef> &Features);

uint32_t getDeviceId(const std::string &Name);

inline constexpr uint32_t encodeGmdId(uint32_t Major, uint32_t Minor,
                                      uint32_t Revision) {
  return ((Major & 0x3ff) << 22 | (Minor & 0xff) << 14 | (Revision & 0x3f));
}

} // end namespace GenX
} // end namespace tools
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_GENX_H
