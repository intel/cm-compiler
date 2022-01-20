/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

_GENX_ void f() {
  vector<uchar, 2> vi;
  vi(0) = 117;
  vi(1) = 231;
  vector<uchar, 2> vo(vi); // vo(0) = 117, vo(1) = 231
}

// RUN: %cmc -march=SKL -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
