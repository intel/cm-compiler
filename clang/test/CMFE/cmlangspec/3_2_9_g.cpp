/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// expected-no-diagnostics

#include <cm/cm.h>

//A template function that multiplies an arbitrary matrix by 3
template <typename T, uint R, uint C>
_GENX_ inline void
mult3(matrix_ref<T, R, C> par)
{
  par = par * 3;
}
   
_GENX_MAIN_ void
kern(matrix<int, 4, 2> p)
{
  matrix<int, 4, 2> m(p); // m = p;
  mult3(m.select_all());  // m = p * 3;
}

// RUN: %cmc -emit-llvm -march=SKL -Xclang -verify -- %s
