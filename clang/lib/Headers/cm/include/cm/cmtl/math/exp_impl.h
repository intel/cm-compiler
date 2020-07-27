/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

#ifndef  CM_CMTL_MATH_EXP_IMPL_H
#define  CM_CMTL_MATH_EXP_IMPL_H

#include "../../cm_traits.h"
#include "../../cm_common.h"
#include "../numbers.h"
#include "../hint.h"
#include "float.h"

namespace cmtl {
namespace math {
namespace exp_detail {

template<typename FloatT, typename FPClassTag>
CM_INLINE void arg_type_check() {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  static_assert(hint::is_fpclass_tag_v<FPClassTag>,
      "wrong tag is provided, hint::fpclass_tag was expected");
  static_assert(!hint::equal_v<FPClassTag, hint::fpclass_tag_def::none_t>,
      "tag must specify at least some floating point class");
}

// ln2_hi + ln2_lo = ln(2)
// Meant for Cody Waite range reduction
// ln2_hi is taken so the product ln2_hi * rough_overflow is representable
// as double precisely.
// ln(2) = 0.b17217f7d1cf79abc9e3b39803f2f6af40f343267298b62d8a0d175b8baafa
constexpr double ln2_hi = 0x0.b17217f7d1p0;
constexpr double ln2_lo = 0x0.0000000000cf79abc9e3b398p0;

// minmax polynomal approximation of exp(x) over the range
// [-ln(2)/2, ln(2)/2]. Got with Remez algorithm.
constexpr int poly_order = 12;
constexpr double poly_coef_init[poly_order] = {
  1.0,
  1.0,
  5.000000000000018389e-01,
  1.666666666666672403e-01,
  4.166666666648806972e-02,
  8.333333333299448800e-03,
  1.388888895232078842e-03,
  1.984126992839627458e-04,
  2.480148547660227862e-05,
  2.755720899040766077e-06,
  2.763264154737745042e-07,
  2.511970386342627276e-08
};


// exp for double overflows when arg is greater than 1023 * ln(2) ~ 709
// The algorithm can handle up to 2 * 1023 * ln(2) argument, so we can cut off
// rough.
constexpr double rough_overflow = 800.0;
// Without denormals underflow happens under -1022 * ln(2) ~ -708, with
// denormals -1074 * ln(2) ~ 744, again there's no problem to take a bit more.
constexpr double rough_underflow = -800.0;

// Represents arg as:
// arg = two_power * ln(2) + remainder
//
// remainder is returned, two_power is stored in \p two_power_out
// two_power must be representable as int.
//
// Cody and Waite reduction algorithm is used.
template<int width>
CM_INLINE vector<double, width>
reduce_range(vector<double, width> arg,
             vector_ref<int, width> two_power_out) {
  vector<int, width> two_power = round_fast<int>(arg * numbers::inv_ln2);
  vector<double, width> remainder_hi = arg - two_power * ln2_hi;
  vector<double, width> remainder_lo = -two_power * ln2_lo;

  two_power_out = two_power;
  return remainder_hi + remainder_lo;
}

template<typename FloatT, typename IntT, int width>
CM_INLINE vector<FloatT, width>
exp2_unsafe(vector<IntT, width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  static_assert(std::is_integral<IntT>::value,
      "expected integral value as argument");
  using UInt = typename detail::FloatDescr<FloatT>::SameSizeUInt;
  vector<UInt, width> two_power = arg + detail::FloatDescr<FloatT>::Exp::Bias;
  two_power <<= detail::FloatDescr<FloatT>::Exp::Offset;
  return two_power.format<FloatT>();
}

// exp only for reduced range [-ln(2)/2, ln(2)/2].
// Polynomal approximation is used.
template<int width>
CM_INLINE vector<double, width> exp_reduced(vector<double, width> r) {
  vector<double, poly_order> poly_coef(poly_coef_init);
  vector<double, width> poly_val = poly_coef[poly_order - 1];
#pragma unroll
  for (int i = poly_order - 2; i >= 0; --i)
    poly_val = poly_val * r + poly_coef[i];

  return poly_val;
}

// original arg = k * ln(2) + r
// Based on precalculated exp(r) and provided k calculates exp(arg).
//
// k is split in sum first + second.
// If k was passed to exp2_unsafe, it could result in overflow (result of
// exp2_unsafe is unspecified, not inf), but first and second are two times
// smaller, so it is safe to pass them to exp2_unsafe.
// If k is meant to cause overflow, its OK because overflow will happen in
// float arithmetic and result in inf, which is correct.
// Similarly underflow and denormal result cases are covered.
template<int width>
CM_INLINE vector<double, width> unfold_exp(vector<double, width> exp_r,
                                           vector<int, width> k) {
  vector<int, width> first = k / 2;
  vector<int, width> second = k - first;

  vector<double, width> two_to_first = exp2_unsafe<double>(first);
  vector<double, width> two_to_second = exp2_unsafe<double>(second);
  return exp_r * two_to_first * two_to_second;
}

// Calculates exponent. Argument range must be less then [-1400, 1400] (no need
// here in precise bounds, as obviously smaller range will be provided).
template<int width>
CM_INLINE vector<double, width> exp_numeric(vector<double, width> arg) {
  vector<int, width> k;
  vector<double, width> r = reduce_range(arg, k.select_all());

  vector<double, width> exp_r = exp_reduced(r);

  return unfold_exp(exp_r, k);
}

// TODO: move to numeric_limits when there is numeric_limits
template<typename FloatT>
CM_INLINE FloatT quiet_NaN() {
  using UInt = typename detail::FloatDescr<FloatT>::SameSizeUInt;
  vector<UInt, 1> uint_nan = ~0;
  vector<FloatT, 1> nan = uint_nan.format<FloatT>();
  return nan[0];
}

template<typename FPClassTag, int width>
CM_INLINE vector<double, width> exp_impl(vector<double, width> arg) {
  Mask<width> is_nan(false);
  if constexpr (FPClassTag::template has_v<hint::fpclass_tag_def::nan_t>) {
    is_nan = isnan(arg);
    arg.merge(0.0, is_nan.get());
  }

  arg = clamp(arg, rough_underflow, rough_overflow);
  vector<double, width> exp = exp_numeric(arg);

  if constexpr (FPClassTag::template has_v<hint::fpclass_tag_def::nan_t>)
    exp.merge(quiet_NaN<double>(), is_nan.get());
  return exp;
}

} // namespace exp_detail
} // namespace math
} // namespace cmtl
#endif //  CM_CMTL_MATH_EXP_IMPL_H
