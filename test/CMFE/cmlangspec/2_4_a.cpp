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

                matrix<float, 2, 2> M;      // global variable declaration
_GENX_VOLATILE_ matrix<float, 4, 8> G;      // volatile global variable declaration

inline float foo(matrix<float, 4, 4> m)
{
  return cm_sum<float>(m);
}

inline void bar(vector<float, 16> v, float f)
{
  matrix<float, 4, 4> m1;
  m1 = v + f;
  float s = foo(m1);
  M = m1.select<2, 2, 2, 2>(0, 0) * s;
}

inline void baz(int i) {
  G += i;
}

_GENX_MAIN_ void kernel(SurfaceIndex inbuf, SurfaceIndex outbuf,
                        int x_pos, int y_pos)
{
  matrix<float, 4, 4> m;
  vector<float, 16>   v;

  read(inbuf, x_pos, y_pos, m);
  v = m;
  bar(v, 0.5f);
  write(outbuf, x_pos, y_pos, M);

  G = 0;
  baz(x_pos);
  write(outbuf, x_pos, y_pos, G);
}

// output a warning just to have some output from the compiler to check
#warning 2_4_a.cpp

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck --implicit-check-not error %s
// CHECK: 1 warning generated
