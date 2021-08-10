/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

template<typename dstTy, typename srcTy>
_GENX_MAIN_ void kernel(dstTy, srcTy)
{
  vector<srcTy, 16> srcA;
  vector<srcTy, 16> srcB;
  vector<dstTy, 16> dst = cm_shr<dstTy>(srcA, srcB);
}

template void kernel<char, int>(char, int);
// CHECK-LABEL: define {{.*}}kernel{{.*}}(i8{{.*}}, i32{{.*}})
// CHECK: [[SAT_RES_CHAR:%[^ ]+]] = call <16 x i8> @llvm.genx.sstrunc.sat.v16i8.v16i32(<16 x i32> %[[RES_CHAR:[^ ]+]])

template void kernel<short, int>(short, int);
// CHECK-LABEL: define {{.*}}kernel{{.*}}(i16{{.*}}, i32{{.*}})
// CHECK: [[SAT_RES_SHORT:%[^ ]+]] = call <16 x i16> @llvm.genx.sstrunc.sat.v16i16.v16i32(<16 x i32> [[RES_SHORT:[^ ]+]])

template void kernel<int, int>(int, int);
// CHECK-LABEL: define {{.*}}kernel{{.*}}(i32{{.*}}, i32{{.*}})
// CHECK: [[SAT_RES_INT:%[^ ]+]] = call <16 x i32> @llvm.genx.sstrunc.sat.v16i32.v16i32(<16 x i32> [[RES_INT:[^ ]+]])

template void kernel<uchar, int>(uchar, int);
// CHECK-LABEL: define {{.*}}kernel{{.*}}(i8{{.*}}, i32{{.*}})
// CHECK: [[SAT_RES_UCHAR:%[^ ]+]] = call <16 x i8> @llvm.genx.ustrunc.sat.v16i8.v16i32(<16 x i32> [[RES_UCHAR:[^ ]+]])

template void kernel<ushort, int>(ushort, int);
// CHECK-LABEL: define {{.*}}kernel{{.*}}(i16{{.*}}, i32{{.*}})
// CHECK: [[SAT_RES_USHORT:%[^ ]+]] = call <16 x i16> @llvm.genx.ustrunc.sat.v16i16.v16i32(<16 x i32> [[RES_USHORT:[^ ]+]])

