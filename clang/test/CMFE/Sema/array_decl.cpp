/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=skl -fcm-pointer -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

template<typename K>
struct A {
  int val;
};

_GENX_MAIN_ void kernelA()
{
  // CHECK: %{{[^ ]+}} = alloca [2 x i32]
  int a[2];
}

template<typename T>
void f()
{
  // CHECK: %{{[^ ]+}} = alloca [1 x %struct.A]
  A<T> a[1];
}

_GENX_MAIN_ void kernelB()
{
  f<float>();
}

