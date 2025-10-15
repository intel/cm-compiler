/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -S -emit-llvm -march=pvc -- %s 2>&1 | FileCheck %s

// CHECK: error: Fence must be used at function scope!
CM_OPTIMIZATIONS_FENCE;

_GENX_MAIN_ void test_uint_SZ7(SurfaceIndex Buffer) {
  unsigned num = 66;
  { CM_OPTIMIZATIONS_FENCE; }
  vector<unsigned, 8> vec = num;
  write(Buffer, 0, vec);
}
