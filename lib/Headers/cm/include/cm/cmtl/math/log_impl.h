/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

SPDX-License-Identifier: MIT
============================= end_copyright_notice ===========================*/

#ifndef  CM_CMTL_MATH_LOG_IMPL_H
#define  CM_CMTL_MATH_LOG_IMPL_H

#include "../../cm_traits.h"
#include "../../cm_common.h"
#include "../numbers.h"
#include "../hint.h"
#include "float.h"
#include "frexp.h"
#include "fpclassify.h"

// The implementation is based on the algorithm discussed in
// The Mathematical-Function Computation Handbook
// by Nelson H.F. Beebe. Certain changes were made to optimize
// the algorithm for vector computations.

namespace cmtl {
namespace math {
namespace log_detail {

// ln2_hi + ln2_lo = ln(2)
// Meant for Cody Waite result unfolding.
// ln2_hi is taken so the product
// ln2_hi * FloatDescr<double>::Exp::MaxSVal is representable
// as double precisely.
// ln(2) = 0.b17217f7d1cf79abc9e3b39803f2f6af40f343267298b62d8a0d175b8baafa
constexpr double ln2_hi = 0x0.b17217f7d1p0;
constexpr double ln2_lo = 0x0.0000000000cf79abc9e3b398p0;

constexpr int poly_order = 8;
constexpr double poly_coef_init[poly_order] = {
  8.333333333333333307726e-02,
  1.250000000000027737181e-02,
  2.232142857093574000534e-03,
  4.340277811119014760447e-04,
  8.877829843859590969703e-05,
  1.878203829749719357775e-05,
  4.049261716236508114464e-06,
  9.985449030888993932992e-07
};

// Represents arg as r * 2^k, where r belongs [sqrt(2)/2, sqrt(2))
template<typename FPClassTag, int width>
CM_INLINE vector<double, width> reduce_range(vector<double, width> arg,
                                             vector_ref<int16_t, width> k) {
  // NaN, -inf must have been handled before.
  // Zero and +inf can come, result of frexp is unspecified,
  // but its OK as in this case the output must be fixed anyway.
  vector<double, width> r = frexp(arg, k,
                                  FPClassTag{} &
                                    (hint::fpclass_tag_def::normal |
                                     hint::fpclass_tag_def::subnormal));
  // r is in the range [0.5, 1) after frexp
  // putting r in the range [sqrt(2)/2, sqrt(2))
#ifdef SIMD_CF_IS_FIXED
  SIMD_IF_BEGIN (r < numbers::inv_sqrt2) {
    r *= 2.0;
    k -= 1;
  } SIMD_IF_END;
#else
  auto r_shifted = r * 2.0;
  auto k_shifted = k - 1;
  auto to_shift = r < numbers::inv_sqrt2;
  r.merge(r_shifted, to_shift);
  k.merge(k_shifted, to_shift);
#endif
  return r;
}

// changes variable to 2 * (orig - 1) / (orig + 1)
template<int width>
CM_INLINE vector<double, width> change_variable(vector<double, width> orig) {
  vector<double, width> nom = orig - 1.0;
  vector<double, width> denom = 0.5 * orig + 0.5;
  return nom / denom;
}

// Calculates polynomial.
template<int width>
CM_INLINE vector<double, width> calc_poly(vector<double, width> g) {
  vector<double, poly_order> poly_coef(poly_coef_init);
  vector<double, width> poly_val = poly_coef[poly_order - 1];
#pragma unroll
  for (int i = poly_order - 2; i >= 0; --i)
    poly_val = poly_val * g + poly_coef[i];

  return poly_val;
}

// Computes log on reduced range.
// log is represented as:
// log(r) = z + z^3 * F(z^2), where z = z(r) - new variable
// F(g) is defined with polynomial approximation.
template<int width>
CM_INLINE vector<double, width> log_reduced(vector<double, width> r) {
  vector<double, width> z = change_variable(r);
  vector<double, width> g = z * z;
  vector<double, width> poly = calc_poly(g);
  return z + z * g * poly;
}

// Based on the computed log for reduced domain calculates log for the
// original arg.
// arg = r * 2 ^ k
template<int width>
CM_INLINE vector<double, width> log_unfold(vector<double, width> log_r,
                                           vector<int16_t, width> k) {
  return (log_r + k * ln2_lo) + k * ln2_hi;
}

// Calculates logarithm for numeric values.
// For all arguments out of range (0.0, +inf) result is unspecified.
template<typename FPClassTag, int width>
CM_INLINE vector<double, width> log_numeric(vector<double, width> arg) {
  vector<int16_t, width> k;
  vector<double, width> r = reduce_range<FPClassTag>(arg, k);
  vector<double, width> log_r = log_reduced(r);
  return log_unfold(log_r, k);
}

// TODO: move to numeric_limits when there is numeric_limits
template<typename FloatT>
CM_INLINE FloatT quiet_NaN() {
  using UInt = typename detail::FloatDescr<FloatT>::SameSizeUInt;
  vector<UInt, 1> uint_nan = ~0;
  vector<FloatT, 1> nan = uint_nan.format<FloatT>();
  return nan[0];
}

// TODO: move to numeric_limits when there is numeric_limits
template<typename FloatT>
CM_INLINE FloatT infinity() {
  using UInt = typename detail::FloatDescr<FloatT>::SameSizeUInt;
  vector<UInt, 1> uint_inf = detail::FloatDescr<FloatT>::Exp::Mask;
  vector<FloatT, 1> inf = uint_inf.format<FloatT>();
  return inf[0];
}

// Fixes result for NaN, inf, zero, negative input.
// New fixed result is returned.
template<typename FPClassTag, int width>
CM_INLINE vector<double, width> fix_corner(vector<double, width> arg,
                                           vector<double, width> res) {
  Mask<width> is_nan_res(arg < 0.0);
  if constexpr (FPClassTag::template has_v<hint::fpclass_tag_def::nan_t>)
    is_nan_res |= isnan(arg);
  res.merge(quiet_NaN<double>(), is_nan_res.get());

  if constexpr (FPClassTag::template has_v<hint::fpclass_tag_def::infinite_t>) {
    res.merge(infinity<double>(), arg == infinity<double>());
  }

  if constexpr (FPClassTag::template has_v<hint::fpclass_tag_def::zero_t>)
    res.merge(-infinity<double>(), arg == 0.0);
  return res;
}

template<typename FPClassTag, int width>
CM_INLINE vector<double, width> log_impl(vector<double, width> arg) {
  vector<double, width> res = log_numeric<FPClassTag>(arg);
  return fix_corner<FPClassTag>(arg, res);
}

} // namespace log_detail
} // namespace math
} // namespace cmtl
#endif //  CM_CMTL_MATH_LOG_IMPL_H
