/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This file provide CM_HAS_<Feature> flag
// It's used for checking platform to support <Feature> 
// 
// USAGE:
//   #if defined(CM_HAS_feature)
//     w = cm_feature<...>(...);
//   #else
//     w = ...; //without feature
//   #endif

#ifndef _CLANG_CM_HAS_INSTR_H_
#define _CLANG_CM_HAS_INSTR_H_

#ifndef CM_HAS_CONTROL
 
namespace CheckVersion {

  //used for creating static_assert only on 2nd stage of
  //substitution - when user tries to get blocked instruction
  template <bool checking>
  struct VersionWrapper final {
    static constexpr bool check = checking;
  };

  template <typename T>
  void Check() {
    CM_STATIC_ERROR(T::check, "Not supported feature for this platform");
  }
}

#define CM_HAS_CONTROL(checking_statement) CheckVersion::Check<CheckVersion::VersionWrapper<checking_statement>>()

//-----------------------------------------------
//-----------------------------------------------

#if (CM_GENX >= 1200) 
  #define CM_HAS_DP4A 1
  #define CM_HAS_DP4A_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_DP4A_CONTROL CM_HAS_CONTROL(false)
#endif


//BFN
#if (CM_GENX >= 1270) //>= XEHP_SDV
  #define CM_HAS_BFN 1
  #define CM_HAS_BFN_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_BFN_CONTROL CM_HAS_CONTROL(false)
#endif

//BF16
#if (CM_GENX >= 1200) //>= TGLLP
  #define CM_HAS_BF16 1
  #define CM_HAS_BF16_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_BF16_CONTROL CM_HAS_CONTROL(false)
#endif

//DPAS
#if (CM_GENX >= 1270) //>= XEHP_SDV
  #define CM_HAS_DPAS 1
  #define CM_HAS_DPAS_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_DPAS_CONTROL CM_HAS_CONTROL(false)
#endif

//DPAS_ODD
#if (CM_GENX >= 1270) //>= XEHP_SDV
  #define CM_HAS_DPAS_ODD 1
  #define CM_HAS_DPAS_ODD_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_DPAS_ODD_CONTROL CM_HAS_CONTROL(false)
#endif

//DPASW
#if (CM_GENX >= 1270) //>= XEHP_SDV
  #define CM_HAS_DPASW 1
  #define CM_HAS_DPASW_CONTROL CM_HAS_CONTROL(true)
#else
  #define CM_HAS_DPASW_CONTROL CM_HAS_CONTROL(false)
#endif


#else  // CM_HAS_CONTROL
  CM_STATIC_ERROR(0, "Redeclaration of CM_HAS_CONTROL! It's used for control version of features!");
#endif // CM_HAS_CONTROL

#endif /* _CLANG_CM_HAS_INSTR_H_ */
