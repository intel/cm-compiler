/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_SPIRV_BARRIER_H_
#define _CLANG_CM_SPIRV_BARRIER_H_

// SPIR-V barrier instructions:
// https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#_barrier_instructions

namespace detail {
namespace spirv {
enum scope {
  cross_device = 0,
  device = 1,
  workgroup = 2,
  subgroup = 3,
  invocation = 4
};
} // namespace spirv
} // namespace detail

void __spirv_ControlBarrier(int scope, int memory_scope, int memory_semantics);

// SPV_INTEL_split_barrier extension
void __spirv_ControlBarrierArriveINTEL(int scope, int memory_scope,
                                       int memory_semantics);
void __spirv_ControlBarrierWaitINTEL(int scope, int memory_scope,
                                     int memory_semantics);

#endif // _CLANG_CM_SPIRV_BARRIER_H_
