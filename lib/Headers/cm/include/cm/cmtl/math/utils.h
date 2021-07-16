/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
