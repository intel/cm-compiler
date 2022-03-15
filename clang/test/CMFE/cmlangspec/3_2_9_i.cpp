/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

int init_a[] = { 0, 2, 4, 6, 8, 10, 12, 14};
int init_b[] = {0, 2, 3};
int init_c[] = {1, 0, 0};

_GENX_ void test1()
{
  // Here: init_a = {{0, 2}, {4, 6}, {8, 10}, {12, 14}};
  matrix<int, 4, 2> a(init_a);
  // Here: init_b = {0, 2, 3}; init_c = {1, 0, 0}
  vector<int, 3> b(init_b), c(init_c);
  vector<int, 3> out;
  out = a.iselect(b, c); // out = {2, 8, 12}
}

// RUN: %cmc -fcmocl -march=SKL -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
