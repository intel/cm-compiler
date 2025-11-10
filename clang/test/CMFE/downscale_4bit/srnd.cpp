/*========================== begin_copyright_notice ============================

Copyright (C) 2024-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=cri -DTYPE=half -emit-llvm -S -o %t.half.ll -- %s
// RUN: FileCheck --check-prefix=HALF %s --input-file=%t.half.ll

// RUN: %cmc -march=cri -DTYPE=__bf16 -emit-llvm -S -o %t.bf16.ll -- %s
// RUN: FileCheck --check-prefix=BF16 %s --input-file=%t.bf16.ll

// RUN: %cmc -march=cri -DTYPE=int16_t -emit-llvm -S -o %t.i16.ll -- %s
// RUN: FileCheck --check-prefix=BF16 %s --input-file=%t.i16.ll

using InputTy = TYPE;
constexpr int Width = 32;

_GENX_VOLATILE_ vector<InputTy, Width> Src0;
_GENX_VOLATILE_ vector<InputTy, Width> Src1;

_GENX_VOLATILE_ vector<uint32_t, Width / 2> Bias;
_GENX_VOLATILE_ vector<uint32_t, Width / 2> Dst;

_GENX_MAIN_ void kernel() {
// HALF: call <16 x i32> @llvm.genx.4bit.downconvert.v16i32(<16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, i8 5, i8 0, i8 0)
// HALF: call <16 x i32> @llvm.genx.4bit.downconvert.v16i32(<16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, i8 4, i8 1, i8 0)
// HALF: call <16 x i32> @llvm.genx.4bit.downconvert.v16i32(<16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, i8 5, i8 2, i8 0)
// HALF: call <16 x i32> @llvm.genx.4bit.downconvert.v16i32(<16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, i8 4, i8 3, i8 0)

// BF16: call <16 x i32> @llvm.genx.4bit.downconvert.v16i32(<16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, i8 2, i8 0, i8 0)
// BF16: call <16 x i32> @llvm.genx.4bit.downconvert.v16i32(<16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, i8 1, i8 1, i8 0)
// BF16: call <16 x i32> @llvm.genx.4bit.downconvert.v16i32(<16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, i8 2, i8 2, i8 0)
// BF16: call <16 x i32> @llvm.genx.4bit.downconvert.v16i32(<16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, <16 x i32> %{{[^,]+}}, i8 1, i8 3, i8 0)

  Dst = cm_downscale<downscale::Int4, downscale::Mode0>(Src0, Src1, Bias);
  Dst = cm_downscale<downscale::E2M1, downscale::Mode1>(Src0, Src1, Bias);
  Dst = cm_downscale<downscale::Int4, downscale::Mode2>(Src0, Src1, Bias);
  Dst = cm_downscale<downscale::E2M1, downscale::Mode3>(Src0, Src1, Bias);
}
