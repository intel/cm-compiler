/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -march=SKL -emit-llvm -S -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

_GENX_ void test_func() {
  vector<unsigned short, 8> vec = {1, 2, 3, 5, 7, 11, 13, 17};
// CHECK: [[VECTOR:%[^ ]+]] = alloca <8 x i16>
// CHECK-NEXT: store <8 x i16> <i16 1, i16 2, i16 3, i16 5, i16 7, i16 11, i16 13, i16 17>, <8 x i16>* [[VECTOR]]
}
