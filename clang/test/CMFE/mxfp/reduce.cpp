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
constexpr int Height = 32;

_GENX_VOLATILE_ matrix<Ty, Height, Width> Src;
_GENX_VOLATILE_ vector<Ty, Height> Dst;

_GENX_MAIN_ void kernel() {
  // HALF: call <32 x half> @llvm.genx.mxfp.reduce.32x32.v32f16.v1024f16(<1024 x half> %{{[^,]+}})
  // BF16: call <32 x bfloat> @llvm.genx.mxfp.reduce.32x32.v32bf16.v1024bf16(<1024 x bfloat> %{{[^,]+}})
  // INT: call <32 x i16> @llvm.genx.mxfp.reduce.32x32.v32i16.v1024i16(<1024 x i16> %{{[^,]+}})
  Dst = cm_mxfp_reduce(Src);
}
