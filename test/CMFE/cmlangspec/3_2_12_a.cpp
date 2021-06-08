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

_GENX_ void test1()
{
  matrix<int, 8, 8> m1;
  // ...
 
  // Vector mask
  vector<ushort, 64> v_mask = (m1 > 0);
  ushort result = v_mask.any();
  if (result) {
    // At least one value in m1 is > 0
    // ...
  }
  if (v_mask.all()) {
    // All values in m1 are > 0
    // ...
  }
  
  // Matrix mask
  matrix<ushort, 8, 8> m_mask = (m1 == 0);
  if (m_mask.all()) {
    // All values in m1 are 0
    // ...
  }
  if ((m1 == 0).all()) {
    // Another way to express the same thing without using an
    // intermediate variable
    // ...
  }
  while ((m1 == 0).any()) {
    // As long as m1 still has a 0, keep looping...
    // ...
  }
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
