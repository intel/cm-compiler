// For some reason this test fails if the RUN lines are at the
// end of the file - why?
// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa

#include <cm/cm.h>

_GENX_ void test1()
{
  ushort offsetx = 0, offsety = 0, sizex = 0;

  vector<int, 16> cond;
  vector<int, 16> v;
  // ...
  SIMD_IF_BEGIN (cond) {
    // ...
  } SIMD_ELSE {
    SIMD_IF_BEGIN (cond < 0) {
      vector<int, 16> local;
      // ...
      local = v.select<16, 1>(offsetx + offsety * sizex);
      // ...
    } SIMD_ELSE {
      // ...
    } SIMD_IF_END;
  } SIMD_IF_END;
  // ...
  SIMD_DO_WHILE_BEGIN {
    vector<int, 16> local;
    // ...
    SIMD_IF_BEGIN (local < 0) {
      SIMD_BREAK;
    } SIMD_IF_END;
    // ...
  } SIMD_DO_WHILE_END (cond < 32);
  // ...
}
