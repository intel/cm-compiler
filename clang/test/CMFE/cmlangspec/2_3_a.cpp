/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

_GENX_MAIN_ void
linear(vector<SurfaceIndex, 2> surf_ids, uint h_pos, uint v_pos, int i, int j)
{
  vector<int,8> in, out;
  // ...
  read(surf_ids(i), h_pos*24, v_pos*6, in);
  //  ...
  write(surf_ids(j), h_pos*24, v_pos*6, out); // expected-warning{{variable 'out' is uninitialized when used here}}
}

// RUN: %cmc -emit-llvm -Xclang -verify -Xclang -verify-ignore-unexpected -- %s
