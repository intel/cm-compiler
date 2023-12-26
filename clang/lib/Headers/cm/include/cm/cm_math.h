/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_math.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef CM_MATH_H
#define CM_MATH_H

#include "cm_abs.h"
#include "cm_common.h"
#include "cm_has_instr.h"
#include "cm_internal.h"
#include "cm_traits.h"

#include "spirv/ocl.h"


////////////////////////////////////////////////////////////////////////////////
// CM arithmetic intrinsics:
//
// inv, log, exp, sqrt, rsqrt, sin, cos, pow, atan, acos, asin, div_ieee,
// sqrt_ieee
//
// share the same requirements.
//
// template <int SZ>
// vector<float, SZ>
// CM_INLINE cm_inv(vector<float, SZ> src0, int flag = _GENX_NOSAT) {
//   vector<float, SZ> _Result = details::__cm_intrinsic_impl_inv(src0);
//   if (flag != _GENX_SAT)
//     return _Result;
//   return details::__cm_intrinsic_impl_sat<float>(_Result);
// }
//
// template <int N1, int N2>
// CM_NODEBUG CM_INLINE
// vector<float, N1 * N2>
// cm_inv(matrix<float, N1, N2> src0, int flag = _GENX_NOSAT) {
//   vector<float, N1 * N2> _Src0 = src0;
//   return cm_inv(_Src0, flag);
// }
//
// CM_INLINE float cm_inv(float src0, int flag = _GENX_NOSAT) {
//   vector<float, 1> _Src0 = src0;
//   vector<float, 1> _Result = cm_inv(_Src0, flag);
//   return _Result(0);
// }
//
// we also make the scalar version template-based by adding
// a "typename T". Since the type can only be float, we hack it
// by defining T=void without instantiating it to be float.

// sqrt_ieee
#define _CM_INTRINSIC_WITH_CONTROL_DEF(type, name, control)                    \
  template <int SZ>                                                            \
  CM_NODEBUG CM_INLINE vector<type, SZ> cm_##name(vector<type, SZ> src0,       \
                                                  int flag = _GENX_NOSAT) {    \
    vector<type, SZ> _Result = details::__cm_intrinsic_impl_##name(src0);      \
    control;                                                                   \
    if (flag != _GENX_SAT)                                                     \
      return _Result;                                                          \
    return details::__cm_intrinsic_impl_sat<type>(_Result);                    \
  }                                                                            \
  template <int N1, int N2>                                                    \
  CM_NODEBUG CM_INLINE vector<type, N1 * N2> cm_##name(                        \
      matrix<type, N1, N2> src0, int flag = _GENX_NOSAT) {                     \
    vector<type, N1 *N2> _Src0 = src0;                                         \
    return cm_##name(_Src0, flag);                                             \
  }                                                                            \
  template <typename T = void>                                                 \
  CM_NODEBUG CM_INLINE type cm_##name(type src0, int flag = _GENX_NOSAT) {     \
    vector<type, 1> _Src0 = src0;                                              \
    vector<type, 1> _Result = cm_##name(_Src0, flag);                          \
    return _Result(0);                                                         \
  }
#define _CM_INTRINSIC_DEF(type, name)                                          \
  _CM_INTRINSIC_WITH_CONTROL_DEF(type, name, 0)

_CM_INTRINSIC_WITH_CONTROL_DEF(float, sqrt_ieee, CM_HAS_IEEE_DIV_SQRT_CONTROL)
_CM_INTRINSIC_DEF(double, sqrt_ieee)

#undef _CM_INTRINSIC_DEF
#undef _CM_INTRINSIC_WITH_CONTROL_DEF

// inv, log, exp, sqrt, rsqrt, sin, cos
#define _CM_INTRINSIC_DEF(type, name, native)                                  \
  template <typename T = void>                                                 \
  CM_NODEBUG CM_INLINE type cm_##name(type src0, int flag = _GENX_NOSAT) {     \
    type _Result = __spirv_ocl_native_##native(src0);                          \
    if (flag != _GENX_SAT)                                                     \
      return _Result;                                                          \
    return details::__cm_intrinsic_impl_sat<type>(_Result);                    \
  }                                                                            \
  template <int SZ>                                                            \
  CM_NODEBUG CM_INLINE vector<type, SZ> cm_##name(vector<type, SZ> src0,       \
                                                  int flag = _GENX_NOSAT) {    \
    if constexpr (SZ == 1)                                                     \
      return cm_##name(src0[0], flag);                                         \
    vector<type, SZ> _Result = __spirv_ocl_native_##native(src0);              \
    if (flag != _GENX_SAT)                                                     \
      return _Result;                                                          \
    return details::__cm_intrinsic_impl_sat<type>(_Result);                    \
  }                                                                            \
  template <int N1, int N2>                                                    \
  CM_NODEBUG CM_INLINE vector<type, N1 * N2> cm_##name(                        \
      matrix<type, N1, N2> src0, int flag = _GENX_NOSAT) {                     \
    vector<type, N1 *N2> _Src0 = src0;                                         \
    return cm_##name(_Src0, flag);                                             \
  }

_CM_INTRINSIC_DEF(half, inv, recip)
_CM_INTRINSIC_DEF(half, log, log2)
_CM_INTRINSIC_DEF(half, exp, exp2)
_CM_INTRINSIC_DEF(half, sqrt, sqrt)
_CM_INTRINSIC_DEF(half, rsqrt, rsqrt)
_CM_INTRINSIC_DEF(half, sin, sin)
_CM_INTRINSIC_DEF(half, cos, cos)

_CM_INTRINSIC_DEF(float, inv, recip)
_CM_INTRINSIC_DEF(float, log, log2)
_CM_INTRINSIC_DEF(float, exp, exp2)
_CM_INTRINSIC_DEF(float, sqrt, sqrt)
_CM_INTRINSIC_DEF(float, rsqrt, rsqrt)
_CM_INTRINSIC_DEF(float, sin, sin)
_CM_INTRINSIC_DEF(float, cos, cos)

#ifdef CM_HAS_BF16
_CM_INTRINSIC_DEF(__bf16, inv, recip)
_CM_INTRINSIC_DEF(__bf16, log, log2)
_CM_INTRINSIC_DEF(__bf16, exp, exp2)
_CM_INTRINSIC_DEF(__bf16, sqrt, sqrt)
_CM_INTRINSIC_DEF(__bf16, rsqrt, rsqrt)
_CM_INTRINSIC_DEF(__bf16, sin, sin)
_CM_INTRINSIC_DEF(__bf16, cos, cos)
#endif // CM_HAS_BF16

#undef _CM_INTRINSIC_DEF

// div_ieee
#define _CM_INTRINSIC_WITH_CONTROL_DEF(ftype, name, control)                   \
  template <int SZ, typename U>                                                \
  CM_NODEBUG CM_INLINE vector<ftype, SZ> cm_##name(                            \
      vector<ftype, SZ> src0, U src1, int flag = _GENX_NOSAT) {                \
    control;                                                                   \
    vector<ftype, SZ> _Src1 = src1;                                            \
    vector<ftype, SZ> _Result =                                                \
        details::__cm_intrinsic_impl_##name(src0, _Src1);                      \
    if (flag != _GENX_SAT)                                                     \
      return _Result;                                                          \
                                                                               \
    return details::__cm_intrinsic_impl_sat<ftype>(_Result);                   \
  }                                                                            \
  template <int N1, int N2, typename U>                                        \
  CM_NODEBUG CM_INLINE vector<ftype, N1 * N2> cm_##name(                       \
      matrix<ftype, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {            \
    vector<ftype, N1 *N2> _Src0 = src0;                                        \
    vector<ftype, N1 *N2> _Src1 = src1;                                        \
    return cm_##name(_Src0, _Src1, flag);                                      \
  }                                                                            \
  template <int SZ, typename U>                                                \
  CM_NODEBUG CM_INLINE                                                         \
      typename std::enable_if<details::is_cm_scalar<U>::value,                 \
                              vector<ftype, SZ> >::type                        \
          cm_##name(U src0, vector<ftype, SZ> src1, int flag = _GENX_NOSAT) {  \
    vector<ftype, SZ> _Src0 = src0;                                            \
    return cm_##name(_Src0, src1, flag);                                       \
  }                                                                            \
  template <int N1, int N2, typename U>                                        \
  CM_NODEBUG CM_INLINE                                                         \
      typename std::enable_if<details::is_cm_scalar<U>::value,                 \
                              vector<ftype, N1 * N2> >::type                   \
          cm_##name(U src0, matrix<ftype, N1, N2> src1,                        \
                    int flag = _GENX_NOSAT) {                                  \
    vector<ftype, N1 *N2> _Src0 = src0;                                        \
    vector<ftype, N1 *N2> _Src1 = src1;                                        \
    return cm_##name(_Src0, _Src1, flag);                                      \
  }                                                                            \
  template <typename T = void>                                                 \
  CM_NODEBUG CM_INLINE ftype cm_##name(ftype src0, ftype src1,                 \
                                       int flag = _GENX_NOSAT) {               \
    vector<ftype, 1> _Src0 = src0;                                             \
    vector<ftype, 1> _Src1 = src1;                                             \
    vector<ftype, 1> _Result = cm_##name(_Src0, _Src1, flag);                  \
    return _Result(0);                                                         \
  }
#define _CM_INTRINSIC_DEF(ftype, name)                                         \
  _CM_INTRINSIC_WITH_CONTROL_DEF(ftype, name, 0)

_CM_INTRINSIC_WITH_CONTROL_DEF(float, div_ieee, CM_HAS_IEEE_DIV_SQRT_CONTROL)
_CM_INTRINSIC_DEF(double, div_ieee)

#undef _CM_INTRINSIC_DEF
#undef _CM_INTRINSIC_WITH_CONTROL_DEF

// pow
#define _CM_INTRINSIC_DEF(ftype, name, native)                                 \
  template <typename T = void>                                                 \
  CM_NODEBUG CM_INLINE ftype cm_##name(ftype src0, ftype src1,                 \
                                       int flag = _GENX_NOSAT) {               \
    ftype _Result = __spirv_ocl_native_##native(src0, src1);                   \
    if (flag != _GENX_SAT)                                                     \
      return _Result;                                                          \
    return details::__cm_intrinsic_impl_sat<ftype>(_Result);                   \
  }                                                                            \
  template <int SZ, typename U>                                                \
  CM_NODEBUG CM_INLINE vector<ftype, SZ> cm_##name(                            \
      vector<ftype, SZ> src0, U src1, int flag = _GENX_NOSAT) {                \
    if constexpr (SZ == 1)                                                     \
      return cm_##name(src0[0], src1, flag);                                   \
    vector<ftype, SZ> _Src1 = src1;                                            \
    vector<ftype, SZ> _Result = __spirv_ocl_native_##native(src0, _Src1);      \
    if (flag != _GENX_SAT)                                                     \
      return _Result;                                                          \
    return details::__cm_intrinsic_impl_sat<ftype>(_Result);                   \
  }                                                                            \
  template <int N1, int N2, typename U>                                        \
  CM_NODEBUG CM_INLINE vector<ftype, N1 * N2> cm_##name(                       \
      matrix<ftype, N1, N2> src0, U src1, int flag = _GENX_NOSAT) {            \
    vector<ftype, N1 *N2> _Src0 = src0;                                        \
    vector<ftype, N1 *N2> _Src1 = src1;                                        \
    return cm_##name(_Src0, _Src1, flag);                                      \
  }                                                                            \
  template <int SZ, typename U>                                                \
  CM_NODEBUG CM_INLINE                                                         \
      typename std::enable_if<details::is_cm_scalar<U>::value,                 \
                              vector<ftype, SZ> >::type                        \
          cm_##name(U src0, vector<ftype, SZ> src1, int flag = _GENX_NOSAT) {  \
    vector<ftype, SZ> _Src0 = src0;                                            \
    return cm_##name(_Src0, src1, flag);                                       \
  }                                                                            \
  template <int N1, int N2, typename U>                                        \
  CM_NODEBUG CM_INLINE                                                         \
      typename std::enable_if<details::is_cm_scalar<U>::value,                 \
                              vector<ftype, N1 * N2> >::type                   \
          cm_##name(U src0, matrix<ftype, N1, N2> src1,                        \
                    int flag = _GENX_NOSAT) {                                  \
    vector<ftype, N1 *N2> _Src0 = src0;                                        \
    vector<ftype, N1 *N2> _Src1 = src1;                                        \
    return cm_##name(_Src0, _Src1, flag);                                      \
  }

_CM_INTRINSIC_DEF(half, pow, powr)
_CM_INTRINSIC_DEF(float, pow, powr)

#ifdef CM_HAS_BF16
_CM_INTRINSIC_DEF(__bf16, pow, powr)
#endif // CM_HAS_BF16

#undef _CM_INTRINSIC_DEF

// cm_sincos
template <int SZ, typename U>
CM_NODEBUG CM_INLINE vector<float, SZ>
cm_sincos(vector_ref<float, SZ> dstcos, U src0, int flag = _GENX_NOSAT) {
  dstcos = cm_cos(src0, flag);
  return cm_sin(src0, flag);
}

template <int N1, int N2, typename U>
CM_NODEBUG CM_INLINE matrix<float, N1, N2>
cm_sincos(matrix_ref<float, N1, N2> dstcos, U src0, int flag = _GENX_NOSAT) {
  dstcos = cm_cos(src0, flag);
  return cm_sin(src0, flag);
}

template <int SZ, typename U>
CM_NODEBUG CM_INLINE vector<half, SZ>
cm_sincos(vector_ref<half, SZ> dstcos, U src0, int flag = _GENX_NOSAT) {
  dstcos = cm_cos(src0, flag);
  return cm_sin(src0, flag);
}

template <int N1, int N2, typename U>
CM_NODEBUG CM_INLINE matrix<half, N1, N2>
cm_sincos(matrix_ref<half, N1, N2> dstcos, U src0, int flag = _GENX_NOSAT) {
  dstcos = cm_cos(src0, flag);
  return cm_sin(src0, flag);
}

// cm_atan

#define _CM_HDR_CONST_PI 3.1415926535897932384626433832795

template <typename T, int SZ>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_float_or_half<T>::value,
                            vector<T, SZ> >::type
    cm_atan(vector<T, SZ> src0, int flag = _GENX_NOSAT) {
  vector<T, SZ> _Src0 = cm_abs(src0);

  vector<ushort, SZ> _Neg = src0 < T(0.0);
  vector<ushort, SZ> _Gt1 = _Src0 > T(1.0);

  _Src0.merge(cm_inv(_Src0), _Gt1);

  vector<T, SZ> _Src0P2 = _Src0 * _Src0;
  vector<T, SZ> _Src0P4 = _Src0P2 * _Src0P2;

  vector<T, SZ> _Result =
      (_Src0P4 * T(0.185696) + ((_Src0 * T(0.787997) + T(0.63693)) * _Src0P2) +
       _Src0) /
      (((((_Src0 * -T(0.000121387) + T(0.00202308)) * _Src0P2) +
         (_Src0 * -T(0.0149145)) + T(0.182569)) *
        _Src0P4) +
       ((_Src0 * T(0.395889) + T(1.12158)) * _Src0P2) + (_Src0 * T(0.636918)) +
       T(1.0));

  _Result.merge(T(_CM_HDR_CONST_PI / 2.0) - _Result, _Gt1);
  _Result.merge(-_Result, _Neg);

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_float_or_half<T>::value,
                            vector<T, N1 * N2> >::type
    cm_atan(matrix<T, N1, N2> src0, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src0 = src0;
  return cm_atan(_Src0, flag);
}

template <typename T>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_float_or_half<T>::value, T>::type
    cm_atan(T src0, int flag = _GENX_NOSAT) {
  vector<T, 1> _Src0 = src0;
  vector<T, 1> _Result = cm_atan(_Src0, flag);
  return _Result(0);
}

// cm_acos

template <typename T, int SZ>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_float_or_half<T>::value,
                            vector<T, SZ> >::type
    cm_acos(vector<T, SZ> src0, int flag = _GENX_NOSAT) {
  vector<T, SZ> _Src0 = cm_abs(src0);

  vector<ushort, SZ> _Neg = src0 < T(0.0);
  vector<ushort, SZ> _TooBig = _Src0 >= T(0.999998);

  // Replace oversized values to ensure no possibility of sqrt of
  // a negative value later
  _Src0.merge(T(0.0), _TooBig);

  vector<T, SZ> _Src01m = T(1.0) - _Src0;

  vector<T, SZ> _Src0P2 = _Src01m * _Src01m;
  vector<T, SZ> _Src0P4 = _Src0P2 * _Src0P2;

  vector<T, SZ> _Result =
      (((_Src01m * T(0.015098965761299077) - T(0.005516443930088506)) *
        _Src0P4) +
       ((_Src01m * T(0.047654245891495528) + T(0.163910606547823220)) *
        _Src0P2) +
       _Src01m * T(2.000291665285952400) - T(0.000007239283986332)) *
      cm_rsqrt(_Src01m * T(2.0));

  _Result.merge(T(0.0), _TooBig);
  _Result.merge(T(_CM_HDR_CONST_PI) - _Result, _Neg);

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_float_or_half<T>::value,
                            vector<T, N1 * N2> >::type
    cm_acos(matrix<T, N1, N2> src0, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src0 = src0;
  return cm_acos(_Src0, flag);
}

template <typename T>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_float_or_half<T>::value, T>::type
    cm_acos(T src0, int flag = _GENX_NOSAT) {
  vector<T, 1> _Src0 = src0;
  vector<T, 1> _Result = cm_acos(_Src0, flag);
  return _Result(0);
}

// cm_asin

template <typename T, int SZ>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_float_or_half<T>::value,
                            vector<T, SZ> >::type
    cm_asin(vector<T, SZ> src0, int flag = _GENX_NOSAT) {
  vector<ushort, SZ> _Neg = src0 < T(0.0);

  vector<T, SZ> _Result = T(_CM_HDR_CONST_PI / 2.0) - cm_acos(cm_abs(src0));

  _Result.merge(-_Result, _Neg);

  if (flag != _GENX_SAT)
    return _Result;

  return details::__cm_intrinsic_impl_sat<T>(_Result);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_float_or_half<T>::value,
                            vector<T, N1 * N2> >::type
    cm_asin(matrix<T, N1, N2> src0, int flag = _GENX_NOSAT) {
  vector<T, N1 *N2> _Src0 = src0;
  return cm_asin(_Src0, flag);
}

template <typename T>
CM_NODEBUG CM_INLINE
    typename std::enable_if<details::is_float_or_half<T>::value, T>::type
    cm_asin(T src0, int flag = _GENX_NOSAT) {
  vector<T, 1> _Src0 = src0;
  vector<T, 1> _Result = cm_asin(_Src0, flag);
  return _Result(0);
}

#endif // CM_MATH_H
