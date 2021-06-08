/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

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

// RUN: not %cmc -emit-llvm -- %s 2>&1 | FileCheck %s

#include <cm/cm.h>

matrix<int,6,4> m;
vector<int,4> v;
struct S1 {
  int all;
} s1;
struct S2 {
  int x;
} s2;
_GENX_MAIN_
void test() {
  unsigned short r1 = m.all;             // missing '()'
  unsigned short r2 = m.all(;            // missing ')'
  unsigned short r3 = m.all(1;           // unexpected expression
  unsigned short r4 = m.all(9);          // unexpected expression
  unsigned short r5 = m.all();           // OK
  unsigned short r6 = m.template all();  // OK
  unsigned short r7 = v.all();           // OK
  unsigned short r8 = v.template all();  // OK
  unsigned short r9 = s1.all;            // OK
  unsigned short r10 = s1.template all;  // not a template
  unsigned short r11 = s2.all();         // no member all
  m.all();                               // expression result unused
  v.all();                               // expression result unused
  m.all() = 1;                           // not assignable
  v.all() = 0;                           // not assignable
  v.all().all();                         // ushort not a structure or union
}
// CHECK: all.cpp:15:28:  error: expected '('
// CHECK: all.cpp:16:29:  error: expected ')'
// CHECK: all.cpp:17:29:  error: expected ')'
// CHECK: all.cpp:18:29:  error: expected ')'
// CHECK: all.cpp:24:36:  error: 'all' following the 'template' keyword does not refer to a template
// CHECK: all.cpp:25:27:  error: no member named 'all' in 'S2'
// CHECK: all.cpp:28:11:  error: expression is not assignable
// CHECK: all.cpp:29:11:  error: expression is not assignable
// CHECK: all.cpp:30:10:  error: member reference base type 'unsigned short' is not a structure or union
// CHECK: all.cpp:26:3:  warning: expression result unused
// CHECK: all.cpp:27:3:  warning: expression result unused
