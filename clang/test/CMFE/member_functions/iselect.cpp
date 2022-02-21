/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -ferror-limit=99 -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

#include <cm/cm.h>

matrix<int,10,6> m;
vector<int,8> v;
vector<ushort,4> idx1;
vector<ushort,4> idx2;
vector<ushort,6> idx3;
vector<float,6> idx4;
ushort idx5;
struct S1 {
  int a;
} s1;
struct S2 {
  int iselect;
} s2;

_GENX_MAIN_
void test() {
  vector<int,4> r1 = v.iselect;                                 // expected '(' // expected-error{{expected '('}}
  vector<int,4> r2 = v.iselect(;                                // expected index // expected-error{{expected expression}}
  vector<int,4> r3 = v.iselect();                               // expected index // expected-error{{expected expression}}
  vector<int,4> r4 = v.iselect(idx1;                            // expected ')' // expected-error{{expected ')'}}
  vector<int,4> r5 = v.iselect(idx1);                           // OK
  vector<int,4> r6 = v.iselect(idx4);                           // index not ushort vector // expected-error{{iselect expects vector<unsigned short, N> index type, 'vector<float,6>'}}
  vector<int,4> r7 = v.iselect(idx5);                           // index not vector // expected-error{{iselect expects vector<unsigned short, N> index type, 'ushort' (aka 'unsigned short')}}
  vector<int,4> r8 = v.iselect(idx1,idx3);                      // one index expected // expected-error{{vector iselect expects 1 index expression}}
  vector<int,4> r9 = v.template iselect(idx1);                  // OK
  vector<int,6> r10 = v.iselect(idx1).iselect(idx3);            // OK
  vector<int,6> r11 = v.iselect(idx1).iselect(idx1,idx2);       // one index expected // expected-error{{vector iselect expects 1 index expression}}

  vector<int,4> r12 = m.iselect;                                // expected '(' // expected-error{{expected '('}}
  vector<int,4> r13 = m.iselect(;                               // expected index // expected-error{{expected expression}}
  vector<int,4> r14 = m.iselect();                              // expected index // expected-error{{expected expression}}
  vector<int,4> r15 = m.iselect(idx1;                           // expected ')' // expected-error{{expected ')'}}
  vector<int,4> r16 = m.iselect(idx1);                          // two indices expected // expected-error{{matrix iselect expects 2 index expressions}}
  vector<int,4> r17 = m.iselect(idx1,idx2);                     // OK
  vector<int,4> r18 = m.iselect(idx1,idx2,idx3,idx1);           // two indices expected // expected-error{{matrix iselect expects 2 index expressions}}
  vector<int,4> r19 = m.iselect(idx4,idx3);                     // index not ushort // expected-error{{iselect expects vector<unsigned short, N> index type, 'vector<float,6>'}}
  vector<int,4> r20 = m.iselect(idx3,idx4);                     // index not ushort // expected-error{{iselect expects vector<unsigned short, N> index type, 'vector<float,6>'}}
  vector<int,4> r21 = m.iselect(idx1,idx3);                     // indices different size // expected-error{{matrix iselect row and column index size must be the same, 4 != 6}}
  vector<int,4> r22 = m.template iselect(idx1,idx2);            // OK
  vector<int,6> r23 = m.iselect(idx1,idx3).iselect(idx3);       // OK // expected-error{{matrix iselect row and column index size must be the same, 4 != 6}}
  vector<int,6> r24 = m.iselect(idx1,idx2).iselect(idx2,idx1);  // one index expected // expected-error{{vector iselect expects 1 index expression}}

  v.iselect(idx3);                                      // expression result unused // expected-warning{{expression result unused}}
  m.iselect(idx1,idx2);                                 // expression result unused // expected-warning{{expression result unused}}
  v.iselect(idx1) = 9;                                  // not assignable // expected-error{{expression is not assignable}}
  m.iselect(idx1,idx2) = 1;                             // not assignable // expected-error{{expression is not assignable}}

  int r30 = s1.iselect;                                 // no member iselect in s1 // expected-error{{no member named 'iselect' in 'S1'}}
  int r31 = s1.iselect(idx1);                           // no member iselect in s1 // expected-error{{no member named 'iselect' in 'S1'}}
  int r32 = s1.template iselect;                        // no member iselect in s1 // expected-error{{no member named 'iselect' in 'S1'}}
  int r33 = s2.iselect;                                 // OK
  int r34 = s2.iselect(idx1);                           // iselect not a function // expected-error{{called object type 'int' is not a function or function pointer}}
  int r35 = s2.template iselect(idx1);                  // iselect not a template // expected-error{{'iselect' following the 'template' keyword does not refer to a template}}
}
