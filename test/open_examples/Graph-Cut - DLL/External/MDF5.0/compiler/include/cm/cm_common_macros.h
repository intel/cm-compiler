/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: cm_common_macros.h 25101 2011-03-18 01:11:02Z ayermolo $"
*** -----------------------------------------------------------------------------------------------
***
*** Copyright  (C) 1985-2014 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:             Puyan Lotfi
***
***
***
***
*** Description: Removes a lot of repetative macro redefinitions in various files.
***
*** -----------------------------------------------------------------------------------------------
**/

#pragma once

// used to control generation of compile time assertions
#if defined(__CLANG_CM) && _MSC_VER >= 1600
    // CM_STATIC_ERROR uses the static_assert mechanism
    #include <assert.h>
    // type traits are often used in CM_STATIC_ERROR conditions
    #include <type_traits>
    #define CM_STATIC_ERROR(C,M)   static_assert((C),"CM:e:" M)
    #define CM_STATIC_WARNING(C,M) static_assert((C),"CM:w:" M)
#else
    #define CM_STATIC_ERROR(C,M)
    #define CM_STATIC_WARNING(C,M)
#endif

#ifndef NEW_CM_RT
#define NEW_CM_RT  // Defined for new CM Runtime APIs
#endif

#ifndef AUTO_CM_MODE_SET
#if defined(__CM)
    /// Defined these macros for the Intel CM Compiler.
    #ifdef __GNUC__
    #define _GENX_MAIN_ __attribute__((genx_main))
    #define _GENX_ __attribute__((genx))
    #define _GENX_STACKCALL_ __attribute__((genx_stackcall))
    #define _CM_OUTPUT_ __attribute__((cm_output))
    #define _CM_CALLABLE_ __attribute__((cm_callable))
    #else
    #define _GENX_MAIN_ __declspec(genx_main)
    #define _GENX_ __declspec(genx)
    #define _GENX_STACKCALL_ __declspec(genx_stackcall)
    #define _CM_OUTPUT_ __declspec(cm_output)
    #define _CM_CALLABLE_ __declspec(cm_callable)
    #endif
#else
    /// Defined these macros for MSVC and GCC.
    #define CM_GENX
    #define CM_EMU
    #define _GENX_MAIN_
    #define _GENX_
    #define _GENX_STACKCALL_
    #define _CM_OUTPUT_
    #define _CM_CALLABLE_
#endif /* __CM */
#define AUTO_CM_MODE_SET
#endif /* AUTO_CM_MODE_SET */

#ifndef CM_NOINLINE
    #ifndef CM_EMU
    #ifndef __GNUC__
    #define CM_NOINLINE __declspec(noinline) 
    #else
    #define CM_NOINLINE __attribute__((noinline)) 
    #endif
    #else
    #define CM_NOINLINE
    #endif /* CM_EMU */
#endif

#ifndef CM_INLINE
    #ifndef __GNUC__
    #define CM_INLINE __forceinline
    #else
    #define CM_INLINE inline __attribute__((always_inline))
    #endif
#endif

#ifndef CM_API
#ifndef __GNUC__
    #if defined(NEW_CM_RT) && defined(LIBCM_TEST_EXPORTS)
    #define CM_API __declspec(dllexport)
    #elif defined(NEW_CM_RT)
    #define CM_API
    #else
    #define CM_API
    #endif /* CM_EMU */
#else
    #if defined(NEW_CM_RT) && defined(LIBCM_TEST_EXPORTS)
    #define CM_API __attribute__((visibility("default")))
    #elif defined(NEW_CM_RT)
    #define CM_API
    #else
    #define CM_API
    #endif /* CM_EMU */
#endif
#endif /* CM_API */

#ifndef CMRT_LIBCM_API
#ifndef __GNUC__
    #if defined(NEW_CM_RT) && defined(LIBCM_TEST_EXPORTS)
    #define CMRT_LIBCM_API __declspec(dllexport)
    #elif defined(NEW_CM_RT)
    #define CMRT_LIBCM_API __declspec(dllimport)
    #else
    #define CMRT_LIBCM_API
    #endif /* CM_EMU */
#else
    #if defined(NEW_CM_RT) && defined(LIBCM_TEST_EXPORTS)
    #define CMRT_LIBCM_API __attribute__((visibility("default")))
    #elif defined(NEW_CM_RT)
    #define CMRT_LIBCM_API __attribute__((visibility("default")))
    #else
    #define CMRT_LIBCM_API
    #endif /* CM_EMU */
#endif
#endif /* CMRT_LIBCM_API */

#define CM_CHK_RESULT(cm_call)                                  \
do {                                                            \
    int result = cm_call;                                       \
    if (result != CM_SUCCESS) {                                 \
        fprintf(stderr, "Invalid CM call at %s:%d. Error %d: ", \
        __FILE__, __LINE__, result);                            \
        fprintf(stderr, ".\n");                                 \
        exit(EXIT_FAILURE);                                     \
    }                                                           \
} while(false)

// For non-emu modes - when jit target is specified a CM_GENn define is created
// We rely on this in the front end in some header files for gen specific support
// Use the standard /D geni define to create this define for EMU mode as well
#ifdef CM_EMU
#if defined(gen7)
#define CM_GEN7
#elif defined(gen7_5)
#define CM_GEN7_5
#elif defined(gen8)
#define CM_GEN8
#elif defined(gen8_5)
#define CM_GEN8_5
#elif defined(gen9)
#define CM_GEN9
#endif
#endif
