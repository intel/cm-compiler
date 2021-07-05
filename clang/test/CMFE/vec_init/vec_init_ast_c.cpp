// RUN: %cmc %dump-ast-for test_func -- %s 2>&1 | FileCheck %s

#include <cm/cm.h>

_GENX_ void test_func() {
  vector<unsigned short, 8> vec = {};
// CHECK:       CompoundStmt 0x{{[^ ]+}} <col:{{[0-9]+}}, line:{{[0-9]+}}:{{[0-9]+}}>
// CHECK-NEXT:    DeclStmt 0x{{[^ ]+}} <line:{{[0-9]+}}:{{[0-9]+}}, col:{{[0-9]+}}>
// CHECK-NEXT:      VarDecl 0x{{[^ ]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> col:{{[0-9]+}} vec 'vector<unsigned short,8>' cinit
// CHECK-NEXT:        InitListExpr 0x{{[^ ]+}} <col:{{[0-9]+}}, col:{{[0-9]+}}> 'vector<unsigned short,8>'
// CHECK-NOT:          ImplicitCastExpr
}
