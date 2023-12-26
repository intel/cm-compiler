/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_SPIRV_OPENCL_H_
#define _CLANG_CM_SPIRV_OPENCL_H_

// SPIR-V OpenCL extended instruction set:
// https://registry.khronos.org/SPIR-V/specs/unified1/OpenCL.ExtendedInstructionSet.100.html

#define _DECL_OCL_OP1(name, type)                                              \
  type __spirv_ocl_##name(type);                                               \
  template <int Width>                                                         \
  vector<type, Width> __spirv_ocl_##name(vector<type, Width>);

#define _DECL_OCL_OP2(name, type)                                              \
  type __spirv_ocl_##name(type, type);                                         \
  template <int Width>                                                         \
  vector<type, Width> __spirv_ocl_##name(vector<type, Width>,                  \
                                         vector<type, Width>);


_DECL_OCL_OP1(native_cos, float)
_DECL_OCL_OP1(native_cos, half)
_DECL_OCL_OP1(native_exp2, float)
_DECL_OCL_OP1(native_exp2, half)
_DECL_OCL_OP1(native_log2, float)
_DECL_OCL_OP1(native_log2, half)
_DECL_OCL_OP1(native_recip, float)
_DECL_OCL_OP1(native_recip, half)
_DECL_OCL_OP1(native_rsqrt, float)
_DECL_OCL_OP1(native_rsqrt, half)
_DECL_OCL_OP1(native_sin, float)
_DECL_OCL_OP1(native_sin, half)
_DECL_OCL_OP1(native_sqrt, float)
_DECL_OCL_OP1(native_sqrt, half)

_DECL_OCL_OP2(native_powr, float)
_DECL_OCL_OP2(native_powr, half)

#ifdef CM_HAS_BF16
_DECL_OCL_OP1(native_cos, __bf16)
_DECL_OCL_OP1(native_exp2, __bf16)
_DECL_OCL_OP1(native_log2, __bf16)
_DECL_OCL_OP1(native_recip, __bf16)
_DECL_OCL_OP1(native_rsqrt, __bf16)
_DECL_OCL_OP1(native_sin, __bf16)
_DECL_OCL_OP1(native_sqrt, __bf16)

_DECL_OCL_OP2(native_powr, __bf16)
#endif // CM_HAS_BF16

#undef _DECL_OCL_OP1
#undef _DECL_OCL_OP2

#endif // _CLANG_CM_SPIRV_OPENCL_H_
