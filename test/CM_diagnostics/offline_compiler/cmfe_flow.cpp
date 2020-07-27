// RUN: %cmoc %w -emit-spirv -o %W1.spv -I%cm_headers -mcpu=pvc
// RUN: %cmoc %w -emit-llvm -o %W1.ll -I%cm_headers -mcpu=pvc
// RUN: %cmoc %w -emit-llvm -S -o %W1.text.ll -I%cm_headers -mcpu=pvc
// RUN: FileCheck -input-file=%W1.text.ll --check-prefix=TEXT_LL %w
// TEXT_LL: @test_kernel
// RUN: rm %W1.spv
// RUN: rm %W1.ll
// RUN: rm %W1.text.ll

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

