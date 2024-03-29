/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// XFAIL: *

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

_GENX_MAIN_
void test2() {
}

// The Finalizer is called as we have specified a target so the -Qxcm_opt_report
// option causes the Finalizer optimization report to be printed
// RUN: %cmc -emit-llvm -march=BDW -Qxcm_opt_report -- %s 2>&1 | FileCheck --check-prefix=CHECK-BDW %s
//
// RUN: %cmc -emit-llvm -march=BDW /Qxcm_opt_report -- %s 2>&1 | FileCheck --check-prefix=CHECK-BDW %s

// CHECK-BDW: cm_opt_report.cpp(4,2):  warning: CM_GENX defined [-W#warnings]
// CHECK-BDW: cm_opt_report.cpp(14,2):  warning: CM_GEN8 defined [-W#warnings]
// CHECK-BDW: 2 warnings generated.
// CHECK-BDW: -platform BDW
