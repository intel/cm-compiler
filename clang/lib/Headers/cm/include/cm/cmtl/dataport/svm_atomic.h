/*========================== begin_copyright_notice ============================

Copyright (C) 2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_CMTL_DATAPORT_SVM_ATOMIC_H
#define CM_CMTL_DATAPORT_SVM_ATOMIC_H

namespace cmtl {
namespace dataport {

// has return value, one source
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE typename std::enable_if<op != ATOMIC_CMPXCHG && op != ATOMIC_FCMPWR &&
                                      op != ATOMIC_INC && op != ATOMIC_DEC,
                                  void>::type
write_atomic(svmptr_t addr, vector<uint, N> elementOffset, vector<T, N> src0,
             vector_ref<T, N> ret) {
  constexpr unsigned grain = 8;
  static_assert((N % grain) == 0);
#pragma unroll
  for (int i = 0; i < N; i += grain) {
    ::cm_svm_atomic(
        op, addr + elementOffset.template select<grain, 1>(i) * sizeof(T),
        ret.template select<grain, 1>(i), src0.template select<grain, 1>(i));
  }
}

// no return value, one source
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE typename std::enable_if<op != ATOMIC_CMPXCHG && op != ATOMIC_FCMPWR &&
                                      op != ATOMIC_INC && op != ATOMIC_DEC,
                                  void>::type
write_atomic(svmptr_t addr, vector<uint, N> elementOffset, vector<T, N> src0) {
  vector<T, N> dummy;
  write_atomic<op, T, N>(addr, elementOffset, src0, dummy);
}

// INC/DEC: return value
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE
    typename std::enable_if<op == ATOMIC_INC || op == ATOMIC_DEC, void>::type
    write_atomic(svmptr_t addr, vector<uint, N> elementOffset,
                 vector_ref<T, N> ret) {
  constexpr unsigned grain = 8;
  static_assert((N % grain) == 0);
#pragma unroll
  for (int i = 0; i < N; i += grain) {
    ::cm_svm_atomic(
        op, addr + elementOffset.template select<grain, 1>(i) * sizeof(T),
        ret.template select<grain, 1>(i));
  }
}

// INC/DEC: no return value
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE
    typename std::enable_if<op == ATOMIC_INC || op == ATOMIC_DEC, void>::type
    write_atomic(svmptr_t addr, vector<uint, N> elementOffset) {
  vector<T, N> dummy;
  write_atomic<op, T, N>(addr, elementOffset, dummy);
}

// CMPXCHG: return value
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE typename std::enable_if<op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR,
                                  void>::type
write_atomic(svmptr_t addr, vector<uint, N> elementOffset, vector<T, N> src0,
             vector<T, N> src1, vector_ref<T, N> ret) {
  constexpr unsigned grain = 8;
  static_assert((N % grain) == 0);
#pragma unroll
  for (int i = 0; i < N; i += grain) {
    ::cm_svm_atomic(
        op, addr + elementOffset.template select<grain, 1>(i) * sizeof(T),
        ret.template select<grain, 1>(i), src0.template select<grain, 1>(i),
        src1.template select<grain, 1>(i));
  }
}

// CMPXCHG: no return value
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE typename std::enable_if<op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR,
                                  void>::type
write_atomic(svmptr_t addr, vector<uint, N> elementOffset, vector<T, N> src0,
             vector<T, N> src1) {
  vector<T, N> dummy;
  write_atomic<op, T, N>(addr, elementOffset, src0, src1, dummy);
}

// has return value, one source with boundary check
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE typename std::enable_if<op != ATOMIC_CMPXCHG && op != ATOMIC_FCMPWR &&
                                      op != ATOMIC_INC && op != ATOMIC_DEC,
                                  void>::type
write_atomic(svmptr_t addr, vector<uint, N> elementOffset, uint size,
             vector<T, N> src0, vector_ref<T, N> ret) {
  for (int i = 0; i < N; i++) {
    if (elementOffset[i] >= size) {
      printf("write_atomic bound access detected at [%d], elementOffset: %d, "
             "size: %d\n",
             i, elementOffset[i], size);
      return;
    }
  }
  write_atomic<op, T, N>(addr, elementOffset, src0, ret);
}

// no return value, one source with boundary check
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE typename std::enable_if<op != ATOMIC_CMPXCHG && op != ATOMIC_FCMPWR &&
                                      op != ATOMIC_INC && op != ATOMIC_DEC,
                                  void>::type
write_atomic(svmptr_t addr, vector<uint, N> elementOffset, uint size,
             vector<T, N> src0) {
  vector<T, N> dummy;
  write_atomic<op, T, N>(addr, elementOffset, size, src0, dummy);
}

// INC/DEC: return value
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE
    typename std::enable_if<op == ATOMIC_INC || op == ATOMIC_DEC, void>::type
    write_atomic(svmptr_t addr, vector<uint, N> elementOffset, uint size,
                 vector_ref<T, N> ret) {
  for (int i = 0; i < N; i++) {
    if (elementOffset[i] >= size) {
      printf("write_atomic bound access detected at [%d], elementOffset: %d, "
             "size: %d\n",
             i, elementOffset[i], size);
      return;
    }
  }

  write_atomic(addr, elementOffset, ret);
}

// INC/DEC: no return value with boundary check
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE
    typename std::enable_if<op == ATOMIC_INC || op == ATOMIC_DEC, void>::type
    write_atomic(svmptr_t addr, vector<uint, N> elementOffset, uint size) {
  vector<T, N> dummy;
  write_atomic<op, T, N>(addr, elementOffset, size, dummy);
}

// CMPXCHG: return value with boundary check
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE typename std::enable_if<op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR,
                                  void>::type
write_atomic(svmptr_t addr, vector<uint, N> elementOffset, uint size,
             vector<T, N> src0, vector<T, N> src1, vector_ref<T, N> ret) {
  for (int i = 0; i < N; i++) {
    if (elementOffset[i] >= size) {
      printf("write_atomic bound access detected at [%d], elementOffset: %d, "
             "size: %d\n",
             i, elementOffset[i], size);
      return;
    }
  }
  write_atomic(addr, elementOffset, src0, src1, ret);
}

// CMPXCHG: no return value with boundary check
template <CmAtomicOpType op, typename T, unsigned N>
CM_INLINE typename std::enable_if<op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR,
                                  void>::type
write_atomic(svmptr_t addr, vector<uint, N> elementOffset, uint size,
             vector<T, N> src0, vector<T, N> src1) {
  vector<T, N> dummy;
  write_atomic<op, T, N>(addr, elementOffset, size, src0, src1, dummy);
}

} // namespace dataport
} // namespace cmtl

#endif // CM_CMTL_DATAPORT_SVM_ATOMIC_H
