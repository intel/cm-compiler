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

template <DataSize DS> constexpr Align data_size_alignment() {
  if constexpr (DS == DataSize::U8)
    return Align::BYTE;
  else if constexpr (DS == DataSize::U16)
    return Align::WORD;
  else if constexpr (DS == DataSize::U32)
    return Align::DWORD;
  else if constexpr (DS == DataSize::U64)
    return Align::QWORD;
  return Align::ELEM_SIZE;
}

namespace details {
template <typename T, int N, int A>
void __cm_builtin_impl_scatter(vector<T, N> src, vector<__generic T *, N> ptrs,
                               vector<ushort, N> mask);
template <typename T, int N, int A>
void __cm_builtin_impl_scatter(vector<T, N> src, vector<__global T *, N> ptrs,
                               vector<ushort, N> mask);
template <typename T, int N, int A>
void __cm_builtin_impl_scatter(vector<T, N> src, vector<__local T *, N> ptrs,
                               vector<ushort, N> mask);
template <typename T, int N, int A>
void __cm_builtin_impl_scatter(vector<T, N> src, vector<__private T *, N> ptrs,
                               vector<ushort, N> mask);
template <typename T, int N, int A>
void __cm_builtin_impl_scatter(vector<T, N> src, vector<T *, N> ptrs,
                               vector<ushort, N> mask);

template <typename T, int N, int A>
vector<T, N> __cm_builtin_impl_gather(vector<__generic T *, N> ptrs,
                                      vector<ushort, N> mask,
                                      vector<T, N> passthru);
template <typename T, int N, int A>
vector<T, N> __cm_builtin_impl_gather(vector<__global T *, N> ptrs,
                                      vector<ushort, N> mask,
                                      vector<T, N> passthru);
template <typename T, int N, int A>
vector<T, N> __cm_builtin_impl_gather(vector<__local T *, N> ptrs,
                                      vector<ushort, N> mask,
                                      vector<T, N> passthru);
template <typename T, int N, int A>
vector<T, N> __cm_builtin_impl_gather(vector<__constant T *, N> ptrs,
                                      vector<ushort, N> mask,
                                      vector<T, N> passthru);
template <typename T, int N, int A>
vector<T, N> __cm_builtin_impl_gather(vector<__private T *, N> ptrs,
                                      vector<ushort, N> mask,
                                      vector<T, N> passthru);
template <typename T, int N, int A>
vector<T, N> __cm_builtin_impl_gather(vector<T *, N> ptrs,
                                      vector<ushort, N> mask,
                                      vector<T, N> passthru);

template <typename T, int A>
void __cm_builtin_impl_store(T val, __generic T *const ptr);

template <typename T, int A>
T __cm_builtin_impl_load(__generic const T *const ptr);

template <typename T, int A>
T __cm_builtin_impl_load(__constant const T *const ptr);
} // namespace details

// ____________________________ Scatter API ________________________________
#define _CM_SCATTER_WITH_AS_DEFS(_AS)                                       \
  template <typename T, int N, int A = Align::ELEM_SIZE>                    \
  CM_NODEBUG CM_INLINE typename std::enable_if<N != 1>::type                \
  scatter(vector<T, N> src, vector<_AS T *, N> ptrs,                        \
          vector<ushort, N> mask = 1) {                                     \
    details::__cm_builtin_impl_scatter<T, N, A>(src, ptrs, mask);           \
  }                                                                         \
  template <typename T, int N, int M, int A = Align::ELEM_SIZE>             \
  CM_NODEBUG CM_INLINE typename std::enable_if<(N != 1) || (M != 1)>::type  \
  scatter(matrix<T, N, M> src, matrix<_AS T *, N, M> ptrs,                  \
          matrix<ushort, N, M> mask = 1) {                                  \
    vector<T, N * M> vSrc = src;                                             \
    vector<_AS T *, N * M> vPtrs = ptrs;                                     \
    vector<ushort, N * M> vMask = mask;                                      \
    details::__cm_builtin_impl_scatter<T, N * M, A>(vSrc, vPtrs, vMask);    \
  }

_CM_SCATTER_WITH_AS_DEFS()
_CM_SCATTER_WITH_AS_DEFS(__private)
_CM_SCATTER_WITH_AS_DEFS(__global)
_CM_SCATTER_WITH_AS_DEFS(__local)
_CM_SCATTER_WITH_AS_DEFS(__generic)

#undef _CM_SCATTER_WITH_AS_DEF
// ________________________________ Gather API ________________________________
#define _CM_GATHER_WITH_AS_DEFS(_AS)                                           \
  template <typename T, int N, int A = Align::ELEM_SIZE>                       \
  CM_NODEBUG CM_INLINE typename std::enable_if<N != 1, vector<T, N> >::type    \
  gather(vector<_AS T *, N> ptrs, vector<ushort, N> mask = 1) {                \
    vector<T, N> passthru;                                                     \
    return details::__cm_builtin_impl_gather<T, N, A>(ptrs, mask, passthru);   \
  }                                                                            \
  template <typename T, int N, int A = Align::ELEM_SIZE>                       \
  CM_NODEBUG CM_INLINE typename std::enable_if<N != 1, vector<T, N> >::type    \
  gather(vector<_AS T *, N> ptrs, vector<ushort, N> mask,                      \
         vector<T, N> passthru) {                                              \
    return details::__cm_builtin_impl_gather<T, N, A>(ptrs, mask, passthru);   \
  }                                                                            \
  template <typename T, int N, int M, int A = Align::ELEM_SIZE>                \
  CM_NODEBUG CM_INLINE                                                         \
      typename std::enable_if<(N != 1) || (M != 1), matrix<T, N, M> >::type    \
  gather(matrix<_AS T *, N, M> ptrs, matrix<ushort, N, M> mask = 1) {          \
    vector<_AS T *, N * M> vPtrs = ptrs;                                       \
    vector<ushort, N * M> vMask = mask;                                        \
    vector<T, N * M> vPassthru;                                                \
    return details::__cm_builtin_impl_gather<T, N * M, A>(vPtrs, vMask,        \
                                                          vPassthru)           \
        .format<T, N, M>();                                                    \
  }                                                                            \
  template <typename T, int N, int M, int A = Align::ELEM_SIZE>                \
  CM_NODEBUG CM_INLINE                                                         \
      typename std::enable_if<(N != 1) || (M != 1), matrix<T, N, M> >::type    \ 
  gather(matrix<_AS T *, N, M> ptrs, matrix<ushort, N, M> mask,                \
         matrix<T, N, M> passthru) {                                           \
    vector<_AS T *, N * M> vPtrs = ptrs;                                       \
    vector<ushort, N * M> vMask = mask;                                        \
    vector<T, N * M> vPassthru = passthru;                                     \
    return details::__cm_builtin_impl_gather<T, N * M, A>(vPtrs, vMask,        \
                                                          vPassthru)           \
        .format<T, N, M>();                                                    \
  }

_CM_GATHER_WITH_AS_DEFS()
_CM_GATHER_WITH_AS_DEFS(__private)
_CM_GATHER_WITH_AS_DEFS(__global)
_CM_GATHER_WITH_AS_DEFS(__constant)
_CM_GATHER_WITH_AS_DEFS(__local)
_CM_GATHER_WITH_AS_DEFS(__generic)

#undef _CM_GATHER_WITH_AS_DEFS
// ____________________________ Load/Store API ____________________________
template <typename T, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE void store(T val, __generic T *const ptr) {
  details::__cm_builtin_impl_store<T, A>(val, ptr);
}

template <typename T, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE T load(__generic const T *const ptr) {
  return details::__cm_builtin_impl_load<T, A>(ptr);
}

template <typename T, int A = Align::ELEM_SIZE>
CM_NODEBUG CM_INLINE T load(__constant const T *const ptr) {
  return details::__cm_builtin_impl_load<T, A>(ptr);
}

#endif /*_CLANG_CM_MEMORY_H_ */
