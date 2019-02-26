/*
 * Copyright (c) 2019, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_sampler.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_SAMPLER_H_
#define _CLANG_CM_SAMPLER_H_

#include "cm_common.h"
#include "cm_traits.h"

// Prototype 3d sampler underlying functions here as used by others in the
// header
template <CM3DSampleOp Op, ChannelMaskType Ch, typename T, int N,
          typename... Args>
void cm_3d_sample(vector_ref<T, N> dst, ushort Aoffimmi,
                  SamplerIndex sampIndex, SurfaceIndex surfIndex,
                  Args... args);
template <CM3DLoadOp Op, ChannelMaskType Ch, typename T, int N,
          typename... Args>
void cm_3d_load(vector_ref<T, N> dst, ushort Aofimmi, SurfaceIndex surfIndex,
                Args... args);

/// \brief Sampler interface.
///
/// \param m the matrix to store the return results, where N is at least the
/// number of enabled channels. The element type T can be one of float, int, or
/// unsigned int based on the format of the surface being sampled.
///
/// \param nullmask a 1-word vector to store the null pixel mask result.
/// Optional.
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
/// \param u the normalized x coordinates of the texels to be sampled.
///
/// \param v the normalized y coordinates for non-1D surface types; ignored
/// otherwise.
///
/// \param r the normalized z coordinates for any 3D surface types; ignored
/// otherwise.
///
template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value, void>::type
sample16(matrix_ref<T, N, 16> m, ChannelMaskType channelMask,
         SurfaceIndex surfIndex, SamplerIndex sampIndex, vector<float, 16> u,
         vector<float, 16> v = 0, vector<float, 16> r = 0) {
#if !(defined(CM_GEN7_5) || defined(CM_GEN8))
#define SAMPLE16(mask)                                                         \
  cm_3d_sample<CM_3D_SAMPLE, mask>(m.format<T>(), 0, sampIndex, surfIndex, u,  \
                                      v, r)
#else
#define SAMPLE16(mask)                                                         \
  m = details::__cm_intrinsic_impl_sample16<T, N, mask>(sampIndex, surfIndex,  \
                                                        u, v, r)
#endif

  switch (channelMask) {
  default:
    break;
  case CM_R_ENABLE:
    SAMPLE16(CM_R_ENABLE);
    break;
  case CM_G_ENABLE:
    SAMPLE16(CM_G_ENABLE);
    break;
  case CM_GR_ENABLE:
    SAMPLE16(CM_GR_ENABLE);
    break;
  case CM_B_ENABLE:
    SAMPLE16(CM_B_ENABLE);
    break;
  case CM_BR_ENABLE:
    SAMPLE16(CM_BR_ENABLE);
    break;
  case CM_BG_ENABLE:
    SAMPLE16(CM_BG_ENABLE);
    break;
  case CM_BGR_ENABLE:
    SAMPLE16(CM_BGR_ENABLE);
    break;
  case CM_A_ENABLE:
    SAMPLE16(CM_A_ENABLE);
    break;
  case CM_AR_ENABLE:
    SAMPLE16(CM_AR_ENABLE);
    break;
  case CM_AG_ENABLE:
    SAMPLE16(CM_AG_ENABLE);
    break;
  case CM_AGR_ENABLE:
    SAMPLE16(CM_AGR_ENABLE);
    break;
  case CM_AB_ENABLE:
    SAMPLE16(CM_AB_ENABLE);
    break;
  case CM_ABR_ENABLE:
    SAMPLE16(CM_ABR_ENABLE);
    break;
  case CM_ABG_ENABLE:
    SAMPLE16(CM_ABG_ENABLE);
    break;
  case CM_ABGR_ENABLE:
    SAMPLE16(CM_ABGR_ENABLE);
    break;
  }

#undef SAMPLE16
}

#if !(defined(CM_GEN7_5) || defined(CM_GEN8))
template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value, void>::type
sample16(matrix_ref<T, N, 16> m, vector_ref<short, 1> nullmask,
         ChannelMaskType channelMask, SurfaceIndex surfIndex,
         SamplerIndex sampIndex, vector<float, 16> u, vector<float, 16> v = 0,
         vector<float, 16> r = 0) {
  vector<T, (N * 16 + (32 / sizeof(T)))> dst3d;

#define SAMPLE16(mask)                                                         \
  cm_3d_sample<CM_3D_SAMPLE | CM_3D_SAMPLE_NULLMASK_ENABLE, mask>(             \
      dst3d, 0, sampIndex, surfIndex, u, v, r)

  switch (channelMask) {
  default:
    break;
  case CM_R_ENABLE:
    SAMPLE16(CM_R_ENABLE);
    break;
  case CM_G_ENABLE:
    SAMPLE16(CM_G_ENABLE);
    break;
  case CM_GR_ENABLE:
    SAMPLE16(CM_GR_ENABLE);
    break;
  case CM_B_ENABLE:
    SAMPLE16(CM_B_ENABLE);
    break;
  case CM_BR_ENABLE:
    SAMPLE16(CM_BR_ENABLE);
    break;
  case CM_BG_ENABLE:
    SAMPLE16(CM_BG_ENABLE);
    break;
  case CM_BGR_ENABLE:
    SAMPLE16(CM_BGR_ENABLE);
    break;
  case CM_A_ENABLE:
    SAMPLE16(CM_A_ENABLE);
    break;
  case CM_AR_ENABLE:
    SAMPLE16(CM_AR_ENABLE);
    break;
  case CM_AG_ENABLE:
    SAMPLE16(CM_AG_ENABLE);
    break;
  case CM_AGR_ENABLE:
    SAMPLE16(CM_AGR_ENABLE);
    break;
  case CM_AB_ENABLE:
    SAMPLE16(CM_AB_ENABLE);
    break;
  case CM_ABR_ENABLE:
    SAMPLE16(CM_ABR_ENABLE);
    break;
  case CM_ABG_ENABLE:
    SAMPLE16(CM_ABG_ENABLE);
    break;
  case CM_ABGR_ENABLE:
    SAMPLE16(CM_ABGR_ENABLE);
    break;
  }

  m = dst3d.select<N * 16, 1>(0).format<T, N, 16>();
  nullmask = dst3d.select<1, 1>(N * 16).format<short>().select<1, 1>(0);

#undef SAMPLE16
}
#endif

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

template <typename T = void>
CM_NODEBUG CM_INLINE void
sample32(vector_ref<ushort, 32> vec, ChannelMaskType channelMask,
         SurfaceIndex surfIndex, SamplerIndex sampIndex, float u, float v,
         float deltaU, float deltaV, OutputFormatControl ofc = CM_16_FULL) {
  matrix_ref<ushort, 1, 32> _Src = vec.format<ushort, 1, 32>();
  sample32(_Src, channelMask, surfIndex, sampIndex, u, v, deltaU, deltaV, ofc);
}

/// \brief Sampler interface.
///
/// \param m the matrix to store the return results, where N is at least the
/// number of enabled channels.
///
/// \param nullmask a 1-word vector to store the null pixel mask result.
/// Optional.
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
/// \param u the unnormalized x coordinates of the texels to be sampled.
///
/// \param v the unnormalized y coordinates for non-1D surface types; ignored
/// otherwise.
///
/// \param r the unnormalized z coordinates for any 3D surface types; ignored
/// otherwise.
///
template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value, void>::type
load16(matrix_ref<T, N, 16> m, ChannelMaskType channelMask,
       SurfaceIndex surfIndex, vector<uint, 16> u, vector<uint, 16> v = 0,
       vector<uint, 16> r = 0) {
#if !(defined(CM_GEN7_5) || defined(CM_GEN8))
#define LOAD16(mask)                                                           \
  cm_3d_load<CM_3D_LOAD_LZ, mask>(m.format<T>(), 0, surfIndex, u, v, r)
#else
#define LOAD16(mask)                                                           \
  m = details::__cm_intrinsic_impl_load16<T, N, mask>(surfIndex, u, v, r)
#endif

  switch (channelMask) {
  default:
    break;
  case CM_R_ENABLE:
    LOAD16(CM_R_ENABLE);
    break;
  case CM_G_ENABLE:
    LOAD16(CM_G_ENABLE);
    break;
  case CM_GR_ENABLE:
    LOAD16(CM_GR_ENABLE);
    break;
  case CM_B_ENABLE:
    LOAD16(CM_B_ENABLE);
    break;
  case CM_BR_ENABLE:
    LOAD16(CM_BR_ENABLE);
    break;
  case CM_BG_ENABLE:
    LOAD16(CM_BG_ENABLE);
    break;
  case CM_BGR_ENABLE:
    LOAD16(CM_BGR_ENABLE);
    break;
  case CM_A_ENABLE:
    LOAD16(CM_A_ENABLE);
    break;
  case CM_AR_ENABLE:
    LOAD16(CM_AR_ENABLE);
    break;
  case CM_AG_ENABLE:
    LOAD16(CM_AG_ENABLE);
    break;
  case CM_AGR_ENABLE:
    LOAD16(CM_AGR_ENABLE);
    break;
  case CM_AB_ENABLE:
    LOAD16(CM_AB_ENABLE);
    break;
  case CM_ABR_ENABLE:
    LOAD16(CM_ABR_ENABLE);
    break;
  case CM_ABG_ENABLE:
    LOAD16(CM_ABG_ENABLE);
    break;
  case CM_ABGR_ENABLE:
    LOAD16(CM_ABGR_ENABLE);
    break;
  }

#undef LOAD16
}

#if !(defined(CM_GEN7_5) || defined(CM_GEN8))
template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    details::is_fp_or_dword_type<T>::value, void>::type
load16(matrix_ref<T, N, 16> m, vector_ref<short, 1> nullmask,
       ChannelMaskType channelMask, SurfaceIndex surfIndex, vector<uint, 16> u,
       vector<uint, 16> v = 0, vector<uint, 16> r = 0) {
  vector<T, (N * 16 + (32 / sizeof(T)))> dst3d;

#define LOAD16(mask)                                                           \
  cm_3d_load<CM_3D_LOAD_LZ | CM_3D_LOAD_NULLMASK_ENABLE, mask>(                \
      dst3d, 0, surfIndex, u, v, r)

  switch (channelMask) {
  default:
    break;
  case CM_R_ENABLE:
    LOAD16(CM_R_ENABLE);
    break;
  case CM_G_ENABLE:
    LOAD16(CM_G_ENABLE);
    break;
  case CM_GR_ENABLE:
    LOAD16(CM_GR_ENABLE);
    break;
  case CM_B_ENABLE:
    LOAD16(CM_B_ENABLE);
    break;
  case CM_BR_ENABLE:
    LOAD16(CM_BR_ENABLE);
    break;
  case CM_BG_ENABLE:
    LOAD16(CM_BG_ENABLE);
    break;
  case CM_BGR_ENABLE:
    LOAD16(CM_BGR_ENABLE);
    break;
  case CM_A_ENABLE:
    LOAD16(CM_A_ENABLE);
    break;
  case CM_AR_ENABLE:
    LOAD16(CM_AR_ENABLE);
    break;
  case CM_AG_ENABLE:
    LOAD16(CM_AG_ENABLE);
    break;
  case CM_AGR_ENABLE:
    LOAD16(CM_AGR_ENABLE);
    break;
  case CM_AB_ENABLE:
    LOAD16(CM_AB_ENABLE);
    break;
  case CM_ABR_ENABLE:
    LOAD16(CM_ABR_ENABLE);
    break;
  case CM_ABG_ENABLE:
    LOAD16(CM_ABG_ENABLE);
    break;
  case CM_ABGR_ENABLE:
    LOAD16(CM_ABGR_ENABLE);
    break;
  }

  m = dst3d.select<N * 16, 1>(0).format<T, N, 16>();
  nullmask = dst3d.select<1, 1>(N * 16).format<short>().select<1, 1>(0);

#undef LOAD16
}
#endif

/// Adaptive Video Scaling (AVS)
///
/// \param m the matrix to sture the return result. The results are returned in
/// ::m with each enabled channel stored in the next row. The actual data
/// returned is determined by a combination of ::channelMask, ::outControl,
/// ::execMode, as well as whether output shuffle is enabled in the sampler
/// state. See the language specification for detailed description.
///
/// \param channelMask Enabled channels, which must be a compile-time constant.
///
/// \param surfIndex The surface index which must correspond to a 2D type
/// surface that has a format with <= 10 bits per channel.
///
/// \param sampIndex The index into the sampler state table.
///
/// \param u The normalized x coordinat of pixel 0.
///
/// \param v The normalized y coordinat of pixel 0.
///
/// \param deltaU The difference in coordinates for adjacent pixels in the X
/// direction.
///
/// \param deltaV The difference in coordinates for adjacent pixels in the Y
/// direction.
///
/// \param u2d Defines the change in the delta U for adjacent pixels in the X
/// direction.
///
/// \param GroupID This field is valid and must be set for Gen7+, for all
/// previous platforms this field is ignored. This parameter will be used to
/// group messages for reorder for sample_8x8 messages. For all messages with
/// the same Group ID they must have the following in common: Surface state,
/// Sampler State, GRFID, M0, and M1 except for Block number.
///
/// \param VerticalBlockNumber This field is valid and must be set for Gen7+,
/// for all previous platforms this field is ignored. This field will specify
/// the vertical block offset being sent for this sample_8x8 messages. This will
/// be equal to the vertical pixel offset from the given address divided by 4.
///
/// \param outControl An enumeration constant that specifies the output format
/// for each pixel.
///
///
/// \param v2d Defines the change in the delta V for adjacent pixels in the Y
/// direction. This parameter is for Gen8+ only, ignored for previous
/// architectures.
///
/// \param execMode An enumeration constant that determines the number of pixels
/// to be returned.
///
/// \param IEFBypass This field enables EIF pass. Gen8 only.
///
template <typename T, int N, int M>
void cm_avs_sampler(matrix_ref<T, N, M> m, ChannelMaskType channelMask,
                    SurfaceIndex surfIndex, SamplerIndex samplerIndex, float u,
                    float v, float deltaU, float deltaV, float u2d,
                    int groupID = -1, int verticalBlockNumber = -1,
                    OutputFormatControl outControl = CM_16_FULL, float v2d = 0,
                    AVSExecMode execMode = CM_AVS_16x4, bool IEFBypass = false);

/// 2d Convolve
///
/// \param m the matrix to store the return result. The output is a matrix of
/// 16-bit signed integers, in 4x16 matrix for CM_CONV_16x4 mode, or a 1x16
/// matrix for CM_CONV_16x1 mode. The data is laid out in sequential pixel
/// order.
///
/// \param surfIndex the surface index, which must correspond to an 8-bit or
/// 16-bit
/// formatted 2D surface.
///
/// \param sampIndex the sampler index
///
/// \param u the normalized x co-ordinate of pixel 0.
///
/// \param v the normalized y co-ordinate of pixel 0.
///
/// \param execMode the execution mode. Two modes are supported, 16x4 mode
/// (CM_CONV_16x4, the default) and 16x1 mode (CM_CONV_16x1). ::execMode must
/// be a compile-time constant and the type of ::m must correspond
/// appropriately.
///
/// \param big_kernel big kernel flag. Ignored on Gen8. On Gen9+ set to true
/// when
/// the kernel is larger than 15x15.
///
template <int N>
void cm_va_2d_convolve(matrix_ref<short, N, 16> m, SurfaceIndex surfIndex,
                       SamplerIndex sampIndex, float u, float v,
                       CONVExecMode execMode = CM_CONV_16x4,
                       bool big_kernel = false);

/// Erode
///
/// \param vect the matrix to store the return result. In 64x4 mode each value
/// int the
/// vector contains 32 values, N is 8. In 64x1 mode, each value in the vector
/// contains
/// 32 values, N is 2. In 32x4 mode each even value [0,6] in the vector contains
/// 32 values, N is 8. In 32x1 mode each value in the vector contains 32 value,
/// N is 1.
///
/// \param surfIndex the surface index, which must correspond to a 32-bit
/// formatted 2D
/// surface.
///
/// \param sampIndex the sampler index.
///
/// \param u the normalized x co-ordinate of pixel 0.
///
/// \param v the normalized y co-ordinate of pixel 0.
///
/// \param execMode the execution mode. There are four modes, 64x4 (CM_ED_64x4,
/// the default),
/// 32x4 (CM_ED_32x4), 64x1 (CM_ED_64x1) and 32x1 (CM_ED_32x1). It is the
/// responsibility of
/// the user to ensure that the return value ::vect is the correct size for
/// mode. ::execMode
/// must be a compile-time constant.
///
template <int N>
void cm_va_erode(vector_ref<uint, N> vect, SurfaceIndex surfIndex,
                 SamplerIndex sampIndex, float u, float v,
                 EDExecMode execMode = CM_ED_64x4);

/// Dilate
///
/// \param vect the matrix to store the return result. In 64x4 mode each value
/// int the
/// vector contains 32 values, N is 8. In 64x1 mode, each value in the vector
/// contains
/// 32 values, N is 2. In 32x4 mode each even value [0,6] in the vector contains
/// 32 values, N is 8. In 32x1 mode each value in the vector contains 32 value,
/// N is 1.
///
/// \param surfIndex the surface index, which must correspond to a 32-bit
/// formatted 2D
/// surface.
///
/// \param sampIndex the sampler index.
///
/// \param u the normalized x co-ordinate of pixel 0.
///
/// \param v the normalized y co-ordinate of pixel 0.
///
/// \param execMode the execution mode. There are four modes, 64x4 (CM_ED_64x4,
/// the default),
/// 32x4 (CM_ED_32x4), 64x1 (CM_ED_64x1) and 32x1 (CM_ED_32x1). It is the
/// responsibility of
/// the user to ensure that the return value ::vect is the correct size for
/// mode. ::execMode
/// must be a compile-time constant.
///
template <int N>
void cm_va_dilate(vector_ref<uint, N> vect, SurfaceIndex surfIndex,
                  SamplerIndex sampIndex, float u, float v,
                  EDExecMode execMode = CM_ED_64x4);

/// MinMax
///
/// \param vect the vector to store the return result. Pixel 0 max and min are
/// stored
/// in ::vect[0] and ::vect[1] respectively. The type of the vector needs to
/// correspond
/// to the input format, vector<uchar, 32> for 8-bit, vector<ushort, 16> for
/// 16-bit.
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// that has
/// either an 8- or 16-bit format.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param mmfMode enable MinMax functionality. Valid values are
/// CM_MINMAX_ENABLE (default),
/// CM_MAX_ENABLE, or CM_MIN_ENABLE.
///
template <typename T, int N>
void cm_va_min_max(vector_ref<T, N> vect, SurfaceIndex surfIndex, float u,
                   float v, MMFEnableMode mmfMode = CM_MINMAX_ENABLE);

/// MinMax Filter
///
/// \param m the matrix to store the return result. The number of pixels per row
/// depends
/// on the surface format. It is the responsibility of the user to make sure a
/// matrix
/// of the correct size is passed in.
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// that has
/// either an 8- or 16-bit format.
///
/// \param sampIndex the sampler index.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param cntrl specifies the output format, must be a compile-time constant.
/// Valid values
/// are CM_16_FULL (default) or CM_8_FULL.
///
/// \param execMode specifies the execution mode, must be a compile-time
/// constant. Value
/// values are CM_MMF_16x4 (default), CM_MMF_16x1, or CM_MMF_1x1.
///
/// \param mmfMode enable MinMax functionality. Valid values are
/// CM_MINMAX_ENABLE (default),
/// CM_MAX_ENABLE or CM_MIN_ENABLE.
///
template <typename T, int N, int M>
void cm_va_min_max_filter(matrix_ref<T, N, M> m, SurfaceIndex surfIndex,
                          SamplerIndex sampIndex, float u, float v,
                          OutputFormatControl cntrl,
                          MMFExecMode execMode = CM_MMF_16x4,
                          MMFEnableMode mmfMode = CM_MINMAX_ENABLE);

/// Boolean Centroid
///
/// \param m the matrix to store the return result.
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
///
/// \param u the normalized x-coordinate of pixel 0
///
/// \param v the normalized y-coordinate of pixel 0
///
/// \param vSize the vertical direction size control
///
/// \param hSize the horizontal direction size control
///
template <typename T, int N, int M>
void cm_va_boolean_centroid(matrix_ref<T, N, M> m, SurfaceIndex surfIndex,
                            float u, float v, uchar vSize, uchar hSize);

/// Centroid
///
/// \param m the matrix to store the return result.
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
///
/// \param u the normalized x-coordinate of pixel 0
///
/// \param v the normalized y-coordinate of pixel 0
///
/// \param vSize the vertical direction size control
///
template <typename T, int N, int M>
void cm_va_centroid(matrix_ref<T, N, M> m, SurfaceIndex surfIndex, float u,
                    float v, uchar vSize);

/// 1d Convolve
///
/// \param m the matrix to store the return result, either matrix<short, 1, 16>
/// for 16x1 mode or matrix<short, 4, 16> for 16x4 mode.
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// that
/// has either 8- or 16-bit format.
///
/// \param sampIndex the sampler index.
///
/// \param isHdirection the convolve direction. TRUE for horizontal, FALSE for
/// vertical. Must be a compile-time constant.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param execMode the execution mode, either CM_CONV_16x4 (the default) or
/// CM_CONV_16x1.
template <int N, int M>
void cm_va_1d_convolution(matrix_ref<short, N, M> m, SurfaceIndex surfIndex,
                          SamplerIndex sampIndex, bool isHdirection, float u,
                          float v, CONVExecMode execMode = CM_CONV_16x4);

/// 1 Pixel Convolve
///
/// \param m the matrix to store the return result. This must be matrix<short,
/// 4, 16> for
/// 16x4 mode, matrix<short, 1, 16> for 16x1 mode or matrix<short, 1, 1> for 1x1
/// mode.
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// with
/// either 8- or 16-bit format.
///
/// \param sampIndex the sampler index.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param exexMode the execution mode. Must be one of CM_CONV_16x4 (16x4 mode),
/// CM_CONV_16x1
/// (16x1 mode) or CM_CONV_1x1 (1x1 mode). This must be a compile-time constant.
///
/// \param offsets the pixel offsets from the u,v address. A row contains 15
/// pairs of x,y.
/// This is not applicable for 1x1 mode, in which case the parameter should be
/// omitted.
template <int N, int M>
void cm_va_1pixel_convolve(matrix_ref<short, N, M> m, SurfaceIndex surfIndex,
                           SamplerIndex sampIndex, float u, float v,
                           CONVExecMode execMode);

template <int N, int M, int ON, int OM>
void cm_va_1pixel_convolve(matrix_ref<short, N, M> m, SurfaceIndex surfIndex,
                           SamplerIndex sampIndex, float u, float v,
                           CONVExecMode execMode,
                           matrix<short, ON, OM> offsets);

/// LBP Creation
///
/// \param m the matrix to store the return result. This should have type
/// matrix<uchar, 4, 16> for only 5x5 or only 3x3 modes, and output is laid out
/// in
/// sequential pixel order. For both 5x5 and 3x3 mode combined, it should have
/// type
/// matrix<uchar, 8, 16>, with the first 4 rows corresponding to the 3x3
/// operation
/// and the final four rows corresponding to the 5x5 operation.
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// that
/// has either 8- or 16-bit format.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param execMode the execution mode. This must be a compile-time constant and
/// must be one of CM_LBP_CREATION_5x5, CM_LBP_CREATION_3x3 or
/// CM_LBP_CREATION_BOTH.
template <int N>
void cm_va_lbp_creation(matrix_ref<uchar, N, 16> m, SurfaceIndex surfIndex,
                        float u, float v, LBPCreationExecMode execMode);

/// LBP Correlation
///
/// \param m the matrix to store the return result. This should have type
/// matrix<uchar, 4, 16>.
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// with
/// an 8- or 16-bit format.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param x_offset_disparity 16-bit signed offset for right image with respect
/// to left image.
template <int N>
void cm_va_lbp_correlation(matrix_ref<uchar, N, 16> m, SurfaceIndex surfIndex,
                           float u, float v, short x_offset_disparity);

/// Correlation Search
///
/// \param m the matrix to store the return result. Has type matrix<int, N, 8>
/// for
/// width less than 8, type matrix<int, N, 16> for width greater than 8. Data
/// is laid out in sequential order. The number of rows depends on the height
/// of the search region. Only valid rows are returned.
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// with
/// an 8- or 16-bit format/
///
/// \param u normalized x co-ordinate of pixel 0.
///
/// \param v normalized y co-ordinate of pixel 0.
///
/// \param verticalOrigin normalized vertical origin of the reference image.
///
/// \param horizontalOrigin normalized horizontal origin of the reference image.
///
/// \param xDirectionSize x direction size of the source correlation. Valid
/// range [1,8]
///
/// \param yDirectionSize y direction size of the source correlation. Valid
/// range [1,8]
///
/// \param xDirectionSearchSize x direction search size. Valid range [3,16]
///
/// \param yDirectionSearchSize y direction search size. Valid range [3,16]
template <int N, int M>
void cm_va_correlation_search(matrix_ref<int, N, M> m, SurfaceIndex surfIndex,
                              float u, float v, float verticalOrigin,
                              float horizontalOrigin, uchar xDirectionSize,
                              uchar yDirectionSize, uchar xDirectionSearchSize,
                              uchar yDirectionSearchSize);

/// Flood Fill
///
/// \param v vector to store the return result, output format is 1bpp.
///
/// \param Is8Connect TRUE 8-connect is used for floodfill, FALSE 4-connect is
/// used.
///
/// \param pixelMaskHDirection Pixel mask for the bottom adjacent pixels in the
/// horizontal direction. Index [0,7] are for rows 0 to 7, index 8 is for row
/// -1,
/// index 9 is for row 8.
///
/// \param pixelMaskVDirectionLeft Pixel mask of the adjacent pixels in vertical
/// direction. Valid values are [0,1023].
///
/// \param pixelMaskVDirectionRight Pixel mask of the adjacent pixels in
/// vertical
/// direction. Valid values are [0,1023].
///
/// \param loopCount Number of times FloodFill will be repeated on HW. Valid
/// range
/// is [1,16]
void cm_va_flood_fill(vector_ref<ushort, 8> v, bool Is8Connect,
                      vector<ushort, 10> pixelMaskHDirection,
                      ushort pixelMaskVDirectionLeft,
                      ushort pixelMaskVDirectionRight, ushort loopCount);

/// HDC 2D Convolve
///
/// \param surfIndex surface index, which must correspond to a 2D surface with
/// either
/// 8- or 16-bit format.
///
/// \param sampIndex sampler index.
///
/// \param u normalized x coordinate of pixel 0.
///
/// \param v normalized y coordinate of pixel 0.
///
/// \param big_kernel set to true when the kernel is larger than 15x15, must be
/// a
/// compile-time constant.
///
/// \param size specifies the format of the output surface, must be a
/// compile-time
/// constant. Valid values are CM_HDC_FORMAT_16S or CM_HDC_FORMAT_8U.
///
/// \param destIndex the index of the 2D surface where data will be written.
///
/// \param x_offset a byte offset into the output surface.
///
/// \param y_offset a row offset into the output surface.
///
/// 16x4 pixels will be written to the output surface. Each pixel is clamped to
/// unsigned char for 8-bit output surfaces.
void cm_va_2d_convolve_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
                           float u, float v, bool big_kernel,
                           CM_FORMAT_SIZE size, SurfaceIndex destIndex,
                           ushort x_offset, ushort y_offset);

/// HDC Erode
///
/// \param surfIndex the surface index, which must correspond to a 32-bit
/// formatted 2D
/// surface.
///
/// \param sampIndex the sampler index.
///
/// \param u the normalized x co-ordinate of pixel 0.
///
/// \param v the normalized y co-ordinate of pixel 0.
///
/// \param destIndex the surface index of the 2D surface where the data will be
/// written.
///
/// \param x_offset a byte offset into the output surface.
///
/// \param y_offset a row offset into the output surface.
void cm_va_erode_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex, float u,
                     float v, SurfaceIndex destIndex, ushort x_offset,
                     ushort y_offset);

/// HDC Dilate
///
/// \param surfIndex the surface index, which must correspond to a 32-bit
/// formatted 2D
/// surface.
///
/// \param sampIndex the sampler index.
///
/// \param u the normalized x co-ordinate of pixel 0.
///
/// \param v the normalized y co-ordinate of pixel 0.
///
/// \param destIndex the surface index of the 2D surface where the data will be
/// written.
///
/// \param x_offset a byte offset into the output surface.
///
/// \param y_offset a row offset into the output surface.
///
void cm_va_dilate_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex, float u,
                      float v, SurfaceIndex destIndex, ushort x_offset,
                      ushort y_offset);

/// HDC MinMax Filter
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// that has
/// either an 8- or 16-bit format.
///
/// \param sampIndex the sampler index.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param mmfMode enable MinMax functionality. Valid values are CM_MAX_ENABLE
/// or CM_MIN_ENABLE.
///
/// \param size the format used for the output. Supported values are
/// CM_HDC_FORMAT_16S and CM_HDC_FORMAT_8U.
///
/// \param destIndex surface index of the 2D surface the data will be written
/// to.
///
/// \param x_offset a byte offset into the output surface.
///
/// \param y_offset a row offset into the output surface.
///
void cm_va_min_max_filter_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
                              float u, float v, MMFEnableMode mmfMode,
                              CM_FORMAT_SIZE size, SurfaceIndex destIndex,
                              ushort x_offset, ushort y_offset);

/// HDC LBP Correlation
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// with
/// an 8- or 16-bit format.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param x_offset_disparity 16-bit signed offset for right image with respect
/// to left image.
///
/// \param destIndex surface index of the 2D surface the data will be written
/// to.
///
/// \param x_offset a byte offset into the output surface.
///
/// \param y_offset a row offset into the output surface.
///
void cm_va_lbp_correlation_hdc(SurfaceIndex surfIndex, float u, float v,
                               short x_offset_disparity, SurfaceIndex destIndex,
                               ushort x_offset, ushort y_offset);

/// HDC LBP Creation
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// that
/// has either 8- or 16-bit format.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param execMode the execution mode. This must be a compile-time constant and
/// must be either CM_LBP_CREATION_5x5 or CM_LBP_CREATION_3x3.
///
/// \param destIndex surface index of the 2D surface the data will be written
/// to.
///
/// \param x_offset a byte offset into the output surface.
///
/// \param y_offset a row offset into the output surface.
///
void cm_va_lbp_creation_hdc(SurfaceIndex surfIndex, float u, float v,
                            LBPCreationExecMode execMode,
                            SurfaceIndex destIndex, ushort x_offset,
                            ushort y_offset);

/// HDC 1 Pixel Convolve
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// with
/// either 8- or 16-bit format.
///
/// \param sampIndex the sampler index.
///
/// \param offsets the offsets of pixels from the U,V address.
///
/// \param size specifies the format used for the output. Two formats are
/// supported, CM_HDC_FORMAT_16S or CM_HDC_FORMAT_8U.
///
/// \param destIndex surface index of the 2D surface the data will be written
/// to.
///
/// \param x_offset a byte offset into the output surface.
///
/// \param y_offset a row offset into the output surface.
///
void cm_va_1pixel_convolve_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
                               float u, float v, matrix<short, 1, 32> offsets,
                               CM_FORMAT_SIZE size, SurfaceIndex destIndex,
                               ushort x_offset, ushort y_offset);

/// HDC 1d Convolve
///
/// \param surfIndex the surface index, which must correspond to a 2D surface
/// that
/// has either 8- or 16-bit format.
///
/// \param sampIndex the sampler index.
///
/// \param isHdirection the convolve direction. TRUE for horizontal, FALSE for
/// vertical. Must be a compile-time constant.
///
/// \param u the normalized x-coordinate of pixel 0.
///
/// \param v the normalized y-coordinate of pixel 0.
///
/// \param size the format used for the output. Supported values are
/// CM_HDC_FORMAT_16S and CM_HDC_FORMAT_8U.
///
/// \param destIndex surface index of the 2D surface the data will be written
/// to.
///
/// \param x_offset a byte offset into the output surface.
///
/// \param y_offset a row offset into the output surface.
///
void cm_va_1d_convolution_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
                              bool isHdirection, float u, float v,
                              CM_FORMAT_SIZE size, SurfaceIndex destIndex,
                              ushort x_offset, ushort y_offset);

#endif // _CLANG_CM_SAMPLER_H_
