#include <cm/cm.h>
#include <cm/cmtl.h>

_GENX_ void test1()
{
  cm_matrix(m, ushort, 4, 8, 10, 5);
  // ...
}

// RUN: %cmc -emit-llvm %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
