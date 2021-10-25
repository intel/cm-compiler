/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc %dump-ast-for test_func -- %s 2>&1 | FileCheck %s

// Check that row() returns vector_ref to matrix row.

#include <cm/cm.h>

extern "C"
_GENX_ void test_func() {
  matrix<short, 4, 8> m;
// CHECK:      CMSelectExpr 0x{{[^ ]*}} <col:26, col:33> 'vector_ref<short,8>' lvalue Kind = 'row'
// CHECK-NEXT:   DeclRefExpr 0x{{[^ ]*}} <col:26> 'matrix<short,4,8>' lvalue Var 0x{{[^ ]*}} 'm'
// CHECK-NEXT:   ImplicitCastExpr 0x{{[^ ]*}} <col:32> 'unsigned short' <IntegralCast>
// CHECK-NEXT:     IntegerLiteral 0x{{[^ ]*}} <col:32> 'int' 2
  vector_ref<short, 8> r(m.row(2));
}
