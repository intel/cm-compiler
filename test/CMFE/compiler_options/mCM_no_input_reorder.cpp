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

_GENX_MAIN_ void foo(SurfaceIndex ibuf, SurfaceIndex obuf, char val, uint h_pos, uint v_pos)
{
    matrix<uchar, 8, 32> in;
    matrix<uchar, 6, 24> out;
    matrix<float, 6, 24> m;

    read(ibuf, h_pos*24, v_pos*6, in); 

    m  = in.select<6,1,24,1>(1,3);

    m += in.select<6,1,24,1>(0,0);
    m += in.select<6,1,24,1>(0,3);
    m += in.select<6,1,24,1>(0,6);

    m += in.select<6,1,24,1>(1,0);
    m += in.select<6,1,24,1>(1,6);

    m += in.select<6,1,24,1>(2,0);
    m += in.select<6,1,24,1>(2,3);
    m += in.select<6,1,24,1>(2,6);
    m += (float)val;

    out = m * 0.111f;

    write(obuf, h_pos*24, v_pos*6, out); 
}

// RUN: %cmc -emit-llvm -march=BDW -mCM_no_input_reorder -- %s | FileCheck %s
//
// CHECK: -platform BDW
// CHECK-NOT: error
// CHECK-NOT: warning

// We check that the generated asm contains arg reorder information that indicates
// no reordering has taken place (default would be to reorder)
//
// RUN: FileCheck -input-file=%W_0.asm -check-prefix=ASM %s
// XFAIL: *
//
// ASM: //.kernel_reordering_info_start
// ASM-NEXT: //            id      location         bytes         class          kind
// ASM-NEXT: //         .arg_1            r1             4       surface      explicit
// ASM-NEXT: //         .arg_2          r1+4             4       surface      explicit
// ASM-NEXT: //         .arg_3          r1+8             1       general      explicit
// ASM-NEXT: //         .arg_4         r1+12             4       general      explicit
// ASM-NEXT: //         .arg_5         r1+16             4       general      explicit
// ASM-NEXT: //.kernel_reordering_info_end

// tidy up the generated files
