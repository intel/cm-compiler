/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

SPDX-License-Identifier: MIT
============================= end_copyright_notice ===========================*/

// XFAIL: *

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

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck --implicit-check-not error %s
// CHECK: 2_3_a.cpp(10,41):  warning: variable 'out' is uninitialized when used here [-Wuninitialized]
