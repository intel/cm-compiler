/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
