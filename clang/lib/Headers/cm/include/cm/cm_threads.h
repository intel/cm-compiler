/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_threads.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_THREADS_H_
#define _CLANG_CM_THREADS_H_

#include "spirv/builtins.h"

CM_NODEBUG CM_INLINE uint cm_local_id(uint dim) {
  return __spirv_BuiltInLocalInvocationId(dim);
}

CM_NODEBUG CM_INLINE uint cm_local_size(uint dim) {
  return __spirv_BuiltInWorkgroupSize(dim);
}

CM_NODEBUG CM_INLINE uint cm_group_id(uint dim) {
  return __spirv_BuiltInWorkgroupId(dim);
}

CM_NODEBUG CM_INLINE uint cm_group_count(uint dim) {
  return __spirv_BuiltInNumWorkgroups(dim);
}

CM_NODEBUG CM_INLINE uint cm_linear_local_id() {
  return __spirv_BuiltInLocalInvocationIndex();
}

CM_NODEBUG CM_INLINE uint cm_linear_local_size() {
  return cm_local_size(0) * cm_local_size(1) * cm_local_size(2);
}

CM_NODEBUG CM_INLINE uint cm_linear_group_id() {
  return cm_group_count(0) * cm_group_count(1) * cm_group_id(2) +
         cm_group_count(0) * cm_group_id(1) + cm_group_id(0);
}

CM_NODEBUG CM_INLINE uint cm_linear_group_count() {
  return cm_group_count(0) * cm_group_count(1) * cm_group_count(2);
}

CM_NODEBUG CM_INLINE uint cm_linear_global_id() {
  return __spirv_BuiltInGlobalLinearId();
}

CM_NODEBUG CM_INLINE uint cm_linear_global_size() {
  return cm_linear_group_count() * cm_linear_local_size();
}

CM_NODEBUG CM_INLINE uint cm_get_hwid() {
  return __spirv_BuiltInGlobalHWThreadIDINTEL();
}

CM_NODEBUG CM_INLINE uint cm_get_tileid() {
  return __spirv_BuiltInSubDeviceIDINTEL();
}

#endif // _CLANG_CM_THREADS_H_
