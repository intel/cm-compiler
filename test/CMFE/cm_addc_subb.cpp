/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

_GENX_MAIN_ void test() {
  vector<unsigned, 16> a_v16;
  vector<unsigned, 16> b_v16;
  vector_ref<unsigned, 16> a_vr16 = a_v16;
  vector_ref<unsigned, 16> b_vr16 = b_v16;
  vector<unsigned, 16> carry_v16;
  vector<unsigned, 16> borrow_v16;

  vector<unsigned, 16> sum_v16_v16 = cm_addc(a_v16, b_v16, carry_v16);
  vector<unsigned, 16> sum_vr16_vr16 = cm_addc(a_vr16, b_vr16, carry_v16);
  vector<unsigned, 16> sum_vr16_v16 = cm_addc(a_vr16, b_v16, carry_v16);
  vector<unsigned, 16> sum_v16_vr16 = cm_addc(a_v16, b_vr16, carry_v16);
  vector<unsigned, 16> sub_v16_v16 = cm_subb(a_v16, b_v16, borrow_v16);
  vector<unsigned, 16> sub_vr16_vr16 = cm_subb(a_vr16, b_vr16, borrow_v16);
  vector<unsigned, 16> sub_vr16_v16 = cm_subb(a_vr16, b_v16, borrow_v16);
  vector<unsigned, 16> sub_v16_vr16 = cm_subb(a_v16, b_vr16, borrow_v16);

  matrix<unsigned, 4, 4> a_m4x4;
  matrix<unsigned, 4, 4> b_m4x4;
  matrix_ref<unsigned, 4, 4> a_mr4x4 = a_m4x4;
  matrix_ref<unsigned, 4, 4> b_mr4x4 = b_m4x4;
  matrix<unsigned, 4, 4> carry_m4x4;
  matrix<unsigned, 4, 4> borrow_m4x4;

  matrix<unsigned, 4, 4> sum_m4x4_m4x4 = cm_addc(a_m4x4, b_m4x4, carry_m4x4);
  matrix<unsigned, 4, 4> sum_mr4x4_mr4x4 = cm_addc(a_mr4x4, b_mr4x4, carry_m4x4);
  matrix<unsigned, 4, 4> sum_mr4x4_m4x4 = cm_addc(a_mr4x4, b_m4x4, carry_m4x4);
  matrix<unsigned, 4, 4> sum_m4x4_mr4x4 = cm_addc(a_m4x4, b_mr4x4, carry_m4x4);

  matrix<unsigned, 4, 4> sub_m4x4_m4x4 = cm_subb(a_m4x4, b_m4x4, borrow_m4x4);
  matrix<unsigned, 4, 4> sub_mr4x4_mr4x4 = cm_subb(a_mr4x4, b_mr4x4, borrow_m4x4);
  matrix<unsigned, 4, 4> sub_mr4x4_m4x4 = cm_subb(a_mr4x4, b_m4x4, borrow_m4x4);
  matrix<unsigned, 4, 4> sub_m4x4_mr4x4 = cm_subb(a_m4x4, b_mr4x4, borrow_m4x4);

  matrix<unsigned, 4, 4> a3;
  unsigned b3;
  matrix<unsigned, 4, 4> carry3;
  matrix<unsigned, 4, 4> sum3 = cm_addc(a3, b3, carry3);
  matrix<unsigned, 4, 4> borrow3;
  matrix<unsigned, 4, 4> sub3 = cm_subb(a3, b3, borrow3);

  unsigned a4;
  unsigned b4;
  unsigned carry4;
  unsigned sum4 = cm_addc(a4, b4, carry4);
  unsigned borrow4;
  unsigned sub4 = cm_subb(a4, b4, borrow4);

  vector<unsigned, 16> a5;
  unsigned b5;
  vector<unsigned, 16> carry5;
  vector<unsigned, 16> sum5 = cm_addc(a5, b5, carry5);
  vector<unsigned, 16> borrow5;
  vector<unsigned, 16> sub5 = cm_subb(a5, b5, borrow5);

  unsigned a6;
  vector<unsigned, 16> b6;
  vector<unsigned, 16> carry6;
  vector<unsigned, 16> sum6 = cm_addc(a6, b6, carry6);
  vector<unsigned, 16> borrow6;
  vector<unsigned, 16> sub6 = cm_subb(a6, b6, borrow6);

  unsigned a7;
  matrix<unsigned, 4, 4> b7;
  matrix<unsigned, 4, 4> carry7;
  matrix<unsigned, 4, 4> sum7 = cm_addc(a7, b7, carry7);
  matrix<unsigned, 4, 4> borrow7;
  matrix<unsigned, 4, 4> sub7 = cm_subb(a7, b7, borrow7);

  vector<unsigned, 16> a8;
  vector<unsigned, 16> b8;
  vector<unsigned, 16> carry8;
  vector<short, 16> sum8 = cm_addc(a8, b8, carry8);
  vector<unsigned, 16> borrow8;
  vector<short, 16> sub8 = cm_subb(a8, b8, borrow8);

  vector<unsigned, 16> a9;
  vector<unsigned, 16> b9;
  vector<unsigned, 16> carry9;
  vector<unsigned long long, 16> sum9 = cm_addc(a9, b9, carry9);
  vector<unsigned, 16> borrow9;
  vector<unsigned long long, 16> sub9 = cm_subb(a9, b9, borrow9);
}

  // CHECK-LABEL: define internal <16 x i32> @{{[_Z7cm_addc][^ ]}}
  // CHECK: [[ADDC_RES_V_V:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.addc.v16i32.v16i32(<16 x i32> [[ADDC_A_V_V:%[^ ]+]], <16 x i32> [[ADDC_B_V_V:%[^ ]+]])
  // CHECK: [[RET_CARRY_V_V:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES_V_V]], 0
  // CHECK: store <16 x i32> [[RET_CARRY_V_V]], <16 x i32>* [[CARRY_V_V:%[^ ]+]]
  // CHECK: [[RET_SUM_V_V:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES_V_V]], 1


  // CHECK-LABEL: define internal <16 x i32> @{{[_Z7cm_subb][^ ]}}
  // CHECK: [[SUBB_RES_V_V:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.subb.v16i32.v16i32(<16 x i32> [[SUBB_A_V_V:%[^ ]+]], <16 x i32> [[SUBB_B_V_V:%[^ ]+]])
  // CHECK: [[RET_BORROW_V_V:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES_V_V]], 0
  // CHECK: store <16 x i32> [[RET_BORROW_V_V]], <16 x i32>* [[BORROW_V_V:%[^ ]+]]
  // CHECK: [[RET_SUB_V_V:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES_V_V]], 1
