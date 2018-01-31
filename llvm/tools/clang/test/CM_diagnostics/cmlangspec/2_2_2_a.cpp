#include <cm/cm.h>

_GENX_ void f() {
  matrix<short, 4, 8> m;
  vector_ref<short, 8> r(m.row(2)); // row() returns the reference to a row
}

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: No CM kernel definitions found
// CHECK: 1 warning generated
