/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

_GENX_ void test1()
{
  matrix<float, 4, 8>  m1;
  matrix<float, 2, 2>  m2;
  // ...
  m2 = m1.select<2, 2, 2, 4>(1, 2);  // v_size = 2, v_stride = 2,
                                     // h_size = 2, h_stride = 4,
                                     // v_offset = 1, h_offset = 2.
}

// RUN: %cmc -fcmocl -march=SKL -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
