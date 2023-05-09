/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -fcm-pointer -march=skl -emit-llvm -o %t.bc -- %s

#include <cm/cm.h>

_GENX_MAIN_ void kernel(float *svmptr [[type("svmptr_t")]],
                        __local uint32_t *l_ptr,
                        __global float *g_ptr,
                        __constant float *c_ptr) {
}
