/*========================== begin_copyright_notice ============================

Copyright (C) 2024-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -S -emit-llvm -march=pvc -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

_GENX_VOLATILE_ vector<float, 16> Src;
_GENX_VOLATILE_ vector<uint16_t, 16> Bias;

_GENX_VOLATILE_ vector<half, 16> Dst;

_GENX_MAIN_ void test() {
  // CHECK: call <16 x half> @llvm.genx.srnd.v16f16.v16f32.v16i16(<16 x float> %{{[^,]+}}, <16 x i16> %{{[^,]+}})
  Dst = cm_srnd<half>(Src, Bias);

  // CHECK: call <1 x half> @llvm.genx.srnd.v1f16.v1f32.v1i16(<1 x float> %{{[^,]+}}, <1 x i16> %{{[^,]+}})
  float SrcS = Src[3];
  uint16_t BiasS = Bias[7];
  Dst[5] = cm_srnd<half>(SrcS, BiasS);
}
