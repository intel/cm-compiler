/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_common.h"
#include "cm_target.h"

/// CM_HAS_<Feature> maros
/// ----------------------
/// Check platform to support <Feature>.
/// Return true if Feature is supported.
//===----------------------------------------------------------------------===//

#ifndef _CLANG_CM_HAS_INSTR_H_
#define _CLANG_CM_HAS_INSTR_H_

#ifndef CM_HAS_CONTROL

namespace CheckVersion {

  // Use structure to create static_assert only on 2nd stage of
  // substitution - when user tries to get blocked instruction.
  template <bool checking>
  struct VersionWrapper final {
    static constexpr bool check = checking;
  };

  template <typename T>
  CM_INLINE CM_NODEBUG void Check() {
    CM_STATIC_ERROR(T::check, "Not supported feature for this platform");
  }
}

#define CM_HAS_CONTROL(checking_statement) CheckVersion::Check<CheckVersion::VersionWrapper<checking_statement>>()

//-----------------------------------------------
//-----------------------------------------------
/// CM_HAS_<Feature>_CONTROL macors
/// -------------------------------
/// Create static_assert if feature isn't supported for this platform.
/// Otherwise, do nothing.
///
/// CM_GENX value of platforms sets according to Frontend/InitPreprocessor.cpp.
//===----------------------------------------------------------------------===//

#define CM_HAS_LONG_LONG 1

#ifdef CM_HAS_BF16
#define CM_HAS_BF16_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_BF16
#define CM_HAS_BF16_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_BF16

#ifdef CM_HAS_TF32
#define CM_HAS_TF32_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_TF32
#define CM_HAS_TF32_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_TF32

#ifdef CM_HAS_BF8
#define CM_HAS_BF8_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_BF8
#define CM_HAS_BF8_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_BF8

#ifdef CM_HAS_HF8
#define CM_HAS_HF8_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_HF8
#define CM_HAS_HF8_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_HF8

#ifdef CM_HAS_BFN
#define CM_HAS_BFN_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_BFN
#define CM_HAS_BFN_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_BFN

#ifdef CM_HAS_DP4A
#define CM_HAS_DP4A_CONTROL CM_HAS_CONTROL(true)
#else // CM_HAS_DP4A
#define CM_HAS_DP4A_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_DP4A

#if (CM_GENX >= 1150) //>= ICLLP
  #define CM_HAS_BIT_ROTATE 1
  #define CM_HAS_BIT_ROTATE_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_BIT_ROTATE_CONTROL CM_HAS_CONTROL(false)
#endif

// Help to detect if a GPU supports legacy messages to pass correct compiler
// flags during online compilation.
#if __CM_INTEL_TARGET_MAJOR >= 20
#define CM_REQUIRES_LEGACY_TRANSLATION 1
#endif

//Gateway event
#if (CM_GENX >= 1150 && CM_GENX <= 1280) //>= ICLLP && <= PVC
  #define CM_HAS_GATEWAY_EVENT
  #define CM_HAS_GATEWAY_EVENT_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_GATEWAY_EVENT_CONTROL CM_HAS_CONTROL(false)
#endif

//IEEE
#if (CM_GENX == 800 ||  /*BWD*/                                                \
     CM_GENX == 900 ||  /*SKL*/                                                \
     CM_GENX == 950 ||  /*KBL*/                                                \
     CM_GENX == 1150 || /*ICLLP*/                                              \
     CM_GENX == 1270 || /*XeHP_SDV*/                                           \
     CM_GENX >= 1280    /*PVC*/                                                \
)
#define CM_HAS_IEEE_DIV_SQRT 1
#define CM_HAS_IEEE_DIV_SQRT_CONTROL CM_HAS_CONTROL(true)
#else  //IEEE
  #define CM_HAS_IEEE_DIV_SQRT_CONTROL CM_HAS_CONTROL(false)
#endif //IEEE

//LSC
#if (CM_GENX >= 1271) //>= DG2
  #define CM_HAS_LSC 1
  #define CM_HAS_LSC_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_LSC_CONTROL CM_HAS_CONTROL(false)
#endif

//LSC_TYPED_2D
#if __CM_INTEL_TARGET_MAJOR >= 20
#define CM_HAS_LSC_TYPED 1
#define CM_HAS_LSC_TYPED_2D 1
#define CM_HAS_LSC_TYPED_CONTROL CM_HAS_CONTROL(true)
#define CM_HAS_LSC_TYPED_2D_CONTROL CM_HAS_CONTROL(true)
#else
#define CM_HAS_LSC_TYPED_CONTROL CM_HAS_CONTROL(false)
#define CM_HAS_LSC_TYPED_2D_CONTROL CM_HAS_CONTROL(false)
#endif

//LSC_UNTYPED_2D
#if (CM_GENX >= 1280) //>= PVC
  #define CM_HAS_LSC_UNTYPED_2D 1
  #define CM_HAS_LSC_UNTYPED_2D_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_LSC_UNTYPED_2D_CONTROL CM_HAS_CONTROL(false)
#endif

// Sample unorm
#if (CM_GENX < 1270) // < XEHP_SDV
  #define CM_HAS_SAMPLE_UNORM 1
  #define CM_HAS_SAMPLE_UNORM_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_SAMPLE_UNORM_CONTROL  CM_HAS_CONTROL(false)
#endif

//BitRotate64
#if (CM_GENX >= 1280) //>= PVC
  #define CM_HAS_BIT_ROTATE_64BIT 1
  #define CM_HAS_BIT_ROTATE_64BIT_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_BIT_ROTATE_64BIT_CONTROL CM_HAS_CONTROL(false)
#endif

#if (CM_GENX >= 1280) // >= PVC
  #define CM_HAS_LSC_SYS_FENCE 1
#endif

#if __CM_INTEL_TARGET_MAJOR >= 20
  #define CM_HAS_LSC_L1L2CC_HINT 1
  #define CM_HAS_LSC_L1L3CC_HINT 1
#endif

#if __CM_INTEL_TARGET_MAJOR >= 20
  #define CM_HAS_LSC_LOAD_L1RI_L2RI_HINT 1
  #define CM_HAS_LSC_LOAD_L1RI_L3RI_HINT 1
#else
  #define CM_HAS_LSC_LOAD_L1RI_L2CA_HINT 1
  #define CM_HAS_LSC_LOAD_L1RI_L3CA_HINT 1
#endif


#ifdef CM_HAS_SLM_CAS_INT64
#define CM_HAS_SLM_CAS_INT64_CONTROL CM_HAS_CONTROL(true)
#else
#define CM_HAS_SLM_CAS_INT64_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_SLM_CAS_INT64

#if __CM_INTEL_TARGET_MAJOR >= 9 && __CM_INTEL_TARGET_MAJOR < 20
#define CM_HAS_TYPED_ATOMIC 1
#define CM_HAS_TYPED_ATOMIC_CONTROL CM_HAS_CONTROL(true)
#else
#define CM_HAS_TYPED_ATOMIC_CONTROL CM_HAS_CONTROL(false)
#endif



#else  // CM_HAS_CONTROL
  CM_STATIC_ERROR(0, "Redeclaration of CM_HAS_CONTROL! It's used for control version of features!");
#endif // CM_HAS_CONTROL

#endif /* _CLANG_CM_HAS_INSTR_H_ */
