// RUN: %cmoc %w -I%cm_headers -emit-spirv   -o %W.spv -mcpu=SKL
// RUN: %cmoc %w -I%cm_headers -emit-llvm    -o %W.bc -mcpu=SKL
// RUN: %cmoc %w -I%cm_headers -emit-llvm -S -o %W.ll -mcpu=SKL

// RUN: %cmoc %W.spv -I%cm_headers -emit-spirv -o output 2>&1 \
// RUN:         | FileCheck %w
//
// RUN: %cmoc %W.bc -I%cm_headers -emit-llvm -o output 2>&1 \
// RUN:         | FileCheck %w

// RUN: %cmoc %W.bc -I%cm_headers -emit-llvm -S -o output 2>&1 \
// RUN:         | FileCheck %w
//
// RUN: rm %W.spv
// RUN: rm %W.bc
// RUN: rm %W.ll

// CHECK: not supported


#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

