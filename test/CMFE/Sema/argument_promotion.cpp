/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -mcpu=skl -S -emit-llvm -o %t.skl.ll -- %s
// RUN: FileCheck %s --input-file %t.skl.ll --check-prefix SKL-CHECK
// RUN: %cmc -g0 -mcpu=tgllp -S -emit-llvm -o %t.lp.ll -- %s
// RUN: FileCheck %s --input-file %t.lp.ll --check-prefix LP-CHECK

#include <cm/cm.h>

extern "C" void foo(...) {}

extern "C" _GENX_MAIN_ void simple(half hlf, float flt) {
  foo(hlf, flt);
// SKL-CHECK-DAG: %[[FLT_EXT:[^ ]+]] = fpext float %{{[^ ]+}} to double
// SKL-CHECK-DAG: %[[HLF_EXT:[^ ]+]] = fpext half %{{[^ ]+}} to double
// SKL-CHECK: call void (...) @foo(double %[[HLF_EXT]], double %[[FLT_EXT]])

// LP-CHECK-DAG: %[[FLT_EXT:[^ ]+]] = load float
// LP-CHECK-DAG: %[[HLF_EXT:[^ ]+]] = fpext half %{{[^ ]+}} to float
// LP-CHECK: call void (...) @foo(float %[[HLF_EXT]], float %[[FLT_EXT]])
}
