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

_GENX_ matrix<float, 2, 2> M;                        // global data declaration

_GENX_ matrix<float, 4, 4> plusone(matrix<float, 4, 4> m)
{
  return m+1.0f;
}

_GENX_ float foo(matrix<float, 4, 4> m)              // user defined GenX function
{
  matrix<float, 4, 4> newm = plusone(m);
  return cm_sum<float>(newm);
}

_GENX_ void bar_value(vector<float, 16> v, float f)  // user defined GenX function
                                                     // using pass-by-value for "v"
{
  matrix<float, 4, 4> m1;
  m1 = v + f;
  float s = foo(m1);
  M = m1.select<2, 2, 2, 2>(0, 0) * s;
}

_GENX_ void bar_ref(vector<float, 16> v, float f,    // user defined GenX function
                    matrix_ref<float, 2, 2> m)       // using pass-by-reference for "m"
{
  matrix<float, 4, 4> m1;
  m1 = v + f;
  float s = foo(m1);
  m = m1.select<2, 2, 2, 2>(0, 0) * s;
}

_GENX_MAIN_ void kernel(SurfaceIndex inbuf, SurfaceIndex outbuf, // GenX kernel function
                        int x_pos, int y_pos)
{
  matrix<float, 4, 4> m;
  vector<float, 16>   v;

  read(inbuf, x_pos, y_pos, m);
  v = m;
  bar_value(v, 0.5f);
  write(outbuf, x_pos, y_pos, M);
  bar_ref(v, 1.0f, M);
  write(outbuf, x_pos, y_pos + 2, M);
}

// output a warning just to have some output from the compiler to check
#warning 4_2_a.cpp

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck --implicit-check-not error %s
// CHECK: warning: 4_2_a.cpp
// CHECK: 1 warning generated
