/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -march=PVC -- %s 2>&1 | FileCheck %s

template <CacheHint L1H, CacheHint L2H, typename T = int, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, int N = details::lsc_default_simt()>
void check_block(SurfaceIndex Idx) {
  auto x = cm_atomic<AtomicOp::IINC, T, VS, DS, L1H, L2H>(Idx, vector<unsigned, N>{});
  (void)x;
}

template <CacheHint L1H, CacheHint L2H, typename T = int, VectorSize VS = VectorSize::N1, DataSize DS = DataSize::U32, int N = details::lsc_default_simt()>
void check_flat(T *Ptr) {
  auto x = cm_ptr_atomic<AtomicOp::IINC, T, VS, DS, L1H, L2H>(Ptr, vector<unsigned, N>{});
  (void)x;
}

_GENX_MAIN_ void check_pass_block(SurfaceIndex Idx) {
  check_block<CacheHint::Default, CacheHint::Default>(Idx);
}

// CHECK-LABEL: error: unsupported cache hint
// CHECK: in instantiation of function template specialization 'check_block<CacheHint::Streaming, CacheHint::Streaming,
_GENX_MAIN_ void check_fail_block(SurfaceIndex Idx) {
  check_block<CacheHint::Streaming, CacheHint::Streaming>(Idx);
}

_GENX_MAIN_ void check_pass_flat(unsigned *Ptr) {
  check_flat<CacheHint::Default, CacheHint::Default>(Ptr);
}

// CHECK-LABEL: error: unsupported cache hint
// CHECK: in instantiation of function template specialization 'check_flat<CacheHint::Streaming, CacheHint::Streaming,
_GENX_MAIN_ void check_fail_flat(unsigned *Ptr) {
  check_flat<CacheHint::Streaming, CacheHint::Streaming>(Ptr);
}
