/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:sampler/sample.h should not be included explicitly");
#endif

#ifndef _CLANG_CM_SAMPLER_SAMPLE_H_
#define _CLANG_CM_SAMPLER_SAMPLE_H_

#include "helper.h"

enum class CM3DSampleOp : int {
  _CM_3D_SAMPLE = 0,
  _CM_3D_SAMPLE_B = 1,
  _CM_3D_SAMPLE_L = 2,
  _CM_3D_SAMPLE_C = 3,
  _CM_3D_SAMPLE_D = 4,
  _CM_3D_SAMPLE_B_C = 5,
  _CM_3D_SAMPLE_L_C = 6,
  _CM_3D_LOD = 9,
  _CM_3D_SAMPLE_D_C = 20,
  _CM_3D_SAMPLE_LZ = 24,
  _CM_3D_SAMPLE_C_LZ = 25,
  _CM_3D_SAMPLE_NULLMASK_ENABLE = 32,
  _CM_3D_SAMPLE_CPS_LOD_COMP_ENABLE = 64
};

inline constexpr CM3DSampleOp operator|(CM3DSampleOp L, CM3DSampleOp R) {
  return static_cast<CM3DSampleOp>(static_cast<int>(L) | static_cast<int>(R));
}

inline constexpr CM3DSampleOp &operator|=(CM3DSampleOp &L, CM3DSampleOp R) {
  return (L = L | R);
}

#define CM_3D_SAMPLE CM3DSampleOp::_CM_3D_SAMPLE
#define CM_3D_SAMPLE_B CM3DSampleOp::_CM_3D_SAMPLE_B
#define CM_3D_SAMPLE_L CM3DSampleOp::_CM_3D_SAMPLE_L
#define CM_3D_SAMPLE_C CM3DSampleOp::_CM_3D_SAMPLE_C
#define CM_3D_SAMPLE_D CM3DSampleOp::_CM_3D_SAMPLE_D
#define CM_3D_SAMPLE_B_C CM3DSampleOp::_CM_3D_SAMPLE_B_C
#define CM_3D_SAMPLE_L_C CM3DSampleOp::_CM_3D_SAMPLE_L_C
#define CM_3D_LOD CM3DSampleOp::_CM_3D_LOD
#define CM_3D_SAMPLE_D_C CM3DSampleOp::_CM_3D_SAMPLE_D_C
#define CM_3D_SAMPLE_LZ CM3DSampleOp::_CM_3D_SAMPLE_LZ
#define CM_3D_SAMPLE_C_LZ CM3DSampleOp::_CM_3D_SAMPLE_C_LZ
#define CM_3D_SAMPLE_NULLMASK_ENABLE CM3DSampleOp::_CM_3D_SAMPLE_NULLMASK_ENABLE
#define CM_3D_SAMPLE_CPS_LOD_COMP_ENABLE                                       \
  CM3DSampleOp::_CM_3D_SAMPLE_CPS_LOD_COMP_ENABLE

namespace details {
template <CM3DSampleOp Op, ChannelMaskType Ch, typename T, int N,
          typename... Args>
void __cm_intrinsic_impl_3d_sample(vector_ref<T, N> Dst, uint16_t AOffImmI,
                                   SamplerIndex Sampler, SurfaceIndex Image,
                                   Args... Srcs);
} // namespace details

template <CM3DSampleOp Op, ChannelMaskType Ch, typename T, int N,
          typename... Args>
CM_NODEBUG CM_INLINE void
cm_3d_sample(vector_ref<T, N> Dst, uint16_t AOffImmI, SamplerIndex Sampler,
             SurfaceIndex Image, Args... Srcs) {
  details::__cm_intrinsic_impl_3d_sample<Op, Ch>(Dst, AOffImmI, Sampler, Image,
                                                  Srcs...);
}

#ifdef CM_HAS_3D_SAMPLE_HALF
#define CM_HAS_3D_SAMPLE_HALF_CONTROL CM_HAS_CONTROL(true)
#else
#define CM_HAS_3D_SAMPLE_HALF_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_3D_SAMPLE_HALF

template <CM3DSampleOp Op, ChannelMaskType Ch, typename T, int N, int M,
          typename... Args>
CM_NODEBUG CM_INLINE void
cm_3d_sample(vector_ref<T, N> Dst, uint16_t AOffImmI, SamplerIndex Sampler,
             SurfaceIndex Image, vector<half, M> Src0, Args... Srcs) {
  CM_HAS_3D_SAMPLE_HALF_CONTROL;
  details::__cm_intrinsic_impl_3d_sample<Op, Ch>(Dst, AOffImmI, Sampler, Image,
                                                  Src0, Srcs...);
}

template <CM3DSampleOp Op, ChannelMaskType Ch, typename T, int N, int M,
          typename... Args>
CM_NODEBUG CM_INLINE void
cm_3d_sample(vector_ref<T, N> Dst, uint16_t AOffImmI, SamplerIndex Sampler,
             SurfaceIndex Image, vector_ref<half, M> Src0, Args... Srcs) {
  CM_HAS_3D_SAMPLE_HALF_CONTROL;
  details::__cm_intrinsic_impl_3d_sample<Op, Ch>(Dst, AOffImmI, Sampler, Image,
                                                  Src0, Srcs...);
}

template <CM3DSampleOp Op, ChannelMaskType Ch, typename T, int N, int M1,
          int M2, typename... Args>
CM_NODEBUG CM_INLINE void
cm_3d_sample(vector_ref<T, N> Dst, uint16_t AOffImmI, SamplerIndex Sampler,
             SurfaceIndex Image, matrix<half, M1, M2> Src0, Args... Srcs) {
  CM_HAS_3D_SAMPLE_HALF_CONTROL;
  details::__cm_intrinsic_impl_3d_sample<Op, Ch>(Dst, AOffImmI, Sampler, Image,
                                                  Src0, Srcs...);
}

template <CM3DSampleOp Op, ChannelMaskType Ch, typename T, int N, int M1,
          int M2, typename... Args>
CM_NODEBUG CM_INLINE void
cm_3d_sample(vector_ref<T, N> Dst, uint16_t AOffImmI, SamplerIndex Sampler,
             SurfaceIndex Image, matrix_ref<half, M1, M2> Src0,
             Args... Srcs) {
  CM_HAS_3D_SAMPLE_HALF_CONTROL;
  details::__cm_intrinsic_impl_3d_sample<Op, Ch>(Dst, AOffImmI, Sampler, Image,
                                                  Src0, Srcs...);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE std::enable_if_t<details::is_fp_or_dword_type<T>::value>
sample16(matrix_ref<T, N, 16> Dst, ChannelMaskType ChannelMask,
         SurfaceIndex Image, SamplerIndex Sampler, vector<float, 16> U,
         vector<float, 16> V = 0.0f, vector<float, 16> R = 0.0f)
    CM_DEPRECATED("use cm_3d_sample instead") {
#define SAMPLE16(M)                                                            \
  case CM_##M##_ENABLE:                                                        \
    if constexpr (N >= details::getNumChannels(CM_##M##_ENABLE))               \
      cm_3d_sample<CM_3D_SAMPLE, CM_##M##_ENABLE>(Dst.format<T>(), 0, Sampler, \
                                                  Image, U, V, R);             \
    break

  switch (ChannelMask) {
    SAMPLE16(R);
    SAMPLE16(G);
    SAMPLE16(GR);
    SAMPLE16(B);
    SAMPLE16(BR);
    SAMPLE16(BG);
    SAMPLE16(BGR);
    SAMPLE16(A);
    SAMPLE16(AR);
    SAMPLE16(AG);
    SAMPLE16(AGR);
    SAMPLE16(AB);
    SAMPLE16(ABR);
    SAMPLE16(ABG);
    SAMPLE16(ABGR);
  default:
    break;
  }
#undef SAMPLE16
}

template <typename T, int N>
CM_NODEBUG CM_INLINE std::enable_if_t<details::is_fp_or_dword_type<T>::value>
sample16(matrix_ref<T, N, 16> Dst, vector_ref<uint16_t, 1> NullMask,
         ChannelMaskType ChannelMask, SurfaceIndex Image, SamplerIndex Sampler,
         vector<float, 16> U, vector<float, 16> V = 0.0f,
         vector<float, 16> R = 0.0f) CM_DEPRECATED("use cm_3d_sample instead") {
  matrix<T, N + 1, 16> DstWithNullMask;

#define SAMPLE16(M)                                                            \
  case CM_##M##_ENABLE:                                                        \
    if constexpr (N >= details::getNumChannels(CM_##M##_ENABLE))               \
      cm_3d_sample<CM_3D_SAMPLE | CM_3D_SAMPLE_NULLMASK_ENABLE,                \
                   CM_##M##_ENABLE>(DstWithNullMask.format<T>(), 0, Sampler,   \
                                    Image, U, V, R);                           \
    break

  switch (ChannelMask) {
    SAMPLE16(R);
    SAMPLE16(G);
    SAMPLE16(GR);
    SAMPLE16(B);
    SAMPLE16(BR);
    SAMPLE16(BG);
    SAMPLE16(BGR);
    SAMPLE16(A);
    SAMPLE16(AR);
    SAMPLE16(AG);
    SAMPLE16(AGR);
    SAMPLE16(AB);
    SAMPLE16(ABR);
    SAMPLE16(ABG);
    SAMPLE16(ABGR);
  default:
    break;
  }
#undef SAMPLE16

  Dst = DstWithNullMask.template select<N, 1, 16, 1>(0, 0);
  NullMask = DstWithNullMask.template select<1, 1, 16, 1>(N, 0)
                 .template format<uint16_t>()
                 .template select<1, 1>(0);
}

#endif // _CLANG_CM_SAMPLER_SAMPLE_H_
