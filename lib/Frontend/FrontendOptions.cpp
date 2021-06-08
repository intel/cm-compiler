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

#include "clang/Frontend/FrontendOptions.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;

InputKind FrontendOptions::getInputKindForExtension(StringRef Extension) {
  return llvm::StringSwitch<InputKind>(Extension)
    .Cases("ast", "pcm", InputKind(InputKind::Unknown, InputKind::Precompiled))
    .Case("c", InputKind::C)
    .Cases("S", "s", InputKind::Asm)
    .Case("i", InputKind(InputKind::C).getPreprocessed())
    .Case("ii", InputKind(InputKind::CXX).getPreprocessed())
    .Case("cui", InputKind(InputKind::CUDA).getPreprocessed())
    .Case("m", InputKind::ObjC)
    .Case("mi", InputKind(InputKind::ObjC).getPreprocessed())
    .Cases("mm", "M", InputKind::ObjCXX)
    .Case("mii", InputKind(InputKind::ObjCXX).getPreprocessed())
    .Cases("C", "cc", "cp", InputKind::CXX)
    .Cases("cpp", "CPP", "c++", "cxx", "hpp", InputKind::CXX)
    .Case("cppm", InputKind::CXX)
    .Case("iim", InputKind(InputKind::CXX).getPreprocessed())
    .Case("cl", InputKind::OpenCL)
    .Case("cu", InputKind::CUDA)
    .Cases("ll", "bc", InputKind::LLVM_IR)
    .Case("spv", InputKind::SPIRV)
    .Default(InputKind::Unknown);
}
