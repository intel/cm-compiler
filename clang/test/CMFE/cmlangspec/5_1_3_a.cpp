/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// expected-no-diagnostics

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

// RUN: %cmc -emit-llvm -march=SKL -Xclang -verify -- %s
