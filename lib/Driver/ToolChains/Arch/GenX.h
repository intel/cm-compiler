/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

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
std::string getGenXTargetCPU(const llvm::opt::ArgList &Args,
                             const Driver *Drv = nullptr);

// get RevId from CPU and args
int getGenXRevId(const std::string &CPU,
                 const llvm::opt::ArgList &Args,
                 const Driver *Drv = nullptr);


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
