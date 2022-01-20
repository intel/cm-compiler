/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>
#include <cm/cmtl.h>

_GENX_ void test1(int i)
{
  vector<ushort, 16> v;
  // ... 
  cmtl::cm_vector_assign(v.select<10,1>(2), i, 3);
  // ...
}

// RUN: %cmc -march=SKL -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
