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

// This test originally formed part of the CM unroll test suite, but is 
// intended to check that the compiler notices a static out-of-bounds
// access within an unrolled loop.
// The icl-cm compiler generated a warning for this, but cmc generates
// an error - so the test has been move to the CM diagnostics suite.
//
// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck %s

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test(SurfaceIndex pInputIndex,SurfaceIndex pOutputIndex ) {
	vector<int, 8> v;
    read(pInputIndex, 0, 0, v );

	#pragma unroll
	for (short x = 0; x < 128; x++) {
		if (x == 4) break;
		v(x) = x + 1;
    v(11) = 101;   // out of bounds
	}

	write(pOutputIndex, 0, 0, v );
}

// CHECK: error: vector element access out-of-bound [0, 8)
// CHECK: 1 error generated.
