/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/
// COM: Check that FE throws an error for incorrect atomic lsc argument type
// RUN: %cmc -march=dg2 -DTYPE="char" -emit-spirv -o %t.ll -- %s 2>&1 | FileCheck %s

using data_t = TYPE;

constexpr unsigned simd = 8;

extern "C" _GENX_MAIN_ //
    void
    kernel(SurfaceIndex buffer [[type("buffer_t")]]) {
  vector<data_t, simd> data;
  vector<data_t, simd> src0 = 0;
  vector<unsigned, simd> offset = 0;

// CHECK: error: unsupported type for lsc atomic source or dest arguments
// CHECK: LLVM ERROR: Frontend detected a fatal error!
  data = cm_atomic<AtomicOp::IADD, data_t>(buffer, offset, src0);
}
