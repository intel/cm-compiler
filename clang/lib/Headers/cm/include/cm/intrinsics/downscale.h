/*========================== begin_copyright_notice ============================

Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0,
              "CM:w:intrinsics/downscale.h should not be included explicitly");
#endif

#ifndef _CLANG_CM_INTRINSICS_DOWNSCALE_4BIT_H_
#define _CLANG_CM_INTRINSICS_DOWNSCALE_4BIT_H_

#include <cm/cm_common.h>
#include <cm/cm_has_instr.h>
#include <cm/cm_traits.h>

#ifdef CM_HAS_DOWNSCALE_4BIT
#define CM_HAS_DOWNSCALE_4BIT_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_DOWNSCALE_4BIT
#define CM_HAS_DOWNSCALE_4BIT_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_DOWNSCALE_4BIT

namespace downscale {
enum Mode {
  // { downscale(src0[0:15]) | downscale(src0[16:31]) << 4, 0,
  //   downscale(src1[0:15]) | downscale(src1[16:31]) << 4, 0 }
  Mode0 = 0,
  // { downscale(src0[0:15]) | downscale(src1[0:15]) << 4, 0,
  //   downscale(src0[16:31]) | downscale(src1[16:31]) << 4, 0 }
  Mode1 = 1,
  // { 0, downscale(src0[0:15]) | downscale(src0[16:31]) << 4,
  //   0, downscale(src1[0:15]) | downscale(src1[16:31]) << 4 }
  Mode2 = 2,
  // { 0, downscale(src0[0:15]) | downscale(src1[0:15]) << 4,
  //   0, downscale(src0[16:31]) | downscale(src1[16:31]) << 4 }
  Mode3 = 3,
};

enum Type {
  E2M1 = 1,
  Int4 = 2,
};
} // namespace downscale

namespace details {
template <unsigned Width>
vector<uint32_t, Width> __cm_intrinsic_impl_downconvert_4bit(
    vector<uint32_t, Width> Src0, vector<uint32_t, Width> Src1,
    vector<uint32_t, Width> Bias, uint8_t CvtType, uint8_t Mode,
    uint8_t RoundingMode);

template <typename InputTy, downscale::Type OutputTy>
struct Downscale4bitCvtType;

template <downscale::Type OutputTy>
struct Downscale4bitCvtType<half, OutputTy>
    : std::integral_constant<uint8_t, OutputTy + 3> {};

#ifdef CM_HAS_BF16
template <downscale::Type OutputTy>
struct Downscale4bitCvtType<__bf16, OutputTy>
    : std::integral_constant<uint8_t, OutputTy> {};
#endif // CM_HAS_BF16

template <downscale::Type OutputTy>
struct Downscale4bitCvtType<int16_t, OutputTy>
    : std::integral_constant<uint8_t, OutputTy> {};

template <typename InputTy> struct IsDownscale4bitInputType : std::false_type {};

template <> struct IsDownscale4bitInputType<half> : std::true_type {};

#ifdef CM_HAS_BF16
template <> struct IsDownscale4bitInputType<__bf16> : std::true_type {};
#endif // CM_HAS_BF16

template <> struct IsDownscale4bitInputType<int16_t> : std::true_type {};

enum RoundingMode {
  Biased = 0,
  RTNE = 1,
};

template <downscale::Type OutputTy, downscale::Mode Mode, typename InputTy,
          unsigned Width>
CM_NODEBUG CM_INLINE void downscale_check() {
  CM_HAS_DOWNSCALE_4BIT_CONTROL;

  constexpr bool IsValidInputTy = IsDownscale4bitInputType<InputTy>::value;
  CM_STATIC_ERROR(IsValidInputTy,
                  "Such InputTy is not supported");

  CM_STATIC_ERROR(OutputTy == downscale::E2M1 || OutputTy == downscale::Int4,
                  "OutputTy must be downscale::E2M1 or downscale::Int4");
  CM_STATIC_ERROR(Mode == downscale::Mode0 || Mode == downscale::Mode1 ||
                  Mode == downscale::Mode2 || Mode == downscale::Mode3,
                  "Mode must be downscale::Mode0, downscale::Mode1, "
                  "downscale::Mode2 or downscale::Mode3");
  CM_STATIC_ERROR(Width % 2 == 0, "Width must be even");
}
} // namespace details

template <downscale::Type OutputTy, downscale::Mode Mode, typename InputTy,
          unsigned Width,
          unsigned IntrWidth = Width * sizeof(InputTy) / sizeof(uint32_t)>
CM_NODEBUG CM_INLINE vector<uint32_t, IntrWidth>
cm_downscale(vector<InputTy, Width> Src0, vector<InputTy, Width> Src1) {
  details::downscale_check<OutputTy, Mode, InputTy, Width>();

  constexpr auto CvtType =
      details::Downscale4bitCvtType<InputTy, OutputTy>::value;

  auto _Src0 = Src0.template format<uint32_t>();
  auto _Src1 = Src1.template format<uint32_t>();
  vector<uint32_t, IntrWidth> Undef;

  return details::__cm_intrinsic_impl_downconvert_4bit(
      _Src0, _Src1, Undef, CvtType, Mode, details::RTNE);
}

template <downscale::Type OutTy, downscale::Mode Mode, typename Ty,
          unsigned Width, unsigned BiasWidth>
CM_NODEBUG CM_INLINE vector<uint32_t, BiasWidth>
cm_downscale(vector<Ty, Width> Src0, vector<Ty, Width> Src1,
             vector<uint32_t, BiasWidth> Bias) {
  details::downscale_check<OutTy, Mode, Ty, Width>();

  constexpr auto CvtType = details::Downscale4bitCvtType<Ty, OutTy>::value;
  constexpr unsigned IntrWidth = Width * sizeof(Ty) / sizeof(uint32_t);
  static_assert(IntrWidth == BiasWidth,
                "Bias width must match formatted input width");

  auto _Src0 = Src0.template format<uint32_t>();
  auto _Src1 = Src1.template format<uint32_t>();

  return details::__cm_intrinsic_impl_downconvert_4bit(
      _Src0, _Src1, Bias, CvtType, Mode, details::Biased);
}

#endif // _CLANG_CM_INTRINSICS_DOWNSCALE_4BIT_H_
