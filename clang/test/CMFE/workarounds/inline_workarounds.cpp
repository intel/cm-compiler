/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// COM: TODO - remove -Xclang -disable-llvm-passes once it is the default
// RUN: %cmc -march=SKL -Xclang -disable-llvm-passes -S -emit-llvm  -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll --check-prefix=DEFAULT_OCL_PRINT_ATTRS

// DEFAULT_OCL_PRINT_ATTRS: define {{[^@]+}}@_Z6print{{[^)]+}}) #[[PRINT_ATTR:[0-9]+]]
// DEFAULT_OCL_PRINT_ATTRS: attributes #[[PRINT_ATTR]] = {
// DEFAULT_OCL_PRINT_ATTRS-SAME: alwaysinline
// DEFAULT_OCL_PRINT_ATTRS-SAME: }

// RUN: %cmc -cm_disable_strong_inline -D__DISABLE_CM_PRINT_INLINE_WA -march=SKL -Xclang -disable-llvm-passes -S -emit-llvm  -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll --check-prefix=OCL_PRINT_ATTRS_WA
// OCL_PRINT_ATTRS_WA: define {{[^@]+}}@_Z6print{{[^)]+}}) #[[PRINT_ATTR:[0-9]+]]
// OCL_PRINT_ATTRS_WA: attributes #[[PRINT_ATTR]] = {
// OCL_PRINT_ATTRS_WA-NOT: alwaysinline
// OCL_PRINT_ATTRS_WA-SAME: }

// RUN: %cmc -march=SKL -Xclang -disable-llvm-passes -S -emit-llvm  -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll --check-prefix=DEFAULT_ADDC_ATTRS
// DEFAULT_ADDC_ATTRS: define {{[^@]+}}@_Z7cm_addc{{[^)]+}}) #[[ADDC_ATTRS:[0-9]+]]
// DEFAULT_ADDC_ATTRS: attributes #[[ADDC_ATTRS]] = {
// DEFAULT_ADDC_ATTRS-SAME: alwaysinline
// DEFAULT_ADDC_ATTRS-SAME: }

// RUN: %cmc -cm_disable_strong_inline -D__DISABLE_SPIRV_WRITER_INLINE_WA -march=SKL -Xclang -disable-llvm-passes -S -emit-llvm  -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll --check-prefix=DEFAULT_ADDC_ATTRS_INLINE_WA
// DEFAULT_ADDC_ATTRS_INLINE_WA: define {{[^@]+}}@_Z7cm_addc{{[^)]+}}) #[[ADDC_ATTRS:[0-9]+]]
// DEFAULT_ADDC_ATTRS_INLINE_WA: attributes #[[ADDC_ATTRS]] = {
// DEFAULT_ADDC_ATTRS_INLINE_WA-NOT: alwaysinline
// DEFAULT_ADDC_ATTRS_INLINE_WA-SAME: }

#include <cm/cm.h>

_GENX_MAIN_ void test() {
  printf("%s\n", "Hello darkness, my old friend");

  vector<unsigned, 16> a;
  vector<unsigned, 16> b;
  vector<unsigned, 16> c;
  vector<unsigned, 16> r = cm_addc(a, b, c);
}
