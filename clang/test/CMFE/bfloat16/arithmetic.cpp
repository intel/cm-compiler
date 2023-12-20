/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -w -march=pvc -emit-llvm -S -Xclang -verify -- %s

void test(bool b) {
  __bf16 bf16;

  bf16 + bf16; // expected-no-diagnostics
  bf16 - bf16; // expected-no-diagnostics
  bf16 * bf16; // expected-no-diagnostics
  bf16 / bf16; // expected-no-diagnostics

  half fp16;

  bf16 + fp16; // expected-no-diagnostics
  fp16 + bf16; // expected-no-diagnostics
  bf16 - fp16; // expected-no-diagnostics
  fp16 - bf16; // expected-no-diagnostics
  bf16 * fp16; // expected-no-diagnostics
  fp16 * bf16; // expected-no-diagnostics
  bf16 / fp16; // expected-no-diagnostics
  fp16 / bf16; // expected-no-diagnostics
  bf16 = fp16; // expected-no-diagnostics
  fp16 = bf16; // expected-no-diagnostics
  bf16 + (b ? fp16 : bf16); // expected-no-diagnostics
}
