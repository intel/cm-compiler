/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -w -march=pvc -emit-llvm -S -Xclang -verify -o %t.ll -- %s
// RUN: FileCheck --input-file=%t.ll %s

// CHECK-LABEL: test_static_cast_from_float
__bf16 test_static_cast_from_float(float in) {
  // CHECK: fptrunc float %{{[^ ]+}} to bfloat
  return static_cast<__bf16>(in); // expected-no-diagnostics
}

// CHECK-LABEL: test_static_cast_from_half
__bf16 test_static_cast_from_half(half in) {
  // CHECK: [[EXT:%[^ ]+]] = fpext half %{{[^ ]+}} to float
  // CHECK: [[TRUNC:%[^ ]+]] = fptrunc float [[EXT]] to bfloat
  return static_cast<__bf16>(in); // expected-no-diagnostics
}

// CHECK-LABEL: test_static_cast_from_float_literal
__bf16 test_static_cast_from_float_literal(void) {
  // CHECK: ret bfloat 0xR3F80
  return static_cast<__bf16>(1.0f); // expected-no-diagnostics
}

// CHECK-LABEL: test_static_cast_from_int
__bf16 test_static_cast_from_int(int in) {
  // CHECK: sitofp i32 %{{[^ ]+}} to bfloat
  return static_cast<__bf16>(in); // expected-no-diagnostics
}

// CHECK-LABEL: test_static_cast_from_int_literal
__bf16 test_static_cast_from_int_literal(void) {
  // CHECK: ret bfloat 0xR3F80
  return static_cast<__bf16>(1); // expected-no-diagnostics
}

__bf16 test_static_cast_bfloat(__bf16 in) {
  return static_cast<__bf16>(in); // expected-no-diagnostics
}

// CHECK-LABEL: test_static_cast_to_float
float test_static_cast_to_float(__bf16 in) {
  // CHECK: fpext bfloat %{{[^ ]+}} to float
  return static_cast<float>(in); // expected-no-diagnostics
}

// CHECK-LABEL: test_static_cast_to_half
half test_static_cast_to_half(__bf16 in) {
  // CHECK: [[EXT:%[^ ]+]] = fpext bfloat %{{[^ ]+}} to float
  // CHECK: [[TRUNC:%[^ ]+]] = fptrunc float [[EXT]] to half
  return static_cast<half>(in); // expected-no-diagnostics
}

// CHECK-LABEL: test_static_cast_to_int
int test_static_cast_to_int(__bf16 in) {
  // CHECK: fptosi bfloat %{{[^ ]+}} to i32
  return static_cast<int>(in); // expected-no-diagnostics
}

// CHECK-LABEL: test_implicit_from_float
__bf16 test_implicit_from_float(float in) {
  // CHECK: fptrunc float %{{[^ ]+}} to bfloat
  return in; // expected-no-diagnostics
}

// CHECK-LABEL: test_implicit_from_half
__bf16 test_implicit_from_half(half in) {
  // CHECK: [[EXT:%[^ ]+]] = fpext half %{{[^ ]+}} to float
  // CHECK: [[TRUNC:%[^ ]+]] = fptrunc float [[EXT]] to bfloat
  return in; // expected-no-diagnostics
}

// CHECK-LABEL: test_implicit_from_float_literal
__bf16 test_implicit_from_float_literal(void) {
  // CHECK: ret bfloat 0xR3F80
  return 1.0f; // expected-no-diagnostics
}

// CHECK-LABEL: test_implicit_from_int
__bf16 test_implicit_from_int(int in) {
  // CHECK: sitofp i32 %{{[^ ]+}} to bfloat
  return in; // expected-no-diagnostics
}

// CHECK-LABEL: test_implicit_from_int_literal
__bf16 test_implicit_from_int_literal(void) {
  // CHECK: ret bfloat 0xR3F80
  return 1; // expected-no-diagnostics
}

__bf16 test_implicit_bfloat(__bf16 in) {
  return in; // expected-no-diagnostics
}

// CHECK-LABEL: test_implicit_to_float
float test_implicit_to_float(__bf16 in) {
  // CHECK: fpext bfloat %{{[^ ]+}} to float
  return in; // expected-no-diagnostics
}

// CHECK-LABEL: test_implicit_to_half
half test_implicit_to_half(__bf16 in) {
  // CHECK: [[EXT:%[^ ]+]] = fpext bfloat %{{[^ ]+}} to float
  // CHECK: [[TRUNC:%[^ ]+]] = fptrunc float [[EXT]] to half
  return in; // expected-no-diagnostics
}

// CHECK-LABEL: test_implicit_to_int
int test_implicit_to_int(__bf16 in) {
  // CHECK: fptosi bfloat %{{[^ ]+}} to i32
  return in; // expected-no-diagnostics
}

__bf16 test_cond(__bf16 a, __bf16 b, bool which) {
  // Conditional operator _should_ be supported, without nonsense
  // complaints like 'types __bf16 and __bf16 are not compatible'
  return which ? a : b; // expected-no-diagnostics
}

__bf16 test_cond_float(__bf16 a, __bf16 b, bool which) {
  return which ? a : 1.0f; // expected-no-diagnostics
}

__bf16 test_cond_int(__bf16 a, __bf16 b, bool which) {
  return which ? a : 1; // expected-no-diagnostics
}
