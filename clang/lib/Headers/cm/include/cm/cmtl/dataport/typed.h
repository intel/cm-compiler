/*========================== begin_copyright_notice ============================

Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_CMTL_DATAPORT_TYPED_H
#define CM_CMTL_DATAPORT_TYPED_H

namespace cmtl {
namespace dataport {


template <int x_offset, int y_offset, typename T, int N, int M>
CM_INLINE void read(SurfaceIndex idx, int X, int Y,
                    matrix_ref<T, N, M> output) {
  auto new_X = X + x_offset;
  auto new_Y = Y + y_offset;
#if (CM_GENX < 1290) // before Xe2
  ::read(idx, new_X * sizeof(T), new_Y, output);
#else
  output = 0;
  if (new_X >= 0 && new_Y >= 0) {
    ::read(idx, new_X * sizeof(T), new_Y, output);
  } else if (new_Y < 0 && new_X >= 0) {
    // out of upper bound
    if constexpr (y_offset < 0 && N > -y_offset) {
      ::read(idx, new_X * sizeof(T), Y,
             output.select<N + y_offset, 1, M, 1>(-y_offset, 0));
    }
  } else if (new_X < 0 && new_Y >= 0) {
    // out of left bound
    if constexpr (x_offset < 0 && M > -x_offset) {
      ::read(idx, X * sizeof(T), new_Y,
             output.select<N, 1, M + x_offset, 1>(0, -x_offset));
    }
  } else {
    // both new_Y < 0 and new_X < 0
    // out of upper left bound
    if constexpr (y_offset < 0 && x_offset < 0 && N > -y_offset &&
                  M > -x_offset) {
      ::read(idx, X * sizeof(T), Y,
             output.select<N + y_offset, 1, M + x_offset, 1>(-y_offset,
                                                             -x_offset));
    }
  }
#endif
}

} // namespace dataport
} // namespace cmtl

#endif // CM_CMTL_DATAPORT_TYPED_H
