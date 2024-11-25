/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// XFAIL: *
// RUN: %cmc -mcpu=UNKNOWN -emit-spirv -o output -- %s 2>&1 \
// RUN:     | FileCheck %s
// CHECK: unknown target

extern "C" _GENX_MAIN_
void test_kernel() {
}

