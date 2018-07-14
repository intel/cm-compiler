// We check that the specified path is used to build the invocation of the finalizer
// RUN: %cmc -### /Qxcm_jit_target=gen10 /mCM_genx_assembler=path/to/genx/finalizer %w 2>&1 | FileCheck %w

#include <cm/cm.h>

_GENX_MAIN_
void test() {
}

#ifdef CM_GENX
#warning CM_GENX defined
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

// CHECK: path/to/genx/finalizer
// CHECK-SAME: "-platform" "CNL"
