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

// output a warning just to have some output from the compiler to check
#warning 5_1_1_a.cpp

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: 5_1_1_a.cpp
// CHECK: 1 warning generated
