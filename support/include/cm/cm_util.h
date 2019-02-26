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

/// Compute the next power of 2 at compile time.
static inline constexpr unsigned int getNextPowerOf2(unsigned int n,
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

static inline constexpr unsigned getMaxNumOfOWordSLM() {
  return 8;
}

} // namespace details

#endif /*_CLANG_CM_UTIL_H_ */
