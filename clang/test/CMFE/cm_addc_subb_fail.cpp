/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -march=SKL -emit-llvm -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

#include <cm/cm.h>

_GENX_MAIN_ void test1() {
  vector<float, 16> a1;
  vector<float, 16> b1;
  vector<int, 16> carry1;
  vector<int, 16> sum1 = cm_addc(a1, b1, carry1); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<int, 16> borrow1;
  vector<int, 16> sub1 = cm_subb(a1, b1, borrow1); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<int, 16> a2;
  vector<int, 16> b2;
  vector<int, 16> carry2;
  vector<int, 16> sum2 = cm_addc(a2, b2, carry2); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<int, 16> borrow2;
  vector<int, 16> sub2 = cm_subb(a2, b2, borrow2); // expected-error{{no matching function for call to 'cm_subb'}}

  matrix<char, 4, 4> a3;
  vector<unsigned short, 16> b3;
  matrix<unsigned int, 2, 8> carry3;
  vector<int, 16> sum3 = cm_addc(a3, b3, carry3); // expected-error{{no matching function for call to 'cm_addc'}}
  matrix<unsigned int, 2, 8> borrow3;
  vector<int, 16> sub3 = cm_subb(a3, b3, borrow3); // expected-error{{no matching function for call to 'cm_subb'}}

  short a4;
  matrix<unsigned char, 4, 4> b4;
  vector<int, 16> carry4;
  vector<unsigned int, 16> sum4 = cm_addc(a4, b4, carry4); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<int, 16> borrow4;
  vector<unsigned int, 16> sub4 = cm_subb(a4, b4, borrow4); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<char, 16> a5;
  vector<char, 16> b5;
  vector<char, 16> carry5;
  vector<char, 16> sum5 = cm_addc(a5, b5, carry5); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<char, 16> borrow5;
  vector<char, 16> sub5 = cm_subb(a5, b5, borrow5); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<unsigned char, 16> a6;
  vector<unsigned char, 16> b6;
  vector<unsigned char, 16> carry6;
  vector<unsigned char, 16> sum6 = cm_addc(a6, b6, carry6); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<unsigned char, 16> borrow6;
  vector<unsigned char, 16> sub6 = cm_subb(a6, b6, borrow6); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<short, 16> a7;
  vector<short, 16> b7;
  vector<short, 16> carry7;
  vector<short, 16> sum7 = cm_addc(a7, b7, carry7); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<short, 16> borrow7;
  vector<short, 16> sub7 = cm_subb(a7, b7, borrow7); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<unsigned short, 16> a8;
  vector<unsigned short, 16> b8;
  vector<unsigned short, 16> carry8;
  vector<unsigned short, 16> sum8 = cm_addc(a8, b8, carry8); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<unsigned short, 16> borrow8;
  vector<unsigned short, 16> sub8 = cm_subb(a8, b8, borrow8); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<int, 16> a9;
  vector<int, 16> b9;
  vector<int, 16> carry9;
  vector<int, 16> sum9 = cm_addc(a9, b9, carry9); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<int, 16> borrow9;
  vector<int, 16> sub9 = cm_subb(a9, b9, borrow9); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<long, 16> a10;
  vector<long, 16> b10;
  vector<long, 16> carry10;
  vector<long, 16> sum10 = cm_addc(a10, b10, carry10); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<long, 16> borrow10;
  vector<long, 16> sub10 = cm_subb(a10, b10, borrow10); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<unsigned long, 16> a11;
  vector<unsigned long, 16> b11;
  vector<unsigned long, 16> carry11;
  vector<unsigned long, 16> sum11 = cm_addc(a11, b11, carry11); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<unsigned long, 16> borrow11;
  vector<unsigned long, 16> sub11 = cm_subb(a11, b11, borrow11); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<long long, 16> a12;
  vector<long long, 16> b12;
  vector<long long, 16> carry12;
  vector<long long, 16> sum12 = cm_addc(a12, b12, carry12); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<long long, 16> borrow12;
  vector<long long, 16> sub12 = cm_subb(a12, b12, borrow12); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<unsigned long long, 16> a13;
  vector<unsigned long long, 16> b13;
  vector<unsigned long long, 16> carry13;
  vector<unsigned long long, 16> sum13 = cm_addc(a13, b13, carry13); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<unsigned long long, 16> borrow13;
  vector<unsigned long long, 16> sub13 = cm_subb(a13, b13, borrow13); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<char, 16> a14;
  vector<unsigned short, 16> b14;
  vector<unsigned int, 16> carry14;
  vector<int, 16> sum14 = cm_addc(a14, b14, carry14); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<unsigned int, 16> borrow14;
  vector<int, 16> sub14 = cm_subb(a14, b14, borrow14); // expected-error{{no matching function for call to 'cm_subb'}}

  short a15;
  vector<unsigned char, 16> b15;
  vector<int, 16> carry15;
  vector<unsigned int, 16> sum15 = cm_addc(a15, b15, carry15); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<int, 16> borrow15;
  vector<unsigned int, 16> sub15 = cm_subb(a15, b15, borrow15); // expected-error{{no matching function for call to 'cm_subb'}}

  vector<unsigned, 16> a16;
  matrix<unsigned, 4, 4> b16;
  vector<unsigned, 16> carry16;
  vector<unsigned, 16> sum16 = cm_addc(a16, b16, carry16); // expected-error{{no matching function for call to 'cm_addc'}}
  vector<unsigned, 16> borrow16;
  vector<unsigned, 16> sub16 = cm_subb(a16, b16, borrow16); // expected-error{{no matching function for call to 'cm_subb'}}

  matrix<unsigned, 4, 4> a17;
  vector<unsigned, 16> b17;
  matrix<unsigned, 4, 4> carry17;
  matrix<unsigned, 4, 4> sum17 = cm_addc(a17, b17, carry17); // expected-error{{no matching function for call to 'cm_addc'}}
  matrix<unsigned, 4, 4> borrow17;
  matrix<unsigned, 4, 4> sub17 = cm_subb(a17, b17, borrow17); // expected-error{{no matching function for call to 'cm_subb'}}
}

