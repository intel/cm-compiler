/**             
*** ---------------------------------------------------------------------------
*** Copyright  (C) 1985-2010 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
*** Authors:             Biju George
***                      
*** Description:
***     CM GENX mode statement definitions (macros) for SIMD control flow.
*** ---------------------------------------------------------------------------
**/

#ifndef GENX_SIMDCONTROLFLOW_HW_H
#define GENX_SIMDCONTROLFLOW_HW_H

#include "cm_def.h"

//-----------------------------------------------------------------------------
// SIMD control flow static declarations
//-----------------------------------------------------------------------------

namespace __CMInternal__ {
    _GENX_ static volatile ushort __cm_internal_simd_marker;
};

//-----------------------------------------------------------------------------
// SIMD "if" definitions
//-----------------------------------------------------------------------------

#define SIMD_IF_BEGIN(...)                                              \
    if ((__CMInternal__::__cm_internal_simd(__VA_ARGS__))) {            \
        __CMInternal__::__cm_internal_simd_marker =                     \
            __CMInternal__::__cm_internal_simd_if_begin();

#define SIMD_ELSEIF(...)                                                \
        __CMInternal__::__cm_internal_simd_marker =                     \
          __CMInternal__::__cm_internal_simd_then_end();             \
    }                                                                   \
    else if ((__CMInternal__::__cm_internal_simd(__VA_ARGS__))) {       \
        __CMInternal__::__cm_internal_simd_marker =                     \
            __CMInternal__::__cm_internal_simd_elseif_begin();

#define SIMD_ELSE                                                       \
        __CMInternal__::__cm_internal_simd_marker =                     \
            __CMInternal__::__cm_internal_simd_then_end();              \
    }                                                                   \
    else {                                                              \
        __CMInternal__::__cm_internal_simd_marker =                     \
            __CMInternal__::__cm_internal_simd_else_begin();

#define SIMD_IF_END                                                     \
        __CMInternal__::__cm_internal_simd_marker =                     \
            __CMInternal__::__cm_internal_simd_if_end();                \
    }                                                                   \
    __CMInternal__::__cm_internal_simd_marker =                         \
        __CMInternal__::__cm_internal_simd_if_join();

//-----------------------------------------------------------------------------
// SIMD "do-while" definitions
//-----------------------------------------------------------------------------

#define SIMD_DO_WHILE_BEGIN                                             \
    do {                                                                \
        __CMInternal__::__cm_internal_simd_marker =                     \
            __CMInternal__::__cm_internal_simd_do_while_begin();

#define SIMD_DO_WHILE_END(...)                                          \
        __CMInternal__::__cm_internal_simd_marker =                     \
            __CMInternal__::__cm_internal_simd_do_while_end();          \
    }                                                                   \
    while ((__CMInternal__::__cm_internal_simd(__VA_ARGS__)))

//-----------------------------------------------------------------------------
// SIMD jump definitions
//-----------------------------------------------------------------------------

#define SIMD_BREAK                                                      \
    __CMInternal__::__cm_internal_simd_marker =                         \
        __CMInternal__::__cm_internal_simd_break();

#define SIMD_CONTINUE                                                   \
    __CMInternal__::__cm_internal_simd_marker =                         \
        __CMInternal__::__cm_internal_simd_continue();

#endif /* GENX_SIMDCONTROLFLOW_HW_H */
