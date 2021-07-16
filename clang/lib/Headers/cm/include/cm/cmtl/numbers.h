/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef  CM_CMTL_NUMBERS_H
#define  CM_CMTL_NUMBERS_H

#include "../cm_traits.h"

/* cm/cmtl/numbers.h is a library with predefined numeric constans.
 * It is an analog of <numbers> in C++20 standard library.
 */

namespace cmtl {
namespace numbers {

namespace detail {
template<typename T>
using enable_if_floating_t = std::enable_if_t<std::is_floating_point<T>::value, T>;
} // namespace detail

// e
template<typename FloatT>
CM_INLINE constexpr FloatT e_v
  = detail::enable_if_floating_t<FloatT>(2.718281828459045235360287471352662498L);

// log_2(e)
template<typename FloatT>
CM_INLINE constexpr FloatT log2e_v
  = detail::enable_if_floating_t<FloatT>(1.442695040888963407359924681001892137L);

// log_10(e)
template<typename FloatT>
CM_INLINE constexpr FloatT log10e_v
  = detail::enable_if_floating_t<FloatT>(0.434294481903251827651128918916605082L);

// pi
template<typename FloatT>
CM_INLINE constexpr FloatT pi_v
  = detail::enable_if_floating_t<FloatT>(3.141592653589793238462643383279502884L);

// 1/pi
template<typename FloatT>
CM_INLINE constexpr FloatT inv_pi_v
  = detail::enable_if_floating_t<FloatT>(0.318309886183790671537767526745028724L);

// 1/sqrt(pi)
template<typename FloatT>
CM_INLINE constexpr FloatT inv_sqrtpi_v
  = detail::enable_if_floating_t<FloatT>(0.564189583547756286948079451560772586L);

// ln(2)
template<typename FloatT>
CM_INLINE constexpr FloatT ln2_v
  = detail::enable_if_floating_t<FloatT>(0.693147180559945309417232121458176568L);

// ln(10)
template<typename FloatT>
CM_INLINE constexpr FloatT ln10_v
  = detail::enable_if_floating_t<FloatT>(2.302585092994045684017991454684364208L);

// sqrt(2)
template<typename FloatT>
CM_INLINE constexpr FloatT sqrt2_v
  = detail::enable_if_floating_t<FloatT>(1.414213562373095048801688724209698079L);

// sqrt(3)
template<typename FloatT>
CM_INLINE constexpr FloatT sqrt3_v
  = detail::enable_if_floating_t<FloatT>(1.732050807568877293527446341505872367L);

// 1/sqrt(2)
template<typename FloatT>
CM_INLINE constexpr FloatT inv_sqrt2_v
  = detail::enable_if_floating_t<FloatT>(0.707106781186547524400844362104849039L);

// 1/sqrt(3)
template<typename FloatT>
CM_INLINE constexpr FloatT inv_sqrt3_v
  = detail::enable_if_floating_t<FloatT>(0.577350269189625764509148780501957456L);

// The Euler-Mascheroni constant
template<typename FloatT>
CM_INLINE constexpr FloatT egamma_v
  = detail::enable_if_floating_t<FloatT>(0.577215664901532860606512090082402431L);

// The golden ratio, (1+sqrt(5))/2
template<typename FloatT>
CM_INLINE constexpr FloatT phi_v
  = detail::enable_if_floating_t<FloatT>(1.618033988749894848204586834365638118L);

// 1/ln(2) = ln(e)/ln(2) = log_2(e)
template<typename FloatT>
CM_INLINE constexpr FloatT inv_ln2_v = log2e_v<FloatT>;

CM_INLINE constexpr double e = e_v<double>;
CM_INLINE constexpr double log2e = log2e_v<double>;
CM_INLINE constexpr double log10e = log10e_v<double>;
CM_INLINE constexpr double pi = pi_v<double>;
CM_INLINE constexpr double inv_pi = inv_pi_v<double>;
CM_INLINE constexpr double inv_sqrtpi = inv_sqrtpi_v<double>;
CM_INLINE constexpr double ln2 = ln2_v<double>;
CM_INLINE constexpr double ln10 = ln10_v<double>;
CM_INLINE constexpr double sqrt2 = sqrt2_v<double>;
CM_INLINE constexpr double sqrt3 = sqrt3_v<double>;
CM_INLINE constexpr double inv_sqrt2 = inv_sqrt2_v<double>;
CM_INLINE constexpr double inv_sqrt3 = inv_sqrt3_v<double>;
CM_INLINE constexpr double egamma = egamma_v<double>;
CM_INLINE constexpr double phi = phi_v<double>;
CM_INLINE constexpr double inv_ln2 = inv_ln2_v<double>;

} // namespace numbers
} // namespace cmtl
#endif // CM_CMTL_NUMBERS_H
