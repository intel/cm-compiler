/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -march=skl -- %s 2> %t.txt
// RUN: FileCheck %s --input-file %t.txt

#include <cm/cm.h>

// CHECK: (24,1): error: Fence must be used at function scope!
CM_OPTIMIZATIONS_FENCE;

_GENX_MAIN_ void test_uint_SZ7(SurfaceIndex Buffer) {
  unsigned num = 66;
  { CM_OPTIMIZATIONS_FENCE; }
  vector<unsigned, 8> vec = num;
  write(Buffer, 0, vec);
}
