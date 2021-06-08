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
  matrix<int, 4, 4>       m1;
  // ...
  matrix_ref<char, 4, 16> m2 = m1.format<char, 4, 16>( );
  // m2 is a reference to the location of m1
  // interpreted as a matrix 4x16 of chars.
  matrix_ref<int, 2, 8>   m3 = m1.format<int, 2, 8>( );
  // m3 is a reference to the location of m1
  // interpreted as a 2x8 integer matrix.
  // (assuming little endian layout, i.e.
  //  the lowest byte address of element
  //  m1(0,0) is referenced by m2(0,0))

  matrix<float, 2, 8>  m4;
  // ...
  vector_ref<float, 16> v1 = m4.format<float>();
  // v1 is a reference to the location of m4
  // interpreted as a vector of 16 floats.
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
