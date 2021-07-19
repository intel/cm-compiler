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
  //CHECK:      [[VECTOR:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[VECTOR]], align 32
  vector<uint, 8> off(init_0_7);
}

//CHECK-LABEL: @test2(
extern "C" void test2()
{
  //CHECK:      [[VECTOR:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[VECTOR]], align 32
  const vector<uint, 8> off(init_0_7);
}

//CHECK-LABEL: @test3(
extern "C" void test3()
{
  //CHECK:      [[VECTOR:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[VECTOR]], align 32
  vector<uint, 8> off(init_0_7_c);
}

//CHECK-LABEL: @test4(
extern "C" void test4()
{
  //CHECK:      [[VECTOR:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[VECTOR]], align 32
  const vector<uint, 8> off(init_0_7_c);
}

//CHECK-LABEL: @test5(
extern "C" void test5()
{
  //CHECK:      [[VECTOR:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[VECTOR]], align 32
  const vector<uint, 8> off = {0,1,2,3,4,5,6,7};
}

//CHECK-LABEL: @test6(
extern "C" void test6()
{
  //CHECK:      [[VECTOR:%[^ ]+]] = alloca <8 x i32>, align 32
  //CHECK-NEXT: store <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>, <8 x i32>* [[VECTOR]], align 32
  vector<uint, 8> off = {0,1,2,3,4,5,6,7};
}

extern "C" _GENX_MAIN_ void test()
{
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
}
