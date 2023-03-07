/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -mcpu=SKL -emit-llvm -S -o %t.text.ll -- %s
// RUN: FileCheck -input-file=%t.text.ll --check-prefix=CHECK-SKL %s
// RUN: %cmc -mcpu=TGL -emit-llvm -S -o %t.text.ll -- %s
// RUN: FileCheck -input-file=%t.text.ll --check-prefix=CHECK-TGL %s
// RUN: %cmc -mcpu=PVC -emit-llvm -S -o %t.text.ll -- %s
// RUN: FileCheck -input-file=%t.text.ll --check-prefix=CHECK-PVC %s

// CHECK-SKL: "target-cpu"="9.0.9"
// CHECK-TGL: "target-cpu"="12.0.0"
// CHECK-PVC: "target-cpu"="12.60.7"

#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}
