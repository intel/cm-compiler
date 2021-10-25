/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

_GENX_ void test()
{
#define PAARM_LEVEL2 255

  matrix<short,1,16> tempBuffer;
  matrix<short,1,16> left;
  matrix<short,1,16> right;
  matrix<short,1,16> top;
  matrix<short,1,16> bot;
  // ...

  #pragma cm_nonstrict
  tempBuffer = (PAARM_LEVEL2 * left) + (PAARM_LEVEL2 * right) + (PAARM_LEVEL2 * top) +
  (PAARM_LEVEL2 * bot );
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck --implicit-check-not error %s
// CHECK: warning: cm_nonstrict is deprecated
