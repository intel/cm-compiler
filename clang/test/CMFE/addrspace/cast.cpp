/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=skl -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

void to_generic(__private void *p_ptr, __local void *l_ptr, __global void *g_ptr) {
  __generic void *ptr;

  // CHECK: %{{[^ ]+}} = addrspacecast i8* %{{[^ ]+}} to i8 addrspace(4)*
  ptr = p_ptr;

  // CHECK: %{{[^ ]+}} = addrspacecast i8 addrspace(3)* %{{[^ ]+}} to i8 addrspace(4)*
  ptr = l_ptr;

  // CHECK: %{{[^ ]+}} = addrspacecast i8 addrspace(1)* %{{[^ ]+}} to i8 addrspace(4)*
  ptr = g_ptr;
}

void from_generic(__generic void *ptr) {
  // CHECK: %{{[^ ]+}} = addrspacecast i8 addrspace(4)* %{{[^ ]+}} to i8*
  __private void *p_ptr = (__private void *)ptr;

  // CHECK: %{{[^ ]+}} = addrspacecast i8 addrspace(4)* %{{[^ ]+}} to i8 addrspace(3)*
  __local void *l_ptr = (__local void *)ptr;

  // CHECK: %{{[^ ]+}} = addrspacecast i8 addrspace(4)* %{{[^ ]+}} to i8 addrspace(1)*
  __global void *g_ptr = (__global void *)ptr;
}
