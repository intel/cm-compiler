/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc  -S -emit-llvm -march=dg2 -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

void test(vector<half, 128> weights, matrix<half, 8, 16> input)
{
   matrix<float, 8, 8> output;
   // Do dpas.8x8 dst:f, acc:f, src[1|2]:hf
   output = cm_dpas<CM_PRECISION_HF, CM_PRECISION_HF, 8, 8>(output.format<float>(), weights.format<int>(), input.format<int>());
}
// CHECK: dpas2
