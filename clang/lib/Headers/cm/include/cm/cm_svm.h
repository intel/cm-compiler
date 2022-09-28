/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
CM_NODEBUG CM_INLINE void cm_ptr_block_read(const T0 *const addr,
                                            vector_ref<T0, N> dst) {
  uintptr_t _Addr = reinterpret_cast<uintptr_t>(addr);
  details::cm_svm_block_read_impl(_Addr, dst);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_ptr_block_read(const T0 *const addr,
                                            matrix_ref<T0, N, M> dst) {
  vector<T0, N * M> _Dst;
  cm_svm_block_read(addr, _Dst);
  dst = _Dst.format<T0, N, M>();
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void
cm_ptr_block_read_unaligned(const T0 *const addr, vector_ref<T0, N> dst) {
  uintptr_t _Addr = reinterpret_cast<uintptr_t>(addr);
  details::cm_svm_block_read_unaligned_impl(_Addr, dst);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void
cm_ptr_block_read_unaligned(const T0 *const addr, matrix_ref<T0, N, M> dst) {
  vector<T0, N * M> _Dst;
  cm_svm_block_read_unaligned(addr, _Dst);
  dst = _Dst.format<T0, N, M>();
}

template <typename T0, int SZ>
CM_NODEBUG CM_INLINE void cm_ptr_block_write(T0 *const addr,
                                             vector<T0, SZ> src) {
  uintptr_t _Addr = reinterpret_cast<uintptr_t>(addr);
  details::cm_svm_block_write_impl(_Addr, src);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_ptr_block_write(T0 *const addr,
                                             matrix<T0, N, M> src) {
  vector<T0, N * M> _Src = src;
  cm_svm_block_write(addr, _Src);
}

namespace details {

template <typename T0, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::isPowerOf2(N, 32), void>::type
cm_svm_scatter_read_impl(vector<uintptr_t, N> vAddr, vector_ref<T0, N> dst) {
  vector<uint64_t, N> _VAddr64(vAddr);
  if constexpr(sizeof(T0) == 1) {
    vector<T0, N*4> _Ret;
    _Ret =
      details::__cm_intrinsic_impl_svm_scatter_read<T0,N,4>(_VAddr64, _Ret);
    dst = _Ret.select<N, 4>(0);
  }
  else if constexpr(sizeof(T0) == 2) {
    vector<T0, N*2> _Ret;
    _Ret =
      details::__cm_intrinsic_impl_svm_scatter_read<T0,N,2>(_VAddr64, _Ret);
    dst = _Ret.select<N, 2>(0);
  }
  else {
    vector<T0, N> _Ret;
    _Ret =
      details::__cm_intrinsic_impl_svm_scatter_read<T0,N,1>(_VAddr64, _Ret);
    dst = _Ret;
  }
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::isPowerOf2(N, 32), void>::type
cm_svm_scatter_write_impl(vector<uintptr_t, N> vAddr, vector<T0, N> src) {
  vector<uint64_t, N> _VAddr64(vAddr);
  if constexpr(sizeof(T0) == 1) {
    vector<T0, N*4> _Data;
    _Data.select<N, 4>(0) = src;
    details::__cm_intrinsic_impl_svm_scatter_write<T0, N, 4>(_VAddr64, _Data);
  }
  else if constexpr(sizeof(T0) == 2) {
    vector<T0, N*2> _Data;
    _Data.select<N, 2>(0) = src;
    details::__cm_intrinsic_impl_svm_scatter_write<T0, N, 2>(_VAddr64, _Data);
  }
  else
    details::__cm_intrinsic_impl_svm_scatter_write<T0, N, 1>(_VAddr64, src);
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
CM_NODEBUG CM_INLINE void cm_ptr_scatter_read(const T0 * const p,
                                              vector<ptrdiff_t, N> offset,
                                              vector_ref<T0, N> dst) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  details::cm_svm_scatter_read_impl(vAddr, dst);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_ptr_scatter_read(const T0 * const p,
                                              matrix<ptrdiff_t, N, M> offset,
                                              matrix_ref<T0, N, M> dst) {
  vector<ptrdiff_t, N * M> _Offset = offset;
  vector<T0, N * M> _Ret;
  cm_svm_scatter_read(p, _Offset, _Ret);
  dst = _Ret.format<T0, N, M>();
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_ptr_scatter_write(T0 const *p,
                                               vector<ptrdiff_t, N> offset,
                                               vector<T0, N> src) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  details::cm_svm_scatter_write_impl(vAddr, src);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_ptr_scatter_write(T0 *const p,
                                               matrix<ptrdiff_t, N, M> offset,
                                               matrix<T0, N, M> src) {
  vector<ptrdiff_t, N * M> _Offset = offset;
  vector<T0, N * M> _Src = src;
  cm_svm_scatter_write(p, _Offset, _Src);
}

/// \brief ptr-base svm or stateless memmory read4.
///
/// Read 8, 16, or 32 4-element vectors, say {R,G,B,A}, where each element is
/// of size dword also referred to as a channel. The elements read from mem are
/// written back to the vector ::vDst and organized channel-wise, i.e. all R's
/// followed by all G's, and so on. Offset of each 4-element vector
/// must be specified in ::vOffset. Note that the offsets are byte-based.
/// One or more channels in the 4-element vector could be masked, and
/// ::vDst contains only the unmasked elements. Only the lower 4 bits of
/// ::mask specify the elements masked. A '1' implies that the corresponding
/// element of each vector will not be read from memory.
/// e.g. if mask = BR_ENABLE, ::vDst would contain
/// B7B6B5B4B3B2B1B0
/// R7R6R5R4R3R2R1R0
/// where the data elements correspond to the 8 vectors
/// (x B7 x R7), (x B6 x R6),... (x B0 x R0) read from memory at the offsets
/// specified in ::vOffset, where 'x' means the value is not read.
///
/// ::M must equal ::N * C where C is the number of channels enabled in ::mask.
/// note: 32 may not hardware native.
///
template <typename T, int N, int M>
typename std::enable_if<(N == 8 || N == 16 || N == 32) && (sizeof(T) == 4)>::type
cm_ptr_read4(const T* const ptr, vector<ptrdiff_t, N> vOffset,
             vector_ref<T, M> vDst,
             ChannelMaskType mask);

/// \brief pointer-based SVM or stateless memory write.
///
/// Write 8, 16, or 32 4-element vectors, say {R,G,B,A}, where each element is
/// of size dword and is also referred to as a channel. The elements to be
/// written must be in the vector ::vSrc and organized channel-wise,
/// i.e. all R's followed by all G's, and so on. Offset of each 4-element
/// vector must be specified in ::vOffset. Note that the offsets are in bytes.
/// One or more channels in the 4-element vector could be masked, and
/// ::vSrc must contain only the unmasked or enabled elements. The argument
/// ::mask specifies the channels that are enabled. Only the enabled channels
/// are written to memory. E.g. if mask = BR_ENABLE
/// (i.e. only R and B channels enabled), and vSrc is
/// B7B6B5B4B3B2B1B0
/// R7R6R5R4R3R2R1R0
/// 8 vectors written to memory are as
/// (x B7 x R7), (x B6 x R6), ... (x B0 x R0)
/// where 'x' means the value is not written.
///
/// M must equal N * C where C is the number of channels enabled in ::mask. T
/// must be of size dword (int, uint, or float). ::mask specifies
/// the channels that are enabled it has to be a compile-time constant of the
/// enum type ChannelMaskType.
template <typename T, int N, int M>
typename std::enable_if<(N == 8 || N == 16 || N == 32) && (sizeof(T) == 4)>::type
cm_ptr_write4(T* ptr, vector<ptrdiff_t, N> vOffset, vector<T, M> vSrc,
              ChannelMaskType mask);

// Defined here separate from other cm_svm_gather/scatter definitions
// due to dependency on 'cm_ptr_read/write4'
template <typename T, int N, int M>
typename std::enable_if<(N == 8 || N == 16 || N == 32) &&
                        (sizeof(T) == 4)>::type
cm_svm_gather4_scaled(vector<svmptr_t, N> vOffset, vector_ref<T, M> vDst,
                      ChannelMaskType mask) {
  vector<ptrdiff_t, N> _OffsetArg = vOffset;
  cm_ptr_read4<T, N, M>((uint64_t)0, _OffsetArg, vDst, mask);
}

template <typename T, int N, int M>
typename std::enable_if<(N == 8 || N == 16 || N == 32) &&
                        (sizeof(T) == 4)>::type
cm_svm_scatter4_scaled(vector<svmptr_t, N> vOffset, vector<T, M> vSrc,
                       ChannelMaskType mask) {
  vector<ptrdiff_t, N> _OffsetArg = vOffset;
  cm_ptr_write4<T, N, M>((uint64_t)0, _OffsetArg, vSrc, mask);
}

// svmptr_t interface
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
  if constexpr (!details::isPowerOf2(N, 32)) {
    CM_STATIC_ERROR((details::isPowerOf2(N, 32)),
                    "Execution size 1, 2, 4, 8, 16, 32 are supported");
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
  if constexpr (Op == ATOMIC_FMAX || Op == ATOMIC_FMIN || Op == ATOMIC_FADD ||
                Op == ATOMIC_FSUB) {
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
cm_ptr_atomic(T *const p, vector<U, N> offset, vector_ref<T, N> dst) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  dst = details::__cm_intrinsic_impl_svm_atomic<Op, T, N>(vAddr, dst);
}

template <CmAtomicOpType Op, typename T, int N, int M, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N * M, 0>(), void>::type
cm_ptr_atomic(T *const p, matrix<U, N, M> offset, matrix_ref<T, N, M> dst) {
  vector<U, N * M> _Offset = offset;
  vector_ref<T, N * M> _Dst = dst.format<T>();
  cm_svm_atomic<Op>(p, _Offset, _Dst);
}

template <CmAtomicOpType Op, typename T, int N, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N, 1>(), void>::type
cm_ptr_atomic(T *const p, vector<U, N> offset, vector_ref<T, N> dst,
           vector<T, N> src0) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  dst = details::__cm_intrinsic_impl_svm_atomic<Op, T, N>(vAddr, src0, dst);
}

template <CmAtomicOpType Op, typename T, int N, int M, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N * M, 1>(), void>::type
cm_ptr_atomic(T *const p, matrix<U, N, M> offset, matrix_ref<T, N, M> dst,
           matrix<T, N, M> src0) {
  vector<U, N * M> _Offset = offset;
  vector_ref<T, N * M> _Dst = dst.format<T>();
  vector<T, N * M> _Src0 = src0;
  cm_svm_atomic<Op>(p, _Offset, _Dst, _Src0);
}

template <CmAtomicOpType Op, typename T, int N, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N, 2>(), void>::type
cm_ptr_atomic(T *const p, vector<U, N> offset, vector_ref<T, N> dst,
           vector<T, N> src0, vector<T, N> src1) {
  uintptr_t base = reinterpret_cast<uintptr_t>(p);
  vector<uintptr_t, N> vAddr = base + offset;
  dst = details::__cm_intrinsic_impl_svm_atomic<Op, T, N>(vAddr, src0, src1, dst);
}

template <CmAtomicOpType Op, typename T, int N, int M, typename U>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::checkSVMAtomic<Op, T, N * M, 2>(), void>::type
cm_ptr_atomic(T *const p, matrix<U, N, M> offset, matrix_ref<T, N, M> dst,
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

/// \brief Shared local memory stateless read.
///
/// Load ::size bytes from memory address ::addr starting at ::offset to the
/// SLM buffer ::slmBuffer. ::size must be a multiple of 256.
///
template <typename T = void>
CM_INLINE void cm_slm_load(uint slmBuffer, svmptr_t addr, uint offset,
                           uint size) {
  vector<uint, 16> vOffset(__cm_init_seq);
  vOffset.select<8, 1>(8) = vOffset.select<8, 1>(0) + 8;

  uint numTotalBlocks = size / 256;
  uint numGroups = cm_linear_local_size();
  uint numBlocks = numTotalBlocks / numGroups;
  uint numLeftOver = numTotalBlocks % numGroups;
  numBlocks += (cm_linear_local_id() < numLeftOver) ? 1 : 0;

  // We just need numBlocks and numGroups
  uint elemSize = sizeof(float);
  uint threadOffsetInSLM = cm_linear_local_id() * 256;
  // in bytes
  uint threadOffsetInMemory = offset + threadOffsetInSLM;
  // in unit of elements
  vector<uint, 16> vOffsets = (threadOffsetInSLM / elemSize) + vOffset * 4;

  for (uint block = 0; block < numBlocks; block++) {
    vector<uint, 32> row0; // 32 floats or 128 Bytes or 4 GRF-registers
    vector<uint, 32> row1;
    vector<uint, 64> rowTrans;
    cm_svm_block_read(addr + threadOffsetInMemory, row0);
    cm_svm_block_read(addr + threadOffsetInMemory + 128, row1);

    // Transpose
    rowTrans.select<8, 1>(0) = row0.select<8, 4>(0);
    rowTrans.select<8, 1>(16) = row0.select<8, 4>(1);
    rowTrans.select<8, 1>(32) = row0.select<8, 4>(2);
    rowTrans.select<8, 1>(48) = row0.select<8, 4>(3);

    rowTrans.select<8, 1>(8) = row1.select<8, 4>(0);
    rowTrans.select<8, 1>(24) = row1.select<8, 4>(1);
    rowTrans.select<8, 1>(40) = row1.select<8, 4>(2);
    rowTrans.select<8, 1>(56) = row1.select<8, 4>(3);

    cm_slm_write4(slmBuffer, vOffsets, rowTrans, SLM_ABGR_ENABLE);
    threadOffsetInMemory += numGroups * 256;
    vOffsets += numGroups * 64;
  }

#if CM_GENX > 900
  cm_slm_fence(CM_GLOBAL_COHERENT_FENCE);
#endif
  cm_barrier();
}

#endif /* _CLANG_CM_SVM_H */
