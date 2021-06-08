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

// XFAIL: *

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck %s

#include <cm/cm.h>

matrix<int,4,6> m;
vector<int,4> v;
int ia;
SurfaceIndex si;
struct S1 {
  int a;
} s1;
struct S2 {
  int row;
} s2;

_GENX_MAIN_
void test() {
  vector<int, 6> r1 = m.row;                // expected '('
  vector<int, 6> r2 = m.row(;               // expected expression
  vector<int, 6> r3 = m.row(1;              // expected ')'
  vector<int, 6> r4 = m.row();              // expected expression
  vector<int, 6> r5 = m.row(0);             // OK
  vector<int, 6> r6 = m.template row(1);    // OK
  vector<int, 6> r7 = m.row(0);             // OK
  vector<int, 6> r8 = v.row(0);             // not a matrix
  vector<int, 6> r9 = v.template row(0);    // not a matrix
  vector<int, 6> r10 = m.row(-1);           // index negative
  vector<int, 6> r11 = m.row(3);            // OK
  vector<int, 6> r12 = m.row(4);            // index out of bounds
  vector<int, 6> r13 = m.row(ia);           // OK
  vector<int, 6> r14 = m.row(si);           // index not an integer
  m.row(0);                                 // expression result unused
  s1.row(2);                                // no row() in s
  s2.row = 1;                               // OK
}
// CHECK: row.cpp(18,28):  error: expected '('
// CHECK: row.cpp(19,29):  error: expected expression
// CHECK: row.cpp(20,30):  error: expected ')'
// CHECK: row.cpp(21,29):  error: expected expression
// CHECK: row.cpp(25,25):  error: row() member function is only valid for matrix/matrix_ref
// CHECK: row.cpp(26,34):  error: row() member function is only valid for matrix/matrix_ref
// CHECK: row.cpp(27,30):  error: row index must be positive
// CHECK: row.cpp(29,30):  warning: row index '4' is out of bounds, matrix has 4 rows [-Wcm-bounds-check]
// CHECK: row.cpp(31,30):  error: row index must be an integer expression
// CHECK: row.cpp(33,6):  error: no member named 'row' in 'S1'
// CHECK: row.cpp(32,3):  warning: expression result unused [-Wunused-value]
// CHECK: 2 warnings and 9 errors generated.
