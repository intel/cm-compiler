/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -march=dg2 -Xclang -verify -- %s
// RUN: %cmc -emit-llvm -march=pvc -Xclang -verify -- %s

#include <cm/cm.h>

_GENX_MAIN_ void kernel() {
  __bf16 s; // expected-no-diagnostics

  vector<__bf16, 16> v;          // expected-no-diagnostics
  vector_ref<__bf16, 16> vr = v; // expected-no-diagnostics

  matrix<__bf16, 4, 5> m;          // expected-no-diagnostics
  matrix_ref<__bf16, 4, 5> mr = m; // expected-no-diagnostics
}
