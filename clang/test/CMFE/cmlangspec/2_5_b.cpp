/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

void foo()
{
  vector<float, 8> f;
  vector<int, 8> i;
  // ...
  f = i;                    // implicit type conversion
  f = vector<short, 8>(i);  // explicit type conversion
  // ...
}

// RUN: %cmc -march=SKL -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
