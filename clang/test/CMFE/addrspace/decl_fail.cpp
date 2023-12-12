/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -march=skl -emit-llvm -o %t.bc -Xclang -verify -- %s

#include <cm/cm.h>

__local uint32_t LocalGV;       // expected-error{{program scope variable must reside in global, constant or private address space}}
__constant uint32_t ConstantGV; // expected-error{{variable in constant address space must be initialized}}

void foo() {
  static __local uint32_t LocalInternal; // expected-error{{static local variable must reside in global, constant or private address space}}
  __local uint32_t LocalAutomatic;       // expected-error{{non-kernel function variable cannot be declared in local address space}}

  __global float GlobalAutomatic;   // expected-error{{function scope variable cannot be declared in global address space}}
  __generic float GenericAutomatic; // expected-error{{automatic variable qualified with an invalid address space}}
}

_GENX_MAIN_ void kernel(uint32_t *svmptr [[type("svmptr_t")]]) {
  __local uint32_t LocalInit = 5; // expected-error{{'__local' variable cannot have an initializer}}
  if (*svmptr) {
    __local uint32_t LocalAutomatic; // expected-error{{variables in the local address space can only be declared in the outermost scope of a kernel function}}
  }
}
