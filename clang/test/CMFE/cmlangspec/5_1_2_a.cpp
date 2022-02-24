/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// expected-no-diagnostics

#include <cm/cm.h>
#include <cm/cmtl.h>

#define WIDTH 4
#define HEIGHT 4

_GENX_MAIN_ void test(SurfaceIndex obuf)
{
  // SurfaceIndex obuf passed in as parameter
  // WIDTH and HEIGHT previously defined
 
  matrix<int, HEIGHT, WIDTH> block;
  int x = get_thread_origin_x() * WIDTH;
  int y = get_thread_origin_y() * HEIGHT;

  // Some work to put some relevant values into block
  // ...

  // Write block to output surface
  cmtl::WriteBlock(obuf, x, y, block.select_all());
}

// RUN: %cmc -emit-llvm -march=SKL -Xclang -verify -- %s
