/*========================== begin_copyright_notice ============================

Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_CMTL_DATAPORT_SLM_H
#define CM_CMTL_DATAPORT_SLM_H

#include "svm.h"

namespace cmtl {
namespace dataport {

// copy `size` bytes from SVM buffer to SLM
template <typename T = int, unsigned width = 32>
CM_INLINE void cm_slm_copy(unsigned dstBuffer, svmptr_t srcBuffer,
                           unsigned size) {
  vector<T, width> data;
  constexpr auto grain = data.n_elems() * sizeof(T); // in bytes
  for (unsigned i = 0; i < size; i += grain) {
    read(srcBuffer + i, data);
    cm_slm_block_write(dstBuffer, i, data);
  }
}

// copy `size` bytes from SLM buffer to SVM
template <typename T = int, unsigned width = 32>
CM_INLINE void cm_slm_copy(svmptr_t dstBuffer, unsigned srcBuffer,
                           unsigned size) {
  vector<T, width> data;
  constexpr auto grain = data.n_elems() * sizeof(T); // in bytes
  for (unsigned i = 0; i < size; i += grain) {
    cm_slm_block_read(srcBuffer, i, data);
    write(dstBuffer + i, data);
  }
}

// vector read
template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void read(unsigned buf, vector_ref<T, width> vec, int off = 0) {
  __local vector<T, width> *ptr =
      reinterpret_cast<__local vector<T, width> *>(buf + off * sizeof(T));
  vec = load<vector<T, width>, Algn::value>(ptr);
}

// vector read
// returning vector
template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
CM_INLINE vector<T, width> read(unsigned buf, int off = 0) {
  vector<T, width> ret;
  read<T, width, Algn>(buf, ret, off);
  return ret;
}

// vector_ref write
template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void write(unsigned buf, vector_ref<T, width> vec, int off = 0) {
  __local vector<T, width> *ptr =
      reinterpret_cast<__local vector<T, width> *>(buf + off * sizeof(T));
  vector<T, width> data = vec;
  store<vector<T, width>, Algn::value>(data, ptr);
}

// vector write
template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void write(unsigned buf, vector<T, width> vec, int off = 0) {
  vector_ref<T, width> vec_ = vec;
  write<T, width, Algn>(buf, vec_, off);
}

// matrix_ref read
template <typename T, unsigned height, unsigned width,
          typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void read(unsigned buf, matrix_ref<T, height, width> m, int off = 0) {
  constexpr int size = height * width;
  vector_ref<T, size> vec = m.template format<T>();

  read<T, size, Algn>(buf, vec, off);
}

// matrix read
// returning matrix
template <typename T, unsigned height, unsigned width,
          typename Algn = AlignTag<Align::OWORD>>
CM_INLINE matrix<T, height, width> read(unsigned buf, int off = 0) {
  matrix<T, height, width> ret;
  read<T, height, width, Algn>(buf, ret, off);
  return ret;
}

// matrix_ref write
template <typename T, unsigned height, unsigned width,
          typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void write(unsigned buf, matrix_ref<T, height, width> m,
                     int off = 0) {
  constexpr unsigned size = height * width;
  vector_ref<T, size> vec = m.template format<T>();

  write<T, size, Algn>(buf, vec, off);
}

// matrix write
template <typename T, unsigned height, unsigned width,
          typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void write(unsigned buf, matrix<T, height, width> m, int off = 0) {
  matrix_ref<T, height, width> m_ = m;
  write<T, height, width, Algn>(buf, m_, off);
}

// vector read with boundary check
template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void read(unsigned buf, vector_ref<T, width> vec, int off,
                    unsigned size) {
  if (off + width > size) {
    printf("Out-of-bound read detected, offset: %d, size: %d\n", (off + width),
           size);
    vec = 0;
    return;
  }

  read<T, width, Algn>(buf, vec, off);
}

// vector read with boundary check
// returning vector
template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
CM_INLINE vector<T, width> read(unsigned buf, int off, int size) {
  vector<T, width> ret;
  read<T, width, Algn>(buf, ret, off, size);
  return ret;
}

// vector_ref write with boundary check
template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void write(unsigned buf, vector_ref<T, width> vec, int off,
                     unsigned size) {
  if (off + width > size) {
    printf("Out-of-bound write detected, offset: %d, size: %d\n", (off + width),
           size);
    return;
  }

  write<T, width, Algn>(buf, vec, off);
}

// vector write with boundary check
template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void write(unsigned buf, vector<T, width> vec, int off,
                     unsigned size) {
  vector_ref<T, width> vec_ = vec;
  write<T, width, Algn>(buf, vec_, off, size);
}

// matrix_ref read with boundary check
template <typename T, unsigned height, unsigned width,
          typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void read(unsigned buf, matrix_ref<T, height, width> m, int off,
                    unsigned size) {
  constexpr unsigned msize = height * width;
  if (off + msize > size) {
    printf("Read bound access detected, offset: %d, size: %d\n", (off + msize),
           size);
    m = 0;
    return;
  }

  vector_ref<T, msize> vec = m.template format<T>();
  read<T, msize, Algn>(buf, vec, off);
}

// matrix read with boundary check
// returning matrix
template <typename T, unsigned height, unsigned width,
          typename Algn = AlignTag<Align::OWORD>>
CM_INLINE matrix<T, height, width> read(unsigned buf, int off, unsigned size) {
  matrix<T, height, width> ret;
  read<T, height, width, Algn>(buf, ret, off, size);
  return ret;
}

// matrix_ref write with boundary check
template <typename T, unsigned height, unsigned width,
          typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void write(unsigned buf, matrix_ref<T, height, width> m, int off,
                     unsigned size) {
  constexpr unsigned msize = height * width;
  if (off + msize > size) {
    printf("Write bound access detected, offset: %d, size: %d\n", (off + msize),
           size);
    return;
  }

  vector_ref<T, msize> vec = m.template format<T>();
  write<T, msize, Algn>(buf, vec, off);
}

// matrix write with boundary check
template <typename T, unsigned height, unsigned width,
          typename Algn = AlignTag<Align::OWORD>>
CM_INLINE void write(unsigned buf, matrix<T, height, width> m, int off,
                     unsigned size) {
  matrix_ref<T, height, width> m_ = m;
  write<T, height, width, Algn>(buf, m_, off, size);
}

} // namespace dataport
} // namespace cmtl

#endif //  CM_CMTL_DATAPORT_SLM_H
