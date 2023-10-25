; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=thumbv8.1m.main-none-none-eabi -mattr=+mve -verify-machineinstrs %s -o - | FileCheck %s

define void @g(i8* %v) {
; CHECK-LABEL: g:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    movs r0, #63
; CHECK-NEXT:    vmov.i32 q0, #0x0
; CHECK-NEXT:    vmsr p0, r0
; CHECK-NEXT:    vpst
; CHECK-NEXT:    vstrbt.8 q0, [r0]
; CHECK-NEXT:    bx lr
entry:
  %0 = load i8, i8* %v, align 1
  %conv = zext i8 %0 to i32
  %broadcast.splatinsert = insertelement <16 x i32> undef, i32 %conv, i32 0
  %broadcast.splat = shufflevector <16 x i32> %broadcast.splatinsert, <16 x i32> undef, <16 x i32> zeroinitializer
  %1 = and <16 x i32> %broadcast.splat, <i32 1, i32 2, i32 4, i32 8, i32 16, i32 32, i32 64, i32 128, i32 256, i32 512, i32 1024, i32 2048, i32 4096, i32 8192, i32 16384, i32 32768>
  %2 = icmp eq <16 x i32> %1, zeroinitializer
  %3 = select <16 x i1> %2, <16 x i8> zeroinitializer, <16 x i8> <i8 0, i8 0, i8 0, i8 0, i8 0, i8 0, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef, i8 undef>
  call void @llvm.masked.store.v16i8.p0v16i8(<16 x i8> %3, <16 x i8>* undef, i32 1, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false>)
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.masked.store.v16i8.p0v16i8(<16 x i8>, <16 x i8>*, i32 immarg, <16 x i1>) #1
