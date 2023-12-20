/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -march=skl -Xclang -verify -- %s
// RUN: %cmc -emit-llvm -march=icllp -Xclang -verify -- %s
// RUN: %cmc -emit-llvm -march=tgllp -Xclang -verify -- %s
// RUN: %cmc -emit-llvm -march=mtl -Xclang -verify -- %s

#include <cm/cm.h>

_GENX_MAIN_ void kernel() {
  __bf16 s; // expected-error{{__bf16 is not supported on this target}}

  vector<__bf16, 16> v;          // expected-error{{__bf16 is not supported on this target}}
  vector_ref<__bf16, 16> vr = v; // expected-error{{__bf16 is not supported on this target}}

  matrix<__bf16, 4, 5> m;          // expected-error{{__bf16 is not supported on this target}}
  matrix_ref<__bf16, 4, 5> mr = m; // expected-error{{__bf16 is not supported on this target}}
}
