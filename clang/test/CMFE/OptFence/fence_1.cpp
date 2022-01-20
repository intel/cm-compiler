/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

// CHECK-NOT: __cm_optfence_begin__
// CHECK-NOT: __cm_optfence_end__
// CHECK-NOT: constant [{{[^ ]*}} x i8] c"Fence must be used at function scope!\00"

_GENX_MAIN_ void test_uint_SZ7(SurfaceIndex Buffer) {
  // CHECK-LABEL: test_uint_SZ7

  vector<unsigned, 4> tBegin = cm_rdtsc();
  // CHECK: store <4 x i32> %{{[^ ,]*}}, <4 x i32>* [[VEC_BEGIN_NAME:%[^ ,]*]],
  CM_OPTIMIZATIONS_FENCE;
  // CHECK:  call void [[FUNC_NAME_0:@[^ ]*__cm_optfence__]](
  // CHECK: ret void

  // CHECK: define internal void [[FUNC_NAME_0]](
  unsigned num = 66;
  // CHECK: store i32 66, i32* [[VAR_NAME_0:%[^ ]*]], align
  {
    CM_OPTIMIZATIONS_FENCE;
    // CHECK:  call void [[FUNC_NAME_1:@[^ ]*__cm_optfence__[^ ]*]](
    if (num == 1) {
      if (num == 2) {
        while (true)
          num += 1;
      } else {
        num = 2;
      }
    }
    num = 10;
  }
  vector<unsigned, 8> vec = num;

  CM_OPTIMIZATIONS_FENCE;
  // CHECK:  call void [[FUNC_NAME_2:@[^ ]*__cm_optfence__[^ ]*]](
  write(Buffer, 0, cm_rdtsc() - tBegin);
  write(Buffer, 64, vec);
}

// CHECK: define internal void [[FUNC_NAME_2]](
// CHECK: call void @llvm.genx.oword.st
// CHECK: call void @llvm.genx.oword.st
