// The -mCM_emit_common_isa option is accepted but ignored
// RUN: %cmc -mCM_emit_common_isa -Qxcm_jit_target=skl %w 2>&1 | FileCheck %w 
// RUN: rm %W.isa %W_0.asm %W_0.dat %W_0.visaasm

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

// CHECK: mCM_emit_common_isa.cpp(12,2): warning: CM_GENX defined [-W#warnings]
// CHECK: mCM_emit_common_isa.cpp(30,2): warning: CM_GEN9 defined [-W#warnings]
// CHECK: 2 warnings generated.
// CHECK: -platform SKL
