/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -g0 -emit-llvm -S -o %t.ll           -- %s
// RUN: %cmc -march=SKL -g0 -emit-spirv   -o %t.from_ir.spv  -- %t.ll
// RUN: %cmc -march=SKL -g0 -emit-spirv   -o %t.from_src.spv -- %s
// RUN: cmp %t.from_ir.spv %t.from_src.spv

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {}
