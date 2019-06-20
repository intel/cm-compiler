/*
 * Copyright (c) 2019, Intel Corporation
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

#include "cm_common.h"
#include "cm_internal.h"

namespace details {
template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void cm_svm_block_read_impl(uintptr_t addr,
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

template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void
cm_svm_block_read_unaligned_impl(uintptr_t addr, vector_ref<T0, SZ> dst) {
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

template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void cm_svm_block_write_impl(uintptr_t addr,
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

} // namespace details

// API for SVM block read and write

// svmptr_t interface.
template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void cm_svm_block_read(svmptr_t addr,
                                            vector_ref<T0, SZ> dst) {
  details::cm_svm_block_read_impl(uintptr_t(addr), dst);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_block_read(svmptr_t addr,
                                            matrix_ref<T0, N, M> dst) {
  vector<T0, N * M> _Dst;
  cm_svm_block_read(addr, _Dst);
  dst = _Dst.format<T0, N, M>();
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_block_read_unaligned(svmptr_t addr,
                                                      vector_ref<T0, N> dst) {
  details::cm_svm_block_read_unaligned_impl(uintptr_t(addr), dst);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void
cm_svm_block_read_unaligned(svmptr_t addr, matrix_ref<T0, N, M> dst) {
  vector<T0, N * M> _Dst;
  cm_svm_block_read_unaligned(addr, _Dst);
  dst = _Dst.format<T0, N, M>();
}

template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void cm_svm_block_write(svmptr_t addr,
                                             vector<T0, SZ> src) {
  details::cm_svm_block_write_impl(uintptr_t(addr), src);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_block_write(svmptr_t addr,
                                             matrix<T0, N, M> src) {
  vector<T0, N * M> _Src = src;
  cm_svm_block_write(addr, _Src);
}

// pointer interface.
template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_block_read(const T0 *const addr,
                                            vector_ref<T0, N> dst) {
  uintptr_t _Addr = reinterpret_cast<uintptr_t>(addr);
  details::cm_svm_block_read_impl(_Addr, dst);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_block_read(const T0 *const addr,
                                            matrix_ref<T0, N, M> dst) {
  vector<T0, N * M> _Dst;
  cm_svm_block_read(addr, _Dst);
  dst = _Dst.format<T0, N, M>();
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void
cm_svm_block_read_unaligned(const T0 *const addr, vector_ref<T0, N> dst) {
  uintptr_t _Addr = reinterpret_cast<uintptr_t>(addr);
  details::cm_svm_block_read_unaligned_impl(_Addr, dst);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void
cm_svm_block_read_unaligned(const T0 *const addr, matrix_ref<T0, N, M> dst) {
  vector<T0, N * M> _Dst;
  cm_svm_block_read_unaligned(addr, _Dst);
  dst = _Dst.format<T0, N, M>();
}

template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void cm_svm_block_write(T0 *const addr,
                                             vector<T0, SZ> src) {
  uintptr_t _Addr = reinterpret_cast<uintptr_t>(addr);
  details::cm_svm_block_write_impl(_Addr, src);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_block_write(T0 *const addr,
                                             matrix<T0, N, M> src) {
  vector<T0, N * M> _Src = src;
  cm_svm_block_write(addr, _Src);
}

namespace details {

template <typename T0, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::isPowerOf2(N, 16), void>::type
cm_svm_scatter_read_impl(vector<uintptr_t, N> vAddr, vector_ref<T0, N> dst) {
  vector<uint64_t, N> _VAddr64(vAddr);
  vector<T0, N> _Ret;
  _Ret = details::__cm_intrinsic_impl_svm_scatter_read<T0>(_VAddr64, _Ret);
  dst = _Ret;
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::isPowerOf2(N, 16), void>::type
cm_svm_scatter_write_impl(vector<uintptr_t, N> vAddr, vector<T0, N> src) {
  vector<uint64_t, N> _VAddr64(vAddr);
  details::__cm_intrinsic_impl_svm_scatter_write<T0>(_VAddr64, src);
}

} // namespace details

// API for SVM scatter read and write

// svmptr_t interface.
template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_scatter_read(vector<svmptr_t, N> vAddr,
                                              vector_ref<T0, N> dst) {
  details::cm_svm_scatter_read_impl(vector<uintptr_t, N>(vAddr), dst);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_scatter_read(matrix<svmptr_t, N, M> vAddr,
                                              matrix_ref<T0, N, M> dst) {
  vector<svmptr_t, N * M> _VAddr = vAddr;
  vector<T0, N * M> _Ret;
  cm_svm_scatter_read(_VAddr, _Ret);
  dst = _Ret.format<T0, N, M>();
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_scatter_write(vector<svmptr_t, N> vAddr,
                                               vector<T0, N> src) {
  details::cm_svm_scatter_write_impl(vector<uintptr_t, N>(vAddr), src);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_scatter_write(matrix<svmptr_t, N, M> vAddr,
                                               matrix<T0, N, M> src) {
  vector<svmptr_t, N * M> _VAddr = vAddr;
  vector<T0, N * M> _Src = src;
  cm_svm_scatter_write(_VAddr, _Src);
}

// pointer interface.
template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_scatter_read(const T0 const *p,
                                              vector<ptrdiff_t, N> offset,
                                              vector_ref<T0, N> dst) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  details::cm_svm_scatter_read_impl(vAddr, dst);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_scatter_read(const T0 const *p,
                                              matrix<ptrdiff_t, N, M> offset,
                                              matrix_ref<T0, N, M> dst) {
  vector<ptrdiff_t, N * M> _Offset = offset;
  vector<T0, N * M> _Ret;
  cm_svm_scatter_read(p, _Offset, _Ret);
  dst = _Ret.format<T0, N, M>();
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_scatter_write(T0 const *p,
                                               vector<ptrdiff_t, N> offset,
                                               vector<T0, N> src) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  details::cm_svm_scatter_write_impl(vAddr, src);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_scatter_write(T0 *const p,
                                               matrix<ptrdiff_t, N, M> offset,
                                               matrix<T0, N, M> src) {
  vector<ptrdiff_t, N * M> _Offset = offset;
  vector<T0, N * M> _Src = src;
  cm_svm_scatter_write(p, _Offset, _Src);
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

namespace details {

template <CmAtomicOpType Op, typename T, int N, unsigned NumSrc>
constexpr bool checkSVMAtomic() {
  if constexpr (!details::isPowerOf2(N, 8)) {
    CM_STATIC_ERROR((details::isPowerOf2(N, 8)),
                    "Execution size 1, 2, 4, 8 are supported");
    return false;
  }

  // No source operand.
  if constexpr (Op == ATOMIC_INC || Op == ATOMIC_DEC) {
    if constexpr (NumSrc != 0) {
      CM_STATIC_ERROR(NumSrc == 0, "No source operands are expected");
      return false;
    }
    if constexpr (details::is_type<T, uint16_t, uint32_t, uint64_t>()) {
      CM_STATIC_ERROR((details::is_type<T, uint16_t, uint32_t, uint64_t>()),
                      "Type UW, UD or UQ is expected");
      return false;
    }
    return true;
  }

  // One source integer operand.
  if constexpr (Op == ATOMIC_ADD || Op == ATOMIC_SUB || Op == ATOMIC_MIN ||
                Op == ATOMIC_MAX || Op == ATOMIC_XCHG || Op == ATOMIC_AND ||
                Op == ATOMIC_OR || Op == ATOMIC_XOR || Op == ATOMIC_MINSINT ||
                Op == ATOMIC_MAXSINT) {
    if constexpr (NumSrc != 1) {
      CM_STATIC_ERROR(NumSrc == 1, "One source operand is expected");
      return false;
    }
    if constexpr ((Op != ATOMIC_MINSINT && Op != ATOMIC_MAXSINT) &&
                  !details::is_type<T, uint16_t, uint32_t, uint64_t>()) {
      CM_STATIC_ERROR((details::is_type<T, uint16_t, uint32_t, uint64_t>()),
                      "Type UW, UD or UQ is expected");
      return false;
    }
    if constexpr ((Op == ATOMIC_MINSINT || Op == ATOMIC_MAXSINT) &&
                  !details::is_type<T, int16_t, int32_t, int64_t>()) {
      CM_STATIC_ERROR((details::is_type<T, int16_t, int32_t, int64_t>()),
                      "Type W, D or Q is expected");
      return false;
    }
    return true;
  }

  // One source float operand.
  if constexpr (Op == ATOMIC_FMAX || Op == ATOMIC_FMIN) {
    if constexpr (NumSrc != 1) {
      CM_STATIC_ERROR(NumSrc == 1, "One source operand is expected");
      return false;
    }
    if constexpr (!details::is_type<T, float, half>()) {
      CM_STATIC_ERROR((details::is_type<T, float, half>()),
                      "Type F or HF is expected");
      return false;
    }
    return true;
  }

  // Two scouce operands.
  if constexpr (Op == ATOMIC_CMPXCHG || Op == ATOMIC_FCMPWR) {
    if constexpr (NumSrc != 2) {
      CM_STATIC_ERROR(NumSrc == 2, "Two source operands are expected");
      return false;
    }
    if constexpr (Op == ATOMIC_CMPXCHG &&
                  !details::is_type<T, uint16_t, uint32_t, uint64_t>()) {
      CM_STATIC_ERROR((details::is_type<T, uint16_t, uint32_t, uint64_t>()),
                      "Type UW, UD or UQ is expected");
      return false;
    }
    if constexpr (Op == ATOMIC_FCMPWR && !details::is_type<T, float, half>()) {
      CM_STATIC_ERROR((details::is_type<T, float, half>()),
                      "Type F or HF is expected");
      return false;
    }
    return true;
  }

  // Unsupported svm atomic Op.
  return false;
}

} // namespace details

// pointer interface
template <CmAtomicOpType Op, typename T, int N, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N, 0>(), void>::type
svm_atomic(T *const p, vector<U, N> offset, vector_ref<T, N> dst) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  dst = details::__cm_intrinsic_impl_svm_atomic<Op, T, N>(vAddr, dst);
}

template <CmAtomicOpType Op, typename T, int N, int M, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N * M, 0>(), void>::type
svm_atomic(T *const p, matrix<U, N, M> offset, matrix_ref<T, N, M> dst) {
  vector<U, N * M> _Offset = offset;
  vector_ref<T, N * M> _Dst = dst.format<T>();
  cm_svm_atomic<Op>(p, _Offset, _Dst);
}

template <CmAtomicOpType Op, typename T, int N, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N, 1>(), void>::type
svm_atomic(T *const p, vector<U, N> offset, vector_ref<T, N> dst,
           vector<T, N> src0) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  dst = details::__cm_intrinsic_impl_svm_atomic<Op, T, N>(vAddr, src0, dst);
}

template <CmAtomicOpType Op, typename T, int N, int M, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N * M, 1>(), void>::type
svm_atomic(T *const p, matrix<U, N, M> offset, matrix_ref<T, N, M> dst,
           matrix<T, N, M> src0) {
  vector<U, N * M> _Offset = offset;
  vector_ref<T, N * M> _Dst = dst.format<T>();
  vector<T, N * M> _Src0 = src0;
  cm_svm_atomic<Op>(p, _Offset, _Dst, _Src0);
}

template <CmAtomicOpType Op, typename T, int N, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N, 2>(), void>::type
svm_atomic(T *const p, vector<U, N> offset, vector_ref<T, N> dst,
           vector<T, N> src0, vector<T, N> src1) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  dst = details::__cm_intrinsic_impl_svm_atomic<Op, T, N>(vAddr, src0, src1, dst);
}

template <CmAtomicOpType Op, typename T, int N, int M, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N * M, 2>(), void>::type
svm_atomic(T *const p, matrix<U, N, M> offset, matrix_ref<T, N, M> dst,
           matrix<T, N, M> src0, matrix<T, N, M> src1) {
  vector<U, N * M> _Offset = offset;
  vector_ref<T, N * M> _Dst = dst.format<T>();
  vector<T, N * M> _Src0 = src0;
  vector<T, N * M> _Src1 = src1;
  cm_svm_atomic<Op>(p, _Offset, _Dst, _Src0, _Src1);
}

// svmptr interface
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N, 0>(), void>::type
svm_atomic(vector<svmptr_t, N> vAddr, vector_ref<T, N> dst) {
  dst = details::__cm_intrinsic_impl_svm_atomic<Op, T, N>(vAddr, dst);
}

template <CmAtomicOpType Op, typename T, int N, int M>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N * M, 0>(), void>::type
svm_atomic(matrix<svmptr_t, N, M> vAddr, matrix_ref<T, N, M> dst) {
  vector<svmptr_t, N * M> _VAddr = vAddr;
  vector_ref<T, N * M> _Dst = dst.format<T>();
  svm_atomic<Op, T, N * M>(_VAddr, _Dst);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N, 1>(), void>::type
svm_atomic(vector<svmptr_t, N> vAddr, vector_ref<T, N> dst, vector<T, N> src0) {
  dst = details::__cm_intrinsic_impl_svm_atomic<Op, T, N>(vAddr, src0, dst);
}

template <CmAtomicOpType Op, typename T, int N, int M>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N * M, 1>(), void>::type
svm_atomic(matrix<svmptr_t, N, M> vAddr, matrix_ref<T, N, M> dst,
           matrix<T, N, M> src0) {
  vector<svmptr_t, N * M> _VAddr = vAddr;
  vector_ref<T, N * M> _Dst = dst.format<T>();
  vector<T, N * M> _Src0 = src0;
  svm_atomic<Op, T, N * M>(_VAddr, _Dst, _Src0);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N, 2>(), void>::type
svm_atomic(vector<svmptr_t, N> vAddr, vector_ref<T, N> dst, vector<T, N> src0,
           vector<T, N> src1) {
  dst = details::__cm_intrinsic_impl_svm_atomic<Op, T, N>(vAddr, src0, src1, dst);
}

template <CmAtomicOpType Op, typename T, int N, int M>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N * M, 2>(), void>::type
svm_atomic(matrix<svmptr_t, N, M> vAddr, matrix_ref<T, N, M> dst,
           matrix<T, N, M> src0, matrix<T, N, M> src1) {
  vector<svmptr_t, N * M> _VAddr = vAddr;
  vector_ref<T, N * M> _Dst = dst.format<T>();
  vector<T, N * M> _Src0 = src0;
  vector<T, N * M> _Src1 = src1;
  svm_atomic<Op, T, N * M>(_VAddr, _Dst, _Src0, _Src1);
}

#endif /* _CLANG_CM_SVM_H */
