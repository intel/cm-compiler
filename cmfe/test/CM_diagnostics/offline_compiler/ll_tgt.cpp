// RUN: %cmoc -mcpu=PVC %w -I%cm_headers -emit-llvm -S -o %W.text.ll
// RUN: FileCheck -input-file=%W.text.ll --check-prefix=CHECK-PVC %w
// RUN: %cmoc -mcpu=SKL %w -I%cm_headers -emit-llvm -S -o %W.text.ll
// RUN: FileCheck -input-file=%W.text.ll --check-prefix=CHECK-SKL %w
// RUN: %cmoc -mcpu=TGL %w -I%cm_headers -emit-llvm -S -o %W.text.ll
// RUN: FileCheck -input-file=%W.text.ll --check-prefix=CHECK-TGL %w
// RUN: rm %W.text.ll
// CHECK-PVC: "target-cpu"="PVC"
// CHECK-SKL: "target-cpu"="SKL"
// CHECK-TGL: "target-cpu"="TGL"


#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

