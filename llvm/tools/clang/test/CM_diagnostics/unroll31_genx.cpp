// This test originally formed part of the CM unroll test suite, but is 
// intended to check that the compiler notices a static out-of-bounds
// access within an unrolled loop.
// The icl-cm compiler generated a warning for this, but cmc generates
// an error - so the test has been move to the CM diagnostics suite.
//
// RUN: %cmc %w 2>&1 | FileCheck %w

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
