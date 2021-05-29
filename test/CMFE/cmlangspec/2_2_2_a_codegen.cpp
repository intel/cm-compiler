// RUN: %cmc -g0 -march=SKL -emit-llvm -S -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll

// Check that row() generates rdregion from matrix that is stored to vector.

#include <cm/cm.h>

// CHECK-LABEL: @test_func(
extern "C"
_GENX_ void test_func() {
  matrix<short, 4, 8> m;
// CHECK:      [[MATRIX:%[^ ]+]] = alloca <32 x i16>
// CHECK-NEXT: [[VECTOR:%[^ ]+]] = alloca <8 x i16>
// CHECK-NEXT: [[LOAD:%[^ ]+]] = load <32 x i16>, <32 x i16>* [[MATRIX]]
// CHECK-NEXT: [[ROWS:%[^ ]+]] = call <8 x i16> @llvm.genx.rdregioni.v8i16.v32i16.i16(<32 x i16> [[LOAD]], i32 0, i32 8, i32 1, i16 32, i32 8)
// CHECK-NEXT: [[COLS:%[^ ]+]] = call <8 x i16> @llvm.genx.rdregioni.v8i16.v8i16.i16(<8 x i16> [[ROWS]], i32 8, i32 8, i32 1, i16 0, i32 8)
// CHECK-NEXT: store <8 x i16> [[COLS]], <8 x i16>* [[VECTOR]]
  vector<short, 8> r(m.row(2));
}
