/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -ferror-limit=99 -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

#include <cm/cm.h>

matrix<int,4,6> m1, m2, m3;
vector<int,8> v1, v2, v3;
matrix<float,4,6> m4;
vector<float,8> v4;
matrix<int,6,4> m5;
vector<int,24> v5;
matrix<int,6,6> m6, m7;
vector<ushort,32> v6, v7;
vector<SurfaceIndex,10> v8, v9;
int mask1;
char mask2;
float mask3;
SurfaceIndex mask4;
vector<int,6> mask5;
vector<int,24> mask6;
matrix<char,4,6> mask7;
matrix<char,6,4> mask8;
matrix<char,4,4> mask9;
matrix<float,4,6> mask10;
matrix<SurfaceIndex,4,6> mask11;
struct S1 {
  int a;
} s1;
struct S2 {
  int merge;
} s2;

_GENX_MAIN_
void test() {
  m1.merge;                                // expected '(' // expected-error{{expected '('}}
  m1.merge(;                               // expected expression // expected-error{{expected expression}}
  m1.merge();                              // expected expression // expected-error{{expected expression}}
  m1.merge(m2;                             // expected ')' // expected-error{{expected ')'}}
  m1.merge(m2);                            // not enough args // expected-error{{too few arguments: merge() expects at least two arguments}}
  m1.merge(m1,mask1);                      // OK
  m1.template merge(m1,mask1);             // OK
  m1.merge(m2,m3,mask1);                   // OK
  m1.template merge(m2,m3,mask1);          // OK
  m1.merge(m1,m2,m3,mask1);                // too many args // expected-error{{too many arguments: merge() expects at most three arguments}}
  m1.merge(m2,mask7).merge(m3,mask1);      // void not a structure // expected-error{{member reference base type 'void' is not a structure or union}}
  m1.merge(m4,mask1);                      // OK - with type conversion
  m1.merge(m5,mask1);                      // OK - matrix different shape
  m1.merge(m6,mask1);                      // matrix size mismatch // expected-error{{cannot convert to type 'matrix<int,4,6>' from type 'matrix<int,6,6>'}}
  m1.merge(m6,m7,mask1);                   // matrix size mismatch // expected-error{{cannot convert to type 'matrix<int,4,6>' from type 'matrix<int,6,6>'}}
  m1.merge(m2,mask7);                      // OK
  m1.merge(m2,mask8);                      // OK
  m1.merge(m5,v5,mask6);                   // OK
  m1.merge(m2,mask4);                      // invalid mask type // expected-error{{invalid merge mask type 'SurfaceIndex' for type 'matrix<int,4,6>'}}
  m1.merge(m2,mask10);                     // invalid mask type // expected-error{{invalid merge mask element type 'float'}}
  m1.merge(m2,mask11);                     // invalid mask type // expected-error{{invalid merge mask element type 'SurfaceIndex'}}
  m1.merge(m5,v5,mask3);                   // invalid mask type // expected-error{{invalid merge mask type 'float' for type 'matrix<int,4,6>'}}
  m1.merge(m2,mask9);                      // mask size mismatch // expected-error{{invalid merge mask type 'matrix<char,4,4>' for type 'matrix<int,4,6>'}}
  m1.merge(v5,mask1);                      // OK
  m1.merge(v5,mask2);                      // warning mask insufficient bits // expected-warning{{merge mask has fewer bits (8) than the number of merge elements (24)}}
  m1.merge(m2,mask6);                      // OK - mask different shape

  v1.merge;                                // expected '(' // expected-error{{expected '('}}
  v1.merge(;                               // expected expression // expected-error{{expected expression}}
  v1.merge();                              // expected expression // expected-error{{expected expression}}
  v1.merge(v2;                             // expected ')' // expected-error{{expected ')'}}
  v1.merge(v2);                            // not enough args // expected-error{{too few arguments: merge() expects at least two arguments}}
  v1.merge(v1,mask1);                      // OK
  v1.template merge(v1,mask1);             // OK
  v1.merge(v2,v3,mask1);                   // OK
  v1.template merge(v2,v3,mask1);          // OK
  v1.merge(v1,v2,v3,mask1);                // too many args // expected-error{{too many arguments: merge() expects at most three arguments}}
  v1.merge(v2,mask2).merge(v3,mask1);      // void not a structure // expected-error{{member reference base type 'void' is not a structure or union}}
  v1.merge(v4,mask1);                      // OK - with type conversion
  v5.merge(m4,mask1);                      // OK - matrix different shape
  v1.merge(v6,mask1);                      // matrix size mismatch // expected-error{{cannot convert to type 'vector<int,8>' from type 'vector<unsigned short,32>'}}
  v1.merge(m7,v6,mask1);                   // matrix size mismatch // expected-error{{cannot convert to type 'vector<int,8>' from type 'matrix<int,6,6>'}}
  v5.merge(m5,mask7);                      // OK
  v5.merge(m5,mask8);                      // OK
  v5.merge(m5,v5,mask6);                   // OK
  v1.merge(v2,v3,mask3);                   // invalid mask type float // expected-error{{invalid merge mask type 'float' for type 'vector<int,8>'}}
  v1.merge(v2,mask9);                      // mask size mismatch // expected-error{{invalid merge mask type 'matrix<char,4,4>' for type 'vector<int,8>'}}
  v5.merge(m5,mask1);                      // OK
  v6.merge(v7,mask2);                      // warning mask insufficient bits // expected-warning{{merge mask has fewer bits (8) than the number of merge elements (32)}}
  v5.merge(m1,mask7);                      // OK - mask different shape
  v8.merge(v9,mask1);                      // invalid element type for merge // expected-error{{invalid element type 'SurfaceIndex'}}

  s1.merge;                                // no member merge in s1 // expected-error{{no member named 'merge' in 'S1'}}
  s1.merge();                              // no member merge in s1 // expected-error{{no member named 'merge' in 'S1'}}
  s1.template merge;                       // no member merge in s1 // expected-error{{no member named 'merge' in 'S1'}}
  s2.merge = 7;                            // OK
  s2.merge();                              // merge not a function // expected-error{{called object type 'int' is not a function or function pointer}}
  s2.template merge();                     // merge not a template // expected-error{{'merge' following the 'template' keyword does not refer to a template}}

  m1.merge(m2,mask1) = 1;                  // not assignable // expected-error{{expression is not assignable}}
  v1.merge(v2,mask1) = 9;                  // not assignable // expected-error{{expression is not assignable}}
}
