/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(
    0, "CM:w:intrinsics/rsqrt.h should not be included explicitly - only "
       "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_INTRINSICS_RSQRT_H_
#define _CLANG_CM_INTRINSICS_RSQRT_H_

#include <cm/cm_common.h>
#include <cm/cm_has_instr.h>
#include <cm/cm_traits.h>
#include <cm/spirv/math.h>

// cm_rsqrt
namespace details {
template <typename Ty> struct rsqrt;

template <> struct rsqrt<double> { using type = double; };

template <typename Ty> using rsqrt_t = typename rsqrt<Ty>::type;
} // namespace details

template <typename T, int SZ>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA vector<details::rsqrt_t<T>, SZ>
cm_rsqrt(vector<T, SZ> src) {
  return __spirv_ocl_rsqrt(src);
}

// Scalar
template <typename T>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA details::rsqrt_t<T> cm_rsqrt(T src) {
  vector<T, 1> _Result = __spirv_ocl_rsqrt(src);
  return _Result(0);
}

template <typename T, int N1, int N2>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA matrix<details::rsqrt_t<T>, N1, N2>
cm_rsqrt(matrix<T, N1, N2> src) {
  vector<T, N1 *N2> Src = src;
  return cm_rsqrt(Src);
}

#endif // _CLANG_CM_INTRINSICS_RSQRT_H_
