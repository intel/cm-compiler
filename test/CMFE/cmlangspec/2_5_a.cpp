/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

void foo() {
  matrix<float, 8, 4> m1;
  matrix<float, 2, 2> m2;

  vector_ref<float, 4> v1(m1.row(2));
  m2 = v1;
  // ...
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
