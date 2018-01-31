#include <cm/cm.h>

_GENX_ void f() {
  vector<int, 2> vi;
  vector<uchar, 2> vo;
  vi(0) = 155;
  vi(1) = 275;
  vo = vector<uchar, 2>(vi);        // vo(0) = 155, vo(1) = 20
  vo = vector<uchar, 2>(vi, SAT);   // vo(0) = 155, vo(1) = 255
}

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: No CM kernel definitions found
// CHECK: 1 warning generated
