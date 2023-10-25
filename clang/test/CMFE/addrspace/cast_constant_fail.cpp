/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -march=skl -emit-llvm -o %t.bc -Xclang -verify -- %s

#include <cm/cm.h>

void from_const(__constant void *c_ptr) {
  void *p_ptr = c_ptr;                         // expected-error{{cannot initialize a variable of type 'void *' with an lvalue of type '__constant void *'}}
  __local void *l_ptr = (__local void *)c_ptr; // expected-error{{casting '__constant void *' to type '__local void *' changes address space of pointer}}

  __generic void *ptr = (__generic void *)c_ptr; // expected-error{{casting '__constant void *' to type '__generic void *' changes address space of pointer}}
  ptr = c_ptr;                                   // expected-error{{assigning '__constant void *' to '__generic void *' changes address space of pointer}}
}

void to_const(void *p_ptr, __local void *l_ptr, __generic void *ptr) {
  __constant void *c_ptr;
  c_ptr = (__constant void *)p_ptr; // expected-error{{casting 'void *' to type '__constant void *' changes address space of pointer}}
  c_ptr = (__constant void *)l_ptr; // expected-error{{casting '__local void *' to type '__constant void *' changes address space of pointer}}
  c_ptr = (__constant void *)ptr;   // expected-error{{casting '__generic void *' to type '__constant void *' changes address space of pointer}}
}
