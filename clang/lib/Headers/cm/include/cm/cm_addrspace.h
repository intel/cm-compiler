/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_addrspace.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CM_ADDRSPACE_H_
#define _CM_ADDRSPACE_H_

#include "spirv/builtins.h"

#define __private __attribute__((opencl_private))
#define __global __attribute__((opencl_global))
#define __constant __attribute__((opencl_constant))
#define __local __attribute__((opencl_local))
#define __generic __attribute__((opencl_generic))

template <typename T>
CM_INLINE CM_NODEBUG __private T *cm_to_private(__generic T *ptr) {
  return (__private T *)__spirv_GenericCastToPtrExplicit_ToPrivate(
      ptr, detail::spirv::storage::Function);
}

template <typename T>
CM_INLINE CM_NODEBUG const __private T *cm_to_private(const __generic T *ptr) {
  return (const __private T *)__spirv_GenericCastToPtrExplicit_ToPrivate(
      ptr, detail::spirv::storage::Function);
}

template <typename T>
CM_INLINE CM_NODEBUG __global T *cm_to_global(__generic T *ptr) {
  return (__global T *)__spirv_GenericCastToPtrExplicit_ToGlobal(
      ptr, detail::spirv::storage::CrossWorkgroup);
}

template <typename T>
CM_INLINE CM_NODEBUG const __global T *cm_to_global(const __generic T *ptr) {
  return (const __global T *)__spirv_GenericCastToPtrExplicit_ToGlobal(
      ptr, detail::spirv::storage::CrossWorkgroup);
}

template <typename T>
CM_INLINE CM_NODEBUG __local T *cm_to_local(__generic T *ptr) {
  return (__local T *)__spirv_GenericCastToPtrExplicit_ToLocal(
      ptr, detail::spirv::storage::Workgroup);
}

template <typename T>
CM_INLINE CM_NODEBUG const __local T *cm_to_local(const __generic T *ptr) {
  return (const __local T *)__spirv_GenericCastToPtrExplicit_ToLocal(
      ptr, detail::spirv::storage::Workgroup);
}

#endif /* _CM_ADDRSPACE_H_ */
