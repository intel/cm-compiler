/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -fcm-pointer -march=skl -emit-llvm -o %t.bc -Xclang -verify -- %s

#include <cm/cm.h>

_GENX_MAIN_ void kernelA(__private float *ptr) { // expected-error{{unsupported function parameter type '__private float *'}}
}

_GENX_MAIN_ void kernelB(__generic float *ptr) { // expected-error{{unsupported function parameter type '__generic float *'}}
}
