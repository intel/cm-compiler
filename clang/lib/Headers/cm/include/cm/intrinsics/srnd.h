/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:intrinsics/srnd.h should not be included explicitly");
#endif

#ifndef _CLANG_INTRINSICS_SRND_H_
#define _CLANG_INTRINSICS_SRND_H_

#include <cm/cm_common.h>
#include <cm/cm_has_instr.h>
#include <cm/cm_internal.h>

namespace details {

template <typename OutputTy, typename InputTy, typename BiasTy, unsigned Width>
vector<OutputTy, Width> __cm_intrinsic_impl_srnd(vector<InputTy, Width> Src,
                                                 vector<BiasTy, Width> Bias);

template <typename OutputTy>
using EnableIfHalfType = std::enable_if_t<is_half_type<OutputTy>::value, half>;
} // namespace details

#if CM_HAS_SRND_FP32_TO_FP16
#define CM_HAS_STOCHASTIC_ROUNDING 1
#define CM_HAS_SRND_FP32_TO_FP16_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_SRND_FP32_TO_FP16
#define CM_HAS_SRND_FP32_TO_FP16_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_SRND_FP32_TO_FP16

template <typename ResTy, typename SrcTy, typename BiasTy, unsigned Width>
CM_NODEBUG CM_INLINE vector<details::EnableIfHalfType<ResTy>, Width>
cm_srnd(vector<SrcTy, Width> Src, vector<BiasTy, Width> Bias) {
  CM_HAS_SRND_FP32_TO_FP16_CONTROL;
  CM_STATIC_ERROR(details::is_float_type<SrcTy>::value,
                  "Unsupported stochastic rounding operation");
  CM_STATIC_ERROR(
      (std::is_same<SrcTy, BiasTy>::value ||
       (std::is_integral<BiasTy>::value && sizeof(BiasTy) == sizeof(ResTy))),
      "Unsupported stochastic rounding bias type");

  return details::__cm_intrinsic_impl_srnd<ResTy>(Src, Bias);
}

template <typename ResTy, typename SrcTy, typename BiasTy, unsigned Height,
          unsigned Width>
CM_NODEBUG CM_INLINE matrix<details::EnableIfHalfType<ResTy>, Height, Width>
cm_srnd(matrix<SrcTy, Height, Width> Src, matrix<BiasTy, Height, Width> Bias) {
  return cm_srnd<ResTy>(Src.template format<SrcTy>(),
                        Bias.template format<BiasTy>());
}

template <typename ResTy, typename SrcTy, typename BiasTy>
CM_NODEBUG CM_INLINE details::EnableIfHalfType<ResTy> cm_srnd(SrcTy Src,
                                                              BiasTy Bias) {
  vector<SrcTy, 1> _Src = Src;
  vector<BiasTy, 1> _Bias = Bias;
  vector<ResTy, 1> _Res = cm_srnd<ResTy>(_Src, _Bias);
  return _Res[0];
}

#if CM_HAS_SRND_FP16_TO_BF8
#define CM_HAS_SRND_FP16_TO_BF8_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_SRND_FP16_TO_BF8
#define CM_HAS_SRND_FP16_TO_BF8_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_SRND_FP16_TO_BF8

namespace details {
template <typename SrcTy>
constexpr bool IsValidTypeForSRND = is_one_of_v<SrcTy, half,
#ifdef CM_HAS_BF16
                                                __bf16,
#endif // CM_HAS_BF16
                                                float>;

template <typename OutputTy>
using EnableIfByteType =
    std::enable_if_t<is_one_of_v<OutputTy, char, signed char, unsigned char>,
                     OutputTy>;

template <typename InputTy, unsigned Width>
vector<uint8_t, Width> __cm_intrinsic_impl_bf8_srnd(vector<InputTy, Width> Src,
                                                    vector<uint8_t, Width> Bias);
} // namespace details

template <typename SrcTy, unsigned Width>
CM_NODEBUG CM_INLINE vector<uint8_t, Width>
cm_srnd_bf8(vector<SrcTy, Width> Src, vector<uint8_t, Width> Bias,
            int Flag = _GENX_NOSAT) {
  CM_HAS_SRND_FP16_TO_BF8_CONTROL;

  CM_STATIC_ERROR(details::IsValidTypeForSRND<SrcTy>,
                  "Unsupported stochastic rounding operation");

  vector<uint8_t, Width> Res;

  if constexpr (details::is_float_type<SrcTy>::value) {
    vector<half, Width> _Src = Src;
    Res = details::__cm_intrinsic_impl_bf8_srnd(_Src, Bias);
  } else {
    Res = details::__cm_intrinsic_impl_bf8_srnd(Src, Bias);
  }

  if (Flag != _GENX_NOSAT)
    return details::__cm_intrinsic_impl_sat<uint8_t>(Res);

  return Res;
}

template <typename SrcTy, unsigned Height, unsigned Width>
CM_NODEBUG CM_INLINE matrix<uint8_t, Height, Width>
cm_srnd_bf8(matrix<SrcTy, Height, Width> Src,
            matrix<uint8_t, Height, Width> Bias, int Flag = _GENX_NOSAT) {
  return cm_srnd_bf8(Src.template format<SrcTy>(),
                     Bias.template format<uint8_t>(), Flag);
}

template <typename SrcTy>
CM_NODEBUG CM_INLINE uint8_t cm_srnd_bf8(SrcTy Src, uint8_t Bias,
                                         int Flag = _GENX_NOSAT) {
  vector<SrcTy, 1> _Src = Src;
  vector<uint8_t, 1> _Bias = Bias;
  vector<uint8_t, 1> _Res = cm_srnd_bf8(_Src, _Bias, Flag);
  return _Res[0];
}

template <typename ResTy, typename SrcTy, typename BiasTy, unsigned Width>
CM_DEPRECATED(
    "cm_srnd with byte output type is deprecated, use cm_srnd_bf8 instead")
CM_NODEBUG CM_INLINE vector<details::EnableIfByteType<ResTy>, Width> cm_srnd(
    vector<SrcTy, Width> Src, vector<BiasTy, Width> Bias,
    int Flag = _GENX_NOSAT) {
  CM_HAS_SRND_FP16_TO_BF8_CONTROL;

  CM_STATIC_ERROR(details::IsValidTypeForSRND<SrcTy>,
                  "Unsupported stochastic rounding operation");
  CM_STATIC_ERROR(
      (std::is_same<SrcTy, BiasTy>::value ||
       (std::is_integral<BiasTy>::value && sizeof(BiasTy) == sizeof(ResTy))),
      "Unsupported stochastic rounding bias type");

  vector<ResTy, Width> Res;

  if constexpr (details::is_float_type<SrcTy>::value) {
    constexpr int Stride = Width > 1 ? sizeof(BiasTy) : 1;

    vector<half, Width> _Src = Src;
    vector<ResTy, Width> _Bias =
        Bias.template format<ResTy>().template select<Width, Stride>(0);

    Res = details::__cm_intrinsic_impl_srnd<ResTy>(_Src, _Bias);
  } else {
    Res = details::__cm_intrinsic_impl_srnd<ResTy>(Src, Bias);
  }

  if (Flag != _GENX_NOSAT)
    return details::__cm_intrinsic_impl_sat<ResTy>(Res);

  return Res;
}

template <typename ResTy, typename SrcTy, typename BiasTy, unsigned Height,
          unsigned Width>
CM_DEPRECATED(
    "cm_srnd with byte output type is deprecated, use cm_srnd_bf8 instead")
CM_NODEBUG CM_INLINE matrix<details::EnableIfByteType<ResTy>, Height,
                            Width> cm_srnd(matrix<SrcTy, Height, Width> Src,
                                           matrix<BiasTy, Height, Width> Bias,
                                           int Flag = _GENX_NOSAT) {
  return cm_srnd<ResTy>(Src.template format<SrcTy>(),
                        Bias.template format<BiasTy>(), Flag);
}

template <typename ResTy, typename SrcTy, typename BiasTy>
CM_DEPRECATED(
    "cm_srnd with byte output type is deprecated, use cm_srnd_bf8 instead")
CM_NODEBUG CM_INLINE details::EnableIfByteType<ResTy> cm_srnd(
    SrcTy Src, BiasTy Bias, int Flag = _GENX_NOSAT) {
  vector<SrcTy, 1> _Src = Src;
  vector<BiasTy, 1> _Bias = Bias;
  vector<ResTy, 1> _Res = cm_srnd<ResTy>(_Src, _Bias, Flag);
  return _Res[0];
}

#endif // _CLANG_INTRINSICS_SRND_H_
