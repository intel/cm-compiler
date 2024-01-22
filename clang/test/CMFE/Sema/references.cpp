/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=tgllp -Xclang -verify -emit-llvm -- %s

#include <cm/cm.h>

_GENX_MAIN_ void kernel_lref_arg(int &) {}  // expected-error{{unsupported function parameter type 'int &'}}
_GENX_MAIN_ void kernel_rref_arg(int &&) {} // expected-error{{unsupported function parameter type 'int &&'}}

_GENX_ void sub_lref_arg(int &) {}  // ok
_GENX_ void sub_rref_arg(int &&) {} // ok

_GENX_MAIN_ void kernel_ref() {
  int x;
  int &xlref = x;                       // ok
  int &&xrref = static_cast<int &&>(x); // ok
}
