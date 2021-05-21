// RUN: %cmc %s -emit-spirv   -o %t.spv -mcpu=SKL
// RUN: %cmc %s -emit-llvm    -o %t.bc -mcpu=SKL
// RUN: %cmc %s -emit-llvm -S -o %t.ll -mcpu=SKL

// RUN: %cmc %t.spv -emit-spirv -o output 2>&1 \
// RUN:         | FileCheck %s
//
// RUN: %cmc %t.bc -emit-llvm -o output 2>&1 \
// RUN:         | FileCheck %s

// RUN: %cmc %t.bc -emit-llvm -S -o output 2>&1 \
// RUN:         | FileCheck %s
//

// CHECK: not supported


#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

