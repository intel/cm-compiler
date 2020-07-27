// XFAIL: *
// RUN: %cmoc -mcpu=UNKNOWN %w -I%cm_headers -emit-spirv -o output 2>&1 \
// RUN:     | FileCheck %w
// CHECK: unknown target

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

