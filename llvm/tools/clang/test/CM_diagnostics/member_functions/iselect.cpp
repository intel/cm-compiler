// RUN: %cmc -ferror-limit=99 %w 2>&1 | FileCheck %w

#include <cm/cm.h>

matrix<int,10,6> m;
vector<int,8> v;
vector<ushort,4> idx1;
vector<ushort,4> idx2;
vector<ushort,6> idx3;
vector<float,6> idx4;
ushort idx5;
struct S1 {
  int a;
} s1;
struct S2 {
  int iselect;
} s2;

_GENX_MAIN_
void test() {
  vector<int,4> r1 = v.iselect;                                 // expected '('
  vector<int,4> r2 = v.iselect(;                                // expected index
  vector<int,4> r3 = v.iselect();                               // expected index
  vector<int,4> r4 = v.iselect(idx1;                            // expected ')'
  vector<int,4> r5 = v.iselect(idx1);                           // OK
  vector<int,4> r6 = v.iselect(idx4);                           // index not ushort vector
  vector<int,4> r7 = v.iselect(idx5);                           // index not vector
  vector<int,4> r8 = v.iselect(idx1,idx3);                      // one index expected
  vector<int,4> r9 = v.template iselect(idx1);                  // OK
  vector<int,6> r10 = v.iselect(idx1).iselect(idx3);            // OK
  vector<int,6> r11 = v.iselect(idx1).iselect(idx1,idx2);       // one index expected

  vector<int,4> r12 = m.iselect;                                // expected '('
  vector<int,4> r13 = m.iselect(;                               // expected index
  vector<int,4> r14 = m.iselect();                              // expected index
  vector<int,4> r15 = m.iselect(idx1;                           // expected ')'
  vector<int,4> r16 = m.iselect(idx1);                          // two indices expected
  vector<int,4> r17 = m.iselect(idx1,idx2);                     // OK
  vector<int,4> r18 = m.iselect(idx1,idx2,idx3,idx1);           // two indices expected
  vector<int,4> r19 = m.iselect(idx4,idx3);                     // index not ushort
  vector<int,4> r20 = m.iselect(idx3,idx4);                     // index not ushort
  vector<int,4> r21 = m.iselect(idx1,idx3);                     // indices different size
  vector<int,4> r22 = m.template iselect(idx1,idx2);            // OK
  vector<int,6> r23 = m.iselect(idx1,idx3).iselect(idx3);       // OK
  vector<int,6> r24 = m.iselect(idx1,idx2).iselect(idx2,idx1);  // one index expected

  v.iselect(idx3);                                      // expression result unused
  m.iselect(idx1,idx2);                                 // expression result unused
  v.iselect(idx1) = 9;                                  // not assignable
  m.iselect(idx1,idx2) = 1;                             // not assignable

  int r30 = s1.iselect;                                 // no member iselect in s1
  int r31 = s1.iselect(idx1);                           // no member iselect in s1
  int r32 = s1.template iselect;                        // iselect not a template
  int r33 = s2.iselect;                                 // OK
  int r34 = s2.iselect(idx1);                           // iselect not a function
  int r35 = s2.template iselect(idx1);                  // iselect not a template
}
// CHECK: iselect.cpp(21,31):  error: expected '('
// CHECK: iselect.cpp(22,32):  error: expected expression
// CHECK: iselect.cpp(23,32):  error: expected expression
// CHECK: iselect.cpp(24,36):  error: expected ')'
// CHECK: iselect.cpp(26,32):  error: iselect expects vector<unsigned short, N> index type, 'vector<float,6>'
// CHECK: iselect.cpp(27,32):  error: iselect expects vector<unsigned short, N> index type, 'ushort' (aka 'unsigned short')
// CHECK: iselect.cpp(28,37):  error: vector iselect expects 1 index expression
// CHECK: iselect.cpp(31,52):  error: vector iselect expects 1 index expression
// CHECK: iselect.cpp(33,32):  error: expected '('
// CHECK: iselect.cpp(34,33):  error: expected expression
// CHECK: iselect.cpp(35,33):  error: expected expression
// CHECK: iselect.cpp(36,37):  error: expected ')'
// CHECK: iselect.cpp(37,37):  error: matrix iselect expects 2 index expressions
// CHECK: iselect.cpp(39,43):  error: matrix iselect expects 2 index expressions
// CHECK: iselect.cpp(40,33):  error: iselect expects vector<unsigned short, N> index type, 'vector<float,6>'
// CHECK: iselect.cpp(41,38):  error: iselect expects vector<unsigned short, N> index type, 'vector<float,6>'
// CHECK: iselect.cpp(42,25):  error: matrix iselect row and column index size must be the same, 4 != 6
// CHECK: iselect.cpp(44,25):  error: matrix iselect row and column index size must be the same, 4 != 6
// CHECK: iselect.cpp(45,57):  error: vector iselect expects 1 index expression
// CHECK: iselect.cpp(49,19):  error: expression is not assignable
// CHECK: iselect.cpp(50,24):  error: expression is not assignable
// CHECK: iselect.cpp(52,16):  error: no member named 'iselect' in 'S1'
// CHECK: iselect.cpp(53,16):  error: no member named 'iselect' in 'S1'
// CHECK: iselect.cpp(54,25):  error: 'iselect' following the 'template' keyword does not refer to a template
// CHECK: iselect.cpp(56,23):  error: called object type 'int' is not a function or function pointer
// CHECK: iselect.cpp(57,25):  error: 'iselect' following the 'template' keyword does not refer to a template
// CHECK: iselect.cpp(47,3):  warning: expression result unused [-Wunused-value]
// CHECK: iselect.cpp(48,3):  warning: expression result unused [-Wunused-value]
// CHECK: 2 warnings and 26 errors generated.
