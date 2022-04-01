/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -Xclang -disable-llvm-passes -g -S -emit-llvm -march=SKL -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

const int init[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

extern "C" _GENX_MAIN_ void kernel(SurfaceIndex status_buf [[type("buffer_t")]]) {
  matrix<int, 4, 4> x = init;
  matrix<int, 4, 4> y = init;
  // CHECK: [[X_ALLOCA:%[A-z0-9.]*]] = alloca <16 x i32>, align 64
  // CHECK: [[Y_ALLOCA:%[A-z0-9.]*]] = alloca <16 x i32>, align 64

  // CHECK-DAG: call void @llvm.dbg.declare(metadata <16 x i32>* [[X_ALLOCA]], metadata [[X_META:![0-9]*]]
  // CHECK-DAG: call void @llvm.dbg.declare(metadata <16 x i32>* [[Y_ALLOCA]], metadata [[Y_META:![0-9]*]]

  // Reference with variable offsets - can't create dbg.data immediately
  matrix_ref<int, 2, 2> value_undef = x.template select<2, 2, 2, 2>(cm_local_id(1), cm_local_id(0));
  // No dbg.value declaration - can't get unmaterialized reference
  matrix_ref<int, 2, 2> value_undef_ref = value_undef;

  // Referenve with constant offsets - emit sequence of dbg.value
  matrix_ref<int, 2, 2> value_def = x.template select<2, 2, 2, 2>(0, 0);
  // CHECK: call void @llvm.dbg.value(metadata <16 x i32>* [[X_ALLOCA]], metadata [[VAL_DEF_META:![0-9]*]], metadata !DIExpression(DW_OP_constu, 0, DW_OP_plus, DW_OP_deref, DW_OP_LLVM_fragment, 0, 32))
  // CHECK: call void @llvm.dbg.value(metadata <16 x i32>* [[X_ALLOCA]], metadata [[VAL_DEF_META]], metadata !DIExpression(DW_OP_constu, 64, DW_OP_plus, DW_OP_deref, DW_OP_LLVM_fragment, 32, 32))
  // CHECK: call void @llvm.dbg.value(metadata <16 x i32>* [[X_ALLOCA]], metadata [[VAL_DEF_META]], metadata !DIExpression(DW_OP_constu, 256, DW_OP_plus, DW_OP_deref, DW_OP_LLVM_fragment, 64, 32))
  // CHECK: call void @llvm.dbg.value(metadata <16 x i32>* [[X_ALLOCA]], metadata [[VAL_DEF_META]], metadata !DIExpression(DW_OP_constu, 320, DW_OP_plus, DW_OP_deref, DW_OP_LLVM_fragment, 96, 32))

  // Change data in reference-by value - need undef materialized value_undef
  x(2, 2) = 1234;
  // CHECK: call void @llvm.dbg.value(metadata <16 x i32> undef, metadata [[VAL_UNDEF_META:![0-9]*]]
  // CHECK: call void @llvm.dbg.value(metadata <16 x i32> undef, metadata [[VAL_UNDEF_REF_META:![0-9]*]]

  // Here value is materialized
  // CHECK: [[RDREG:%[A-z0-9.]*]] = call <4 x i32> @llvm.genx.rdregioni.v4i32.v8i32.i16(<8 x i32>
  // CHECK-DAG: call void @llvm.dbg.value(metadata <4 x i32> [[RDREG]], metadata [[VAL_UNDEF_REF_META]]
  write(status_buf, 0, value_undef_ref.template format<int>());
}

// CHECK-DAG: [[X_META]] = !DILocalVariable(name: "x"
// CHECK-DAG: [[Y_META]] = !DILocalVariable(name: "y"
// CHECK-DAG: [[VAL_DEF_META]] = !DILocalVariable(name: "value_def"
// CHECK-DAG: [[VAL_UNDEF_META]] = !DILocalVariable(name: "value_undef"
// CHECK-DAG: [[VAL_UNDEF_REF_META]] = !DILocalVariable(name: "value_undef_ref"
