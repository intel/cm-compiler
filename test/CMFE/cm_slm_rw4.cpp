/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// COM: Checks whether the cm_slm_read4 / write_4 functions are defined according to the spec
// RUN: %cmc -march=skl -emit-llvm -Xclang -verify -- %s

#include <cm/cm.h>

void foo(uint slmBuffer, vector_ref<uint, 8> v_Addr, vector_ref<float, 8> v_Data) {
  cm_slm_read4 (slmBuffer, v_Addr, v_Data, SLM_R_ENABLE); //expected-no-diagnostics
  cm_slm_write4(slmBuffer, v_Addr, v_Data, SLM_R_ENABLE); //expected-no-diagnostics
}
