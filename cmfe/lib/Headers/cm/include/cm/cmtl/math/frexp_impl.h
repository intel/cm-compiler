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

#ifndef  CM_CMTL_MATH_IMPL_FREXP_H
#define  CM_CMTL_MATH_IMPL_FREXP_H

#include "../../cm_traits.h"
#include "../../cm_common.h"
#include "../hint.h"
#include "fpclassify.h"
#include "float.h"
#include "utils.h"

namespace cmtl {
namespace math {
namespace detail {

template<typename FloatT, typename IntT>
CM_INLINE void frexp_arg_type_check() {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  static_assert(std::is_integral<IntT>::value,
      "expected integral value as argument");
}

template<typename FloatT, typename IntT, typename FPClassTag>
CM_INLINE void frexp_arg_type_check() {
  frexp_arg_type_check<FloatT, IntT>();
  static_assert(hint::is_fpclass_tag_v<FPClassTag>,
      "wrong tag is provided, hint::fpclass_tag was expected");
  static_assert(!hint::equal_v<FPClassTag, hint::fpclass_tag_def::none_t>,
      "tag must specify at least some floating point class");
}

/* here normalization is in terms of C std library
 * sign * 0.1xxxxxxxx * 2^exp
 *
 * returns float in range (-1, -0.5] U [0.5, 1)
 * If element of arg is not normal,
 * corresponding returned element is unspecified.
 */
template<typename FloatT, int width>
CM_INLINE vector<FloatT, width>
get_normalized_fract(vector<FloatT, width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  return set_exp(arg, -1);
}

/* this function doesn't rewrite \p exp, but adds to it
 * when \p exp is originaly zeroed we get frexp for normals
 */
template<typename FloatT, typename IntT, int width>
CM_INLINE vector<FloatT, width>
frexp_normal_exp_delta(vector<FloatT, width> arg,
                       vector_ref<IntT, width> exp) {
  frexp_arg_type_check<FloatT, IntT>();

  exp += get_exp(arg) + 1;
  return get_normalized_fract(arg);
}

/* frexp only for normalized values
 * When income is not normilized, returned
 * and stored in \p exp values are unspecified.
 */
template<typename FloatT, typename IntT, int width>
CM_INLINE vector<FloatT, width>
frexp_normal(vector<FloatT, width> arg,
             vector_ref<IntT, width> exp) {
  frexp_arg_type_check<FloatT, IntT>();

  exp = 0;
  return frexp_normal_exp_delta(arg, exp);
}

// frexp for normal and subnormal values.
// For other values result is uncpecified.
template<typename FPClassTag, typename FloatT, typename IntT, int width>
CM_INLINE vector<FloatT, width>
frexp_subn_normal(vector<FloatT, width> arg,
                  vector_ref<IntT, width> exp) {
  frexp_arg_type_check<FloatT, IntT, FPClassTag>();
  static_assert(detail::is_pow2(width) && width > 1 &&
      width <= 32, "width must be power of 2, greater than one "
      "and less than or equal to 32.");

  exp = 0;
  // really issubnormal is enough, but is_zero_or_subn is faster
  // to calculate and zero must be fixed at the end anyway
#ifdef SIMD_CF_IS_FIXED
  SIMD_IF_BEGIN (is_zero_or_subn(arg).get_vector()) {
    arg *= FloatDescr<FloatT>::Normalizer;
    exp = -FloatDescr<FloatT>::LogNormalizer;
  } SIMD_IF_END;
#else
  auto normalized_arg = arg * FloatDescr<FloatT>::Normalizer;
  auto normalized_exp = -FloatDescr<FloatT>::LogNormalizer;
  Mask<width> to_normalize = is_zero_or_subn(arg);
  arg.merge(normalized_arg, to_normalize.get());
  exp.merge(normalized_exp, to_normalize.get());
#endif
  arg = frexp_normal_exp_delta(arg, exp);

  return arg;
}

// frexp for all classes of floats
template<typename FPClassTag, typename FloatT, typename IntT, int width>
CM_INLINE vector<FloatT, width>
frexp_general(vector<FloatT, width> arg,
              vector_ref<IntT, width> exp) {
  frexp_arg_type_check<FloatT, IntT, FPClassTag>();
  using namespace hint;

  const auto orig_arg = arg;
  if constexpr (FPClassTag::template has_v<fpclass_tag_def::subnormal_t>)
    arg = frexp_subn_normal<FPClassTag>(arg, exp);
  else
    arg = frexp_normal(arg, exp);

  // need to fix exp if arg had zero
  if constexpr (FPClassTag::template has_v<hint::fpclass_tag_def::zero_t>)
    exp.merge(0, orig_arg == 0.0);

  // for zero, inf, nan the original argument must be returned
  if constexpr (FPClassTag::template has_v<or_t<fpclass_tag_def::special_t,
                                       fpclass_tag_def::zero_t>>) {
    auto is_inf_nan_zero = is_inf_or_nan(orig_arg) |
                           Mask<width>(orig_arg == 0.0);
    arg.merge(orig_arg, is_inf_nan_zero.get());
  }
  return arg;
}

template<typename FPClassTag, typename FloatT, typename IntT, int width>
CM_INLINE vector<FloatT, width>
frexp_impl(vector<FloatT, width> arg,
           vector_ref<IntT, width> exp) {
  frexp_arg_type_check<FloatT, IntT, FPClassTag>();
  using namespace hint;

  if constexpr (equal_v<FPClassTag, fpclass_tag_def::normal_t>)
    return frexp_normal(arg, exp);

  if constexpr (equal_v<FPClassTag, fpclass_tag_def::zero_t>) {
    exp = 0;
    return arg;
  }

  if constexpr (equal_v<FPClassTag, fpclass_tag_def::special_t>)
    return arg;

  vector<int16_t, width> internal_exp;
  auto frac = frexp_general<FPClassTag>(arg, internal_exp);
  exp = internal_exp;
  return frac;
}

} // namespace detail
} // namespace math
} // namespace cmtl
#endif //  CM_CMTL_MATH_IMPL_FREXP_H
