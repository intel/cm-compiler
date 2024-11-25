/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

// CHECK: [[RES_V_STRUCT32:%[^ ]+]] = type { <16 x i32>, <16 x i32> }
// CHECK: [[RES_S_STRUCT32:%[^ ]+]] = type { i32, i32 }

// CHECK-LABEL: test32VectorAddc
_GENX_MAIN_ void test32VectorAddc() {

  vector<unsigned, 16> a_v16;
  vector<unsigned, 16> b_v16;
  vector_ref<unsigned, 16> a_vr16 = a_v16;
  vector_ref<unsigned, 16> b_vr16 = b_v16;
  vector<unsigned, 16> carry_v16;
  // CHECK-DAG: [[SRET_ADDR_V_VR_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_VR_V_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_VR_VR_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_V_V_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64

  vector<unsigned, 16> sum_v16_v16 = cm_addc(a_v16, b_v16, carry_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_V_V_ADDC]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_V_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_V_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_V_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_V_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_V_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUM_ADDR_V_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_V_V]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sum_vr16_vr16 = cm_addc(a_vr16, b_vr16, carry_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_VR_VR_ADDC]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_VR_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_VR_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_VR_VR]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_VR_VR]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_VR_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_VR_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUM_ADDR_VR_VR]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_VR_VR]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sum_vr16_v16 = cm_addc(a_vr16, b_v16, carry_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_VR_V_ADDC]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_V_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_VR_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_VR_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_VR_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_V_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_VR_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUM_ADDR_VR_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_VR_V]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sum_v16_vr16 = cm_addc(a_v16, b_vr16, carry_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_V_VR_ADDC]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_VR_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_V_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_V_VR]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_V_VR]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_VR_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUM_ADDR_V_VR]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_V_VR]], <16 x i32>* %{{.*}}, align 64
}

// CHECK-LABEL: test32VectorSub
_GENX_MAIN_ void test32VectorSub() {

  vector<unsigned, 16> a_v16;
  vector<unsigned, 16> b_v16;
  vector_ref<unsigned, 16> a_vr16 = a_v16;
  vector_ref<unsigned, 16> b_vr16 = b_v16;
  vector<unsigned, 16> borrow_v16;
  // CHECK-DAG: [[SRET_ADDR_V_VR_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_VR_V_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_VR_VR_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_V_V_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64

  vector<unsigned, 16> sub_v16_v16 = cm_subb(a_v16, b_v16, borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_V_V_SUBB]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_V_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_V_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_V_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_V_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_V_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUB_ADDR_V_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_V_V]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sub_vr16_vr16 = cm_subb(a_vr16, b_vr16, borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_VR_VR_SUBB]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_VR_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_VR_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_VR_VR]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_VR_VR]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_VR_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_VR_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUB_ADDR_VR_VR]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_VR_VR]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sub_vr16_v16 = cm_subb(a_vr16, b_v16, borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_VR_V_SUBB]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_V_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_VR_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_VR_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_VR_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_V_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_VR_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUB_ADDR_VR_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_VR_V]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sub_v16_vr16 = cm_subb(a_v16, b_vr16, borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_V_VR_SUBB]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_VR_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_V_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_V_VR]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_V_VR]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_VR_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUB_ADDR_V_VR]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_V_VR]], <16 x i32>* %{{.*}}, align 64
}

// CHECK-LABEL: test32MatrixAddc
_GENX_MAIN_ void test32MatrixAddc() {

  matrix<unsigned, 4, 4> a_m4x4;
  matrix<unsigned, 4, 4> b_m4x4;
  matrix_ref<unsigned, 4, 4> a_mr4x4 = a_m4x4;
  matrix_ref<unsigned, 4, 4> b_mr4x4 = b_m4x4;
  matrix<unsigned, 4, 4> carry_m4x4;
  // CHECK-DAG: [[SRET_ADDR_V_VR_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_VR_V_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_VR_VR_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_V_V_ADDC:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64

  matrix<unsigned, 4, 4> sum_m4x4_m4x4 = cm_addc(a_m4x4, b_m4x4, carry_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_V_V_ADDC]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_V_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_V_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_V_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_V_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_V_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUM_ADDR_V_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_V_V]], <16 x i32>* %{{.*}}, align 64

  matrix<unsigned, 4, 4> sum_mr4x4_mr4x4 = cm_addc(a_mr4x4, b_mr4x4, carry_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_VR_VR_ADDC]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_VR_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_VR_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_VR_VR]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_VR_VR]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_VR_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_VR_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUM_ADDR_VR_VR]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_VR_VR]], <16 x i32>* %{{.*}}, align 64

  matrix<unsigned, 4, 4> sum_mr4x4_m4x4 = cm_addc(a_mr4x4, b_m4x4, carry_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_VR_V_ADDC]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_V_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_VR_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_VR_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_VR_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_V_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_VR_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUM_ADDR_VR_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_VR_V]], <16 x i32>* %{{.*}}, align 64

  matrix<unsigned, 4, 4> sum_m4x4_mr4x4 = cm_addc(a_m4x4, b_mr4x4, carry_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_V_VR_ADDC]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_VR_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_V_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_V_VR]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_V_VR]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUM_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_VR_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUM_ADDR_V_VR]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_V_VR]], <16 x i32>* %{{.*}}, align 64
}

// CHECK-LABEL: test32MatrixSubb
_GENX_MAIN_ void test32MatrixSubb() {

  matrix<unsigned, 4, 4> a_m4x4;
  matrix<unsigned, 4, 4> b_m4x4;
  matrix_ref<unsigned, 4, 4> a_mr4x4 = a_m4x4;
  matrix_ref<unsigned, 4, 4> b_mr4x4 = b_m4x4;
  matrix<unsigned, 4, 4> borrow_m4x4;
  // CHECK-DAG: [[SRET_ADDR_V_VR_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_VR_V_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_VR_VR_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_V_V_SUBB:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64

  matrix<unsigned, 4, 4> sub_m4x4_m4x4 = cm_subb(a_m4x4, b_m4x4, borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_V_V_SUBB]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_V_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_V_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_V_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_V_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_V_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_V_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUB_ADDR_V_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_V_V]], <16 x i32>* %{{.*}}, align 64

  matrix<unsigned, 4, 4> sub_mr4x4_mr4x4 = cm_subb(a_mr4x4, b_mr4x4, borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_VR_VR_SUBB]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_VR_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_VR_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_VR_VR]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_VR_VR]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_VR_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_VR_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_VR_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUB_ADDR_VR_VR]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_VR_VR]], <16 x i32>* %{{.*}}, align 64

  matrix<unsigned, 4, 4> sub_mr4x4_m4x4 = cm_subb(a_mr4x4, b_m4x4, borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_VR_V_SUBB]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_V_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_VR_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_VR_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_VR_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_VR_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_VR_V_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_VR_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUB_ADDR_VR_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_VR_V]], <16 x i32>* %{{.*}}, align 64

  matrix<unsigned, 4, 4> sub_m4x4_mr4x4 = cm_subb(a_m4x4, b_mr4x4, borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_V_VR_SUBB]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_VR_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_V_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_V_VR]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_V_VR]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_SUB_ADDR_V_VR:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_V_VR_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_VR:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_SUB_ADDR_V_VR]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_V_VR]], <16 x i32>* %{{.*}}, align 64
}

// CHECK-LABEL: test32Scalar
_GENX_MAIN_ void test32Scalar() {
  unsigned a;
  unsigned b;
  unsigned carry_borrow;
  // CHECK-DAG: [[SRET_ADDR_S_SUBB:%[^ ]+]] = alloca [[RES_S_STRUCT32]], align 4
  // CHECK-DAG: [[SRET_ADDR_S_ADDC:%[^ ]+]] = alloca [[RES_S_STRUCT32]], align 4

  unsigned sum_s_s = cm_addc(a, b, carry_borrow);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_S_STRUCT32]]* sret align 4 [[SRET_ADDR_S_ADDC]], i32 %{{.*}}, i32 %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_S:%[^ ]+]] = getelementptr inbounds [[RES_S_STRUCT32]], [[RES_S_STRUCT32]]* [[SRET_ADDR_S_ADDC]], i32 0, i32 1
  // CHECK: [[RET_CARRY_S:%[^ ]+]] = load i32, i32* [[RET_CARRY_ADDR_S]], align 4
  // CHECK: store i32 [[RET_CARRY_S]], i32* {{.*}}, align 4
  // CHECK: [[RET_SUM_ADDR_S:%[^ ]+]] = getelementptr inbounds [[RES_S_STRUCT32]], [[RES_S_STRUCT32]]* [[SRET_ADDR_S_ADDC]], i32 0, i32 0
  // CHECK: [[RET_SUM_S:%[^ ]+]] = load i32, i32* [[RET_SUM_ADDR_S]], align 4
  // CHECK: store i32 [[RET_SUM_S]], i32* %{{.*}}, align 4

  unsigned sub_s_s = cm_subb(a, b, carry_borrow);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_S_STRUCT32]]* sret align 4 [[SRET_ADDR_S_SUBB]], i32 %{{.*}}, i32 %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_S:%[^ ]+]] = getelementptr inbounds [[RES_S_STRUCT32]], [[RES_S_STRUCT32]]* [[SRET_ADDR_S_SUBB]], i32 0, i32 1
  // CHECK: [[RET_BORROW_S:%[^ ]+]] = load i32, i32* [[RET_BORROW_ADDR_S]], align 4
  // CHECK: store i32 [[RET_BORROW_S]], i32* {{.*}}, align 4
  // CHECK: [[RET_SUB_ADDR_S:%[^ ]+]] = getelementptr inbounds [[RES_S_STRUCT32]], [[RES_S_STRUCT32]]* [[SRET_ADDR_S_SUBB]], i32 0, i32 0
  // CHECK: [[RET_SUB_S:%[^ ]+]] = load i32, i32* [[RET_SUB_ADDR_S]], align 4
  // CHECK: store i32 [[RET_SUB_S]], i32* %{{.*}}, align 4
}

// CHECK-LABEL: test32MixedVector
_GENX_MAIN_ void test32MixedVector() {
  vector<unsigned, 16> av16;
  vector<unsigned, 16> bv16;
  unsigned as;
  unsigned bs;
  vector<unsigned, 16> carry_borrow_v16;

  // CHECK-DAG: [[SRET_ADDR_SUB_S_V:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_SUM_S_V:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_SUB_V_S:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_SUM_V_S:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64

  vector<unsigned, 16> sum_v_s = cm_addc(av16, bs, carry_borrow_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUM_V_S]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_V_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_V_S]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_V_S:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_SUM_V_S]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_SUM_V_S]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_V_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_V_S]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_S:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUM_V_S]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_V_S]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sub_v_s = cm_subb(av16, bs, carry_borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUB_V_S]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_V_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_V_S]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_V_S:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_SUB_V_S]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_SUB_V_S]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_V_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_V_S]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_S:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUB_V_S]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_V_S]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sum_s_v = cm_addc(as, bv16, carry_borrow_v16);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUM_S_V]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_S_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_S_V]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_S_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_SUM_S_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_SUM_S_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_S_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_S_V]], i32 0, i32 0
  // CHECK: [[RET_SUM_S_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUM_S_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_S_V]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sub_s_v = cm_subb(as, bv16, carry_borrow_v16);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUB_S_V]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_S_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_S_V]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_S_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_SUB_S_V]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_SUB_S_V]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_S_V:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_S_V]], i32 0, i32 0
  // CHECK: [[RET_SUB_S_V:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUB_S_V]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_S_V]], <16 x i32>* %{{.*}}, align 64

  vector<unsigned, 16> sum_v_const = cm_addc(av16, 1, carry_borrow_v16);
  vector<unsigned, 16> sub_v_const = cm_subb(av16, 1, carry_borrow_v16);
}

// CHECK-LABEL: test32MixedMatrix
_GENX_MAIN_ void test32MixedMatrix() {
  matrix<unsigned, 4, 4> am4x4;
  matrix<unsigned, 4, 4> bm4x4;
  unsigned as;
  unsigned bs;
  matrix<unsigned, 4, 4> carry_borrow_m4x4;

  // CHECK-DAG: [[SRET_ADDR_SUB_S_M:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_SUM_S_M:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_SUB_M_S:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_SUM_M_S:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64

  matrix<unsigned, 4, 4> sum_m_s = cm_addc(am4x4, bs, carry_borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUM_M_S]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_M_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_M_S]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_M_S:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_SUM_M_S]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_SUM_M_S]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_M_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_M_S]], i32 0, i32 0
  // CHECK: [[RET_SUM_M_S:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUM_M_S]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_M_S]], <16 x i32>* %{{.*}}, align 64

  matrix<unsigned, 4, 4> sub_m_s = cm_subb(am4x4, bs, carry_borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUB_M_S]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_M_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_M_S]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_M_S:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_SUB_M_S]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_SUB_M_S]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_M_S:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_M_S]], i32 0, i32 0
  // CHECK: [[RET_SUB_M_S:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUB_M_S]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_M_S]], <16 x i32>* %{{.*}}, align 64

  matrix<unsigned, 4, 4> sum_s_m = cm_addc(as, bm4x4, carry_borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUM_S_M]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_S_M:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_S_M]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_S_M:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_SUM_S_M]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_SUM_S_M]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_S_M:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_S_M]], i32 0, i32 0
  // CHECK: [[RET_SUM_S_M:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUM_S_M]], align 64
  // CHECK: store <16 x i32> [[RET_SUM_S_M]], <16 x i32>* %{{.*}}, align 64

  matrix<unsigned, 4, 4> sub_s_m = cm_subb(as, bm4x4, carry_borrow_m4x4);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUB_S_M]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_S_M:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_S_M]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_S_M:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_SUB_S_M]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_SUB_S_M]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_S_M:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_S_M]], i32 0, i32 0
  // CHECK: [[RET_SUB_S_M:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUB_S_M]], align 64
  // CHECK: store <16 x i32> [[RET_SUB_S_M]], <16 x i32>* %{{.*}}, align 64
}

// CHECK-LABEL: test32Cast
_GENX_MAIN_ void test32Cast() {
  vector<unsigned, 16> av;
  vector<unsigned, 16> bv;
  vector<unsigned, 16> carry_borrow;

  // CHECK-DAG: [[SRET_ADDR_SUB_V_V_LONG:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_SUM_V_V_LONG:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_SUB_V_V_SHORT:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64
  // CHECK-DAG: [[SRET_ADDR_SUM_V_V_SHORT:%[^ ]+]] = alloca [[RES_V_STRUCT32]], align 64

  vector<short, 16> sum_v_v_short = cm_addc(av, bv, carry_borrow);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUM_V_V_SHORT]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_V_V_SHORT:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_V_V_SHORT]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_V_V_SHORT:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_SUM_V_V_SHORT]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_SUM_V_V_SHORT]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_V_V_SHORT:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_V_V_SHORT]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_V_SHORT:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUM_V_V_SHORT]], align 64
  // CHECK: [[CONV_RET_SUM8:%[^ ]+]] = trunc <16 x i32> [[RET_SUM_V_V_SHORT]] to <16 x i16>
  // CHECK: store <16 x i16> [[CONV_RET_SUM8]], <16 x i16>* %{{.*}}, align 32

  vector<short, 16> sub_v_v_short = cm_subb(av, bv, carry_borrow);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUB_V_V_SHORT]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_V_V_SHORT:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_V_V_SHORT]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_V_V_SHORT:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_SUB_V_V_SHORT]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_SUB_V_V_SHORT]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_V_V_SHORT:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_V_V_SHORT]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_V_SHORT:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUB_V_V_SHORT]], align 64
  // CHECK: [[CONV_RET_SUB8:%[^ ]+]] = trunc <16 x i32> [[RET_SUB_V_V_SHORT]] to <16 x i16>
  // CHECK: store <16 x i16> [[CONV_RET_SUB8]], <16 x i16>* %{{.*}}, align 32

  vector<unsigned long long, 16> sum_v_v_long = cm_addc(av, bv, carry_borrow);
  // CHECK: call void {{.*}}__spirv_IAddCarryI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUM_V_V_LONG]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_CARRY_ADDR_SUM_V_V_LONG:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_V_V_LONG]], i32 0, i32 1
  // CHECK: [[RET_CARRY_SUM_V_V_LONG:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_CARRY_ADDR_SUM_V_V_LONG]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_CARRY_SUM_V_V_LONG]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUM_V_V_LONG:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUM_V_V_LONG]], i32 0, i32 0
  // CHECK: [[RET_SUM_V_V_LONG:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUM_V_V_LONG]], align 64
  // CHECK: [[CONV_RET_SUM9:%[^ ]+]] = zext <16 x i32> [[RET_SUM_V_V_LONG]] to <16 x i64>
  // CHECK: store <16 x i64> [[CONV_RET_SUM9]], <16 x i64>* %{{.*}}, align 128

  vector<unsigned long long, 16> sub_v_v_long = cm_subb(av, bv, carry_borrow);
  // CHECK: call void {{.*}}__spirv_ISubBorrowI{{.*}}([[RES_V_STRUCT32]]* sret align 64 [[SRET_ADDR_SUB_V_V_LONG]], <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: [[RET_BORROW_ADDR_SUB_V_V_LONG:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_V_V_LONG]], i32 0, i32 1
  // CHECK: [[RET_BORROW_SUB_V_V_LONG:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_BORROW_ADDR_SUB_V_V_LONG]], align 64
  // CHECK: call void @llvm.genx.vstore.v16i32.p0v16i32(<16 x i32> [[RET_BORROW_SUB_V_V_LONG]], <16 x i32>* %{{.*}})
  // CHECK: [[RET_ADDR_SUB_V_V_LONG:%[^ ]+]] = getelementptr inbounds [[RES_V_STRUCT32]], [[RES_V_STRUCT32]]* [[SRET_ADDR_SUB_V_V_LONG]], i32 0, i32 0
  // CHECK: [[RET_SUB_V_V_LONG:%[^ ]+]] = load <16 x i32>, <16 x i32>* [[RET_ADDR_SUB_V_V_LONG]], align 64
  // CHECK: [[CONV_RET_SUB9:%[^ ]+]] = zext <16 x i32> [[RET_SUB_V_V_LONG]] to <16 x i64>
  // CHECK: store <16 x i64> [[CONV_RET_SUB9]], <16 x i64>* %{{.*}}, align 128
}
