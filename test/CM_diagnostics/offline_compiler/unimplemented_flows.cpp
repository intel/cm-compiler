// RUN: %cmc -mcpu=SKL %s -o output 2>&1 \
// RUN:     | FileCheck %s
// RUN: %cmc -mcpu=SKL %s -o output 2>&1 \
// RUN:     | FileCheck %s
// CHECK: not implemented

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

