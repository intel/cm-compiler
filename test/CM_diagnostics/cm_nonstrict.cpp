#include <cm/cm.h>

#define C 255

_GENX_MAIN_ void foo(SurfaceIndex S, short a, short b, short c, short d)
{
  matrix<short,1,16> tempBuffer;
  #pragma cm_nonstrict
  tempBuffer = (C * a) + (C * b) + (C * c) + (C * d);
  write(S, 0, 0, tempBuffer);
}


// warning expected for use of #pragma cm_nonstrict
// RUN: %cmc -emit-llvm -march=SKL %s 2>&1 | FileCheck %s
//
// CHECK:  cm_nonstrict.cpp(8,11):  warning: cm_nonstrict is deprecated - see CM LLVM Porting Guide [-Wdeprecated]
// CHECK: 1 warning generated.
// CHECK -platform SKL

// warning for use of #pragma cm_nonstrict expected to be suppressed by -Wno-deprecated option
// RUN: %cmc -emit-llvm -march=SKL -Wno-deprecated %s 2>&1 | FileCheck --check-prefix=NOWARN %s
//
// NOWARN-NOT: cm_nonstrict is deprecated
// NOWARN-NOT: warning generated.
// NOWARN -platform SKL
