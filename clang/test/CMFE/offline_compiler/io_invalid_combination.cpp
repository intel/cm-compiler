/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=pvc -emit-spirv   -o %t.spv -mcpu=pvc -- %s
// RUN: %cmc -march=pvc -emit-llvm    -o %t.bc -mcpu=pvc -- %s
// RUN: %cmc -march=pvc -emit-llvm -S -o %t.ll -mcpu=pvc -- %s

// RUN: %cmc -march=pvc -emit-spirv -o output 2>&1 -- %t.spv \
// RUN:         | FileCheck %s
//
// RUN: %cmc -march=pvc -emit-llvm -o output 2>&1 -- %t.bc \
// RUN:         | FileCheck %s

// RUN: %cmc -march=pvc -emit-llvm -S -o output 2>&1 -- %t.bc \
// RUN:         | FileCheck %s
//

// CHECK: not supported

extern "C" _GENX_MAIN_
void test_kernel() {
}

