// RUN: %cmc %w 2>&1 | FileCheck %w

#include <cm/cm.h>

matrix<int,4,6> m;
vector<int,4> v;
int i;
SurfaceIndex si;
struct S1 {
  int a;
} s1;
struct S2 {
  int n_cols;
} s2;

_GENX_MAIN_
void test() {
  int r1 = m.n_cols;                // expected '('
  int r2 = m.n_cols(;               // expected ')'
  int r3 = m.n_cols(;               // expected ')'
  int r4 = m.n_cols();              // OK
  int r5 = m.n_cols(0);             // unexpected expression
  int r6 = m.template n_cols();     // OK
  int r7 = v.n_cols();              // not a matrix
  int r8 = v.template n_cols();     // not a matrix
  int r9 = s1.n_cols;               // no n_cols in s1
  int r10 = s1.n_cols();            // no n_cols() in s1
  int r11 = s2.n_cols;              // OK
  int r12 = s2.n_cols();            // n_cols not a function
  int r13 = s2.template n_cols();   // not a template
  s2.n_cols = 1;                    // OK
  s2.template n_cols = 2;           // not a template
  m.n_cols() = 2;                   // not assignable
}
// CHECK: n_cols.cpp(18,20):  error: expected '('
// CHECK: n_cols.cpp(19,21):  error: expected ')'
// CHECK: n_cols.cpp(20,21):  error: expected ')'
// CHECK: n_cols.cpp(22,21):  error: expected ')'
// CHECK: n_cols.cpp(24,14):  error: n_cols() member function is only valid for matrix/matrix_ref
// CHECK: n_cols.cpp(25,23):  error: n_cols() member function is only valid for matrix/matrix_ref
// CHECK: n_cols.cpp(26,15):  error: no member named 'n_cols' in 'S1'
// CHECK: n_cols.cpp(27,16):  error: no member named 'n_cols' in 'S1'
// CHECK: n_cols.cpp(29,22):  error: called object type 'int' is not a function or function pointer
// CHECK: n_cols.cpp(30,25):  error: 'n_cols' following the 'template' keyword does not refer to a template
// CHECK: n_cols.cpp(32,15):  error: 'n_cols' following the 'template' keyword does not refer to a template
// CHECK: n_cols.cpp(33,14):  error: expression is not assignable
// CHECK: 12 errors generated.
