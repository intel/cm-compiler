/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_dpas.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_DPAS_H_
#define _CLANG_CM_DPAS_H_

#include "cm_common.h"
#include "cm_has_instr.h"
#include "cm_traits.h"

//////////////////////////////////////////
// DPAS intrinsic begin
//////////////////////////////////////////

inline constexpr int get_dpas_execution_size(CmPrecisionType src1_precision) {
  (void)src1_precision;
#if !defined(CM_GENX)
  CM_STATIC_WARNING(0,
                    "GEN not specified so cm_dpas() code may not be optimal");
  return 8;
#endif // !defined(CM_GENX)
  if (CM_GENX >= 1280)
    return 16;
  return 8;
}

template <typename T>
inline constexpr bool check_type(CmPrecisionType src1_precision) {
  (void)src1_precision;

  return details::is_dword_type<T>::value;
}

template <CmPrecisionType src1_precision, CmPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename T, typename T1,
          typename T2, int N, int N1, int N2>
CM_NODEBUG CM_INLINE void cm_dpas_check_common() {

  CM_HAS_DPAS_CONTROL;

  CM_STATIC_ERROR(check_type<T1>(src1_precision), "Src1 type is incorrect");

  CM_STATIC_ERROR(check_type<T2>(src2_precision), "Src2 type is incorrect");

  CM_STATIC_ERROR((systolic_depth == 8) || (systolic_depth == 4),
                  "systolic_depth must be 8 or 4");

  CM_STATIC_ERROR((repeat_count >= 1) && (repeat_count <= 8),
                  "repeat_count must be within 1 to 8");

  constexpr int DPAS_EXECUTION_SIZE = get_dpas_execution_size(src1_precision);

  CM_STATIC_ERROR((N == DPAS_EXECUTION_SIZE * repeat_count),
                  "Unsupported execution size in dpas");

  // DPAS RepeatCount is restricted to event counts except 8x1
  if constexpr (repeat_count > 1 && repeat_count % 2 == 1)
    CM_HAS_DPAS_ODD_CONTROL;

  constexpr unsigned ops_per_channel =
      get_ops_per_channel(src1_precision, src2_precision);
  CM_STATIC_ERROR(ops_per_channel != 0xFFFFFFFF,
                  "invalid combination of Src1/Src2 precision");
  constexpr unsigned src1_precision_bits = get_precision_bits(src1_precision);

  CM_STATIC_ERROR(
      N1 == ((src1_precision_bits * systolic_depth * ops_per_channel * N) /
             (repeat_count * sizeof(T1) * 8)),
      "invalid size for Src1");

  constexpr unsigned src2_precision_bits = get_precision_bits(src2_precision);
  CM_STATIC_ERROR(N2 == ((src2_precision_bits * systolic_depth *
                          ops_per_channel * repeat_count) /
                         (sizeof(T2) * 8)),
                  "invalid size for Src2");
}

template <CmPrecisionType src1_precision, CmPrecisionType src2_precision,
          typename T0, int systolic_depth, int repeat_count, typename T,
          typename T1, typename T2, int N, int N1, int N2>
CM_NODEBUG CM_INLINE void cm_dpas_check_types() {

  // INT //////////////////////////////////////////////////////////////////////
  // types: dst, src0, src1, src2
  // ud, d | ud, d | ub, b | ub, b
  // ud, d | ud, d | u4, s4, u2, s2 | ub, b
  // ud, d | ud, d | ub, b | u4, s4, u2, s2
  // ud, d | ud, d | u4, s4, u2, s2 | u4, s4, u2, s2
  constexpr bool check_int =
      details::is_one_of_v<T0, unsigned int, int> &&
      details::is_one_of_v<T, unsigned int, int> &&
      details::is_one_of_enum_v<
          CmPrecisionType, src1_precision, CmPrecisionType::CM_Precision_S8,
          CmPrecisionType::CM_Precision_U8, CmPrecisionType::CM_Precision_U4,
          CmPrecisionType::CM_Precision_S4, CmPrecisionType::CM_Precision_U2,
          CmPrecisionType::CM_Precision_S2> &&
      details::is_one_of_enum_v<
          CmPrecisionType, src2_precision, CmPrecisionType::CM_Precision_S8,
          CmPrecisionType::CM_Precision_U8, CmPrecisionType::CM_Precision_U4,
          CmPrecisionType::CM_Precision_S4, CmPrecisionType::CM_Precision_U2,
          CmPrecisionType::CM_Precision_S2>;
  //////////////////////////////////////////////////////////////////////////////
  // HF ////////////////////////////////////////////////////////////////////////
  // f,hf | f, hf | hf | hf
  constexpr bool check_hf =
      (details::is_one_of_v<T0, float, half> &&
       details::is_one_of_v<T, float, half> &&
       details::is_one_of_enum_v<CmPrecisionType, src1_precision,
                                 CmPrecisionType::CM_Precision_FP16> &&
       details::is_one_of_enum_v<CmPrecisionType, src2_precision,
                                 CmPrecisionType::CM_Precision_FP16>);
  //////////////////////////////////////////////////////////////////////////////
  // BF16 //////////////////////////////////////////////////////////////////////
  // f, bf | f, bf | bf | bf
  CM_HAS_BF16_CONTROL;

  constexpr bool check_bf16 =
      details::is_one_of_v<T0, float, short> &&
      details::is_one_of_v<T, float, short> &&
      details::is_one_of_enum_v<CmPrecisionType, src1_precision,
                                CmPrecisionType::CM_Precision_BF16> &&
      details::is_one_of_enum_v<CmPrecisionType, src2_precision,
                                CmPrecisionType::CM_Precision_BF16>;

  //////////////////////////////////////////////////////////////////////////////
  // TF32 //////////////////////////////////////////////////////////////////////
#ifdef CM_HAS_TF32
  // f | f | tf32 | tf32
  constexpr bool check_tf32 =
      details::is_one_of_v<T0, float> && details::is_one_of_v<T, float> &&
      details::is_one_of_enum_v<CmPrecisionType, src1_precision,
                                CmPrecisionType::CM_Precision_TF32> &&
      details::is_one_of_enum_v<CmPrecisionType, src2_precision,
                                CmPrecisionType::CM_Precision_TF32>;
#else  // CM_HAS_TF32
  constexpr bool check_tf32 = false;
#endif // CM_HAS_TF32


  if constexpr (check_hf &&
                (std::is_same<T0, half>::value || std::is_same<T, half>::value))
    CM_HAS_DPAS_ACC_HALF_CONTROL;
  if constexpr (check_bf16 && (std::is_same<T0, short>::value ||
                               std::is_same<T, short>::value))
    CM_HAS_DPAS_ACC_BF16_CONTROL;

  constexpr bool is_right = (check_int || check_hf || check_bf16 || check_tf32);

  if (!is_right) {
    CM_STATIC_WARNING(is_right, "types: dst | src0 | src1(from template "
                                "parameter) | src2(from template parameter)");
    CM_STATIC_WARNING(
        is_right,
        "ud, d | ud, d | ub, b, u4, s4, u2, s2 | ub, b, u4, s4, u2, s2");
    CM_STATIC_WARNING(is_right, "f, bf | f, bf | bf | bf");
    CM_STATIC_WARNING(is_right, "f, hf | f, hf | hf | hf");
#if defined(CM_HAS_TF32)
    CM_STATIC_WARNING(is_right, "f | f | tf32 | tf32");
#endif // defined(CM_HAS_TF32)

    CM_STATIC_ERROR(is_right, "unsupported dpas type");
  }
}

template <CmPrecisionType src1_precision, CmPrecisionType src2_precision,
          typename T0, int systolic_depth, int repeat_count, typename T,
          typename T1, typename T2, int N, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T0, N>
cm_dpas(vector<T, N> src0, vector<T1, N1> src1, vector<T2, N2> src2,
        int flag = _GENX_NOSAT) {
  cm_dpas_check_common<src1_precision, src2_precision, systolic_depth,
                       repeat_count, T, T1, T2, N, N1, N2>();

  cm_dpas_check_types<src1_precision, src2_precision, T0, systolic_depth,
                      repeat_count, T, T1, T2, N, N1, N2>();

  vector<T, N> _Src0 = src0;
  vector<T1, N1> _Src1 = src1;
  vector<T2, N2> _Src2 = src2;
  vector<T0, N> _Result =
      details::__cm_intrinsic_impl_dpas<src1_precision, src2_precision,
                                        systolic_depth, repeat_count, T0>(
          _Src0, _Src1, _Src2);
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}

template <CmPrecisionType src1_precision, CmPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename T, typename T1,
          typename T2, int N, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T, N>
cm_dpas(vector<T, N> src0, vector<T1, N1> src1, vector<T2, N2> src2,
        int flag = _GENX_NOSAT) {
  return cm_dpas<src1_precision, src2_precision, T, systolic_depth,
                 repeat_count>(src0, src1, src2, flag);
}

template <CmPrecisionType src1_precision, CmPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename T, typename T1,
          typename T2, int N, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T, N> cm_dpas(int dummy, vector<T1, N1> src1,
                                          vector<T2, N2> src2,
                                          int flag = _GENX_NOSAT) {
  cm_dpas_check_common<src1_precision, src2_precision, systolic_depth,
                       repeat_count, T, T1, T2, N, N1, N2>();

  CM_STATIC_ERROR(details::is_fp_or_dword_type<T>::value,
                  "Dst and Src0 must be FP or DWORD type");

  vector<T1, N1> _Src1 = src1;
  vector<T2, N2> _Src2 = src2;
  vector<T, N> _Result =
      details::__cm_intrinsic_impl_dpas_nosrc0<src1_precision, src2_precision,
                                               systolic_depth, repeat_count, T,
                                               T1, T2, N>(NULL, _Src1, _Src2);
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

//////////////////////////////////////////
// DPAS intrinsic end
//////////////////////////////////////////

//////////////////////////////////////////
// DPASW intrinsic begin
//////////////////////////////////////////
template <CmPrecisionType src1_precision, CmPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename T, typename T1,
          typename T2, int N, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T, N>
cm_dpasw(vector<T, N> src0, vector<T1, N1> src1, vector<T2, N2> src2,
         int flag = _GENX_NOSAT) {
  CM_HAS_DPASW_CONTROL;

  CM_STATIC_ERROR(details::is_fp_or_dword_type<T>::value,
                  "Dst and Src0 must be FP or DWORD type");

  CM_STATIC_ERROR(details::is_dword_type<T1>::value, "Src1 must be DWORD type");

  CM_STATIC_ERROR(details::is_dword_type<T2>::value, "Src2 must be DWORD type");

  CM_STATIC_ERROR((N == 8 * repeat_count), "Execution size must be 8");

  CM_STATIC_ERROR((systolic_depth == 8) || (systolic_depth == 4),
                  "systolic_depth must be 8 or 4");

  CM_STATIC_ERROR((repeat_count >= 1) && (repeat_count <= 8),
                  "repeat_count must be within 1 to 8");

  constexpr unsigned ops_per_channel =
      get_ops_per_channel(src1_precision, src2_precision);
  CM_STATIC_ERROR(ops_per_channel != 0xFFFFFFFF,
                  "invalid combination of Src1/Src2 precision");

  constexpr unsigned src1_precision_bits = get_precision_bits(src1_precision);
  CM_STATIC_ERROR(
      N1 == ((src1_precision_bits * systolic_depth * ops_per_channel * N) /
             (repeat_count * sizeof(T1) * 8)),
      "invalid size for Src1");

  constexpr unsigned src2_precision_bits = get_precision_bits(src2_precision);
  CM_STATIC_ERROR(N2 == ((src2_precision_bits * systolic_depth *
                          ops_per_channel * ((repeat_count + 1) / 2)) /
                         (sizeof(T2) * 8)),
                  "invalid size for Src2");

  vector<T, N> _Src0 = src0;
  vector<T1, N1> _Src1 = src1;
  vector<T2, N2> _Src2 = src2;
  vector<T, N> _Result =
      details::__cm_intrinsic_impl_dpasw<src1_precision, src2_precision,
                                         systolic_depth, repeat_count, T>(
          _Src0, _Src1, _Src2);
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <CmPrecisionType src1_precision, CmPrecisionType src2_precision,
          int systolic_depth, int repeat_count, typename T, typename T1,
          typename T2, int N, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T, N> cm_dpasw(int dummy, vector<T1, N1> src1,
                                           vector<T2, N2> src2,
                                           int flag = _GENX_NOSAT) {
  CM_HAS_DPASW_CONTROL;

  CM_STATIC_ERROR(details::is_fp_or_dword_type<T>::value,
                  "Dst and Src0 must be FP or DWORD type");

  CM_STATIC_ERROR(details::is_dword_type<T1>::value, "Src1 must be DWORD type");

  CM_STATIC_ERROR(details::is_dword_type<T2>::value, "Src2 must be DWORD type");

  CM_STATIC_ERROR(N == 8 * repeat_count, "Execution size must be 8");
  CM_STATIC_ERROR((systolic_depth == 8) || (systolic_depth == 4),
                  "systolic_depth must be 8 or 4");

  CM_STATIC_ERROR((repeat_count >= 1) && (repeat_count <= 8),
                  "repeat_count must be within 1 to 8");

  constexpr unsigned ops_per_channel =
      get_ops_per_channel(src1_precision, src2_precision);
  CM_STATIC_ERROR(ops_per_channel != 0xFFFFFFFF,
                  "invalid combination of Src1/Src2 precision");

  constexpr unsigned src1_precision_bits = get_precision_bits(src1_precision);
  CM_STATIC_ERROR(
      N1 == ((src1_precision_bits * systolic_depth * ops_per_channel * N) /
             (repeat_count * sizeof(T1) * 8)),
      "invalid size for Src1");

  constexpr unsigned src2_precision_bits = get_precision_bits(src2_precision);
  CM_STATIC_ERROR(N2 == ((src2_precision_bits * systolic_depth *
                          ops_per_channel * ((repeat_count + 1) / 2)) /
                         (sizeof(T2) * 8)),
                  "invalid size for Src2");

  vector<T1, N1> _Src1 = src1;
  vector<T2, N2> _Src2 = src2;
  vector<T, N> _Result =
      details::__cm_intrinsic_impl_dpasw_nosrc0<src1_precision, src2_precision,
                                                systolic_depth, repeat_count, T,
                                                T1, T2, N>(NULL, _Src1, _Src2);
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

//////////////////////////////////////////
// DPASW intrinsic end
//////////////////////////////////////////

#endif // _CLANG_CM_DPAS_H_
