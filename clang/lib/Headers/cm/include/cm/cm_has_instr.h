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

#if (CM_GENX >= 1200)
  #define CM_HAS_DP4A 1
  #define CM_HAS_DP4A_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_DP4A_CONTROL CM_HAS_CONTROL(false)
#endif

#if (CM_GENX >= 1150) //>= ICLLP
  #define CM_HAS_BIT_ROTATE 1
  #define CM_HAS_BIT_ROTATE_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_BIT_ROTATE_CONTROL CM_HAS_CONTROL(false)
#endif


//BFN
#if (CM_GENX >= 1270) //>= XEHP_SDV
  #define CM_HAS_BFN 1
  #define CM_HAS_BFN_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_BFN_CONTROL CM_HAS_CONTROL(false)
#endif


//DPAS
#ifdef CM_HAS_DPAS
  #define CM_HAS_DPAS_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_DPAS_CONTROL CM_HAS_CONTROL(false)
#endif

//DPASW
#ifdef CM_HAS_DPASW
  #define CM_HAS_DPASW_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_DPASW_CONTROL CM_HAS_CONTROL(false)
#endif

//ACC_BF16 and ACC_HALF
#if (CM_GENX >= 1280) //>= PVC
  #define CM_HAS_DPAS_ACC_HALF 1
  #define CM_HAS_DPAS_ACC_BF16 1
  #define CM_HAS_DPAS_ACC_HALF_CONTROL CM_HAS_CONTROL(true)
  #define CM_HAS_DPAS_ACC_BF16_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_DPAS_ACC_HALF_CONTROL CM_HAS_CONTROL(false)
  #define CM_HAS_DPAS_ACC_BF16_CONTROL CM_HAS_CONTROL(false)
#endif

//BF16
#if defined(CM_HAS_BF16)
  #define CM_HAS_BF16_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_BF16_CONTROL CM_HAS_CONTROL(false)
#endif

#define CM_HAS_DPAS_INT_MIX 1
#define CM_HAS_DPAS_INT_MIX_CONTROL CM_HAS_CONTROL(true)

#define CM_HAS_DPAS_INT2 1
#define CM_HAS_DPAS_INT2_CONTROL CM_HAS_CONTROL(true)

#define CM_HAS_DPAS_INT4 1
#define CM_HAS_DPAS_INT4_CONTROL CM_HAS_CONTROL(true)

#define CM_HAS_DPAS_INT8 1
#define CM_HAS_DPAS_INT8_CONTROL CM_HAS_CONTROL(true)


//DPAS_ODD
#define CM_HAS_DPAS_ODD 1

//Gateway event
#if (CM_GENX >= 1150 && CM_GENX <= 1280) //>= ICLLP && <= PVC
  #define CM_HAS_GATEWAY_EVENT
  #define CM_HAS_GATEWAY_EVENT_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_GATEWAY_EVENT_CONTROL CM_HAS_CONTROL(false)
#endif

//IEEE
#if (CM_GENX == 800  || /*BWD*/         \
     CM_GENX == 900  || /*SKL*/         \
     CM_GENX == 950  || /*KBL*/         \
     CM_GENX == 1150 || /*ICLLP*/       \
     CM_GENX == 1270 || /*XeHP_SDV*/    \
     CM_GENX == 1280    /*PVC*/         )
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


// TF32
//  #if ((CM_GENX == 1280 && CM_GENX_REVID >= 5) || CM_GENX > 1280) //>= PVCXT
#if (CM_GENX >= 1280) //>= PVC
#define CM_HAS_TF32 1
#define CM_HAS_TF32_CONTROL CM_HAS_CONTROL(true)
#else
#define CM_HAS_TF32_CONTROL CM_HAS_CONTROL(false)
#endif

//BitRotate64
#if (CM_GENX >= 1280) //>= PVC
  #define CM_HAS_BIT_ROTATE_64BIT 1
  #define CM_HAS_BIT_ROTATE_64BIT_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_BIT_ROTATE_64BIT_CONTROL CM_HAS_CONTROL(false)
#endif

#if (CM_GENX >= 1280) // >= PVC
  #define CM_HAS_STOCHASTIC_ROUNDING 1
  #define CM_HAS_STOCHASTIC_ROUNDING_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_STOCHASTIC_ROUNDING_CONTROL CM_HAS_CONTROL(false)
#endif
#if (CM_GENX >= 1280) // >= PVC
  #define CM_HAS_LSC_SYS_FENCE 1
#endif
#if (CM_GENX <= 1280)
  #define CM_HAS_LSC_LOAD_L1RI_L2CA_HINT 1
  #define CM_HAS_LSC_LOAD_L1RI_L3CA_HINT 1
#endif

#ifdef CM_HAS_SLM_CAS_INT64
#define CM_HAS_SLM_CAS_INT64_CONTROL CM_HAS_CONTROL(true)
#else
#define CM_HAS_SLM_CAS_INT64_CONTROL CM_HAS_CONTROL(false)
#endif // CM_HAS_SLM_CAS_INT64

#if (CM_GENX >= 900) // >=SKL
#define CM_HAS_TYPED_ATOMIC 1
#define CM_HAS_TYPED_ATOMIC_CONTROL CM_HAS_CONTROL(true)
#else
#define CM_HAS_TYPED_ATOMIC_CONTROL CM_HAS_CONTROL(false)
#endif


#else  // CM_HAS_CONTROL
  CM_STATIC_ERROR(0, "Redeclaration of CM_HAS_CONTROL! It's used for control version of features!");
#endif // CM_HAS_CONTROL

#endif /* _CLANG_CM_HAS_INSTR_H_ */
