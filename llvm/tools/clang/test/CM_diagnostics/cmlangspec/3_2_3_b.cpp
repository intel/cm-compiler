#include <cm/cm.h>
#include <cm/cmtl.h>

_GENX_ void test1()
{
  cm_matrix(m, ushort, 4, 8, 10, 5);
  // ...
}

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: No CM kernel definitions found
// CHECK: 1 warning generated
