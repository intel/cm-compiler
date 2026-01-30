/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_target.h should not be included explicitly");
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

// BMG
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(20, 1, 0) ||                   \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(20, 2, 0)
#define CM_GENX 1290
#define CM_XE2_HPG
#endif

// LNL
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(20, 4, 0)
#define CM_GENX 1295
#define CM_XE2_LPG
#endif

// PTL
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(30, 0, 0) ||                   \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(30, 1, 0)
#define CM_GENX 1300
#define CM_XE3_LPG
#endif

// WCL, NVL-S, NVL-U
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(30, 3, 0) ||                   \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(30, 4, 0) ||                   \
    __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(30, 5, 0)
#define CM_GENX 1300
#define CM_XE3_LPG
#endif

// CRI
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(35, 11, 0)
#define CM_GENX 1380
#define CM_XE3P_CRI
#endif

// NVL-P
#if __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(35, 10, 0)
#define CM_GENX 1360
#define CM_XE3P_LPG
#endif

#define CM_GENX_REVID __CM_INTEL_TARGET_REVISION

// DG2 or newer
#if __CM_INTEL_TARGET_MAJOR >= 20 ||                                           \
    (__CM_INTEL_TARGET_MAJOR == 12 && __CM_INTEL_TARGET_MINOR >= 55)
#define __CM_INTEL_TARGET_DG2_OR_ABOVE
#endif

// PVC or newer
#if __CM_INTEL_TARGET_MAJOR >= 20 ||                                           \
    (__CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 60, 0) ||                 \
     __CM_INTEL_TARGET_CORE == __CM_INTEL_TARGET(12, 61, 0))
#define __CM_INTEL_TARGET_PVC_OR_ABOVE
#define __CM_DEFAULT_SIMT 32
#else
#define __CM_DEFAULT_SIMT 16
#endif

// Make Gen target specific warnings into errors
#pragma clang diagnostic error "-Wgen-target"

#endif /* _CLANG_CM_TARGET_H_ */
