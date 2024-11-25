/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

_GENX_ void test1()
{
  matrix<uint, 4, 4> m;
  vector<uint, 4>    v;
  // ...
  v = m.row(2);         // the 2nd row of m is copied to v
  m.column(3) = 0;      // the 3rd column of m are replaced with 0.
}

// RUN: %cmc -march=SKL -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
