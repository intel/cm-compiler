/*========================== begin_copyright_notice ============================

Copyright (C) 2014-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_sampler.h should not be included explicitly");
#endif

#ifndef _CLANG_CM_SAMPLER_H_
#define _CLANG_CM_SAMPLER_H_

#include "cm_common.h"
#include "cm_traits.h"

/// \brief Sampler interface.
///
/// \param m the matrix to store the return results, where N is at least the
/// number of enabled channels.
///
/// \param channelMask enabled channels which must be a compile time constant.
///
/// \param surfIndex an abstract handle that represents the surface created by
/// CM host runtime and must be passed through kernel function parameters. CM
/// does not allow the explicit use of local / global variable or modification
/// of such abstract data types in kernel functions, except used as function
/// call argument.
///
/// \param sampIndex the index into the sampler state table. This is an abstract
/// handle that represents the sampler state created by CM host runtime and
/// must be passed through kernel function parameters.CM does not allow the
/// explicit use of local / global variable or modification of such abstract
/// data types in kernel functions, except used as function call argument.
///
/// \param u the normalized x coordinate of pixel 0.
///
/// \param v the normalized y coordinate of pixel 0.
///
/// \param deltaU the difference in coordinates for adjacent pixels in the x
/// direction.
///
/// \param deltaV the difference in coordinates for adjacent pixels in the y
/// direction.
///
/// \param ofc output format control parameter. The following are valid values
/// to use for this parameter:
///
/// - CM_16_FULL two bytes returned for each pixel.
///
/// - CM_16_DOWN_SAMPLE 16 bit chrominance downsampled. Like CM_16_FULL except
///   only even pixels are returned for R and B channels.
///
/// - CM_8_FULL one byte returned for each pixel.
///
/// - CM_8_DOWN_SAMPLE 8 but chrominance downsampled. Like CM_8_FULL but only
///   even pixels are returned for R and B channels.
///
template <int N>
CM_NODEBUG CM_INLINE void
sample32(matrix_ref<ushort, N, 32> m, ChannelMaskType channelMask,
         SurfaceIndex surfIndex, SamplerIndex sampIndex, float u, float v,
         float deltaU, float deltaV, OutputFormatControl ofc = CM_16_FULL) {
  CM_HAS_SAMPLE_UNORM_CONTROL;

#define SAMPLE32(mask)                                                         \
  switch (ofc) {                                                               \
  default:                                                                     \
    break;                                                                     \
  case CM_16_FULL:                                                             \
    m = details::__cm_intrinsic_impl_sample32<N, mask, CM_16_FULL>(            \
        sampIndex, surfIndex, u, v, deltaU, deltaV);                           \
    break;                                                                     \
  case CM_16_DOWN_SAMPLE:                                                      \
    m = details::__cm_intrinsic_impl_sample32<N, mask, CM_16_DOWN_SAMPLE>(     \
        sampIndex, surfIndex, u, v, deltaU, deltaV);                           \
    break;                                                                     \
  case CM_8_FULL:                                                              \
    m = details::__cm_intrinsic_impl_sample32<N, mask, CM_8_FULL>(             \
        sampIndex, surfIndex, u, v, deltaU, deltaV);                           \
    break;                                                                     \
  case CM_8_DOWN_SAMPLE:                                                       \
    m = details::__cm_intrinsic_impl_sample32<N, mask, CM_8_DOWN_SAMPLE>(      \
        sampIndex, surfIndex, u, v, deltaU, deltaV);                           \
    break;                                                                     \
  }

  switch (channelMask) {
  default:
    break;
  case CM_R_ENABLE:
    SAMPLE32(CM_R_ENABLE)
    break;
  case CM_G_ENABLE:
    SAMPLE32(CM_G_ENABLE)
    break;
  case CM_GR_ENABLE:
    SAMPLE32(CM_GR_ENABLE)
    break;
  case CM_B_ENABLE:
    SAMPLE32(CM_B_ENABLE)
    break;
  case CM_BR_ENABLE:
    SAMPLE32(CM_BR_ENABLE)
    break;
  case CM_BG_ENABLE:
    SAMPLE32(CM_BG_ENABLE)
    break;
  case CM_BGR_ENABLE:
    SAMPLE32(CM_BGR_ENABLE)
    break;
  case CM_A_ENABLE:
    SAMPLE32(CM_A_ENABLE)
    break;
  case CM_AR_ENABLE:
    SAMPLE32(CM_AR_ENABLE)
    break;
  case CM_AG_ENABLE:
    SAMPLE32(CM_AG_ENABLE)
    break;
  case CM_AGR_ENABLE:
    SAMPLE32(CM_AGR_ENABLE)
    break;
  case CM_AB_ENABLE:
    SAMPLE32(CM_AB_ENABLE)
    break;
  case CM_ABR_ENABLE:
    SAMPLE32(CM_ABR_ENABLE)
    break;
  case CM_ABG_ENABLE:
    SAMPLE32(CM_ABG_ENABLE)
    break;
  case CM_ABGR_ENABLE:
    SAMPLE32(CM_ABGR_ENABLE)
    break;
  }

#undef SAMPLE32
}

#endif // _CLANG_CM_SAMPLER_H_
