/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -march=skl -- %s 2> %t.txt
// RUN: FileCheck %s --input-file %t.txt
// XFAIL: *

// CHECK: OptFenceLowering cannot be global or not stack object

#include <cm/cm.h>

_GENX_MAIN_ void test_uint_SZ7(SurfaceIndex Buffer) {
  cm::CMOptimizationsFence *fence1 = new cm::CMOptimizationsFence;
}
