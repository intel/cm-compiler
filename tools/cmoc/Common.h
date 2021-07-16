/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

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
