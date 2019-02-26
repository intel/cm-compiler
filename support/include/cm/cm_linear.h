/*
 * Copyright (c) 2019, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_linear.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_LINEAR_H_
#define _CLANG_CM_LINEAR_H_

////////////////////////////////////////////////////////////////////////////////
// Size and group intrinsics
// These use builtins underneath to calculate appropriate values
//
// Defined in a separate header like this as they are used extensively in other
// headers and need to be defined early on in the header stack
////////////////////////////////////////////////////////////////////////////////

CM_INLINE uint cm_linear_local_id() {
  return cm_local_size(0) * cm_local_size(1) * cm_local_id(2) +
         cm_local_size(0) * cm_local_id(1) + cm_local_id(0);
}

CM_INLINE uint cm_linear_local_size() {
  return cm_local_size(0) * cm_local_size(1) * cm_local_size(2);
}

CM_INLINE uint cm_linear_group_id() {
  return cm_group_count(0) * cm_group_count(1) * cm_group_id(2) +
         cm_group_count(0) * cm_group_id(1) + cm_group_id(0);
}

CM_INLINE uint cm_linear_group_count() {
  return cm_group_count(0) * cm_group_count(1) * cm_group_count(2);
}

CM_INLINE uint cm_linear_global_id() {
  return cm_linear_group_id() * cm_linear_local_size() + cm_linear_local_id();
}

CM_INLINE uint cm_linear_global_size() {
  return cm_linear_group_count() * cm_linear_local_size();
}

#endif // _CLANG_CM_LINEAR_H_
