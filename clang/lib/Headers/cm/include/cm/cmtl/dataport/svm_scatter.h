/*========================== begin_copyright_notice ============================

Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_CMTL_DATAPORT_SVM_SCATTER_H
#define CM_CMTL_DATAPORT_SVM_SCATTER_H

namespace cmtl {
namespace dataport {

template <unsigned size>
constexpr unsigned grain_scatter() {
  static_assert(size);

  unsigned ret = 1;
  while (ret < 32 && (size & ret) == 0) {
    ret <<= 1;
  }

  return ret;
}

// vector read
template <typename T, unsigned width>
CM_INLINE void read(svmptr_t addr, vector_ref<uint, width> offset,
                    vector_ref<T, width> vec, int off = 0) {
  vector<svmptr_t, width> vaddr = (offset + off) * sizeof(T) + addr;

  constexpr unsigned grain = grain_scatter<width>();
#pragma unroll
  for (int i = 0; i < width; i += grain) {
    ::cm_svm_scatter_read<T, grain>(vaddr.template select<grain, 1>(i),
                                    vec.template select<grain, 1>(i));
  }
}

// matrix read
template <typename T, unsigned height, unsigned width>
CM_INLINE void read(svmptr_t addr, vector_ref<uint, height * width> offset,
                    matrix_ref<T, height, width> m, int off = 0) {
  constexpr unsigned size = height * width;
  vector_ref<T, size> vec = m.template format<T>();
  read<T, size>(addr, offset, vec, off);
}

// vector read
// returning vector
template <typename T, unsigned width>
CM_INLINE vector<T, width> read(svmptr_t addr, vector_ref<uint, width> offset,
                                int off = 0) {
  vector<T, width> ret;
  read<T, width>(addr, offset, ret, off);
  return ret;
}

// matrix read
// returning matrix
template <typename T, unsigned height, unsigned width>
CM_INLINE matrix<T, height, width> read(svmptr_t addr,
                                        vector_ref<uint, height * width> offset,
                                        int off = 0) {
  matrix<T, height, width> ret;
  read<T, width>(addr, offset, ret, off);
  return ret;
}

// vector write
template <typename T, unsigned width>
CM_INLINE void write(svmptr_t addr, vector_ref<uint, width> offset,
                     vector_ref<T, width> vec, int off = 0) {
  vector<svmptr_t, width> vaddr = (offset + off) * sizeof(T) + addr;

  constexpr unsigned grain = grain_scatter<width>();
#pragma unroll
  for (int i = 0; i < width; i += grain) {
    ::cm_svm_scatter_write<T, grain>(vaddr.template select<grain, 1>(i),
                                     vec.template select<grain, 1>(i));
  }
}

// matrix_ref write
template <typename T, unsigned height, unsigned width>
CM_INLINE void write(svmptr_t addr, vector_ref<uint, height * width> offset,
                     matrix_ref<T, height, width> m, int off = 0) {
  constexpr unsigned size = height * width;
  vector_ref<T, size> vec = m.template format<T>();
  write<T, size>(addr, offset, vec, off);
}

// matrix write
template <typename T, unsigned height, unsigned width>
CM_INLINE void write(svmptr_t addr, vector_ref<uint, height * width> offset,
                     matrix<T, height, width> m, int off = 0) {
  matrix_ref<T, height, width> m_ = m;
  write<T, height, width>(addr, offset, m_, off);
}

// boundary check
template <unsigned width>
CM_INLINE int check(vector_ref<uint, width> offset, int off, unsigned size) {
#pragma unroll
  for (int i = 0; i < width; i++) {
    if (off + offset[i] >= size) {
      printf("Out-of-bound access detected at [%d], offset: %d, size: %d\n", i,
             off + offset[i], size);
      return 1;
    }
  }
  return 0;
}

// vector read with boundary check
template <typename T, unsigned width>
CM_INLINE void read(svmptr_t addr, vector_ref<uint, width> offset,
                    vector_ref<T, width> vec, int off, unsigned size) {
  if (check<width>(offset, off, size)) {
    vec = 0;
    return;
  }
  read<T, width>(addr, offset, vec, off);
}

// vector read with boundary check
// returning vector
template <typename T, unsigned width>
CM_INLINE vector<T, width> read(svmptr_t addr, vector_ref<uint, width> offset,
                                int off, unsigned size) {
  vector<T, width> ret;
  read<T, width>(addr, offset, ret, off, size);
  return ret;
}

// vector_ref write with boundary check
template <typename T, unsigned width>
CM_INLINE void write(svmptr_t addr, vector_ref<uint, width> offset,
                     vector_ref<T, width> vec, int off, unsigned size) {
  if (check<width>(offset, off, size)) {
    return;
  }
  write<T, width>(addr, offset, vec, off);
}

// vector write with boundary check
template <typename T, int width>
CM_INLINE void write(svmptr_t addr, vector_ref<uint, width> offset,
                     vector<T, width> vec, int off, int size) {
  vector_ref<T, width> vec_ = vec;
  write<T, width>(addr, offset, vec_, off, size);
}

// scalar read
// returning value
template <typename T>
CM_INLINE T read(svmptr_t addr, int off = 0) {
  vector<svmptr_t, 1> vaddr = addr + off * sizeof(T);
  vector<T, 1> data;
  ::cm_svm_scatter_read(vaddr, data);
  return data[0];
}

// scalar read with boundary check
// returning value
// FIXME returning 0 when out-of-bound access is detected
template <typename T>
CM_INLINE T read(svmptr_t addr, int off, int size) {
  if (off >= size) {
    printf("Out-of-bound access detected, offset: %d, size: %d\n", off, size);
    return 0;
  }
  return read<T>(addr, off);
}

// vector_ref<1> write
template <typename T>
CM_INLINE void write(svmptr_t addr, vector_ref<T, 1> vec, int off = 0) {
  vector<svmptr_t, 1> vaddr = addr + off * sizeof(T);
  ::cm_svm_scatter_write(vaddr, vec);
}

// vector<1> write
template <typename T>
CM_INLINE void write(svmptr_t addr, vector<T, 1> vec, int off = 0) {
  vector_ref<T, 1> vec_ = vec;
  write<T>(addr, vec_);
}

// matrix_ref<1, 1> write
template <typename T>
CM_INLINE void write(svmptr_t addr, matrix_ref<T, 1, 1> m, int off = 0) {
  vector<svmptr_t, 1> vaddr = addr + off * sizeof(T);
  vector_ref<T, 1> vec_ = m.template format<T>();
  ::cm_svm_scatter_write(vaddr, vec_);
}

// matrix<1, 1> write
template <typename T>
CM_INLINE void write(svmptr_t addr, matrix<T, 1, 1> m, int off = 0) {
  vector<svmptr_t, 1> vaddr = addr + off * sizeof(T);
  vector_ref<T, 1> vec_ = m.template format<T>();
  ::cm_svm_scatter_write(vaddr, vec_);
}

// vector_ref<1> write with boundary check
template <typename T>
CM_INLINE void write(svmptr_t addr, vector_ref<T, 1> data, int off, int size) {
  if (off >= size) {
    printf("Out-of-bound access detected, offset: %d, size: %d\n", off, size);
    return;
  }
  write<T>(addr, data, off);
}

template <typename T>
struct is_scalar {
  static constexpr bool value = false;
};

#define IS_ARITHMETIC(T)                \
  template <>                           \
  struct is_scalar<T> {                 \
    static constexpr bool value = true; \
  };

IS_ARITHMETIC(char);
IS_ARITHMETIC(unsigned char);
IS_ARITHMETIC(short);
IS_ARITHMETIC(unsigned short);
IS_ARITHMETIC(int);
IS_ARITHMETIC(unsigned int);
IS_ARITHMETIC(long);
IS_ARITHMETIC(unsigned long);
IS_ARITHMETIC(long long);
IS_ARITHMETIC(unsigned long long);
IS_ARITHMETIC(float);
#ifdef CM_HAS_DOUBLE
IS_ARITHMETIC(double);
#endif

// scalar write
template <typename T>
typename std::enable_if<is_scalar<T>::value, void>::type write(svmptr_t addr,
                                                               T data,
                                                               int off = 0) {
  vector<svmptr_t, 1> vaddr = addr + off * sizeof(T);
  vector<T, 1> vdata = data;
  ::cm_svm_scatter_write(vaddr, vdata);
}

// scalar write with boundary check
template <typename T>
typename std::enable_if<is_scalar<T>::value, void>::type write(svmptr_t addr,
                                                               T data, int off,
                                                               int size) {
  if (off >= size) {
    printf("Out-of-bound access detected, offset: %d, size: %d\n", off, size);
    return;
  }
  write<T>(addr, data, off);
}

} // namespace dataport
} // namespace cmtl

#endif // CM_CMTL_DATAPORT_SVM_SCATTER_H
