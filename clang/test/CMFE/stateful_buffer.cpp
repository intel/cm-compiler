/*========================== begin_copyright_notice ============================

Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

_GENX_MAIN_
void test(SurfaceIndex buf [[type("buffer_t")]]) {
}

// RUN: %cmc -emit-llvm -march=dg2 -- %s 2>&1 | count 0
// RUN: %cmc -emit-llvm -march=pvc -Wno-cm-buffer-not-supported -- %s 2>&1 | count 0

// RUN: %cmc -emit-llvm -march=pvc -- %s 2>&1 | FileCheck --check-prefix=PVC %s
// PVC: stateful_buffer.cpp(19{{, ?}}24): warning: stateful buffer 'buffer_t' is not supported by the level zero runtime on the given target
// PVC: void test(SurfaceIndex buf
// PVC: 1 warning generated.
