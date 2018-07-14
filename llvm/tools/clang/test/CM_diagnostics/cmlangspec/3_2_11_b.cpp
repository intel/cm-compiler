#include <cm/cm.h>

_GENX_ void test1()
{
  matrix<int,4,2>   m, src1, src2;
  matrix<ushort,4,2> mask;
  // ...
  m.merge(src1, src2, mask);
  // m       src_1   src_2   mask   --->   m
  // 2 2     4 4     8 8     1 0           4 8
  // 2 2     4 4     8 8     1 1           4 4
  // 2 2     4 4     8 8     0 1           8 4
  // 2 2     4 4     8 8     0 0           8 8
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
