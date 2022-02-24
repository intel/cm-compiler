/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// expected-no-diagnostics

#include <cm/cm.h>
#include <cm/cmtl.h>

#define SLM_BUFFER_SIZE 64

_GENX_MAIN_ void test(SurfaceIndex slmDebugSurface)
{
  cm_slm_init(SLM_BUFFER_SIZE);

  // SurfaceIndex slmDebugSurface passed in as parameter
  // slmX handle is allocated here assuming cm_slm_init etc already done
  // elsewhere
  // SLM_BUFFER_SIZE defined elsewhere

  uint slmX = cm_slm_alloc(SLM_BUFFER_SIZE);

  // Some work to put some relevant values into SLM
  // ...

  // Dump SLM to memory
  // In this case we're dumping the whole of the allocated buffer
  cmtl::DumpSLM(slmX, slmDebugSurface, SLM_BUFFER_SIZE);
}

// RUN: %cmc -emit-llvm -march=SKL -Xclang -verify -- %s
