/*
 * Copyright (c) 2019, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _CLANG_CM_H_
#define _CLANG_CM_H_

#include "cm_target.h"
#include "cm_common.h"
#include "cm_linear.h"
#include "cm_send.h"
#include "cm_atomic.h"
#include "cm_dataport.h"
#include "cm_lsc.h"
#include "cm_emu.h"
#include "cm_internal.h"
#include "cm_sampler.h"
#include "cm_traits.h"
#include "cm_vme.h"
#include "cm_gateway.h"
#include "cm_svm.h"
#include "cm_printf.h"

// cm_abs
namespace details {

template <typename T0, typename T1, int SZ>
CM_NODEBUG CM_INLINE vector<T0, SZ>
__cm_abs_common_internal(vector<T1, SZ> src0, int flag = _GENX_NOSAT) {
  vector<T1, SZ> _Result = __cm_intrinsic_impl_abs(src0);
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

// cm_add
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_add(vector<T1, SZ> src0, U src1,
                                           int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<decltype(src0), U>::type
  ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  return details::__cm_intrinsic_impl_add<T0>(_Src0, _Src1, flag);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_add(matrix<T1, N1, N2> src0, U src1,
                                               int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_add<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value,
    typename std::remove_const<T0>::type>::type
cm_add(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<T1, T2>::type ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  vector<T0, 1> _Result = cm_add<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T1>::value &&details::is_cm_vector<T2>::value,
    decltype(cm_add<T0>(T2(), T1()))>::type
cm_add(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  return cm_add<T0>(src1, src0, flag);
}

// cm_mul
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_mul(vector<T1, SZ> src0, U src1,
                                           int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<decltype(src0), U>::type
  ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  return details::__cm_intrinsic_impl_mul<T0>(_Src0, _Src1, flag);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_mul(matrix<T1, N1, N2> src0, U src1,
                                               int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_mul<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value,
    typename std::remove_const<T0>::type>::type
cm_mul(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<T1, T2>::type ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  vector<T0, 1> _Result = cm_mul<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T1>::value &&details::is_cm_vector<T2>::value,
    decltype(cm_mul<T0>(T2(), T1()))>::type
cm_mul(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  return cm_mul<T0>(src1, src0, flag);
}

// cm_avg
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, SZ> >::type
cm_avg(vector<T1, SZ> src0, U src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<decltype(src0), U>::type
  ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  return details::__cm_intrinsic_impl_avg<T0>(_Src0, _Src1, flag);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, N1 *N2> >::type
cm_avg(matrix<T1, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_avg<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value &&std::is_integral<T0>::value &&
            std::is_integral<T1>::value &&std::is_integral<T2>::value,
    typename std::remove_const<T0>::type>::type
cm_avg(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<T1, T2>::type ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  vector<T0, 1> _Result = cm_avg<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T1>::value &&details::is_cm_vector<T2>::value &&
        std::is_integral<T0>::value &&std::is_integral<T1>::value &&
            std::is_integral<T2>::value,
    decltype(cm_avg<T0>(T2(), T1()))>::type
cm_avg(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  return cm_avg<T0>(src1, src0, flag);
}

// cm_shl
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, SZ> >::type
cm_shl(vector<T1, SZ> src0, U src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<decltype(src0), U>::type
  ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  return details::__cm_intrinsic_impl_shl<T0>(_Src0, _Src1, flag);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, N1 *N2> >::type
cm_shl(matrix<T1, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_shl<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value &&std::is_integral<T0>::value &&
            std::is_integral<T1>::value &&std::is_integral<T2>::value,
    typename std::remove_const<T0>::type>::type
cm_shl(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<T1, T2>::type ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  vector<T0, 1> _Result = cm_shl<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

// cm_shr
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, SZ> >::type
cm_shr(vector<T1, SZ> src0, U src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<decltype(src0), U>::type ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  typename details::vector_type<ComputationTy>::type _Result = _Src0 >> _Src1;

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, N1 *N2> >::type
cm_shr(matrix<T1, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_shr<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value &&std::is_integral<T0>::value &&
            std::is_integral<T1>::value &&std::is_integral<T2>::value,
    typename std::remove_const<T0>::type>::type
cm_shr(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<T1, T2>::type ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  vector<T0, 1> _Result = cm_shr<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

// cm_lsr
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, SZ> >::type
cm_lsr(vector<T1, SZ> src0, U src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<T1, T1>::type _IntermedTy;
  typedef typename details::make_unsigned<_IntermedTy>::type ComputationTy;
  vector<ComputationTy, SZ> _Src0 = src0;
  vector<ComputationTy, SZ> _Result = _Src0 >> src1;

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, N1 *N2> >::type
cm_lsr(matrix<T1, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_lsr<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value &&std::is_integral<T0>::value &&
            std::is_integral<T1>::value &&std::is_integral<T2>::value,
    typename std::remove_const<T0>::type>::type
cm_lsr(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<T1, T2>::type ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  vector<T0, 1> _Result = cm_lsr<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T1>::value &&details::is_cm_vector<T2>::value &&
        std::is_integral<T0>::value &&std::is_integral<T1>::value &&
            std::is_integral<T2>::value,
    decltype(cm_lsr<T0>(T2(), T1()))>::type
cm_lsr(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  return cm_lsr<T0>(src1, src0, flag);
}

// cm_asr
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, SZ> >::type
cm_asr(vector<T1, SZ> src0, U src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<T1, T1>::type _IntermedTy;
  typedef typename details::make_signed<_IntermedTy>::type ComputationTy;
  vector<ComputationTy, SZ> _Src0 = src0;
  vector<ComputationTy, SZ> _Result = _Src0 >> src1;

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value &&
        std::is_integral<U>::value,
    vector<T0, N1 *N2> >::type
cm_asr(matrix<T1, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_asr<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value &&std::is_integral<T0>::value &&
            std::is_integral<T1>::value &&std::is_integral<T2>::value,
    typename std::remove_const<T0>::type>::type
cm_asr(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  typedef typename details::computation_type<T1, T2>::type ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  vector<T0, 1> _Result = cm_asr<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T1>::value &&details::is_cm_vector<T2>::value &&
        std::is_integral<T0>::value &&std::is_integral<T1>::value &&
            std::is_integral<T2>::value,
    decltype(cm_asr<T0>(T2(), T1()))>::type
cm_asr(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  return cm_asr<T0>(src1, src0, flag);
}

// cm_imul
#ifndef CM_HAS_LONG_LONG
// use mulh instruction for high half
template <typename T0, typename T1, typename U, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_dword_type<T0>::value &&details::is_dword_type<T1>::value &&
        details::is_dword_type<U>::value,
    vector<T0, SZ> >::type
cm_imul(vector_ref<T0, SZ> rmd, vector<T1, SZ> src0, U src1) {
  typedef typename details::computation_type<decltype(src0), U>::type
  ComputationTy;
  typename details::vector_type<ComputationTy>::type _Src0 = src0;
  typename details::vector_type<ComputationTy>::type _Src1 = src1;
  rmd = _Src0 * _Src1;
  return details::__cm_intrinsic_impl_imul(_Src0, _Src1);
}

#else
// cm_imul bdw+ version: use qw=dw*dw multiply.
// We need to special case SZ==1 to avoid "error: when select size is 1, the
// stride must also be 1" on the selects.
template <typename T0, typename T1, typename U, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_dword_type<T0>::value &&details::is_dword_type<T1>::value &&
            details::is_dword_type<U>::value &&SZ == 1,
    vector<T0, SZ> >::type
cm_imul(vector_ref<T0, SZ> rmd, vector<T1, SZ> src0, U src1) {
  typedef typename details::computation_type<decltype(rmd), long long>::type
  ComputationTy;
  ComputationTy _Product = cm_mul<long long>(src0, src1);
  rmd = _Product.format<T0>().select<1, 1>(0);
  return _Product.format<T0>().select<1, 1>(1);
}

template <typename T0, typename T1, typename U, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_dword_type<T0>::value &&details::is_dword_type<T1>::value &&
            details::is_dword_type<U>::value &&SZ != 1,
    vector<T0, SZ> >::type
cm_imul(vector_ref<T0, SZ> rmd, vector<T1, SZ> src0, U src1) {
  typedef typename details::computation_type<decltype(rmd), long long>::type
  ComputationTy;
  ComputationTy _Product = cm_mul<long long>(src0, src1);
  rmd = _Product.format<T0>().select<SZ, 2>(0);
  return _Product.format<T0>().select<SZ, 2>(1);
}
#endif

// cm_imul wrappers
template <typename T0, typename T1, typename U, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2>
cm_imul(vector_ref<T0, N1 *N2> rmd, matrix<T1, N1, N2> src0, U src1) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_imul(rmd, _Src0, src1);
}

template <typename T0, typename T1, typename U, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2>
cm_imul(matrix_ref<T0, N1, N2> rmd, matrix<T1, N1, N2> src0, U src1) {
  vector_ref<T0, N1 *N2> _Rmd = rmd.format<T0>();
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_imul(_Rmd, _Src0, src1);
}

template <typename T0, typename T1, typename U, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<details::is_cm_scalar<U>::value,
                                             vector<T0, SZ> >::type
cm_imul(vector_ref<T0, SZ> rmd, U src0, vector<T1, SZ> src1) {
  return cm_imul(rmd, src1, src0);
}

template <typename T0, typename T1, typename U, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<details::is_cm_scalar<U>::value,
                                             vector<T0, N1 *N2> >::type
cm_imul(vector_ref<T0, N1 *N2> rmd, U src0, matrix<T1, N1, N2> src1) {
  return cm_imul(rmd, src1, src0);
}

template <typename T0, typename T, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T>::value &&details::is_cm_scalar<U>::value &&
        details::is_cm_scalar<T0>::value,
    T0>::type
cm_imul(vector_ref<T0, 1> rmd, T src0, U src1) {
  vector<T, 1> _src0 = src0;
  vector<U, 1> _src1 = src1;
  vector<T0, 1> res = cm_imul(rmd, _src0.select_all(), _src1.select_all());
  return res(0);
}

// cm_quot
template <typename T, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value,
    vector<T, SZ> >::type
cm_quot(vector<T, SZ> src0, U src1) {
  return src0 / src1;
}

template <typename T, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value,
    vector<T, N1 *N2> >::type
cm_quot(matrix<T, N1, N2> src0, U src1) {
  return src0 / src1;
}

template <typename T0, typename T1>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        std::is_integral<T0>::value &&std::is_integral<T1>::value,
    typename std::remove_const<T0>::type>::type
cm_quot(T0 src0, T1 src1) {
  return src0 / src1;
}

// cm_mod
template <typename T, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value,
    vector<T, SZ> >::type
cm_mod(vector<T, SZ> src0, U src1) {
  return src0 % src1;
}

template <typename T, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value,
    vector<T, N1 *N2> >::type
cm_mod(matrix<T, N1, N2> src0, U src1) {
  return src0 % src1;
}

template <typename T0, typename T1>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        std::is_integral<T0>::value &&std::is_integral<T1>::value,
    typename std::remove_const<T0>::type>::type
cm_mod(T0 src0, T1 src1) {
  return src0 % src1;
}

// cm_div, compute quotient and remainder of division.
template <typename T, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value,
    vector<T, SZ> >::type
cm_div(vector_ref<T, SZ> remainder, vector<T, SZ> src0, U src1) {
  remainder = src0 % src1;
  return src0 / src1;
}

template <typename T, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value,
    vector<T, N1 *N2> >::type
cm_div(matrix_ref<T, N1, N2> remainder, matrix<T, N1, N2> src0, U src1) {
  remainder = src0 % src1;
  return src0 / src1;
}

template <typename T, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value,
    vector<T, N1 *N2> >::type
cm_div(vector_ref<T, N1 *N2> remainder, matrix<T, N1, N2> src0, U src1) {
  vector<T, N1 *N2> _Src0 = src0;
  remainder = _Src0 % src1;
  return _Src0 / src1;
}

template <typename T, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value &&
        details::is_cm_scalar<U>::value,
    vector<T, SZ> >::type
cm_div(vector_ref<T, SZ> remainder, U src0, vector<T, SZ> src1) {
  remainder = src0 % src1;
  return src0 / src1;
}

template <typename T, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value &&
        details::is_cm_scalar<U>::value,
    vector<T, N1 *N2> >::type
cm_div(matrix_ref<T, N1, N2> remainder, U src0, matrix<T, N1, N2> src1) {
  remainder = src0 % src1;
  return src0 / src1;
}

template <typename T, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T>::value &&std::is_integral<U>::value &&
        details::is_cm_scalar<U>::value,
    vector<T, N1 *N2> >::type
cm_div(vector_ref<T, N1 *N2> remainder, U src0, matrix<T, N1, N2> src1) {
  vector<T, N1 *N2> _Src1 = src1;
  remainder = src0 % _Src1;
  return src0 / _Src1;
}

template <typename RT, typename T0, typename T1>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<RT>::value &&details::is_cm_scalar<T0>::value &&
        details::is_cm_scalar<T1>::value,
    typename std::remove_const<RT>::type>::type
cm_div(vector_ref<typename std::remove_const<RT>::type, 1> remainder, T0 src0,
       T1 src1) {
  remainder(0) = src0 % src1;
  return src0 / src1;
}

// cm_min and cm_max
//
// Restriction:
//
// The source operands must be both of integer or both of floating-point type.
//
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_max(vector<T1, SZ> src0, U src1,
                                           int flag = _GENX_NOSAT) {
  typedef typename details::common_type<decltype(src0), U>::type CommonTy;
  typename details::vector_type<CommonTy>::type _Src0 = src0;
  typename details::vector_type<CommonTy>::type _Src1 = src1;
  typename details::vector_type<CommonTy>::type _Result =
      details::__cm_intrinsic_impl_max(_Src0, _Src1);

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_max(matrix<T1, N1, N2> src0, U src1,
                                               int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_max<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T1>::value &&details::is_cm_vector<T2>::value,
    decltype(cm_max<T0>(T2(), T1()))>::type
cm_max(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  return cm_max<T0>(src1, src0, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value,
    T0>::type
cm_max(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  vector<T1, 1> _Src0 = src0;
  vector<T2, 1> _Src1 = src1;
  vector<T0, 1> _Result = cm_max<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_min(vector<T1, SZ> src0, U src1,
                                           int flag = _GENX_NOSAT) {
  typedef typename details::common_type<decltype(src0), U>::type CommonTy;
  typename details::vector_type<CommonTy>::type _Src0 = src0;
  typename details::vector_type<CommonTy>::type _Src1 = src1;
  typename details::vector_type<CommonTy>::type _Result =
      details::__cm_intrinsic_impl_min(_Src0, _Src1);

  if (flag != _GENX_SAT)
    return _Result;
  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_min(matrix<T1, N1, N2> src0, U src1,
                                               int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_min<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T1>::value &&details::is_cm_vector<T2>::value,
    decltype(cm_min<T0>(T2(), T1()))>::type
cm_min(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  return cm_min<T0>(src1, src0, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value,
    T0>::type
cm_min(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  vector<T1, 1> _Src0 = src0;
  vector<T2, 1> _Src1 = src1;
  vector<T0, 1> _Result = cm_min<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

// Dot product builtins
#if defined(CM_GEN7_5) || defined(CM_GEN8) || defined(CM_GEN8_5) ||            \
    defined(CM_GEN9) || defined(CM_GEN9_5)
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_dp2(vector<T1, SZ> src0, U src1,
                                           int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
  vector<float, SZ> _Src0 = src0;
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Result = details::__cm_intrinsic_impl_dp2(_Src0, _Src1);
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}
template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_dp2(matrix<T1, N1, N2> src0, U src1,
                                               int flag = _GENX_NOSAT) {
  vector<float, N1 *N2> _Src0 = src0;
  return cm_dp2<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_dp3(vector<T1, SZ> src0, U src1,
                                           int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
  vector<float, SZ> _Src0 = src0;
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Result = details::__cm_intrinsic_impl_dp3(_Src0, _Src1);
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}
template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_dp3(matrix<T1, N1, N2> src0, U src1,
                                               int flag = _GENX_NOSAT) {
  vector<float, N1 *N2> _Src0 = src0;
  return cm_dp3<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_dp4(vector<T1, SZ> src0, U src1,
                                           int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
  vector<float, SZ> _Src0 = src0;
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Result = details::__cm_intrinsic_impl_dp4(_Src0, _Src1);
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}
template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_dp4(matrix<T1, N1, N2> src0, U src1,
                                               int flag = _GENX_NOSAT) {
  vector<float, N1 *N2> _Src0 = src0;
  return cm_dp4<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename U, int SZ>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_dph(vector<T1, SZ> src0, U src1,
                                           int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
  vector<float, SZ> _Src0 = src0;
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Result = details::__cm_intrinsic_impl_dph(_Src0, _Src1);
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T0>(_Result);
}

template <typename T0, typename T1, typename U, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_dph(matrix<T1, N1, N2> src0, U src1,
                                               int flag = _GENX_NOSAT) {
  vector<float, N1 *N2> _Src0 = src0;
  return cm_dph<T0>(_Src0, src1, flag);
}

template <typename RT, typename T1, typename T2, int SZ>
CM_NODEBUG CM_INLINE vector<RT, SZ>
cm_line(vector<T1, 4> src0, vector<T2, SZ> src1, int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");

  vector<float, 4> _Src0 = src0;
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Result = details::__cm_intrinsic_impl_line(_Src0, _Src1);

  vector<RT, SZ> Result;
  if (flag == _GENX_SAT)
    Result = details::__cm_intrinsic_impl_sat<RT>(_Result);
  else
    Result = _Result;

  return Result;
}

template <typename RT, typename T1, typename T2, int N1, int N2>
CM_NODEBUG CM_INLINE vector<RT, N1 *N2>
cm_line(vector<T1, 4> src0, matrix<T2, N1, N2> src1, int flag = _GENX_NOSAT) {
  vector<T2, N1 *N2> _Src1 = src1;
  return cm_line<RT>(src0, _Src1, flag);
}

template <typename RT, typename T1, typename T2, int N1, int N2>
CM_NODEBUG CM_INLINE vector<RT, N1 *N2> cm_line(matrix<T1, 1, 4> src0,
                                                matrix<T2, N1, N2> src1,
                                                int flag = _GENX_NOSAT) {
  vector<T1, 4> _Src0 = src0;
  vector<T2, N1 *N2> _Src1 = src1;
  return cm_line<RT>(_Src0, _Src1, flag);
}

template <typename RT, typename T, int SZ>
CM_NODEBUG CM_INLINE vector<RT, SZ>
cm_line(float P, float Q, vector<T, SZ> src1, int flag = _GENX_NOSAT) {
  vector<float, 4> _Src0 = P;
  _Src0(3) = Q;
  return cm_line<RT>(_Src0, src1, flag);
}

template <typename RT, typename T, int N1, int N2>
CM_NODEBUG CM_INLINE vector<RT, N1 *N2>
cm_line(float P, float Q, matrix<T, N1, N2> src1, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src1 = src1;
  return cm_line<RT>(P, Q, _Src1, flag);
}

#else
// The old implementation is to generate vISA IRs for dp2/dp3/dp4/dph/line.
// Now We change to use direct mul/add, and hope to generate mad instructions
// at the end, to still get the performance as good as HW solution.
// We rely on "pragma unroll" to get better code.
// The only input and return types for these APIs are floats.
// In order to be able to use the old emu code, we keep the template argument
// for the type, although the type "T" can only be float.
// We use enable_if to force the float type only.
// If the gen is not specified we warn the programmer that they are potentially
// using a less efficient implementation if not on GEN10 or above.
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T1>::value &&std::is_floating_point<
        T1>::value &&details::is_fp_or_dword_type<U>::value &&
        std::is_floating_point<U>::value,
    vector<T0, SZ> >::type
cm_dp2(vector<T1, SZ> src0, U src1, int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
#if !defined(CM_GENX)
  CM_STATIC_WARNING(0, "GEN not specified so cm_dp2() code may not be optimal");
#endif
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Result;
#pragma unroll
  for (int i = 0; i < SZ; i += 4) {
    _Result.select<4,1>(i) = src0[i] * _Src1[i] + src0[i + 1] * _Src1[i + 1];
  }
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T1>(_Result);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T1>::value &&std::is_floating_point<
        T1>::value &&details::is_fp_or_dword_type<U>::value &&
        std::is_floating_point<U>::value,
    vector<T0, N1 *N2> >::type
cm_dp2(matrix<T1, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {
  vector<float, N1 *N2> _Src0 = src0;
  return cm_dp2<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T1>::value &&std::is_floating_point<
        T1>::value &&details::is_fp_or_dword_type<U>::value &&
        std::is_floating_point<U>::value,
    vector<T0, SZ> >::type
cm_dp3(vector<T1, SZ> src0, U src1, int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
#if !defined(CM_GENX)
  CM_STATIC_WARNING(0, "GEN not specified so cm_dp3() code may not be optimal");
#endif
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Result;
#pragma unroll
  for (int i = 0; i < SZ; i += 4) {
    _Result.select<4,1>(i) = src0[i] * _Src1[i] + src0[i + 1] * _Src1[i + 1] +
        src0[i + 2] * _Src1[i + 2];
  }
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T1>(_Result);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T1>::value &&std::is_floating_point<
        T1>::value &&details::is_fp_or_dword_type<U>::value &&
        std::is_floating_point<U>::value,
    vector<T0, N1 *N2> >::type
cm_dp3(matrix<T1, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_dp3<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T1>::value &&std::is_floating_point<
        T1>::value &&details::is_fp_or_dword_type<U>::value &&
        std::is_floating_point<U>::value,
    vector<T0, SZ> >::type
cm_dp4(vector<T1, SZ> src0, U src1, int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
#if !defined(CM_GENX)
  CM_STATIC_WARNING(0, "GEN not specified so cm_dp4() code may not be optimal");
#endif
  vector<T1, SZ> _Src1 = src1;
  vector<float, SZ> _Result;
#pragma unroll
  for (int i = 0; i < SZ; i += 4) {
    _Result.select<4,1>(i) = src0[i] * _Src1[i] + src0[i + 1] * _Src1[i + 1] +
        src0[i + 2] * _Src1[i + 2] + src0[i + 3] * _Src1[i + 3];
  }
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T1>(_Result);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T1>::value &&std::is_floating_point<
        T1>::value &&details::is_fp_or_dword_type<U>::value &&
        std::is_floating_point<U>::value,
    vector<T0, N1 *N2> >::type
cm_dp4(matrix<T1, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_dp4<T0>(_Src0, src1, flag);
}

template <typename T, typename U, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&std::is_floating_point<T>::value &&
        details::is_fp_or_dword_type<U>::value &&
            std::is_floating_point<U>::value,
    vector<T, SZ> >::type
cm_dph(vector<T, SZ> src0, U src1, int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
#if !defined(CM_GENX)
  CM_STATIC_WARNING(0, "GEN not specified so cm_dph() code may not be optimal");
#endif
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Result;
#pragma unroll
  for (int i = 0; i < SZ; i += 4) {
    _Result.select<4,1>(i) = src0[i] * _Src1[i] + src0[i + 1] * _Src1[i + 1] +
        src0[i + 2] * _Src1[i + 2] + 1.0 * _Src1[i + 3];
  }
  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <typename T, typename U, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&std::is_floating_point<T>::value &&
        details::is_fp_or_dword_type<U>::value &&
            std::is_floating_point<U>::value,
    vector<T, N1 *N2> >::type
cm_dph(matrix<T, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src0 = src0;
  return cm_dph<T>(_Src0, src1, flag);
}

template <typename T, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&std::is_floating_point<T>::value,
    vector<T, SZ> >::type
cm_line(vector<T, 4> src0, vector<T, SZ> src1, int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
#if !defined(CM_GENX)
  CM_STATIC_WARNING(0, "GEN not specified so cm_line() code may not be optimal");
#endif

  vector<T, SZ> _Src1 = src1;
  vector<T, SZ> _Result;
#pragma unroll
  for (int i = 0; i < SZ; i += 4) {
    _Result.select<4,1>(i) = src0[0] * src1[i] + src0[3];
  }

  if (flag == _GENX_SAT)
    _Result = details::__cm_intrinsic_impl_sat<T>(_Result);

  return _Result;
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&std::is_floating_point<T>::value,
    vector<T, N1 *N2> >::type
cm_line(vector<T, 4> src0, matrix<T, N1, N2> src1, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src1 = src1;
  return cm_line<T>(src0, _Src1, flag);
}

template <typename T, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&std::is_floating_point<T>::value,
    vector<T, SZ> >::type
cm_line(float P, float Q, vector<T, SZ> src1, int flag = _GENX_NOSAT) {
  vector<T, 4> _Src0 = P;
  _Src0(3) = Q;
  return cm_line<T>(_Src0, src1, flag);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&std::is_floating_point<T>::value,
    vector<T, N1 *N2> >::type
cm_line(float P, float Q, matrix<T, N1, N2> src1, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src1 = src1;
  return cm_line<T>(P, Q, _Src1, flag);
}

#endif

template <typename T, int SZ>
CM_NODEBUG CM_INLINE vector<T, SZ> cm_frc(vector<T, SZ> src0) {
  vector<float, SZ> _Src0 = src0;
  return details::__cm_intrinsic_impl_frc(_Src0);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T, N1 *N2> cm_frc(matrix<T, N1, N2> src0) {
  vector<T, N1 *N2> _Src0 = src0;
  return cm_frc<T>(_Src0);
}

template <typename T> CM_NODEBUG CM_INLINE T cm_frc(T src0) {
  vector<T, 1> _Src0 = src0;
  vector<T, 1> _Result = cm_frc<T>(_Src0);
  return _Result(0);
}

// cm_lzd
template <typename RT, typename T0, int SZ>
CM_NODEBUG CM_INLINE vector<RT, SZ> cm_lzd(vector<T0, SZ> src0,
                                           int flag = _GENX_NOSAT) {
  // Saturation parameter ignored
  vector<uint, SZ> _Src0 = src0;
  return details::__cm_intrinsic_impl_lzd<uint>(_Src0);
}

template <typename RT, typename T0, int N1, int N2>
CM_NODEBUG CM_INLINE vector<RT, N1 *N2> cm_lzd(matrix<T0, N1, N2> src0,
                                               int flag = _GENX_NOSAT) {
  vector<T0, N1 *N2> _Src0 = src0;
  return cm_lzd<RT>(_Src0, flag);
}

template <typename RT, typename T0>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<RT>::value &&details::is_cm_scalar<T0>::value,
    typename std::remove_const<RT>::type>::type
cm_lzd(T0 src0, int flag = _GENX_NOSAT) {
  vector<T0, 1> _Src0 = src0;
  vector<RT, 1> _Result = cm_lzd<RT>(_Src0);
  return _Result(0);
}

////////////////////////////////////////////////////////////////////////////////
// CM reduction intrinsics:
//
// cm_sum, cm_prod, cm_reduced_min and cm_reduced_max.
////////////////////////////////////////////////////////////////////////////////
template <typename T0, typename T1, int SZ>
CM_NODEBUG CM_INLINE T0 cm_sum(vector<T1, SZ> src, int flag = _GENX_NOSAT) {
  vector<T1, SZ> _PredSrc = details::__cm_intrinsic_impl_simdcf_predgen(src, 0);

  if (flag != _GENX_SAT)
    return details::__cm_intrinsic_impl_sum<T0>(_PredSrc);
  return details::__cm_intrinsic_impl_sum_sat<T0>(_PredSrc);
}
template <typename T0, typename T1, int N1, int N2>
CM_NODEBUG CM_INLINE T0 cm_sum(matrix<T1, N1, N2> src, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src = src;
  return cm_sum<T0>(_Src, flag);
}

template <typename T0, typename T1, int SZ>
CM_NODEBUG CM_INLINE T0 cm_prod(vector<T1, SZ> src, int flag = _GENX_NOSAT) {
  vector<T1, SZ> _PredSrc = details::__cm_intrinsic_impl_simdcf_predgen(src, 1);

  if (flag != _GENX_SAT)
    return details::__cm_intrinsic_impl_prod<T0>(_PredSrc);
  return details::__cm_intrinsic_impl_prod_sat<T0>(_PredSrc);
}
template <typename T0, typename T1, int N1, int N2>
CM_NODEBUG CM_INLINE T0
cm_prod(matrix<T1, N1, N2> src, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src = src;
  return cm_prod<T0>(_Src, flag);
}

template <typename T0, typename T1, int SZ>
CM_NODEBUG CM_INLINE T0
cm_reduced_min(vector<T1, SZ> src, int flag = _GENX_NOSAT) {
  vector<T1, SZ> _PredSrc = details::__cm_intrinsic_impl_simdcf_predmin(src);

  T1 _Res = details::__cm_intrinsic_impl_reduced_min(_PredSrc);
  if (flag != _GENX_SAT)
    return _Res;

  return details::__cm_intrinsic_impl_sat<T0>(_Res);
}
template <typename T0, typename T1, int N1, int N2>
CM_NODEBUG CM_INLINE T0
cm_reduced_min(matrix<T1, N1, N2> src, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src = src;
  return cm_reduced_min<T0>(_Src, flag);
}

template <typename T0, typename T1, int SZ>
CM_NODEBUG CM_INLINE T0
cm_reduced_max(vector<T1, SZ> src, int flag = _GENX_NOSAT) {
  vector<T1, SZ> _PredSrc = details::__cm_intrinsic_impl_simdcf_predmax(src);

  T1 _Res = details::__cm_intrinsic_impl_reduced_max(_PredSrc);
  if (flag != _GENX_SAT)
    return _Res;

  return details::__cm_intrinsic_impl_sat<T0>(_Res);
}
template <typename T0, typename T1, int N1, int N2>
CM_NODEBUG CM_INLINE T0
cm_reduced_max(matrix<T1, N1, N2> src, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src = src;
  return cm_reduced_max<T0>(_Src);
}

// cm_sad2
template <typename T0, typename T1, int SZ, typename U>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_sad2(vector<T1, SZ> src0, U src1,
                                            int flag = _GENX_NOSAT) {
  typedef typename details::byte_type<T1>::type _BT1;
  vector<_BT1, SZ> _Src0 = src0;
  vector<_BT1, SZ> _Src1 = src1;
  return details::__cm_intrinsic_impl_sad2<T0, _BT1, SZ>(_Src0, _Src1);
}

template <typename T0, typename T1, int N1, int N2, typename U>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_sad2(matrix<T1, N1, N2> src0, U src1,
                                                int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_sad2<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value,
    typename std::remove_const<T0>::type>::type
cm_sad2(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  typedef typename details::byte_type<T1>::type _BT1;
  vector<_BT1, 1> _Src0 = src0;
  vector<_BT1, 1> _Src1 = src1;
  vector<T0, 1> _Result = cm_sad2<T0>(_Src0, _Src1, flag);
  return _Result(0);
}

template <typename T0, typename T1, typename T2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T1>::value &&details::is_cm_vector<T2>::value,
    decltype(cm_sad2<T0>(T2(), T1()))>::type
cm_sad2(T1 src0, T2 src1, int flag = _GENX_NOSAT) {
  return cm_sad2<T0>(src1, src0, flag);
}

// cm_sada2
template <typename T0, typename T1, int SZ, typename U, typename V>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_integral<T0>::value &&std::is_integral<T1>::value,
    vector<T0, SZ> >::type
cm_sada2(vector<T1, SZ> src0, U src1, V src2, int flag = _GENX_NOSAT) {
  typedef typename details::byte_type<T1>::type _BT1;
  typedef typename details::word_type<T0>::type _WT0;
  CM_STATIC_ERROR(
      std::is_integral<_BT1>::value && sizeof(_BT1) == sizeof(char) &&
          std::is_integral<_WT0>::value && sizeof(_WT0) == sizeof(short),
      "operand conversion failed");
  vector<_BT1, SZ> _Src0 = src0;
  vector<_BT1, SZ> _Src1 = src1;
  vector<_WT0, SZ> _Src2 = src2;
  return details::__cm_intrinsic_impl_sada2<T0>(_Src0, _Src1, _Src2, flag);
}

template <typename T0, typename T1, int N1, int N2, typename U, typename V>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2>
cm_sada2(matrix<T1, N1, N2> src0, U src1, V src2, int flag = _GENX_NOSAT) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_sada2<T0>(_Src0, src1, src2, flag);
}

template <typename T0, typename T1, typename T2, typename T3>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value &&
        details::is_cm_scalar<T2>::value &&details::is_cm_scalar<T3>::value,
    typename std::remove_const<T0>::type>::type
cm_sada2(T1 src0, T2 src1, T3 src2, int flag = _GENX_NOSAT) {
  typedef typename details::byte_type<T1>::type _BT1;
  typedef typename details::word_type<T0>::type _WT0;
  CM_STATIC_ERROR(
      std::is_integral<_BT1>::value && sizeof(_BT1) == sizeof(char) &&
          std::is_integral<_WT0>::value && sizeof(_WT0) == sizeof(short),
      "operand conversion failed");
  vector<_BT1, 1> _Src0 = src0;
  vector<_BT1, 1> _Src1 = src1;
  vector<_WT0, 1> _Src2 = src2;
  vector<_WT0, 1> _Result = cm_sada2<T0>(_Src0, _Src1, _Src2, flag);
  return _Result(0);
}

template <typename T0, typename T1, typename T2, typename T3>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T1>::value &&details::is_cm_vector<T2>::value,
    decltype(cm_sada2<T0>(T2(), T1(), T3()))>::type
cm_sada2(T1 src0, T2 src1, T3 src2, int flag = _GENX_NOSAT) {
  return cm_sada2<T0>(src1, src0, src2, flag);
}

// cm_lrp
#if defined(CM_GEN7_5) || defined(CM_GEN8) || defined(CM_GEN8_5) ||            \
    defined(CM_GEN9) || defined(CM_GEN9_5)

template <int SZ, typename U, typename V>
CM_NODEBUG CM_INLINE vector<float, SZ> cm_lrp(vector<float, SZ> src0, U src1,
                                              V src2, int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ >= 4 && (SZ & 0x3) == 0,
                  "vector size must be a multiple of 4");
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Src2 = src2;
  vector<float, SZ> _Result =
      details::__cm_intrinsic_impl_lrp<SZ>(src0, _Src1, _Src2);

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<float>(_Result);
}

#else

// The old implementation is to generate vISA IRs for lrp.
// Now We change to use direct mul/add, and hope to generate mad instructions
// at the end, to still get the performance as good as HW solution. 
// The only input and return types for these APIs are floats.
// In order to be able to use the old emu code, we keep the template argument
// for the type, although the type "T" can only be float.
// We use enable_if to force the float type only.
// If the gen is not specified we warn the programmer that they are potentially
// using less efficient implementation.
template <typename T, int SZ, typename U, typename V>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&std::is_floating_point<T>::value &&
        details::is_fp_or_dword_type<U>::value &&
            std::is_floating_point<U>::value,
    vector<T, SZ> >::type
cm_lrp(vector<T, SZ> src0, U src1, V src2, int flag = _GENX_NOSAT) {
#if !defined(CM_GENX)
  CM_STATIC_WARNING(0, "GEN not specified so cm_lrp() code may not be optimal");
#endif
  vector<float, SZ> _Src1 = src1;
  vector<float, SZ> _Src2 = src2;
  vector<float, SZ> _Result;
  _Result = _Src1 * src0 + _Src2 * (1.0f - src0);
  if (flag != _GENX_SAT)
    return _Result;
  return details::__cm_intrinsic_impl_sat<T>(_Result);
}
#endif

template <int N1, int N2, typename U, typename V>
CM_NODEBUG CM_INLINE vector<float, N1 *N2>
cm_lrp(matrix<float, N1, N2> src0, U src1, V src2, int flag = _GENX_NOSAT) {
  vector<float, N1 *N2> _Src0 = src0;
  return cm_lrp(_Src0, src1, src2, flag);
}

// cm_pln
template <int SZ>
CM_NODEBUG CM_INLINE vector<float, SZ>
cm_pln(vector<float, 4> src0, vector<float, SZ> src1, vector<float, SZ> src2,
       int flag = _GENX_NOSAT) {
  CM_STATIC_ERROR(SZ >= 8 && (SZ & 0x7) == 0,
                  "vector size must be a multiple of 8");

  // __cm_intrinsic_impl_pln() requires src1 and src2 to be combined into
  // a single matrix, interleaving the values together in blocks of 8
  // items (ie, a block of 8 from src1, then a block of 8 from src2, then
  // the next block of 8 from src1, then the next block of 8 from src2,
  // and so-on.)
  matrix<float, (SZ >> 3), 16> _Src12;

  _Src12.select<(SZ >> 3), 1, 8, 1>(0, 0) = src1.format<float, (SZ >> 3), 8>();
  _Src12.select<(SZ >> 3), 1, 8, 1>(0, 8) = src2.format<float, (SZ >> 3), 8>();

  vector<float, SZ> _Result =
      details::__cm_intrinsic_impl_pln<SZ>(src0, _Src12.format<float>());

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<float>(_Result);
}

template <int N1, int N2>
CM_NODEBUG CM_INLINE vector<float, N1 *N2>
cm_pln(matrix<float, 1, 4> src0, matrix<float, N1, N2> src1,
       matrix<float, N1, N2> src2, int flag = _GENX_NOSAT) {
  vector<float, 4> _Src0 = src0;
  vector<float, N1 *N2> _Src1 = src1;
  vector<float, N1 *N2> _Src2 = src2;
  return cm_pln(_Src0, _Src1, _Src2, flag);
}

// cm_bf_reverse
template <typename T0, typename T1, int SZ>
CM_NODEBUG CM_INLINE vector<T0, SZ> cm_bf_reverse(vector<T1, SZ> src0) {
  vector<unsigned, SZ> _Src0 = src0;
  return details::__cm_intrinsic_impl_bfrev<unsigned>(_Src0);
}

template <typename T0, typename T1, int N1, int N2>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_bf_reverse(matrix<T1, N1, N2> src0) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_bf_reverse<T0>(_Src0);
}

template <typename T0, typename T1>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T1>::value,
    typename std::remove_const<T0>::type>::type
cm_bf_reverse(T1 src0) {
  vector<T1, 1> _Src0 = src0;
  vector<T0, 1> _Result = cm_bf_reverse<T0>(_Src0);
  return _Result(0);
}

// cm_bf_insert
template <typename T0, typename T1, int SZ, typename U, typename V, typename W>
CM_NODEBUG CM_INLINE typename std::enable_if<std::is_integral<T1>::value,
                                             vector<T0, SZ> >::type
cm_bf_insert(U src0, V src1, W src2, vector<T1, SZ> src3) {
  typedef typename details::dword_type<T1>::type _DT1;
  CM_STATIC_ERROR(std::is_integral<_DT1>::value && sizeof(_DT1) == sizeof(int),
                  "operand conversion failed");
  vector<_DT1, SZ> _Src0 = src0;
  vector<_DT1, SZ> _Src1 = src1;
  vector<_DT1, SZ> _Src2 = src2;
  vector<_DT1, SZ> _Src3 = src3;

  return details::__cm_intrinsic_impl_bfins<_DT1>(_Src0, _Src1, _Src2, _Src3);
}

template <typename T0, typename T1, int N1, int N2, typename U, typename V,
          typename W>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_bf_insert(U src0, V src1, W src2,
                                                     matrix<T1, N1, N2> src3) {
  vector<T1, N1 *N2> _Src3 = src3;
  return cm_bf_insert<T0>(src0, src1, src2, _Src3);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T4>::value,
    typename std::remove_const<T0>::type>::type
cm_bf_insert(T1 src0, T2 src1, T3 src2, T4 src3) {
  vector<T4, 1> _Src3 = src3;
  vector<T0, 1> _Result = cm_bf_insert<T0>(src0, src1, src2, _Src3);
  return _Result(0);
}

// cm_bf_extract
template <typename T0, typename T1, int SZ, typename U, typename V>
CM_NODEBUG CM_INLINE typename std::enable_if<std::is_integral<T1>::value,
                                             vector<T0, SZ> >::type
cm_bf_extract(U src0, V src1, vector<T1, SZ> src2) {
  typedef typename details::dword_type<T1>::type _DT1;
  CM_STATIC_ERROR(std::is_integral<_DT1>::value && sizeof(_DT1) == sizeof(int),
                  "operand conversion failed");
  vector<_DT1, SZ> _Src0 = src0;
  vector<_DT1, SZ> _Src1 = src1;
  vector<_DT1, SZ> _Src2 = src2;

  return details::__cm_intrinsic_impl_bfext<_DT1>(_Src0, _Src1, _Src2);
}

template <typename T0, typename T1, int N1, int N2, typename U, typename V>
CM_NODEBUG CM_INLINE vector<T0, N1 *N2> cm_bf_extract(U src0, V src1,
                                                      matrix<T1, N1, N2> src2) {
  vector<T1, N1 *N2> _Src2 = src2;
  return cm_bf_extract<T0>(src0, src1, _Src2);
}

template <typename T0, typename T1, typename T2, typename T3>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_cm_scalar<T0>::value &&details::is_cm_scalar<T3>::value,
    typename std::remove_const<T0>::type>::type
cm_bf_extract(T1 src0, T2 src1, T3 src2) {
  vector<T3, 1> _Src2 = src2;
  vector<T0, 1> _Result = cm_bf_extract<T0>(src0, src1, _Src2);
  return _Result(0);
}

////////////////////////////////////////////////////////////////////////////////
// CM arithmetic intrinsics:
//
// inv, log, exp, sqrt, rsqrt, sin, cos
//
// share the same requirements.
//
// template <int SZ>
// vector<float, SZ>
// CM_INLINE cm_inv(vector<float, SZ> src0, int flag = _GENX_NOSAT) {
//   vector<float, SZ> _Result = details::__cm_intrinsic_impl_inv(src0);
//   if (flag != _GENX_SAT)
//     return _Result;
//   return details::__cm_intrinsic_impl_sat<float>(_Result);
// }
//
// template <int N1, int N2>
// CM_NODEBUG CM_INLINE
// vector<float, N1 * N2>
// cm_inv(matrix<float, N1, N2> src0, int flag = _GENX_NOSAT) {
//   vector<float, N1 * N2> _Src0 = src0;
//   return cm_inv(_Src0, flag);
// }
//
// CM_INLINE float cm_inv(float src0, int flag = _GENX_NOSAT) {
//   vector<float, 1> _Src0 = src0;
//   vector<float, 1> _Result = cm_inv(_Src0, flag);
//   return _Result(0);
// }
//
// we also make the scalar version template-based by adding
// a "typename T". Since the type can only be float, we hack it
// by defining T=void without instantiating it to be float.

#define _CM_INTRINSIC_DEF(type, name)                                          \
  template <int SZ>                                                            \
  CM_NODEBUG CM_INLINE                                                         \
  vector<type, SZ> cm_##name(vector<type, SZ> src0, int flag = _GENX_NOSAT) {  \
    vector<type, SZ> _Result = details::__cm_intrinsic_impl_##name(src0);      \
    if (flag != _GENX_SAT)                                                     \
      return _Result;                                                          \
    return details::__cm_intrinsic_impl_sat<type>(_Result);                    \
  }                                                                            \
  template <int N1, int N2>                                                    \
  CM_NODEBUG CM_INLINE vector<type, N1 *N2> cm_##name(                         \
      matrix<type, N1, N2> src0, int flag = _GENX_NOSAT) {                     \
    vector<type, N1 *N2> _Src0 = src0;                                         \
    return cm_##name(_Src0, flag);                                             \
  }                                                                            \
  template <typename T = void>                                                 \
  CM_NODEBUG CM_INLINE type cm_##name(type src0, int flag = _GENX_NOSAT) {     \
    vector<type, 1> _Src0 = src0;                                              \
    vector<type, 1> _Result = cm_##name(_Src0, flag);                          \
    return _Result(0);                                                         \
  }

_CM_INTRINSIC_DEF(half, inv)
_CM_INTRINSIC_DEF(half, log)
_CM_INTRINSIC_DEF(half, exp)
_CM_INTRINSIC_DEF(half, sqrt)
_CM_INTRINSIC_DEF(half, rsqrt)
_CM_INTRINSIC_DEF(half, sin)
_CM_INTRINSIC_DEF(half, cos)

_CM_INTRINSIC_DEF(float, inv)
_CM_INTRINSIC_DEF(float, log)
_CM_INTRINSIC_DEF(float, exp)
_CM_INTRINSIC_DEF(float, sqrt)
_CM_INTRINSIC_DEF(float, sqrt_ieee)
_CM_INTRINSIC_DEF(float, rsqrt)
_CM_INTRINSIC_DEF(float, sin)
_CM_INTRINSIC_DEF(float, cos)

_CM_INTRINSIC_DEF(double, sqrt_ieee)

#undef _CM_INTRINSIC_DEF

#define _CM_INTRINSIC_DEF(ftype, name)                                         \
  template <int SZ, typename U>                                                \
  CM_NODEBUG CM_INLINE vector<ftype, SZ> cm_##name(                            \
      vector<ftype, SZ> src0, U src1, int flag = _GENX_NOSAT) {                \
    vector<ftype, SZ> _Src1 = src1;                                            \
    vector<ftype, SZ> _Result =                                                \
        details::__cm_intrinsic_impl_##name(src0, _Src1);                      \
    if (flag != _GENX_SAT)                                                     \
      return _Result;                                                          \
                                                                               \
    return details::__cm_intrinsic_impl_sat<ftype>(_Result);                   \
  }                                                                            \
  template <int N1, int N2, typename U>                                        \
  CM_NODEBUG CM_INLINE vector<ftype, N1 *N2> cm_##name(                        \
      matrix<ftype, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {            \
    vector<ftype, N1 *N2> _Src0 = src0;                                        \
    vector<ftype, N1 *N2> _Src1 = src1;                                        \
    return cm_##name(_Src0, _Src1, flag);                                      \
  }                                                                            \
  template <int SZ, typename U>                                                \
  CM_NODEBUG CM_INLINE typename std::enable_if<                                \
      details::is_cm_scalar<U>::value, vector<ftype, SZ> >::type               \
  cm_##name(U src0, vector<ftype, SZ> src1, int flag = _GENX_NOSAT) {          \
    vector<ftype, SZ> _Src0 = src0;                                            \
    return cm_##name(_Src0, src1, flag);                                       \
  }                                                                            \
  template <int N1, int N2, typename U>                                        \
  CM_NODEBUG CM_INLINE typename std::enable_if<                                \
      details::is_cm_scalar<U>::value, vector<ftype, N1 *N2> >::type           \
  cm_##name(U src0, matrix<ftype, N1, N2> src1, int flag = _GENX_NOSAT) {      \
    vector<ftype, N1 *N2> _Src0 = src0;                                        \
    vector<ftype, N1 *N2> _Src1 = src1;                                        \
    return cm_##name(_Src0, _Src1, flag);                                      \
  }                                                                            \
  CM_NODEBUG CM_INLINE ftype                                                   \
  cm_##name(ftype src0, ftype src1, int flag = _GENX_NOSAT) {                  \
    vector<ftype, 1> _Src0 = src0;                                             \
    vector<ftype, 1> _Src1 = src1;                                             \
    vector<ftype, 1> _Result = cm_##name(_Src0, _Src1, flag);                  \
    return _Result(0);                                                         \
  }

_CM_INTRINSIC_DEF(half, pow)
_CM_INTRINSIC_DEF(float, pow)

_CM_INTRINSIC_DEF(float, div_ieee)
_CM_INTRINSIC_DEF(double, div_ieee)

#undef _CM_INTRINSIC_DEF

// cm_sincos
template <int SZ, typename U>
CM_NODEBUG CM_INLINE vector<float, SZ>
cm_sincos(vector_ref<float, SZ> dstcos, U src0, int flag = _GENX_NOSAT) {
  dstcos = cm_cos(src0, flag);
  return cm_sin(src0, flag);
}

template <int N1, int N2, typename U>
CM_NODEBUG CM_INLINE matrix<float, N1, N2>
cm_sincos(matrix_ref<float, N1, N2> dstcos, U src0, int flag = _GENX_NOSAT) {
  dstcos = cm_cos(src0, flag);
  return cm_sin(src0, flag);
}

template <int SZ, typename U>
CM_NODEBUG CM_INLINE vector<half, SZ>
cm_sincos(vector_ref<half, SZ> dstcos, U src0, int flag = _GENX_NOSAT) {
  dstcos = cm_cos(src0, flag);
  return cm_sin(src0, flag);
}

template <int N1, int N2, typename U>
CM_NODEBUG CM_INLINE matrix<half, N1, N2>
cm_sincos(matrix_ref<half, N1, N2> dstcos, U src0, int flag = _GENX_NOSAT) {
  dstcos = cm_cos(src0, flag);
  return cm_sin(src0, flag);
}

// cm_atan

#define _CM_HDR_CONST_PI 3.1415926535897932384626433832795

template <typename T, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_float_or_half<T>::value, vector<T, SZ> >::type
cm_atan(vector<T, SZ> src0, int flag = _GENX_NOSAT) {
  vector<T, SZ> _Src0 = cm_abs(src0);

  vector<ushort, SZ> _Neg = src0 < T(0.0);
  vector<ushort, SZ> _Gt1 = _Src0 > T(1.0);

  _Src0.merge(cm_inv(_Src0), _Gt1);

  vector<T, SZ> _Src0P2 = _Src0 * _Src0;
  vector<T, SZ> _Src0P4 = _Src0P2 * _Src0P2;

  vector<T, SZ> _Result =
      (_Src0P4 * T(0.185696) + ((_Src0 * T(0.787997) + T(0.63693)) * _Src0P2) +
       _Src0) /
      (((((_Src0 * -T(0.000121387) + T(0.00202308)) * _Src0P2) +
         (_Src0 * -T(0.0149145)) + T(0.182569)) *
        _Src0P4) +
       ((_Src0 * T(0.395889) + T(1.12158)) * _Src0P2) + (_Src0 * T(0.636918)) +
       T(1.0));

  _Result.merge(T(_CM_HDR_CONST_PI / 2.0) - _Result, _Gt1);
  _Result.merge(-_Result, _Neg);

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_float_or_half<T>::value, vector<T, N1 *N2> >::type
cm_atan(matrix<T, N1, N2> src0, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src0 = src0;
  return cm_atan(_Src0, flag);
}

template <typename T>
CM_NODEBUG
CM_INLINE typename std::enable_if<details::is_float_or_half<T>::value, T>::type
cm_atan(T src0, int flag = _GENX_NOSAT) {
  vector<T, 1> _Src0 = src0;
  vector<T, 1> _Result = cm_atan(_Src0, flag);
  return _Result(0);
}

// cm_acos

template <typename T, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_float_or_half<T>::value, vector<T, SZ> >::type
cm_acos(vector<T, SZ> src0, int flag = _GENX_NOSAT) {
  vector<T, SZ> _Src0 = cm_abs(src0);

  vector<ushort, SZ> _Neg = src0 < T(0.0);
  vector<ushort, SZ> _TooBig = _Src0 >= T(0.999998);

  // Replace oversized values to ensure no possibility of sqrt of
  // a negative value later
  _Src0.merge(T(0.0), _TooBig);

  vector<T, SZ> _Src01m = T(1.0) - _Src0;

  vector<T, SZ> _Src0P2 = _Src01m * _Src01m;
  vector<T, SZ> _Src0P4 = _Src0P2 * _Src0P2;

  vector<T, SZ> _Result =
      (((_Src01m * T(0.015098965761299077) - T(0.005516443930088506)) *
        _Src0P4) +
       ((_Src01m * T(0.047654245891495528) + T(0.163910606547823220)) *
        _Src0P2) +
       _Src01m * T(2.000291665285952400) - T(0.000007239283986332)) *
      cm_rsqrt(_Src01m * T(2.0));

  _Result.merge(T(0.0), _TooBig);
  _Result.merge(T(_CM_HDR_CONST_PI) - _Result, _Neg);

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_float_or_half<T>::value, vector<T, N1 *N2> >::type
cm_acos(matrix<T, N1, N2> src0, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src0 = src0;
  return cm_acos(_Src0, flag);
}

template <typename T>
CM_NODEBUG
CM_INLINE typename std::enable_if<details::is_float_or_half<T>::value, T>::type
cm_acos(T src0, int flag = _GENX_NOSAT) {
  vector<T, 1> _Src0 = src0;
  vector<T, 1> _Result = cm_acos(_Src0, flag);
  return _Result(0);
}

// cm_asin

template <typename T, int SZ>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_float_or_half<T>::value, vector<T, SZ> >::type
cm_asin(vector<T, SZ> src0, int flag = _GENX_NOSAT) {
  vector<ushort, SZ> _Neg = src0 < T(0.0);

  vector<T, SZ> _Result = T(_CM_HDR_CONST_PI / 2.0) - cm_acos(cm_abs(src0));

  _Result.merge(-_Result, _Neg);

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_float_or_half<T>::value, vector<T, N1 *N2> >::type
cm_asin(matrix<T, N1, N2> src0, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src0 = src0;
  return cm_asin(_Src0, flag);
}

template <typename T>
CM_NODEBUG
CM_INLINE typename std::enable_if<details::is_float_or_half<T>::value, T>::type
cm_asin(T src0, int flag = _GENX_NOSAT) {
  vector<T, 1> _Src0 = src0;
  vector<T, 1> _Result = cm_asin(_Src0, flag);
  return _Result(0);
}

////////////////////////////////////////////////////////////////////////////////
// Rounding intrinsics.
////////////////////////////////////////////////////////////////////////////////

#define _CM_INTRINSIC_DEF(name)                                                \
  template <typename T, int SZ>                                                \
  CM_NODEBUG CM_INLINE                                                         \
  vector<T, SZ> cm_##name(vector<float, SZ> src0, int flag = _GENX_NOSAT) {    \
    vector<float, SZ> _Result = details::__cm_intrinsic_impl_##name(src0);     \
    if (flag != _GENX_SAT)                                                     \
      return _Result;                                                          \
    return details::__cm_intrinsic_impl_sat<T>(_Result);                       \
  }                                                                            \
  template <typename T, int N1, int N2>                                        \
  CM_NODEBUG CM_INLINE vector<T, N1 *N2> cm_##name(matrix<float, N1, N2> src0, \
                                                   int flag = _GENX_NOSAT) {   \
    vector<float, N1 *N2> _Src0 = src0;                                        \
    return cm_##name<T>(_Src0, flag);                                          \
  }                                                                            \
  template <typename T>                                                        \
  CM_NODEBUG CM_INLINE T cm_##name(float src0, int flag = _GENX_NOSAT) {       \
    vector<float, 1> _Src0 = src0;                                             \
    vector<T, 1> _Result = cm_##name<T>(_Src0, flag);                          \
    return _Result(0);                                                         \
  }

_CM_INTRINSIC_DEF(rndd)
_CM_INTRINSIC_DEF(rndu)
_CM_INTRINSIC_DEF(rnde)
_CM_INTRINSIC_DEF(rndz)

#undef _CM_INTRINSIC_DEF

template <int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N == 8 || N == 16 || N == 32), uint>::type
cm_pack_mask(vector<ushort, N> src0) {
  return details::__cm_intrinsic_impl_pack_mask(src0);
}

template <int N1, int N2>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N1 *N2 == 8 || N1 *N2 == 16 || N1 * N2 == 32), uint>::type
cm_pack_mask(matrix<ushort, N1, N2> src0) {
  return details::__cm_intrinsic_impl_pack_mask(src0);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N == 8 || N == 16 || N == 32), vector<ushort, N> >::type
cm_unpack_mask(uint src0) {
  return details::__cm_intrinsic_impl_unpack_mask<T, N>(src0);
}

template <int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N != 8 && N != 16 && N < 32), uint>::type
cm_pack_mask(vector<ushort, N> src0) {
  vector<ushort, (N < 8 ? 8 : N < 16 ? 16 : 32)> _src0 = 0;
  _src0.template select<N, 1>() = src0.template format<ushort>();
  return cm_pack_mask(_src0);
}

template <int N1, int N2>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N1 *N2 != 8 && N1 * N2 != 16 && N1 * N2 < 32), uint>::type
cm_pack_mask(matrix<ushort, N1, N2> src0) {
  vector < ushort, (N1 *N2) < 8 ? 8 : ((N1 *N2) < 16 ? 16 : 32) > _src0 = 0;
  _src0.template select<N1 *N2, 1>() = src0.template format<ushort>();
  return cm_pack_mask(_src0);
}

/// Count component-wise the total bits set in source operand.
template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<std::is_integral<T>::value, vector<uint, N> >::type
cm_cbit(vector<T, N> src0) {
  return details::__cm_intrinsic_impl_cbit(src0);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE
typename std::enable_if<std::is_integral<T>::value, vector<uint, N1 *N2> >::type
cm_cbit(matrix<T, N1, N2> src) {
  vector<T, N1 *N2> _Src = src;
  return cm_cbit(_Src);
}

template <typename T>
CM_NODEBUG CM_INLINE
typename std::enable_if<std::is_integral<T>::value &&
                        details::is_cm_scalar<T>::value, uint>::type
cm_cbit(T src) {
  vector<T, 1> _Src = src;
  vector<uint, 1> _Result = cm_cbit(_Src);
  return _Result(0);
}

/// Find component-wise the first bit from LSB side
template <int N>
CM_NODEBUG CM_INLINE
vector<unsigned, N> cm_fbl(vector<unsigned, N> src) {
  return details::__cm_intrinsic_impl_fbl(src);
}

template <int N1, int N2>
CM_NODEBUG CM_INLINE
vector<unsigned, N1 * N2>
cm_fbl(matrix<unsigned, N1, N2> src) {
  vector<unsigned, N1 * N2> _Src = src;
  return cm_fbl(_Src);
}

template <typename T = void>
CM_NODEBUG CM_INLINE
unsigned cm_fbl(unsigned src) {
  vector<unsigned, 1> _Src = src;
  vector<unsigned, 1> _Result = cm_fbl(_Src);
  return _Result(0);
}

/// Find component-wise the first bit from MSB side.
template <int N>
CM_NODEBUG CM_INLINE vector<int, N> cm_fbh(vector<int, N> src) {
  return details::__cm_intrinsic_impl_sfbh(src);
}

template <int N>
CM_NODEBUG CM_INLINE vector<unsigned int, N>
cm_fbh(vector<unsigned int, N> src) {
  return details::__cm_intrinsic_impl_ufbh(src);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<details::is_dword_type<T>::value,
                                             vector<T, N1 *N2> >::type
cm_fbh(matrix<T, N1, N2> src) {
  vector<T, N1 *N2> _Src = src;
  return cm_fbh(_Src);
}

template <typename T>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_dword_type<T>::value &&details::is_cm_scalar<T>::value, T>::type
cm_fbh(T src) {
  vector<T, 1> _Src = src;
  vector<T, 1> _Result = cm_fbh(_Src);
  return _Result(0);
}

template <typename T = void> vector<uint, 4> cm_rdtsc();

//////////////////////////////////////////
// Legacy f16 to f32 and vice versa
//////////////////////////////////////////

// Not really required for cmc as we support half types now
// Provided for backward compatibility
template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_word_type<T>::value &&details::is_cm_scalar<T>::value,
    vector<float, N> >::type
cm_f16tof32(const vector<T, N> src) {
  vector<float, N> _ret = src.format<half>();
  return _ret;
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_word_type<T>::value &&details::is_cm_scalar<T>::value,
    matrix<float, N1, N2> >::type
cm_f16tof32(const matrix<T, N1, N2> src) {
  matrix<float, N1, N2> _ret = src.format<half>();
  return _ret;
}

template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_floating_point<T>::value &&details::is_cm_scalar<T>::value,
    vector<ushort, N> >::type
cm_f32tof16(const vector<T, N> src) {
  vector<half, N> _ret = src;
  return _ret.format<ushort>();
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<
    std::is_floating_point<T>::value &&details::is_cm_scalar<T>::value,
    matrix<ushort, N1, N2> >::type
cm_f32tof16(const matrix<T, N1, N2> src) {
  matrix<half, N1, N2> _ret = src;
  return _ret.format<ushort>();
}


////////////////////////////////////////////////////////////////////////////////
// SIMD control follow related macros.
////////////////////////////////////////////////////////////////////////////////
#ifndef _SIMD_CF_
#define _SIMD_CF_

// If-Else Statement
#define SIMD_IF_BEGIN(...)                                                     \
  if (details::__cm_intrinsic_impl_simdcf_any((__VA_ARGS__), __FILE__,         \
                                              __LINE__)) {

#define SIMD_ELSEIF(...)                                                       \
  }                                                                            \
  else if (details::__cm_intrinsic_impl_simdcf_any((__VA_ARGS__), __FILE__,    \
                                                   __LINE__)) {

#define SIMD_ELSE                                                              \
  }                                                                            \
  else {
#define SIMD_IF_END }

// Do-While Statement
#define SIMD_DO_WHILE_BEGIN do {
#define SIMD_DO_WHILE_END(...)                                                 \
  }                                                                            \
  while (details::__cm_intrinsic_impl_simdcf_any((__VA_ARGS__), __FILE__,      \
                                                 __LINE__))

#define SIMD_BREAK break
#define SIMD_CONTINUE continue

#endif // _SIMD_CF_

#endif // _CLANG_CM_H_
