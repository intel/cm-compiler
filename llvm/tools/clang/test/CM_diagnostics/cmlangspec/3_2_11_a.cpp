#include <cm/cm.h>

_GENX_ void test1()
{
  matrix<int,2,4>   m, src;
  matrix<uchar,2,4> mask;
  // ...
  m.merge(src, mask);
  // m           src         mask      --->  m
  // 2 2 2 2     4 4 4 4     1 1 0 1         4 4 2 4
  // 2 2 2 2     4 4 4 4     0 1 1 0         2 4 4 2

  vector<int, 4> v1, v2;
  int imask = 0xA;
  // ...
  v1.merge(v2, imask);
  // v1          v2          imask     --->  v1
  // 2 2 2 2     4 4 4 4     0xA: 1010       2 4 2 4
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
