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
***     CM internal functions (not to be called directly in CM programs).
*** ---------------------------------------------------------------------------
**/

#ifndef CM_INTERNAL_H
#define CM_INTERNAL_H

#ifdef CM_EMU
#include "cm_internal_emu.h"
#elif defined(CM_GENX)
#include "cm_internal_hw.h"
#endif

#ifdef CM_EMU

#define CM_STATIC_BUFFER_0  (*get_global_surface_index(0))
#define CM_STATIC_BUFFER_1  (*get_global_surface_index(1))
#define CM_STATIC_BUFFER_2  (*get_global_surface_index(2))
#define CM_STATIC_BUFFER_3  (*get_global_surface_index(3))

#else

#ifndef __GNUC__
extern __declspec(genx) SurfaceIndex _cm_global_surface_0; 
extern __declspec(genx) SurfaceIndex _cm_global_surface_1;
extern __declspec(genx) SurfaceIndex _cm_global_surface_2;
extern __declspec(genx) SurfaceIndex _cm_global_surface_3;
#else
extern __attribute__((genx)) SurfaceIndex _cm_global_surface_0; 
extern __attribute__((genx)) SurfaceIndex _cm_global_surface_1;
extern __attribute__((genx)) SurfaceIndex _cm_global_surface_2;
extern __attribute__((genx)) SurfaceIndex _cm_global_surface_3;
#endif

#define CM_STATIC_BUFFER_0 _cm_global_surface_0
#define CM_STATIC_BUFFER_1 _cm_global_surface_1
#define CM_STATIC_BUFFER_2 _cm_global_surface_2
#define CM_STATIC_BUFFER_3 _cm_global_surface_3

#endif


#endif /* CM_INTERNAL_H */

