/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc %dump-ast-for test_func -- %s 2>&1 | FileCheck %s

#include <cm/cm.h>

_GENX_ void test_func() {
  vector<unsigned short, 8> vec = {1, 2, 3, 5, 7, 11, 13, 17};
// CHECK:       CompoundStmt 0x{{[^ ]+}} <col:{{[0-9]+}}, line:{{[0-9]+}}:{{[0-9]+}}>
// CHECK-NEXT:    DeclStmt 0x{{[^ ]+}} <line:{{[0-9]+}}:{{[0-9]+}}, col:{{[0-9]+}}>
// CHECK-NEXT:      VarDecl 0x{{[^ ]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> col:{{[0-9]+}} vec 'vector<unsigned short,8>' cinit
// CHECK-NEXT:        InitListExpr 0x{{[^ ]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'vector<unsigned short,8>'
// CHECK-NEXT:          ImplicitCastExpr 0x{{[^ ]+}} <col:{{[0-9]+}}> 'unsigned short' <IntegralCast>
// CHECK-NEXT:            IntegerLiteral 0x{{[^ ]+}} <col:{{[0-9]+}}> 'int' 1
// CHECK-NEXT:          ImplicitCastExpr 0x{{[^ ]+}} <col:{{[0-9]+}}> 'unsigned short' <IntegralCast>
// CHECK-NEXT:            IntegerLiteral 0x{{[^ ]+}} <col:{{[0-9]+}}> 'int' 2
// CHECK-NEXT:          ImplicitCastExpr 0x{{[^ ]+}} <col:{{[0-9]+}}> 'unsigned short' <IntegralCast>
// CHECK-NEXT:            IntegerLiteral 0x{{[^ ]+}} <col:{{[0-9]+}}> 'int' 3
// CHECK-NEXT:          ImplicitCastExpr 0x{{[^ ]+}} <col:{{[0-9]+}}> 'unsigned short' <IntegralCast>
// CHECK-NEXT:            IntegerLiteral 0x{{[^ ]+}} <col:{{[0-9]+}}> 'int' 5
// CHECK-NEXT:          ImplicitCastExpr 0x{{[^ ]+}} <col:{{[0-9]+}}> 'unsigned short' <IntegralCast>
// CHECK-NEXT:            IntegerLiteral 0x{{[^ ]+}} <col:{{[0-9]+}}> 'int' 7
// CHECK-NEXT:          ImplicitCastExpr 0x{{[^ ]+}} <col:{{[0-9]+}}> 'unsigned short' <IntegralCast>
// CHECK-NEXT:            IntegerLiteral 0x{{[^ ]+}} <col:{{[0-9]+}}> 'int' 11
// CHECK-NEXT:          ImplicitCastExpr 0x{{[^ ]+}} <col:{{[0-9]+}}> 'unsigned short' <IntegralCast>
// CHECK-NEXT:            IntegerLiteral 0x{{[^ ]+}} <col:{{[0-9]+}}> 'int' 13
// CHECK-NEXT:          ImplicitCastExpr 0x{{[^ ]+}} <col:{{[0-9]+}}> 'unsigned short' <IntegralCast>
// CHECK-NEXT:            IntegerLiteral 0x{{[^ ]+}} <col:{{[0-9]+}}> 'int' 17
// CHECK-NOT:          ImplicitCastExpr
}
