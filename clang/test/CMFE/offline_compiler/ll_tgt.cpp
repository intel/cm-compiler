/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -mcpu=TGL -emit-llvm -S -o %t.text.ll -- %s
// RUN: FileCheck -input-file=%t.text.ll --check-prefix=CHECK-TGL %s
// RUN: %cmc -mcpu=PVC -emit-llvm -S -o %t.text.ll -- %s
// RUN: FileCheck -input-file=%t.text.ll --check-prefix=CHECK-PVC %s

// CHECK-TGL: "target-cpu"="12.0.0"
// CHECK-PVC: "target-cpu"="12.60.7"

extern "C" _GENX_MAIN_
void test_kernel() {
}
