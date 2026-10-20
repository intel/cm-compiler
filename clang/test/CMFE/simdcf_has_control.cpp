/*========================== begin_copyright_notice ============================

Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// A platform-gated builtin used under SIMD control flow at different widths
// in one module must not trip "mismatching SIMD width of called subroutine".

// RUN: %cmc -march=pvc -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

_GENX_MAIN_ void test16(SurfaceIndex Buf, vector<float, 16> Src,
                        vector<int, 16> Cond) {
  vector<int, 16> Dst = 0;
  SIMD_IF_BEGIN(Cond > 0) { Dst = cm_tf32_cvt<int>(Src); }
  SIMD_IF_END;
  write(Buf, 0, Dst);
}

_GENX_MAIN_ void test8(SurfaceIndex Buf, vector<float, 8> Src,
                       vector<int, 8> Cond) {
  vector<int, 8> Dst = 0;
  SIMD_IF_BEGIN(Cond > 0) { Dst = cm_tf32_cvt<int>(Src); }
  SIMD_IF_END;
  write(Buf, 0, Dst);
}

// CHECK-LABEL: define {{.*}}test16
// CHECK: select <16 x i1>
// CHECK-LABEL: define {{.*}}test8
// CHECK: select <8 x i1>
