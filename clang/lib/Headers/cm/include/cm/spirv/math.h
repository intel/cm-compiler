/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_SPIRV_MATH_H_
#define _CLANG_CM_SPIRV_MATH_H_

// SPIR-V Kernel built-ins are available as function calls.
// See
// https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#_arithmetic_instructions

namespace details {

template <typename T> struct ResScalar {
  T Res;
  T C;
};

template <typename T, int Width> struct ResVector {
  vector<T, Width> Res;
  vector<T, Width> C;
};

} // namespace details

template <typename T> details::ResScalar<T> __spirv_IAddCarry(T A, T B);

template <typename T, int Width>
details::ResVector<T, Width> __spirv_IAddCarry(vector<T, Width> A,
                                               vector<T, Width> B);

template <typename T> details::ResScalar<T> __spirv_ISubBorrow(T A, T B);

template <typename T, int Width>
details::ResVector<T, Width> __spirv_ISubBorrow(vector<T, Width> A,
                                                vector<T, Width> B);

template <typename T> T __spirv_FRem(T A, T B);

#endif // _CLANG_CM_SPIRV_BUILTINS_H_
