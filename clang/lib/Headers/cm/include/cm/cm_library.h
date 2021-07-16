/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_LIBRARY_
#define _CLANG_CM_LIBRARY_

#include "cm_internal.h"

////////////////////////////////////////////////////////////////////////////////
// Implement CM as a C++ library with minimum clang changes
////////////////////////////////////////////////////////////////////////////////
namespace experimental {

/**
 * The following code snippet illustrates the library based syntax for the CM
 * language.
 *
\code{.cpp}

#include <cm/cm_library.h>

using namespace experimental;

__declspec(genx_main) void kernel_explicit_ref(int *p) {
  simd_vector<unsigned, 32> offsets(0, 1);
  simd_vector<int, 32> v0;

  v0 = load(p, offsets);

  simd_vector_ref<int, 8, 1, decltype(v0)> r1 = v0.sel<8, 1>(0);  // r1 = v0[ 0: 8]
  simd_vector_ref<int, 8, 1, decltype(v0)> r2 = v0.sel<8, 1>(8);  // r2 = v0[ 8:16]
  simd_vector_ref<int, 8, 1, decltype(v0)> r3 = v0.sel<8, 1>(16); // r3 = v0[16:24]
  simd_vector_ref<int, 8, 1, decltype(v0)> r4 = v0.sel<8, 1>(24); // r4 = v0[24:32]

  simd_vector_ref<int, 4, 2, decltype(r1)> r10 = r1.sel<4, 2>();  // r10 = r1[0:8:2] = v0[ 0: 8:2]
  simd_vector_ref<int, 4, 2, decltype(r2)> r20 = r2.sel<4, 2>();  // r20 = r2[0:8:2] = v0[ 8:16:2]
  simd_vector_ref<int, 4, 2, decltype(r3)> r30 = r3.sel<4, 2>();  // r30 = r3[0:8:2] = v0[16:24:2]
  simd_vector_ref<int, 4, 2, decltype(r4)> r40 = r4.sel<4, 2>();  // r40 = r4[0:8:2] = v0[24:32:2]

  r10 += 199;
  r20 += 299;
  r30 += 399;
  r40 += 499;

  store(p, v0, offsets);
}

\endcode
*/

/** simd_vector_ref forward declaration */
template <typename T, int width, int stride, typename BaseTy>
class simd_vector_ref;

/**
 * The vector class.
 *
 * This class is a wrapper class for llvm vector values. Additionaly this class
 * supports region operations that map to GENX regions. The return type of a
 * region select is a simd_vector_ref type. This by-ref vector models
 * read-update-write semantics, which is the building block concept in the CM
 * language.
 */
template <typename T, int n> class simd_vector {
public:
  /**
   * The element type of this vector.
   */
  using element_type = T;

  /**
   * The vector type wrapper for llvm vector types.
   */
  using vector_type = typename details::simd_type<T, n>::type;

  /**
   * The number of elements in this vector.
   */
  static constexpr int length = n;

  template <typename V, int width, int stride, typename U>
  friend class simd_vector_ref;

  /**
   * Constructors.
   */
  inline simd_vector() { }
  inline simd_vector(const simd_vector<T, n> &v) { m_data = v.m_data; }
  inline simd_vector(const simd_vector<T, n> &&v) { m_data = v.m_data; }
  inline simd_vector(vector_type val) { m_data = val; }
  inline CM_NODEBUG simd_vector(T val, T stride = T()) {
    if (stride == T())
      m_data = val;
    else {
      #pragma unroll
      for (int i = 0; i < n; i++) {
        m_data[i] = val;
        val += stride;
      }
    }
  }

  /**
   * Convert to another vector type.
   */
  template <typename U>
  explicit inline operator simd_vector<U, n>() const {
    return read();
  }

  template <typename U>
  explicit inline operator typename details::simd_type<U, n>::type() const {
    return read();
  }

  /**
   * Assignment operators.
   */
  inline CM_NODEBUG simd_vector<T, n>& operator=(const simd_vector<T, n>& other) {
    m_data = other.m_data;
    return *this;
  }

  /**
   * Returns the underlying vector value.
   */
  inline vector_type data() const { return m_data; }

  /**
   * Select a region of elements from a vector.
   */
  template <int width, int stride>
  inline CM_NODEBUG simd_vector_ref<T, width, stride, simd_vector<T, n>>
  sel(int offset = 0) {
    simd_vector_ref<T, width, stride, simd_vector<T, n>> ref(*this, offset);
    return ref;
  }

  /**
   * Read a vector region.
   */
  template <int width, int stride>
  inline CM_NODEBUG simd_vector<T, width> read(int offset) const {
    using namespace details;
    return __cm_intrinsic_impl_rdregion<width, stride, T, n>(data(), offset);
  }

  /**
   * Read the entire region.
   */
  inline simd_vector<T, n> read() const { return *this; }

  /**
   * Update a vector region.
   */
  template <int width, int stride>
  inline CM_NODEBUG simd_vector<T, n> &write(simd_vector<T, width> input,
                                             int offset) {
    m_data = details::__cm_intrinsic_impl_wrregion<width, stride, T, n, width>(
        m_data, input.data(), offset);
    return *this;
  }

  /**
   * Update the entire region.
   */
  inline simd_vector<T, n> &write(simd_vector<T, n> input) {
    m_data = input.data();
    return *this;
  }

  /**
   * Read a single element.
   */
  inline CM_NODEBUG T operator[](int index) const {
    simd_vector<T, 1> val = read<1, 0>(index);
    return val.data()[0];
  }

private:
  /**
   * The undderlying data for this vector.
   */
  vector_type m_data;
};

/**
 * The vector reference class.
 *
 * This class represnts a region on a base object which could be a simd_vector
 * or another vector reference.
 */
template <typename T, int width, int stride, typename BaseTy>
class simd_vector_ref {
public:
  /**
   * The element type of this vector.
   */
  using element_type = T;

  /**
   * The vector type wrapper for llvm vector types
   */
  using vector_type = typename details::simd_type<T, width>::type;

  /**
   * The type of this class
   */
  using type = simd_vector_ref<T, width, stride, BaseTy>;

  /**
   * The number of elements in this vector.
   */
  static constexpr int length = width;

  template <typename U, int n> friend class simd_vector;

  /**
   * Constructors.
   */
  inline simd_vector_ref(BaseTy &base, int offset)
      : m_base(base), m_offset(offset) {}
  inline simd_vector_ref(const type &other)
      : m_base(other.m_base), m_offset(other.m_offset) {}
  inline simd_vector_ref(type &&other)
      : m_base(other.m_base), m_offset(other.m_offset) {}


  type& operator=(type&& other) = delete;
  type& operator=(const type& other) = delete;
  type& operator=(type other) = delete;

  /**
   * Select a region of elements from a vector.
   */
  inline CM_NODEBUG simd_vector<T, width> read() const {
    using namespace details;
    auto base = m_base.read();
    constexpr int n = BaseTy::length;
    return __cm_intrinsic_impl_rdregion<width, stride, T, n>(base.data(),
                                                             m_offset);
  }

  /**
   * Update a vector region.
   */
  template <int stride1, typename BaseTy1>
  inline CM_NODEBUG type &write(simd_vector_ref<T, width, stride1, BaseTy1> input) {
    using namespace details;
    simd_vector<T, width> inVec = input.read();
    auto base = m_base.read();
    constexpr int n = BaseTy::length;
    auto merged = __cm_intrinsic_impl_wrregion<width, stride, T, n, width>(
        base.data(), inVec.data(), m_offset);
    m_base.write(merged);
    return *this;
  }

  /**
   * Update a vector region.
   */
  inline CM_NODEBUG type &write(simd_vector<T, width> input) {
    using namespace details;
    auto base = m_base.read();
    constexpr int n = BaseTy::length;
    auto merged = __cm_intrinsic_impl_wrregion<width, stride, T, n, width>(
        base.data(), input.data(), m_offset);
    m_base.write(merged);
    return *this;
  }

  /**
   * Assignment operators.
   */
  inline CM_NODEBUG type &operator=(T val) {
    simd_vector<T, width> input = val;
    write(input);
    return *this;
  }
  inline CM_NODEBUG type& operator=(vector_type val) {
    simd_vector<T, width> input = val;
    write(input);
    return *this;
  }

  inline CM_NODEBUG type& operator+=(simd_vector<T, width> val) {
    simd_vector<T, width> base = read();
    write(val.data() + base.data());
    return *this;
  }

  inline CM_NODEBUG type& operator+=(T val) {
    simd_vector<T, width> base = read();
    write(simd_vector<T, width>(val).data() + base.data());
    return *this;
  }

  inline CM_NODEBUG type &operator=(const simd_vector<T, width> &val) {
    write(val);
    return *this;
  }

  /**
   * select a region of elements.
   */
  template <int width1, int stride1>
  inline CM_NODEBUG simd_vector_ref<T, width1, stride1, type> sel(int offset = 0) {
    simd_vector_ref<T, width1, stride1, type> ref(*this, offset);
    return ref;
  }

private:
  /**
   * The refenence to the base object.
   */
  BaseTy &m_base;

  /**
   * The element offset of this reference object relative to the base object.
   */
  int m_offset;
};

template <typename U, typename T, int n>
inline CM_NODEBUG simd_vector<U, n> convert(simd_vector<T, n> val) {
  return __builtin_convertvector(val.data(),
                                 typename details::simd_type<U, n>::type);
}

template <typename T, typename U, int n>
CM_NODEBUG inline auto operator+(simd_vector<T, n> lhs, simd_vector<U, n> rhs)
    -> simd_vector<decltype(T() + U()), n> {
  using R = decltype(T() + U());
  simd_vector<R, n> lhs_ = convert<R>(lhs);
  simd_vector<R, n> rhs_ = convert<R>(rhs);
  return lhs_.data() + rhs_.data();
}

template <typename T, typename U, int n>
CM_NODEBUG inline auto operator+(simd_vector<T, n> lhs, U rhs)
    -> simd_vector<decltype(T() + U()), n> {
  simd_vector<U, n> rhs_ = rhs;
  return lhs + rhs_;
}

template <typename T, int n>
static inline CM_NODEBUG simd_vector<T, n>
load(T* p, simd_vector<unsigned, n> offsets) {
  constexpr unsigned N = 8;
  static_assert(n % N == 0, "only multiple of 8 are supported.");
  simd_vector<T, n> result;

  #pragma unroll
  for (unsigned i = 0; i + N <= n; i += N) {
    simd_vector_ref<unsigned, N, 1, decltype(offsets)> ref = offsets.template sel<N, 1>(i);
    simd_vector<unsigned, N> offsets_i = ref.read();
    simd_vector<uint64_t, N> addrs(reinterpret_cast<uint64_t>(p));
    simd_vector<uint64_t, N> offsets_i1 = convert<uint64_t>(offsets_i);
    addrs = addrs + offsets_i1;
    simd_vector<T, N> Undef;
    result.template sel<N, 1>(i).write(
        details::__cm_intrinsic_impl_svm_read<T, N>(addrs.data(),
                                                    Undef.data()));
  }
  return result;
}

template <typename T, int n>
static inline CM_NODEBUG void store(T* p, simd_vector<T, n> vals,
                                    simd_vector<unsigned, n> offsets) {
  constexpr unsigned N = 8;
  static_assert(n % N == 0, "only multiple of 8 are supported.");
  #pragma unroll
  for (unsigned i = 0; i + N <= n; i += N) {
    simd_vector<unsigned, N> offsets_i = offsets.template sel<N, 1>(i).read();
    simd_vector<uint64_t, N> addrs(reinterpret_cast<uint64_t>(p));
    simd_vector<uint64_t, N> offsets_i1 = convert<uint64_t>(offsets_i);
    addrs = addrs + offsets_i1;
    details::__cm_intrinsic_impl_svm_write<T, N>(
        addrs.data(), vals.template sel<N, 1>(i).read().data());
  }
}

} // namespace experimental

#endif // _CLANG_CM_LIBRARY_
