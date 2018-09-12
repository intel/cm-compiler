/**             
*** ---------------------------------------------------------------------------
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
*** Authors:             Biju George
***                      
*** Description:
***     CM declarations and statement definitions (macros) for SIMD control
***     flow.
*** ---------------------------------------------------------------------------
**/

#ifndef GENX_SIMDCONTROLFLOW_H
#define GENX_SIMDCONTROLFLOW_H

//-----------------------------------------------------------------------------
// SIMD control flow statement definitions
//-----------------------------------------------------------------------------

#ifdef CM_EMU
#include "genx_simdcontrolflow_emu.h"
#elif defined(CM_GEN6) || defined(CM_GEN7) || defined(CM_GEN7_5)  || defined(CM_GEN8) || defined(CM_GEN8_5)|| defined(CM_GEN9) || defined(CM_JIT)
#include "genx_simdcontrolflow_hw.h"
#endif /* CM_EMU */

#endif /* GENX_SIMDCONTROLFLOW_H */
