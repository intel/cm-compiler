#include <cm/cm.h>

_GENX_ void test1()
{
  matrix<int, 8, 8>    m1, m2;
  matrix<ushort, 8, 8> m_mask;
  vector<int, 4>       v;
  vector<ushort, 4>    v_mask;

  m_mask = ( m1 >= m2 );
  v_mask = ( v != 0 );
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
