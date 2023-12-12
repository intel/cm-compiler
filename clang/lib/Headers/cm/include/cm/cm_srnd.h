/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2023 Intel Corporation

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

template <typename ResTy, typename SrcTy, typename BiasTy, int Width>
CM_NODEBUG CM_INLINE vector<ResTy, Width> cm_srnd(vector<SrcTy, Width> src,
                                                  vector<BiasTy, Width> bias) {
  CM_HAS_STOCHASTIC_ROUNDING_CONTROL;

  constexpr bool is_hf16_fp32 = details::is_half_type<ResTy>::value &&
                                details::is_float_type<SrcTy>::value;
  CM_STATIC_ERROR(is_hf16_fp32, "Unsupported stochastic rounding operation");

  constexpr bool is_bias_type_valid =
      std::is_same<SrcTy, BiasTy>::value ||
      (std::is_integral<BiasTy>::value && sizeof(BiasTy) == sizeof(ResTy));
  CM_STATIC_ERROR(is_bias_type_valid,
                  "Unsupported stochastic rounding bias type");

vector<ResTy, Width> _res;

  _res = details::__cm_intrinsic_impl_srnd<ResTy>(src, bias);

  return _res;
}

template <typename ResTy, typename SrcTy, typename BiasTy, int Height,
          int Width>
CM_NODEBUG CM_INLINE matrix<ResTy, Height, Width>
cm_srnd(matrix<SrcTy, Height, Width> src,
        matrix<BiasTy, Height, Width> bias) {
  vector<SrcTy, Height * Width> _src = src;
  vector<BiasTy, Height * Width> _bias = bias;

  return cm_srnd<ResTy>(_src, _bias);
}

template <typename ResTy, typename SrcTy, typename BiasTy>
CM_NODEBUG CM_INLINE ResTy
cm_srnd(SrcTy src, BiasTy bias) {
  vector<SrcTy, 1> _src = src;
  vector<BiasTy, 1> _bias = bias;

  vector<ResTy, 1> _res = cm_srnd<ResTy>(_src, _bias);

  return _res[0];
}

#endif // _CLANG_CM_SRND_H_
