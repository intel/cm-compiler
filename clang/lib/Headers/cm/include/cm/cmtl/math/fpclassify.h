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

#ifndef CM_CMTL_MATH_FPCLASSIFY_H
#define CM_CMTL_MATH_FPCLASSIFY_H

#include "float.h"
#include "../mask.h"

/* This header provides functions for floating point values classification.
 *
 * Functions:
 *    isnormal
 *    isnan
 *    isinf
 *    issubnormal
 *    fpclassify
 */

namespace cmtl {
namespace math {

// vector analog to std::isnormal
template<typename FloatT, int Width>
CM_INLINE Mask<Width>
isnormal(vector<FloatT, Width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  return ~detail::is_zero_or_subn(arg) &
         ~detail::is_inf_or_nan(arg);
}

// vector analog to std::isnan
template<typename FloatT, int width>
CM_INLINE Mask<width>
isnan(vector<FloatT, width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  constexpr auto upper = detail::FloatDescr<FloatT>::One <<
    (detail::FloatDescr<FloatT>::BitSize - 1);
  constexpr auto positive_mask = upper - 1u;
  constexpr auto lower = detail::FloatDescr<FloatT>::Exp::Mask;
  using UInt = typename detail::FloatDescr<FloatT>::SameSizeUInt;

  vector<UInt, width> uarg = arg.format<UInt>();
  // zero sign bit
  uarg &= positive_mask;
  return Mask<width>(uarg > lower);
}

// vector analog to std::isinf
template<typename FloatT, int Width>
CM_INLINE Mask<Width>
isinf(vector<FloatT, Width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  return detail::is_inf_or_nan(arg) &
         detail::is_zero_or_inf_or_pow2norm(arg);
}

// returns wether elements of \p arg denormalized
template<typename FloatT, int Width>
CM_INLINE Mask<Width>
issubnormal(vector<FloatT, Width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  return detail::is_zero_or_subn(arg) &
         ~detail::is_zero_or_inf_or_pow2norm(arg);
}

/* if normal case is excluded, float class can be easily defined only by
 * exp and frac equality to 0
 *                     exp == 0 | frac == 0  */
enum {            //            |
  fp_nan = 0,     //          0 | 0
  fp_infinite,    //          0 | 1
  fp_subnormal,   //          1 | 0
  fp_zero,        //          1 | 1
  fp_normal       //          x | x
};

/* vector analog to std::fpclassify
 *
 * returns vector of fp_infinite, fp_nan, fp_normal,
 *                   fp_subnormal, fp_zero
 */
template<typename FloatT, int Width>
CM_INLINE vector<int, Width>
fpclassify(vector<FloatT, Width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");

  vector<ushort, Width> is_null_exp =
    detail::is_zero_or_subn(arg).get_vector();
  vector<ushort, Width> is_null_frac =
    detail::is_zero_or_inf_or_pow2norm(arg).get_vector();
  vector<ushort, Width> fpclass =
    (is_null_exp << 1) + is_null_frac;
  MaskIntT is_normal = isnormal(arg);
  fpclass.merge(fp_normal, is_normal);
  return fpclass;
}

} // namespace math
} // namespace cmtl
#endif // CM_CMTL_MATH_FPCLASSIFY_H
