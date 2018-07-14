// RUN: %cmc %w 2>&1 | FileCheck %w

#include <cm/cm.h>

matrix<int,6,4> m;
vector<int,4> v;
struct S1 {
  int any;
} s1;
struct S2 {
  int x;
} s2;
_GENX_MAIN_
void test() {
  unsigned short r1 = m.any;             // missing '()'
  unsigned short r2 = m.any(;            // missing ')'
  unsigned short r3 = m.any(1;           // unexpected expression
  unsigned short r4 = m.any(9);          // unexpected expression
  unsigned short r5 = m.any();           // OK
  unsigned short r6 = m.template any();  // OK
  unsigned short r7 = v.any();           // OK
  unsigned short r8 = v.template any();  // OK
  unsigned short r9 = s1.any;            // OK
  unsigned short r10 = s1.template any;  // not a template
  unsigned short r11 = s2.any();         // no member any
  m.any();                               // expression result unused
  v.any();                               // expression result unused
  m.any() = 1;                           // not assignable
  v.any() = 0;                           // not assignable
  v.any().any();                         // ushort not a structure or union
}
// CHECK: any.cpp(15,28):  error: expected '('
// CHECK: any.cpp(16,29):  error: expected ')'
// CHECK: any.cpp(17,29):  error: expected ')'
// CHECK: any.cpp(18,29):  error: expected ')'
// CHECK: any.cpp(24,36):  error: 'any' following the 'template' keyword does not refer to a template
// CHECK: any.cpp(25,27):  error: no member named 'any' in 'S2'
// CHECK: any.cpp(28,11):  error: expression is not assignable
// CHECK: any.cpp(29,11):  error: expression is not assignable
// CHECK: any.cpp(30,10):  error: member reference base type 'unsigned short' is not a structure or union
// CHECK: any.cpp(26,3):  warning: expression result unused [-Wunused-value]
// CHECK: any.cpp(27,3):  warning: expression result unused [-Wunused-value]
// CHECK: 2 warnings and 9 errors generated.
