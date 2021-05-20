// RUN: %cmc -mcpu=PVC %s -emit-llvm -S -o %t.text.ll
// RUN: FileCheck -input-file=%t.text.ll --check-prefix=CHECK-PVC %s
// RUN: %cmc -mcpu=SKL %s -emit-llvm -S -o %t.text.ll
// RUN: FileCheck -input-file=%t.text.ll --check-prefix=CHECK-SKL %s
// RUN: %cmc -mcpu=TGL %s -emit-llvm -S -o %t.text.ll
// RUN: FileCheck -input-file=%t.text.ll --check-prefix=CHECK-TGL %s
// CHECK-PVC: "target-cpu"="PVC"
// CHECK-SKL: "target-cpu"="SKL"
// CHECK-TGL: "target-cpu"="TGL"


#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

