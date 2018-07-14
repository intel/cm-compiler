#include <cm/cm.h>
#include <cm/cmtl.h>

#define WIDTH 4

_GENX_MAIN_ void test(SurfaceIndex obuf)
{
  // SurfaceIndex obuf passed in as parameter
  // WIDTH previously defined
 
  vector<int, WIDTH> block;
  int x = get_thread_origin_x() * WIDTH;

  // Some work to put some relevant values into block
  // ...

  // Write block to output surface
  cmtl::WriteLinear(obuf, x, block.select_all());
}

// output a warning just to have some output from the compiler to check
#warning 5_1_4_a.cpp

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: 5_1_4_a.cpp
// CHECK: 1 warning generated
