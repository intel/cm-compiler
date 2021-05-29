// XFAIL: *

// RUN: %cmc -mcpu=SKL -o output -- %s 2>&1 \
// RUN:     | FileCheck %s
// RUN: %cmc -mcpu=SKL -o output -- %s 2>&1 \
// RUN:     | FileCheck %s
// CHECK: not implemented

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

