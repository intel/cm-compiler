/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -ferror-limit=99 -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

#include <cm/cm.h>

matrix<int,4,6> m;
vector<int,4> v;
struct S1 {
  int a;
} s1;
struct S2 {
  int genx_select;
} s2;

_GENX_MAIN_
void test() {
  vector_ref<int, 4> r1 = m.genx_select;                   // expected '<' // expected-error{{expected '<'}}
  vector_ref<int, 4> r2 = m.genx_select<;                  // expected expression // expected-error{{expected expression}}
  vector_ref<int, 4> r3 = m.genx_select<4;                 // expected '>' // expected-error{{expected '>'}}
  vector_ref<int, 4> r4 = m.genx_select<4,1,1;             // expected '>' // expected-error{{expected '>'}}
  vector_ref<int, 4> r5 = m.genx_select<4,1,1,1;           // expected '>' // expected-error{{expected '>'}}
  vector_ref<int, 4> r6 = m.genx_select<4,1,1,1();         // not a function // expected-error{{called object type 'int' is not a function or function pointer}}
  vector_ref<int, 4> r7 = m.genx_select<4,1,1,1>;          // expected '(' // expected-error{{expected '('}}
  vector_ref<int, 4> r8 = m.genx_select<9,8,7,6>(;         // expected expression // expected-error{{expected expression}}
  vector_ref<int, 4> r9 = m.genx_select<4,1,1,1>();        // deprecated // expected-error{{genx_select() is deprecated - use replicate()}}
  vector_ref<int, 4> r10 = m.genx_select<4,1,1,1>(3);      // deprecated // expected-error{{genx_select() is deprecated - use replicate()}}
  vector_ref<int, 4> r11 = m.genx_select<4,1,1,1>(3,1);    // deprecated // expected-error{{genx_select() is deprecated - use replicate()}}
  vector_ref<int, 4> r12 = m.genx_select();                // expected '<' // expected-error{{expected '<'}}

  vector_ref<int, 4> r13 = v.genx_select;                  // expected '<' // expected-error{{expected '<'}}
  vector_ref<int, 4> r14 = v.genx_select<;                 // expected expression // expected-error{{expected expression}}
  vector_ref<int, 4> r15 = v.genx_select<4,1,1,1;          // expected '>' // expected-error{{expected '>'}}
  vector_ref<int, 4> r16 = v.genx_select<4;                // expected '>' // expected-error{{expected '>'}}
  vector_ref<int, 4> r17 = v.genx_select<4,1;              // expected '>' // expected-error{{expected '>'}}
  vector_ref<int, 4> r18 = v.genx_select<4,1();            // not a function // expected-error{{called object type 'int' is not a function or function pointer}}
  vector_ref<int, 4> r19 = v.genx_select<4,1>;             // expected '(' // expected-error{{expected '('}}
  vector_ref<int, 4> r20 = v.genx_select<9,8>(;            // expected expression // expected-error{{expected expression}}
  vector_ref<int, 4> r21 = v.genx_select<4,1>();           // deprecated // expected-error{{genx_select() is deprecated - use replicate()}}
  vector_ref<int, 4> r22 = v.genx_select<4,1>(3);          // deprecated // expected-error{{genx_select() is deprecated - use replicate()}}
  vector_ref<int, 4> r23 = v.genx_select<4,1>(3,1);        // deprecated // expected-error{{genx_select() is deprecated - use replicate()}}
  vector_ref<int, 4> r24 = v.genx_select();                // expected '<' // expected-error{{expected '<'}}

  int r25 = s1.genx_select;                                // no member genx_select in s1 // expected-error{{no member named 'genx_select' in 'S1'}}
  int r26 = s1.genx_select();                              // no member genx_select in s1 // expected-error{{no member named 'genx_select' in 'S1'}}
  int r27 = s1.genx_select<4;                              // no member genx_select in s // expected-error{{no member named 'genx_select' in 'S1'}}
  int r28 = s1.template genx_select;                       // genx_select not a template
  int r29 = s2.genx_select;                                // OK
  int r30 = s2.genx_select();                              // not a function // expected-error{{called object type 'int' is not a function or function pointer}}
  int r31 = s2.genx_select<4;                              // OK
  int r32 = s2.template genx_select;                       // genx_select not a template // expected-error{{'genx_select' following the 'template' keyword does not refer to a template}}

  m.genx_select<1,1,1,1>() = 9;                            // deprecated
  v.genx_select<1>() = 7;                                  // deprecated

}
