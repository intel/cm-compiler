/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_printfocl.h should not be included explicitly - only "
               "<cm/cm.h> is required");
#endif

#ifndef _CM_PRINTFOCL_H_
#define _CM_PRINTFOCL_H_

#include "cm_common.h"

#ifdef __CM_OCL_RUNTIME

// Results into 'OpExtInst printf' instruction in SPIR-V.
int __spirv_ocl_printf(const char *, ...);

template <typename... Targs> __CM_PRINT_INLINE_WA auto printf(Targs... Fargs) {
  return __spirv_ocl_printf(Fargs...);
}

template <typename... Targs>
__CM_PRINT_INLINE_WA auto cm_printf(Targs... Fargs) {
  return __spirv_ocl_printf(Fargs...);
}

template <typename... Targs> __CM_PRINT_INLINE_WA auto cmprint(Targs... Fargs) {
  return __spirv_ocl_printf(Fargs...);
}

#endif // __CM_OCL_RUNTIME
#endif // _CM_PRINTFOCL_H_
