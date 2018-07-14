#include <cm/cm.h>

_GENX_ void test1()
{
  matrix<int, 4, 4>       m1;
  // ...
  matrix_ref<char, 4, 16> m2 = m1.format<char, 4, 16>( );
  // m2 is a reference to the location of m1
  // interpreted as a matrix 4x16 of chars.
  matrix_ref<int, 2, 8>   m3 = m1.format<int, 2, 8>( );
  // m3 is a reference to the location of m1
  // interpreted as a 2x8 integer matrix.
  // (assuming little endian layout, i.e.
  //  the lowest byte address of element
  //  m1(0,0) is referenced by m2(0,0))

  matrix<float, 2, 8>  m4;
  // ...
  vector_ref<float, 16> v1 = m4.format<float>();
  // v1 is a reference to the location of m4
  // interpreted as a vector of 16 floats.
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
