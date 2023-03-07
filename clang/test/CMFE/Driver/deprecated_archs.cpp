/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -emit-llvm -mcpu=bdw -- %s 2>&1 | FileCheck %s --check-prefix=WARN-BDW
// WARN-BDW: Target 'bdw' is deprecated and will be removed in next SDK

// RUN: %cmc -emit-llvm -mcpu=bxt -- %s 2>&1 | FileCheck %s --check-prefix=WARN-BXT
// WARN-BXT: Target 'bxt' is deprecated and will be removed in next SDK

// RUN: %cmc -emit-llvm -mcpu=glk -- %s 2>&1 | FileCheck %s --check-prefix=WARN-GLK
// WARN-GLK: Target 'glk' is deprecated and will be removed in next SDK

// RUN: %cmc -emit-llvm -mcpu=skl -- %s 2>&1 | FileCheck %s --allow-empty --implicit-check-not warning

// XFAIL: *
