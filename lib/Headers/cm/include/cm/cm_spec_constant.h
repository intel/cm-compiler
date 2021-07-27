/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(
    0, "CM:w:cm_spec_constant.h should not be included explicitly - only "
       "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_SPEC_CONSTANT_H_
#define _CLANG_CM_SPEC_CONSTANT_H_

// Transforms to OpSpecConstant in SPIR-V
template <typename T> T __spirv_SpecConstant(uint32_t, T);

/// \brief Declare a new integer-type scalar specialization constant.
///
/// This version supports only integral types.
///
/// @param T Specialization constant type.
///
/// @param ID Specialization constant id.
///
/// @param DefaultValue The specialization constant value if it is not set by
/// runtime.
///
template <typename T, uint32_t ID, T DefaultValue = T{0}>
CM_NODEBUG CM_INLINE std::enable_if_t<
    details::is_cm_scalar<T>::value && std::is_integral<T>::value, T>
cm_spec_constant() {
  return __spirv_SpecConstant(ID, DefaultValue);
}

/// \brief Declare a new floating-point-type scalar specialization constant.
///
/// This version supports only floating-point types types. The default
/// specialization constant value is always zero.
///
/// @param T Specialization constant type.
///
/// @param ID Specialization constant id.
///
template <typename T, uint32_t ID>
CM_NODEBUG CM_INLINE std::enable_if_t<
    details::is_cm_scalar<T>::value && std::is_floating_point<T>::value, T>
cm_spec_constant() {
  return __spirv_SpecConstant(ID, T{0});
}

#endif
