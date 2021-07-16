/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
