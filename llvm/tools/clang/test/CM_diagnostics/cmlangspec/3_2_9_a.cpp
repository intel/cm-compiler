#include <cm/cm.h>

_GENX_ void test1()
{
  vector<uint, 8> v1;
  matrix<uint, 4, 4> m1;

  m1(2,3) = v1(2);
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
