#include <cm/cm.h>

_GENX_MAIN_ void
linear(vector<SurfaceIndex, 2> surf_ids, uint h_pos, uint v_pos, int i, int j)
{
  vector<int,8> in, out;
  // ...
  read(surf_ids(i), h_pos*24, v_pos*6, in);
  //  ...
  write(surf_ids(j), h_pos*24, v_pos*6, out);
}

// output a warning just to have some output from the compiler to check
#warning 2_3_a.cpp

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: 1 warning generated
