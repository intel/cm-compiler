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

#ifndef CM_CMTL_MATH_EXP_H
#define CM_CMTL_MATH_EXP_H

#include "exp_impl.h"

#include "../../cm_traits.h"
#include "../../cm_common.h"

namespace cmtl {
namespace math {

/* exp: Computes e (Euler's number, 2.7182818...) raised to the given
 *      power \p arg.
 *
 * If overflow occurs,      +inf is returned.
 * If underflow occurs,     +0.0 is returned.
 * If the argument is +inf, +inf is returned.
 * if the argument is -inf, +0.0 is returned.
 * If the argument is NaN,   NaN is returned.
 *
 * exp supports floating point class hints - hint::fpclass_tag (for more
 * information look at cmtl/hint.h header). If no hint is provided the most
 * common case is considered.
 */
template<typename FloatT, int width,
         typename FPClassTag = hint::fpclass_tag_def::general_t>
CM_INLINE vector<FloatT, width>
exp(vector<FloatT, width> arg,
    FPClassTag = hint::fpclass_tag_def::general) {
  exp_detail::arg_type_check<FloatT, FPClassTag>();
  return exp_detail::exp_impl<FPClassTag>(arg);
}

template<typename FloatT, int width, int hight,
         typename FPClassTag = hint::fpclass_tag_def::general_t>
CM_INLINE matrix<FloatT, hight, width>
exp(matrix<FloatT, hight, width> arg,
    FPClassTag tag = hint::fpclass_tag_def::general) {
  exp_detail::arg_type_check<FloatT, FPClassTag>();
  vector<FloatT, width * hight> vec_arg = arg.format<FloatT>();
  return exp(vec_arg, tag);
}

template<typename FloatT,
         typename FPClassTag = hint::fpclass_tag_def::general_t,
         std::enable_if_t<std::is_floating_point<FloatT>::value, int> = 0>
CM_INLINE FloatT
exp(FloatT arg,
    FPClassTag tag = hint::fpclass_tag_def::general) {
  exp_detail::arg_type_check<FloatT, FPClassTag>();
  vector<FloatT, 1> vec_arg = arg;
  vector<FloatT, 1> vec_exp = exp(vec_arg, tag);
  return vec_exp[0];
}

} // namespace math
} // namespace cmtl
#endif // CM_CMTL_MATH_EXP_H
