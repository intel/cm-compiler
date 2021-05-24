#include <cm/cm.h>
#include <cm/cmtl.h>

_GENX_ void test1()
{
  cm_vector(v, ushort, 16, 2, 3);
  // ...
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
