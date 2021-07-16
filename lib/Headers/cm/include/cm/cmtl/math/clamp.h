/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_CMTL_MATH_CLAMP_H
#define CM_CMTL_MATH_CLAMP_H

#include "../mask.h"
#include "../../cm_traits.h"
#include "../../cm_common.h"

namespace cmtl {
namespace math {

/* clamp: clamps the provided argument arg with boundaries [lo, hi]
 *
 * For each element of provided arguments: if the argument element is greater
 * than the higher bound, higher bound is returned, otherwise if the argument
 * element is less than the lower bound, the lower bound is returned,
 * otherwise the argument element is returned unchanged.
 * One can consider this function as a saturation of the argument with the
 * provided boundaries.
 *
 * low bound (lo) must be less or equal to high bound (hi), otherwise result
 * is unspecified.
 *
 * T must be either integral or floating point value. But for floating point
 * values arguments cannot have NaNs, if so result is unspecified.
 */
template<typename T, int width>
CM_INLINE vector<T, width>
clamp(vector<T, width> arg, vector<T, width> lo, vector<T, width> hi) {
  arg = cm_min<T>(arg, hi);
  arg = cm_max<T>(arg, lo);
  return arg;
}

template<typename T, int width>
CM_INLINE vector<T, width>
clamp(vector<T, width> arg, T lo, T hi) {
  vector<T, width> vec_lo = lo;
  vector<T, width> vec_hi = hi;
  return clamp(arg, vec_lo, vec_hi);
}

template<typename T, int width, int hight>
CM_INLINE matrix<T, hight, width>
clamp(matrix<T, hight, width> arg,
      matrix<T, hight, width> lo,
      matrix<T, hight, width> hi) {
  vector<T, width * hight> vec_arg = arg.format<T>();
  vector<T, width * hight> vec_lo = lo.format<T>();
  vector<T, width * hight> vec_hi = hi.format<T>();
  return clamp(vec_arg, vec_lo, vec_hi);
}

template<typename T, int width, int hight>
CM_INLINE matrix<T, hight, width>
clamp(matrix<T, hight, width> arg, T lo, T hi) {
  matrix<T, hight, width> mtx_lo = lo;
  matrix<T, hight, width> mtx_hi = hi;
  return clamp(arg, mtx_lo, mtx_hi);
}

template<typename T>
CM_INLINE T clamp(T arg, T lo, T hi) {
  vector<T, 1> vec_arg = arg;
  vector<T, 1> vec_lo = lo;
  vector<T, 1> vec_hi = hi;
  vector<T, 1> vec_res = clamp(vec_arg, vec_lo, vec_hi);
  return vec_res[0];
}

} // namespace math
} // namespace cmtl
#endif // CM_CMTL_MATH_CLAMP_H
