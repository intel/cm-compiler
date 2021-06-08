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

int init_a[] = { 0, 2, 4, 6, 8, 10, 12, 14};
int init_b[] = {0, 2, 3};
int init_c[] = {1, 0, 0};

_GENX_ void test1()
{
  // Here: init_a = {{0, 2}, {4, 6}, {8, 10}, {12, 14}};
  matrix<int, 4, 2> a(init_a);
  // Here: init_b = {0, 2, 3}; init_c = {1, 0, 0}
  vector<int, 3> b(init_b), c(init_c);
  vector<int, 3> out;
  out = a.iselect(b, c); // out = {2, 8, 12}
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
