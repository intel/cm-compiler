/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:dpas/bdpas.h should not be included explicitly");
#endif

#ifndef _CLANG_CM_DPAS_BDPAS_H_
#define _CLANG_CM_DPAS_BDPAS_H_

#include <cm/cm_common.h>
#include <cm/cm_has_instr.h>
#include <cm/cm_traits.h>

#include "helpers.h"

#ifdef CM_HAS_BDPAS
#define CM_HAS_BDPAS_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_BDPAS
#define CM_HAS_BDPAS_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_BDPAS

namespace details {
template <typename ResTy, typename AccTy>
constexpr bool is_valid_bdpas(CmPrecisionType Src1Ty, CmPrecisionType Src2Ty) {
  bool IsValid = is_valid_dpas_fp16<ResTy, AccTy>(Src1Ty, Src2Ty) |
                 is_valid_dpas_bf16<ResTy, AccTy>(Src1Ty, Src2Ty) |
                 is_valid_dpas_fp8<ResTy, AccTy>(Src1Ty, Src2Ty) |
                 is_valid_dpas_fp4<ResTy, AccTy>(Src1Ty, Src2Ty);
  return IsValid;
}

template <CmPrecisionType Src1Ty, CmPrecisionType Src2Ty, int SystolicDepth,
          int RepeatCount, typename ResTy, typename AccTy, typename T1,
          typename T2, int AccSize, int Src1Size, int Src2Size>
CM_NODEBUG CM_INLINE void bdpas_check_common() {
  constexpr int ExecSize = get_dpas_execution_size(Src1Ty);

  CM_HAS_BDPAS_CONTROL;

  CM_STATIC_ERROR(is_dpas_source_type<T1>(Src1Ty), "Src1 type is incorrect");
  CM_STATIC_ERROR(is_dpas_source_type<T2>(Src2Ty), "Src2 type is incorrect");

  constexpr bool IsValidBDpas = is_valid_bdpas<ResTy, AccTy>(Src1Ty, Src2Ty);
  CM_STATIC_ERROR(IsValidBDpas, "Invalid combination of BDPAS data types");

  CM_STATIC_ERROR(RepeatCount == 8, "Invalid repeat count for BDPAS operation");
  CM_STATIC_ERROR(SystolicDepth == 8,
                  "Systolic depth must be 8 for BDPAS operation");

  CM_STATIC_ERROR(AccSize == get_dpas_acc_size(RepeatCount, Src1Ty),
                  "Unsupported execution size in DPAS operation");

  CM_STATIC_ERROR(Src1Size == get_dpas_src1_size(Src1Ty, Src2Ty, SystolicDepth),
                  "Invalid size for Src1 in DPAS operation");
  CM_STATIC_ERROR(Src2Size == get_dpas_src2_size(Src1Ty, Src2Ty, RepeatCount,
                                                 SystolicDepth),
                  "Invalid size for Src2 in DPAS operation");
}

constexpr int BDpasScaleBlockSize = 32;

constexpr int get_bdpas_src1_scale_size(int SystolicDepth,
                                        CmPrecisionType Src1Ty) {
  auto ExecSize = get_dpas_execution_size(Src1Ty);
  auto OpsPerChannel = get_dpas_ops_per_channel(Src1Ty, Src1Ty);
  auto KDimension = SystolicDepth * OpsPerChannel;

  auto ScaleSize = KDimension / BDpasScaleBlockSize;
  if (ScaleSize < 1)
    ScaleSize = 1;

  return ScaleSize * ExecSize;
}

constexpr int get_bdpas_src2_scale_size(int SystolicDepth, int RepeatCount,
                                        CmPrecisionType Src2Ty) {
  auto OpsPerChannel = get_dpas_ops_per_channel(Src2Ty, Src2Ty);
  auto KDimension = SystolicDepth * OpsPerChannel;

  auto ScaleSize = KDimension / BDpasScaleBlockSize;
  if (ScaleSize < 1)
    ScaleSize = 1;

  return ScaleSize * RepeatCount;
}

template <int SystolicDepth, CmPrecisionType Src1Precision>
using BDpasSrc1ScaleType =
    vector<uint8_t, get_bdpas_src1_scale_size(SystolicDepth, Src1Precision)>;

template <int SystolicDepth, int RepeatCount, CmPrecisionType Src2Precision>
using BDpasSrc2ScaleType =
    vector<uint8_t, get_bdpas_src2_scale_size(SystolicDepth, RepeatCount,
                                              Src2Precision)>;

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename ResTy, typename AccTy,
          typename Src1Ty, typename Src2Ty, int AccSize, int Src1Size,
          int Src2Size>
vector<ResTy, AccSize> __cm_intrinsic_impl_bdpas(
    vector<AccTy, AccSize> Acc, vector<Src1Ty, Src1Size> Src1,
    vector<Src2Ty, Src2Size> Src2,
    BDpasSrc1ScaleType<SystolicDepth, Src1Precision> Src1Scale,
    BDpasSrc2ScaleType<SystolicDepth, RepeatCount, Src2Precision> Src2Scale);
} // namespace details

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename ResTy, typename AccTy,
          typename Src1Ty, typename Src2Ty, int AccSize, int Src1Size,
          int Src2Size>
CM_NODEBUG CM_INLINE vector<ResTy, AccSize>
cm_bdpas(vector<AccTy, AccSize> Acc, vector<Src1Ty, Src1Size> Src1,
         vector<Src2Ty, Src2Size> Src2,
         details::BDpasSrc1ScaleType<SystolicDepth, Src1Precision> Src1Scale,
         details::BDpasSrc2ScaleType<SystolicDepth, RepeatCount, Src2Precision>
             Src2Scale) {
  details::bdpas_check_common<Src1Precision, Src2Precision, SystolicDepth,
                              RepeatCount, ResTy, AccTy, Src1Ty, Src2Ty,
                              AccSize, Src1Size, Src2Size>();

  return details::__cm_intrinsic_impl_bdpas<
      Src1Precision, Src2Precision, SystolicDepth, RepeatCount, ResTy, AccTy,
      Src1Ty, Src2Ty, AccSize, Src1Size, Src2Size>(Acc, Src1, Src2, Src1Scale,
                                                   Src2Scale);
}

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename AccTy, typename Src1Ty,
          typename Src2Ty, int AccSize, int Src1Size, int Src2Size>
CM_NODEBUG CM_INLINE vector<AccTy, AccSize>
cm_bdpas(vector<AccTy, AccSize> Acc, vector<Src1Ty, Src1Size> Src1,
         vector<Src2Ty, Src2Size> Src2,
         details::BDpasSrc1ScaleType<SystolicDepth, Src1Precision> Src1Scale,
         details::BDpasSrc2ScaleType<SystolicDepth, RepeatCount, Src2Precision>
             Src2Scale) {
  return cm_bdpas<Src1Precision, Src2Precision, SystolicDepth, RepeatCount,
                  AccTy, AccTy, Src1Ty, Src2Ty, AccSize, Src1Size, Src2Size>(
      Acc, Src1, Src2, Src1Scale, Src2Scale);
}

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename ResTy, typename AccTy,
          typename Src1Ty, typename Src2Ty, int AccSize, int Src1Size,
          int Src2Size>
CM_NODEBUG CM_INLINE vector<ResTy, AccSize>
cm_bdpas(vector<AccTy, AccSize> Acc, vector<Src1Ty, Src1Size> Src1,
         vector<Src2Ty, Src2Size> Src2, int NullSrc1Scale,
         details::BDpasSrc2ScaleType<SystolicDepth, RepeatCount, Src2Precision>
             Src2Scale) {
  (void)NullSrc1Scale;

  details::BDpasSrc1ScaleType<SystolicDepth, Src1Precision> Src1Scale = 127;

  return cm_bdpas<Src1Precision, Src2Precision, SystolicDepth, RepeatCount,
                  ResTy, AccTy, Src1Ty, Src2Ty, AccSize, Src1Size, Src2Size>(
      Acc, Src1, Src2, Src1Scale, Src2Scale);
}

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename AccTy, typename Src1Ty,
          typename Src2Ty, int AccSize, int Src1Size, int Src2Size>
CM_NODEBUG CM_INLINE vector<AccTy, AccSize>
cm_bdpas(vector<AccTy, AccSize> Acc, vector<Src1Ty, Src1Size> Src1,
         vector<Src2Ty, Src2Size> Src2, int NullSrc1Scale,
         details::BDpasSrc2ScaleType<SystolicDepth, RepeatCount, Src2Precision>
             Src2Scale) {
  return cm_bdpas<Src1Precision, Src2Precision, SystolicDepth, RepeatCount,
                  AccTy, AccTy, Src1Ty, Src2Ty, AccSize, Src1Size, Src2Size>(
      Acc, Src1, Src2, NullSrc1Scale, Src2Scale);
}

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename ResTy, typename AccTy,
          typename Src1Ty, typename Src2Ty, int AccSize, int Src1Size,
          int Src2Size>
CM_NODEBUG CM_INLINE vector<ResTy, AccSize>
cm_bdpas(vector<AccTy, AccSize> Acc, vector<Src1Ty, Src1Size> Src1,
         vector<Src2Ty, Src2Size> Src2,
         details::BDpasSrc1ScaleType<SystolicDepth, Src1Precision> Src1Scale,
         int NullSrc2Scale = 0) {
  (void)NullSrc2Scale;

  details::BDpasSrc2ScaleType<SystolicDepth, RepeatCount, Src2Precision>
      Src2Scale = 127;

  return cm_bdpas<Src1Precision, Src2Precision, SystolicDepth, RepeatCount,
                  ResTy, AccTy, Src1Ty, Src2Ty, AccSize, Src1Size, Src2Size>(
      Acc, Src1, Src2, Src1Scale, Src2Scale);
}

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename AccTy, typename Src1Ty,
          typename Src2Ty, int AccSize, int Src1Size, int Src2Size>
CM_NODEBUG CM_INLINE vector<AccTy, AccSize>
cm_bdpas(vector<AccTy, AccSize> Acc, vector<Src1Ty, Src1Size> Src1,
         vector<Src2Ty, Src2Size> Src2,
         details::BDpasSrc1ScaleType<SystolicDepth, Src1Precision> Src1Scale,
         int NullSrc2Scale = 0) {
  return cm_bdpas<Src1Precision, Src2Precision, SystolicDepth, RepeatCount,
                  AccTy, AccTy, Src1Ty, Src2Ty, AccSize, Src1Size, Src2Size>(
      Acc, Src1, Src2, Src1Scale, NullSrc2Scale);
}

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename ResTy, typename Src1Ty,
          typename Src2Ty,
          int AccSize = details::get_dpas_acc_size(RepeatCount, Src1Precision),
          int Src1Size = details::get_dpas_src1_size(
              Src1Precision, Src2Precision, SystolicDepth),
          int Src2Size = details::get_dpas_src2_size(
              Src1Precision, Src2Precision, RepeatCount, SystolicDepth)>
CM_NODEBUG CM_INLINE vector<ResTy, AccSize>
cm_bdpas(int Null, vector<Src1Ty, Src1Size> Src1, vector<Src2Ty, Src2Size> Src2,
         details::BDpasSrc1ScaleType<SystolicDepth, Src1Precision> Src1Scale,
         details::BDpasSrc2ScaleType<SystolicDepth, RepeatCount, Src2Precision>
             Src2Scale) {
  (void)Null;

  vector<ResTy, AccSize> Acc = ResTy(0);

  return cm_bdpas<Src1Precision, Src2Precision, SystolicDepth, RepeatCount,
                  ResTy, ResTy, Src1Ty, Src2Ty, AccSize, Src1Size, Src2Size>(
      Acc, Src1, Src2, Src1Scale, Src2Scale);
}

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename ResTy, typename Src1Ty,
          typename Src2Ty,
          int AccSize = details::get_dpas_acc_size(RepeatCount, Src1Precision),
          int Src1Size = details::get_dpas_src1_size(
              Src1Precision, Src2Precision, SystolicDepth),
          int Src2Size = details::get_dpas_src2_size(
              Src1Precision, Src2Precision, RepeatCount, SystolicDepth)>
CM_NODEBUG CM_INLINE vector<ResTy, AccSize>
cm_bdpas(int Null, vector<Src1Ty, Src1Size> Src1, vector<Src2Ty, Src2Size> Src2,
         int NullSrc1Scale,
         details::BDpasSrc2ScaleType<SystolicDepth, RepeatCount, Src2Precision>
             Src2Scale) {
  (void)Null;
  (void)NullSrc1Scale;

  vector<ResTy, AccSize> Acc = ResTy(0);
  details::BDpasSrc1ScaleType<SystolicDepth, Src1Precision> Src1Scale = 127;

  return cm_bdpas<Src1Precision, Src2Precision, SystolicDepth, RepeatCount,
                  ResTy, ResTy, Src1Ty, Src2Ty, AccSize, Src1Size, Src2Size>(
      Acc, Src1, Src2, Src1Scale, Src2Scale);
}

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename ResTy, typename Src1Ty,
          typename Src2Ty,
          int AccSize = details::get_dpas_acc_size(RepeatCount, Src1Precision),
          int Src1Size = details::get_dpas_src1_size(
              Src1Precision, Src2Precision, SystolicDepth),
          int Src2Size = details::get_dpas_src2_size(
              Src1Precision, Src2Precision, RepeatCount, SystolicDepth)>
CM_NODEBUG CM_INLINE vector<ResTy, AccSize>
cm_bdpas(int Null, vector<Src1Ty, Src1Size> Src1, vector<Src2Ty, Src2Size> Src2,
         details::BDpasSrc1ScaleType<SystolicDepth, Src1Precision> Src1Scale,
         int NullSrc2Scale = 0) {
  (void)Null;
  (void)NullSrc2Scale;

  vector<ResTy, AccSize> Acc = ResTy(0);
  details::BDpasSrc2ScaleType<SystolicDepth, RepeatCount, Src2Precision>
      Src2Scale = 127;

  return cm_bdpas<Src1Precision, Src2Precision, SystolicDepth, RepeatCount,
                  ResTy, ResTy, Src1Ty, Src2Ty, AccSize, Src1Size, Src2Size>(
      Acc, Src1, Src2, Src1Scale, Src2Scale);
}

#endif // _CLANG_CM_DPAS_BDPAS_H_
