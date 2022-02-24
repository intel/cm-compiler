/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// expected-no-diagnostics

#include <cm/cm.h>

_GENX_MAIN_ void printf_demo_genx()
{
  int tx = get_thread_origin_x();
  int ty = get_thread_origin_y();

  printf("Number of bytes returned: %d\n", printf("Hello CM from %dx%d\n", tx, ty));

  double foo = 4.2;
  printf("Print a double: %f\n", (double)foo);

  printf("Print a string: %s\n", "Hello");

  if (tx == 0 && ty == 0) // Only print on tid 0
  {
    printf("Hello from tid 0 with tx/ty %d/%d \n", tx, ty);
  }
}

// RUN: %cmc -emit-llvm -march=SKL -Xclang -verify -- %s
