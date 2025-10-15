/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -S -march=bmg -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

extern "C" _GENX_MAIN_ void load_3d_l(SurfaceIndex i [[type("image2d_t")]],
                                      vector<int, 8> u, vector<int, 8> v) {
  matrix<int, 4, 8> dst;
  // CHECK: call <32 x i32> @llvm.genx.3d.load.{{[^)]*}}(i32 27
  cm_3d_load<CM_3D_LOAD_L, CM_ABGR_ENABLE>(dst.format<int>(), 0, i, u, v);
}
