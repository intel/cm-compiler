/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -mcpu=skl -S -emit-llvm -cm-printf-spec=ocl -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix PRINTF-CHECK
// RUN: %cmc -g0 -mcpu=skl -S -emit-llvm -cm-printf-spec ocl -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix PRINTF-CHECK
// RUN: %cmc -g0 -mcpu=skl -S -emit-llvm -cm-printf-spec=legacy -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix PRINTF-CHECK
// RUN: %cmc -g0 -mcpu=skl -S -emit-llvm -cm-printf-spec legacy -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix PRINTF-CHECK
// RUN: %cmc -g0 -mcpu=skl -S -emit-llvm -cm-printf-spec=my-spec -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix PRINTF-CHECK
// RUN: %cmc -g0 -mcpu=skl -S -emit-llvm -cm-printf-spec my-spec -- %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix PRINTF-CHECK

// PRINTF-CHECK: warning: '-cm-printf-spec{{.*}}' is ignored
