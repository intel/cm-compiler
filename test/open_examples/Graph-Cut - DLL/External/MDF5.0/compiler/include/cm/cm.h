/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: cm.h 25815 2011-07-20 07:42:24Z sghosh1 $"
*** -----------------------------------------------------------------------------------------------
***
*** Copyright  (C) 1985-2007 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:           Alexey V. Aliev  
***                    Oleg Mirochnik
***                      
***                      
***
*** Description: General header for inclusion in the Cm programms
***
*** -----------------------------------------------------------------------------------------------
**/

#ifndef CM_H
#define CM_H

// Add new Macro for each new release
#ifndef CM_1_0
#define CM_1_0          100
#endif 
#ifndef CM_2_0
#define CM_2_0          200
#endif
#ifndef CM_2_1
#define CM_2_1          201
#endif
#ifndef CM_2_2
#define CM_2_2          202
#endif
#ifndef CM_2_3
#define CM_2_3          203
#endif
#ifndef CM_2_4
#define CM_2_4          204
#endif
#ifndef CM_3_0
#define CM_3_0          300
#endif
#ifndef CM_4_0
#define CM_4_0          400
#endif
#ifndef CM_5_0
#define CM_5_0          500
#endif

//Define MDF version for new feature
#ifndef __INTEL_MDF 
#define __INTEL_MDF     CM_5_0
#endif

#define CM_V2
//#define CM_V1
//#define NEW_CM_RT

#include "cm_common_macros.h"

#if defined(CM_GEN5) || defined(CM_GEN6) || defined(CM_GEN7) || defined (CM_GEN7_5) || defined(CM_GEN8) || defined(CM_GEN8_5) || defined(CM_GEN9) || defined(CM_JIT)
#define CM_GENX
#endif

#ifdef CM_EMU
 #define CM_execute_kernels CM_execute_kernels_emu
 #define CM_set_thread_count CM_set_thread_count_emu
 
 #define CM_register_buffer CM_register_buffer_emu
 #define CM_unregister_buffer CM_unregister_buffer_emu
 #define CM_modify_buffer CM_modify_buffer_emu

 #define CM_register_sampler CM_register_sampler_emu 
 #define CM_register_sampler8x8 CM_register_sampler8x8_emu
 #define CM_sampler_sample16 CM_sampler_sample16_emu
 #define CM_sampler_sample32 CM_sampler_sample32_emu
 #define CM_sampler_sample8x8 CM_sampler_sample8x8_emu
 
 #define CM_register_sampler_surface_state CM_register_sampler_surface_state_emu
 #define CM_register_sampler8x8_surface_state CM_register_sampler8x8_surface_state_emu

#if defined(CM_GEN6) || defined(CM_VME_GEN6)
 #define LoadSurfaceIdx LoadSurfaceIdx_emu
 #define LoadSearchPaths LoadSearchPaths_emu
#endif

#if defined(CM_GEN7) || defined(CM_VME_GEN7)
 #define LoadSurfaceIdx LoadSurfaceIdx_gen7_emu
 #define LoadSearchPaths LoadSearchPaths_gen7_emu
#endif

#endif /* CM_EMU */

//__declspec(dllimport)

#include "cm_def.h"
#include "cm_vm.h"
#include "cm_intrin.h"
#include "cm_internal.h"

#ifdef CM_GENX
 #include "genx_kernelrun.h"
 #include "genx_dataport.h"
 #include "genx_threading.h"
 #include "genx_sampler.h"
 #include "genx_sampler8x8.h"
 #include "genx_vme.h"
 //#include "dyn_helper.h"
 #include "genx_simdcontrolflow.h"
#endif /* CM_GENX */

#include "cm_slm_user.h"
#include "cm_printf_device.h"
#include "cm_color.h"
#endif /* CM_H */
