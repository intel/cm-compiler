/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

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
  vector<int, 6> r1 = m.row;                // expected '(' // expected-error{{expected '('}}
  vector<int, 6> r2 = m.row(;               // expected expression // expected-error{{expected expression}}
  vector<int, 6> r3 = m.row(1;              // expected ')' // expected-error{{expected ')'}}
  vector<int, 6> r4 = m.row();              // expected expression // expected-error{{expected expression}}
  vector<int, 6> r5 = m.row(0);             // OK
  vector<int, 6> r6 = m.template row(1);    // OK
  vector<int, 6> r7 = m.row(0);             // OK
  vector<int, 6> r8 = v.row(0);             // not a matrix // expected-error{{row() member function is only valid for matrix/matrix_ref}}
  vector<int, 6> r9 = v.template row(0);    // not a matrix // expected-error{{row() member function is only valid for matrix/matrix_ref}}
  vector<int, 6> r10 = m.row(-1);           // index negative // expected-error{{row index must be positive}}
  vector<int, 6> r11 = m.row(3);            // OK
  vector<int, 6> r12 = m.row(4);            // index out of bounds // expected-warning{{row index '4' is out of bounds, matrix has 4 rows}}
  vector<int, 6> r13 = m.row(ia);           // OK
  vector<int, 6> r14 = m.row(si);           // index not an integer // expected-error{{row index must be an integer expression}}
  m.row(0);                                 // expression result unused // expected-warning{{expression result unused}}
  s1.row(2);                                // no row() in s // expected-error{{no member named 'row' in 'S1'}}
  s2.row = 1;                               // OK
}
