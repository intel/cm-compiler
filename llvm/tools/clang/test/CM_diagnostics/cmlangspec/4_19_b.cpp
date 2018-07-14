#include <cm/cm.h>

_GENX_ void test()
{
  int opacity = 0;
  matrix<uchar, 4, 16> fbitmap1;
  matrix<uchar, 4, 16> Lval;
  matrix<short, 4, 16> Hval;

#pragma cm_nonstrict
{
  matrix<short, 4, 16> temp_matrix = (Lval * (128 - opacity) +  Hval * opacity) >> 7;
  fbitmap1 = temp_matrix;
}

}

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: cm_nonstrict is deprecated
