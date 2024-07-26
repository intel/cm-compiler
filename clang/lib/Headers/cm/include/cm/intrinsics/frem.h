/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0,
              "CM:w:intrinsics/frem.h should not be included explicitly - only "
              "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_INTRINSICS_FREM_H_
#define _CLANG_CM_INTRINSICS_FREM_H_

#include <cm/cm_common.h>
#include <cm/cm_has_instr.h>
#include <cm/cm_traits.h>
#include <cm/spirv/math.h>

// cm_frem
namespace details {
template <typename Ty> struct frem;

template <> struct frem<float> { using type = float; };

template <> struct frem<double> { using type = double; };

template <typename Ty> using frem_t = typename frem<Ty>::type;
} // namespace details

template <typename T, int SZ>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA vector<details::frem_t<T>, SZ>
cm_frem(vector<T, SZ> src0, vector<T, SZ> src1) {
  return __spirv_FRem(src0, src1);
}

// Scalar
template <typename T>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA details::frem_t<T> cm_frem(T src0,
                                                                    T src1) {
  vector<T, 1> _Result = __spirv_FRem(src0, src1);
  return _Result(0);
}

template <typename T, int N>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA vector<details::frem_t<T>, N>
cm_frem(T src0, vector<T, N> src1) {
  vector<T, N> Src0 = src0;
  return cm_frem(Src0, src1);
}

template <typename T, int N>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA vector<T, N> cm_frem(vector<T, N> src0,
                                                              T src1) {
  vector<T, N> Src1 = src1;
  return cm_frem(src0, Src1);
}

template <typename T, int N1, int N2>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA matrix<details::frem_t<T>, N1, N2>
cm_frem(matrix<T, N1, N2> src0, matrix<T, N1, N2> src1) {
  vector<T, N1 *N2> Src0 = src0;
  vector<T, N1 *N2> Src1 = src1;
  return cm_frem(Src0, Src1);
}

template <typename T, int N1, int N2>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA matrix<details::frem_t<T>, N1, N2>
cm_frem(T src0, matrix<T, N1, N2> src1) {
  matrix<T, N1, N2> Src0 = src0;
  return cm_frem(Src0, src1);
}

template <typename T, int N1, int N2>
CM_NODEBUG __SPIRV_WRITER_INLINE_WA matrix<T, N1, N2>
cm_frem(matrix<T, N1, N2> src0, T src1) {
  matrix<T, N1, N2> Src1 = src1;
  return cm_frem(src0, Src1);
}

#endif // _CLANG_CM_INTRINSICS_FREM_H_
