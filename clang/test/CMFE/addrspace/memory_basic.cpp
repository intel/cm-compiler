/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=skl -fcm-pointer -g0 -S -emit-llvm -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

#include <cm/cm.h>

void check_sg_vector(vector<__local int *, 4> fromVec, vector<__global int *, 4> toVec) {
  // CHECK: call <4 x i32> @llvm.masked.gather.v4i32.v4p4i32(<4 x i32 addrspace(4)*> %{{[^,]+}}, i32 8
  vector<int, 4> res1 = gather<int, 4, Align::QWORD>(fromVec);

  vector<ushort, 4> mask{1, 0, 1, 0};
  vector<int, 4> passthru{42, 42, 42, 42};
  vector<int, 4> res2 = gather<int, 4, Align::QWORD>(fromVec, mask, passthru);

  // CHECK: call void @llvm.masked.scatter.v4i32.v4p4i32(<4 x i32> %{{[^,]+}}, <4 x i32 addrspace(4)*> %{{[^,]+}}, i32 8
  scatter<int, 4, Align::QWORD>(res1 + res2, toVec);
}

void check_sg_matrix(matrix<__local int *, 2, 2> fromMatrix, matrix<__global int *, 2, 2> toMatrix) {
  // CHECK: call <4 x i32> @llvm.masked.gather.v4i32.v4p4i32(<4 x i32 addrspace(4)*> %{{[^,]+}}, i32 4
  matrix<int, 2, 2> res1 = gather<int, 2, 2>(fromMatrix);

  matrix<ushort, 2, 2> mask;
  mask.row(0) = 1;
  mask.row(1) = 0;

  matrix<int, 2, 2> passthru;
  passthru.row(0) = 42;
  passthru.row(1) = 42;

  matrix<int, 2, 2> res2 = gather<int, 2, 2>(fromMatrix, mask, passthru);
  // CHECK: call void @llvm.masked.scatter.v4i32.v4p4i32(<4 x i32> %{{[^,]+}}, <4 x i32 addrspace(4)*> %{{[^,]+}}, i32 4
  scatter<int, 2, 2>(res1 + res2, toMatrix);
}

extern "C" _GENX_MAIN_ void kernel(__local int *lptr, __global int *gptr) {
  // CHECK: load i32, i32 addrspace(4)* %{{[^,]+}}, align 16
  int a = load<int, Align::OWORD>(lptr);

  // CHECK: store i32 %{{[^,]+}}, i32 addrspace(4)* %{{[^,]+}}, align 16
  store<int, Align::OWORD>(a, gptr);

  vector<__local int *, 4> fromVec{lptr, lptr, lptr, lptr};
  vector<__global int *, 4> toVec{gptr, gptr, gptr, gptr};
  check_sg_vector(fromVec, toVec);

  matrix<__local int *, 2, 2> fromMatrix = fromVec;
  matrix<__global int *, 2, 2> toMatrix = toVec;
  check_sg_matrix(fromMatrix, toMatrix);
}
