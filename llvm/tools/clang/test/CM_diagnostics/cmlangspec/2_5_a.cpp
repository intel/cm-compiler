void foo() {
  matrix<float, 8, 4> m1;
  matrix<float, 2, 2> m2;

  vector_ref<float, 4> v1(m1.row(2));
  m2 = v1;
  // ...
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
