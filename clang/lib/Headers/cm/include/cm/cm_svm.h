/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2023 Intel Corporation

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
#include "cm_memory.h"

// API for SVM scatter read and write.
template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_scatter_read(vector<svmptr_t, N> vAddr,
                                              vector_ref<T0, N> dst) {
  if constexpr (N == 1) {
    __global T0 *ptr = reinterpret_cast<__global T0 *>(vAddr(0));
    dst(0) = load(ptr);
  } else {
    vector<__global T0 *, N> vPtrs =
        reinterpret_cast<vector<__global T0 *, N> >(vAddr);
    dst = gather(vPtrs);
  }
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_scatter_read(matrix<svmptr_t, N, M> vAddr,
                                              matrix_ref<T0, N, M> dst) {
  if constexpr ((N == 1) && (M == 1)) {
    __global T0 *ptr = reinterpret_cast<__global T0 *>(vAddr(0, 0));
    dst(0, 0) = load(ptr);
  } else {
    matrix<__global T0 *, N, M> vPtrs =
        reinterpret_cast<matrix<__global T0 *, N, M> >(vAddr);
    dst = gather(vPtrs);
  }
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_scatter_write(vector<svmptr_t, N> vAddr,
                                               vector<T0, N> src) {
  if constexpr (N == 1) {
    __global T0 *ptr = reinterpret_cast<__global T0 *>(vAddr(0));
    store(src(0), ptr);
  } else {
    vector<__global T0 *, N> vPtrs =
        reinterpret_cast<vector<__global T0 *, N> >(vAddr);
    scatter(src, vPtrs);
  }
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_scatter_write(matrix<svmptr_t, N, M> vAddr,
                                               matrix<T0, N, M> src) {
  if constexpr ((N == 1) && (M == 1)) {
    __global T0 *ptr = reinterpret_cast<__global T0 *>(vAddr(0, 0));
    store(src(0, 0), ptr);
  } else {
    matrix<__global T0 *, N, M> vPtrs =
        reinterpret_cast<matrix<__global T0 *, N, M> >(vAddr);
    scatter(src, vPtrs);
  }
}

// API for SVM block read and write.
template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_block_read(svmptr_t addr,
                                            vector_ref<T0, N> dst) {
  const __global vector<T0, N> *const vPtr =
      reinterpret_cast<const __global vector<T0, N> *const>(addr);
  dst = load<vector<T0, N>, Align::OWORD>(vPtr);
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_block_read_unaligned(svmptr_t addr,
                                                      vector_ref<T0, N> dst) {
  const __global vector<T0, N> *const vPtr =
      reinterpret_cast<const __global vector<T0, N> *const>(addr);
  dst = load<vector<T0, N>, Align::DWORD>(vPtr);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_block_read(svmptr_t addr,
                                            matrix_ref<T0, N, M> dst) {
  const __global matrix<T0, N, M> *const vPtr =
      reinterpret_cast<const __global matrix<T0, N, M> *const>(addr);
  dst = load<matrix<T0, N, M>, Align::OWORD>(vPtr);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void
cm_svm_block_read_unaligned(svmptr_t addr, matrix_ref<T0, N, M> dst) {
  const __global matrix<T0, N, M> *const vPtr =
      reinterpret_cast<const __global matrix<T0, N, M> *const>(addr);
  dst = load<matrix<T0, N, M>, Align::DWORD>(vPtr);
}

template <typename T0, int N>
CM_NODEBUG CM_INLINE void cm_svm_block_write(svmptr_t addr, vector<T0, N> src) {
  __global vector<T0, N> *const vPtr =
      reinterpret_cast<__global vector<T0, N> *const>(addr);
  store(src, vPtr);
}

template <typename T0, int N, int M>
CM_NODEBUG CM_INLINE void cm_svm_block_write(svmptr_t addr,
                                             matrix<T0, N, M> src) {
  __global matrix<T0, N, M> *const vPtr =
      reinterpret_cast<__global matrix<T0, N, M> *const>(addr);
  store(src, vPtr);
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
CM_NODEBUG CM_INLINE
typename std::enable_if<(N == 8 || N == 16 || N == 32) &&
                        (sizeof(T) == 4)>::type
cm_svm_gather4_scaled(vector<svmptr_t, N> vOffset, vector_ref<T, M> vDst,
                      ChannelMaskType mask) {
  vector<ptrdiff_t, N> _OffsetArg = vOffset;
  // cm_ptr_read4 accepts 'mask' value with compile-time-constant type.
#define TMPL_CASE_PTR_READ4(chanmask)                                   \
  case CM_##chanmask##_ENABLE:                                          \
    cm_ptr_read4<T, N, M>(static_cast<T*>(nullptr), _OffsetArg, vDst,   \
                          CM_##chanmask##_ENABLE);                      \
    break;

  if constexpr (M == N) {
    // Single-channel cases
    switch(mask) {
      TMPL_CASE_PTR_READ4(R);
      TMPL_CASE_PTR_READ4(G);
      TMPL_CASE_PTR_READ4(B);
      TMPL_CASE_PTR_READ4(A);
    }
  }
  else if constexpr (M == N * 2) {
    // 2-channel cases
    switch(mask) {
      TMPL_CASE_PTR_READ4(GR);
      TMPL_CASE_PTR_READ4(BR);
      TMPL_CASE_PTR_READ4(BG);
      TMPL_CASE_PTR_READ4(AR);
      TMPL_CASE_PTR_READ4(AG);
      TMPL_CASE_PTR_READ4(AB);
    }
  }
  else if constexpr (M == N * 3) {
    // 3-channel cases
    switch(mask) {
      TMPL_CASE_PTR_READ4(BGR);
      TMPL_CASE_PTR_READ4(AGR);
      TMPL_CASE_PTR_READ4(ABR);
      TMPL_CASE_PTR_READ4(ABG);
    }
  }
  else if constexpr (M == N * 4) {
    // Only 4-channel case : ABGR
    cm_ptr_read4<T, N, M>(static_cast<T*>(nullptr), _OffsetArg, vDst,
                          CM_ABGR_ENABLE);
  }
#undef TMPL_CASE_PTR_READ4
}

template <typename T, int N, int M>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N == 8 || N == 16 || N == 32) &&
                        (sizeof(T) == 4)>::type
cm_svm_scatter4_scaled(vector<svmptr_t, N> vOffset, vector<T, M> vSrc,
                       ChannelMaskType mask) {
  vector<ptrdiff_t, N> _OffsetArg = vOffset;
  // cm_ptr_write4 accepts 'mask' value with compile-time-constant type.
#define TMPL_CASE_PTR_WRITE4(chanmask)                                  \
  case CM_##chanmask##_ENABLE:                                          \
    cm_ptr_write4<T, N, M>(static_cast<T*>(nullptr), _OffsetArg, vSrc,  \
                           CM_##chanmask##_ENABLE);                     \
    break;

  if constexpr (M == N) {
    // Single-channel cases
    switch(mask) {
      TMPL_CASE_PTR_WRITE4(R);
      TMPL_CASE_PTR_WRITE4(G);
      TMPL_CASE_PTR_WRITE4(B);
      TMPL_CASE_PTR_WRITE4(A);
    }
  }
  else if constexpr (M == N * 2) {
    // 2-channel cases
    switch(mask) {
      TMPL_CASE_PTR_WRITE4(GR);
      TMPL_CASE_PTR_WRITE4(BR);
      TMPL_CASE_PTR_WRITE4(BG);
      TMPL_CASE_PTR_WRITE4(AR);
      TMPL_CASE_PTR_WRITE4(AG);
      TMPL_CASE_PTR_WRITE4(AB);
    }
  }
  else if constexpr (M == N * 3) {
    // 3-channel cases
    switch(mask) {
      TMPL_CASE_PTR_WRITE4(BGR);
      TMPL_CASE_PTR_WRITE4(AGR);
      TMPL_CASE_PTR_WRITE4(ABR);
      TMPL_CASE_PTR_WRITE4(ABG);
    }
  }
  else if constexpr (M == N * 4) {
    // Only 4-channel case : ABGR
    cm_ptr_write4<T, N, M>(static_cast<T*>(nullptr), _OffsetArg, vSrc,
                           CM_ABGR_ENABLE);
  }

#undef TMPL_CASE_PTR_WRITE4
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
    if constexpr (!details::is_type<T, uint16_t, uint32_t, uint64_t>()) {
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

#endif /* _CLANG_CM_SVM_H */
