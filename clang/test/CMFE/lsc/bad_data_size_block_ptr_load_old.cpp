/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -emit-llvm -march=PVC -- %s 2>&1 | FileCheck %s

// Old ptr block load.
template<typename T, DataSize DS>
void test_ptr(const T *In, unsigned Offset) {
  CM_STATIC_WARNING(DS == DataSize::U32, "LABEL");
  auto x = cm_ptr_load<T, VectorSize::N4, DS>(In, Offset);
  (void)x;
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<char, DataSize::U8>' requested here
// CHECK: error: Transposed load can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_load_ptr_u8(const char *In, unsigned Offset) {
  test_ptr<char, DataSize::U8>(In, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<short, DataSize::U16>' requested here
// CHECK: error: Transposed load can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_load_ptr_u16(const short *In, unsigned Offset) {
  test_ptr<short, DataSize::U16>(In, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<unsigned int, DataSize::U8U32>' requested here
// CHECK: error: Transposed load can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_load_ptr_u8u32(const unsigned *In, unsigned Offset) {
  test_ptr<unsigned, DataSize::U8U32>(In, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<unsigned int, DataSize::U16U32>' requested here
// CHECK: error: Transposed load can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_load_ptr_u16u32(const unsigned *In, unsigned Offset) {
  test_ptr<unsigned, DataSize::U16U32>(In, Offset);
}

// CHECK-LABEL: in instantiation of function template specialization 'test_ptr<unsigned int, DataSize::U16U32H>' requested here
// CHECK: error: Transposed load can work only with U32 and U64 data sizes
_GENX_MAIN_ void bad_load_ptr_u16u32h(const unsigned *In, unsigned Offset) {
  test_ptr<unsigned, DataSize::U16U32H>(In, Offset);
}
