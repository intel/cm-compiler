/*========================== begin_copyright_notice ============================

Copyright (C) 2026 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// Platforms older than Gen12 are no longer supported.

// RUN: not %cmc -emit-llvm -march=skl -- %s 2>&1 | FileCheck --check-prefix=CHECK-SKL %s
// RUN: not %cmc -emit-llvm -march=bdw -- %s 2>&1 | FileCheck --check-prefix=CHECK-BDW %s
// RUN: not %cmc -emit-llvm -march=icllp -- %s 2>&1 | FileCheck --check-prefix=CHECK-ICLLP %s
// RUN: not %cmc -emit-llvm -march=gen9 -- %s 2>&1 | FileCheck --check-prefix=CHECK-GEN9 %s
// RUN: not %cmc -emit-llvm -march=11.0.0 -- %s 2>&1 | FileCheck --check-prefix=CHECK-GMD %s
// Gen9 (SKL) PCI id.
// RUN: not %cmc -emit-llvm -march=0x1912 -- %s 2>&1 | FileCheck --check-prefix=CHECK-PCI %s

// The oldest supported platform still works.
// RUN: %cmc -emit-llvm -march=tgllp -S -o /dev/null -- %s

// CHECK-SKL: invalid arch name 'skl'
// CHECK-BDW: invalid arch name 'bdw'
// CHECK-ICLLP: invalid arch name 'icllp'
// CHECK-GEN9: invalid arch name 'gen9'
// CHECK-GMD: invalid arch name '11.0.0'
// CHECK-PCI: invalid arch name '0x1912'

#include <cm/cm.h>

_GENX_MAIN_ void test(SurfaceIndex idx) {}
