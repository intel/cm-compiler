/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

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

// XFAIL: *

#include <cm/cm.h>

#define C 255

_GENX_MAIN_ void foo(SurfaceIndex S, short a, short b, short c, short d)
{
  matrix<short,1,16> tempBuffer;
  #pragma cm_nonstrict
  tempBuffer = (C * a) + (C * b) + (C * c) + (C * d);
  write(S, 0, 0, tempBuffer);
}


// warning expected for use of #pragma cm_nonstrict
// RUN: %cmc -emit-llvm -march=SKL -- %s 2>&1 | FileCheck %s
//
// CHECK:  cm_nonstrict.cpp(8,11):  warning: cm_nonstrict is deprecated - see CM LLVM Porting Guide [-Wdeprecated]
// CHECK: 1 warning generated.
// CHECK -platform SKL

// warning for use of #pragma cm_nonstrict expected to be suppressed by -Wno-deprecated option
// RUN: %cmc -emit-llvm -march=SKL -Wno-deprecated -- %s 2>&1 | FileCheck --check-prefix=NOWARN %s
//
// NOWARN-NOT: cm_nonstrict is deprecated
// NOWARN-NOT: warning generated.
// NOWARN -platform SKL
