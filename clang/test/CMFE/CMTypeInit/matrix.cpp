/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -S -emit-llvm -march=SKL -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

int       init_0_7[]   = {0,1,2,3,4,5,6,7};
const int init_0_7_c[] = {0,1,2,3,4,5,6,7};

//CHECK-LABEL: @test1(
extern "C" void test1()
{
  matrix<uint, 4, 2> off(init_0_7);
  //CHECK:      [[MATRIX:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[MATRIX]], align 32
}

//CHECK-LABEL: @test2(
extern "C" void test2()
{
  const matrix<uint, 4, 2> off(init_0_7);
  //CHECK:      [[MATRIX:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[MATRIX]], align 32
}

//CHECK-LABEL: @test3(
extern "C" void test3()
{
  matrix<uint, 4, 2> off(init_0_7_c);
  //CHECK:      [[MATRIX:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[MATRIX]], align 32
}

//CHECK-LABEL: @test4(
extern "C" void test4()
{
  const matrix<uint, 4, 2> off(init_0_7_c);
  //CHECK:      [[MATRIX:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[MATRIX]], align 32
}

extern "C" _GENX_MAIN_ void test()
{
  test1();
  test2();
  test3();
  test4();
}
