/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -emit-llvm -- %s 2>&1 | FileCheck --check-prefix=CHECK-NONE %s
// RUN: not %cmc -emit-llvm -march=xxx -- %s 2>&1 | FileCheck --check-prefix=CHECK-XXX %s
// RUN: not %cmc -emit-llvm -mcpu=yyy -- %s 2>&1 | FileCheck --check-prefix=CHECK-YYY %s
// RUN: not %cmc -emit-llvm -march=xxx -mcpu=yyy -- %s 2>&1 | FileCheck --check-prefix=CHECK-YYY %s

// CHECK-NONE: invalid arch name 'unknown'
// CHECK-XXX: invalid arch name 'xxx'
// CHECK-YYY: invalid arch name 'yyy'
