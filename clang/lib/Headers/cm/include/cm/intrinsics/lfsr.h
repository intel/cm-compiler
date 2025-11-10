/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:intrinsics/lfsr.h should not be included explicitly");
#endif

#ifndef _CLANG_CM_INTRINSICS_LFSR_H_
#define _CLANG_CM_INTRINSICS_LFSR_H_

#include <cm/cm_common.h>
#include <cm/cm_has_instr.h>
#include <cm/cm_traits.h>

#ifdef CM_HAS_LFSR
#define CM_HAS_LFSR_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_LFSR
#define CM_HAS_LFSR_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_LFSR

namespace details {
template <unsigned Width>
vector<uint32_t, Width> __cm_intrinsic_impl_lfsr(vector<uint32_t, Width> Seed,
                                                 vector<uint32_t, Width> Poly,
                                                 uint8_t Mode);

template <typename Ty, unsigned Width>
using EnableIfLfsrValidTy =
    std::enable_if_t<details::is_one_of_v<Ty, int8_t, uint8_t, int16_t,
                                          uint16_t, int32_t, uint32_t> &&
                         (Width * sizeof(Ty) % sizeof(uint32_t) == 0),
                     vector<Ty, Width> >;

template <typename Ty> struct LfsrMode;

template <> struct LfsrMode<uint8_t> : std::integral_constant<uint8_t, 2> {};
template <> struct LfsrMode<uint16_t> : std::integral_constant<uint8_t, 1> {};
template <> struct LfsrMode<uint32_t> : std::integral_constant<uint8_t, 0> {};
template <> struct LfsrMode<int8_t> : std::integral_constant<uint8_t, 2> {};
template <> struct LfsrMode<int16_t> : std::integral_constant<uint8_t, 1> {};
template <> struct LfsrMode<int32_t> : std::integral_constant<uint8_t, 0> {};
} // namespace details

template <typename Ty, unsigned Width>
CM_NODEBUG CM_INLINE details::EnableIfLfsrValidTy<Ty, Width>
cm_lfsr(vector<Ty, Width> Seed, vector<Ty, Width> Poly) {
  CM_HAS_LFSR_CONTROL;

  constexpr auto Mode = details::LfsrMode<Ty>::value;

  auto _Seed = Seed.template format<uint32_t>();
  auto _Poly = Poly.template format<uint32_t>();

  auto _Res = details::__cm_intrinsic_impl_lfsr(_Seed, _Poly, Mode);

  return _Res.template format<Ty>();
}

template <typename Ty, unsigned Width>
CM_NODEBUG CM_INLINE details::EnableIfLfsrValidTy<Ty, Width>
cm_lfsr(vector<Ty, Width> Seed,
        vector<Ty, sizeof(uint32_t) / sizeof(Ty)> Poly) {
  constexpr auto ElementsPerLane = sizeof(uint32_t) / sizeof(Ty);

  vector<uint32_t, Width / ElementsPerLane> _Poly =
      Poly.template format<uint32_t>()[0];

  return cm_lfsr(Seed, _Poly.template format<Ty>());
}

template <typename Ty, unsigned Width>
CM_NODEBUG CM_INLINE details::EnableIfLfsrValidTy<Ty, Width>
cm_lfsr(vector<Ty, Width> Seed, Ty Poly) {
  constexpr auto ElementsPerLane = sizeof(uint32_t) / sizeof(Ty);

  vector<Ty, ElementsPerLane> _Poly = Poly;

  return cm_lfsr(Seed, _Poly);
}

#endif // _CLANG_CM_INTRINSICS_LFSR_H_
