/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=skl -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

void foo(__generic void *ptr) {
  // CHECK-DAG: %{{[^ ]+}} = call i8* @_Z42__spirv_GenericCastToPtrExplicit_ToPrivatePU9CLgenericvi(i8 addrspace(4)* %{{[^ ]+}}, i32 7)
  __private void *p_ptr = cm_to_private(ptr);

  // CHECK-DAG: %{{[^ ]+}} = call i8 addrspace(3)* @_Z40__spirv_GenericCastToPtrExplicit_ToLocalPU9CLgenericvi(i8 addrspace(4)* %{{[^ ]+}}, i32 4)
  __local void *l_ptr = cm_to_local(ptr);

  // CHECK-DAG: %{{[^ ]+}} = call i8 addrspace(1)* @_Z41__spirv_GenericCastToPtrExplicit_ToGlobalPU9CLgenericvi(i8 addrspace(4)* %{{[^ ]+}}, i32 5)
  __global void *g_ptr = cm_to_global(ptr);
}
