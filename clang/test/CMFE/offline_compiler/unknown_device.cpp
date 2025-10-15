/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-spirv -o output -- %s 2>&1 | FileCheck %s
// CHECK: error: invalid arch name 'unknown'

extern "C" _GENX_MAIN_
void test_kernel() {
}

