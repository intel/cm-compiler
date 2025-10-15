/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -march=PVC -DXe_ONLY_LEGAL -- %s
// RUN: %cmc -emit-llvm -march=bmg -DXe2_ONLY_LEGAL -- %s

// RUN: %cmc -emit-llvm -march=PVC -DXe2_ONLY_LEGAL -- %s 2>&1 | FileCheck %s --check-prefix CHECK-PVC
// RUN: %cmc -emit-llvm -march=bmg -DBOTH_ILLEGAL -- %s 2>&1 | FileCheck %s --check-prefix CHECK-BOTH

template <CacheHint L1H, CacheHint L2H, DataSize DS = DataSize::U32>
void check_block(SurfaceIndex Idx) {
  cm_prefetch<1, DS, L1H, L2H>(Idx, unsigned{});
}

template <CacheHint L1H, CacheHint L2H, DataSize DS = DataSize::U32>
void check_flat(const unsigned *const Ptr) {
  cm_ptr_prefetch<1, DS, L1H, L2H>(Ptr,
                                   unsigned{});
}

_GENX_MAIN_ void check(SurfaceIndex Idx, unsigned *Ptr) {
#define CHECK(L1H, L2H)                                \
  check_block<CacheHint::L1H, CacheHint::L2H>(Idx);    \
  check_flat<CacheHint::L1H, CacheHint::L2H>(Ptr);

  CHECK(Uncached, Cached);
  CHECK(Cached, Uncached);
  CHECK(Cached, Cached);
  CHECK(Streaming, Uncached);
  CHECK(Streaming, Cached);

#ifdef Xe2_ONLY_LEGAL
// CHECK-PVC: error: unsupported cache hint
// CHECK-PVC: in instantiation of function template specialization 'check_block<CacheHint::Uncached, CacheHint::ConstCached,
// CHECK-PVC: error: unsupported cache hint
// CHECK-PVC: in instantiation of function template specialization 'check_flat<CacheHint::Uncached, CacheHint::ConstCached,
  CHECK(Uncached, ConstCached);
// CHECK-PVC: error: unsupported cache hint
// CHECK-PVC: in instantiation of function template specialization 'check_block<CacheHint::Cached, CacheHint::ConstCached,
// CHECK-PVC: error: unsupported cache hint
// CHECK-PVC: in instantiation of function template specialization 'check_flat<CacheHint::Cached, CacheHint::ConstCached,
  CHECK(Cached, ConstCached);
#endif

#ifdef BOTH_ILLEGAL
// Check only several cases

// CHECK-BOTH: error: unsupported cache hint
// CHECK-BOTH: in instantiation of function template specialization 'check_block<CacheHint::ReadInvalidate, CacheHint::Cached,
// CHECK-BOTH: error: unsupported cache hint
// CHECK-BOTH: in instantiation of function template specialization 'check_flat<CacheHint::ReadInvalidate, CacheHint::Cached,
  CHECK(ReadInvalidate, Cached);

// CHECK-BOTH: error: unsupported cache hint
// CHECK-BOTH: in instantiation of function template specialization 'check_block<CacheHint::ReadInvalidate, CacheHint::ReadInvalidate,
// CHECK-BOTH: error: unsupported cache hint
// CHECK-BOTH: in instantiation of function template specialization 'check_flat<CacheHint::ReadInvalidate, CacheHint::ReadInvalidate,
  CHECK(ReadInvalidate, ReadInvalidate);

// CHECK-BOTH: error: unsupported cache hint
// CHECK-BOTH: in instantiation of function template specialization 'check_block<CacheHint::Streaming, CacheHint::Streaming,
// CHECK-BOTH: error: unsupported cache hint
// CHECK-BOTH: in instantiation of function template specialization 'check_flat<CacheHint::Streaming, CacheHint::Streaming,
  CHECK(Streaming, Streaming);
// CHECK-BOTH: error: unsupported cache hint
// CHECK-BOTH: in instantiation of function template specialization 'check_block<CacheHint::WriteThrough, CacheHint::WriteBack,
// CHECK-BOTH: error: unsupported cache hint
// CHECK-BOTH: in instantiation of function template specialization 'check_flat<CacheHint::WriteThrough, CacheHint::WriteBack,
  CHECK(WriteThrough, WriteBack);

#endif
}
