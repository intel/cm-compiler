/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_memory.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_MEMORY_H_
#define _CLANG_CM_MEMORY_H_

#include "cm_common.h"

// Alignment in number of bytes.
enum Align {
  ELEM_SIZE = 0,
  BYTE = 1,
  WORD = 2,
  DWORD = 4,
  QWORD = 8,
  OWORD = 16
};

namespace details {
template <typename T, int N, int A>
void __cm_builtin_impl_scatter(vector<T, N> src, vector<__generic T *, N> ptrs,
                               vector<ushort, N> mask);

template <typename T, int N, int A>
vector<T, N> __cm_builtin_impl_gather(vector<__generic T *, N> ptrs,
                                      vector<ushort, N> mask,
                                      vector<T, N> passthru);

template <typename T, int A>
void __cm_builtin_impl_store(T val, __generic T *const ptr);

template <typename T, int A>
T __cm_builtin_impl_load(__generic const T *const ptr);

} // namespace details

// ____________________________ Scatter API ____________________________
template <typename T, int N, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE void scatter(vector<T, N> src,
                                  vector<__generic T *, N> ptrs,
                                  vector<ushort, N> mask = 1) {
  details::__cm_builtin_impl_scatter<T, N, A>(src, ptrs, mask);
}

template <typename T, int N, int M, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE void scatter(matrix<T, N, M> src,
                                  matrix<__generic T *, N, M> ptrs,
                                  matrix<ushort, N, M> mask = 1) {
  vector<T, N * M> vSrc = src;
  vector<__generic T *, N * M> vPtrs = ptrs;
  vector<ushort, N * M> vMask = mask;
  details::__cm_builtin_impl_scatter<T, N * M, A>(vSrc, vPtrs, vMask);
}

// ____________________________ Gather API ____________________________
template <typename T, int N, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE vector<T, N> gather(vector<__generic T *, N> ptrs,
                                         vector<ushort, N> mask = 1) {
  vector<T, N> passthru;
  return details::__cm_builtin_impl_gather<T, N, A>(ptrs, mask, passthru);
}

template <typename T, int N, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE vector<T, N> gather(vector<__generic T *, N> ptrs,
                                         vector<ushort, N> mask,
                                         vector<T, N> passthru) {
  return details::__cm_builtin_impl_gather<T, N, A>(ptrs, mask, passthru);
}

template <typename T, int N, int M, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE matrix<T, N, M> gather(matrix<__generic T *, N, M> ptrs,
                                            matrix<ushort, N, M> mask = 1) {
  vector<__generic T *, N * M> vPtrs = ptrs;
  vector<ushort, N * M> vMask = mask;
  vector<T, N * M> vPassthru;
  return details::__cm_builtin_impl_gather<T, N * M, A>(vPtrs, vMask, vPassthru)
      .format<T, N, M>();
}

template <typename T, int N, int M, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE matrix<T, N, M> gather(matrix<__generic T *, N, M> ptrs,
                                            matrix<ushort, N, M> mask,
                                            matrix<T, N, M> passthru) {
  vector<__generic T *, N * M> vPtrs = ptrs;
  vector<ushort, N * M> vMask = mask;
  vector<T, N * M> vPassthru = passthru;
  return details::__cm_builtin_impl_gather<T, N * M, A>(vPtrs, vMask, vPassthru)
      .format<T, N, M>();
}

// ____________________________ Load/Store API ____________________________
template <typename T, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE void store(T val, __generic T *const ptr) {
  details::__cm_builtin_impl_store<T, A>(val, ptr);
}

template <typename T, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE T load(__generic const T *const ptr) {
  return details::__cm_builtin_impl_load<T, A>(ptr);
}

#endif /*_CLANG_CM_MEMORY_H_ */
