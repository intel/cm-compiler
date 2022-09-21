/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_cvt.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_CVT_H_
#define _CLANG_CM_CVT_H_

#include "cm_common.h"
#include "cm_internal.h"
#include "cm_traits.h"
#include "cm_has_instr.h"


// float32 to tf32 convertion one direction
template <typename T, typename T0, int N>
CM_NODEBUG CM_INLINE vector<T, N> cm_tf32_cvt(vector<T0, N> src0) {
  CM_STATIC_ERROR(
      (std::is_same<int, typename std::remove_const<T>::type>::value &&
       std::is_same<float, typename std::remove_const<T0>::type>::value),
      "Invalid type for cm_tf32_fp32_cvt: src->dst must be float->int");
  CM_HAS_TF32_CONTROL;

  return details::__cm_intrinsic_impl_tf32_cvt<T>(src0);
}

template <typename T, typename T0, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T, N1 * N2>
cm_tf32_cvt(matrix<T0, N1, N2> src) {
  CM_HAS_TF32_CONTROL;

  vector<T0, N1 *N2> _Src = src;
  return cm_tf32_cvt<T>(_Src);
}

template <typename T, typename T0>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_cm_scalar<T>::value &&
                                details::is_cm_scalar<T0>::value,
                            typename std::remove_const<T>::type>::type
    cm_tf32_cvt(T0 src) {
  CM_HAS_TF32_CONTROL;

  vector<T0, 1> _Src = src;
  vector<T, 1> _Result = cm_tf32_cvt<T>(_Src);
  return _Result(0);
}

#endif // _CLANG_CM_CVT_H_
