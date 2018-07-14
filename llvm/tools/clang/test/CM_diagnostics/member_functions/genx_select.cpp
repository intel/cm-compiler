// RUN: %cmc -ferror-limit=99 %w 2>&1 | FileCheck %w

#include <cm/cm.h>

matrix<int,4,6> m;
vector<int,4> v;
struct S1 {
  int a;
} s1;
struct S2 {
  int genx_select;
} s2;

_GENX_MAIN_
void test() {
  vector_ref<int, 4> r1 = m.genx_select;                   // expected '<'
  vector_ref<int, 4> r2 = m.genx_select<;                  // expected expression
  vector_ref<int, 4> r3 = m.genx_select<4;                 // expected '>'
  vector_ref<int, 4> r4 = m.genx_select<4,1,1;             // expected '>'
  vector_ref<int, 4> r5 = m.genx_select<4,1,1,1;           // expected '>'
  vector_ref<int, 4> r6 = m.genx_select<4,1,1,1();         // not a function
  vector_ref<int, 4> r7 = m.genx_select<4,1,1,1>;          // expected '('
  vector_ref<int, 4> r8 = m.genx_select<9,8,7,6>(;         // expected expression
  vector_ref<int, 4> r9 = m.genx_select<4,1,1,1>();        // deprecated
  vector_ref<int, 4> r10 = m.genx_select<4,1,1,1>(3);      // deprecated
  vector_ref<int, 4> r11 = m.genx_select<4,1,1,1>(3,1);    // deprecated
  vector_ref<int, 4> r12 = m.genx_select();                // expected '<'

  vector_ref<int, 4> r13 = v.genx_select;                  // expected '<'
  vector_ref<int, 4> r14 = v.genx_select<;                 // expected expression
  vector_ref<int, 4> r15 = v.genx_select<4,1,1,1;          // expected '>'
  vector_ref<int, 4> r16 = v.genx_select<4;                // expected '>'
  vector_ref<int, 4> r17 = v.genx_select<4,1;              // expected '>'
  vector_ref<int, 4> r18 = v.genx_select<4,1();            // not a function
  vector_ref<int, 4> r19 = v.genx_select<4,1>;             // expected '('
  vector_ref<int, 4> r20 = v.genx_select<9,8>(;            // expected expression
  vector_ref<int, 4> r21 = v.genx_select<4,1>();           // deprecated
  vector_ref<int, 4> r22 = v.genx_select<4,1>(3);          // deprecated
  vector_ref<int, 4> r23 = v.genx_select<4,1>(3,1);        // deprecated
  vector_ref<int, 4> r24 = v.genx_select();                // expected '<'

  int r25 = s1.genx_select;                                // no member genx_select in s1
  int r26 = s1.genx_select();                              // no member genx_select in s1
  int r27 = s1.genx_select<4;                              // no member genx_select in s
  int r28 = s1.template genx_select;                       // genx_select not a template
  int r29 = s2.genx_select;                                // OK
  int r30 = s2.genx_select();                              // not a function
  int r31 = s2.genx_select<4;                              // OK
  int r32 = s2.template genx_select;                       // genx_select not a template

  m.genx_select<1,1,1,1>() = 9;                            // deprecated
  v.genx_select<1>() = 7;                                  // deprecated

}
// CHECK: genx_select.cpp(16,40):  error: expected '<'
// CHECK: genx_select.cpp(17,41):  error: expected expression
// CHECK: genx_select.cpp(18,42):  error: expected '>'
// CHECK: genx_select.cpp(19,46):  error: expected '>'
// CHECK: genx_select.cpp(20,48):  error: expected '>'
// CHECK: genx_select.cpp(21,48):  error: called object type 'int' is not a function or function pointer
// CHECK: genx_select.cpp(22,49):  error: expected '('
// CHECK: genx_select.cpp(23,50):  error: expected expression
// CHECK: genx_select.cpp(24,29):  error: genx_select() is deprecated - use replicate()
// CHECK: genx_select.cpp(25,30):  error: genx_select() is deprecated - use replicate()
// CHECK: genx_select.cpp(26,30):  error: genx_select() is deprecated - use replicate()
// CHECK: genx_select.cpp(27,41):  error: expected '<'
// CHECK: genx_select.cpp(29,41):  error: expected '<'
// CHECK: genx_select.cpp(30,42):  error: expected expression
// CHECK: genx_select.cpp(31,49):  error: expected '>'
// CHECK: genx_select.cpp(32,43):  error: expected '>'
// CHECK: genx_select.cpp(33,45):  error: expected '>'
// CHECK: genx_select.cpp(34,45):  error: called object type 'int' is not a function or function pointer
// CHECK: genx_select.cpp(35,46):  error: expected '('
// CHECK: genx_select.cpp(36,47):  error: expected expression
// CHECK: genx_select.cpp(37,30):  error: genx_select() is deprecated - use replicate()
// CHECK: genx_select.cpp(38,30):  error: genx_select() is deprecated - use replicate()
// CHECK: genx_select.cpp(39,30):  error: genx_select() is deprecated - use replicate()
// CHECK: genx_select.cpp(40,41):  error: expected '<'
// CHECK: genx_select.cpp(42,16):  error: no member named 'genx_select' in 'S1'
// CHECK: genx_select.cpp(43,16):  error: no member named 'genx_select' in 'S1'
// CHECK: genx_select.cpp(44,16):  error: no member named 'genx_select' in 'S1'
// CHECL: genx_select.cpp(45,25):  error: 'genx_select' following the 'template' keyword does not refer to a template
// CHECK: genx_select.cpp(47,27):  error: called object type 'int' is not a function or function pointer
// CHECK: genx_select.cpp(49,25):  error: 'genx_select' following the 'template' keyword does not refer to a template
// CHECK: 32 errors generated.
