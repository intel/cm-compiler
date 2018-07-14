// The Finalizer should not be called as we haven't specified a target
// The /Qxno_clang_cm_diag option is ignored
// RUN: %cmc /Qxno_clang_cm_diag %w 2>&1 | FileCheck %w 
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

// CHECK: no_clang_cm_diag_slash.cpp(15,2):  warning: CM_GENX not defined [-W#warnings]
// CHECK: 1 warning generated.
// CHECK-NOT: -platform
