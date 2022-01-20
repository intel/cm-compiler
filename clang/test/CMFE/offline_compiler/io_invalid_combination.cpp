/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -emit-spirv   -o %t.spv -mcpu=SKL -- %s
// RUN: %cmc -march=SKL -emit-llvm    -o %t.bc -mcpu=SKL -- %s
// RUN: %cmc -march=SKL -emit-llvm -S -o %t.ll -mcpu=SKL -- %s

// RUN: %cmc -march=SKL -emit-spirv -o output 2>&1 -- %t.spv \
// RUN:         | FileCheck %s
//
// RUN: %cmc -march=SKL -emit-llvm -o output 2>&1 -- %t.bc \
// RUN:         | FileCheck %s

// RUN: %cmc -march=SKL -emit-llvm -S -o output 2>&1 -- %t.bc \
// RUN:         | FileCheck %s
//

// CHECK: not supported


#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

