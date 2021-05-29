#include <cm/cm.h>

_GENX_ void f() {
  vector<uchar, 2> vi;
  vi(0) = 117;
  vi(1) = 231;
  vector<uchar, 2> vo(vi); // vo(0) = 117, vo(1) = 231
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
