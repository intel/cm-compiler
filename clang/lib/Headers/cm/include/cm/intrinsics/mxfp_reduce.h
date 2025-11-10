/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:intrinsics/mxfp_reduce.h should not be included"
                 "explicitly");
#endif

#ifndef _CLANG_CM_INTRINSICS_MXFP_REDUCE_H_
#define _CLANG_CM_INTRINSICS_MXFP_REDUCE_H_

#include <cm/cm_common.h>
#include <cm/cm_has_instr.h>

// MXFP reduction is supported on the platforms which also support BDPAS.
#ifdef CM_HAS_BDPAS
#define CM_HAS_MXFP_REDUCE 1
#define CM_HAS_MXFP_REDUCE_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_BDPAS
#define CM_HAS_MXFP_REDUCE_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_BDPAS

namespace details {
template <typename T>
vector<T, 32> __cm_intrinsic_impl_mxfp_reduce(matrix<T, 32, 32> Src);

template <typename T>
vector<T, 32> __cm_intrinsic_impl_mxfp_linearize(vector<T, 32> Src);
} // namespace details

template <typename T>
CM_NODEBUG CM_INLINE vector<T, 32> cm_mxfp_reduce(matrix<T, 32, 32> Src) {
  CM_HAS_MXFP_REDUCE_CONTROL;
  CM_STATIC_ERROR(sizeof(T) == 2,
                  "mxfp_reduce only supports 16-bit data types");
  return details::__cm_intrinsic_impl_mxfp_reduce(Src);
}

template <typename T>
CM_NODEBUG CM_INLINE vector<T, 32> cm_mxfp_linearize(vector<T, 32> Src) {
  CM_HAS_MXFP_REDUCE_CONTROL;
  CM_STATIC_ERROR(sizeof(T) == 2,
                  "mxfp_linearize only supports 16-bit data types");
  return details::__cm_intrinsic_impl_mxfp_linearize(Src);
}

#endif // _CLANG_CM_INTRINSICS_MXFP_REDUCE_H_
