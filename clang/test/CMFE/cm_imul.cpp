/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

_GENX_MAIN_ void test_uint_SZ7(vector<uint, 7> a, vector<uint, 7> b, vector<uint, 7> lo, vector<uint, 7> hi) {
  hi = cm_imul<uint>(lo, a, b);
  // CHECK:   [[IMAD_U7:%.*]] = call { <7 x i32>, <7 x i32> } @llvm.genx.uimad.v7i32.v7i32(<7 x i32> [[SRC0_U7:%.*]], <7 x i32> [[SRC1_U7:%.*]], <7 x i32> zeroinitializer)
  // CHECK:   [[LO_U7:%.*]] = extractvalue { <7 x i32>, <7 x i32> } [[IMAD_U7]], 1
  // CHECK:   [[HI_U7:%.*]] = extractvalue { <7 x i32>, <7 x i32> } [[IMAD_U7]], 0
  // CHECK:   store <7 x i32> [[LO_U7]], <7 x i32>* [[LO_ADDR_U7:%.*]]
}

_GENX_MAIN_ void test_uint_SZ16(vector<uint, 16> a, vector<uint, 16> b, vector<uint, 16> lo, vector<uint, 16> hi) {
  hi = cm_imul<uint>(lo, a, b);
  // CHECK:   [[IMAD_U16:%.*]] = call { <16 x i32>, <16 x i32> } @llvm.genx.uimad.v16i32.v16i32(<16 x i32> [[SRC0_U16:%.*]], <16 x i32> [[SRC1_U16:%.*]], <16 x i32> zeroinitializer)
  // CHECK:   [[LO_U16:%.*]] = extractvalue { <16 x i32>, <16 x i32> } [[IMAD_U16]], 1
  // CHECK:   [[HI_U16:%.*]] = extractvalue { <16 x i32>, <16 x i32> } [[IMAD_U16]], 0
  // CHECK:   store <16 x i32> [[LO_U16]], <16 x i32>* [[LO_ADDR_U16:%.*]]
}

_GENX_MAIN_ void test_int_SZ7(vector<int, 7> a, vector<int, 7> b, vector<int, 7> lo, vector<int, 7> hi) {
  hi = cm_imul<int>(lo, a, b);
  // CHECK:   [[IMAD_S7:%.*]] = call { <7 x i32>, <7 x i32> } @llvm.genx.simad.v7i32.v7i32(<7 x i32> [[SRC0_S7:%.*]], <7 x i32> [[SRC1_S7:%.*]], <7 x i32> zeroinitializer)
  // CHECK:   [[LO_S7:%.*]] = extractvalue { <7 x i32>, <7 x i32> } [[IMAD_S7]], 1
  // CHECK:   [[HI_S7:%.*]] = extractvalue { <7 x i32>, <7 x i32> } [[IMAD_S7]], 0
  // CHECK:   store <7 x i32> [[LO_S7]], <7 x i32>* [[LO_ADDR_S7:%.*]]
}

_GENX_MAIN_ void test_int_SZ16(vector<int, 16> a, vector<int, 16> b, vector<int, 16> lo, vector<int, 16> hi) {
  hi = cm_imul<int>(lo, a, b);
  // CHECK:   [[IMAD_S16:%.*]] = call { <16 x i32>, <16 x i32> } @llvm.genx.simad.v16i32.v16i32(<16 x i32> [[SRC0_S16:%.*]], <16 x i32> [[SRC1_S16:%.*]], <16 x i32> zeroinitializer)
  // CHECK:   [[LO_S16:%.*]] = extractvalue { <16 x i32>, <16 x i32> } [[IMAD_S16]], 1
  // CHECK:   [[HI_S16:%.*]] = extractvalue { <16 x i32>, <16 x i32> } [[IMAD_S16]], 0
  // CHECK:   store <16 x i32> [[LO_S16]], <16 x i32>* [[LO_ADDR_S16:%.*]]
}

_GENX_MAIN_ void test_int_SZ1(vector<int, 1> a, vector<int, 1> b, vector<int, 1> lo, vector<int, 1> hi) {
  hi = cm_imul<int>(lo, a, b);
  // CHECK: [[IMAD_S1:%.*]] = call { i32, i32 } @llvm.genx.simad.i32.i32(i32 [[SRC0_S1:%.*]], i32 [[SRC0_S1:%.*]], i32 0)
  // CHECK: [[LO_S1:%.*]] = extractvalue { i32, i32 } [[IMAD_S1]], 1
  // CHECK: [[HI_S1:%.*]] = extractvalue { i32, i32 } [[IMAD_S1]], 0
  // CHECK: store i32 [[LO_S1]], i32* [[LO_ADDR_S1:%.*]]
}

_GENX_MAIN_ void test_int_scalar(int a, int b, int lo, int hi) {
  hi = cm_imul<int>(lo, a, b);
  // CHECK: [[IMAD_SCALAR:%.*]] = call { i32, i32 } @llvm.genx.simad.i32.i32(i32 [[SRC0_SCALAR:%.*]], i32 [[SRC0_SCALAR:%.*]], i32 0)
  // CHECK: [[LO_SCALAR:%.*]] = extractvalue { i32, i32 } [[IMAD_SCALAR]], 1
  // CHECK: [[HI_SCALAR:%.*]] = extractvalue { i32, i32 } [[IMAD_SCALAR]], 0
  // CHECK: store i32 [[LO_SCALAR]], i32* [[LO_ADDR_SCALAR:%.*]]
}
