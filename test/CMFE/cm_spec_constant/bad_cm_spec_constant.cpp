/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -march=skl -emit-llvm -S -o %t.ll -- %s 2>&1 | FileCheck %s


#include <cm/cm.h>

extern "C" _GENX_MAIN_ void test_spec_constant() {
  auto sp1 = cm_spec_constant<bool, 0>();
// CHECK: error: no matching function for call to 'cm_spec_constant'
// CHECK-COUNT-2: candidate template ignored: requirement 'details::is_cm_scalar<bool>::value' was not satisfied [with T = bool, ID = 0]
}

extern "C" _GENX_MAIN_ void test_fp_spec_constant() {
  auto sp1 = cm_spec_constant<half, 111, 0>();
// CHECK: error: no matching function for call to 'cm_spec_constant'
// CHECK: candidate template ignored: substitution failure: too many template arguments for function template 'cm_spec_constant'
// CHECK: candidate template ignored: invalid explicitly-specified argument for template parameter 'DefaultValue'
  auto sp2 = cm_spec_constant<float, 112, 1.f>();
// CHECK: error: no matching function for call to 'cm_spec_constant'
// CHECK: candidate template ignored: substitution failure: too many template arguments for function template 'cm_spec_constant'
// CHECK: candidate template ignored: invalid explicitly-specified argument for template parameter 'DefaultValue'
  auto sp3 = cm_spec_constant<double, 113, 0>();
// CHECK: error: no matching function for call to 'cm_spec_constant'
// CHECK: candidate template ignored: substitution failure: too many template arguments for function template 'cm_spec_constant'
// CHECK: candidate template ignored: invalid explicitly-specified argument for template parameter 'DefaultValue'
}

