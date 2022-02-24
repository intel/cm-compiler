/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// expected-no-diagnostics

#include <cm/cm.h>
#include <cm/cmtl.h>

#define WIDTH 4
#define HEIGHT 4

_GENX_MAIN_ void test(SurfaceIndex ibuf, int surfWidth)
{
  // SurfaceIndex ibuf passed in as parameter
  // WIDTH and HEIGHT previously defined
  // surfWidth previously defined or passed as parameter
  matrix<int, HEIGHT, WIDTH> block;
  int x = get_thread_origin_x() * WIDTH;
  int y = get_thread_origin_y() * HEIGHT;

  // Read without considering non-dword aligned surface and replication
  cmtl::ReadBlock(ibuf, x, y, block.select_all());

  // Read taking into consideration non-dword aligned surfaces
  cmtl::ReadBlock(ibuf, x, y, block.select_all(), surfWidth);
}

// RUN: %cmc -emit-llvm -march=SKL -Xclang -verify -- %s
