/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// XFAIL: *

// The test expects that the resulting assembly file should mention all the
// platforms passed to the finalizer.
// "SKL" is passed as a result of -mcpu argument,
// -mllvm -finalizer-opts should result in appending "TGL" platform to the
// finalizer options

// RUN: %cmc -emit-llvm -mcpu=SKL -mCM_genx_assembler="%genxir" -mllvm -finalizer-opts="-nocompaction" %s
// RUN: FileCheck --input-file %W_0.asm %s
// CHECK: -nocompaction

#include <cm/cm.h>

_GENX_MAIN_
void hacked_name() {
}
