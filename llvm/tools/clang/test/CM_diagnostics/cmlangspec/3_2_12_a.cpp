#include <cm/cm.h>

_GENX_ void test1()
{
  matrix<int, 8, 8> m1;
  // ...
 
  // Vector mask
  vector<ushort, 64> v_mask = (m1 > 0);
  ushort result = v_mask.any();
  if (result) {
    // At least one value in m1 is > 0
    // ...
  }
  if (v_mask.all()) {
    // All values in m1 are > 0
    // ...
  }
  
  // Matrix mask
  matrix<ushort, 8, 8> m_mask = (m1 == 0);
  if (m_mask.all()) {
    // All values in m1 are 0
    // ...
  }
  if ((m1 == 0).all()) {
    // Another way to express the same thing without using an
    // intermediate variable
    // ...
  }
  while ((m1 == 0).any()) {
    // As long as m1 still has a 0, keep looping...
    // ...
  }
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
