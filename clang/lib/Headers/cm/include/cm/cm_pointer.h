/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_POINTER_H
#define _CLANG_CM_POINTER_H

#include "cm_common.h"
#include "cm_internal.h"
#include "cm_memory.h"
#include "cm_traits.h"

template <typename T0, int N>
CM_NODEBUG CM_INLINE void
cm_ptr_scatter_read(const T0 *const p, vector<ptrdiff_t, N> offset,
                    vector_ref<details::remove_address_space_t<T0>, N> dst) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  if constexpr (N == 1) {
    T0 *ptr = reinterpret_cast<T0 *>(base + offset(0));
    dst(0) = load(ptr);
  } else {
    vector<uintptr_t, N> vAddr = base + offset;
    vector<T0 *, N> vPtrs = reinterpret_cast<vector<T0 *, N> >(vAddr);
    dst = gather(vPtrs);
  }
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void
cm_ptr_scatter_read(const T0 *const p, matrix<ptrdiff_t, N, M> offset,
                    matrix_ref<details::remove_address_space_t<T0>, N, M> dst) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  if constexpr ((N == 1) && (M == 1)) {
    T0 *ptr = reinterpret_cast<T0 *>(base + offset(0, 0));
    dst(0, 0) = load(ptr);
  } else {
    matrix<uintptr_t, N, M> vAddr = base + offset;
    matrix<T0 *, N, M> vPtrs = reinterpret_cast<matrix<T0 *, N, M> >(vAddr);
    dst = gather(vPtrs);
  }
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void
cm_ptr_scatter_write(T0 const *p, vector<ptrdiff_t, N> offset,
                     vector<details::remove_address_space_t<T0>, N> src) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  if constexpr (N == 1) {
    T0 *ptr = reinterpret_cast<T0 *>(base + offset(0));
    store(src(0), ptr);
  } else {
    vector<uintptr_t, N> vAddr = base + offset;
    vector<T0 *, N> vPtrs = reinterpret_cast<vector<T0 *, N> >(vAddr);
    scatter(src, vPtrs);
  }
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void
cm_ptr_scatter_write(T0 *const p, matrix<ptrdiff_t, N, M> offset,
                     matrix<details::remove_address_space_t<T0>, N, M> src) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  if constexpr ((N == 1) && (M == 1)) {
    T0 *ptr = reinterpret_cast<T0 *>(base + offset(0, 0));
    store(src(0, 0), ptr);
  } else {
    matrix<uintptr_t, N, M> vAddr = base + offset;
    matrix<T0 *, N, M> vPtrs = reinterpret_cast<matrix<T0 *, N, M> >(vAddr);
    scatter(src, vPtrs);
  }
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void
cm_ptr_block_read(const T0 *const addr,
                  vector_ref<details::remove_address_space_t<T0>, N> dst) {
  using VTy = vector<details::remove_address_space_t<T0>, N>;
  const __generic VTy *const vPtr =
      reinterpret_cast<const __generic VTy *const>(addr);
  dst = load<VTy, Align::OWORD>(vPtr);
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_ptr_block_read_unaligned(
    const T0 *const addr,
    vector_ref<details::remove_address_space_t<T0>, N> dst) {
  using VTy = vector<details::remove_address_space_t<T0>, N>;
  const __generic VTy *const vPtr =
      reinterpret_cast<const __generic VTy *const>(addr);
  dst = load<VTy, Align::DWORD>(vPtr);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void
cm_ptr_block_read(const T0 *const addr,
                  matrix_ref<details::remove_address_space_t<T0>, N, M> dst) {
  using VMTy = matrix<details::remove_address_space_t<T0>, N, M>;
  const __generic VMTy *const vPtr =
      reinterpret_cast<const __generic VMTy *const>(addr);
  dst = load<VMTy, Align::OWORD>(vPtr);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_ptr_block_read_unaligned(
    const T0 *const addr,
    matrix_ref<details::remove_address_space_t<T0>, N, M> dst) {
  using VMTy = matrix<details::remove_address_space_t<T0>, N, M>;
  const __generic VMTy *const vPtr =
      reinterpret_cast<const __generic VMTy *const>(addr);
  dst = load<VMTy, Align::OWORD>(vPtr);
}

template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void
cm_ptr_block_write(T0 *const addr,
                   vector<details::remove_address_space_t<T0>, SZ> src) {
  using VTy = vector<details::remove_address_space_t<T0>, SZ>;
  __generic VTy *const vPtr = reinterpret_cast<__generic VTy *const>(addr);
  store(src, vPtr);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void
cm_ptr_block_write(T0 *const addr,
                   matrix<details::remove_address_space_t<T0>, N, M> src) {
  using VMTy = matrix<details::remove_address_space_t<T0>, N, M>;
  __generic VMTy *const vPtr = reinterpret_cast<__generic VMTy *const>(addr);
  store(src, vPtr);
}

#endif /* _CLANG_CM_POINTER_H */
