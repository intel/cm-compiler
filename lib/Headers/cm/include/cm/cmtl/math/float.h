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

#ifndef  CM_CMTL_MATH_IMPL_FLOAT_H
#define  CM_CMTL_MATH_IMPL_FLOAT_H

#include "../mask.h"
#include "../../cm_util.h"

namespace cmtl {
namespace math {
namespace detail {

template<typename T>
struct FloatDescrImpl;

template<>
struct FloatDescrImpl<float> {
  using SameSizeSInt = int32_t;
  using SameSizeUInt = uint32_t;
  static constexpr int BitSize = 32;
  // the minimal number, being multiplied by which subnorm becomes normal
  // pow(2, 23) should be enough, but glibc uses pow(2, 25)
  // and they probaly know what they do
  static constexpr float Normalizer = 3.3554432000e+07; // pow(2, 25)
  static constexpr int LogNormalizer = 25;
  struct Frac {
    static constexpr int BitSize = 23;
  };
  struct Exp {
    static constexpr int BitSize = 8;
    static constexpr int Bias = 127;
  };
};

template<>
struct FloatDescrImpl<double> {
  using SameSizeSInt = int64_t;
  using SameSizeUInt = uint64_t;
  static constexpr int BitSize = 64;
  static constexpr float Normalizer = 1.80143985094819840000e+16; // pow(2, 54)
  static constexpr int LogNormalizer = 54;
  struct Frac {
    static constexpr int BitSize = 52;
  };
  struct Exp {
    static constexpr int BitSize = 11;
    static constexpr int Bias = 1023;
  };
};

template<typename FloatT>
struct FloatDescr {
  using SameSizeSInt = typename FloatDescrImpl<FloatT>::SameSizeSInt;
  using SameSizeUInt = typename FloatDescrImpl<FloatT>::SameSizeUInt;
  static constexpr int BitSize = FloatDescrImpl<FloatT>::BitSize;
  static constexpr FloatT Normalizer = FloatDescrImpl<FloatT>::Normalizer;
  static constexpr int LogNormalizer = FloatDescrImpl<FloatT>::LogNormalizer;
  // to avoid writing to much static_cast
  static constexpr SameSizeUInt One = 1;
  struct Frac {
    static constexpr int BitSize = FloatDescrImpl<FloatT>::Frac::BitSize;
    static constexpr int Offset = 0;
    static constexpr SameSizeUInt Mask = ((One << BitSize) - One) << Offset;
  };
  struct Exp {
    static constexpr int BitSize = FloatDescrImpl<FloatT>::Exp::BitSize;
    static constexpr int Offset = Frac::BitSize;
    static constexpr int Bias = FloatDescrImpl<FloatT>::Exp::Bias;
    static constexpr SameSizeUInt MaxUVal = (One << BitSize) - One;
    static constexpr SameSizeUInt MinUVal = 0;
    static constexpr SameSizeSInt MaxSVal =
      static_cast<SameSizeSInt>(MaxUVal) - Bias;
    static constexpr SameSizeSInt MinSVal = -Bias;
    static constexpr SameSizeUInt Mask = MaxUVal << Offset;
  };
  struct Sign {
    static constexpr int BitSize = 1;
    static constexpr int Offset = Frac::BitSize + Exp::BitSize;
    static constexpr SameSizeUInt Mask = One << Offset;
  };
  SameSizeUInt PosInfUInt = Exp::Mask;
  SameSizeUInt NegInfUInt = Exp::Mask + Sign::Mask;
};

/* returns the value of exp in float as it is stored
 * returns int in range [0, 1 << Exp::BitSize)
 */
template <typename FloatT, int width>
CM_INLINE vector<int, width> get_biased_exp(vector<FloatT, width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");

  using UInt = typename FloatDescr<FloatT>::SameSizeUInt;
  vector<UInt, width> uarg = arg.format<UInt>();
  vector<unsigned, width> dword_arg;
  int exp_offset = FloatDescr<FloatT>::Exp::Offset;
  int exp_width = FloatDescr<FloatT>::Exp::BitSize;
  if constexpr (sizeof(UInt) > sizeof(unsigned)) {
    dword_arg = details::read_region<width, 2>(uarg.format<unsigned>(), 1);
    exp_offset -= 32;
  } else
    dword_arg = uarg;

  return cm_bf_extract<int>(exp_width, exp_offset, dword_arg);
}

/* returns signed exponent according to IEEE representation
 * arg = 1.xxxxxxxx * 2^exp   <- this exponent
 */
template<typename FloatT, int Width>
CM_INLINE vector<int, Width>
get_exp(vector<FloatT, Width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");

  vector<int, Width> exp = get_biased_exp(arg);
  exp -= FloatDescr<FloatT>::Exp::Bias;

  return exp;
}

/* returns the value of fraction in float as it is stored
 * returns int in range [0, 1 << Fract::BitSize)
 */
template<typename FloatT, int Width>
CM_INLINE vector<typename FloatDescr<FloatT>::SameSizeSInt, Width>
get_frac(vector<FloatT, Width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");

  using SSUInt = typename FloatDescr<FloatT>::SameSizeUInt;
  vector<SSUInt, Width> frac;

  frac = arg.format<SSUInt>() & FloatDescr<FloatT>::Frac::Mask;
  frac >>= FloatDescr<FloatT>::Frac::Offset;

  return frac;
}

/* sets exponent in floating number
 * arg = 1.xxxxxxxx * 2^exp   <- this exponent
 *
 * \p arg - old value
 * \p new_exp - new exponent to be set,
 *              each element must be in range [Exp::MinSVal, Exp::MaxSVal]
 *
 */
template<typename IntT, typename FloatT, int width>
CM_INLINE vector<FloatT, width>
set_exp(vector<FloatT, width> arg, vector<IntT, width> new_exp) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  static_assert(std::is_integral<IntT>::value,
      "expected integral value as argument");

  using UInt = typename FloatDescr<FloatT>::SameSizeUInt;
  vector<unsigned, width> biased_exp = new_exp + FloatDescr<FloatT>::Exp::Bias;

  vector<UInt, width> uarg = arg.format<UInt>();
  vector<unsigned, width> dword_arg;
  int exp_offset = FloatDescr<FloatT>::Exp::Offset;
  int exp_width = FloatDescr<FloatT>::Exp::BitSize;
  if constexpr (sizeof(UInt) > sizeof(unsigned)) {
    dword_arg = details::read_region<width, 2>(uarg.format<unsigned>(), 1);
    exp_offset -= 32;
  } else
    dword_arg = uarg;

  vector<unsigned, width> dword_res =
    cm_bf_insert<unsigned>(exp_width, exp_offset, biased_exp, dword_arg);
  if constexpr (sizeof(UInt) > sizeof(unsigned))
    details::write_region<2>(uarg.format<unsigned>(), dword_res, 1);
  else
    uarg = dword_res;
  return uarg.format<FloatT>();
}

template<typename IntT, typename FloatT, int Width>
CM_INLINE vector<FloatT, Width>
set_exp(vector<FloatT, Width> arg, IntT new_exp) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  static_assert(std::is_integral<IntT>::value,
      "expected integral value as argument");

  vector<IntT, Width> vec_exp = new_exp;
  return set_exp(arg, vec_exp);
}

/*   \exp|        |       |              |
 *    \  |    0   |   ~0  |     other    |
 * frac\ |        |       |              |
 * _____\|________|_______|______________|
 *       |        |       |              |
 *   0   |  zero  |  inf  | (power of 2) |
 * ______|________|_______|--normalized--|
 *       |        |       |              |
 * other |  subn  |  nan  |              |
 * ______|________|_______|______________|
 *
 * the concrete class of float is easy to define
 * using the following 3 basis functions:
 * 1) is_zero_or_subn             (exp  ==  0)
 * 2) is_inf_or_nan               (exp  == ~0)
 * 3) is_zero_or_inf_or_pow2norm  (frac ==  0)
 */

template<typename FloatT, int width>
CM_INLINE Mask<width>
is_zero_or_subn(vector<FloatT, width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  auto exp = detail::get_biased_exp(arg);
  // max exp bit size is 11 for double
  auto exp_lo = exp.format<int16_t>().select<width, 2>(0);
  return Mask<width>(exp_lo == 0);
}

template<typename FloatT, int Width>
CM_INLINE Mask<Width>
is_inf_or_nan(vector<FloatT, Width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  auto exp = detail::get_biased_exp(arg);
  return Mask<Width>(exp == FloatDescr<FloatT>::Exp::MaxUVal);
}

template<typename FloatT, int Width>
CM_INLINE Mask<Width>
is_zero_or_inf_or_pow2norm(vector<FloatT, Width> arg) {
  static_assert(std::is_floating_point<FloatT>::value,
      "expected floating point value as argument");
  auto frac = detail::get_frac(arg);
  return Mask<Width>(frac == 0);
}

} // namespace detail
} // namespace math
} // namespace cmtl
#endif //  CM_CMTL_MATH_IMPL_FLOAT_H
