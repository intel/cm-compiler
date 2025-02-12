/*========================== begin_copyright_notice ============================

Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=ptl -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

constexpr unsigned size = 16;

_GENX_MAIN_ void test_f16_to_bf8(vector<half, size> input, vector<uchar, size> output) {
  output = cm_bf8_cvt<uchar>(input);
  // CHECK: call <16 x i8> @llvm.genx.qf.cvt.v16i8.v16f16(<16 x half> {{%.*}})
}

_GENX_MAIN_ void test_f32_to_bf8(vector<float, size> input, vector<uchar, size> output) {
  output = cm_bf8_cvt<uchar>(input);
  // CHECK: call <16 x i8> @llvm.genx.qf.cvt.v16i8.v16f16(<16 x half> {{%.*}})
}

_GENX_MAIN_ void test_bf8_to_f16(vector<uchar, size> input, vector<half, size> output) {
  output = cm_bf8_cvt<half>(input);
  // CHECK: call <16 x half> @llvm.genx.qf.cvt.v16f16.v16i8(<16 x i8> {{%.*}})
}

_GENX_MAIN_ void test_bf8_to_f32(vector<uchar, size> input, vector<float, size> output) {
  output = cm_bf8_cvt<float>(input);
  // CHECK: call <16 x half> @llvm.genx.qf.cvt.v16f16.v16i8(<16 x i8> {{%.*}})
}
