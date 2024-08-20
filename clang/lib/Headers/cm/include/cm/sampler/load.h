/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:sampler/load.h should not be included explicitly - "
                 "only <cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_SAMPLER_LOAD_H_
#define _CLANG_CM_SAMPLER_LOAD_H_

#include "helper.h"

enum class CM3DLoadOp : int {
  _CM_3D_LOAD = 7,
  _CM_3D_LOAD_LZ = 26,
  _CM_3D_LOAD_L = 27,
  _CM_3D_LOAD_2DMS_W = 28,
  _CM_3D_LOAD_MCS = 29,
  _CM_3D_LOAD_NULLMASK_ENABLE = 32
};

inline CM3DLoadOp operator|(CM3DLoadOp L, CM3DLoadOp R) {
  return static_cast<CM3DLoadOp>(static_cast<int>(L) | static_cast<int>(R));
}

inline CM3DLoadOp &operator|=(CM3DLoadOp &L, CM3DLoadOp R) {
  return (L = L | R);
}

#define CM_3D_LOAD CM3DLoadOp::_CM_3D_LOAD
#define CM_3D_LOAD_LZ CM3DLoadOp::_CM_3D_LOAD_LZ
#define CM_3D_LOAD_L CM3DLoadOp::_CM_3D_LOAD_L
#define CM_3D_LOAD_2DMS_W CM3DLoadOp::_CM_3D_LOAD_2DMS_W
#define CM_3D_LOAD_MCS CM3DLoadOp::_CM_3D_LOAD_MCS
#define CM_3D_LOAD_NULLMASK_ENABLE CM3DLoadOp::_CM_3D_LOAD_NULLMASK_ENABLE

template <CM3DLoadOp Op, ChannelMaskType Ch, typename T, int N,
          typename... Args>
void cm_3d_load(vector_ref<T, N> Dst, uint16_t AOffImmI, SurfaceIndex Image,
                Args... Srcs);

template <typename T, int N>
CM_NODEBUG CM_INLINE std::enable_if_t<details::is_fp_or_dword_type<T>::value>
load16(matrix_ref<T, N, 16> Dst, ChannelMaskType ChannelMask,
       SurfaceIndex Image, vector<uint32_t, 16> U, vector<uint32_t, 16> V = 0,
       vector<uint32_t, 16> R = 0) {
#define LOAD16(M)                                                              \
  case CM_##M##_ENABLE:                                                        \
    if constexpr (N >= details::getNumChannels(CM_##M##_ENABLE))               \
      cm_3d_load<CM_3D_LOAD_LZ, CM_##M##_ENABLE>(Dst.format<T>(), 0, Image, U, \
                                                 V, R);                        \
    break

  switch (ChannelMask) {
    LOAD16(R);
    LOAD16(G);
    LOAD16(GR);
    LOAD16(B);
    LOAD16(BR);
    LOAD16(BG);
    LOAD16(BGR);
    LOAD16(A);
    LOAD16(AR);
    LOAD16(AG);
    LOAD16(AGR);
    LOAD16(AB);
    LOAD16(ABR);
    LOAD16(ABG);
    LOAD16(ABGR);
  default:
    break;
  }
#undef LOAD16
}

template <typename T, int N>
CM_NODEBUG CM_INLINE std::enable_if_t<details::is_fp_or_dword_type<T>::value>
load16(matrix_ref<T, N, 16> Dst, vector_ref<uint16_t, 1> NullMask,
       ChannelMaskType ChannelMask, SurfaceIndex Image, vector<uint32_t, 16> U,
       vector<uint32_t, 16> V = 0, vector<uint32_t, 16> R = 0) {
  matrix<T, N + 1, 16> DstWithNullMask;

#define LOAD16(M)                                                              \
  case CM_##M##_ENABLE:                                                        \
    if constexpr (N >= details::getNumChannels(CM_##M##_ENABLE))               \
      cm_3d_load<CM_3D_LOAD_LZ | CM_3D_LOAD_NULLMASK_ENABLE, CM_##M##_ENABLE>( \
          DstWithNullMask.format<T>(), 0, Image, U, V, R);                     \
    break

  switch (ChannelMask) {
    LOAD16(R);
    LOAD16(G);
    LOAD16(GR);
    LOAD16(B);
    LOAD16(BR);
    LOAD16(BG);
    LOAD16(BGR);
    LOAD16(A);
    LOAD16(AR);
    LOAD16(AG);
    LOAD16(AGR);
    LOAD16(AB);
    LOAD16(ABR);
    LOAD16(ABG);
    LOAD16(ABGR);
  default:
    break;
  }
#undef LOAD16

  Dst = DstWithNullMask.template select<N, 1, 16, 1>(0, 0);
  NullMask = DstWithNullMask.template select<1, 1, 16, 1>(N, 0)
                 .template format<uint16_t>()
                 .template select<1, 1>(0);
}
#endif // _CLANG_CM_SAMPLER_LOAD_H_
