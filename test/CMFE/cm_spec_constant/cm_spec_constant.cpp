/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=skl -emit-llvm -S -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll


#include <cm/cm.h>

// CHECK-LABEL: @test_spec_constant
extern "C" _GENX_MAIN_ void test_spec_constant() {
  auto sp1 = cm_spec_constant<int, 0>();
  static_assert(std::is_same<decltype(sp1), int>::value);
// CHECK: {{[^)]+}} = call i32 @_Z20__spirv_SpecConstantIiET_jS0_(i32 0, i32 0)
  auto sp2 = cm_spec_constant<int, 0, 1>();
  static_assert(std::is_same<decltype(sp2), int>::value);
// CHECK: {{[^)]+}} = call i32 @_Z20__spirv_SpecConstantIiET_jS0_(i32 0, i32 1)
  auto sp3 = cm_spec_constant<char, 1, 2>();
  static_assert(std::is_same<decltype(sp3), char>::value);
// CHECK: {{[^)]+}} = call signext i8 @_Z20__spirv_SpecConstantIcET_jS0_(i32 1, i8 signext 2)
  auto sp4 = cm_spec_constant<unsigned char, 2>();
  static_assert(std::is_same<decltype(sp4), unsigned char>::value);
// CHECK: {{[^)]+}} = call zeroext i8 @_Z20__spirv_SpecConstantIhET_jS0_(i32 2, i8 zeroext 0)
  auto sp5 = cm_spec_constant<signed char, 3>();
  static_assert(std::is_same<decltype(sp5), signed char>::value);
// CHECK: {{[^)]+}} = call signext i8 @_Z20__spirv_SpecConstantIaET_jS0_(i32 3, i8 signext 0)
  auto sp6 = cm_spec_constant<short, 4, -15>();
  static_assert(std::is_same<decltype(sp6), short>::value);
// CHECK: {{[^)]+}} = call signext i16 @_Z20__spirv_SpecConstantIsET_jS0_(i32 4, i16 signext -15)
  auto sp7 = cm_spec_constant<unsigned short, 5>();
  static_assert(std::is_same<decltype(sp7), unsigned short>::value);
// CHECK: {{[^)]+}} = call zeroext i16 @_Z20__spirv_SpecConstantItET_jS0_(i32 5, i16 zeroext 0)
  auto sp8 = cm_spec_constant<unsigned int, 6, 9999>();
  static_assert(std::is_same<decltype(sp8), unsigned int>::value);
// CHECK: {{[^)]+}} = call i32 @_Z20__spirv_SpecConstantIjET_jS0_(i32 6, i32 9999)
  auto sp9 = cm_spec_constant<unsigned long int, 7>();
  static_assert(std::is_same<decltype(sp9), unsigned long int>::value);
// CHECK: {{[^)]+}} = call i64 @_Z20__spirv_SpecConstantImET_jS0_(i32 7, i64 0)
  auto sp10 = cm_spec_constant<long int, 8, -999>();
  static_assert(std::is_same<decltype(sp10), long int>::value);
// CHECK: {{[^)]+}} = call i64 @_Z20__spirv_SpecConstantIlET_jS0_(i32 8, i64 -999)
  auto sp11 = cm_spec_constant<long long int, 9>();
  static_assert(std::is_same<decltype(sp11), long long int>::value);
// CHECK: {{[^)]+}} = call i64 @_Z20__spirv_SpecConstantIxET_jS0_(i32 9, i64 0)
  auto sp12 = cm_spec_constant<unsigned long long int, 10>();
  static_assert(std::is_same<decltype(sp12), unsigned long long int>::value);
// CHECK: {{[^)]+}} = call i64 @_Z20__spirv_SpecConstantIyET_jS0_(i32 10, i64 0)
}

// CHECK-LABEL: @test_fp_spec_constant
extern "C" _GENX_MAIN_ void test_fp_spec_constant() {
  auto sp1 = cm_spec_constant<half, 111>();
  static_assert(std::is_same<decltype(sp1), half>::value);
// CHECK: {{[^)]+}} = call half @_Z20__spirv_SpecConstantIDhET_jS0_(i32 111, half 0xH0000)
  auto sp2 = cm_spec_constant<float, 112>();
  static_assert(std::is_same<decltype(sp2), float>::value);
// CHECK: {{[^)]+}} = call float @_Z20__spirv_SpecConstantIfET_jS0_(i32 112, float 0.000000e+00)
  auto sp3 = cm_spec_constant<double, 113>();
  static_assert(std::is_same<decltype(sp3), double>::value);
// CHECK: {{[^)]+}} = call double @_Z20__spirv_SpecConstantIdET_jS0_(i32 113, double 0.000000e+00)
}

