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
_GENX_VOLATILE_ vector<__bf16, M * N> AccBf16;
_GENX_VOLATILE_ vector<short, M * N> AccBf16Short;

_GENX_VOLATILE_ vector<unsigned, K * N> Src1;
_GENX_VOLATILE_ vector<unsigned, M * K> Src2;

_GENX_VOLATILE_ vector<uchar, N> Src1Scale;
_GENX_VOLATILE_ vector<uchar, M> Src2Scale;

_GENX_MAIN_ void kernel() {
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  Acc = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M, float>(NULL, Src1, Src2, Src1Scale, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M, float>(NULL, Src1, Src2, NULL, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M, float>(NULL, Src1, Src2, Src1Scale, NULL);

  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  Acc = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M>(Acc, Src1, Src2, Src1Scale, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M>(Acc, Src1, Src2, NULL, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M>(Acc, Src1, Src2, Src1Scale, NULL);

  // CHECK: call <128 x bfloat> @llvm.genx.bdpas.v128bf16.v128bf16.v128i32.v64i32.v16i8.v8i8(<128 x bfloat> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x bfloat> @llvm.genx.bdpas.v128bf16.v128bf16.v128i32.v64i32.v16i8.v8i8(<128 x bfloat> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x bfloat> @llvm.genx.bdpas.v128bf16.v128bf16.v128i32.v64i32.v16i8.v8i8(<128 x bfloat> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  AccBf16 = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M>(AccBf16, Src1, Src2, Src1Scale, Src2Scale);
  AccBf16 = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M>(AccBf16, Src1, Src2, NULL, Src2Scale);
  AccBf16 = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M>(AccBf16, Src1, Src2, Src1Scale, NULL);

  // CHECK: call <128 x i16> @llvm.genx.bdpas.v128i16.v128i16.v128i32.v64i32.v16i8.v8i8(<128 x i16> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x i16> @llvm.genx.bdpas.v128i16.v128i16.v128i32.v64i32.v16i8.v8i8(<128 x i16> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x i16> @llvm.genx.bdpas.v128i16.v128i16.v128i32.v64i32.v16i8.v8i8(<128 x i16> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  AccBf16Short = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M>(AccBf16Short, Src1, Src2, Src1Scale, Src2Scale);
  AccBf16Short = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M>(AccBf16Short, Src1, Src2, NULL, Src2Scale);
  AccBf16Short = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M>(AccBf16Short, Src1, Src2, Src1Scale, NULL);

  // CHECK: call <128 x bfloat> @llvm.genx.bdpas.v128bf16.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128bf16.v128i32.v64i32.v16i8.v8i8(<128 x bfloat> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x bfloat> @llvm.genx.bdpas.v128bf16.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128bf16.v128i32.v64i32.v16i8.v8i8(<128 x bfloat> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x i16> @llvm.genx.bdpas.v128i16.v128f32.v128i32.v64i32.v16i8.v8i8(<128 x float> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  // CHECK: call <128 x float> @llvm.genx.bdpas.v128f32.v128i16.v128i32.v64i32.v16i8.v8i8(<128 x i16> %{{[^,]+}}, <128 x i32> %{{[^,]+}}, <64 x i32> %{{[^,]+}}, <16 x i8> %{{[^,]+}}, <8 x i8> %{{[^,]+}}, i32 9, i32 9, i32 8, i32 8)
  AccBf16 = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M, __bf16>(Acc, Src1, Src2, Src1Scale, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M, float>(AccBf16, Src1, Src2, Src1Scale, Src2Scale);
  AccBf16 = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M, __bf16>(Acc, Src1, Src2, NULL, Src2Scale);
  Acc = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M, float>(AccBf16, Src1, Src2, NULL, Src2Scale);
  AccBf16Short = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M, short>(Acc, Src1, Src2, Src1Scale, NULL);
  Acc = cm_bdpas<CM_PRECISION_BF, CM_PRECISION_BF, K, M, float>(AccBf16Short, Src1, Src2, Src1Scale, NULL);
}
