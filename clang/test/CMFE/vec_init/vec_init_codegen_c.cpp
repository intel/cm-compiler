/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -march=SKL -emit-llvm -S -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

// Check that uninitialized vector elements default to 0

#include <cm/cm.h>

_GENX_ void test_func() {
  vector<unsigned short, 8> vec = {};
// CHECK: [[VECTOR:%[^ ]+]] = alloca <8 x i16>
// CHECK-NEXT: store <8 x i16> zeroinitializer, <8 x i16>* [[VECTOR]]
}
