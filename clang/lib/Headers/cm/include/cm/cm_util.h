/*========================== begin_copyright_notice ============================

Copyright (C) 2014-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_util.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_UTIL_H_
#define _CLANG_CM_UTIL_H_

#include "cm_traits.h"
#include "cm_common.h"

namespace details {

/// Constant in number of bytes.
enum {
  BYTE = 1,
  WORD = 2,
  DWORD = 4,
  QWORD = 8,
  OWORD = 16,
  GRF = 32
};

/// Round up N to be multiple of M
static constexpr unsigned int roundUpNextMultiple(unsigned int N,
                                                  unsigned int M) {
  return ((N + M - 1) / M) * M;
}

/// Compute the next power of 2 at compile time.
static constexpr unsigned int getNextPowerOf2(unsigned int n,
                                              unsigned int k = 1) {
  return (k >= n) ? k : getNextPowerOf2(n, k * 2);
}

/// Check if a given 32 bit positive integer is a power of 2 at compile time.
static inline constexpr bool isPowerOf2(unsigned int n) {
  return (n & (n - 1)) == 0;
}

static inline constexpr bool isPowerOf2(unsigned int n, unsigned int limit) {
  return (n & (n - 1)) == 0 && n <= limit;
}

/// Do the assignment conditionally. This is to avoid template instantiation
/// errors for unreachable branches. For example, the following code will fail
/// to instantiate, for any integer pair (N1, N2). This is because although
/// true/false-branch cannot be executed, both branches need fully
/// instantitated, resulting out-of-bound errors during compilation.
///
/// \code
/// template <int N1, int N2>
/// void foo(vector<float, N1> v1, vector<float, N2> v2) {
///   if (N1 < N2)
///     v1 = v2.select<N1, 1>();
///   else
///     v2 = v2.select<N2, 1>();
/// }
/// \code
template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<(N1 <= N2), void>::type
if_assign(vector_ref<T, N1> v1, vector<T, N2> v2) {
  v1 = v2.select<N1, 1>();
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<(N1 > N2), void>::type
if_assign(vector_ref<T, N1> v1, vector<T, N2> v2) {}

template <typename T, int N, int M1, int M2>
CM_NODEBUG CM_INLINE typename std::enable_if<(M1 <= M2), void>::type
if_assign(matrix_ref<T, N, M1> m1, matrix<T, N, M2> m2) {
  m1 = m2.select<N, 1, M1, 1>();
}

template <typename T, int N, int M1, int M2>
CM_NODEBUG CM_INLINE typename std::enable_if<(M1 > M2), void>::type
if_assign(matrix_ref<T, N, M1> m1, matrix<T, N, M2> m2) {}

// Wrapper around vector select method.
// Unlike select this fucntion ignores stride when width is 1,
// so it is easier to use in generic code.
// The functionality of select is reduced to only reading the region.
template<int width, int stride, typename T, int size>
CM_NODEBUG CM_INLINE vector<T, width>
read_region(vector<T, size> vec, int offset) {
  static_assert(width > 0 && width < size, "invalid width");
  static_assert(stride > 0 && (width - 1) * stride < size,
      "invalid invalid stride");
  vector<T, width> selection;
  if constexpr (width == 1)
    selection = vec[offset];
  else
    selection = vec.select<width, stride>(offset);
  return selection;
}

// Wrapper around vector select method.
// Unlike select this fucntion ignores stride when width is 1,
// so it is easier to use in generic code.
// The functionality of select is reduced to only writing the region.
template<int stride, typename T, int size, int width>
CM_NODEBUG CM_INLINE void
write_region(vector_ref<T, size> vec, vector<T, width> insertion, int offset) {
  static_assert(stride > 0 && (width - 1) * stride < size,
      "invalid invalid stride");
  if constexpr (width == 1)
    vec[offset] = insertion[0];
  else
    vec.select<width, stride>(offset) = insertion;
}

static inline constexpr unsigned getMaxNumOfOWordSLM() {
#if defined(CM_GEN12)
  return 16;
#else
  return 8;
#endif
}

// to emit warnings, dependent on if function actually called
template <typename T = void>
constexpr bool always_false() {
  return false;
}


} // namespace details

#endif /*_CLANG_CM_UTIL_H_ */
