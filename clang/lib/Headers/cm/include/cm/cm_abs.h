/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_abs.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef CM_ABS_H
#define CM_ABS_H

#include "cm_common.h"
#include "cm_has_instr.h"
#include "cm_internal.h"
#include "cm_traits.h"

namespace details {

template <typename T0, typename T1, int SZ>
CM_NODEBUG CM_INLINE vector<T0, SZ>
__cm_abs_common_internal(vector<T1, SZ> src0, int flag = _GENX_NOSAT) {
  using T1U =
      typename std::conditional<std::is_integral<T1>::value,
                                typename make_unsigned<T1>::type, T1>::type;

  vector<T1U, SZ> _Result =
      std::is_unsigned<T1>::value ? src0 : __cm_intrinsic_impl_abs(src0);
  if (flag != _GENX_SAT)
    return _Result;

  return __cm_intrinsic_impl_sat<T0>(_Result);
}

template <typename T0, typename T1, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2>
__cm_abs_common_internal(matrix<T1, N1, N2> src0, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return __cm_abs_common_internal<T0>(_Src0, flag);
}

template <typename T0, typename T1>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value,
    typename std::remove_const<T0>::type>::type
__cm_abs_common_internal(T1 src0, int flag = _GENX_NOSAT) {
  typedef typename std::remove_const<T0>::type _T0;
  typedef typename std::remove_const<T1>::type _T1;

  vector<_T1, 1> _Src0 = src0;
  vector<_T0, 1> _Result = __cm_abs_common_internal<_T0>(_Src0, flag);
  return _Result(0);
}
}

template <typename T0, typename T1, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    !std::is_same<typename std::remove_const<T0>::type,
                  typename std::remove_const<T1>::type>::value,
    vector<T0, SZ> >::type
cm_abs(vector<T1, SZ> src0, int flag = _GENX_NOSAT) {
  return details::__cm_abs_common_internal<T0, T1, SZ>(src0, flag);
}

template <typename T0, typename T1, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    !std::is_same<typename std::remove_const<T0>::type,
                  typename std::remove_const<T1>::type>::value,
    vector<T0, N1 *N2> >::type
cm_abs(matrix<T1, N1, N2> src0, int flag = _GENX_NOSAT) {
  return details::__cm_abs_common_internal<T0, T1, N1, N2>(src0, flag);
}

template <typename T0, typename T1>
CM_NODEBUG CM_INLINE typename std::enable_if<
    !std::is_same<typename std::remove_const<T0>::type,
                  typename std::remove_const<T1>::type>::value &&
        details::is_cm_scalar<T0>::value && details::is_cm_scalar<T1>::value,
    typename std::remove_const<T0>::type>::type
cm_abs(T1 src0, int flag = _GENX_NOSAT) {
  return details::__cm_abs_common_internal<T0, T1>(src0, flag);
}

template <typename T1, int SZ>
CM_NODEBUG CM_INLINE vector<T1, SZ> cm_abs(vector<T1, SZ> src0,
                                           int flag = _GENX_NOSAT) {
  return details::__cm_abs_common_internal<T1, T1, SZ>(src0, flag);
}

template <typename T1, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T1, N1 *N2> cm_abs(matrix<T1, N1, N2> src0,
                                               int flag = _GENX_NOSAT) {
  return details::__cm_abs_common_internal<T1, T1, N1, N2>(src0, flag);
}

template <typename T1>
CM_NODEBUG
CM_INLINE typename std::enable_if<details::is_cm_scalar<T1>::value,
                                  typename std::remove_const<T1>::type>::type
cm_abs(T1 src0, int flag = _GENX_NOSAT) {
  return details::__cm_abs_common_internal<T1, T1>(src0, flag);
}

#endif // CM_ABS_H
