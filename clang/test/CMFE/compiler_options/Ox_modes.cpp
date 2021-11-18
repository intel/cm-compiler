/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: IGC_CMFE_CC1_EXTRA="-O2;-disable-llvm-passes" %cmc -fcmocl -march=skl -emit-llvm -S -o %t.ll -- %s
// RUN: FileCheck --input-file=%t.ll %s --check-prefixes=MODE_O2,MODE_O2_NO_NOINLINE
// RUN: FileCheck --input-file=%t.ll %s --check-prefixes=MODE_O2,MODE_O2_NO_OPTNONE
// MODE_O2: subgroup_8{{.*}}#[[SUBGROUP_ATTR:[0-9]+]]
// MODE_O2: attributes #[[SUBGROUP_ATTR]] = {
// MODE_O2_NO_NOINLINE-NOT: noinline
// MODE_O2_NO_OPTNONE-NOT: optnone
// MODE_O2-SAME: }

// RUN: IGC_CMFE_CC1_EXTRA="-O0" %cmc -fcmocl -march=skl -emit-llvm -S -o %t.ll -- %s
// RUN: FileCheck --input-file=%t.ll %s --check-prefixes=MODE_O0,MODE_O0_NOINLINE
// RUN: FileCheck --input-file=%t.ll %s --check-prefixes=MODE_O0,MODE_O0_OPTNONE
// MODE_O0: subgroup_8{{.*}}#[[SUBGROUP_ATTR:[0-9]+]]
// MODE_O0: attributes #[[SUBGROUP_ATTR]] = {
// MODE_O0_NOINLINE-SAME: noinline
// MODE_O0_OPTNONE-SAME: optnone

// RUN: %cmc -fcmocl -march=skl -emit-llvm -S -o %t.ll -- %s
// RUN: FileCheck --input-file=%t.ll %s --check-prefixes=MODE_DEFAULT,MODE_DEFAULT_NO_NOINLINE
// RUN: FileCheck --input-file=%t.ll %s --check-prefixes=MODE_DEFAULT,MODE_DEFAULT_NO_OPTNONE
// MODE_DEFAULT: subgroup_8{{.*}}#[[SUBGROUP_ATTR:[0-9]+]]
// MODE_DEFAULT: attributes #[[SUBGROUP_ATTR]] = {
// MODE_DEFAULT_NO_NOINLINE-NOT: noinline
// MODE_DEFAULT_NO_OPTNONE-NOT: optnone
// MODE_DEFAULT-SAME: }

#include <cm/cm.h>

int S1(vector<int, 8> v_out) {
  return v_out[0];
}

extern "C" _GENX_MAIN_ void subgroup_8(
    SurfaceIndex L [[type("buffer_t")]],
    SurfaceIndex R [[type("buffer_t")]],
    SurfaceIndex Res  [[type("buffer_t")]]
) {
    vector<int64_t, 8> l;
    vector<int64_t, 8> res;

    read(L, 0, l);
    res = S1(l);

    write(Res, 0, res);
}

