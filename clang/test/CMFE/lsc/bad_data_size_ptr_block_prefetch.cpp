/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -emit-llvm -march=PVC -- %s 2>&1 | FileCheck %s

// New ptr prefetch.
template<DataSize DS>
void test_ptr(const unsigned *Ptr, unsigned Offset) {
  CM_STATIC_WARNING(DS == DataSize::U32, "LABEL");
  cm_ptr_prefetch<4, DS>(Ptr, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<DataSize::U8>' requested here
// CHECK: error: Transposed prefetch can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_prefetch_ptr_u8(const unsigned *Ptr, unsigned Offset) {
  test_ptr<DataSize::U8>(Ptr, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<DataSize::U16>' requested here
// CHECK: error: Transposed prefetch can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_prefetch_ptr_u16(const unsigned *Ptr, unsigned Offset) {
  test_ptr<DataSize::U16>(Ptr, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<DataSize::U8U32>' requested here
// CHECK: error: Transposed prefetch can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_prefetch_ptr_u8u32(const unsigned *Ptr, unsigned Offset) {
  test_ptr<DataSize::U8U32>(Ptr, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<DataSize::U16U32>' requested here
// CHECK: error: Transposed prefetch can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_prefetch_ptr_u16u32(const unsigned *Ptr, unsigned Offset) {
  test_ptr<DataSize::U16U32>(Ptr, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<DataSize::U16U32H>' requested here
// CHECK: error: Transposed prefetch can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_prefetch_ptr_u16u32h(const unsigned *Ptr, unsigned Offset) {
  test_ptr<DataSize::U16U32H>(Ptr, Offset);
}
