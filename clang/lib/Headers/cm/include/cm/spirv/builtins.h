/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2023 Intel Corporation

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

// SPV_INTEL_hw_thread_queries extension
int __spirv_BuiltInSubDeviceIDINTEL();
int __spirv_BuiltInGlobalHWThreadIDINTEL();

namespace detail {
namespace spirv {
enum scope {
  cross_device = 0,
  device = 1,
  workgroup = 2,
  subgroup = 3,
  invocation = 4
};

enum storage {
  UniformConstant = 0,
  Input = 1,
  Uniform = 2,
  Output = 3,
  Workgroup = 4,
  CrossWorkgroup = 5,
  Private = 6,
  Function = 7,
  Generic = 8,
  PushConstant = 9,
  AtomicCounter = 10,
  Image = 11,
  StorageBuffer = 12
};
} // namespace spirv
} // namespace detail

void __spirv_ControlBarrier(int scope, int memory_scope, int memory_semantics);

__attribute__((opencl_private)) void *
__spirv_GenericCastToPtrExplicit_ToPrivate(
    __attribute__((opencl_generic)) void *ptr, int storage);
const __attribute__((opencl_private)) void *
__spirv_GenericCastToPtrExplicit_ToPrivate(
    const __attribute__((opencl_generic)) void *ptr, int storage);

__attribute__((opencl_global)) void *__spirv_GenericCastToPtrExplicit_ToGlobal(
    __attribute__((opencl_generic)) void *ptr, int storage);
const __attribute__((opencl_global)) void *
__spirv_GenericCastToPtrExplicit_ToGlobal(
    const __attribute__((opencl_generic)) void *ptr, int storage);

__attribute__((opencl_local)) void *__spirv_GenericCastToPtrExplicit_ToLocal(
    __attribute__((opencl_generic)) void *ptr, int storage);
const __attribute__((opencl_local)) void *
__spirv_GenericCastToPtrExplicit_ToLocal(
    const __attribute__((opencl_generic)) void *ptr, int storage);

// SPV_INTEL_split_barrier extension
void __spirv_ControlBarrierArriveINTEL(int scope, int memory_scope,
                                       int memory_semantics);
void __spirv_ControlBarrierWaitINTEL(int scope, int memory_scope,
                                     int memory_semantics);

// SPV_INTEL_bfloat16_conversion extension
float __spirv_ConvertBF16ToFINTEL(short);
short __spirv_ConvertFToBF16INTEL(float);

template <int Width>
vector<float, Width> __spirv_ConvertBF16ToFINTEL(vector<short, Width>);
template <int Width>
vector<short, Width> __spirv_ConvertFToBF16INTEL(vector<float, Width>);

#endif // _CLANG_CM_SPIRV_BUILTINS_H_
