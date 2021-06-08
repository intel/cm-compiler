/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

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

#ifndef LLVM_CLANG_TOOLS_CLANG_CMOC_COMMON_H
#define LLVM_CLANG_TOOLS_CLANG_CMOC_COMMON_H

#include "llvm/Support/ErrorHandling.h"

#include <string>
#include <vector>

enum class InputKind { TEXT, IR, SPIRV, Unsupported };

struct ILTranslationResult {
  std::vector<char> KernelBinary;
};

void translateIL(const std::string &CPUName, int RevId,
                 const std::string &BinaryFormat, const std::string &Features,
                 const std::string &APIOptions,
                 const std::vector<std::string> &BackendOptions,
                 const std::vector<char> &SPIRV_IR, InputKind IK,
                 bool TimePasses, ILTranslationResult &Result);

bool isCmocDebugEnabled();

[[noreturn]] static void FatalError(const std::string &Err) {
  llvm::report_fatal_error(Err, false);
}

#endif
