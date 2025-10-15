/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -S -emit-llvm -march=pvc -- %s 2>&1 | FileCheck %s

// CHECK: OptFenceLowering cannot be global or not stack object

_GENX_MAIN_ void test_uint_SZ7(SurfaceIndex Buffer) {
  cm::CMOptimizationsFence *fence1 = new cm::CMOptimizationsFence;
}
