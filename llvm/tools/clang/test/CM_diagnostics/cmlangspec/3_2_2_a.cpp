#include <cm/cm.h>

_GENX_ void f() {
  vector<uchar, 2> vi;
  vi(0) = 117;
  vi(1) = 231;
  vector<uchar, 2> vo(vi); // vo(0) = 117, vo(1) = 231
}

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: No CM kernel definitions found
// CHECK: 1 warning generated
