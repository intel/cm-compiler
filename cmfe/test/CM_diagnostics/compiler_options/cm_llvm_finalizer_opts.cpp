// The test expects that the resulting assembly file should mention all the
// platforms passed to the finalizer.
// "SKL" is passed as a result of -mcpu argument,
// -mllvm -finalizer-opts should result in appending "TGL" platform to the
// finalizer options

// RUN: %cmc -mCM_printfargs -mcpu=SKL -mCM_genx_assembler="%genxir" -mdump_asm -mCM_old_asm_name -mllvm -finalizer-opts="-nocompaction" %w
// RUN: FileCheck --input-file %W_0.asm %w
// CHECK: -nocompaction
// RUN: rm %W.isa
// RUN: rm %W_0.asm
// RUN: rm %W_0.dat
// RUN: rm %W_0.visaasm

#include <cm/cm.h>

_GENX_MAIN_
void hacked_name() {
}
