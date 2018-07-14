// RUN: %cmc %w 2>&1 | FileCheck %w

#include <cm/cm.h>

matrix<int,4,6> m;
vector<int,4> v;
int ia;
SurfaceIndex si;
struct S1 {
  int a;
} s1;
struct S2 {
  int column;
} s2;

_GENX_MAIN_
void test() {
  vector<int, 4> r1 = m.column;                // expected '('
  vector<int, 4> r2 = m.column(;               // expected expression
  vector<int, 4> r3 = m.column(1;              // expected ')'
  vector<int, 4> r4 = m.column();              // expected expression
  vector<int, 4> r5 = m.column(0);             // OK
  vector<int, 4> r6 = m.template column(1);    // OK
  vector<int, 4> r7 = m.column(0);             // OK
  vector<int, 4> r8 = v.column(0);             // not a matrix
  vector<int, 4> r9 = v.template column(0);    // not a matrix
  vector<int, 4> r10 = m.column(-1);           // index negative
  vector<int, 4> r11 = m.column(5);            // OK
  vector<int, 4> r12 = m.column(6);            // index out of bounds
  vector<int, 4> r13 = m.column(ia);           // OK
  vector<int, 4> r14 = m.column(si);           // index not an integer
  m.column(0);                                 // expression result unused
  s1.column(2);                                // no column() in s
  s2.column = 1;                               // OK
}
// CHECK: column.cpp(18,31):  error: expected '('
// CHECK: column.cpp(19,32):  error: expected expression
// CHECK: column.cpp(20,33):  error: expected ')'
// CHECK: column.cpp(21,32):  error: expected expression
// CHECK: column.cpp(25,25):  error: column() member function is only valid for matrix/matrix_ref
// CHECK: column.cpp(26,34):  error: column() member function is only valid for matrix/matrix_ref
// CHECK: column.cpp(27,33):  error: column index must be positive
// CHECK: column.cpp(29,33):  warning: column index '6' is out of bounds, matrix has 6 columns [-Wcm-bounds-check]
// CHECK: column.cpp(31,33):  error: column index must be an integer expression
// CHECK: column.cpp(33,6):  error: no member named 'column' in 'S1'
// CHECK: column.cpp(32,3):  warning: expression result unused [-Wunused-value]
// CHECK: 2 warnings and 9 errors generated.
