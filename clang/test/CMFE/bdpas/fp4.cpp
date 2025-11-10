/*========================== begin_copyright_notice ============================

Copyright (C) 2024-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -S -march=cri -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

constexpr int M = 8;
constexpr int N = 16;
constexpr int K = 8;

_GENX_VOLATILE_ vector<float, M * N> Acc;

_GENX_VOLATILE_ vector<unsigned, K * N> Src1;
_GENX_VOLATILE_ vector<unsigned, M * K> Src2;

_GENX_VOLATILE_ matrix<uchar, 2, 32> Src1Scale;
_GENX_VOLATILE_ matrix<uchar, 2, 32> Src2Scale;

_GENX_MAIN_ void kernel() {
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v32i8.v16i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <32 x i8> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, i32 15, i32 15, i32 8, i32 8)
  Acc = cm_bdpas<CM_PRECISION_E2M1, CM_PRECISION_E2M1, K, M>(Acc, Src1, Src2, Src1Scale.select<2, 1, 16, 1>(0, 0), Src2Scale.select<2, 1, 8, 1>(0, 16));
}
