#include <cm/cm.h>

_GENX_ void f() {
  vector<uchar, 2> vi, vo;
  vi(0) = 117;
  vi(1) = 231;
  vo = vi;  // vo(0) = 117, vo(1) = 231

  matrix<uint, 4, 4>      m1, m2;
  matrix_ref<uint, 4, 4>  m3 = m1;
  char c = '?';

  m1 = m2;  // elements of m2 are copied to m1
  m1 = c;   // c is copied to all elements of m1
            // (implicit type conversion char --> uint is performed)
  m2 = m3;  // elements of m1 are copied to m2
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
