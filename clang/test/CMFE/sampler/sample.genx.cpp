/*========================== begin_copyright_notice ============================

Copyright (C) 2022-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -march=tgllp -- %s | FileCheck -allow-empty --implicit-check-not error %s
// RUN: not %cmc -S -emit-llvm -march=dg2 -- %s 2>&1 | FileCheck --check-prefix=DG2 %s
// RUN: not %cmc -S -emit-llvm -march=bmg -- %s 2>&1 | FileCheck --check-prefix=BMG %s

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
// DG2: error: Not supported feature for this platform
// BMG: error: Not supported feature for this platform
}
