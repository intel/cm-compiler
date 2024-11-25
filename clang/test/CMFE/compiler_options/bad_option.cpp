/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// XFAIL: *

// RUN: %cmc -emit-llvm -bad-option -- %s 2>&1 | FileCheck %s

_GENX_MAIN_
void test() {
}

// CHECK: cmc: error: unsupported option '-b ad-option'
