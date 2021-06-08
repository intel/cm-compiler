/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

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

#include <cm/cm.h>

#ifdef CM_GENX
#warning CM_GENX defined
#else
#warning CM_GENX not defined
#endif

#ifdef CM_GEN7_5
#warning CM_GEN7_5 defined
#endif

#ifdef CM_GEN8
#warning CM_GEN8 defined
#endif

#ifdef CM_GEN8_5
#warning CM_GEN8_5 defined
#endif

#ifdef CM_GEN9
#warning CM_GEN9 defined
#endif

#ifdef CM_GEN9_5
#warning CM_GEN9_5 defined
#endif

#ifdef CM_GEN10
#warning CM_GEN10 defined
#endif

#ifdef CM_GEN11
#warning CM_GEN11 defined
#endif

_GENX_MAIN_
void test1(SurfaceIndex S, vector<uchar,64> i) {
  vector<uchar,16> v = i.select<16,1>(0)  +
                       i.select<16,1>(16) +
                       i.select<16,1>(32) +
                       i.select<16,1>(48);
  write(S,0,0,v);
}

_GENX_MAIN_
void test2(SurfaceIndex S) {
}

// XFAIL: *

// The Finalizer is called as we have specified a target so the -mCM_print_asm_count
// option causes the gen asm instruction counts to be printed
// RUN: %cmc -emit-llvm -march=BDW -Qxcm_print_asm_count -- %s 2>&1 | FileCheck --check-prefix=CHECK-BDW %s
//
// RUN: %cmc -emit-llvm -march=BDW /Qxcm_print_asm_count -- %s 2>&1 | FileCheck --check-prefix=CHECK-BDW %s

// CHECK-BDW: cm_print_asm_count.cpp(4,2):  warning: CM_GENX defined [-W#warnings]
// CHECK-BDW: cm_print_asm_count.cpp(14,2):  warning: CM_GEN8 defined [-W#warnings]
// CHECK-BDW: 2 warnings generated.
// CHECK-BDW: -platform BDW
// CHECK-BDW: Kernel cm_print_asm_count_0 : {{[0-9]+}} asm_count 
// CHECK-BDW: Kernel cm_print_asm_count_1 : {{[0-9]+}} asm_count 
// CHECK-BDW-NOT: asm_count
