/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// XFAIL: *

#include <cm/cm.h>
#include <cm/cmtl.h>

#define WIDTH 4

_GENX_MAIN_ void test(SurfaceIndex ibuf)
{
  // SurfaceIndex ibuf passed in as parameter
  // WIDTH previously defined

  vector<int, WIDTH> block;
  int x = get_thread_origin_x() * WIDTH;

  // Read block from input surface
  cmtl::ReadLinear(ibuf, x, block.select_all());
}

// output a warning just to have some output from the compiler to check
#warning 5_1_3_a.cpp

// RUN: %cmc -emit-llvm -Wno-pass-failed -- %s 2>&1 | FileCheck --implicit-check-not error %s
// CHECK: warning: 5_1_3_a.cpp
// CHECK: 1 warning generated
