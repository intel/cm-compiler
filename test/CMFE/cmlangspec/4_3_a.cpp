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

#include <cm/cm.h>

_GENX_ void test()
{
  matrix<float, 2, 2>  angle;
  vector<float, 4>  r;
  // angle = ...;
  // r = ...;

  //Possible values of the elements of r is 0, 1, 2, ... 255
  r = cm_mul<uchar>(cm_sin(angle), r, SAT);

  matrix<int, 2, 3>  m1(20000), m2(20000);
  vector<int,6> v1, v2;
  //
  v1 = cm_add<short>(m1, m2, SAT);  // v1 = 32767
  v2 = cm_add<short>(m1, m2);       // v2 = -25536
  //
  vector<float, 8> v3; // v3 = {1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0};
  vector<float, 8> v4; // v4 = {0.2, 0.2, 0.2, 0.2, 0.3, 0.3, 0.3, 0.3};
  vector<float, 8> v5;
  // ...
  v5 = cm_dp4<float>(v3, v4);  // v5 = {0.8, 0.8, 0.8, 0.8, 2.4, 2.4, 2.4, 2.4}

  vector<float, 4> v6; //v6 = {0.1, 0.2, 0.3, 0.4};
  vector<float, 8> v7; // v7 = {1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0};
  vector<float, 8> v8;
  // ...
  v8 = cm_line<float>(v6, v7); // v8 = {0.5, 0.6, 0.8, 1.2, 2.0, 3.6, 6.8, 13.2}
}

// RUN: %cmc -emit-llvm -march=SKL -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
