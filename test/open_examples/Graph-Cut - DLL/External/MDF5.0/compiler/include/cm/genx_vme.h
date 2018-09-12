/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: genx_vme.h 26085 2011-10-05 23:53:30Z kchen24 $"
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
*** Authors:             Kaiyu Chen
***                      
***                      
***                      
***
*** Description: Cm VME APIs
***
*** -----------------------------------------------------------------------------------------------
**/

#ifndef CM_VME_H
#define CM_VME_H

typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int   U32;

typedef unsigned char       uint1;  // unsigned byte
typedef unsigned short      uint2;  // unsigned word
typedef unsigned int        uint4;  // unsigned dword
typedef unsigned long long  uint8;  // unsigned qword
typedef char                int1;   // byte
typedef short               int2;   // word
typedef int                 int4;   // dword
typedef long long           int8;   // qword

#if defined(CM_GEN6) || defined(CM_VME_GEN6)
#include "gen6_vme.h" 
#endif

#if defined(CM_GEN7) || defined(CM_VME_GEN7)
#include "gen7_vme.h" 
#endif

#if defined(CM_GEN7_5) || defined(CM_VME_GEN7_5)
#include "gen7_5_vme.h" 
#endif

#if defined(CM_GEN8) || defined(CM_VME_GEN8) || defined(CM_GEN8_5) || defined(CM_VME_GEN8_5) 
#include "gen8_vme.h" 
#endif

#if defined(CM_GEN9) || defined(CM_VME_GEN9) || defined(CM_GEN9_5) || defined(CM_VME_GEN9_5)
#include "gen9_vme.h" 
#endif

#if defined(CM_GEN10) || defined(CM_VME_GEN10)
#include "gen10_vme.h"
#endif

#endif /* CM_VME_H */
