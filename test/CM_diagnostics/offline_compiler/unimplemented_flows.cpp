// RUN: %cmoc -mcpu=SKL %w -I%cm_headers -o output 2>&1 \
// RUN:     | FileCheck %w
// RUN: %cmoc -mcpu=SKL %w -I%cm_headers -o output 2>&1 \
// RUN:     | FileCheck %w
// CHECK: not implemented

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

