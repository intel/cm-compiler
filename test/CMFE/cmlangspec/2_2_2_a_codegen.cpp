/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

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

// RUN: %cmc -g0 -march=SKL -emit-llvm -S -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

// Check that row() generates rdregion from matrix that is stored to vector.

#include <cm/cm.h>

// CHECK-LABEL: @test_func(
extern "C"
_GENX_ void test_func() {
  matrix<short, 4, 8> m;
// CHECK:      [[MATRIX:%[^ ]+]] = alloca <32 x i16>
// CHECK-NEXT: [[VECTOR:%[^ ]+]] = alloca <8 x i16>
// CHECK-NEXT: [[LOAD:%[^ ]+]] = load <32 x i16>, <32 x i16>* [[MATRIX]]
// CHECK-NEXT: [[ROWS:%[^ ]+]] = call <8 x i16> @llvm.genx.rdregioni.v8i16.v32i16.i16(<32 x i16> [[LOAD]], i32 0, i32 8, i32 1, i16 32, i32 8)
// CHECK-NEXT: [[COLS:%[^ ]+]] = call <8 x i16> @llvm.genx.rdregioni.v8i16.v8i16.i16(<8 x i16> [[ROWS]], i32 8, i32 8, i32 1, i16 0, i32 8)
// CHECK-NEXT: store <8 x i16> [[COLS]], <8 x i16>* [[VECTOR]]
  vector<short, 8> r(m.row(2));
}
