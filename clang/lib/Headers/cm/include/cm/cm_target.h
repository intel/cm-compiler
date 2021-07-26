/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_target.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif //(__INCLUDE_LEVEL__ == 1)

#ifndef _CLANG_CM_TARGET_H_
#define _CLANG_CM_TARGET_H_

#if (CM_GENX >= 900 && CM_GENX <= 1150)
  #define CM_HAS_VA 1
#endif //(CM_GENX >= 900 && CM_GENX <= 1150)

#if (CM_GENX >= 900 && CM_GENX <= 1150)
  #define CM_HAS_VA_PLUS 1
#endif //(CM_GENX >= 900 && CM_GENX <= 1150)


// Make Gen target specific warnings into errors
#pragma clang diagnostic error "-Wgen-target"

#endif /* _CLANG_CM_TARGET_H_ */
