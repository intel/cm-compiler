/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_SPIRV_CONVERSION_H_
#define _CLANG_CM_SPIRV_CONVERSION_H_

// SPIR-V conversion instructions:
// https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#_conversion_instructions

namespace detail {
namespace spirv {
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

#endif // _CLANG_CM_SPIRV_CONVERSION_H_
