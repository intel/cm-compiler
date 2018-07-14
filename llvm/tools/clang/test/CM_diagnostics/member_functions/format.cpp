// RUN: %cmc -ferror-limit=99 %w 2>&1 | FileCheck %w

#include <cm/cm.h>

matrix<int,4,2> m;
vector<int,8> v1;
vector<short,15> v2;
struct S1 {
  int a;
} s1;
struct S2 {
  int format;
} s2;
int i;
const int two = 2;
const int minus_two = -2;

_GENX_MAIN_
void test() {
  vector_ref<int,8> r1 = v1.format;                                       // expected '<'
  vector_ref<int,8> r2 = v1.format<;                                      // expected a type
  vector_ref<int,8> r3 = v1.format<1;                                     // expected a type
  vector_ref<int,8> r4 = v1.format<int;                                   // expected '>'
  vector_ref<int,8> r5 = v1.format<int, int>();                           // incomplete cast
  matrix_ref<int,4,2> r6 = v1.format<int, int(4), 2>();                   // OK
  vector_ref<int,8> r7 = v1.format<int>(5);                               // unexpected expression
  vector_ref<int,8> r8 = v1.format<int>();                                // OK
  vector_ref<int,4> r9 = v1.format<int,4>();                              // expected 0 or 2 dimensions
  vector_ref<int,4> r10 = v1.format<int,4,1,2,3>();                       // expected no more than 2 dimensions
  matrix_ref<int,4,2> r11 = v1.format<int,4,2>();                         // OK
  matrix_ref<int,8,2> r12 = v1.format<int,8,2>();                         // result too large
  matrix_ref<int,8,2> r13 = v1.format<int,5+i,2>();                       // row not a constant
  matrix_ref<int,8,2> r14 = v1.format<int,8,two>();                       // result too large
  matrix_ref<int,8,2> r15 = v1.format<int,4,i>();                         // column not a constant
  matrix_ref<int,8,2> r16 = v1.format<int,0,2>();                         // zero row
  matrix_ref<int,8,2> r17 = v1.format<int,4,0>();                         // zero column
  matrix_ref<int,8,2> r18 = v1.format<int,-1,2>();                        // negative row
  matrix_ref<int,8,2> r19 = v1.format<int,4,-2>();                        // negative column
  matrix_ref<int,8,2> r20 = v1.format<int,4,-two>();                      // negative column
  matrix_ref<int,8,2> r21 = v1.format<int,4,minus_two>();                 // negative column
  vector_ref<int,7> r22 = v2.format<int>();                               // OK
  matrix_ref<short,8,2> r23 = v1.format<short,8,2>();                     // OK
  vector_ref<short,16> r24 = v1.template format<short>();                 // OK
  vector_ref<int,8> r25 = v1.format<char>().format<int>();                // OK
  matrix_ref<short, 2, 2> r26 = m.format<char,4,2>().format<short,2,2>(); // result too small

  int r27 = s1.format;                                                    // no member format in s1
  int r28 = s1.format();                                                  // no member format in s1
  int r29 = s1.template format;                                           // format not a template
  int r30 = s2.format;                                                    // OK
  int r31 = s2.format<4;                                                  // OK
  int r32 = s2.format();                                                  // format not a function
  int r33 = s2.template format();                                         // format not a template

  matrix_ref<short,3,3> r40 = m.format<short,3,3>();                      // result too small
  matrix_ref<char,1,31> r41 = m.format<char,1,31>();                      // result too small
  matrix_ref<short,1,15> r42 = v1.format<short,1,15>();                   // result too small
  matrix_ref<char,1,31> r43 = v1.format<char,1,31>();                     // result too small

  m.format<char>();                                                       // expression result unused
  v1.format<char,16,2>();                                                 // expression result unused
  m.format<short>() = 1;                                                  // OK
  v2.format<int>() = 9;                                                   // OK
}
// CHECK: format.cpp(20,35):  error: expected '<'
// CHECK: format.cpp(21,36):  error: expected a type
// CHECK: format.cpp(22,36):  error: expected a type
// CHECK: format.cpp(23,39):  error: expected '>'
// CHECK: format.cpp(24,44):  error: expected '(' for function-style cast or type construction
// CHECK: format.cpp(26,41):  error: expected ')'
// CHECK: format.cpp(28,41):  error: format<>() expects 0 or 2 dimensions
// CHECK: format.cpp(29,45):  error: format<>() expects no more than 2 dimensions
// CHECK: format.cpp(31,29):  error: format matrix size 64 does not match the source size 32 in bytes
// CHECK: format.cpp(32,45):  error: format rows value must be a constant integer expression
// CHECK: format.cpp(33,29):  error: format matrix size 64 does not match the source size 32 in bytes
// CHECK: format.cpp(34,45):  error: format columns value must be a constant integer expression
// CHECK: format.cpp(35,43):  error: format rows value must be greater than 0
// CHECK: format.cpp(36,45):  error: format columns value must be greater than 0
// CHECK: format.cpp(37,43):  error: format rows value must be greater than 0
// CHECK: format.cpp(38,45):  error: format columns value must be greater than 0
// CHECK: format.cpp(39,45):  error: format columns value must be greater than 0
// CHECK: format.cpp(40,45):  error: format columns value must be greater than 0
// CHECK: format.cpp(45,33):  error: format matrix size 8 does not match the source size 32 in bytes
// CHECK: format.cpp(45,66):  error: expected '(' for function-style cast or type construction
// CHECK: format.cpp(45,67):  error: expected unqualified-id
// CHECK: format.cpp(45,67):  error: expected ';' at end of declaration
// CHECK: format.cpp(47,16):  error: no member named 'format' in 'S1'
// CHECK: format.cpp(48,16):  error: no member named 'format' in 'S1'
// CHECK: format.cpp(49,25):  error: 'format' following the 'template' keyword does not refer to a template
// CHECK: format.cpp(52,22):  error: called object type 'int' is not a function or function pointer
// CHECK: format.cpp(53,25):  error: 'format' following the 'template' keyword does not refer to a template
// CHECK: format.cpp(55,31):  error: format matrix size 18 does not match the source size 32 in bytes
// CHECK: format.cpp(56,31):  error: format matrix size 31 does not match the source size 32 in bytes
// CHECK: format.cpp(57,32):  error: format matrix size 30 does not match the source size 32 in bytes
// CHECK: format.cpp(58,31):  error: format matrix size 31 does not match the source size 32 in bytes
// CHECK: format.cpp(60,3):  warning: expression result unused [-Wunused-value]
// CHECK: format.cpp(61,3):  warning: expression result unused [-Wunused-value]
// CHECK: 2 warnings and 31 errors generated.
