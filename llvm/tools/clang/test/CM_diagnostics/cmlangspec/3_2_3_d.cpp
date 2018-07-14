#include <cm/cm.h>
#include <cm/cmtl.h>

_GENX_ void test1(int i)
{
  vector<ushort, 16> v;
  // ... 
  cmtl::cm_vector_assign(v.select<10,1>(2), i, 3);
  // ...
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
