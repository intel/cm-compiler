/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cmtl.h>

_GENX_ void test1()
{
  cm_vector(v, ushort, 16, 2, 3);
  // ...
}

// RUN: %cmc -march=pvc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
