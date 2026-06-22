/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:dpas/dpasw.h should not be included explicitly");
#endif

#ifndef _CLANG_CM_DPAS_DPASW_H_
#define _CLANG_CM_DPAS_DPASW_H_

#include <cm/cm_common.h>
#include <cm/cm_has_instr.h>
#include <cm/cm_traits.h>

#include "helpers.h"

#ifdef CM_HAS_DPASW
#define CM_HAS_DPASW_CONTROL CM_HAS_CONTROL(true)
#else
#define CM_HAS_DPASW_CONTROL CM_HAS_CONTROL(false)
#endif

namespace details {
template <CmPrecisionType Src1Ty, CmPrecisionType Src2Ty, int SystolicDepth,
          int RepeatCount, typename AccTy, typename T1, typename T2,
          int AccSize, int Src1Size, int Src2Size>
CM_NODEBUG CM_INLINE void dpasw_check_common() {
  constexpr int ExecSize = get_dpas_execution_size(Src1Ty);

  CM_HAS_DPASW_CONTROL;

  CM_STATIC_ERROR(is_dpas_source_type<T1>(Src1Ty), "Src1 type is incorrect");
  CM_STATIC_ERROR(is_dpas_source_type<T2>(Src2Ty), "Src2 type is incorrect");

  CM_STATIC_ERROR((is_valid_dpas<AccTy, AccTy>(Src1Ty, Src2Ty)),
                  "Invalid combination of DPASW data types");

  CM_STATIC_ERROR(is_valid_repeat_count(RepeatCount, Src1Ty, Src2Ty),
                  "Invalid repeat count for DPASW operation");
  CM_STATIC_ERROR(SystolicDepth == 8,
                  "Systolic depth must be 8 for DPASW operation");

  CM_STATIC_ERROR(AccSize == get_dpas_acc_size(RepeatCount, Src1Ty),
                  "Unsupported execution size in DPASW operation");

  CM_STATIC_ERROR(Src1Size == get_dpas_src1_size(Src1Ty, Src2Ty, SystolicDepth),
                  "Invalid size for Src1 in DPASW operation");
  CM_STATIC_ERROR(Src2Size == get_dpas_src2_size(Src1Ty, Src2Ty,
                                                 (RepeatCount + 1) / 2,
                                                 SystolicDepth),
                  "Invalid size for Src2 in DPASW operation");
}

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename AccTy, typename Src1Ty,
          typename Src2Ty, int AccSize, int Src1Size, int Src2Size>
vector<AccTy, AccSize> __cm_intrinsic_impl_dpasw(vector<AccTy, AccSize> Acc,
                                                 vector<Src1Ty, Src1Size> Src1,
                                                 vector<Src2Ty, Src2Size> Src2);

template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
          int SystolicDepth, int RepeatCount, typename AccTy, typename Src1Ty,
          typename Src2Ty, int AccSize, int Src1Size, int Src2Size>
vector<AccTy, AccSize>
__cm_intrinsic_impl_dpasw_nosrc0(int Null, vector<Src1Ty, Src1Size> Src1,
                                 vector<Src2Ty, Src2Size> Src2);
} // namespace details

template <CmPrecisionType Src1Ty, CmPrecisionType Src2Ty, int SystolicDepth,
          int RepeatCount, typename AccTy, typename T1, typename T2,
          int AccSize, int Src1Size, int Src2Size>
CM_NODEBUG CM_INLINE vector<AccTy, AccSize>
cm_dpasw(vector<AccTy, AccSize> Acc, vector<T1, Src1Size> Src1,
         vector<T2, Src2Size> Src2, int flag = _GENX_NOSAT) {
  using namespace details;
  dpasw_check_common<Src1Ty, Src2Ty, SystolicDepth, RepeatCount, AccTy, T1, T2,
                     AccSize, Src1Size, Src2Size>();

  auto _Result = __cm_intrinsic_impl_dpasw<Src1Ty, Src2Ty, SystolicDepth,
                                           RepeatCount, AccTy>(Acc, Src1, Src2);

  if (flag != _GENX_SAT)
    return _Result;

  return __cm_intrinsic_impl_sat<AccTy>(_Result);
}

template <CmPrecisionType Src1Ty, CmPrecisionType Src2Ty, int SystolicDepth,
          int RepeatCount, typename AccTy, typename T1, typename T2,
          int AccSize = details::get_dpas_acc_size(RepeatCount, Src1Ty),
          int Src1Size = details::get_dpas_src1_size(Src1Ty, Src2Ty,
                                                     SystolicDepth),
          int Src2Size = details::get_dpas_src2_size(
              Src1Ty, Src2Ty, (RepeatCount + 1) / 2, SystolicDepth)>
CM_NODEBUG CM_INLINE vector<AccTy, AccSize>
cm_dpasw(int Dummy, vector<T1, Src1Size> Src1, vector<T2, Src2Size> Src2,
         int flag = _GENX_NOSAT) {
  using namespace details;
  (void)Dummy;
  dpasw_check_common<Src1Ty, Src2Ty, SystolicDepth, RepeatCount, AccTy, T1, T2,
                     AccSize, Src1Size, Src2Size>();

  auto _Result =
      __cm_intrinsic_impl_dpasw_nosrc0<Src1Ty, Src2Ty, SystolicDepth,
                                       RepeatCount, AccTy, T1, T2, AccSize>(
          NULL, Src1, Src2);

  if (flag != _GENX_SAT)
    return _Result;

  return __cm_intrinsic_impl_sat<AccTy>(_Result);
}

#endif // _CLANG_CM_DPAS_DPASW_H_
