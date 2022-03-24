/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: not %cmc -fcmocl -emit-llvm -- %s 2>&1 | FileCheck %s
// RUN: not %cmc -fcmocl -emit-llvm -march=xxx -- %s 2>&1 | FileCheck %s
// RUN: not %cmc -fcmocl -emit-llvm -mcpu=yyy -- %s 2>&1 | FileCheck %s
// RUN: not %cmc -fcmocl -emit-llvm -march=xxx -mcpu=yyy -- %s 2>&1 | FileCheck %s

// CHECK: Unknown target achitecture, please use -march=<T> with correct target
