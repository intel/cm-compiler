// The Finalizer should not be called as we haven't specified a target
// RUN: %cmc -mCM_opt_mask:0 %w 2>&1 | FileCheck %w 
// RUN: rm %W.isa

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

// CHECK: mCM_opt_mask_colon.cpp(14,2):  warning: CM_GENX not defined [-W#warnings]
// CHECK: 1 warning generated.
// CHECK-NOT: -platform
