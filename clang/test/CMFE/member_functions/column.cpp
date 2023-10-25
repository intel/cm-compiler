/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -emit-llvm -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

#include <cm/cm.h>

matrix<int,4,6> m;
vector<int,4> v;
int ia;
SurfaceIndex si;
struct S1 {
  int a;
} s1;
struct S2 {
  int column;
} s2;

_GENX_MAIN_
void test() {
  vector<int, 4> r1 = m.column;                // expected '(' // expected-error{{expected '('}}
  vector<int, 4> r2 = m.column(;               // expected expression // expected-error{{expected expression}}
  vector<int, 4> r3 = m.column(1;              // expected ')' // expected-error{{expected ')'}}
  vector<int, 4> r4 = m.column();              // expected expression // expected-error{{expected expression}}
  vector<int, 4> r5 = m.column(0);             // OK
  vector<int, 4> r6 = m.template column(1);    // OK
  vector<int, 4> r7 = m.column(0);             // OK
  vector<int, 4> r8 = v.column(0);             // not a matrix // expected-error{{column() member function is only valid for matrix/matrix_ref}}
  vector<int, 4> r9 = v.template column(0);    // not a matrix // expected-error{{column() member function is only valid for matrix/matrix_ref}}
  vector<int, 4> r10 = m.column(-1);           // index negative // expected-error{{column index must be positive}}
  vector<int, 4> r11 = m.column(5);            // OK
  vector<int, 4> r12 = m.column(6);            // index out of bounds // expected-warning{{column index '6' is out of bounds, matrix has 6 columns}}
  vector<int, 4> r13 = m.column(ia);           // OK
  vector<int, 4> r14 = m.column(si);           // index not an integer // expected-error{{column index must be an integer expression}}
  m.column(0);                                 // expression result unused // fixme-expected-warning{{expression result unused}}
  s1.column(2);                                // no column() in s // expected-error{{no member named 'column' in 'S1'}}
  s2.column = 1;                               // OK
}
