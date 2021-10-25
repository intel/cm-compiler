/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_bfn.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_BFN_H_
#define _CLANG_CM_BFN_H_

#include "cm_common.h"
#include "cm_internal.h"
#include "cm_has_instr.h"

/// \brief BFNT -- boolean sources enum
///
/// This enum contains BFN_X, BFN_Y and BFN_Z elements
/// corresponding to s0, s1, s2 in documentation
///
enum class BFNT : unsigned char {
  X = 0xAA,
  Y = 0xCC,
  Z = 0xF0
};

constexpr BFNT BFN_X = BFNT::X;
constexpr BFNT BFN_Y = BFNT::Y;
constexpr BFNT BFN_Z = BFNT::Z;

static constexpr BFNT operator~(BFNT x) {
  unsigned char val = static_cast<unsigned char>(x);
  unsigned char res = ~val;
  return static_cast<BFNT>(res);
}

static constexpr BFNT operator|(BFNT x, BFNT y) {
  unsigned char arg0 = static_cast<unsigned char>(x);
  unsigned char arg1 = static_cast<unsigned char>(y);
  unsigned char res = arg0 | arg1;
  return static_cast<BFNT>(res);
}

static constexpr BFNT operator&(BFNT x, BFNT y) {
  unsigned char arg0 = static_cast<unsigned char>(x);
  unsigned char arg1 = static_cast<unsigned char>(y);
  unsigned char res = arg0 & arg1;
  return static_cast<BFNT>(res);
}

static constexpr BFNT operator^(BFNT x, BFNT y) {
  unsigned char arg0 = static_cast<unsigned char>(x);
  unsigned char arg1 = static_cast<unsigned char>(y);
  unsigned char res = arg0 ^ arg1;
  return static_cast<BFNT>(res);
}

/// \brief Boolean Function Calculation
///
/// @param BVAL Function index (from BFNT enum values)
///
/// @param s0 First boolean function arg
///
/// @param s1 Second boolean function arg
///
/// @param s2 Third boolean function arg
///
/// Example: d = cm_bfn<~BFN_X & ~BFN_Y & ~BFN_Z>(s0, s1, s2);
///
template <BFNT BVAL, typename T>
typename std::enable_if<details::is_cm_scalar<T>::value, T>::type
cm_bfn(T s0, T s1, T s2) {
  CM_HAS_BFN_CONTROL; 

  return details::__cm_intrinsic_impl_bfn(s0, s1, s2, static_cast<unsigned char>(BVAL));
}

template <BFNT BVAL, typename T, int N>
vector<T, N> cm_bfn(vector<T, N> s0, vector<T, N> s1, vector<T, N> s2) {
  CM_HAS_BFN_CONTROL;

  return details::__cm_intrinsic_impl_bfn(s0, s1, s2, static_cast<unsigned char>(BVAL));
}

#endif // _CLANG_CM_BFN_H_

