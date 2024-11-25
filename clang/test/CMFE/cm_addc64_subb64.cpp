/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

// CHECK: [[RES_V_STRUCT64:%[^ ]+]] = type { <16 x i64>, <16 x i64> }
// CHECK: [[RES_S_STRUCT64:%[^ ]+]] = type { i64, i64 }

// CHECK-LABEL: test64VectorAddc
_GENX_MAIN_ void test64VectorAddc() {

  vector<unsigned long long, 16> a_v16;
  vector<unsigned long long, 16> b_v16;
  vector_ref<unsigned long long, 16> a_vr16 = a_v16;
  vector_ref<unsigned long long, 16> b_vr16 = b_v16;
  vector<unsigned long long, 16> carry_v16;
  // CHECK-DAG: [[SRET_ADDR_V_VR_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_VR_V_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_VR_VR_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_V_V_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128

  vector<unsigned long long, 16> sum_v16_v16 = cm_addc(a_v16, b_v16, carry_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_V_V_ADDC]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_V_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_V_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_V_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_V_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_V_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUM_ADDR_V_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_V_V]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sum_vr16_vr16 = cm_addc(a_vr16, b_vr16, carry_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_VR_VR_ADDC]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_VR_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_VR_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_VR_VR]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_VR_VR]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_VR_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_VR_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUM_ADDR_VR_VR]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_VR_VR]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sum_vr16_v16 = cm_addc(a_vr16, b_v16, carry_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_VR_V_ADDC]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_V_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_VR_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_VR_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_VR_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_V_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_VR_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUM_ADDR_VR_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_VR_V]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sum_v16_vr16 = cm_addc(a_v16, b_vr16, carry_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_V_VR_ADDC]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_VR_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_V_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_V_VR]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_V_VR]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_VR_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUM_ADDR_V_VR]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_V_VR]], <16 x i64>* %{{.*}}, align 128
}

// CHECK-LABEL: test64VectorSub
_GENX_MAIN_ void test64VectorSub() {

  vector<unsigned long long, 16> a_v16;
  vector<unsigned long long, 16> b_v16;
  vector_ref<unsigned long long, 16> a_vr16 = a_v16;
  vector_ref<unsigned long long, 16> b_vr16 = b_v16;
  vector<unsigned long long, 16> borrow_v16;
  // CHECK-DAG: [[SRET_ADDR_V_VR_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_VR_V_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_VR_VR_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_V_V_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128

  vector<unsigned long long, 16> sub_v16_v16 = cm_subb(a_v16, b_v16, borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_V_V_SUBB]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_V_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_V_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_V_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_V_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_V_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUB_ADDR_V_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_V_V]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sub_vr16_vr16 = cm_subb(a_vr16, b_vr16, borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_VR_VR_SUBB]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_VR_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_VR_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_VR_VR]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_VR_VR]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_VR_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_VR_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUB_ADDR_VR_VR]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_VR_VR]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sub_vr16_v16 = cm_subb(a_vr16, b_v16, borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_VR_V_SUBB]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_V_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_VR_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_VR_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_VR_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_V_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_VR_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUB_ADDR_VR_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_VR_V]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sub_v16_vr16 = cm_subb(a_v16, b_vr16, borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_V_VR_SUBB]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_VR_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_V_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_V_VR]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_V_VR]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_VR_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUB_ADDR_V_VR]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_V_VR]], <16 x i64>* %{{.*}}, align 128
}

// CHECK-LABEL: test64MatrixAddc
_GENX_MAIN_ void test64MatrixAddc() {

  matrix<unsigned long long, 4, 4> a_m4x4;
  matrix<unsigned long long, 4, 4> b_m4x4;
  matrix_ref<unsigned long long, 4, 4> a_mr4x4 = a_m4x4;
  matrix_ref<unsigned long long, 4, 4> b_mr4x4 = b_m4x4;
  matrix<unsigned long long, 4, 4> carry_m4x4;
  // CHECK-DAG: [[SRET_ADDR_V_VR_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_VR_V_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_VR_VR_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_V_V_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128

  matrix<unsigned long long, 4, 4> sum_m4x4_m4x4 = cm_addc(a_m4x4, b_m4x4, carry_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_V_V_ADDC]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_V_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_V_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_V_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_V_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_V_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUM_ADDR_V_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_V_V]], <16 x i64>* %{{.*}}, align 128

  matrix<unsigned long long, 4, 4> sum_mr4x4_mr4x4 = cm_addc(a_mr4x4, b_mr4x4, carry_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_VR_VR_ADDC]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_VR_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_VR_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_VR_VR]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_VR_VR]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_VR_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_VR_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUM_ADDR_VR_VR]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_VR_VR]], <16 x i64>* %{{.*}}, align 128

  matrix<unsigned long long, 4, 4> sum_mr4x4_m4x4 = cm_addc(a_mr4x4, b_m4x4, carry_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_VR_V_ADDC]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_V_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_VR_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_VR_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_VR_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_V_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_VR_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUM_ADDR_VR_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_VR_V]], <16 x i64>* %{{.*}}, align 128

  matrix<unsigned long long, 4, 4> sum_m4x4_mr4x4 = cm_addc(a_m4x4, b_mr4x4, carry_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_V_VR_ADDC]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_VR_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_V_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_V_VR]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_V_VR]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_VR_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUM_ADDR_V_VR]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_V_VR]], <16 x i64>* %{{.*}}, align 128
}

// CHECK-LABEL: test64MatrixSubb
_GENX_MAIN_ void test64MatrixSubb() {

  matrix<unsigned long long, 4, 4> a_m4x4;
  matrix<unsigned long long, 4, 4> b_m4x4;
  matrix_ref<unsigned long long, 4, 4> a_mr4x4 = a_m4x4;
  matrix_ref<unsigned long long, 4, 4> b_mr4x4 = b_m4x4;
  matrix<unsigned long long, 4, 4> borrow_m4x4;
  // CHECK-DAG: [[SRET_ADDR_V_VR_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_VR_V_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_VR_VR_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_V_V_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128

  matrix<unsigned long long, 4, 4> sub_m4x4_m4x4 = cm_subb(a_m4x4, b_m4x4, borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_V_V_SUBB]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_V_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_V_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_V_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_V_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_V_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUB_ADDR_V_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_V_V]], <16 x i64>* %{{.*}}, align 128

  matrix<unsigned long long, 4, 4> sub_mr4x4_mr4x4 = cm_subb(a_mr4x4, b_mr4x4, borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_VR_VR_SUBB]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_VR_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_VR_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_VR_VR]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_VR_VR]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_VR_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_VR_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUB_ADDR_VR_VR]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_VR_VR]], <16 x i64>* %{{.*}}, align 128

  matrix<unsigned long long, 4, 4> sub_mr4x4_m4x4 = cm_subb(a_mr4x4, b_m4x4, borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_VR_V_SUBB]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_V_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_VR_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_VR_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_VR_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_VR_V_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_VR_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUB_ADDR_VR_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_VR_V]], <16 x i64>* %{{.*}}, align 128

  matrix<unsigned long long, 4, 4> sub_m4x4_mr4x4 = cm_subb(a_m4x4, b_mr4x4, borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_V_VR_SUBB]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_VR_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_V_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_V_VR]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_V_VR]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_V_VR_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_VR:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_SUB_ADDR_V_VR]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_V_VR]], <16 x i64>* %{{.*}}, align 128
}

// CHECK-LABEL: test64Scalar
_GENX_MAIN_ void test64Scalar() {
  unsigned long long a;
  unsigned long long b;
  unsigned long long carry_borrow;
  // CHECK-DAG: [[SRET_ADDR_S_SUBB:%[^ ]+]] = alloca [[RES_S_STRUCT64]], align 8
  // CHECK-DAG: [[SRET_ADDR_S_ADDC:%[^ ]+]] = alloca [[RES_S_STRUCT64]], align 8

  unsigned long long sum_s_s = cm_addc(a, b, carry_borrow);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_S_STRUCT64]]* sret align 8 [[SRET_ADDR_S_ADDC]], i64 %{{.*}}, i64 %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_S:%[^ ]+]] = getelementptr inbounds [[RES_S_STRUCT64]], [[RES_S_STRUCT64]]* [[SRET_ADDR_S_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_S:%[^ ]+]] = load i64, i64* [[RET_CARRY_ADDR_S]], align 8
  // CHECK: store i64 [[RET_CARRY_S]], i64* {{.*}}, align 8
  // CHECK: [[RET_SUM_ADDR_S:%[^ ]+]] = getelementptr inbounds [[RES_S_STRUCT64]], [[RES_S_STRUCT64]]* [[SRET_ADDR_S_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_S:%[^ ]+]] = load i64, i64* [[RET_SUM_ADDR_S]], align 8
  // CHECK: store i64 [[RET_SUM_S]], i64* %{{.*}}, align 8

  unsigned long long sub_s_s = cm_subb(a, b, carry_borrow);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_S_STRUCT64]]* sret align 8 [[SRET_ADDR_S_SUBB]], i64 %{{.*}}, i64 %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_S:%[^ ]+]] = getelementptr inbounds [[RES_S_STRUCT64]], [[RES_S_STRUCT64]]* [[SRET_ADDR_S_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_S:%[^ ]+]] = load i64, i64* [[RET_BORROW_ADDR_S]], align 8
  // CHECK: store i64 [[RET_BORROW_S]], i64* {{.*}}, align 8
  // CHECK: [[RET_SUB_ADDR_S:%[^ ]+]] = getelementptr inbounds [[RES_S_STRUCT64]], [[RES_S_STRUCT64]]* [[SRET_ADDR_S_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_S:%[^ ]+]] = load i64, i64* [[RET_SUB_ADDR_S]], align 8
  // CHECK: store i64 [[RET_SUB_S]], i64* %{{.*}}, align 8
}

// CHECK-LABEL: test64MixedVector
_GENX_MAIN_ void test64MixedVector() {
  vector<unsigned long long, 16> av16;
  vector<unsigned long long, 16> bv16;
  unsigned long long as;
  unsigned long long bs;
  vector<unsigned long long, 16> carry_borrow_v16;

  // CHECK-DAG: [[SRET_ADDR_SUB_S_V:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_SUM_S_V:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_SUB_V_S:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_SUM_V_S:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128

  vector<unsigned long long, 16> sum_v_s = cm_addc(av16, bs, carry_borrow_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUM_V_S]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_V_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_V_S]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_V_S:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_SUM_V_S]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_SUM_V_S]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_V_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_V_S]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_S:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUM_V_S]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_V_S]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sub_v_s = cm_subb(av16, bs, carry_borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUB_V_S]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_V_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_V_S]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_V_S:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_SUB_V_S]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_SUB_V_S]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_V_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_V_S]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_S:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUB_V_S]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_V_S]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sum_s_v = cm_addc(as, bv16, carry_borrow_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUM_S_V]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_S_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_S_V]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_S_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_SUM_S_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_SUM_S_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_S_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_S_V]], i32 0, i32 0
  // CHECK: [[RET_SUM_S_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUM_S_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_S_V]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sub_s_v = cm_subb(as, bv16, carry_borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUB_S_V]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_S_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_S_V]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_S_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_SUB_S_V]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_SUB_S_V]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_S_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_S_V]], i32 0, i32 0
  // CHECK: [[RET_SUB_S_V:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUB_S_V]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_S_V]], <16 x i64>* %{{.*}}, align 128
}

// CHECK-LABEL: test64MixedMatrix
_GENX_MAIN_ void test64MixedMatrix() {
  matrix<unsigned long long, 4, 4> am4x4;
  matrix<unsigned long long, 4, 4> bm4x4;
  unsigned long long as;
  unsigned long long bs;
  matrix<unsigned long long, 4, 4> carry_borrow_m4x4;

  // CHECK-DAG: [[SRET_ADDR_SUB_S_M:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_SUM_S_M:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_SUB_M_S:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_SUM_M_S:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128

  matrix<unsigned long long, 4, 4> sum_m_s = cm_addc(am4x4, bs, carry_borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUM_M_S]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_M_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_M_S]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_M_S:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_SUM_M_S]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_SUM_M_S]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_M_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_M_S]], i32 0, i32 0
  // CHECK: [[RET_SUM_M_S:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUM_M_S]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_M_S]], <16 x i64>* %{{.*}}, align 128

  matrix<unsigned long long, 4, 4> sub_m_s = cm_subb(am4x4, bs, carry_borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUB_M_S]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_M_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_M_S]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_M_S:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_SUB_M_S]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_SUB_M_S]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_M_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_M_S]], i32 0, i32 0
  // CHECK: [[RET_SUB_M_S:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUB_M_S]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_M_S]], <16 x i64>* %{{.*}}, align 128

  matrix<unsigned long long, 4, 4> sum_s_m = cm_addc(as, bm4x4, carry_borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUM_S_M]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_S_M:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_S_M]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_S_M:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_SUM_S_M]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_SUM_S_M]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_S_M:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_S_M]], i32 0, i32 0
  // CHECK: [[RET_SUM_S_M:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUM_S_M]], align 128
  // CHECK: store <16 x i64> [[RET_SUM_S_M]], <16 x i64>* %{{.*}}, align 128

  matrix<unsigned long long, 4, 4> sub_s_m = cm_subb(as, bm4x4, carry_borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUB_S_M]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_S_M:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_S_M]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_S_M:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_SUB_S_M]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_SUB_S_M]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_S_M:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_S_M]], i32 0, i32 0
  // CHECK: [[RET_SUB_S_M:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUB_S_M]], align 128
  // CHECK: store <16 x i64> [[RET_SUB_S_M]], <16 x i64>* %{{.*}}, align 128
}

// CHECK-LABEL: test64Cast
_GENX_MAIN_ void test64Cast() {
  vector<unsigned long long, 16> av;
  vector<unsigned long long, 16> bv;
  vector<unsigned long long, 16> carry_borrow;

  // CHECK-DAG: [[SRET_ADDR_SUB_V_V_LONG:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_SUM_V_V_LONG:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_SUB_V_V_SHORT:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128
  // CHECK-DAG: [[SRET_ADDR_SUM_V_V_SHORT:%[^ ]+]] = alloca [[RES_V_STRUCT64]], align 128

  vector<short, 16> sum_v_v_short = cm_addc(av, bv, carry_borrow);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUM_V_V_SHORT]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_V_V_SHORT:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_V_V_SHORT]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_V_V_SHORT:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_SUM_V_V_SHORT]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_SUM_V_V_SHORT]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_V_V_SHORT:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_V_V_SHORT]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_V_SHORT:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUM_V_V_SHORT]], align 128
  // CHECK: [[CONV_RET_SUM8:%[^ ]+]] = trunc <16 x i64> [[RET_SUM_V_V_SHORT]] to <16 x i16>
  // CHECK: store <16 x i16> [[CONV_RET_SUM8]], <16 x i16>* %{{.*}}, align 32

  vector<short, 16> sub_v_v_short = cm_subb(av, bv, carry_borrow);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUB_V_V_SHORT]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_V_V_SHORT:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_V_V_SHORT]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_V_V_SHORT:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_SUB_V_V_SHORT]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_SUB_V_V_SHORT]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_V_V_SHORT:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_V_V_SHORT]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_V_SHORT:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUB_V_V_SHORT]], align 128
  // CHECK: [[CONV_RET_SUB8:%[^ ]+]] = trunc <16 x i64> [[RET_SUB_V_V_SHORT]] to <16 x i16>
  // CHECK: store <16 x i16> [[CONV_RET_SUB8]], <16 x i16>* %{{.*}}, align 32

  vector<unsigned, 16> sum_v_v_long = cm_addc(av, bv, carry_borrow);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUM_V_V_LONG]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_V_V_LONG:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_V_V_LONG]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_V_V_LONG:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_CARRY_ADDR_SUM_V_V_LONG]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_CARRY_SUM_V_V_LONG]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_V_V_LONG:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUM_V_V_LONG]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_V_LONG:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUM_V_V_LONG]], align 128
  // CHECK: [[CONV_RET_SUM9:%[^ ]+]] = trunc <16 x i64> [[RET_SUM_V_V_LONG]] to <16 x i32>
  // CHECK: store <16 x i32> [[CONV_RET_SUM9]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sub_v_v_long = cm_subb(av, bv, carry_borrow);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT64]]* sret align 128 [[SRET_ADDR_SUB_V_V_LONG]], <16 x i64> %{{.*}}, <16 x i64> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_V_V_LONG:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_V_V_LONG]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_V_V_LONG:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_BORROW_ADDR_SUB_V_V_LONG]], align 128
  // CHECK: call void @llvm.genx.vstore.v16i64.p0v16i64(<16 x i64> [[RET_BORROW_SUB_V_V_LONG]], <16 x i64>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_V_V_LONG:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT64]], [[RES_V_STRUCT64]]* [[SRET_ADDR_SUB_V_V_LONG]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_V_LONG:%[^ ]+]] = load <16 x i64>, <16 x i64>* [[RET_ADDR_SUB_V_V_LONG]], align 128
  // CHECK: [[CONV_RET_SUB9:%[^ ]+]] = trunc <16 x i64> [[RET_SUB_V_V_LONG]] to <16 x i32>
  // CHECK: store <16 x i32> [[CONV_RET_SUB9]], <16 x i32>* %{{.*}}, align 64
}
