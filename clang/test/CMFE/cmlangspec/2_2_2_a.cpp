#include <cm/cm.h>

_GENX_ void f() {
  matrix<short, 4, 8> m;
  vector_ref<short, 8> r(m.row(2)); // row() returns the reference to a row
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
