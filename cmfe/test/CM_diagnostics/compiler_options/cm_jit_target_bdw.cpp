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

#ifdef CM_GEN10_5
#warning CM_GEN10_5 defined
#endif

#ifdef CM_GEN11
#warning CM_GEN11 defined
#endif

// We test a number of different ways to specify a BDW jit target option.
// All are equivalent, and should produce the same results, so we only need
// one set of CHECK values for all of these tests.
// We check the expected Gen variant macros are defined (and no others), and
// that the Finalizer is called with the expected platform option.
// We also check that the expected files are generated by deleting them, 
// which also leaves things tidy for the next test.

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm -Qxcm_jit_target=bdw %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm -Qxcm_jit_target:bdw %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm -Qxcm_jit_targetbdw %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm -Qxcm_jit_target=gen8 %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm -Qxcm_jit_target=GEN8 %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm /Qxcm_jit_target=BDW %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm /Qxcm_jit_target:bdW %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm /Qxcm_jit_targetBdw %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm /Qxcm_jit_targetbDw %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm -Qxcm_jit_target -mcpu=BDW %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm -Qxcm_jit_target -mcpu=Gen8 %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm -march=Bdw -Qxcm_jit_target %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// RUN: %cmc -mCM_printfargs -mCM_old_asm_name -mdump_asm -Qxcm_jit_target -march=Gen8 %w 2>&1 | FileCheck %w
// RUN: rm %W.isa %W_0.visaasm %W_0.asm %W_0.dat

// CHECK: cm_jit_target_bdw.cpp(11,9):  warning: CM_GENX defined with value 800 [-W#pragma-messages]
// CHECK: cm_jit_target_bdw.cpp(21,2):  warning: CM_GEN8 defined [-W#warnings]
// CHECK: 2 warnings generated.
// CHECK: -platform BDW
