#include <cm/cm.h>
#include <cm/cmtl.h>

_GENX_ void test1()
{
  cm_vector(v, ushort, 16, 2, 3);
  // ...
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
