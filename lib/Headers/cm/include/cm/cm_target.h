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

#if !(CM_GENX == 1280 && CM_GENX_REVID <= 2) //PVC
#define CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT
#endif // !(CM_GENX == 1280 && CM_GENX_REVID <= 2)

// On PVC non-transpose LSC messages have SIMD32 layout
// So 16-channels non-transposed lsc messages with VectorSze != 1
// aren't supported on PVC
#if !defined(CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT)
#define CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, VS) \
  CM_STATIC_ERROR(                                                             \
      N == details::lsc_default_simt() || VS == VectorSize::N1,                \
      "unexpected number of channels for non-transpose lsc message");
#else
#define CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, VS)
#endif //!defined(CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT)

// Make Gen target specific warnings into errors
#pragma clang diagnostic error "-Wgen-target"

#endif /* _CLANG_CM_TARGET_H_ */
