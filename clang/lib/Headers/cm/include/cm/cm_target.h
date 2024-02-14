/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_target.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif //(__INCLUDE_LEVEL__ == 1)

#ifndef _CLANG_CM_TARGET_H_
#define _CLANG_CM_TARGET_H_

#if !defined(__CM_INTEL_TARGET_MAJOR)
#error __CM_INTEL_TARGET_MAJOR is undefined
#endif // !defined(__CM_INTEL_TARGET_MAJOR)

#if !defined(__CM_INTEL_TARGET_MINOR)
#error __CM_INTEL_TARGET_MINOR is undefined
#endif // !defined(__CM_INTEL_TARGET_MINOR)

#if !defined(__CM_INTEL_TARGET_REVISION)
#error __CM_INTEL_TARGET_REVISION is undefined
#endif // !defined(__CM_INTEL_TARGET_REVISION)

#define __CM_INTEL_TARGET(major, minor, revision)                              \
  (((major)&0x3ff) << 22 | ((minor)&0xff) << 14 | ((revision)&0x3f))

#define __CM_INTEL_TARGET_ID                                                   \
  __CM_INTEL_TARGET(__CM_INTEL_TARGET_MAJOR, __CM_INTEL_TARGET_MINOR,          \
                    __CM_INTEL_TARGET_REVISION)

// Legacy CM_GENX macros definition
#define __CM_INTEL_TARGET_CORE                                                 \
  __CM_INTEL_TARGET(__CM_INTEL_TARGET_MAJOR, __CM_INTEL_TARGET_MINOR, 0)

// BDW
#if __CM_INTEL_TARGET_MAJOR == 8
#define CM_GENX 800
#define CM_GEN8
#endif

// SKL
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(9, 0, 0)
#define CM_GENX 900
#define CM_GEN9
#endif

// KBL, CFL, WHL, AML, CML
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(9, 1, 0) ||                    \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(9, 2, 0) ||                    \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(9, 5, 0) ||                    \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(9, 6, 0) ||                    \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(9, 7, 0)
#define CM_GENX 950
#define CM_GEN9_5
#endif

// APL, BXT
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(9, 3, 0)
#define CM_GENX 920
#define CM_GEN9
#endif

// GLK
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(9, 4, 0)
#define CM_GENX 970
#define CM_GEN9_5
#endif

// ICLLP, EHL, JSL
#if __CM_INTEL_TARGET_MAJOR == 11
#define CM_GENX 1150
#define CM_GEN11
#endif

// TGLLP
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 0, 0)
#define CM_GENX 1200
#define CM_GEN12
#endif

// RKL
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 1, 0)
#define CM_GENX 1201
#define CM_GEN12
#endif

// ADLS, RPLS
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 2, 0)
#define CM_GENX 1230
#define CM_GEN12
#endif

// ADLP
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 3, 0)
#define CM_GENX 1220
#define CM_GEN12
#endif

// ADLN
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 4, 0)
#define CM_GENX 1240
#define CM_GEN12
#endif

// DG1
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 10, 0)
#define CM_GENX 1210
#define CM_GEN12
#endif

// XE-HP-SDV
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 50, 0)
#define CM_GENX 1270
#define CM_XEHP
#endif

#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 55, 0) ||                  \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 56, 0) ||                  \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 57, 0)
#define CM_GENX 1271
#define CM_XEHPG
#endif

// PVC
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 60, 0) || \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 61, 0)
#define CM_GENX 1280
#define CM_XEHPC
#endif

// MTL, ARL-S
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 70, 0) ||                  \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 71, 0)
#define CM_GENX 1275
#define CM_XELPG
#endif

// ARL-H
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 74, 0)
#define CM_GENX 1276
#define CM_XELPGPLUS
#endif

// LNL
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(20, 4, 0)
#define CM_GENX 1295
#define CM_XE2_LPG
#endif

#define CM_GENX_REVID __CM_INTEL_TARGET_REVISION

#if (CM_GENX >= 900 && CM_GENX <= 1150)
  #define CM_HAS_VA 1
#endif //(CM_GENX >= 900 && CM_GENX <= 1150)

#if (CM_GENX >= 900 && CM_GENX <= 1150)
  #define CM_HAS_VA_PLUS 1
#endif //(CM_GENX >= 900 && CM_GENX <= 1150)

#if 1 // !(CM_GENX == 1280 && CM_GENX_REVID <= 2) //PVC
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
