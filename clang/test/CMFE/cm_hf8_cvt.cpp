/*========================== begin_copyright_notice ============================

Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=ptl -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

constexpr unsigned size = 16;

_GENX_MAIN_ void test_f16_to_hf8(vector<half, size> input, vector<char, size> output) {
  output = cm_hf8_cvt<char>(input);
  // CHECK: call <16 x i8> @llvm.genx.hf8.cvt.v16i8.v16f16(<16 x half> {{%.*}})
}

_GENX_MAIN_ void test_hf8_to_f16(vector<char, size> input, vector<half, size> output) {
  output = cm_hf8_cvt<half>(input);
  // CHECK: call <16 x half> @llvm.genx.hf8.cvt.v16f16.v16i8(<16 x i8> {{%.*}})
}
