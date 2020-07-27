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

#ifndef  CM_CMTL_MATH_IMPL_UTILS_H
#define  CM_CMTL_MATH_IMPL_UTILS_H

namespace cmtl {
namespace math {
namespace detail {

// is_pow2: checks whether var is power of 2
// Works for integral types.
template<typename T>
constexpr bool is_pow2(T var) {
  static_assert(std::is_integral<T>::value,
      "wrong argument: must be integral");
  if (var <= 0)
    return false;
  return !(var & (var - 1));
}

} // namespace detail
} // namespace math
} // namespace cmtl
#endif //  CM_CMTL_MATH_IMPL_UTILS_H
