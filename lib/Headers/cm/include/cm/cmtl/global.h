/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_CMTL_GLOBAL_H
#define CM_CMTL_GLOBAL_H

#include "../cm_common.h"

namespace cmtl {
namespace global {

/* This function loads elements of c-array \p global with offsets from \p offset
 * and returns loaded value in a vector.
 *
 * Arguments:
 *  \p global - pointer to first element of a global c-array of T elements.
 *  \p offset - vector of offsets in number of elements.
 *
 * If \p global doesn't point to a global c-array T[], behavior is undefined.
 * If \p global doesn't have element with provided offset, behavior is undefined.
 *
 * Usage example:
 *  // in global scope
 *  const int data[] = {42, 41, 40, 39, 38, 37, 36, 35};
 *  const int offset_init[] = {0, 7, 3, 4};
 *
 *  // in a kernel
 *  vector<int, 4> offset (offset_init);
 *  vector<int, 4> result = global::load(data, offset);
 *  // result == {42, 35, 39, 38}
 * End of example.
 */
template<typename T, int Width, int N>
CM_INLINE vector<T, Width> load(const T (&global)[N], vector<int, Width> offset) {
  vector<T, Width> res;
#pragma unroll
  for (int i = 0; i < Width; ++i)
    res[i] = global[offset(i)];
  return res;
}

} // namespace global
} // namespace cmtl

#endif // CM_CMTL_GLOBAL_H
