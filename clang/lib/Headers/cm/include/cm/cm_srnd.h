/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_srnd.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_SRND_H_
#define _CLANG_CM_SRND_H_

#include "cm_common.h"
#include "cm_has_instr.h"
#include "cm_internal.h"

template <typename T0, int N, typename T1>
CM_NODEBUG CM_INLINE vector<T0, N>
cm_srnd(vector<T1, N> src1, vector<T1, N> src2) {
  CM_HAS_STOCHASTIC_ROUNDING_CONTROL;

  constexpr bool is_hf16_fp32 =
      details::is_half_type<T0>::value && details::is_float_type<T1>::value;
  CM_STATIC_ERROR(is_hf16_fp32, "unsupported srnd type");

  vector<T0, N> _Result;

  vector<T1, N> _Src1 = src1;
  vector<T1, N> _Src2 = src2;
  _Result = details::__cm_intrinsic_impl_srnd<T0, N, T1>(_Src1, _Src2);

  return _Result;
}

template <typename T0, int N, int M, typename T1>
CM_NODEBUG CM_INLINE matrix<T0, N, M>
cm_srnd(matrix<T1, N, M> src1, matrix<T1, N, M> src2) {
  vector<T1, N *M> _Src1 = src1;
  vector<T1, N *M> _Src2 = src2;

  return cm_srnd<T0>(_Src1, _Src2);
}

template <typename T0, typename T1>
CM_NODEBUG CM_INLINE T0
cm_srnd(T1 src1, T1 src2) {
  vector<T1, 1> _Src1 = src1;
  vector<T1, 1> _Src2 = src2;
  vector<T0, 1> _Res;

  _Res = cm_srnd<T0>(_Src1, _Src2);

  return _Res[0];
}

#endif // _CLANG_CM_SRND_H_
