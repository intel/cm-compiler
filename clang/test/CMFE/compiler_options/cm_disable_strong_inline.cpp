/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// COM: TODO - remove -Xclang -disable-llvm-passes once it is the default

// RUN: %cmc -Xclang -disable-llvm-passes -S -emit-llvm -march=SKL -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll --check-prefix=ALWAYS_INLINE
// ALWAYS_INLINE: someFunction{{.*}}#[[F_ATTR:[0-9]+]]
// ALWAYS_INLINE: attributes #[[F_ATTR]] = {
// ALWAYS_INLINE-SAME: alwaysinline
// ALWAYS_INLINE-SAME: }

// RUN: %cmc -Xclang -disable-llvm-passes -cm_disable_strong_inline -S -emit-llvm -march=SKL -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll --check-prefix=NO_ALWAYS_INLINE
// NO_ALWAYS_INLINE: someFunction{{.*}}#[[F_ATTR:[0-9]+]]
// NO_ALWAYS_INLINE: attributes #[[F_ATTR]] = {
// NO_ALWAYS_INLINE-NOT: alwaysinline
// NO_ALWAYS_INLINE-SAME: }

#include <cm/cm.h>

inline void someFunction() {
}

_GENX_MAIN_ void test() {
  someFunction();
}
