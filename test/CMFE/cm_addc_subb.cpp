// RUN: %cmc -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

_GENX_MAIN_ void test() {
  vector<unsigned, 16> a1;
  vector<unsigned, 16> b1;
  vector<unsigned, 16> carry1;
  vector<unsigned, 16> sum1 = cm_addc(a1, b1, carry1);
  // CHECK: [[ADDC_RES1:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.addc.v16i32.v16i32(<16 x i32> [[ADDC_A1:%[^ ]+]], <16 x i32> [[ADDC_B1:%[^ ]+]])
  // CHECK: [[RET_CARRY1:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES1]], 0
  // CHECK: store <16 x i32> [[RET_CARRY1]], <16 x i32>* [[CARRY1:%[^ ]+]]
  // CHECK: [[RET_SUM1:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES1]], 1
  // CHECK: store <16 x i32> [[RET_SUM1]], <16 x i32>* [[SUM1:%[^ ]+]]
  vector<unsigned, 16> borrow1;
  vector<unsigned, 16> sub1 = cm_subb(a1, b1, borrow1);
  // CHECK: [[SUBB_RES1:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.subb.v16i32.v16i32(<16 x i32> [[SUBB_A1:%[^ ]+]], <16 x i32> [[SUBB_B1:%[^ ]+]])
  // CHECK: [[RET_BORROW1:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES1]], 0
  // CHECK: store <16 x i32> [[RET_BORROW1]], <16 x i32>* [[BORROW1:%[^ ]+]]
  // CHECK: [[RET_SUB1:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES1]], 1
  // CHECK: store <16 x i32> [[RET_SUB1]], <16 x i32>* [[SUB1:%[^ ]+]]

  matrix<unsigned, 4, 4> a2;
  matrix<unsigned, 4, 4> b2;
  matrix<unsigned, 4, 4> carry2;
  matrix<unsigned, 4, 4> sum2 = cm_addc(a2, b2, carry2);
  // CHECK: [[ADDC_RES2:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.addc.v16i32.v16i32(<16 x i32> [[ADDC_A2:%[^ ]+]], <16 x i32> [[ADDC_B2:%[^ ]+]])
  // CHECK: [[RET_CARRY2:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES2]], 0
  // CHECK: store <16 x i32> [[RET_CARRY2]], <16 x i32>* [[CARRY2:%[^ ]+]]
  // CHECK: [[RET_SUM2:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES2]], 1
  // CHECK: store <16 x i32> [[RET_SUM2]], <16 x i32>* [[SUM2:%[^ ]+]]
  matrix<unsigned, 4, 4> borrow2;
  matrix<unsigned, 4, 4> sub2 = cm_subb(a2, b2, borrow2);
  // CHECK: [[SUBB_RES2:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.subb.v16i32.v16i32(<16 x i32> [[SUBB_A2:%[^ ]+]], <16 x i32> [[SUBB_B2:%[^ ]+]])
  // CHECK: [[RET_BORROW2:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES2]], 0
  // CHECK: store <16 x i32> [[RET_BORROW2]], <16 x i32>* [[BORROW2:%[^ ]+]]
  // CHECK: [[RET_SUB2:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES2]], 1
  // CHECK: store <16 x i32> [[RET_SUB2]], <16 x i32>* [[SUB2:%[^ ]+]]

  matrix<unsigned, 4, 4> a3;
  unsigned b3;
  matrix<unsigned, 4, 4> carry3;
  matrix<unsigned, 4, 4> sum3 = cm_addc(a3, b3, carry3);
  // CHECK: [[ADDC_RES3:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.addc.v16i32.v16i32(<16 x i32> [[ADDC_A3:%[^ ]+]], <16 x i32> [[ADDC_B3:%[^ ]+]])
  // CHECK: [[RET_CARRY3:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES3]], 0
  // CHECK: store <16 x i32> [[RET_CARRY3]], <16 x i32>* [[CARRY3:%[^ ]+]]
  // CHECK: [[RET_SUM3:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES3]], 1
  // CHECK: store <16 x i32> [[RET_SUM3]], <16 x i32>* [[SUM3:%[^ ]+]]
  matrix<unsigned, 4, 4> borrow3;
  matrix<unsigned, 4, 4> sub3 = cm_subb(a3, b3, borrow3);
  // CHECK: [[SUBB_RES3:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.subb.v16i32.v16i32(<16 x i32> [[SUBB_A3:%[^ ]+]], <16 x i32> [[SUBB_B3:%[^ ]+]])
  // CHECK: [[RET_BORROW3:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES3]], 0
  // CHECK: store <16 x i32> [[RET_BORROW3]], <16 x i32>* [[BORROW3:%[^ ]+]]
  // CHECK: [[RET_SUB3:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES3]], 1
  // CHECK: store <16 x i32> [[RET_SUB3]], <16 x i32>* [[SUB3:%[^ ]+]]

  unsigned a4;
  unsigned b4;
  unsigned carry4;
  unsigned sum4 = cm_addc(a4, b4, carry4);
  // CHECK: [[ADDC_RES4:%[^ ]+]] = call { <1 x i32>, <1 x i32> } @llvm.genx.addc.v1i32.v1i32(<1 x i32> [[ADDC_A4:%[^ ]+]], <1 x i32> [[ADDC_B4:%[^ ]+]])
  // CHECK: [[RET_CARRY4:%[^ ]+]] = extractvalue { <1 x i32>, <1 x i32> } [[ADDC_RES4]], 0
  // CHECK: store <1 x i32> [[RET_CARRY4]], <1 x i32>* [[CARRY4:%[^ ]+]]
  // CHECK: [[RET_SUM4:%[^ ]+]] = extractvalue { <1 x i32>, <1 x i32> } [[ADDC_RES4]], 1
  // CHECK: store <1 x i32> [[RET_SUM4]], <1 x i32>* [[SUM4:%[^ ]+]]
  unsigned borrow4;
  unsigned sub4 = cm_subb(a4, b4, borrow4);
  // CHECK: [[SUBB_RES4:%[^ ]+]] = call { <1 x i32>, <1 x i32> } @llvm.genx.subb.v1i32.v1i32(<1 x i32> [[SUBB_A4:%[^ ]+]], <1 x i32> [[SUBB_B4:%[^ ]+]])
  // CHECK: [[RET_BORROW4:%[^ ]+]] = extractvalue { <1 x i32>, <1 x i32> } [[SUBB_RES4]], 0
  // CHECK: store <1 x i32> [[RET_BORROW4]], <1 x i32>* [[BORROW4:%[^ ]+]]
  // CHECK: [[RET_SUB4:%[^ ]+]] = extractvalue { <1 x i32>, <1 x i32> } [[SUBB_RES4]], 1
  // CHECK: store <1 x i32> [[RET_SUB4]], <1 x i32>* [[SUB4:%[^ ]+]]

  vector<unsigned, 16> a5;
  unsigned b5;
  vector<unsigned, 16> carry5;
  vector<unsigned, 16> sum5 = cm_addc(a5, b5, carry5);
  // CHECK: [[ADDC_RES5:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.addc.v16i32.v16i32(<16 x i32> [[ADDC_A5:%[^ ]+]], <16 x i32> [[ADDC_B5:%[^ ]+]])
  // CHECK: [[RET_CARRY5:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES5]], 0
  // CHECK: store <16 x i32> [[RET_CARRY5]], <16 x i32>* [[CARRY5:%[^ ]+]]
  // CHECK: [[RET_SUM5:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES5]], 1
  // CHECK: store <16 x i32> [[RET_SUM5]], <16 x i32>* [[SUM5:%[^ ]+]]
  vector<unsigned, 16> borrow5;
  vector<unsigned, 16> sub5 = cm_subb(a5, b5, borrow5);
  // CHECK: [[SUBB_RES5:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.subb.v16i32.v16i32(<16 x i32> [[SUBB_A5:%[^ ]+]], <16 x i32> [[SUBB_B5:%[^ ]+]])
  // CHECK: [[RET_BORROW5:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES5]], 0
  // CHECK: store <16 x i32> [[RET_BORROW5]], <16 x i32>* [[BORROW5:%[^ ]+]]
  // CHECK: [[RET_SUB5:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES5]], 1
  // CHECK: store <16 x i32> [[RET_SUB5]], <16 x i32>* [[SUB5:%[^ ]+]]

  unsigned a6;
  vector<unsigned, 16> b6;
  vector<unsigned, 16> carry6;
  vector<unsigned, 16> sum6 = cm_addc(a6, b6, carry6);
  // CHECK: [[ADDC_RES6:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.addc.v16i32.v16i32(<16 x i32> [[ADDC_A6:%[^ ]+]], <16 x i32> [[ADDC_B6:%[^ ]+]])
  // CHECK: [[RET_CARRY6:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES6]], 0
  // CHECK: store <16 x i32> [[RET_CARRY6]], <16 x i32>* [[CARRY6:%[^ ]+]]
  // CHECK: [[RET_SUM6:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES6]], 1
  // CHECK: store <16 x i32> [[RET_SUM6]], <16 x i32>* [[SUM6:%[^ ]+]]
  vector<unsigned, 16> borrow6;
  vector<unsigned, 16> sub6 = cm_subb(a6, b6, borrow6);
  // CHECK: [[SUBB_RES6:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.subb.v16i32.v16i32(<16 x i32> [[SUBB_A6:%[^ ]+]], <16 x i32> [[SUBB_B6:%[^ ]+]])
  // CHECK: [[RET_BORROW6:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES6]], 0
  // CHECK: store <16 x i32> [[RET_BORROW6]], <16 x i32>* [[BORROW6:%[^ ]+]]
  // CHECK: [[RET_SUB6:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES6]], 1
  // CHECK: store <16 x i32> [[RET_SUB6]], <16 x i32>* [[SUB6:%[^ ]+]]

  unsigned a7;
  matrix<unsigned, 4, 4> b7;
  matrix<unsigned, 4, 4> carry7;
  matrix<unsigned, 4, 4> sum7 = cm_addc(a7, b7, carry7);
  // CHECK: [[ADDC_RES7:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.addc.v16i32.v16i32(<16 x i32> [[ADDC_A7:%[^ ]+]], <16 x i32> [[ADDC_B7:%[^ ]+]])
  // CHECK: [[RET_CARRY7:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES7]], 0
  // CHECK: store <16 x i32> [[RET_CARRY7]], <16 x i32>* [[CARRY7:%[^ ]+]]
  // CHECK: [[RET_SUM7:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES7]], 1
  // CHECK: store <16 x i32> [[RET_SUM7]], <16 x i32>* [[SUM7:%[^ ]+]]
  matrix<unsigned, 4, 4> borrow7;
  matrix<unsigned, 4, 4> sub7 = cm_subb(a7, b7, borrow7);
  // CHECK: [[SUBB_RES7:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.subb.v16i32.v16i32(<16 x i32> [[SUBB_A7:%[^ ]+]], <16 x i32> [[SUBB_B7:%[^ ]+]])
  // CHECK: [[RET_BORROW7:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES7]], 0
  // CHECK: store <16 x i32> [[RET_BORROW7]], <16 x i32>* [[BORROW7:%[^ ]+]]
  // CHECK: [[RET_SUB7:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES7]], 1
  // CHECK: store <16 x i32> [[RET_SUB7]], <16 x i32>* [[SUB7:%[^ ]+]]

  vector<unsigned, 16> a8;
  vector<unsigned, 16> b8;
  vector<unsigned, 16> carry8;
  vector<short, 16> sum8 = cm_addc(a8, b8, carry8);
  // CHECK: [[ADDC_RES8:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.addc.v16i32.v16i32(<16 x i32> [[ADDC_A8:%[^ ]+]], <16 x i32> [[ADDC_B8:%[^ ]+]])
  // CHECK: [[RET_CARRY8:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES8]], 0
  // CHECK: store <16 x i32> [[RET_CARRY8]], <16 x i32>* [[CARRY8:%[^ ]+]]
  // CHECK: [[RET_SUM8:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES8]], 1
  // CHECK: [[CONV_RET_SUM8:%[^ ]+]] = trunc <16 x i32> [[RET_SUM8]] to <16 x i16>
  // CHECK: store <16 x i16> [[CONV_RET_SUM8]], <16 x i16>* [[SUM8:%[^ ]+]]
  vector<unsigned, 16> borrow8;
  vector<short, 16> sub8 = cm_subb(a8, b8, borrow8);
  // CHECK: [[SUBB_RES8:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.subb.v16i32.v16i32(<16 x i32> [[SUBB_A8:%[^ ]+]], <16 x i32> [[SUBB_B8:%[^ ]+]])
  // CHECK: [[RET_BORROW8:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES8]], 0
  // CHECK: store <16 x i32> [[RET_BORROW8]], <16 x i32>* [[BORROW8:%[^ ]+]]
  // CHECK: [[RET_SUB8:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES8]], 1
  // CHECK: [[CONV_RET_SUB8:%[^ ]+]] = trunc <16 x i32> [[RET_SUB8]] to <16 x i16>
  // CHECK: store <16 x i16> [[CONV_RET_SUB8]], <16 x i16>* [[SUB8:%[^ ]+]]

  vector<unsigned, 16> a9;
  vector<unsigned, 16> b9;
  vector<unsigned, 16> carry9;
  vector<unsigned long long, 16> sum9 = cm_addc(a9, b9, carry9);
  // CHECK: [[ADDC_RES9:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.addc.v16i32.v16i32(<16 x i32> [[ADDC_A9:%[^ ]+]], <16 x i32> [[ADDC_B9:%[^ ]+]])
  // CHECK: [[RET_CARRY9:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES9]], 0
  // CHECK: store <16 x i32> [[RET_CARRY9]], <16 x i32>* [[CARRY9:%[^ ]+]]
  // CHECK: [[RET_SUM9:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[ADDC_RES9]], 1
  // CHECK: [[CONV_RET_SUM9:%[^ ]+]] = zext <16 x i32> [[RET_SUM9]] to <16 x i64>
  // CHECK: store <16 x i64> [[CONV_RET_SUM9]], <16 x i64>* [[SUM9:%[^ ]+]]
  vector<unsigned, 16> borrow9;
  vector<unsigned long long, 16> sub9 = cm_subb(a9, b9, borrow9);
  // CHECK: [[SUBB_RES9:%[^ ]+]] = call { <16 x i32>, <16 x i32> } @llvm.genx.subb.v16i32.v16i32(<16 x i32> [[SUBB_A9:%[^ ]+]], <16 x i32> [[SUBB_B9:%[^ ]+]])
  // CHECK: [[RET_BORROW9:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES9]], 0
  // CHECK: store <16 x i32> [[RET_BORROW9]], <16 x i32>* [[BORROW9:%[^ ]+]]
  // CHECK: [[RET_SUB9:%[^ ]+]] = extractvalue { <16 x i32>, <16 x i32> } [[SUBB_RES9]], 1
  // CHECK: [[CONV_RET_SUB9:%[^ ]+]] = zext <16 x i32> [[RET_SUB9]] to <16 x i64>
  // CHECK: store <16 x i64> [[CONV_RET_SUB9]], <16 x i64>* [[SUB9:%[^ ]+]]
}
