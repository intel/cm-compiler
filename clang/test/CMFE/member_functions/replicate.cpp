/*========================== begin_copyright_notice ============================

Copyright (C) 2014-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -emit-llvm -ferror-limit=999 -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

#include <cm/cm.h>

matrix<int,4,6> m;
vector<int,4> v1;
vector<short,15> v2;
struct S1 {
  int a;
} s1;
struct S2 {
  int replicate;
} s2;
int i;
const int two = 2;
const int minus_two = -2;

_GENX_MAIN_
void test() {
  vector<int,8> r1 = v1.replicate;                                       // expected '<' // expected-error{{expected '<'}}
  vector<int,8> r2 = v1.replicate<;                                      // expected expression // expected-error{{expected expression}}
  vector<int,8> r3 = v1.replicate<2;                                     // expected '>' // expected-error{{expected '>'}}
  vector<int,8> r4 = v1.replicate<2>;                                    // expected '(' // expected-error{{expected '('}}
  vector<int,8> r5 = v1.replicate<2>(;                                   // expected expression // expected-error{{expected expression}}
  vector<int,8> r6 = v1.replicate<2>();                                  // OK
  vector<int,8> r7 = v1.replicate<two>();                                // OK
  vector<int,8> r8 = v1.replicate<0>();                                  // zero REP // expected-error{{replicate REP value cannot be zero}}
  vector<int,8> r9 = v1.replicate<two + minus_two>();                    // zero REP // expected-error{{replicate REP value cannot be zero}}
  vector<int,8> r10 = v1.replicate<minus_two>();                         // negative REP // expected-error{{replicate REP value cannot be negative (-2)}}
  vector<int,8> r11 = v1.replicate<2,>();                                // expected expression // expected-error{{expected expression}}
  vector<int,8> r12 = v1.replicate<2,4>();                               // OK
  vector<int,8> r13 = v1.replicate<2,0>();                               // zero Width // expected-error{{replicate width value cannot be zero}}
  vector<int,8> r14 = v1.replicate<2,minus_two>();                       // negative Width // expected-error{{replicate width value cannot be negative (-2)}}
  vector<int,8> r15 = v1.replicate<2,4,>();                              // expected expression // expected-error{{expected expression}}
  vector<int,4> r16 = v1.replicate<2,2,2>();                             // OK
  vector<int,8> r17 = v2.replicate<4,0,2>();                             // OK
  vector<int,8> r18 = v1.replicate<4,2,0>();                             // zero Width // expected-error{{replicate width value cannot be zero}}
  vector<int,8> r19 = v1.replicate<4,-8,2>();                            // negative VS // expected-error{{replicate vertical stride value cannot be negative (-8)}}
  vector<int,8> r20 = v1.replicate<4,2,-6>();                            // negative Width // expected-error{{replicate width value cannot be negative (-6)}}
  vector<int,8> r21 = v1.replicate<4,2,4,>();                            // expected expression // expected-error{{expected expression}}
  vector<int,16> r22 = v1.replicate<4,0,4,0>();                          // OK
  vector<int,16> r23 = v1.replicate<4,2,0,2>();                          // zero width // expected-error{{replicate width value cannot be zero}}
  vector<int,16> r24 = v1.replicate<4,2,4,2,two>();                      // too many args // expected-error{{too many arguments: replicate expects at most 4 constant integer values}}
  vector<int,16> r25 = v1.replicate<4,2,4,2,2,3>();                      // too many args // expected-error{{too many arguments: replicate expects at most 4 constant integer values}}
  vector<int,16> r26 = v2.replicate<4,1+minus_two,4,2>();                // negative HS // expected-error{{replicate vertical stride value cannot be negative (-1)}}
  vector<int,16> r27 = v1.replicate<4,2,minus_two,2>();                  // negative Width // expected-error{{replicate width value cannot be negative (-2)}}
  vector<int,16> r28 = v1.replicate<4,2,4,minus_two-1>();                // negative VS // expected-error{{replicate horizontal stride value cannot be negative (-3)}}
  vector<int,8> r29 = v1.replicate<4>(two+1);                            // offset not applicable // expected-error{{replicate does not accept an offset when only the REP argument is specified}}
  vector<int,8> r30 = v1.replicate<4,2>(2);                              // OK
  vector<int,8> r31 = v1.template replicate<4,2>(i);                     // OK
  vector<int,8> r32 = v2.replicate<2,2>(2,1-two);                        // too many offsets // expected-error{{too many offsets: replicate of a vector expects at most 1 integer offset}}
  vector<int,8> r33 = v1.replicate<4,2>(minus_two);                      // negative offset // expected-error{{replicate offset cannot be negative (-2)}}
  vector<int,8> r34 = v1.replicate<i>();                                 // REP not integer const // expected-error{{replicate value must be a constant integer expression}}
  vector<int,8> r35 = v1.replicate<2,i>();                               // Width not integer const // expected-error{{replicate value must be a constant integer expression}}
  vector<int,8> r36 = v1.replicate<2,i,4>();                             // VS not integer const // expected-error{{replicate value must be a constant integer expression}}
  vector<int,8> r37 = v1.replicate<2,2,i>();                             // Width not integer const // expected-error{{replicate value must be a constant integer expression}}
  vector<int,8> r38 = v1.replicate<2,2,i+two,2>();                       // Width not integer const // expected-error{{replicate value must be a constant integer expression}}
  vector<int,8> r39 = v1.replicate<2,2,4,(5*i)>();                       // HS not integer const // expected-error{{replicate value must be a constant integer expression}}
  vector<int,8> r40 = m.replicate;                                       // expected '<' // expected-error{{expected '<'}}
  vector<int,8> r41 = m.replicate<;                                      // expected expression // expected-error{{expected expression}}
  vector<int,8> r42 = m.replicate<2;                                     // expected '>' // expected-error{{expected '>'}}
  vector<int,8> r43 = m.replicate<2>;                                    // expected '(' // expected-error{{expected '('}}
  vector<int,8> r44 = m.replicate<2>(;                                   // expected expression // expected-error{{expected expression}}
  vector<int,48> r45 = m.template replicate<2>();                        // OK
  vector<int,48> r46 = m.replicate<two>();                               // OK
  vector<int,8> r47 = m.replicate<0>();                                  // zero REP // expected-error{{replicate REP value cannot be zero}}
  vector<int,8> r48 = m.replicate<two + minus_two>();                    // zero REP // expected-error{{replicate REP value cannot be zero}}
  vector<int,8> r59 = m.replicate<minus_two>();                          // negative REP // expected-error{{replicate REP value cannot be negative (-2)}}
  vector<int,8> r60 = m.replicate<2,>();                                 // expected expression // expected-error{{expected expression}}
  vector<int,8> r61 = m.replicate<2,4>();                                // OK
  vector<int,8> r62 = m.replicate<2,0>();                                // zero Width // expected-error{{replicate width value cannot be zero}}
  vector<int,8> r63 = m.replicate<2,minus_two>();                        // negative Width // expected-error{{replicate width value cannot be negative (-2)}}
  vector<int,8> r64 = m.replicate<2,4,>();                               // expected expression // expected-error{{expected expression}}
  vector<int,8> r65 = m.replicate<4,2,2>();                              // OK
  vector<int,8> r66 = m.replicate<4,0,2>();                              // OK
  vector<int,8> r67 = m.replicate<4,2,0>();                              // zero Width // expected-error{{replicate width value cannot be zero}}
  vector<int,8> r68 = m.replicate<4,-8,2>();                             // negative VS // expected-error{{replicate vertical stride value cannot be negative (-8)}}
  vector<int,8> r69 = m.replicate<4,2,-6>();                             // negative Width // expected-error{{replicate width value cannot be negative (-6)}}
  vector<int,8> r70 = m.replicate<4,2,4,>();                             // expected expression // expected-error{{expected expression}}
  vector<int,16> r71 = m.replicate<4,0,4,0>();                           // OK
  vector<int,16> r72 = m.replicate<4,2,0,2>();                           // zero width // expected-error{{replicate width value cannot be zero}}
  vector<int,16> r73 = m.replicate<4,2,4,2,two>();                       // too many args // expected-error{{too many arguments: replicate expects at most 4 constant integer values}}
  vector<int,16> r74 = m.replicate<4,2,4,2,2,3>();                       // too many args // expected-error{{too many arguments: replicate expects at most 4 constant integer values}}
  vector<int,16> r75 = m.replicate<4,1+minus_two,4,2>();                 // negative HS // expected-error{{replicate vertical stride value cannot be negative (-1)}}
  vector<int,16> r76 = m.replicate<4,2,minus_two,2>();                   // negative Width // expected-error{{replicate width value cannot be negative (-2)}}
  vector<int,16> r77 = m.replicate<4,2,4,minus_two-1>();                 // negative VS // expected-error{{replicate horizontal stride value cannot be negative (-3)}}
  vector<int,8> r78 = m.replicate<4>(two+1);                             // offset not applicable // expected-error{{replicate does not accept an offset when only the REP argument is specified}}
  vector<int,8> r79 = m.replicate<4,2>(2);                               // OK
  vector<int,8> r80 = m.template replicate<4,2>(i,two-i);                // OK
  vector<int,8> r81 = m.replicate<2,2>(2,1-two,5*i,0);                   // too many offsets // expected-error{{too many offsets: replicate of a matrix expects at most 2 integer offsets}}
  vector<int,8> r82 = m.replicate<4,2>(minus_two);                       // negative offset // expected-error{{replicate offset cannot be negative (-2)}}
  vector<int,8> r83 = m.replicate<4,2>(0,minus_two);                     // negative offset // expected-error{{replicate offset cannot be negative (-2)}}
  vector<int,8> r84 = m.replicate<4.0f>();                               // REP not integer const // expected-error{{replicate value must be a constant integer expression}}
  vector<int,8> r85 = m.replicate<2,4.0f>();                             // Width not integer const // expected-error{{replicate value must be a constant integer expression}}
  vector<int,8> r86 = m.replicate<4,2>(1.0f);                            // offset not integer const // expected-error{{replicate offset must be an integer expression}}
  vector<int,8> r87 = m.replicate<4,2>(0,0.0f);                          // offset not integer const // expected-error{{replicate offset must be an integer expression}}

  vector<int,24> r88 = v1.replicate<2>().template replicate<3>();        // OK
  vector<short,8> r89 = m.replicate<4,2>(5).replicate<2,2,4,1>(4);       // OK // expected-warning{{replicate out of bounds - size of source vector exceeded}}

  int r90 = s1.replicate;                                                // no member replicate in s1  // expected-error{{no member named 'replicate' in 'S1'}}
  int r91 = s1.replicate();                                              // no member replicate in s1  // expected-error{{no member named 'replicate' in 'S1'}}
  int r92 = s1.template replicate;                                       // no member replicate in s1  // expected-error{{no member named 'replicate' in 'S1'}}
  int r93 = s2.replicate;                                                // OK
  int r94 = s2.replicate<4;                                              // OK
  int r95 = s2.replicate();                                              // replicate not a function // expected-error{{called object type 'int' is not a function or function pointer}}
  int r96 = s2.template replicate();                                     // replicate not a template // expected-error{{'replicate' following the 'template' keyword does not refer to a template}}

  vector<int,96> r100 = m.replicate<4,24>();                             // OK
  vector<int,96> r101 = m.replicate<4,24>(1);                            // out of bounds // expected-warning{{replicate out of bounds - size of source matrix exceeded}}
  vector<int,96> r102 = m.replicate<4,24>(0,1);                          // out of bounds // expected-warning{{replicate out of bounds - size of source matrix exceeded}}
  vector<int,24> r103 = m.replicate<1,24>(1,0);                          // out of bounds // expected-warning{{replicate out of bounds - size of source matrix exceeded}}
  vector<int,24> r104 = m.replicate<1,24>(0,1);                          // out of bounds // expected-warning{{replicate out of bounds - size of source matrix exceeded}}
  vector<int,6>  r105 = m.replicate<1,6>(3,0);                           // OK
  vector<int,6>  r106 = m.replicate<1,6>(3,1);                           // out of bounds // expected-warning{{replicate out of bounds - size of source matrix exceeded}}
  vector<int,24> r107 = m.replicate<4,5,6>(0,1);                         // OK
  vector<int,24> r108 = m.replicate<4,7,6>();                            // out of bounds // expected-warning{{replicate out of bounds - size of source matrix exceeded}}
  vector<int,24> r109 = m.replicate<4,5,6>(0,minus_two);                 // negative offset // expected-error{{replicate offset cannot be negative (-2)}}

  vector<int,24> r110 = v1.replicate<6,4>();                             // OK
  vector<int,4>  r111 = v1.replicate<1,4>(1);                            // out of bounds // expected-warning{{replicate out of bounds - size of source vector exceeded}}
  vector<int,4>  r112 = v1.replicate<2,2>(2);                            // OK
  vector<int,4>  r113 = v1.replicate<2,2,2>(2);                          // out of bounds // expected-warning{{replicate out of bounds - size of source vector exceeded}}
  vector<int,4>  r114 = v1.replicate<2,2>(-3);                           // negative offset // expected-error{{replicate offset cannot be negative (-3)}}

  vector<int,16> r120 = v1.replicate<4,1,4,0>();                         // OK
  vector<int,16> r121 = v1.replicate<4,1,4,0>(1);                        // out of bounds // expected-warning{{replicate out of bounds - size of source vector exceeded}}
  vector<int,16> r123 = v1.replicate<4,0,4,1>(0);                        // OK
  vector<int,16> r124 = v1.replicate<4,0,4,1>(1);                        // out of bounds // expected-warning{{replicate out of bounds - size of source vector exceeded}}
  vector<int,16> r125 = v1.replicate<4,1,4,1>(0);                        // out of bounds // expected-warning{{replicate out of bounds - size of source vector exceeded}}

  v1.replicate<16>();                                                    // expression result unused // fixme-expected-warning{{expression result unused}}
  m.replicate<2,2>();                                                    // expression result unused // fixme-expected-warning{{expression result unused}}
  v2.replicate<2,4,3,2>() = 9;                                           // expression not assignable // expected-error{{expression is not assignable}}
  m.replicate<2>() = 1;                                                  // expression not assignable // expected-error{{expression is not assignable}}
}
