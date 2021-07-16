/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// XFAIL: *

// RUN: %cmc -mcpu=SKL -o output -- %s 2>&1 \
// RUN:     | FileCheck %s
// RUN: %cmc -mcpu=SKL -o output -- %s 2>&1 \
// RUN:     | FileCheck %s
// CHECK: not implemented

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

