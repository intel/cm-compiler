/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

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
