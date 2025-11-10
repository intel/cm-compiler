/*========================== begin_copyright_notice ============================

Copyright (C) 2024-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=cri -DTYPE=half -emit-llvm -S -o %t.half.ll -- %s
// RUN: FileCheck --check-prefix=HALF %s --input-file=%t.half.ll

// RUN: %cmc -march=cri -DTYPE=__bf16 -emit-llvm -S -o %t.bf16.ll -- %s
// RUN: FileCheck --check-prefix=BF16 %s --input-file=%t.bf16.ll

// RUN: %cmc -march=cri -DTYPE=int16_t -emit-llvm -S -o %t.int.ll -- %s
// RUN: FileCheck --check-prefix=INT %s --input-file=%t.int.ll

using Ty = TYPE;
constexpr int Width = 32;

_GENX_VOLATILE_ vector<Ty, Width> Src;
_GENX_VOLATILE_ vector<Ty, Width> Dst;

_GENX_MAIN_ void kernel() {
  // HALF: call <32 x half> @llvm.genx.mxfp.linearize.v32f16(<32 x half> %{{[^,]+}})
  // BF16: call <32 x bfloat> @llvm.genx.mxfp.linearize.v32bf16(<32 x bfloat> %{{[^,]+}})
  // INT: call <32 x i16> @llvm.genx.mxfp.linearize.v32i16(<32 x i16> %{{[^,]+}})
  Dst = cm_mxfp_linearize(Src);
}
