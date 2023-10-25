/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// Check that -verify option is respected and diagnostic emission is checked.

// RUN: %cmc -emit-llvm -march=SKL -Xclang -verify -- %s

int foo() {} // expected-warning{{non-void function does not return a value}}
