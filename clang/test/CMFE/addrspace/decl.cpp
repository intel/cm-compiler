/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=skl -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

// COM: Check that local automatic variable is translated as global variable.
// CHECK: @_ZZ6kernelvE14LocalAutomatic = internal addrspace(3)

uint32_t DefaultGV;
__private uint32_t PrivateGV;

__global vector<unsigned short, 8> GlobalGV;
__constant uint32_t ConstantGV = 5;

_GENX_MAIN_ void kernel() {
  __local uint32_t LocalAutomatic;
  __private uint32_t PrivateAutomatic;
  uint32_t DefaultAutomatic;

  static uint32_t __global GlobalInternal;
  static uint32_t __constant ConstantInternal = 6;
  static uint32_t DefaultIntarnal;
}
