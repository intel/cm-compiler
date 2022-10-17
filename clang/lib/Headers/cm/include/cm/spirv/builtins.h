/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_SPIRV_BUILTINS_H_
#define _CLANG_CM_SPIRV_BUILTINS_H_

// SPIR-V Kernel built-ins are available as function calls.
// See https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#BuiltIn
unsigned int __spirv_BuiltInWorkDim();
unsigned long __spirv_BuiltInGlobalSize(int dim);
unsigned long __spirv_BuiltInGlobalInvocationId(int dim);
unsigned long __spirv_BuiltInWorkgroupSize(int dim);
unsigned long __spirv_BuiltInEnqueuedWorkgroupSize(int dim);
unsigned long __spirv_BuiltInLocalInvocationId(int dim);
unsigned long __spirv_BuiltInNumWorkgroups(int dim);
unsigned long __spirv_BuiltInWorkgroupId(int dim);
unsigned long __spirv_BuiltInGlobalOffset(int dim);
unsigned long __spirv_BuiltInGlobalLinearId();
unsigned long __spirv_BuiltInLocalInvocationIndex();
unsigned int __spirv_BuiltInSubgroupSize();
unsigned int __spirv_BuiltInSubgroupMaxSize();
unsigned int __spirv_BuiltInNumSubgroups();
unsigned int __spirv_BuiltInNumEnqueuedSubgroups();
unsigned int __spirv_BuiltInSubgroupId();
unsigned int __spirv_BuiltInSubgroupLocalInvocationId();

#endif // _CLANG_CM_SPIRV_BUILTINS_H_
