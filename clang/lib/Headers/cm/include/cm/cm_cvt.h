/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2023 Intel Corporation

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

#include "spirv/extensions/intel/bfloat16_conversion.h"

template <typename T, typename T0, int N>
CM_NODEBUG CM_INLINE vector<T, N> cm_qf_cvt(vector<T0, N> src0,
                                            int flag = _GENX_NOSAT) {
  //Check: platform has bf8 type.
  //qf is an alias of bf8.
  CM_HAS_BF8_CONTROL;

  constexpr bool is_bf8_hf16 =
      details::is_one_of_v<T, uchar> && details::is_one_of_v<T0, half>;
  constexpr bool is_hf16_bf8 =
      details::is_one_of_v<T, half> && details::is_one_of_v<T0, uchar>;
  constexpr bool emu_bf8_fp32 =
      details::is_one_of_v<T, uchar> && details::is_one_of_v<T0, float>;
  constexpr bool emu_fp32_bf8 =
      details::is_one_of_v<T, float> && details::is_one_of_v<T0, uchar>;

  CM_STATIC_ERROR((is_bf8_hf16 || is_hf16_bf8 || emu_bf8_fp32 || emu_fp32_bf8),
                  "unssupported cm_qf_cvt type: "
                  "src->dst must be uchar->half, "
                  "half->uchar for straight conversion or "
                  "float->uchar, "
                  "uchar->float for emulation");

  vector<T, N> _Result;

  // if emulation, convert fp32->hf or hf->fp32
  if constexpr (emu_bf8_fp32) {
    vector<half, N> _Src0 = src0;
    _Result = details::__cm_intrinsic_impl_qf_cvt<T>(_Src0);
  } else if constexpr (emu_fp32_bf8) {
    vector<T0, N> _Src0 = src0;
    vector<half, N> res0 = details::__cm_intrinsic_impl_qf_cvt<half>(_Src0);
    _Result = res0;
  } else {
    vector<T0, N> _Src0 = src0;
    _Result = details::__cm_intrinsic_impl_qf_cvt<T>(_Src0);
  }

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <typename T, typename T0, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T, N1 * N2> cm_qf_cvt(matrix<T0, N1, N2> src,
                                                  int flag = _GENX_NOSAT) {
  vector<T0, N1 *N2> _Src = src;
  return cm_qf_cvt<T>(_Src, flag);
}

template <typename T, typename T0>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_cm_scalar<T>::value &&
                                details::is_cm_scalar<T0>::value,
                            typename std::remove_const<T>::type>::type
    cm_qf_cvt(T0 src, int flag = _GENX_NOSAT) {
  CM_HAS_BF8_CONTROL;

  vector<T0, 1> _Src = src;
  vector<T, 1> _Result = cm_qf_cvt<T>(_Src, flag);
  return _Result(0);
}

// convert from float32 to bfloat8 one direction
// as qf and bf8 are the same type, cast is alias to exist cast
template <typename T, typename T0, int N>
CM_NODEBUG CM_INLINE vector<T, N> cm_bf8_cvt(vector<T0, N> src0,
                                             int flag = _GENX_NOSAT) {
  CM_HAS_BF8_CONTROL;

  return cm_qf_cvt<T, T0, N>(src0, flag);
}

template <typename T, typename T0, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T, N1 * N2> cm_bf8_cvt(matrix<T0, N1, N2> src,
                                                   int flag = _GENX_NOSAT) {
  CM_HAS_BF8_CONTROL;

  vector<T0, N1 *N2> _Src = src;
  return cm_qf_cvt<T>(_Src, flag);
}

template <typename T, typename T0>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_cm_scalar<T>::value &&
                                details::is_cm_scalar<T0>::value,
                            typename std::remove_const<T>::type>::type
    cm_bf8_cvt(T0 src, int flag = _GENX_NOSAT) {
  CM_HAS_BF8_CONTROL;

  vector<T0, 1> _Src = src;
  vector<T, 1> _Result = cm_qf_cvt<T>(_Src, flag);
  return _Result(0);
}

// hfloat8 cast operations
template <typename T, typename T0, int N>
CM_NODEBUG CM_INLINE vector<T, N> cm_hf8_cvt(vector<T0, N> src0,
                                             int flag = _GENX_NOSAT) {
  CM_HAS_HF8_CONTROL;

  constexpr bool is_hf8_hf16 =
      details::is_one_of_v<T, char> && details::is_one_of_v<T0, half>;
  constexpr bool is_hf16_hf8 =
      details::is_one_of_v<T, half> && details::is_one_of_v<T0, char>;

  CM_STATIC_ERROR((is_hf8_hf16 || is_hf16_hf8),
                  "unsupported cm_hf8_cvt type: src->dst must be char->half or "
                  "half->char for conversion ");

  vector<T0, N> _Src0 = src0;
  vector<T, N> _Result = details::__cm_intrinsic_impl_hf8_cvt<T>(_Src0);

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <typename T, typename T0, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T, N1 * N2> cm_hf8_cvt(matrix<T0, N1, N2> src,
                                                   int flag = _GENX_NOSAT) {
  vector<T0, N1 *N2> _Src = src;
  return cm_hf8_cvt<T>(_Src, flag);
}

template <typename T, typename T0>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_cm_scalar<T>::value &&
                                details::is_cm_scalar<T0>::value,
                            typename std::remove_const<T>::type>::type
    cm_hf8_cvt(T0 src, int flag = _GENX_NOSAT) {
  vector<T0, 1> _Src = src;
  vector<T, 1> _Result = cm_hf8_cvt<T>(_Src, flag);
  return _Result(0);
}

// float<->bfloat16 conversion
template <typename DstTy, typename SrcTy, int Width>
CM_NODEBUG CM_INLINE vector<DstTy, Width> cm_bf_cvt(vector<SrcTy, Width> Src) {
  using namespace details;
  CM_HAS_BF16_CONTROL;
  CM_STATIC_ERROR(
      (is_float_type<DstTy>::value &&
       (is_half_type<SrcTy>::value || is_word_type<SrcTy>::value)) ||
          (is_float_type<SrcTy>::value &&
           (is_half_type<DstTy>::value || is_word_type<DstTy>::value)),
      "Invalid type for cm_bf_cvt: src->dst must be {half,short,ushort}->float "
      "or float->{half,short,ushort}");

  if constexpr (is_float_type<SrcTy>::value) {
    vector<short, Width> Dst;
    if constexpr (Width == 1) {
      Dst = __spirv_ConvertFToBF16INTEL(Src[0]);
    } else {
      Dst = __spirv_ConvertFToBF16INTEL(Src);
    }
    return Dst.template format<DstTy>();
  } else if constexpr (Width == 1) {
    return __spirv_ConvertBF16ToFINTEL(Src.template format<short>()[0]);
  } else {
    return __spirv_ConvertBF16ToFINTEL(Src.template format<short>());
  }
}

template <typename DstTy, typename SrcTy, int Height, int Width>
CM_NODEBUG CM_INLINE vector<DstTy, Height * Width>
cm_bf_cvt(matrix<SrcTy, Height, Width> Src) {
  return cm_bf_cvt<DstTy>(Src.template format<SrcTy>());
}

template <typename DstTy, typename SrcTy>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::is_cm_scalar<DstTy>::value &&
                        details::is_cm_scalar<SrcTy>::value,
                        typename std::remove_const<DstTy>::type>::type
cm_bf_cvt(SrcTy Src) {
  vector<SrcTy, 1> _Src = Src;
  vector<DstTy, 1> _Result = cm_bf_cvt<DstTy>(_Src);
  return _Result[0];
}

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
