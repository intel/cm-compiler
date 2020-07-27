/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

#ifndef CM_CM_MASK_H
#define CM_CM_MASK_H

#include "cm_traits.h"
#include "cm_common.h"
#include "cm_internal.h"

using MaskIntT = uint;
using MaskVecElemT = ushort;
template<int size>
using MaskVecT = vector<MaskVecElemT, size>;

namespace details {

template<int size>
constexpr int legal_mask_size() {
  static_assert(size <= 32, "wrong argument: size is too big");
  static_assert(size != 8 && size != 16 && size != 32,
    "wrong argument: initial size must be illegal");

  if (size < 8)
    return 8;
  if (size < 16)
    return 16;
  return 32;
}

} // namespace details

template <int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N == 8 || N == 16 || N == 32), MaskIntT>::type
cm_pack_mask(MaskVecT<N> src0) {
  return details::__cm_intrinsic_impl_pack_mask(src0);
}

template <int N1, int N2>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N1 *N2 == 8 || N1 *N2 == 16 || N1 * N2 == 32), MaskIntT>::type
cm_pack_mask(matrix<MaskVecElemT, N1, N2> src0) {
  return details::__cm_intrinsic_impl_pack_mask(src0);
}

template <int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N != 8 && N != 16 && N < 32), MaskIntT>::type
cm_pack_mask(MaskVecT<N> src0) {
  MaskVecT<details::legal_mask_size<N>()> _src0 = 0;
  _src0.template select<N, 1>() = src0.template format<MaskVecElemT>();
  return cm_pack_mask(_src0);
}

template <int N1, int N2>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N1 *N2 != 8 && N1 * N2 != 16 && N1 * N2 < 32), MaskIntT>::type
cm_pack_mask(matrix<MaskVecElemT, N1, N2> src0) {
  MaskVecT<details::legal_mask_size<N1 *N2>()> _src0 = 0;
  _src0.template select<N1 *N2, 1>() = src0.template format<MaskVecElemT>();
  return cm_pack_mask(_src0);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N == 8 || N == 16 || N == 32), MaskVecT<N> >::type
cm_unpack_mask(MaskIntT src0) {
  return details::__cm_intrinsic_impl_unpack_mask<T, N>(src0);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N != 8 && N != 16 && N < 32), MaskVecT<N> >::type
cm_unpack_mask(MaskIntT src0) {
  static_assert(N > 0);

  auto legal_vec =
    details::__cm_intrinsic_impl_unpack_mask<T, details::legal_mask_size<N>()>(src0);
  return legal_vec.select<N, 1>(0);
}

#endif // CM_CM_MASK_H
