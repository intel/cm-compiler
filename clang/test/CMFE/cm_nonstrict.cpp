/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

#define C 255

_GENX_MAIN_ void foo(SurfaceIndex S, short a, short b, short c, short d)
{
  matrix<short,1,16> tempBuffer;
  #pragma cm_nonstrict // expected-warning{{cm_nonstrict is deprecated - see CM LLVM Porting Guide}}
  tempBuffer = (C * a) + (C * b) + (C * c) + (C * d);
  write(S, 0, 0, tempBuffer);
}


// warning expected for use of #pragma cm_nonstrict
// RUN: %cmc -emit-llvm -march=SKL -Xclang -verify -Xclang -verify-ignore-unexpected -- %s
//
