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

#ifndef CM_CMTL_MATH_FREXP_H
#define CM_CMTL_MATH_FREXP_H

#include "frexp_impl.h"

#include "../../cm_traits.h"
#include "../../cm_common.h"

namespace cmtl {
namespace math {

/* frexp - analog to std::frexp. It decomposes every elemet of the given vector
 * into normalized fraction and exponent.
 *
 * Return: vector of fractions.
 * Vector of exponents is returned through \p exp.
 *
 * If arg elem is +/-0.0, it is returned unmodified, 0 is stored in exp.
 * If arg elem is inf or nan, it is returned unmodified, unspecified value
 * is stored in exp.
 * Otherwise fraction is in the range (-1, 0.5] U [0.5, 1) and corresponding
 * exponent is stored in exp.
 *
 * NOTE: concept of normalization differs in C std library from IEEE
 * C lib: sign * 0.xxxxxxxxxxx * 2^exp
 * IEEE : sign * 1.xxxxxxxxxxx * 2^exp
 *
 * frexp supports floating point class hints - hint::fpclass_tag (for more
 * information look at cmtl/hint.h header). If no hint is provided the most
 * common case is considered.
 *
 * If the user cannot guarantee with a hint, that arg doesn't have subnormal
 * values, width must be power of 2, greater than one and less than or equal
 * to 32.
 * If IntT capacity isn't enough to store exponent, behavior is undefined.
 */
template<typename FloatT, typename IntT, int width,
         typename FPClassTag = hint::fpclass_tag_def::general_t>
CM_INLINE vector<FloatT, width>
frexp(vector<FloatT, width> arg, vector_ref<IntT, width> exp,
      FPClassTag = hint::fpclass_tag_def::general) {
  detail::frexp_arg_type_check<FloatT, IntT, FPClassTag>();

  return detail::frexp_impl<FPClassTag>(arg, exp);
}

template<typename FloatT, typename IntT, int width, int hight,
         typename FPClassTag = hint::fpclass_tag_def::general_t>
CM_INLINE matrix<FloatT, hight, width>
frexp(matrix<FloatT, hight, width> arg, matrix_ref<IntT, hight, width> exp,
      FPClassTag tag = hint::fpclass_tag_def::general) {
  detail::frexp_arg_type_check<FloatT, IntT, FPClassTag>();

  vector<FloatT, width * hight> vec_arg = arg.format<FloatT>();
  return frexp(vec_arg, exp.format<IntT>(), tag);
}


template<typename FloatT, typename IntT,
         typename FPClassTag = hint::fpclass_tag_def::general_t,
         std::enable_if_t<std::is_floating_point<FloatT>::value, int> = 0,
         std::enable_if_t<std::is_integral<IntT>::value, int> = 0>
CM_INLINE FloatT frexp(FloatT arg, IntT &exp,
                       FPClassTag tag = hint::fpclass_tag_def::general) {
  detail::frexp_arg_type_check<FloatT, IntT, FPClassTag>();

  // FIXME: implement separate algorithm for scalar case
  vector<FloatT, 2> vec_arg = arg;
  vector<IntT, 2> vec_exp;
  vector<FloatT, 2> vec_res = frexp(vec_arg, vec_exp.select_all(), tag);
  exp = vec_exp(0);
  return vec_res(0);
}


} // namespace math
} // namespace cmtl
#endif // CM_CMTL_MATH_FREXP_H
