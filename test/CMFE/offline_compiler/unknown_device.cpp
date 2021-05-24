// XFAIL: *
// RUN: %cmc -mcpu=UNKNOWN -emit-spirv -o output -- %s 2>&1 \
// RUN:     | FileCheck %s
// CHECK: unknown target

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

