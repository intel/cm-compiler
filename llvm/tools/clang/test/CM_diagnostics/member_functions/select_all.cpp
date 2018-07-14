// RUN: %cmc %w 2>&1 | FileCheck %w

#include <cm/cm.h>

matrix<int,4,6> m;
vector<int,4> v;
struct S1 {
  int a;
} s1;
struct S2 {
  int select_all;
} s2;

_GENX_MAIN_
void test() {
  matrix_ref<int, 4, 6> r1 = m.select_all;                // expected '('
  matrix_ref<int, 4, 6> r2 = m.select_all(;               // expected ')'
  matrix_ref<int, 4, 6> r3 = m.select_all(1;              // expected ')'
  matrix_ref<int, 4, 6> r5 = m.select_all(0);             // unexpected expression
  matrix_ref<int, 4, 6> r4 = m.select_all();              // OK
  matrix_ref<int, 4, 6> r6 = m.template select_all();     // OK
  matrix_ref<int, 4, 6> r7 = m.select_all().select_all(); // OK

  vector_ref<int, 4> r8 = v.select_all;                   // expected '('
  vector_ref<int, 4> r9 = v.select_all(;                  // expected ')'
  vector_ref<int, 4> r10 = v.select_all(3;                // expected ')'
  vector_ref<int, 4> r11 = v.select_all(5);               // unexpected expression
  vector_ref<int, 4> r12 = v.select_all();                // OK
  vector_ref<int, 4> r13 = v.template select_all();       // OK
  vector_ref<int, 4> r14 = v.select_all().select_all();   // OK

  int r15 = s1.select_all;                                // no member select_all in s1
  int r16 = s1.select_all();                              // no member select_all in s1
  int r17 = s1.template select_all;                       // select_all not a template
  int r18 = s2.select_all;                                // OK
  int r19 = s2.select_all();                              // select_all not a function
  int r20 = s2.template select_all();                     // select_all not a template

  m.select_all();                                         // expression result unused
  v.select_all();                                         // expression result unused
  m.select_all() = 1;                                     // OK
  v.select_all() = 9;                                     // OK
}
// CHECK: select_all.cpp(16,42):  error: expected '('
// CHECK: select_all.cpp(17,43):  error: expected ')'
// CHECK: select_all.cpp(18,43):  error: expected ')'
// CHECK: select_all.cpp(19,43):  error: expected ')'
// CHECK: select_all.cpp(24,39):  error: expected '('
// CHECK: select_all.cpp(25,40):  error: expected ')'
// CHECK: select_all.cpp(26,41):  error: expected ')'
// CHECK: select_all.cpp(27,41):  error: expected ')'
// CHECK: select_all.cpp(32,16):  error: no member named 'select_all' in 'S1'
// CHECK: select_all.cpp(34,25):  error: 'select_all' following the 'template' keyword does not refer to a template
// CHECK: select_all.cpp(36,26):  error: called object type 'int' is not a function or function pointer
// CHECK: select_all.cpp(37,25):  error: 'select_all' following the 'template' keyword does not refer to a template
// CHECK: select_all.cpp(39,3):  warning: expression result unused [-Wunused-value]
// CHECK: select_all.cpp(40,3):  warning: expression result unused [-Wunused-value]
// CHECK: 2 warnings and 13 errors generated.
