#include <cm/cm.h>

_GENX_ void test()
{
#define PAARM_LEVEL2 255

  matrix<short,1,16> tempBuffer;
  matrix<short,1,16> left;
  matrix<short,1,16> right;
  matrix<short,1,16> top;
  matrix<short,1,16> bot;
  // ...

  #pragma cm_nonstrict
  tempBuffer = (PAARM_LEVEL2 * left) + (PAARM_LEVEL2 * right) + (PAARM_LEVEL2 * top) +
  (PAARM_LEVEL2 * bot );
}

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: cm_nonstrict is deprecated
