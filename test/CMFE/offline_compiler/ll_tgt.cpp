// RUN: %cmc -mcpu=SKL -emit-llvm -S -o %t.text.ll -- %s
// RUN: FileCheck -input-file=%t.text.ll --check-prefix=CHECK-SKL %s

// CHECK-SKL: "target-cpu"="SKL"

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

