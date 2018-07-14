#include <cm/cm.h>

_GENX_ void test1()
{
  matrix<float, 4, 8>  m1;
  matrix<float, 2, 2>  m2;

  m1.select<4, 1, 4, 2>(0, 0) = 0.0f; // selected elements of m1
                                      // are replaced with 0.0f
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
