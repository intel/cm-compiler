/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_SLM_H
#define _CLANG_CM_SLM_H

#include "cm_common.h"
#include "cm_internal.h"
#include "cm_memory.h"

// Implementation of block-load.
template <typename T, int N, Align A>
CM_NODEBUG CM_INLINE auto cm_load_slm(unsigned Offset) {
  const __local vector<T, N> *const vPtr =
      reinterpret_cast<const __local vector<T, N> *const>(Offset);
  return load<vector<T, N>, A>(vPtr);
}

// Block-load interface with optional datasize.
template <typename T, int NElts, DataSize DS = DataSize::Default>
CM_NODEBUG CM_INLINE auto cm_load_slm(unsigned Offset) {
  constexpr DataSize _DS = details::lsc_data_size<T, DS>();
  constexpr Align _Align = data_size_alignment<DS>();
  return cm_load_slm<T, NElts, _Align>(Offset);
}

// Deprecated block-load interface with optional datasize.
template <typename T, VectorSize VS, DataSize DS = DataSize::Default>
CM_NODEBUG CM_INLINE auto cm_load_slm(unsigned Offset) {
  CM_STATIC_WARNING(details::always_false<decltype(VS)>(),
                    "Please use new interface with explicit NElts");
  constexpr DataSize _DS = details::lsc_data_size<T, DS>();
  constexpr unsigned _NElts = details::lsc_vector_size<VS>();
  return cm_load_slm<T, _NElts, _DS>(Offset);
}

// Block-load interface with attr which indicates the offset alignment.
// The attr should be one of ::GENX_NONE ::GENX_DWALIGNED
// ::GENX_MODIFIED_DWALIGNED ::GENX_CONSTANT_DWALIGNED.
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_block_read(uint slmBuffer, CmBufferAttrib attr,
                                            int offset,
                                            vector_ref<T, N> output) {
  if (attr == GENX_DWALIGNED || attr == GENX_MODIFIED_DWALIGNED ||
      attr == GENX_CONSTANT_DWALIGNED) {
    output = cm_load_slm<T, N, Align::DWORD>(slmBuffer + offset);
    return;
  }
  output = cm_load_slm<T, N, Align::OWORD>(slmBuffer + offset);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_block_read(uint slmBuffer, int X,
                                            vector_ref<T, N> src) {
  cm_slm_block_read(slmBuffer, GENX_NONE, X, src);
}

// Implementation of block-store.
template <typename T, int N, Align A>
CM_NODEBUG CM_INLINE void cm_store_slm(unsigned Offset, vector<T, N> Data) {
  __local vector<T, N> *const vPtr =
      reinterpret_cast<__local vector<T, N> *const>(Offset);
  return store<vector<T, N>, A>(Data, vPtr);
}

// Block-store interface with optional datasize.
template <typename T, int NElts, DataSize DS = DataSize::Default>
CM_NODEBUG CM_INLINE void cm_store_slm(unsigned Offset, vector<T, NElts> Data) {
  constexpr DataSize _DS = details::lsc_data_size<T, DS>();
  constexpr Align _Align = data_size_alignment<DS>();
  return cm_store_slm<T, NElts, _Align>(Offset, Data);
}

// Block-store with oword alignment.
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_block_write(uint slmBuffer, int offset,
                                             vector<T, N> src) {
  cm_store_slm<T, N, Align::OWORD>(slmBuffer + offset, src);
}

// Gathering read implementation. Offsets are in bytes.
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_read_scaled(uint slmBuffer,
                                             vector<uint, N> vAddr,
                                             vector_ref<T, N> vDst) {
  if constexpr (N == 1) {
    __local T *ptr = reinterpret_cast<__local T *>(slmBuffer + vAddr(0));
    vDst(0) = load(ptr);
  } else {
    vector<__local T *, N> vPtrs =
        reinterpret_cast<vector<__local T *, N> >(slmBuffer + vAddr);
    vDst = gather(vPtrs);
  }
}

// Deprecated gathering read. Offsets are in bytes.
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_read_scaled(uint slmBuffer,
                                             vector<ushort, N> vAddr,
                                             vector_ref<T, N> vDst)
    CM_DEPRECATED("Please use 'cm_slm_read_scaled' with 'uint' as the element "
                  "offset type!") {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_read_scaled(slmBuffer, _VAddr, vDst);
}

// Gathering read. Offsets are in element size.
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_read(uint slmBuffer, vector<uint, N> vAddr,
                                      vector_ref<T, N> vDst) {
  vAddr *= sizeof(T);
  cm_slm_read_scaled(slmBuffer, vAddr, vDst);
}

// Deprecated gathering read. Offsets are in element size.
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_read(uint slmBuffer, vector<ushort, N> vAddr,
                                      vector_ref<T, N> vDst)
    CM_DEPRECATED("Please use 'cm_slm_read_scaled' with 'uint' as the element "
                  "offset type!") {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_read(slmBuffer, _VAddr, vDst);
}

// Scattering write implementation. Offsets are in bytes.
template <typename T, int N>
CM_NODEBUG CM_INLINE void
cm_slm_write_scaled(uint slmBuffer, vector<uint, N> vAddr, vector<T, N> vSrc) {
  if constexpr (N == 1) {
    __local T *ptr = reinterpret_cast<__local T *>(slmBuffer + vAddr(0));
    store(vSrc(0), ptr);
  } else {
    vector<__local T *, N> vPtrs =
        reinterpret_cast<vector<__local T *, N> >(slmBuffer + vAddr);
    scatter(vSrc, vPtrs);
  }
}

// Deprecated scattering write. Offsets are in bytes.
template <typename T, int N>
CM_NODEBUG CM_INLINE void
cm_slm_write_scaled(uint slmBuffer, vector<ushort, N> vAddr, vector<T, N> vSrc)
    CM_DEPRECATED("Please use 'cm_slm_write_scaled' with 'uint' as the element "
                  "offset type!") {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_write_scaled(slmBuffer, _VAddr, vSrc);
}

// Scattering write. Offsets are in element size.
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_write(uint slmBuffer, vector<uint, N> vAddr,
                                       vector<T, N> vSrc) {
  vAddr *= sizeof(T);
  cm_slm_write_scaled(slmBuffer, vAddr, vSrc);
}

// Deprecated scattering write. Offsets are in element size.
template <typename T, int N>
CM_NODEBUG CM_INLINE void cm_slm_write(uint slmBuffer, vector<ushort, N> vAddr,
                                       vector<T, N> vSrc)
    CM_DEPRECATED("Please use 'cm_slm_write_scaled' with 'uint' as the element "
                  "offset type!") {
  vector<uint, N> _VAddr = vAddr;
  cm_slm_write(slmBuffer, _VAddr, vSrc);
}

/// \brief Shared local memory stateless read.
///
/// Load ::size bytes from memory address ::addr starting at ::offset to the
/// SLM buffer ::slmBuffer. ::size must be a multiple of 256.
///
CM_INLINE void cm_slm_load(uint slmBuffer, svmptr_t addr, uint offset,
                           uint size) {
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

  for (uint block = 0; block < numBlocks; block++) {
    using _VTy = vector<uint, 64>;
    _VTy data;
    const __global _VTy *const vSVMPtr =
        reinterpret_cast<const __global _VTy *const>(addr +
                                                     threadOffsetInMemory);
    __local _VTy *const vSLMPtr =
        reinterpret_cast<__local _VTy *const>(slmBuffer + threadOffsetInSLM);
    data = load<_VTy, Align::OWORD>(vSVMPtr);
    store<_VTy, Align::OWORD>(data, vSLMPtr);
    threadOffsetInMemory += numGroups * 256;
    threadOffsetInSLM += numGroups * 256;
  }

#if CM_GENX > 900
  cm_slm_fence(CM_GLOBAL_COHERENT_FENCE);
#endif
  cm_barrier();
}

#endif /* _CLANG_CM_SLM_H */
