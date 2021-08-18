/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_CMTL_MATH_LOG_H
#define CM_CMTL_MATH_LOG_H

#include "log_impl.h"

#include "../../cm_traits.h"
#include "../../cm_common.h"

namespace cmtl {
namespace math {

/* log: Computes the natural logarithm of \p arg.
 *
 * If the argument is +/-0.0,  -inf is returned.
 * if the argument is +inf,    +inf is returned.
 * If the argument is negative, NaN is returned.
 * If the argument is NaN,      NaN is returned.
 *
 * log supports floating point class hints - hint::fpclass_tag (for more
 * information look at cmtl/hint.h header). If no hint is provided the most
 * common case is considered.
 */
template<typename FloatT, int width,
         typename FPClassTag = hint::fpclass_tag_def::general_t>
CM_INLINE vector<FloatT, width>
log(vector<FloatT, width> arg,
    FPClassTag = hint::fpclass_tag_def::general) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  return log_detail::log_impl<FPClassTag>(arg);
}

template<typename FloatT, int width, int hight,
         typename FPClassTag = hint::fpclass_tag_def::general_t>
CM_INLINE matrix<FloatT, hight, width>
log(matrix<FloatT, hight, width> arg,
    FPClassTag tag = hint::fpclass_tag_def::general) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  vector<FloatT, width * hight> vec_arg = arg.format<FloatT>();
  return log(vec_arg, tag);
}

template<typename FloatT,
         typename FPClassTag = hint::fpclass_tag_def::general_t,
         std::enable_if_t<std::is_floating_point<FloatT>::value, int> = 0>
CM_INLINE FloatT log(FloatT arg,
    FPClassTag tag = hint::fpclass_tag_def::general) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  vector<FloatT, 2> vec_arg = arg;
  vector<FloatT, 2> vec_log = log(vec_arg, tag);
  return vec_log[0];
}

} // namespace math
} // namespace cmtl
#endif // CM_CMTL_MATH_LOG_H

