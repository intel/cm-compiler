/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

constexpr auto src1_precision = CmPrecisionType::CM_Precision_BF16;
constexpr auto src2_precision = CmPrecisionType::CM_Precision_BF16;

using res_t = float;
using src1_t = unsigned;
using src2_t = unsigned;

constexpr unsigned exec_size = CM_GRF_WIDTH / (sizeof(res_t) * 8);
constexpr unsigned repeat_count = 8;
constexpr unsigned systolic_depth = 8;

constexpr auto N = exec_size * repeat_count;
constexpr auto N1 = systolic_depth * exec_size;
constexpr auto N2 = repeat_count * systolic_depth;

auto test_dpasw(vector<src1_t, N1> src1, vector<src2_t, N2> src2) {
  return cm_dpas<src1_precision, src2_precision, systolic_depth, repeat_count, res_t, src1_t, src2_t, N>(NULL, src1, src2);
}

// RUN: %cmc -emit-llvm -march=acm-g10  -- %s 2>&1 | FileCheck --allow-empty --implicit-check-not error: %s
// RUN: %cmc -emit-llvm -march=acm-g11  -- %s 2>&1 | FileCheck --allow-empty --implicit-check-not error: %s
// RUN: %cmc -emit-llvm -march=acm-g12  -- %s 2>&1 | FileCheck --allow-empty --implicit-check-not error: %s
// RUN: %cmc -emit-llvm -march=arl-h  -- %s 2>&1 | FileCheck --allow-empty --implicit-check-not error: %s
// RUN: %cmc -emit-llvm -march=pvc  -- %s 2>&1 | FileCheck --allow-empty --implicit-check-not error: %s
// RUN: %cmc -emit-llvm -march=lnl-m  -- %s 2>&1 | FileCheck --allow-empty --implicit-check-not error: %s
//
// Checks that dpas is available on supported platforms.

// RUN: %cmc -emit-llvm -march=tgllp -- %s 2>&1 | FileCheck --check-prefix NOSUPPORT %s
// RUN: %cmc -emit-llvm -march=mtl-u -- %s 2>&1 | FileCheck --check-prefix NOSUPPORT %s
// RUN: %cmc -emit-llvm -march=mtl-p -- %s 2>&1 | FileCheck --check-prefix NOSUPPORT %s
// RUN: %cmc -emit-llvm -march=arl-s -- %s 2>&1 | FileCheck --check-prefix NOSUPPORT %s
// RUN: %cmc -emit-llvm -march=pvc-vg -- %s 2>&1 | FileCheck --check-prefix NOSUPPORT %s
//
// Checks that dpas is unavailable on unsupported platforms.

// NOSUPPORT: error:
