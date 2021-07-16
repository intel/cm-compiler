/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_CMTL_MATH_ROUND_H
#define CM_CMTL_MATH_ROUND_H

#include "../mask.h"
#include "../../cm_traits.h"
#include "../../cm_common.h"

namespace cmtl {
namespace math {

/* round_fast<integral>: rounds argument to the nearest integer.
 * It's an analog to lround/llround in C standard library.
 * The function is inaccurate for some range of arguments, though it faster
 * than general algorithm that considers those cases (more info below).
 *
 * Argument is scalar, vector or matrix of floating point type.
 * Correspondingly scalar, vector or matrix of provided integer type
 * is returned.
 *
 * Example of usage:
 *    vector<double, 16> fvec = .....;
 *    vector<int, 16> ivec = cmtl::math::round_fast<int>(fvec);
 *
 *
 * If argument value is out of range of provided integer type,
 * behavior is undefined.
 *
 * When the argument 2^frac_bit_size <= arg < 2^(frac_bit_size + 1)
 * e.g. for float 2^23 <= arg < 2^24, rounding mode can effect the result.
 * The result will be the real result +/- 1.
 */

namespace detail_round_fast {
template<typename FloatT, typename IntT>
void arg_type_check() {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  static_assert(std::is_integral<IntT>::value,
      "expected integral value as template parameter");
}
} // namespace detail

template<typename IntT, typename FloatT, int width>
CM_INLINE vector<IntT, width> round_fast(vector<FloatT, width> arg) {
  detail_round_fast::arg_type_check<FloatT, IntT>();

  Mask<width> is_negative(arg < 0.0);
  vector<FloatT, width> delta = 0.5;
  vector<FloatT, width> delta_for_neg = -0.5;
  delta.merge(delta_for_neg, is_negative.get());
  return arg + delta;
}

template<typename IntT, typename FloatT, int width, int hight>
CM_INLINE matrix<IntT, hight, width>
round_fast(matrix<FloatT, hight, width> arg) {
  detail_round_fast::arg_type_check<FloatT, IntT>();

  vector<FloatT, width * hight> vec_arg = arg.format<FloatT>();
  return round_fast<IntT>(vec_arg);
}

template<typename IntT, typename FloatT,
    std::enable_if_t<std::is_floating_point<FloatT>::value, int> = 0,
    std::enable_if_t<std::is_integral<IntT>::value, int> = 0>
CM_INLINE IntT round_fast(FloatT arg) {
  detail_round_fast::arg_type_check<FloatT, IntT>();
  return arg + (arg < 0.0 ? -0.5 : 0.5);
}

} // namespace math
} // namespace cmtl
#endif // CM_CMTL_MATH_ROUND_H
