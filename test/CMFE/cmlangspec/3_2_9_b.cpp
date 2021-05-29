#include <cm/cm.h>

_GENX_ void test1()
{
  vector<uint, 8> v1;
  matrix<uint, 4, 4> m1;
  matrix<ushort, 4, 4> m2;

  m1[2][3] = v1[2];
  v1[m2[0][3]] += 1;
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
