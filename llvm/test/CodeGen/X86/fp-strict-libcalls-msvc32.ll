; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -O3 -mtriple=i686-pc-windows-msvc -mattr=+cmov < %s | FileCheck %s

define float @ceil(float %x) #0 {
; CHECK-LABEL: ceil:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subl $12, %esp
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    fstpl (%esp)
; CHECK-NEXT:    calll _ceil
; CHECK-NEXT:    fstps {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    addl $12, %esp
; CHECK-NEXT:    retl
  %result = call float @llvm.experimental.constrained.ceil.f32(float %x, metadata !"fpexcept.strict") #0
  ret float %result
}

define float @cos(float %x) #0 {
; CHECK-LABEL: cos:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subl $12, %esp
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    fstpl (%esp)
; CHECK-NEXT:    calll _cos
; CHECK-NEXT:    fstps {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    addl $12, %esp
; CHECK-NEXT:    retl
  %result = call float @llvm.experimental.constrained.cos.f32(float %x, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  ret float %result
}

define float @exp(float %x) #0 {
; CHECK-LABEL: exp:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subl $12, %esp
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    fstpl (%esp)
; CHECK-NEXT:    calll _exp
; CHECK-NEXT:    fstps {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    addl $12, %esp
; CHECK-NEXT:    retl
  %result = call float @llvm.experimental.constrained.exp.f32(float %x, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  ret float %result
}

define float @floor(float %x) #0 {
; CHECK-LABEL: floor:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subl $12, %esp
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    fstpl (%esp)
; CHECK-NEXT:    calll _floor
; CHECK-NEXT:    fstps {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    addl $12, %esp
; CHECK-NEXT:    retl
  %result = call float @llvm.experimental.constrained.floor.f32(float %x, metadata !"fpexcept.strict") #0
  ret float %result
}

define float @frem(float %x, float %y) #0 {
; CHECK-LABEL: frem:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subl $20, %esp
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    fxch %st(1)
; CHECK-NEXT:    fstpl {{[0-9]+}}(%esp)
; CHECK-NEXT:    fstpl (%esp)
; CHECK-NEXT:    calll _fmod
; CHECK-NEXT:    fstps {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    addl $20, %esp
; CHECK-NEXT:    retl
  %result = call float @llvm.experimental.constrained.frem.f32(float %x, float %y, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  ret float %result
}

define float @log(float %x) #0 {
; CHECK-LABEL: log:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subl $12, %esp
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    fstpl (%esp)
; CHECK-NEXT:    calll _log
; CHECK-NEXT:    fstps {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    addl $12, %esp
; CHECK-NEXT:    retl
  %result = call float @llvm.experimental.constrained.log.f32(float %x, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  ret float %result
}

define float @log10(float %x) #0 {
; CHECK-LABEL: log10:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subl $12, %esp
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    fstpl (%esp)
; CHECK-NEXT:    calll _log10
; CHECK-NEXT:    fstps {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    addl $12, %esp
; CHECK-NEXT:    retl
  %result = call float @llvm.experimental.constrained.log10.f32(float %x, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  ret float %result
}

define float @pow(float %x, float %y) #0 {
; CHECK-LABEL: pow:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subl $20, %esp
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    fxch %st(1)
; CHECK-NEXT:    fstpl {{[0-9]+}}(%esp)
; CHECK-NEXT:    fstpl (%esp)
; CHECK-NEXT:    calll _pow
; CHECK-NEXT:    fstps {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    addl $20, %esp
; CHECK-NEXT:    retl
  %result = call float @llvm.experimental.constrained.pow.f32(float %x, float %y, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  ret float %result
}

define float @sin(float %x) #0 {
; CHECK-LABEL: sin:
; CHECK:       # %bb.0:
; CHECK-NEXT:    subl $12, %esp
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    fstpl (%esp)
; CHECK-NEXT:    calll _sin
; CHECK-NEXT:    fstps {{[0-9]+}}(%esp)
; CHECK-NEXT:    flds {{[0-9]+}}(%esp)
; CHECK-NEXT:    addl $12, %esp
; CHECK-NEXT:    retl
  %result = call float @llvm.experimental.constrained.sin.f32(float %x, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  ret float %result
}

attributes #0 = { strictfp }

declare float @llvm.experimental.constrained.ceil.f32(float, metadata)
declare float @llvm.experimental.constrained.cos.f32(float, metadata, metadata)
declare float @llvm.experimental.constrained.exp.f32(float, metadata, metadata)
declare float @llvm.experimental.constrained.floor.f32(float, metadata)
declare float @llvm.experimental.constrained.frem.f32(float, float, metadata, metadata)
declare float @llvm.experimental.constrained.log.f32(float, metadata, metadata)
declare float @llvm.experimental.constrained.log10.f32(float, metadata, metadata)
declare float @llvm.experimental.constrained.pow.f32(float, float, metadata, metadata)
declare float @llvm.experimental.constrained.sin.f32(float, metadata, metadata)
