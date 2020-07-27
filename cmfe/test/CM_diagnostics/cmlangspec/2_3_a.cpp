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

// RUN: %cmc -mCM_old_asm_name %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: 2_3_a.cpp(10,41):  warning: variable 'out' is uninitialized when used here [-Wuninitialized]
