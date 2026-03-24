/*========================== begin_copyright_notice ============================

Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

_GENX_MAIN_
void test(svmptr_t buf [[type("svmptr_t")]]) {
}
// check that no warning is generated
// RUN: %cmc -emit-llvm -march=bmg -- %s 2>&1 | count 0
