; RUN: opt < %s -instcombine -S | FileCheck %s

; CHECK: uitofp i8
; CHECK-NEXT: ret float
define float @foo(i8 %x) {
entry:
  %conv2 = zext i8 %x to i32
  %conv3 = sitofp i32 %conv2 to float
  ret float %conv3
}
