#include <cm/cm.h>

_GENX_ void test1()
{
  matrix<uint, 4, 4> m;
  vector<uint, 4>    v;
  // ...
  v = m.row(2);         // the 2nd row of m is copied to v
  m.column(3) = 0;      // the 3rd column of m are replaced with 0.
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
