#include <cm/cm.h>

_GENX_ void test1()
{
  vector<float, 8>   v1, v2, v3;
  matrix<uint, 2, 4> m1;
  matrix<uint, 4, 2> m2;

  v1 = v2 + v3;
  v1 *= 2.0f;
  m2 = m1 - v1;   // the data elements of m1 and v1 are
                  // selected in row major ordering for the operation
                  // i.e. v1[0-3] is subtracted from m1.row(0),
                  // and v1[4-7] is subtracted from m1.row(1).
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
