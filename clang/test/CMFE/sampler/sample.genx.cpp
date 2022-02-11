/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -march=SKL -- %s | FileCheck -allow-empty --implicit-check-not error %s
// RUN: not %cmc -S -emit-llvm -march=XEHP_SDV -- %s 2>&1 | FileCheck --check-prefix=XEHP_SDV %s

#include <cm/cm.h>

extern "C" _GENX_MAIN_ void
test_sample32(SamplerIndex SamplerConfig,
              SurfaceIndex Buf,
              SurfaceIndex OBuf,
              float U,
              float V,
              float DU,
              float DV)
{
  matrix<ushort, 4, 32> M;
  sample32(M, CM_A_ENABLE, Buf, SamplerConfig, U, V, DU, DV);
// XEHP_SDV: error: Not supported feature for this platform
}
