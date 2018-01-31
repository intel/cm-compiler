/*
 * Copyright (c) 2018, Intel Corporation
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
static_assert(0, "CM:w:cm_svm.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_SVM_H
#define _CLANG_CM_SVM_H

// API for SVM access

template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void cm_svm_block_read(svmptr_t addr,
                                            vector_ref<T0, SZ> dst) {
  constexpr unsigned _Sz = sizeof(T0) * SZ;
  uint64_t _Addr = addr;

  CM_STATIC_ERROR(_Sz >= details::OWORD, "block size must be at least 1 oword");
  CM_STATIC_ERROR(_Sz % details::OWORD == 0,
                  "block size must be whole number of owords");
  CM_STATIC_ERROR(details::isPowerOf2(_Sz / details::OWORD),
                  "block must be 1, 2, 4 or 8 owords long");
  CM_STATIC_ERROR(_Sz <= 8 * details::OWORD,
                  "block size must be at most 8 owords");

  dst = details::__cm_intrinsic_impl_svm_block_read<T0, SZ>(_Addr);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_block_read(svmptr_t addr,
                                            matrix_ref<T0, N, M> dst) {
  vector<T0, M *M> _Dst;

  cm_svm_block_read(addr, _Dst);
  dst = _Dst.format<T0, N, M>();
}

template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void cm_svm_block_read_unaligned(svmptr_t addr,
                                                      vector_ref<T0, SZ> dst) {
  constexpr unsigned _Sz = sizeof(T0) * SZ;
  uint64_t _Addr = addr;

  CM_STATIC_ERROR(_Sz >= details::OWORD, "block size must be at least 1 oword");
  CM_STATIC_ERROR(_Sz % details::OWORD == 0,
                  "block size must be whole number of owords");
  CM_STATIC_ERROR(details::isPowerOf2(_Sz / details::OWORD),
                  "block must be 1, 2, 4 or 8 owords long");
  CM_STATIC_ERROR(_Sz <= 8 * details::OWORD,
                  "block size must be at most 8 owords");

  dst = details::__cm_intrinsic_impl_svm_block_read_unaligned<T0, SZ>(_Addr);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void
cm_svm_block_read_unaligned(svmptr_t addr, matrix_ref<T0, N, M> dst) {
  vector<T0, M *M> _Dst;

  cm_svm_block_read_unaligned(addr, _Dst);
  dst = _Dst.format<T0, N, M>();
}

template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void cm_svm_block_write(svmptr_t addr,
                                             vector<T0, SZ> src) {
  constexpr unsigned _Sz = sizeof(T0) * SZ;
  uint64_t _Addr = addr;

  CM_STATIC_ERROR(_Sz >= details::OWORD, "block size must be at least 1 oword");
  CM_STATIC_ERROR(_Sz % details::OWORD == 0,
                  "block size must be whole number of owords");
  CM_STATIC_ERROR(details::isPowerOf2(_Sz / details::OWORD),
                  "block must be 1, 2, 4 or 8 owords long");
  CM_STATIC_ERROR(_Sz <= 8 * details::OWORD,
                  "block size must be at most 8 owords");

  details::__cm_intrinsic_impl_svm_block_write<T0>(_Addr, src);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_block_write(svmptr_t addr,
                                             matrix<T0, N, M> src) {
  vector<T0, M *M> _Src = src;

  cm_svm_block_write(addr, _Src);
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::isPowerOf2(N, 16), void>::type
cm_svm_scatter_read(vector<svmptr_t, N> vAddr,
                    vector_ref<T0, N> dst) {
  vector<uint64_t, N> _VAddr64(vAddr);
  vector<T0, N> _Ret;

  _Ret = details::__cm_intrinsic_impl_svm_scatter_read<T0>(_VAddr64, _Ret);
  dst = _Ret;
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_scatter_read(matrix<svmptr_t, N, M> vAddr,
                                              matrix_ref<T0, N, M> dst) {
  vector<svmptr_t, N *M> _VAddr = vAddr;
  vector<T0, N *M> _Ret;

  cm_svm_scatter_read(_VAddr, _Ret);

  dst = _Ret.format<T0, N, M>();
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::isPowerOf2(N, 16), void>::type
cm_svm_scatter_write(vector<svmptr_t, N> vAddr,
                     vector<T0, N> src) {
  vector<uint64_t, N> _VAddr64(vAddr);

  details::__cm_intrinsic_impl_svm_scatter_write<T0>(_VAddr64, src);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_scatter_write(matrix<svmptr_t, N, M> vAddr,
                                               matrix<T0, N, M> src) {
  vector<svmptr_t, N *M> _VAddr = vAddr;
  vector<T0, N *M> _Src = src;

  cm_svm_scatter_write(_VAddr, _Src);
}

template <typename T0, int SZ>
void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, SZ> vAddr,
                   vector_ref<T0, SZ> dst, vector<T0, SZ> src0,
                   vector<T0, SZ> src1);

template <typename T0, int N, int M>
void cm_svm_atomic(CmAtomicOpType op, matrix<svmptr_t, N, M> vAddr,
                   matrix_ref<T0, N, M> dst, matrix<T0, N, M> src0,
                   matrix<T0, N, M> src1);

template <typename T0, int SZ>
void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, SZ> vAddr,
                   vector_ref<T0, SZ> dst, vector<T0, SZ> src0);

template <typename T0, int N, int M>
void cm_svm_atomic(CmAtomicOpType op, matrix<svmptr_t, N, M> vAddr,
                   matrix_ref<T0, N, M> dst, matrix<T0, N, M> src0);

template <typename T0, int SZ>
void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, SZ> vAddr,
                   vector_ref<T0, SZ> dst);

template <typename T0, int N, int M>
void cm_svm_atomic(CmAtomicOpType op, matrix<svmptr_t, N, M> vAddr,
                   matrix_ref<T0, N, M> dst);

#endif /* _CLANG_CM_GATEWAY_H */
