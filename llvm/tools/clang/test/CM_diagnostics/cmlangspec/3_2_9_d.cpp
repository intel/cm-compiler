#include <cm/cm.h>

_GENX_ void test1()
{
  vector<int, 8> a;
  vector<int, 4> b = 42;

  a.select<4, 2>(0) = b;  // selected elements of a are replaced
                          // with elements of b (all elements of b are
                          // copied to elements a(0), a(2), a(4), a(6))
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
