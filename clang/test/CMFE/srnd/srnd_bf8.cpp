/*========================== begin_copyright_notice ============================

Copyright (C) 2024-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -S -emit-llvm -march=cri -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

_GENX_VOLATILE_ vector<float, 16> SrcF;
_GENX_VOLATILE_ vector<half, 16> SrcH;
_GENX_VOLATILE_ vector<__bf16, 16> SrcBF;
_GENX_VOLATILE_ vector<uint8_t, 16> Bias;

_GENX_VOLATILE_ vector<uint8_t, 16> Dst;

_GENX_MAIN_ void test() {
  // CHECK: fptrunc <16 x float> %{{[^,]+}} to <16 x half>
  // CHECK: call <16 x i8> @llvm.genx.srnd.v16i8.v16f16.v16i8(<16 x half> %{{[^,]+}}, <16 x i8> %{{[^,]+}})
  Dst = cm_srnd<uint8_t>(SrcF, Bias);

  // CHECK: call <16 x i8> @llvm.genx.srnd.v16i8.v16f16.v16i8(<16 x half> %{{[^,]+}}, <16 x i8> %{{[^,]+}})
  Dst = cm_srnd<uint8_t>(SrcH, Bias);

  // CHECK: call <16 x i8> @llvm.genx.srnd.v16i8.v16bf16.v16i8(<16 x bfloat> %{{[^,]+}}, <16 x i8> %{{[^,]+}})
  Dst = cm_srnd<uint8_t>(SrcBF, Bias);

  // CHECK: fptrunc <16 x float> %{{[^,]+}} to <16 x half>
  // CHECK: call <16 x i8> @llvm.genx.biased.rounding.bf8.v16i8.v16f16(<16 x half> %{{[^,]+}}, <16 x i8> %{{[^,]+}})
  Dst = cm_srnd_bf8(SrcF, Bias);

  // CHECK: call <16 x i8> @llvm.genx.biased.rounding.bf8.v16i8.v16f16(<16 x half> %{{[^,]+}}, <16 x i8> %{{[^,]+}})
  Dst = cm_srnd_bf8(SrcH, Bias);

  // CHECK: call <16 x i8> @llvm.genx.biased.rounding.bf8.v16i8.v16bf16(<16 x bfloat> %{{[^,]+}}, <16 x i8> %{{[^,]+}})
  Dst = cm_srnd_bf8(SrcBF, Bias);
}
