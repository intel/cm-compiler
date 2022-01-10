/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

// CHECK-NOT: __cm_optfence_begin__
// CHECK-NOT: __cm_optfence_end__
// CHECK-NOT: constant [{{[^ ]*}} x i8] c"Fence must be used at function scope!\00"

#include <cm/cm.h>

_GENX_MAIN_ void test_uint_SZ7(SurfaceIndex Buffer) {
  // CHECK-LABEL: test_uint_SZ7
  unsigned num = 66;
  // CHECK: store i32 66, i32* [[VAR_NAME_0:%[^ ]*]], align
  {
    // CHECK:  call void [[FUNC_NAME_0:@[^.]*.__cm_optfence__]](i32* [[VAR_NAME_0]])
    CM_OPTIMIZATIONS_FENCE;
    if (num == 1) {
      if (num == 2) {
        while (true)
          num += 1;
      } else {
        num = 2;
      }
    }
    num = 10;
    // CHECK: load i32, i32* [[VAR_NAME_0]], align
  }
  vector<unsigned, 8> vec = num;
  write(Buffer, 0, vec);
  // CHECK: call void @llvm.genx.oword.st.v8i32
}

// CHECK: define internal void [[FUNC_NAME_0]](
// CHECK: store i32 10, i32*
