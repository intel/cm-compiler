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
  int n_elems;
} s2;

_GENX_MAIN_
void test() {
  int r1 = v.n_elems;                // expected '(' // expected-error{{expected '('}}
  int r2 = v.n_elems(;               // expected ')' // expected-error{{expected ')'}}
  int r3 = v.n_elems(1;              // expected ')' // expected-error{{expected ')'}}
  int r4 = v.n_elems(0);             // unexpected expression // expected-error{{expected ')'}}
  int r5 = v.n_elems();              // OK
  int r6 = v.template n_elems();     // OK
  int r7 = m.n_elems;                // expected '(' // expected-error{{expected '('}}
  int r8 = m.n_elems(;               // expected ')' // expected-error{{expected ')'}}
  int r9 = m.n_elems(0);             // unexpected expression // expected-error{{expected ')'}}
  int r10 = m.n_elems();             // OK
  int r11 = m.template n_elems();    // OK
  int r12 = s1.n_elems;              // no n_elems in s1 // expected-error{{no member named 'n_elems' in 'S1'}}
  int r13 = s1.n_elems();            // no n_elems() in s1 // expected-error{{no member named 'n_elems' in 'S1'}}
  int r14 = s2.n_elems;              // OK
  int r15 = s2.n_elems();            // n_elems not a function // expected-error{{called object type 'int' is not a function or function pointer}}
  int r16 = s2.template n_elems();   // not a template // expected-error{{'n_elems' following the 'template' keyword does not refer to a template}}
  s2.n_elems = 1;                    // OK
  s2.template n_elems = 2;           // not a template // expected-error{{'n_elems' following the 'template' keyword does not refer to a template}}
  v.n_elems() = 2;                   // not assignable // expected-error{{expression is not assignable}}
}
