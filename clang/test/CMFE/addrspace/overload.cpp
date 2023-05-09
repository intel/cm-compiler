/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -fcm-pointer -march=skl -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

void foo(void *ptr);
void foo(__private void *ptr);
void foo(__global void *ptr);
void foo(__local void *ptr);
void foo(__constant void *ptr);
void foo(__generic void *ptr);

void bar() {
  void *d_ptr = nullptr;
  __private void *p_ptr = nullptr;
  __global void *g_ptr = nullptr;
  __local void *l_ptr = nullptr;
  __constant void *c_ptr = nullptr;
  __generic void *ptr = nullptr;

  // CHECK: call void @_Z3fooPv(i8* %{{[^ ]+}})
  foo(d_ptr);

  // CHECK: call void @_Z3fooPU9CLprivatev(i8* %{{[^ ]+}})
  foo(p_ptr);

  // CHECK: call void @_Z3fooPU8CLglobalv(i8 addrspace(1)* %{{[^ ]+}})
  foo(g_ptr);

  // CHECK: call void @_Z3fooPU7CLlocalv(i8 addrspace(3)* %{{[^ ]+}})
  foo(l_ptr);

  // CHECK: call void @_Z3fooPU10CLconstantv(i8 addrspace(2)* %{{[^ ]+}})
  foo(c_ptr);

  // CHECK: call void @_Z3fooPU9CLgenericv(i8 addrspace(4)* %{{[^ ]+}})
  foo(ptr);
}
