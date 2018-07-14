#include <cm/cm.h>

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

#ifdef CM_GEN11
#warning CM_GEN11 defined
#endif

_GENX_MAIN_
void test1(SurfaceIndex S, vector<uchar,64> i) {
  vector<uchar,16> v = i.select<16,1>(0)  +
                       i.select<16,1>(16) +
                       i.select<16,1>(32) +
                       i.select<16,1>(48);
  write(S,0,0,v);
}

// The Finalizer is called so we generate an .asm file, which should 
// contain some debug information as we haven't specified -Qxcm_release.
// RUN: %cmc -Qxcm_jit_target=BDW %w 2>&1 | FileCheck %w 
// RUN: cat %W_0.asm | FileCheck --check-prefix=ASM %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// CHECK: cm_release.cpp(4,2):  warning: CM_GENX defined [-W#warnings]
// CHECK: cm_release.cpp(14,2):  warning: CM_GEN8 defined [-W#warnings]
// CHECK: 2 warnings generated.
// CHECK: -platform BDW
// CHECK-NOT: -stripcomment

// ASM: .kernel
// ASM: .declare

// The Finalizer is called so we generate an .asm file, which should not
// contain any debug information as we also specified -Qxcm_release.
// RUN: %cmc -Qxcm_jit_target=BDW -Qxcm_release %w 2>&1 | FileCheck --check-prefix=CHECK-RELEASE %w
// RUN: cat %W_0.asm | FileCheck --check-prefix=ASM-RELEASE %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat
//
// RUN: %cmc /Qxcm_jit_target=BDW /Qxcm_release %w 2>&1 | FileCheck --check-prefix=CHECK-RELEASE %w
// RUN: cat %W_0.asm | FileCheck --check-prefix=ASM-RELEASE %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// CHECK-RELEASE: cm_release.cpp(4,2):  warning: CM_GENX defined [-W#warnings]
// CHECK-RELEASE: cm_release.cpp(14,2):  warning: CM_GEN8 defined [-W#warnings]
// CHECK-RELEASE: 2 warnings generated.
// CHECK-RELEASE: -platform BDW

// ASM-RELEASE-NOT: .kernel
// ASM-RELEASE-NOT: .declare
