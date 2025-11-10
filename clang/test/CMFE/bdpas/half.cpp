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
_GENX_VOLATILE_ vector<half, M * N> AccHalf;

_GENX_VOLATILE_ vector<unsigned, K * N> Src1;
_GENX_VOLATILE_ vector<unsigned, M * K> Src2;

_GENX_VOLATILE_ vector<uchar, N> Src1Scale;
_GENX_VOLATILE_ vector<uchar, M> Src2Scale;

_GENX_MAIN_ void kernel() {
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  Acc = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M, float>(NULL, Src1, Src2, Src1Scale, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M, float>(NULL, Src1, Src2, NULL, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M, float>(NULL, Src1, Src2, Src1Scale, NULL);

  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  Acc = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M>(Acc, Src1, Src2, Src1Scale, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M>(Acc, Src1, Src2, NULL, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M>(Acc, Src1, Src2, Src1Scale, NULL);

  // CHECK: call <128 x half> @llvm.genx.bdpas.v128f16.v128f16.v128i32.v64i32.v16i8.v8i8(<128 x half> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x half> @llvm.genx.bdpas.v128f16.v128f16.v128i32.v64i32.v16i8.v8i8(<128 x half> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x half> @llvm.genx.bdpas.v128f16.v128f16.v128i32.v64i32.v16i8.v8i8(<128 x half> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  AccHalf = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M>(AccHalf, Src1, Src2, Src1Scale, Src2Scale);
  AccHalf = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M>(AccHalf, Src1, Src2, NULL, Src2Scale);
  AccHalf = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M>(AccHalf, Src1, Src2, Src1Scale, NULL);

  // CHECK: call <128 x half> @llvm.genx.bdpas.v128f16.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f16.v128i32.v64i32.v16i8.v8i8(<128 x half> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x half> @llvm.genx.bdpas.v128f16.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f16.v128i32.v64i32.v16i8.v8i8(<128 x half> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x half> @llvm.genx.bdpas.v128f16.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f16.v128i32.v64i32.v16i8.v8i8(<128 x half> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 10, i32 10, i32 8, i32 8)
  AccHalf = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M, half>(Acc, Src1, Src2, Src1Scale, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M, float>(AccHalf, Src1, Src2, Src1Scale, Src2Scale);
  AccHalf = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M, half>(Acc, Src1, Src2, NULL, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M, float>(AccHalf, Src1, Src2, NULL, Src2Scale);
  AccHalf = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M, half>(Acc, Src1, Src2, Src1Scale, NULL);
  Acc = cm_bdpas<CM_PRECISION_HF, CM_PRECISION_HF, K, M, float>(AccHalf, Src1, Src2, Src1Scale, NULL);
}
