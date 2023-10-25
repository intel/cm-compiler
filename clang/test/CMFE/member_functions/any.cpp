/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -emit-llvm -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

#include <cm/cm.h>

matrix<int,6,4> m;
vector<int,4> v;
struct S1 {
  int any;
} s1;
struct S2 {
  int x;
} s2;
_GENX_MAIN_
void test() {
  unsigned short r1 = m.any;             // missing '()' // expected-error{{expected '('}}
  unsigned short r2 = m.any(;            // missing ')' // expected-error{{expected ')'}}
  unsigned short r3 = m.any(1;           // unexpected expression // expected-error{{expected ')'}}
  unsigned short r4 = m.any(9);          // unexpected expression // expected-error{{expected ')'}}
  unsigned short r5 = m.any();           // OK
  unsigned short r6 = m.template any();  // OK
  unsigned short r7 = v.any();           // OK
  unsigned short r8 = v.template any();  // OK
  unsigned short r9 = s1.any;            // OK
  unsigned short r10 = s1.template any;  // not a template // expected-error{{'any' following the 'template' keyword does not refer to a template}}
  unsigned short r11 = s2.any();         // no member any // expected-error{{no member named 'any' in 'S2'}}
  m.any();                               // expression result unused // fixme-expected-warning{{expression result unused}}
  v.any();                               // expression result unused // fixme-expected-warning{{expression result unused}}
  m.any() = 1;                           // not assignable // expected-error{{expression is not assignable}}
  v.any() = 0;                           // not assignable // expected-error{{expression is not assignable}}
  v.any().any();                         // ushort not a structure or union // expected-error{{member reference base type 'unsigned short' is not a structure or union}}
}
