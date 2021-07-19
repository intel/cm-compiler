/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This test checks that initialized vector has constant restrictions

// RUN: %cmc -emit-llvm -march=SKL -Xclang -verify-ignore-unexpected -Xclang -verify -emit-llvm -ferror-limit=100 -- %s

#include <cm/cm.h>

const int init_0_7[] = {0,1,2,3,4,5,6,7};

extern "C" _GENX_MAIN_ void test()
{
  const vector<uint, 8> vec (init_0_7);
  vector<uint, 8>       vec2(init_0_7);
  vec  = 1; //expected-error{{cannot assign to variable 'vec' with const-qualified type 'vector<uint ,8>const'}}
  vec2 = 1;

  const matrix<uint, 4, 2> matr (init_0_7);
  matrix<uint, 4, 2>       matr2(init_0_7);
  matr  +=  matr.select<4, 1, 2, 1>(0, 0); //expected-error{{cannot assign to variable 'matr' with const-qualified type 'matrix<uint ,4,2>const'}}
  matr2 += matr2.select<4, 1, 2, 1>(0, 0);
}
