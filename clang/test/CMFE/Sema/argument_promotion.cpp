/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -mcpu=tgllp -S -emit-llvm -o %t.lp.ll -- %s
// RUN: FileCheck %s --input-file %t.lp.ll --check-prefix LP-CHECK

extern "C" void foo(...) {}

extern "C" _GENX_MAIN_ void simple(half hlf, float flt) {
  foo(hlf, flt);
// LP-CHECK-DAG: %[[FLT_EXT:[^ ]+]] = load float
// LP-CHECK-DAG: %[[HLF_EXT:[^ ]+]] = fpext half %{{[^ ]+}} to float
// LP-CHECK: call void (...) @foo(float %[[HLF_EXT]], float %[[FLT_EXT]])
}
