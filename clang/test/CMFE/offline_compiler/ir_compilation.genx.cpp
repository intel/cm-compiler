/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=pvc -g0 -emit-llvm -S -o %t.ll           -- %s
// RUN: %cmc -march=pvc -g0 -emit-spirv   -o %t.from_ir.spv  -- %t.ll
// RUN: %cmc -march=pvc -g0 -emit-spirv   -o %t.from_src.spv -- %s
// RUN: cmp %t.from_ir.spv %t.from_src.spv

extern "C" _GENX_MAIN_
void test_kernel() {}
