/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_vme.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_VME_H_
#define _CLANG_CM_VME_H_


#if defined(CM_GEN7_5)
#include "gen7_5_vme.h"
#elif defined(CM_GEN8) || defined(CM_GEN8_5)
#include "gen8_vme.h"
#elif defined(CM_GEN9) || defined(CM_GEN9_5)
#include "gen9_vme.h"
#elif defined(CM_GEN10)
#include "gen10_vme.h"
#elif defined(CM_GEN11)
#include "gen11_vme.h"
#elif defined(CM_GEN12)
#include "gen12_vme.h"
#else
#endif

#endif /* _CLANG_CM_VME_H_ */
