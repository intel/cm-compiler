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
static_assert(0, "CM:w:cm_dataport.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_DATAPORT_H_
#define _CLANG_CM_DATAPORT_H_

#include "cm_util.h"
#include "cm_internal.h"

typedef enum _CmFenceMask_ {
  CM_GLOBAL_COHERENT_FENCE = 0x1,
  CM_L3_FLUSH_INSTRUCTIONS = 0x2,
  CM_L3_FLUSH_TEXTURE_DATA = 0x4,
  CM_L3_FLUSH_CONSTANT_DATA = 0x8,
  CM_L3_FLUSH_RW_DATA = 0x10,
  CM_LOCAL_BARRIER = 0x20,
  CM_L1_FLUASH_RO_DATA = 0x40,
  CM_SW_BARRIER = 0x80
} CmFenceMask;

#define MODIFIED(A) A, GENX_MODIFIED
#define TOP_FIELD(A) A, GENX_TOP_FIELD
#define BOTTOM_FIELD(A) A, GENX_BOTTOM_FIELD
#define MODIFIED_TOP_FIELD(A) A, GENX_TOP_FIELD
#define MODIFIED_BOTTOM_FIELD(A) A, GENX_BOTTOM_FIELD
#define DWALIGNED(A) A, GENX_DWALIGNED
#define MODIFIED_DWALIGNED(A) A, GENX_MODIFIED_DWALIGNED
#define CONSTANT(A) A, GENX_CONSTANT
#define CONSTANT_DWALIGNED(A) A, GENX_CONSTANT_DWALIGNED

/// \brief OWord block read.
///
/// @param idx Surface index, which must correspond to a buffer.
///
/// @param attr indicates the offset alignment and properties, one of ::GENX_NONE
/// ::GENX_DWALIGNED ::GENX_MODIFIED_DWALIGNED ::GENX_CONSTANT_DWALIGNED.
///
/// @param offset zero based offset of the input buffer in bytes. Must be oword
/// (i.e. 16 bytes) aligned unless one of the DWALIGNED attributes is present.
///
/// @param output the data location to be read.
///
template <typename T, int N>
CM_NODEBUG CM_INLINE void read(SurfaceIndex idx, CmBufferAttrib attr,
                               int offset, vector_ref<T, N> output) {
  constexpr unsigned _Sz = sizeof(T) * N;

  if constexpr(_Sz < details::OWORD) {
    constexpr unsigned _N = details::OWORD / sizeof(T);

    // Choose implementation based on the buffer attribute. Since this attribute
    // is not a compilation time constant, we cannot statically verify the
    // requirement that attribute is either GENX_NONE or GENX_DWALIGNED. Leave
    // this check to the emulation mode.
    if (attr == GENX_DWALIGNED || attr == GENX_MODIFIED_DWALIGNED
        || attr == GENX_CONSTANT_DWALIGNED) {
      vector<T, _N> _Output =
          details::__cm_intrinsic_impl_oword_read_dwaligned<T, _N>(idx, offset);
      return details::if_assign(output, _Output);
    }

    // By default, no attribute.
    vector<T, _N> _Output =
        details::__cm_intrinsic_impl_oword_read<T, _N>(idx, offset);
    return details::if_assign(output, _Output);
  }

  // Check the data size.
  CM_STATIC_ERROR(details::getNextPowerOf2(_Sz) <= 8 * details::OWORD,
                  "OWord block size must be at most 8");

  if constexpr(!details::isPowerOf2(_Sz)) {
    constexpr unsigned _N = details::getNextPowerOf2(_Sz) / sizeof(T);

    if (attr == GENX_DWALIGNED || attr == GENX_MODIFIED_DWALIGNED
        || attr == GENX_CONSTANT_DWALIGNED) {
      vector<T, _N> _Output =
          details::__cm_intrinsic_impl_oword_read_dwaligned<T, _N>(idx, offset);
      return details::if_assign(output, _Output);
    }

    // By default, no attribute.
    vector<T, _N> _Output =
        details::__cm_intrinsic_impl_oword_read<T, _N>(idx, offset);
    return details::if_assign(output, _Output);
  }

  // No padding case.
  if (attr == GENX_DWALIGNED || attr == GENX_MODIFIED_DWALIGNED
      || attr == GENX_CONSTANT_DWALIGNED)
    output =
        details::__cm_intrinsic_impl_oword_read_dwaligned<T, N>(idx, offset);
  else
    output = details::__cm_intrinsic_impl_oword_read<T, N>(idx, offset);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE void read(SurfaceIndex index, int X,
                               vector_ref<T, N> src) {
  read(index, GENX_NONE, X, src);
}

/// \brief OWord block write.
/// @param idx surface index, which must correspond to a buffer.
///
/// @param offset zero based offset of the input buffer in bytes.
///
/// @param src the data to be written. The size of vector can be only 1, 2, 4 or
/// 8 owords.
template <typename T, int N>
CM_NODEBUG CM_INLINE void write(SurfaceIndex index, int offset,
                                vector<T, N> src) {
  constexpr unsigned Sz = N * sizeof(T);
  CM_STATIC_ERROR(details::isPowerOf2(Sz) && Sz >= details::OWORD &&
                      Sz <= 8 * details::OWORD,
                  "vector must be 1/2/4/8 OWords");
  details::__cm_intrinsic_impl_oword_write(index, offset, src);
}

/// \brief HWord block read.
/// @param idx surface index, which must correspond to a buffer.
///
/// @param offset zero based offset of the input buffer in bytes. Must be HWord aligned.
///
/// @param dst the data to be read. The size of vector can be only 1, 2, 4 or
/// 8 HWords.
template <int N>
CM_NODEBUG CM_INLINE void cm_hword_read(SurfaceIndex index, uint offset,
  vector_ref<uint, N> dst) {
  vector<uint, 8> header = 0;
  const uint exDesc = 0xa;
  uint desc = cm_get_value(index);

  constexpr unsigned Sz = sizeof(uint) * N;
  CM_STATIC_ERROR(details::isPowerOf2(N) && Sz >= details::GRF &&
    Sz <= 8 * details::GRF,
    "Data size must be 1/2/4/8 HWords");

  switch (N)
  {
  case 8:
    desc |= (0x0 << 8);  // MESSAGE_SPECIFIC_CONTROL
    break;
  case 16:
    desc |= (0x1 << 8);
    break;
  case 32:
    desc |= (0x2 << 8);
    break;
  case 64:
    desc |= (0x3 << 8);
    break;
  }

  desc += 0x1 << 13;
  desc += 0x1 << 14;
  desc += 0x1 << 19;
  desc += (N / 8) << 20;   // Response length
  desc += 0x1 << 25;  // Msg length
  header(2) = offset;

  cm_send(dst.format<uint, 1, N>(), header.format<uint, 1, 8>(),
    exDesc, desc, 0u);
}

/// \brief HWord block write.
/// @param idx surface index, which must correspond to a buffer.
///
/// @param offset zero based offset of the input buffer in bytes. Must be HWord aligned.
///
/// @param src the data to be written. The size of vector can be only 1, 2, 4 or
/// 8 HWords.
template <int N>
CM_NODEBUG CM_INLINE void cm_hword_write(SurfaceIndex index, int offset,
  vector<uint, N> src) {
  vector<uint, 8> header = 0;
  const uint exDesc = ((N / 8) << 6) + 0xa;
  uint desc = cm_get_value(index);

  constexpr unsigned Sz = sizeof(uint) * N;
  CM_STATIC_ERROR(details::isPowerOf2(Sz) && Sz >= details::GRF &&
    Sz <= 8 * details::GRF,
    "Data size must be 1/2/4/8 HWords");

  switch (N)
  {
  case 8:
    desc |= (0x0 << 8);  // MESSAGE_SPECIFIC_CONTROL
    break;
  case 16:
    desc |= (0x1 << 8);
    break;
  case 32:
    desc |= (0x2 << 8);
    break;
  case 64:
    desc |= (0x3 << 8);
    break;
  }

  desc += 0x1 << 13;
  desc += 0x9 << 14;
  desc += 0x1 << 19;
  desc += 0x1 << 25;  // Msg length
  header(2) = offset;

  cm_sends(NULL, header.format<uint, 1, 8>(), src.format<uint, 1, N>(),
    exDesc, desc, 0u);
}

/// \brief Media block read.
///
/// @param idx surface index, which must correspond to a 2D type surface.
///
/// @param attr attribute applied to the surface. Only ::GENX_TOP_FIELD,
/// ::GENX_BOTTOM_FIELD and ::GENX_NONE are supported. ::GENX_TOP_FIELD
/// indicates only the top field surface data are needed. ::BOTTOM_FIELD
/// indicates only the bottom field surface data are needed.
///
/// @param X zero based X-coordinate of the left upper rectangle corner in
/// bytes. For regular surfaces, the X-offset must be pixel aligned. For
/// surfaces with compact format (e.g., YUY2), this must be DWord (i.e. 4
/// bytes) aligned.
///
/// @param Y zero based Y-coordinate of the left upper rectangle corner in rows.
///
/// @param output the data location to be read. the width of matrix m must be
/// pixel aligned.
///
/// For out-of-bounds read, the address is clamped to the nearest edge of the
/// surface, and the pixel in that position is returned. Out-of-bound behavior
/// is undefined, however, if the surface width is not DWord-aligned.
///

#define MEDIA_READ_SWITCH(_Output, attr, _M)                                   \
  switch (attr) {                                                              \
  default:                                                                     \
    break;                                                                     \
  case GENX_NONE:                                                              \
    _Output = details::__cm_intrinsic_impl_media_read<T, N, M, _M, GENX_NONE>( \
        idx, X, Y);                                                            \
    break;                                                                     \
  case GENX_TOP_FIELD:                                                         \
    _Output =                                                                  \
        details::__cm_intrinsic_impl_media_read<T, N, M, _M, GENX_TOP_FIELD>(  \
            idx, X, Y);                                                        \
    break;                                                                     \
  case GENX_BOTTOM_FIELD:                                                      \
    _Output = details::__cm_intrinsic_impl_media_read<                         \
        T, N, M, _M, GENX_BOTTOM_FIELD>(idx, X, Y);                            \
    break;                                                                     \
  case GENX_MODIFIED:                                                          \
    _Output =                                                                  \
        details::__cm_intrinsic_impl_media_read<T, N, M, _M, GENX_MODIFIED>(   \
            idx, X, Y);                                                        \
    break;                                                                     \
  }

template <typename T, int N, int M>
CM_NODEBUG CM_INLINE void read(SurfaceIndex idx, CmBufferAttrib attr, int X,
                               int Y, matrix_ref<T, N, M> output) {

  constexpr unsigned Width = M * sizeof(T);
  constexpr unsigned RoundedWidth =
      Width < 4 ? 4 : details::getNextPowerOf2(Width);
  // Check the data sizes.
  CM_STATIC_ERROR(RoundedWidth * N <= 256u,
                  "data does not fit into a single dataport transaction");
  CM_STATIC_ERROR(RoundedWidth <= 64u, "valid block width is in range [1, 64]");
  CM_STATIC_ERROR(N <= 64u, "valid block height is in range [1, 64]");
  if (Width != RoundedWidth) {
    constexpr unsigned _M = RoundedWidth / sizeof(T);
    matrix<T, N, _M> _Output;
    MEDIA_READ_SWITCH(_Output, attr, _M);
    return details::if_assign(output, _Output.select<N, 1, M, 1>(0, 0));
  }

  // No padding.
  MEDIA_READ_SWITCH(output, attr, M);
}

#undef MEDIA_READ_SWITCH

template <typename T, int N, int M>
CM_NODEBUG CM_INLINE void read(SurfaceIndex idx, int X, int Y,
                               matrix_ref<T, N, M> output) {
  read(idx, GENX_NONE, X, Y, output);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE void read(SurfaceIndex idx, int X, int Y,
                               vector_ref<T, N> output) {
  matrix_ref<T, 1, N> _Output = output.format<T, 1, N>();
  read(idx, X, Y, _Output);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE void read(SurfaceIndex idx, CmBufferAttrib attr, int X,
                               int Y, vector_ref<T, N> output) {
  matrix_ref<T, 1, N> _Output = output.format<T, 1, N>();
  read(idx, attr, X, Y, _Output);
}

/// \brief Media block write
///
/// @param idx surface index, which must correspond to a 2D type buffer.
///
/// @param attribute applied to the surface. Only ::GENX_TOP_FIELD,
/// ::GENX_BOTTOM_FIELD and ::GENX_NONE are supported. ::GENX_TOP_FIELD
/// indicates only the top field surface data are needed. ::BOTTOM_FIELD
/// indicates only the bottom field surface data are needed.
///
/// @param X zero based X-coordinate of the left upper rectangle corner in
/// bytes. For regular surfaces, the X-offset must be pixel aligned. For
/// surfaces with compact format (e.g., YUY2), this must be DWord (i.e. 4
/// bytes) aligned.
///
/// @param Y zero based Y-coordinate of the left upper rectangle corner in rows.
///
/// @param output the data to be written. The width of matrix must be DWord
/// aligned.
#define MEDIA_WRITE(attr)                                                      \
  details::__cm_intrinsic_impl_media_write<T, N, M, _M, attr>(idx, X, Y, _Src)

template <typename T, int N, int M>
CM_NODEBUG CM_INLINE void write(SurfaceIndex idx, CmBufferAttrib attr, int X,
                                int Y, matrix<T, N, M> src) {
  constexpr unsigned Width = M * sizeof(T);
  constexpr unsigned RoundedWidth =
      Width < 4 ? 4 : details::getNextPowerOf2(Width);

  // Check the data sizes.
  CM_STATIC_ERROR(RoundedWidth * N <= 256u,
                  "data does not fit into a single dataport transaction");
  CM_STATIC_ERROR(RoundedWidth <= 64u, "valid block width is in range [1, 64]");
  CM_STATIC_ERROR(N <= 64u, "valid block height is in range [1, 64]");

  // Fix the register pitch alignment.
  constexpr unsigned _M = RoundedWidth / sizeof(T);
  matrix<T, N, _M> _Src;
  _Src.select<N, 1, M, 1>() = src;

  // Choose implementation based on the buffer attribute. Note that the real
  // width will be also passed in due to pitch alignment fixing.
  switch (attr) {
  default:
    break;
  case GENX_NONE:
    MEDIA_WRITE(GENX_NONE);
    break;
  case GENX_TOP_FIELD:
    MEDIA_WRITE(GENX_TOP_FIELD);
    break;
  case GENX_BOTTOM_FIELD:
    MEDIA_WRITE(GENX_BOTTOM_FIELD);
    break;
  }
}
#undef MEDIA_WRITE

template <typename T, int N, int M>
CM_NODEBUG CM_INLINE void write(SurfaceIndex idx, int X, int Y,
                                matrix<T, N, M> src) {
  write(idx, GENX_NONE, X, Y, src);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE void write(SurfaceIndex index, int X, int Y,
                                vector<T, N> src) {
  matrix<T, 1, N> _Src = src;
  write(index, GENX_NONE, X, Y, _Src);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE void write(SurfaceIndex index, CmBufferAttrib attr, int X,
                                int Y, vector<T, N> src) {
  matrix<T, 1, N> _Src = src;
  write(index, attr, X, Y, _Src);
}

template <typename T, int N, int M, int _M>
CM_NODEBUG CM_INLINE void
media_block_read_plane(SurfaceIndex idx, CmBufferAttrib attr,
                       CmSurfacePlaneIndex plane, int X, int Y,
                       matrix_ref<T, N, _M> _Output) {
  if (plane == GENX_SURFACE_Y_PLANE) {
    switch (attr) {
    default:
      break;
    case GENX_NONE:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_NONE, GENX_SURFACE_Y_PLANE>(idx, X, Y);
      break;
    case GENX_TOP_FIELD:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_TOP_FIELD, GENX_SURFACE_Y_PLANE>(idx, X, Y);
      break;
    case GENX_BOTTOM_FIELD:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_BOTTOM_FIELD, GENX_SURFACE_Y_PLANE>(idx, X, Y);
      break;
    case GENX_MODIFIED:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_MODIFIED, GENX_SURFACE_Y_PLANE>(idx, X, Y);
      break;
    } // switch
  } else if ((plane == GENX_SURFACE_U_PLANE) ||
             (plane == GENX_SURFACE_UV_PLANE)) {
    switch (attr) {
    default:
      break;
    case GENX_NONE:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_NONE, GENX_SURFACE_U_PLANE>(idx, X, Y);
      break;
    case GENX_TOP_FIELD:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_TOP_FIELD, GENX_SURFACE_U_PLANE>(idx, X, Y);
      break;
    case GENX_BOTTOM_FIELD:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_BOTTOM_FIELD, GENX_SURFACE_U_PLANE>(idx, X, Y);
      break;
    case GENX_MODIFIED:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_MODIFIED, GENX_SURFACE_U_PLANE>(idx, X, Y);
      break;
    } // switch
  } else if (plane == GENX_SURFACE_V_PLANE) {
    switch (attr) {
    default:
      break;
    case GENX_NONE:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_NONE, GENX_SURFACE_V_PLANE>(idx, X, Y);
      break;
    case GENX_TOP_FIELD:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_TOP_FIELD, GENX_SURFACE_V_PLANE>(idx, X, Y);
      break;
    case GENX_BOTTOM_FIELD:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_BOTTOM_FIELD, GENX_SURFACE_V_PLANE>(idx, X, Y);
      break;
    case GENX_MODIFIED:
      _Output = details::__cm_intrinsic_impl_read_plane<
          T, N, _M, GENX_MODIFIED, GENX_SURFACE_V_PLANE>(idx, X, Y);
      break;
    } // switch
  }   // if
}

template <typename T, int N, int M>
CM_NODEBUG CM_INLINE void read_plane(SurfaceIndex index, CmBufferAttrib attr,
                                     CmSurfacePlaneIndex plane, int X, int Y,
                                     matrix_ref<T, N, M> output) {
  constexpr unsigned Width = M * sizeof(T);
  if (Width < details::DWORD) {
    constexpr unsigned _M = details::DWORD / sizeof(T);
    matrix<T, N, _M> _Output;
    media_block_read_plane<T, N, M, _M>(index, attr, plane, X, Y, _Output);
    return details::if_assign(output, _Output);
  }
  if (!details::isPowerOf2(Width)) {
    constexpr unsigned _M = details::getNextPowerOf2(Width) / sizeof(T);
    matrix<T, N, _M> _Output;
    media_block_read_plane<T, N, M, _M>(index, attr, plane, X, Y, _Output);
    return details::if_assign(output, _Output);
  }

  // No padding.
  media_block_read_plane<T, N, M, M>(index, attr, plane, X, Y, output);
};

template <typename T, int N, int M>
CM_NODEBUG CM_INLINE void read_plane(SurfaceIndex index,
                                     CmSurfacePlaneIndex plane, int X, int Y,
                                     matrix_ref<T, N, M> src) {
  read_plane(index, GENX_NONE, plane, X, Y, src);
};

#define MEDIA_WRITE_PLANE(attr, plane)                                         \
  details::__cm_intrinsic_impl_write_plane<T, N, M, _M, attr, plane>(idx, X,   \
                                                                     Y, _Src)

template <typename T, int N>
CM_NODEBUG CM_INLINE void read_plane(SurfaceIndex idx,
                                     CmSurfacePlaneIndex plane, int X, int Y,
                                     vector_ref<T, N> output) {
  matrix_ref<T, 1, N> _Output = output.format<T, 1, N>();
  read_plane(idx, GENX_NONE, plane, X, Y, _Output);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE void read_plane(SurfaceIndex idx, CmBufferAttrib attr,
                                     CmSurfacePlaneIndex plane, int X, int Y,
                                     vector_ref<T, N> output) {
  matrix_ref<T, 1, N> _Output = output.format<T, 1, N>();
  read_plane(idx, attr, plane, X, Y, _Output);
};

#define MEDIA_WRITE_PLANE(attr, plane)                                         \
  details::__cm_intrinsic_impl_write_plane<T, N, M, _M, attr, plane>(idx, X,   \
                                                                     Y, _Src)

template <typename T, int N, int M>
CM_NODEBUG CM_INLINE void write_plane(SurfaceIndex idx, CmBufferAttrib attr,
                                      CmSurfacePlaneIndex plane, int X, int Y,
                                      matrix<T, N, M> src) {
  constexpr unsigned Width = M * sizeof(T);
  // Check the data alignment.
  CM_STATIC_ERROR(Width % details::DWORD == 0,
                  "matrix block width must be DWord aligned");
  constexpr unsigned _M = details::getNextPowerOf2(Width) / sizeof(T);

  // Check the data sizes.
  CM_STATIC_ERROR(_M * N * sizeof(T) <= 256u,
                  "data does not fit into a single dataport transaction");
  CM_STATIC_ERROR(_M <= 64u, "valid block width is in range [1, 64]");
  CM_STATIC_ERROR(N <= 64u, "valid block height is in range [1, 64]");

  // Fix the register pitch alignment.
  matrix<T, N, _M> _Src;
  _Src.select<N, 1, M, 1>() = src;

  // Choose implementation based on the buffer attribute. Note that the real
  // width will be also passed in due to pitch alignment fixing.
  if (plane == GENX_SURFACE_Y_PLANE) {
    switch (attr) {
    default:
      break;
    case GENX_NONE:
      MEDIA_WRITE_PLANE(GENX_NONE, GENX_SURFACE_Y_PLANE);
      break;
    case GENX_TOP_FIELD:
      MEDIA_WRITE_PLANE(GENX_TOP_FIELD, GENX_SURFACE_Y_PLANE);
      break;
    case GENX_BOTTOM_FIELD:
      MEDIA_WRITE_PLANE(GENX_BOTTOM_FIELD, GENX_SURFACE_Y_PLANE);
      break;
    } // switch
  } else if ((plane == GENX_SURFACE_U_PLANE) ||
             (plane == GENX_SURFACE_UV_PLANE)) {
    switch (attr) {
    default:
      break;
    case GENX_NONE:
      MEDIA_WRITE_PLANE(GENX_NONE, GENX_SURFACE_U_PLANE);
      break;
    case GENX_TOP_FIELD:
      MEDIA_WRITE_PLANE(GENX_TOP_FIELD, GENX_SURFACE_U_PLANE);
      break;
    case GENX_BOTTOM_FIELD:
      MEDIA_WRITE_PLANE(GENX_BOTTOM_FIELD, GENX_SURFACE_U_PLANE);
      break;
    } // switch
  } else if (plane == GENX_SURFACE_V_PLANE) {
    switch (attr) {
    default:
      break;
    case GENX_NONE:
      MEDIA_WRITE_PLANE(GENX_NONE, GENX_SURFACE_V_PLANE);
      break;
    case GENX_TOP_FIELD:
      MEDIA_WRITE_PLANE(GENX_TOP_FIELD, GENX_SURFACE_V_PLANE);
      break;
    case GENX_BOTTOM_FIELD:
      MEDIA_WRITE_PLANE(GENX_BOTTOM_FIELD, GENX_SURFACE_V_PLANE);
      break;
    } // switch
  }   // if
};
#undef MEDIA_WRITE_PLANE

template <typename T, int N, int M>
CM_NODEBUG CM_INLINE void write_plane(SurfaceIndex index,
                                      CmSurfacePlaneIndex plane, int X, int Y,
                                      matrix<T, N, M> src) {
  write_plane(index, GENX_NONE, plane, X, Y, src);
};

template <typename T, int N>
CM_NODEBUG CM_INLINE void write_plane(SurfaceIndex index,
                                      CmSurfacePlaneIndex plane, int X, int Y,
                                      vector<T, N> src) {
  matrix<T, 1, N> _Src = src;
  write_plane(index, GENX_NONE, plane, X, Y, _Src);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE void write(SurfaceIndex index, CmBufferAttrib attr,
                                CmSurfacePlaneIndex plane, int X, int Y,
                                vector<T, N> src) {
  matrix<T, 1, N> _Src = src;
  write(index, attr, plane, X, Y, _Src);
}

/// \brief Scaled scattered read.
///
/// \param index surface index, which must correspond to a buffer.
///
/// \param globalByteOffest zero based global offset of a set of scattered
/// elements to be read from the surface. This offset is in bytes.
///
/// \param elementByteOffset zero based offset of each element (relative to the
/// global offset) to be read; ::N must be a power of 2 and at most 16. This
/// offset is in bytes.
///
/// \param ret the data location to store the return result.
///
/// ::T T can be either char, uchar, short, ushort, int, uint or float.
///
template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(sizeof(T) <= 4) && details::isPowerOf2(N, 16),
                        void>::type
read_scaled(SurfaceIndex index, uint globalByteOffest,
            vector<uint, N> elementByteOffset, vector_ref<T, N> ret) {
  if constexpr(sizeof(T) == 4)
    ret = details::__cm_intrinsic_impl_scatter_read(
        index, globalByteOffest, elementByteOffset, ret, T());
  else {
    typedef typename details::dword_type<T>::type T1;
    vector<T1, N> _Ret;
    // Data to read is a vector of dword type. The last dummy argument is
    // to specify the element size.
    ret = details::__cm_intrinsic_impl_scatter_read(
        index, globalByteOffest, elementByteOffset, _Ret, T());
  }
}

/// \brief DWord scattered read.
///
/// \param index surface index, which must correspond to a buffer.
///
/// \param globalOffset zero based global offset of a set of scattered elements
/// to be read from the surface. This offset is in units of elements.
///
/// \param elementOffset zero based offset of each element (relative to the
/// global offset) to be read; ::N must be a power of 2 and at most 16. This
/// offset is in units of elements.
///
/// \param ret the data location to store the return result.
///
/// ::T T can be either char, uchar, short, ushort, int, uint or float.
///
template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(sizeof(T) <= 4) && details::isPowerOf2(N, 16),
                        void>::type
read(SurfaceIndex index, uint globalOffset, vector<uint, N> elementOffset,
     vector_ref<T, N> ret) {
  globalOffset *= sizeof(T);
  elementOffset *= sizeof(T);
  read_scaled(index, globalOffset, elementOffset, ret);
}

// scattered read supports the CONSTANT and MODIFIED for surface attribute
template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(sizeof(T) <= 4) && details::isPowerOf2(N, 16),
                        void>::type
read(SurfaceIndex index, CmBufferAttrib attr, uint globalOffset,
     vector<uint, N> elementOffset, vector_ref<T, N> ret) {
  if (attr == GENX_CONSTANT || attr == GENX_MODIFIED) {
    read<T, N>(index, globalOffset, elementOffset, ret);
  }
}

/// \brief Scalar read.
///
/// \param index surface index, which must correspond to a buffer.
///
/// \param elementIndex zero based offset of the element.
///        This offset is in units of elements.
///
/// ::T T can be any type <= 4 bytes
///
template<typename T>
CM_NODEBUG CM_INLINE
typename std::enable_if<(sizeof(T) <= 4), T>::type
read(SurfaceIndex index, uint elementIndex) {
  vector<T, 1> ret;
  vector<uint, 1> offset = elementIndex * sizeof(T);

  if constexpr(sizeof(T) == 4)
    ret = details::__cm_intrinsic_impl_scatter_read(
        index, 0, offset, ret, T());
  else {
    typedef typename details::dword_type<T>::type T1;
    vector<T1, 1> _Ret;
    // Data to read is a vector of dword type. The last dummy argument is
    // to specify the element size.
    ret = details::__cm_intrinsic_impl_scatter_read(
        index, 0, offset, _Ret, T());
  }

  return ret(0);
}

/// \brief Scaled scattered write.
///
/// \param index surface index, which must correspond to a buffer.
///
/// \param globalByteOffset zero based global offset of a set of scattered
/// elements to be written to the surface. This offset is in units of bytes.
///
/// \param elementByteOffset zero based offset of each element (relative to the
/// global offset) to be written; ::N must be a power of 2 and at most 16. This
/// offset is in units of bytes.
///
/// \param data the data to be written.
///
/// ::T T can be either char, uchar, short, ushort, int, uint or float.
///
/// Note: for any src < 4 bytes there is a cost to mov the value into a 4 byte
/// type (of which upper bytes are ignored)
///
template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(sizeof(T) <= 4) && details::isPowerOf2(N, 16),
                        void>::type
write_scaled(SurfaceIndex index, uint globalByteOffset,
             vector<uint, N> elementByteOffset, vector<T, N> data) {
  if constexpr(sizeof(T) == 4)
    details::__cm_intrinsic_impl_scatter_write(index, globalByteOffset,
                                               elementByteOffset, data, T());
  else {
    typedef typename details::dword_type<T>::type T1;
    vector<T1, N> _Data = data;
    // Data to write is a vector of dword type. The last dummy argument is
    // to specify the element size.
    details::__cm_intrinsic_impl_scatter_write(index, globalByteOffset,
                                               elementByteOffset, _Data, T());
  }
}

/// \brief DWord scattered write.
///
/// \param index surface index, which must correspond to a buffer.
///
/// \param globalOffset zero based global offset of a set of scattered
/// elements to be written to the surface. This offset is in units of elements.
///
/// \param elementOffset zero based offset of each element (relative to the
/// global offset) to be written; ::N must be a power of 2 and at most 16. This
/// offset is in units of elements.
///
/// \param data the data to be written.
///
/// ::T T can be either char, uchar, short, ushort, int, uint or float.
///
/// Note: for any src < 4 bytes there is a cost to mov the value into a 4 byte
/// type (of which upper bytes are ignored)
///
template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(sizeof(T) <= 4) && details::isPowerOf2(N, 16),
                        void>::type
write(SurfaceIndex index, uint globalOffset, vector<uint, N> elementOffset,
      vector<T, N> data) {
  globalOffset *= sizeof(T);
  elementOffset *= sizeof(T);
  write_scaled(index, globalOffset, elementOffset, data);
}

/// \brief Scalar dword write.
///
/// \param index surface index, which must correspond to a buffer.
///
/// \param globaOffset zero based global offset of single element to be written
/// to the surface. This offset is in units of elements.
///
/// \param elementOffset zero based offset of the element (relative to the
/// global offset) to be written. This offset is in units of elements.
///
/// \param data the data to be written (in this case a scalar)
///
/// ::T T can be any type <= 4 bytes
///
/// Note: for any src < 4 bytes there is a cost to mov the value into a 4 byte
/// type (of which upper bytes are ignored)
///
template <typename T>
CM_NODEBUG CM_INLINE
typename std::enable_if<(sizeof(T) <= 4) && details::is_cm_scalar<T>::value,
                        void>::type
write(SurfaceIndex index, uint globalOffset, uint elementOffset, T data) {
  globalOffset *= sizeof(T);
  elementOffset *= sizeof(T);
  if constexpr(sizeof(T) == 4) {
    vector<uint, 1> elementOffset_vec = elementOffset;
    vector<T, 1> data_vec = data;
    details::__cm_intrinsic_impl_scatter_write(index, globalOffset,
                                               elementOffset_vec, data_vec, T());
  } else {
    typedef typename details::dword_type<T>::type T1;
    vector<T1, 1> _Data = data;
    // Data to write is a vector of dword type. The last dummy argument is
    // to specify the element size.
    vector<uint, 1> elementOffset_vec = elementOffset;
    details::__cm_intrinsic_impl_scatter_write(index, globalOffset,
                                               elementOffset_vec, _Data, T());
  }
}

template <typename T>
CM_NODEBUG CM_INLINE
typename std::enable_if<(sizeof(T) <= 4) && details::is_cm_scalar<T>::value,
                        void>::type
write(SurfaceIndex index, uint elementOffset, T data) {
  write<T>(index, 0, elementOffset, data);
}

/// \brief Typed surface read.
///
/// \param surfIndex surface index, which must correspond to a 1D, 2D or 3D
/// surface.
///
/// \param channelMask enabled channels. Must be a compile time constant.
///
/// \param m the matrix to store the return results. The type T must be of size
/// of DWord (i.e., int, uint, or float). The size N1 is at least the number of
/// enabled channels and N2 must be either 8 or 16.
///
/// \param u the x coordinates of the data elements to be read from surface,
/// which
/// must be in unit of pixels. The size N2 must be either 8 or 16.
///
/// \param v (optional, default = 0) the y coordinates of the data elements to
/// be
/// read from non-1D surface types; ignored otherwise.
///
/// \param r (optional, default = 0) the z coordinates of the data elements to
/// be read from 3D surface types; ignored otherwise.
///
/// The compiler generates code for GenX hardware to perform scattered read from
/// the given offsets. The results are returned in ::m with each enabled channel
/// returned in the next row of ::m. The enabled channels are returned in R, G,
/// B, A order with no gap in ::m for disabled channels.
///
template <typename T, int N1, int N2>
typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&(N2 == 8 || N2 == 16)>::type
read_typed(SurfaceIndex surfIndex, ChannelMaskType channelMask,
           matrix_ref<T, N1, N2> m, vector<uint, N2> u, vector<uint, N2> v = 0,
           vector<uint, N2> r = 0);

template <typename T, int N>
typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&(N == 8 || N == 16)>::type
read_typed(SurfaceIndex surfIndex, ChannelMaskType channelMask,
           vector_ref<T, N> res, vector<uint, N> u, vector<uint, N> v = 0,
           vector<uint, N> r = 0);

/// \brief Typed surface write.
///
/// \param surfIndex surface index, which must correspond to a 1D, 2D or 3D
/// surface.
///
/// \param channelMask enabled channels. Must be a compile time constant.
///
/// \param m the matrix that stores the data to be written. The type T must be
/// of size of DWord (i.e., int, uint, or float). The size N1 is at least the
/// number of enabled channels and N2 must be either 8 or 16.
///
/// \param u the x coordinates of the data elements to be read from surface,
/// which must be in unit of pixels. The size N2 must be either 8 or 16.
///
/// \param v (optional, default = 0) the y coordinates of the data elements to
/// be read from non-1D surface types; ignored otherwise.
///
/// \param r (optional, default = 0) the z coordinates of the data elements to
/// be read from 3D surface types; ignored otherwise.
///
/// The compiler generates code for GenX hardware to perform scattered write to
/// the given offsets. Only enabled channels are written to the surface.
/// Out-of-bound reads return zero, while out-of-bound writes are dropped.
///
template <typename T, int N1, int N2>
typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&(N2 == 8 || N2 == 16)>::type
write_typed(SurfaceIndex surfIndex, ChannelMaskType channelMask,
            matrix<T, N1, N2> m, vector<uint, N2> u, vector<uint, N2> v = 0,
            vector<uint, N2> r = 0);

/// \brief Untyped surface read.
///
/// \param surfIndex surface index, which must correspond to a buffer.
///
/// \param channelMask enabled channels. Must be a compile time constant.
///
/// \param m the matrix that stores the data to be written. The type T must be
/// of size of DWord (i.e., int, uint, or float). The size N1 is at least the
/// number of enabled channels and N2 must be either 8 or 16.
///
/// \param u the offsets of the date elements to be read from surface,
/// which must be in unit of dwords. The size N2 must be either 8 or 16.
///
/// The compiler generates code for GenX hardware to perform scattered read from
/// offsets given by ::u. The results are returned in ::m with each enabled
/// channelreturned in the next row of ::m. The enabled channels are returned in
/// R, G, B, A order with no gap in ::m for disabled channels.
///
template <typename T, int N1, int N2>
typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&(N2 == 8 || N2 == 16)>::type
read_untyped(SurfaceIndex surfIndex, ChannelMaskType channelMask,
             matrix_ref<T, N1, N2> m, vector<uint, N2> u);

/// \brief Untyped surface write.
///
/// \param surfIndex surface index, which must correspond to a buffer.
///
/// \param channelMask enabled channels. Must be a compile time constant.
///
/// \param m the matrix to store the return results. The type T must be of size
/// of DWord (i.e., int, uint, or float). The size N1 is at least the number of
/// enabled channels and N2 must be either 8 or 16.
///
/// \param u the offsets of the data elements to written surface, which must be
/// in unit of DWords. The size N2 must be either 8 or 16.
///
/// The compiler generates code for GenX hardware to perform scattered read from
/// the given offsets. The results are returned in ::m with each enabled channel
/// returned in the next row of ::m. The enabled channels are returned in R, G,
/// B, A order with no gap in ::m for disabled channels.
///
template <typename T, int N1, int N2>
typename std::enable_if<
    details::is_fp_or_dword_type<T>::value &&(N2 == 8 || N2 == 16)>::type
write_untyped(SurfaceIndex surfIndex, ChannelMaskType channelMask,
              matrix<T, N1, N2> m, vector<uint, N2> u);

/// \brief Shared local memory read.
///
/// Read 8 or 16 data elements (byte, word, or dword) from the SLM buffer
/// ::slmBuffer at the element-offsets specified in ::vAddr, and write back into
/// the vector ::vDst.
///

template <typename T, int N>
CM_NODEBUG
CM_INLINE typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_read_scaled(uint slmBuffer, vector<uint, N> vAddr, vector_ref<T, N> vDst) {
  vDst = details::__cm_intrinsic_impl_slm_read<T, N, sizeof(T)>(slmBuffer,
    vAddr, vDst);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
(N == 8 || N == 16) && (sizeof(T) == 1 || sizeof(T) == 2)>::type
cm_slm_read_scaled(uint slmBuffer, vector<uint, N> vAddr, vector_ref<T, N> vDst) {
  typedef typename details::dword_type<T>::type T1;
  vector<T1, N> _VDst = vDst;
  vDst = details::__cm_intrinsic_impl_slm_read<T1, N, sizeof(T)>(slmBuffer,
    vAddr, _VDst);
}

template <typename T, int N>
CM_NODEBUG
CM_INLINE typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_read_scaled(uint slmBuffer, vector<ushort, N> vAddr, vector_ref<T, N> vDst) {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_read_scaled(slmBuffer, _VAddr, vDst);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
(N == 8 || N == 16) && (sizeof(T) == 1 || sizeof(T) == 2)>::type
cm_slm_read_scaled(uint slmBuffer, vector<ushort, N> vAddr, vector_ref<T, N> vDst) {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_read_scaled(slmBuffer, _VAddr, vDst);
}

template <typename T, int N>
CM_NODEBUG
CM_INLINE typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_read(uint slmBuffer, vector<uint, N> vAddr, vector_ref<T, N> vDst) {
  vAddr = vAddr * sizeof(T);
  vDst = details::__cm_intrinsic_impl_slm_read<T, N, sizeof(T)>(slmBuffer,
                                                                vAddr, vDst);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    (N == 8 || N == 16) && (sizeof(T) == 1 || sizeof(T) == 2)>::type
cm_slm_read(uint slmBuffer, vector<uint, N> vAddr, vector_ref<T, N> vDst) {
  typedef typename details::dword_type<T>::type T1;
  vector<T1, N> _VDst = vDst;
  vAddr = vAddr * sizeof(T);
  vDst = details::__cm_intrinsic_impl_slm_read<T1, N, sizeof(T)>(slmBuffer,
                                                                 vAddr, _VDst);
}

template <typename T, int N>
CM_NODEBUG
CM_INLINE typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_read(uint slmBuffer, vector<ushort, N> vAddr, vector_ref<T, N> vDst) {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_read(slmBuffer, _VAddr, vDst);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    (N == 8 || N == 16) && (sizeof(T) == 1 || sizeof(T) == 2)>::type
cm_slm_read(uint slmBuffer, vector<ushort, N> vAddr, vector_ref<T, N> vDst) {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_read(slmBuffer, _VAddr, vDst);
}

/// \brief Shared local memory write.
///
/// Write 8 or 16 data elements (byte, word, or dword) given in the vector
/// ::vSrc into the SLM buffer ::slmBuffer at the element-offsets specified in
/// ::vAddr. Note that the addresses are in units of element size andd writes to
/// overlapping addresses will have undefined write ordering. N = 8 or 16, and
/// ::T could be of size byte, word or dword.
///

template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
(N == 8 || N == 16) && (sizeof(T) == 1 || sizeof(T) == 2)>::type
cm_slm_write_scaled(uint slmBuffer, vector<uint, N> vAddr, vector<T, N> vSrc) {
  typedef typename details::dword_type<T>::type T1;
  vector<T1, N> _VSrc = vSrc;
  details::__cm_intrinsic_impl_slm_write<T1, N, sizeof(T)>(slmBuffer, vAddr,
    _VSrc);
}

template <typename T, int N>
CM_NODEBUG
CM_INLINE typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_write_scaled(uint slmBuffer, vector<uint, N> vAddr, vector<T, N> vSrc) {
  details::__cm_intrinsic_impl_slm_write<T, N, sizeof(T)>(slmBuffer, vAddr,
    vSrc);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
(N == 8 || N == 16) && (sizeof(T) == 1 || sizeof(T) == 2)>::type
cm_slm_write_scaled(uint slmBuffer, vector<ushort, N> vAddr, vector<T, N> vSrc) {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_write_scaled(slmBuffer, _VAddr, vSrc);
}

template <typename T, int N>
CM_NODEBUG
CM_INLINE typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_write_scaled(uint slmBuffer, vector<ushort, N> vAddr, vector<T, N> vSrc) {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_write_scaled(slmBuffer, _VAddr, vSrc);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    (N == 8 || N == 16) && (sizeof(T) == 1 || sizeof(T) == 2)>::type
cm_slm_write(uint slmBuffer, vector<uint, N> vAddr, vector<T, N> vSrc) {
  typedef typename details::dword_type<T>::type T1;
  vector<T1, N> _VSrc = vSrc;
  vAddr = vAddr * sizeof(T);
  details::__cm_intrinsic_impl_slm_write<T1, N, sizeof(T)>(slmBuffer, vAddr,
                                                           _VSrc);
}

template <typename T, int N>
CM_NODEBUG
CM_INLINE typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_write(uint slmBuffer, vector<uint, N> vAddr, vector<T, N> vSrc) {
  vAddr = vAddr * sizeof(T);
  details::__cm_intrinsic_impl_slm_write<T, N, sizeof(T)>(slmBuffer, vAddr,
                                                          vSrc);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE typename std::enable_if<
    (N == 8 || N == 16) && (sizeof(T) == 1 || sizeof(T) == 2)>::type
cm_slm_write(uint slmBuffer, vector<ushort, N> vAddr, vector<T, N> vSrc) {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_write(slmBuffer, _VAddr, vSrc);
}

template <typename T, int N>
CM_NODEBUG
CM_INLINE typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_write(uint slmBuffer, vector<ushort, N> vAddr, vector<T, N> vSrc) {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_write(slmBuffer, _VAddr, vSrc);
}

/// \brief Shared local memmory read.
///
/// Read 8 or 16 4-element vectors, say {R,G,B,A}, where each element is of size
/// dword and is also referred to as a channel. The elements read from SLM are
/// written back to the vector ::vDst and organized channel-wise, i.e. all R's
/// followed by all G's, and so on. Address of each 4-element vector inside the
/// SLM buffer 'slmBuffer' must be specified in ::vAddr. Note that the
/// addresses are in units of element size. One or more channels in the
/// 4-element vector could be masked, and ::vDst contains only the unmasked
/// elements. Only the lower 4 bits of ::mask specify the elements masked. A '1'
/// implies that the corresponding element of each vector will not be read from
/// SLM. e.g. if mask = SLM_BR_ENABLE, ::vDst would contain
/// B7B6B5B4B3B2B1B0
/// R7R6R5R4R3R2R1R0
/// where the data elements correspond to the 8 vectors
/// (x B7 x R7), (x B6 x R6),... (x B0 x R0) read from SLM at the addresses
/// specified in ::vAddr, where 'x' means the value is not read.
///
/// ::M must equal ::N * C where C is the number of channels enabled in ::mask.
///
template <typename T, int N, int M>
typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_read4(uint slmBuffer, vector<uint, N> vAddr, vector_ref<T, M> vDst,
             SLM_ChannelMaskType mask);

template <typename T, int N, int M>
CM_DEPRECATED(
    "please use 'cm_slm_read4' with 'uint' as the element offset type!")
    CM_NODEBUG typename std::enable_if<
        (N == 8 || N == 16) &&
        (sizeof(T) == 4)>::type cm_slm_read4(uint slmBuffer,
                                             vector<ushort, N> vAddr,
                                             vector_ref<T, M> vDst,
                                             SLM_ChannelMaskType mask);

template <typename T, int N, int M>
CM_DEPRECATED(
    "please use 'cm_slm_read4' with 'SLM_ChannelMaskType' as the mask type!")
    CM_NODEBUG typename std::enable_if<
        (N == 8 || N == 16) &&
        (sizeof(T) == 4)>::type cm_slm_read4(uint slmBuffer,
                                             vector<ushort, N> vAddr,
                                             vector_ref<T, M> vDst, int mask);

template <typename T, int N, int M>
typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_read4_scaled(uint slmBuffer, vector<uint, N> vAddr, vector_ref<T, M> vDst,
  SLM_ChannelMaskType mask);

template <typename T, int N, int M>
CM_DEPRECATED(
  "please use 'cm_slm_read4_scaled' with 'uint' as the element offset type!")
  CM_NODEBUG typename std::enable_if<
  (N == 8 || N == 16) &&
  (sizeof(T) == 4)>::type cm_slm_read4_scaled(uint slmBuffer,
    vector<ushort, N> vAddr,
    vector_ref<T, M> vDst,
    SLM_ChannelMaskType mask);

template <typename T, int N, int M>
CM_DEPRECATED(
  "please use 'cm_slm_read4_scaled' with 'SLM_ChannelMaskType' as the mask type!")
  CM_NODEBUG typename std::enable_if<
  (N == 8 || N == 16) &&
  (sizeof(T) == 4)>::type cm_slm_read4_scaled(uint slmBuffer,
    vector<ushort, N> vAddr,
    vector_ref<T, M> vDst, int mask);

template <typename T, int N, int M>
CM_DEPRECATED(
  "please use 'cm_slm_read4_scaled' with 'SLM_ChannelMaskType' as the mask type!")
  CM_NODEBUG typename std::enable_if<
  (N == 8 || N == 16) &&
  (sizeof(T) == 4)>::type cm_slm_read4_scaled(uint slmBuffer,
    vector<uint, N> vAddr,
    vector_ref<T, M> vDst, int mask);

/// \brief Share local memory write.
///
/// Write 8 or 16 4-element vectors, say {R,G,B,A}, where each element is of
/// size dword and is also referred to as a channel. The elements to be written
/// must be in the vector ::vSrc and organized channel-wise, i.e. all R's
/// followed by all G's, and so on. Address of each 4-element vector inside the
/// SLM buffer ::slmBuffer must be specified in ::vAddr. Note that the
/// addresses are in units of element size. One or more channels in the
/// 4-element vector could be masked, and ::vSrc must contain only the unmasked
/// or enabled elements. The argument ::mask specifies the channels that are
/// enabled. Only the enabled channels are written to SLM. E.g. if mask =
/// SLM_BR_ENABLE (i.e. only R and B channels enabled), and vSrc is
/// B7B6B5B4B3B2B1B0
/// R7R6R5R4R3R2R1R0
/// 8 vectors written to SLM are as
/// (x B7 x R7), (x B6 x R6), ... (x B0 x R0)
/// where 'x' means the value is not written.
///
/// M must equal N * C where C is the number of channels enabled in ::mask. T
/// must be of size dword (int, uint, or float). ::mask specifies
/// the channels that are enabled it has to be a compile-time constant of the
/// enum type SLM_ChannelMaskType.
template <typename T, int N, int M>
typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_write4(uint slmBuffer, vector<uint, N> vAddr, vector<T, M> vSrc,
              SLM_ChannelMaskType mask);

template <typename T, int N, int M>
CM_DEPRECATED(
    "please use 'cm_slm_write4' with 'uint' as the element offset type!")
    CM_NODEBUG typename std::enable_if<
        (N == 8 || N == 16) &&
        (sizeof(T) == 4)>::type cm_slm_write4(uint slmBuffer,
                                              vector<ushort, N> vAddr,
                                              vector<T, M> vSrc,
                                              SLM_ChannelMaskType mask);

template <typename T, int N, int M>
CM_DEPRECATED(
    "please use 'cm_slm_write4' with 'SLM_ChannelMaskType' as the mask type!")
    CM_NODEBUG typename std::enable_if<
        (N == 8 || N == 16) &&
        (sizeof(T) == 4)>::type cm_slm_write4(uint slmBuffer,
                                              vector<ushort, N> vAddr,
                                              vector<T, M> vSrc, int mask);

template <typename T, int N, int M>
typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
cm_slm_write4_scaled(uint slmBuffer, vector<uint, N> vAddr, vector<T, M> vSrc,
  SLM_ChannelMaskType mask);

template <typename T, int N, int M>
CM_DEPRECATED(
  "please use 'cm_slm_write4_scaled' with 'uint' as the element offset type!")
  CM_NODEBUG typename std::enable_if<
  (N == 8 || N == 16) &&
  (sizeof(T) == 4)>::type cm_slm_write4_scaled(uint slmBuffer,
    vector<ushort, N> vAddr,
    vector<T, M> vSrc,
    SLM_ChannelMaskType mask);

template <typename T, int N, int M>
CM_DEPRECATED(
  "please use 'cm_slm_write4_scaled' with 'SLM_ChannelMaskType' as the mask type!")
  CM_NODEBUG typename std::enable_if<
  (N == 8 || N == 16) &&
  (sizeof(T) == 4)>::type cm_slm_write4_scaled(uint slmBuffer,
    vector<ushort, N> vAddr,
    vector<T, M> vSrc, int mask);

template <typename T, int N, int M>
CM_DEPRECATED(
  "please use 'cm_slm_write4_scaled' with 'SLM_ChannelMaskType' as the mask type!")
  CM_NODEBUG typename std::enable_if<
  (N == 8 || N == 16) &&
  (sizeof(T) == 4)>::type cm_slm_write4_scaled(uint slmBuffer,
    vector<uint, N> vAddr,
    vector<T, M> vSrc, int mask);

static const uint __cm_init_seq[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

/// \brief Shared local memory read.
///
/// Load ::size bytes from memory surface ::index starting at ::offset to the
/// SLM buffer ::slmBuffer. ::size must be a multiple of 256.
///
template <typename T = void>
CM_INLINE void cm_slm_load(uint slmBuffer, SurfaceIndex index, uint offset,
                           uint size) {
  vector<uint, 16> vOffset(__cm_init_seq);
  vOffset.select<8, 1>(8) = vOffset.select<8, 1>(0) + 8;

  uint numTotalBlocks = size / 256;
  uint numGroups = cm_linear_local_size();
  uint numBlocks = numTotalBlocks / numGroups;
  uint numLeftOver = numTotalBlocks % numGroups;
  numBlocks += (cm_linear_local_id() < numLeftOver) ? 1 : 0;

  // We just need numBlocks and numGroups
  uint elemSize = sizeof(float);
  uint threadOffsetInSLM = cm_linear_local_id() * 256;
  // in bytes
  uint threadOffsetInMemory = offset + threadOffsetInSLM;
  // in unit of elements
  vector<uint, 16> vOffsets = (threadOffsetInSLM / elemSize) + vOffset * 4;

  for (uint block = 0; block < numBlocks; block++) {
    vector<uint, 32> row0; // 32 floats or 128 Bytes or 4 GRF-registers
    vector<uint, 32> row1;
    vector<uint, 64> rowTrans;
    read(index, threadOffsetInMemory, row0);
    read(index, threadOffsetInMemory + 128, row1);

    // Transpose
    rowTrans.select<8, 1>(0) = row0.select<8, 4>(0);
    rowTrans.select<8, 1>(16) = row0.select<8, 4>(1);
    rowTrans.select<8, 1>(32) = row0.select<8, 4>(2);
    rowTrans.select<8, 1>(48) = row0.select<8, 4>(3);

    rowTrans.select<8, 1>(8) = row1.select<8, 4>(0);
    rowTrans.select<8, 1>(24) = row1.select<8, 4>(1);
    rowTrans.select<8, 1>(40) = row1.select<8, 4>(2);
    rowTrans.select<8, 1>(56) = row1.select<8, 4>(3);

    cm_slm_write4(slmBuffer, vOffsets, rowTrans, SLM_ABGR_ENABLE);
    threadOffsetInMemory += numGroups * 256;
    vOffsets += numGroups * 64;
  }

#if CM_GENX > 900
  cm_slm_fence(CM_GLOBAL_COHERENT_FENCE);
#endif
  cm_barrier();
}

/// cm_slm_atomic
///
/// This performs atomic read-modify-write operations on the destination
/// locations addressed. The destination locations are given in ::vAddr and the
/// desired atomic operation is specified as ::op. The value returned in ::vDst
/// depends on the atomic operation. Whether the two optional sources, ::vSrc0
/// and ::vSrc1, are needed also depends on the atomic operation.
///
/// * both vSrc0 and vSrc1 are required for ATOMIC_CMPXCHG
/// * no source is required form ATOMIC_INC, ATOMIC_DEC, and ATOMIC_PREDEC
/// * only vSrc0 is required for others
/// * vDst is optionally be 0 which indicates no return value to be used.
///
/// Atomic operation ::op must be a compile-time constant.
///
template <typename T, int N>
typename std::enable_if<(N == 8 || N == 16) && sizeof(T) == 4, void>::type
cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
              vector_ref<T, N> vDst, vector<T, N> vSrc0, vector<T, N> vSrc1);

template <typename T, int N>
typename std::enable_if<(N == 8 || N == 16) && sizeof(T) == 4, void>::type
cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
              int dummy, vector<T, N> vSrc0, vector<T, N> vSrc1);

template <typename T, int N>
typename std::enable_if<(N == 8 || N == 16) && sizeof(T) == 4, void>::type
cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
              vector_ref<T, N> vDst);

template <int N>
typename std::enable_if<(N == 8 || N == 16), void>::type
cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
              int dummy);

template <typename T, int N>
typename std::enable_if<(N == 8 || N == 16) && sizeof(T) == 4, void>::type
cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
              vector_ref<T, N> vDst, vector<T, N> vSrc0);

template <typename T, int N>
typename std::enable_if<(N == 8 || N == 16) && sizeof(T) == 4, void>::type
cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
              int dummy, vector<T, N> vSrc0);

/// \brief SLM OWord block read.
///
/// @param slmBuffer, which must correspond to the slm buffer.
///
/// @param attr indicates the offset alignment and properties, one of ::GENX_NONE
/// ::GENX_DWALIGNED ::GENX_MODIFIED_DWALIGNED ::GENX_CONSTANT_DWALIGNED.
///
/// @param offset zero based offset of the input buffer in bytes. Must be oword
/// (i.e. 16 bytes) aligned unless one of the DWALIGNED attributes is present.
///
/// @param output the data location to be read.
///
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_block_read(uint slmBuffer, CmBufferAttrib attr,
                                            int offset,
                                            vector_ref<T, N> output) {
  constexpr unsigned _Sz = sizeof(T) * N;

  if constexpr(_Sz < details::OWORD) {
    constexpr unsigned _N = details::OWORD / sizeof(T);

    // Choose implementation based on the buffer attribute. Since this attribute
    // is not a compilation time constant, we cannot statically verify the
    // requirement that attribute is either GENX_NONE or GENX_DWALIGNED. Leave
    // this check to the emulation mode.
    if (attr == GENX_DWALIGNED || attr == GENX_MODIFIED_DWALIGNED ||
        attr == GENX_CONSTANT_DWALIGNED) {
      vector<T, _N> _Output =
          details::__cm_intrinsic_impl_slm_oword_read_dwaligned<T, _N>(
              slmBuffer, offset);
      return details::if_assign(output, _Output);
    }

    // By default, no attribute.
    vector<T, _N> _Output =
        details::__cm_intrinsic_impl_slm_oword_read<T, _N>(slmBuffer, offset);
    return details::if_assign(output, _Output);
  }

  // Check the data size.
  CM_STATIC_ERROR(details::getNextPowerOf2(_Sz) <=
                      details::getMaxNumOfOWordSLM() * details::OWORD,
                  "read must not exceed the maximal number of owords");

  if constexpr(!details::isPowerOf2(_Sz)) {
    constexpr unsigned _N = details::getNextPowerOf2(_Sz) / sizeof(T);

    if (attr == GENX_DWALIGNED || attr == GENX_MODIFIED_DWALIGNED
        || attr == GENX_CONSTANT_DWALIGNED) {
      vector<T, _N> _Output =
          details::__cm_intrinsic_impl_slm_oword_read_dwaligned<T, _N>(slmBuffer, offset);
      return details::if_assign(output, _Output);
    }

    // By default, no attribute.
    vector<T, _N> _Output =
        details::__cm_intrinsic_impl_slm_oword_read<T, _N>(slmBuffer, offset);
    return details::if_assign(output, _Output);
  }

  // No padding case.
  if (attr == GENX_DWALIGNED || attr == GENX_MODIFIED_DWALIGNED
      || attr == GENX_CONSTANT_DWALIGNED)
    output =
        details::__cm_intrinsic_impl_slm_oword_read_dwaligned<T, N>(slmBuffer, offset);
  else
    output = details::__cm_intrinsic_impl_slm_oword_read<T, N>(slmBuffer, offset);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_block_read(uint slmBuffer, int X,
                                            vector_ref<T, N> src) {
  cm_slm_block_read(slmBuffer, GENX_NONE, X, src);
}

/// \brief OWord block write.
/// @param slmBuffer, index corresponds to the SLM buffer.
///
/// @param offset zero based offset of the input buffer in bytes.
///
/// @param src the data to be written. The size of vector can be only 1, 2, 4 or
/// 8 owords.
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_block_write(uint slmBuffer, int offset,
                                vector<T, N> src) {
  constexpr unsigned Sz = N * sizeof(T);
  CM_STATIC_ERROR(details::isPowerOf2(Sz) && Sz >= details::OWORD &&
                      Sz <= details::getMaxNumOfOWordSLM() * details::OWORD,
                  "vector must not exceed the maximal number of owords");
  details::__cm_intrinsic_impl_slm_oword_write(slmBuffer, offset, src);
}

#endif /* _CLANG_CM_DATAPORT_H_ */
