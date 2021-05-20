// XFAIL: *
// RUN: %cmc -mcpu=UNKNOWN %s -emit-spirv -o output 2>&1 \
// RUN:     | FileCheck %s
// CHECK: unknown target

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

