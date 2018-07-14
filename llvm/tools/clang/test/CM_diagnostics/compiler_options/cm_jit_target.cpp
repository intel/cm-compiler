#include <cm/cm.h>

_GENX_MAIN_
void test() {
}

#define STRING2(x) #x
#define STRING(x) STRING2(x)

#ifdef CM_GENX
#pragma message ( "CM_GENX defined with value " STRING(CM_GENX) )
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

#ifdef CM_GEN11
#warning CM_GEN11 defined
#endif

// The Finalizer should not be called as we haven't specified a target
// RUN: %cmc -Qxcm_jit_target %w 2>&1 | FileCheck %w 
// RUN: rm %W.isa

// CHECK: cm_jit_target.cpp(13,2):  warning: CM_GENX not defined [-W#warnings]
// CHECK: 1 warning generated.
// CHECK-NOT: -platform
