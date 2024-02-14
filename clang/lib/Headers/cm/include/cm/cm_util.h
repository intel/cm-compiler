/*========================== begin_copyright_notice ============================

Copyright (C) 2014-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_util.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_UTIL_H_
#define _CLANG_CM_UTIL_H_

#include "cm_common.h"
#include "cm_has_instr.h"
#include "cm_traits.h"

namespace details {

/// Constant in number of bytes.
enum { BYTE = 1, WORD = 2, DWORD = 4, QWORD = 8, OWORD = 16, GRF = 32 };

/// Round up N to be multiple of M
static constexpr unsigned int roundUpNextMultiple(unsigned int N,
                                                  unsigned int M) {
  return ((N + M - 1) / M) * M;
}

/// Compute the next power of 2 at compile time.
static constexpr unsigned int getNextPowerOf2(unsigned int n,
                                              unsigned int k = 1) {
  return (k >= n) ? k : getNextPowerOf2(n, k * 2);
}

/// Check if a given 32 bit positive integer is a power of 2 at compile time.
static inline constexpr bool isPowerOf2(unsigned int n) {
  return (n & (n - 1)) == 0;
}

static inline constexpr bool isPowerOf2(unsigned int n, unsigned int limit) {
  return (n & (n - 1)) == 0 && n <= limit;
}

/// Do the assignment conditionally. This is to avoid template instantiation
/// errors for unreachable branches. For example, the following code will fail
/// to instantiate, for any integer pair (N1, N2). This is because although
/// true/false-branch cannot be executed, both branches need fully
/// instantitated, resulting out-of-bound errors during compilation.
///
/// \code
/// template <int N1, int N2>
/// void foo(vector<float, N1> v1, vector<float, N2> v2) {
///   if (N1 < N2)
///     v1 = v2.select<N1, 1>();
///   else
///     v2 = v2.select<N2, 1>();
/// }
/// \code
template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<(N1 <= N2), void>::type
if_assign(vector_ref<T, N1> v1, vector<T, N2> v2) {
  v1 = v2.select<N1, 1>();
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<(N1 > N2), void>::type
if_assign(vector_ref<T, N1> v1, vector<T, N2> v2) {}

template <typename T, int N, int M1, int M2>
CM_NODEBUG CM_INLINE typename std::enable_if<(M1 <= M2), void>::type
if_assign(matrix_ref<T, N, M1> m1, matrix<T, N, M2> m2) {
  m1 = m2.select<N, 1, M1, 1>();
}

template <typename T, int N, int M1, int M2>
CM_NODEBUG CM_INLINE typename std::enable_if<(M1 > M2), void>::type
if_assign(matrix_ref<T, N, M1> m1, matrix<T, N, M2> m2) {}

// Wrapper around vector select method.
// Unlike select this fucntion ignores stride when width is 1,
// so it is easier to use in generic code.
// The functionality of select is reduced to only reading the region.
template <int width, int stride, typename T, int size>
CM_NODEBUG CM_INLINE vector<T, width> read_region(vector<T, size> vec,
                                                  int offset) {
  static_assert(width > 0 && width < size, "invalid width");
  static_assert(stride > 0 && (width - 1) * stride < size,
                "invalid invalid stride");
  vector<T, width> selection;
  if constexpr (width == 1)
    selection = vec[offset];
  else
    selection = vec.select<width, stride>(offset);
  return selection;
}

// Wrapper around vector select method.
// Unlike select this fucntion ignores stride when width is 1,
// so it is easier to use in generic code.
// The functionality of select is reduced to only writing the region.
template <int stride, typename T, int size, int width>
CM_NODEBUG CM_INLINE void write_region(vector_ref<T, size> vec,
                                       vector<T, width> insertion, int offset) {
  static_assert(stride > 0 && (width - 1) * stride < size,
                "invalid invalid stride");
  if constexpr (width == 1)
    vec[offset] = insertion[0];
  else
    vec.select<width, stride>(offset) = insertion;
}

static inline constexpr unsigned getMaxNumOfOWordSLM() {
#if __CM_INTEL_TARGET_CORE >= 12
  return 16;
#else
  return 8;
#endif
}

// to emit warnings, dependent on if function actually called
template <typename T = void> constexpr bool always_false() { return false; }

template <VectorSize VS> constexpr unsigned lsc_vector_size() {
  constexpr unsigned NElts[] = {0, 1, 2, 3, 4, 8, 16, 32, 64};
  return NElts[static_cast<unsigned>(VS)];
}

template <unsigned N> constexpr VectorSize lsc_vector_size() {
  switch (N) {
  case 1:
    return VectorSize::N1;
  case 2:
    return VectorSize::N2;
  case 3:
    return VectorSize::N3;
  case 4:
    return VectorSize::N4;
  case 8:
    return VectorSize::N8;
  case 16:
    return VectorSize::N16;
  case 32:
    return VectorSize::N32;
  case 64:
    return VectorSize::N64;
  default:
    break;
  }
  return VectorSize::N0;
}

template <typename T, DataSize DS> constexpr DataSize lsc_data_size() {
  if constexpr (DS != DataSize::Default)
    return DS;
  else if constexpr (sizeof(T) == 1)
    return DataSize::U8;
  else if constexpr (sizeof(T) == 2)
    return DataSize::U16;
  else if constexpr (sizeof(T) == 4)
    return DataSize::U32;
  else if constexpr (sizeof(T) == 8)
    return DataSize::U64;
  else if constexpr (DS == DataSize::Default)
    static_assert(DS != DataSize::Default && "unsupported data type");
  return DS;
}

template <typename T, int N, VectorSize VS> CM_INLINE auto lsc_data_type_ext() {
  constexpr unsigned NumElts = lsc_vector_size<VS>() * N;
  static_assert(NumElts > 0 && "unexpected number of elements");
  if constexpr (sizeof(T) < 4)
    return vector<uint32_t, NumElts>();
  else
    return vector<T, NumElts>();
}

template <typename T, int N, VectorSize VS> CM_INLINE auto lsc_data_type() {
  constexpr unsigned NumElts = lsc_vector_size<VS>() * N;
  static_assert(NumElts > 0 && "unexpected number of elements");
  return vector<T, NumElts>();
}
// U8 and U16 types  are not supported
// use U8U32 and U16U32 instead
constexpr DataSize lsc_expand_ds(DataSize ds) {
  if (ds == DataSize::U8)
    return DataSize::U8U32;
  if (ds == DataSize::U16)
    return DataSize::U16U32;
  return ds;
}

template <typename T> struct lsc_expand_type {
  typedef typename std::conditional<sizeof(T) < 4, uint32_t, T>::type type;
};

// fp has to be bitcsted to uint before zextention
template <typename T> struct lsc_bitcast_type {
private:
  typedef typename std::conditional<sizeof(T) == 2, uint16_t, T>::type _type1;
  typedef typename std::conditional<sizeof(T) == 1, uint8_t, T>::type _type2;

public:
  typedef
      typename std::conditional<sizeof(_type2) == 1, _type2, _type1>::type type;
};
// format U8U32 and U16U32 back to U8 and U16
template <typename T, typename From, typename To>
CM_INLINE To lsc_format_ret(From from) {
  auto _Formatted = from.format<T>();
  constexpr int stride =
      from.n_elems() == 1 ? 1 : _Formatted.n_elems() / from.n_elems();
  To _Res = _Formatted.select<from.n_elems(), stride>(0);
  return _Res;
};

template <AtomicOp Op> constexpr int lsc_atomic_nsrcs() {
  switch (Op) {
  case AtomicOp::IINC:
  case AtomicOp::IDEC:
  case AtomicOp::LOAD:
    return 0;
  case AtomicOp::STORE:
  case AtomicOp::IADD:
  case AtomicOp::ISUB:
  case AtomicOp::SMIN:
  case AtomicOp::SMAX:
  case AtomicOp::UMIN:
  case AtomicOp::UMAX:
  case AtomicOp::FSUB:
  case AtomicOp::FMIN:
  case AtomicOp::FMAX:
  case AtomicOp::FADD:
  case AtomicOp::AND:
  case AtomicOp::OR:
  case AtomicOp::XOR:
    return 1;
  case AtomicOp::ICAS:
  case AtomicOp::FCAS:
    return 2;
  default:
    break;
  }
  return 0;
}

// Compute the data size for 2d block load or store.
template <typename T, int NBlocks, int Height, int Width, bool Transposed,
          bool Transformed>
constexpr int getBlock2dDataSize() {
  if (Transformed)
    return roundUpNextMultiple(Height, 4 / sizeof(T)) * Width * NBlocks;
  return Width * Height * NBlocks;
}

constexpr int getRoundedWidthFor2dTypedLSC(int Width) {
  return Width < 4 ? 4 : details::getNextPowerOf2(Width);
}

// Return the default SIMT width.
template <typename T = void> constexpr int lsc_default_simt() {
#if CM_GENX >= 1280
  return 32; // SIMD32: PVC and later
#else        // CM_GENX < 1280
  return 16; // SIMD16: DG2
#endif       // CM_GENX >= 1280
}

// Check for valid type for atomic source and dest arguments
template <typename T> constexpr bool lsc_check_atomic_src() {
  // 8-bit atomics are unsupported
  return sizeof(T) >= 2;
}

template <ChannelMaskType Mask>
constexpr VectorSize lsc_get_vector_size_from_channel_mask() {
  switch (Mask) {
  case ChannelMaskType::_CM_R_ENABLE:
  case ChannelMaskType::_CM_G_ENABLE:
  case ChannelMaskType::_CM_B_ENABLE:
  case ChannelMaskType::_CM_A_ENABLE:
    return VectorSize::N1;
  case ChannelMaskType::_CM_GR_ENABLE:
  case ChannelMaskType::_CM_BR_ENABLE:
  case ChannelMaskType::_CM_BG_ENABLE:
  case ChannelMaskType::_CM_AR_ENABLE:
  case ChannelMaskType::_CM_AG_ENABLE:
  case ChannelMaskType::_CM_AB_ENABLE:
    return VectorSize::N2;
  case ChannelMaskType::_CM_BGR_ENABLE:
  case ChannelMaskType::_CM_AGR_ENABLE:
  case ChannelMaskType::_CM_ABR_ENABLE:
  case ChannelMaskType::_CM_ABG_ENABLE:
    return VectorSize::N3;
  case ChannelMaskType::_CM_ABGR_ENABLE:
    return VectorSize::N4;
  default:
    break;
  }
  return VectorSize::N0;
}

template <ChannelMaskType Mask>
constexpr unsigned lsc_get_num_elements_from_channel_mask() {
  constexpr auto _VS = lsc_get_vector_size_from_channel_mask<Mask>();
  return lsc_vector_size<_VS>();
}

} // namespace details

#endif /*_CLANG_CM_UTIL_H_ */
