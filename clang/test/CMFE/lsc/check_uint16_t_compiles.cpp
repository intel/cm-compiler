/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/
// COM: Runs have no checks. They just ensure that FE compiles atomic lsc with different types
// RUN: %cmc -march=dg2 -DTYPE="float" -emit-spirv -o %t.ll -- %s
// RUN: %cmc -march=dg2 -DTYPE="unsigned" -emit-spirv -o %t.ll -- %s
// RUN: %cmc -march=dg2 -DTYPE="unsigned short" -emit-spirv -o %t.ll -- %s

using data_t = TYPE;

constexpr unsigned simd = 8;

extern "C" _GENX_MAIN_ //
    void
    kernel(SurfaceIndex buffer [[type("buffer_t")]]) {
  vector<data_t, simd> data;
  vector<data_t, simd> src0 = 0;
  vector<unsigned, simd> offset = 0;

  data = cm_atomic<AtomicOp::IADD, data_t>(buffer, offset, src0);
}
