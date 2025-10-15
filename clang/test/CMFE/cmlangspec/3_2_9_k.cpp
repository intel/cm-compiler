/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// COM: Just checking that all these constructions are valid
_GENX_ void test1() {
  matrix<int, 1, 1> mrx;
  vector<int, 1> vec;

  printf("%d", mrx[0][0]);
  printf("%d", mrx[0](0));
  printf("%d", mrx(0, 0));

  printf("%d", vec[0]);
  printf("%d", vec(0));
}

// RUN: %cmc -march=pvc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
