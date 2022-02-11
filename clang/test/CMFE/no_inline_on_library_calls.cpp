/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// COM: TODO - remove -Xclang -disable-llvm-passes once it is the default

// RUN: %cmc -Xclang -disable-llvm-passes -D__DISABLE_INLINING_ON_CM_LIBRARY_CALLS -S -emit-llvm -march=SKL -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll --check-prefix=LIB_CALLS_NOT_INLINED
// LIB_CALLS_NOT_INLINED: cm_linear_global_id{{.*}}#[[F_ATTR:[0-9]+]]
// LIB_CALLS_NOT_INLINED: attributes #[[F_ATTR]] = {
// LIB_CALLS_NOT_INLINED-NOT: alwaysinline
// LIB_CALLS_NOT_INLINED-SAME: }

// RUN: %cmc  -Xclang -disable-llvm-passes -S -emit-llvm -march=SKL -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll --check-prefix=LIB_CALLS_INLINED
// LIB_CALLS_INLINED: cm_linear_global_id{{.*}}#[[F_ATTR:[0-9]+]]
// LIB_CALLS_INLINED: attributes #[[F_ATTR]] = {
// LIB_CALLS_INLINED-SAME: alwaysinline
// LIB_CALLS_INLINED-SAME: }

#include <cm/cm.h>

_GENX_MAIN_ void test() {
  auto x = cm_linear_global_id();
}
