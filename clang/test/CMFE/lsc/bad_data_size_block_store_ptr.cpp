/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -emit-llvm -march=PVC -- %s 2>&1 | FileCheck %s

// Ptr block store.
template<typename T, DataSize DS>
void test_ptr(T *In, unsigned Offset) {
  CM_STATIC_WARNING(DS == DataSize::U32, "LABEL");
  vector<T, 4> Data(0);
  cm_ptr_store<T, 4, DS>(In, Offset, Data);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<char, DataSize::U8>' requested here
// CHECK: error: Transposed store can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_store_ptr_u8(char *In, unsigned Offset) {
  test_ptr<char, DataSize::U8>(In, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<short, DataSize::U16>' requested here
// CHECK: error: Transposed store can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_store_ptr_u16(short *In, unsigned Offset) {
  test_ptr<short, DataSize::U16>(In, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<unsigned int, DataSize::U8U32>' requested here
// CHECK: error: Transposed store can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_store_ptr_u8u32(unsigned *In, unsigned Offset) {
  test_ptr<unsigned, DataSize::U8U32>(In, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<unsigned int, DataSize::U16U32>' requested here
// CHECK: error: Transposed store can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_store_ptr_u16u32(unsigned *In, unsigned Offset) {
  test_ptr<unsigned, DataSize::U16U32>(In, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<unsigned int, DataSize::U16U32H>' requested here
// CHECK: error: Transposed store can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_store_ptr_u16u32h(unsigned *In, unsigned Offset) {
  test_ptr<unsigned, DataSize::U16U32H>(In, Offset);
}
