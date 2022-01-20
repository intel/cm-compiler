/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -emit-llvm -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

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
  int r1 = m.n_rows;                // expected '(' // expected-error{{expected '('}}
  int r2 = m.n_rows(;               // expected ')' // expected-error{{expected ')'}}
  int r3 = m.n_rows(3;              // expected ')' // expected-error{{expected ')'}}
  int r4 = m.n_rows();              // OK
  int r5 = m.n_rows(0);             // unexpected expression // expected-error{{expected ')'}}
  int r6 = m.template n_rows();     // OK
  int r7 = v.n_rows();              // not a matrix // expected-error{{n_rows() member function is only valid for matrix/matrix_ref}}
  int r8 = v.template n_rows();     // not a matrix // expected-error{{n_rows() member function is only valid for matrix/matrix_ref}}
  int r9 = s1.n_rows;               // no n_rows in s1 // expected-error{{no member named 'n_rows' in 'S1'}}
  int r10 = s1.n_rows();            // no n_rows() in s1 // expected-error{{no member named 'n_rows' in 'S1'}}
  int r11 = s2.n_rows;              // OK
  int r12 = s2.n_rows();            // n_rows not a function // expected-error{{called object type 'int' is not a function or function pointer}}
  int r13 = s2.template n_rows();   // not a template // expected-error{{'n_rows' following the 'template' keyword does not refer to a template}}
  s2.n_rows = 1;                    // OK
  s2.template n_rows = 2;           // not a template // expected-error{{'n_rows' following the 'template' keyword does not refer to a template}}
  m.n_rows() = 2;                   // not assignable // expected-error{{expression is not assignable}}
}
