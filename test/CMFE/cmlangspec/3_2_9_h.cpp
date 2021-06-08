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
  vector<int, 65> inVector = 0;
  vector<int, 8>  outVector = 0;
  vector<int, 12> tempVector = 0;
  vector<ushort, 12>  idx;

  inVector.select<2,3>(4) = 19; // Here: inVector(4) = 19, inVector(7) = 19
  //Now: inVector = {0,0,0,19,0,0,0,19,0...}

  outVector = inVector.select<8,1>(0);
  //Now: outVector = {0,0,0,19,0,0,0,19}
 
  idx = 7; idx(0) = 2; idx(2) = 4; idx(3) = 10;
  // Now: idx = {2,7,4,10,7,7,7,7,7,7,7,7}

  tempVector = inVector.iselect(idx);
  // Now: tempVector = {0,19,19,0,19,19,0,0,0,0}
  
  tempVector = tempVector * 2;
  // Now: tempVector = {0,38,38,0,38,38, ... }

  outVector.select<8,1>(0) += tempVector.select<8,1>(1);
  // Now: outVector = {38,38,0,57,38,38,57,38}
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
