#include <cm/cm.h>

_GENX_ void test1()
{
  matrix<float, 4, 2> m1;
  // ...
  vector_ref<float, 4> v1(m1.select<2,1,2,1>(0,0).format<float>());
  // v1 is a reference to rows 0 and 1 of m1

  vector_ref<float, 4> v2(m1.select<2,1,2,1>(1,0).format<float>());
  // v2 is a reference to rows 1 and 2 of m1

  v2 = v1 + v2;
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
