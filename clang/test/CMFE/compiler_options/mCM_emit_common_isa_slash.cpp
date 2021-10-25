/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// XFAIL: *

// The /mCM_emit_common_isa option is accepted but ignored
// RUN: %cmc -emit-llvm /mCM_emit_common_isa -march=skl -- %s 2>&1 | FileCheck %s

#include <cm/cm.h>

_GENX_MAIN_
void test() {
}

#ifdef CM_GENX
#warning CM_GENX defined
#else
#warning CM_GENX not defined
#endif

#ifdef CM_GEN7_5
#warning CM_GEN7_5 defined
#endif

#ifdef CM_GEN8
#warning CM_GEN8 defined
#endif

#ifdef CM_GEN8_5
#warning CM_GEN8_5 defined
#endif

#ifdef CM_GEN9
#warning CM_GEN9 defined
#endif

#ifdef CM_GEN9_5
#warning CM_GEN9_5 defined
#endif

#ifdef CM_GEN10
#warning CM_GEN10 defined
#endif

// CHECK: mCM_emit_common_isa_slash.cpp(12,2): warning: CM_GENX defined [-W#warnings]
// CHECK: mCM_emit_common_isa_slash.cpp(30,2): warning: CM_GEN9 defined [-W#warnings]
// CHECK: 2 warnings generated.
// CHECK: -platform SKL
