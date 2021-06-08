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
int i;
SurfaceIndex si;
struct S1 {
  int a;
} s1;
struct S2 {
  int n_rows;
} s2;

_GENX_MAIN_
void test() {
  int r1 = m.n_rows;                // expected '('
  int r2 = m.n_rows(;               // expected ')'
  int r3 = m.n_rows(3;              // expected ')'
  int r4 = m.n_rows();              // OK
  int r5 = m.n_rows(0);             // unexpected expression
  int r6 = m.template n_rows();     // OK
  int r7 = v.n_rows();              // not a matrix
  int r8 = v.template n_rows();     // not a matrix
  int r9 = s1.n_rows;               // no n_rows in s1
  int r10 = s1.n_rows();            // no n_rows() in s1
  int r11 = s2.n_rows;              // OK
  int r12 = s2.n_rows();            // n_rows not a function
  int r13 = s2.template n_rows();   // not a template
  s2.n_rows = 1;                    // OK
  s2.template n_rows = 2;           // not a template
  m.n_rows() = 2;                   // not assignable
}
// CHECK: n_rows.cpp(18,20):  error: expected '('
// CHECK: n_rows.cpp(19,21):  error: expected ')'
// CHECK: n_rows.cpp(20,21):  error: expected ')'
// CHECK: n_rows.cpp(22,21):  error: expected ')'
// CHECK: n_rows.cpp(24,14):  error: n_rows() member function is only valid for matrix/matrix_ref
// CHECK: n_rows.cpp(25,23):  error: n_rows() member function is only valid for matrix/matrix_ref
// CHECK: n_rows.cpp(26,15):  error: no member named 'n_rows' in 'S1'
// CHECK: n_rows.cpp(27,16):  error: no member named 'n_rows' in 'S1'
// CHECK: n_rows.cpp(29,22):  error: called object type 'int' is not a function or function pointer
// CHECK: n_rows.cpp(30,25):  error: 'n_rows' following the 'template' keyword does not refer to a template
// CHECK: n_rows.cpp(32,15):  error: 'n_rows' following the 'template' keyword does not refer to a template
// CHECK: n_rows.cpp(33,14):  error: expression is not assignable
// CHECK: 12 errors generated.
