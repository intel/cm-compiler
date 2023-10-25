/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// Tests that option -mCM_max_slm works

// check that test have too much SLM
// RUN: not %cmc -emit-llvm -S -march=pvc -g0 -- %s >& %t.err
// RUN: FileCheck --check-prefix=ERRX -allow-empty %s < %t.err

// check that option works:
// RUN: %cmc -emit-llvm -S -march=pvc -mCM_max_slm=1024 -g0 -- %s >& %t.ok
// RUN: FileCheck --check-prefix=GOODX -allow-empty %s < %t.ok

// ERRX: use slm, but slm size is too large
// GOODX-NOT: use slm, but slm size is too large
// XFAIL: *

#include <cm/cm.h>

constexpr unsigned slm_size = 200 * 1024;

_GENX_MAIN_
void slm_test(SurfaceIndex ibuf [[type("buffer_t")]],
              SurfaceIndex obuf [[type("buffer_t")]]) {
  cm_slm_init(slm_size);
  auto slm = cm_slm_alloc(slm_size);
  constexpr unsigned slm_grain = 8 * 16 / sizeof(unsigned);
  for (unsigned i = 0; i < slm_size; i += slm_grain) {
    vector<unsigned, slm_grain> tmp = cm_load<unsigned, slm_grain>(ibuf, i);
    cm_store_slm(slm + i, tmp);
  }
}
