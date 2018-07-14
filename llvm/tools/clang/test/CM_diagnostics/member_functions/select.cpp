// RUN: %cmc -ferror-limit=99 %w 2>&1 | FileCheck %w

#include <cm/cm.h>

vector<int,16> v1;
vector<short,15> v2;
matrix<int,16,4> m1;
struct S1 {
  int a;
} s1;
struct S2 {
  int select;
} s2;
int i, j;
const int two = 2;
const int minus_two = -2;

_GENX_MAIN_
void test() {
  vector_ref<int,8> r1 = v1.select;                                       // expected '<'
  vector_ref<int,8> r2 = v1.select<;                                      // expected expression
  vector_ref<int,8> r3 = v1.select<>();                                   // expected expression
  vector_ref<int,8> r4 = v1.select<8;                                     // expected '>'
  vector_ref<int,8> r5 = v1.select<8,;                                    // expected expression
  vector_ref<int,8> r6 = v1.select<8,2;                                   // expected '>'
  vector_ref<int,8> r7 = v1.select<8,2>;                                  // expected '('
  vector_ref<int,8> r8 = v1.select<8,2>(;                                 // expected ')'
  vector_ref<int,8> r9 = v1.select<8,2>();                                // OK
  vector_ref<int,8> r10 = v1.select<8>();                                 // too few constant args
  vector_ref<int,8> r11 = v1.select<8,1,2,1>();                           // too many constant args
  vector_ref<int,8> r12 = v1.select<4+i,2>();                             // size not constant int
  vector_ref<int,8> r13 = v1.select<4,j>();                               // stride not constant int
  vector_ref<int,2> r14 = v1.select<two,2>();                             // OK
  vector_ref<int,4> r15 = v1.select<4,two>();                             // OK
  vector_ref<int,8> r16 = v1.select<0,2>();                               // size must be > 0
  vector_ref<int,8> r17 = v1.select<4,0>();                               // stride must be > 0
  vector_ref<int,8> r18 = v1.select<minus_two,2>();                       // size must be > 0
  vector_ref<int,8> r19 = v1.select<4,minus_two>();                       // stride must be > 0
  vector_ref<int,8> r20 = v1.select<8,2>(1);                              // OK
  vector_ref<int,8> r21 = v1.select<8,2>(,0);                             // expected expression
  vector_ref<int,8> r22 = v1.select<8,2>(1,2);                            // too many offsets
  vector_ref<int,4> r23 = v1.select<4,2>(2,3,1,2);                        // too many offsets
  vector_ref<int,1> r24 = v1.select<1,2>();                               // if size is 1 then stride must be 1
  vector_ref<int,8> r25 = v1.select<8,1>(minus_two);                      // negative offset
  vector_ref<int,17> r26 = v1.select<17,1>();                             // size too large
  vector_ref<int,8> r27 = v1.select<8,3>();                               // stride causes out of bounds
  vector_ref<int,8> r28 = v1.select<8,2>(2);                              // offset causes out of bounds
  vector_ref<int,8> r29 = v1.select<8,1>(14+two);                         // offset out of bounds

  matrix_ref<int,8,3> r41 = m1.select;                                    // expected '<'
  matrix_ref<int,8,3> r42 = m1.select<;                                   // expected expression
  matrix_ref<int,8,3> r43 = m1.select<>();                                // expected expression
  matrix_ref<int,8,3> r44 = m1.select<8;                                  // expected '>'
  matrix_ref<int,8,3> r45 = m1.select<8,;                                 // expected expression
  matrix_ref<int,8,3> r46 = m1.select<8,2;                                // expected '>'
  matrix_ref<int,8,3> r47 = m1.select<8,2,3;                              // expected '>'
  matrix_ref<int,8,3> r48 = m1.select<8,2,3,1;                            // expected '>'
  matrix_ref<int,8,3> r49 = m1.select<8,2,3,1>;                           // expected '('
  matrix_ref<int,8,3> r50 = m1.select<8,2,3,1>(;                          // expected ')'
  matrix_ref<int,8,3> r51 = m1.select<8>();                               // too few constant args
  matrix_ref<int,8,3> r52 = m1.select<8,2>();                             // too few constant args
  matrix_ref<int,8,3> r53 = m1.select<8,2,3>();                           // too few constant args
  matrix_ref<int,8,3> r54 = m1.select<8,1,3,1,2,1>();                     // too many constant args
  matrix_ref<int,8,3> r55 = m1.select<4+i,2,3,1>();                       // v_size not constant int
  matrix_ref<int,8,3> r56 = m1.select<4,j>();                             // v_stride not constant int
  matrix_ref<int,8,3> r57 = m1.select<two,2,3,1>();                       // OK
  matrix_ref<int,8,3> r58 = m1.select<8,two,3,1>();                       // OK
  matrix_ref<int,8,3> r59 = m1.select<8,2,two+1,1>();                     // OK
  matrix_ref<int,8,3> r60 = m1.select<8,1,3,two-1>();                     // OK
  matrix_ref<int,8,2> r61 = m1.select<0,2,3,1>();                         // v_size must be > 0
  matrix_ref<int,8,3> r62 = m1.select<4,0,3,1>();                         // v_stride must be > 0
  matrix_ref<int,8,2> r63 = m1.select<8,2,0,1>();                         // h_size must be > 0
  matrix_ref<int,8,3> r64 = m1.select<4,2,3,0>();                         // h_stride must be > 0
  matrix_ref<int,8,3> r65 = m1.select<minus_two,2,3,1>();                 // v_size must be > 0
  matrix_ref<int,8,3> r66 = m1.select<4,minus_two,3,1>();                 // v_stride must be > 0
  matrix_ref<int,8,3> r67 = m1.select<8,2,minus_two,1>();                 // h_size must be > 0
  matrix_ref<int,8,3> r68 = m1.select<8,2,3,minus_two>();                 // h_stride must be > 0
  matrix_ref<int,8,3> r69 = m1.select<8,2,3,1>(1);                        // OK
  matrix_ref<int,8,3> r70 = m1.select<8,2,3,1>(,0);                       // expected expression
  matrix_ref<int,8,3> r71 = m1.select<8,1,3,1>(3,1);                      // OK
  matrix_ref<int,4,3> r72 = m1.select<4,3,3,1>(2,3,1,2);                  // too many offsets
  matrix_ref<int,1,1> r73 = m1.select<1,2,3,1>();                         // if v_size is 1 then v_stride must be 1
  matrix_ref<int,1,1> r74 = m1.select<1,1,1,3>();                         // if h_size is 1 then h_stride must be 1
  matrix_ref<int,8,3> r75 = m1.select<8,1,3,1>(minus_two);                // negative row offset
  matrix_ref<int,8,3> r77 = m1.select<8,1,3,1>(0,1+minus_two);            // negative column offset
  matrix_ref<int,17,3> r78 = m1.select<17,1,3,1>();                       // row size too large
  matrix_ref<int,8,5> r79 = m1.select<8,1,5,1>();                         // column size too large
  matrix_ref<int,8,3> r80 = m1.select<8,3,3,1>();                         // v_stride causes out of bounds
  matrix_ref<int,8,3> r81 = m1.select<8,1,3,3>();                         // h_stride causes out of bounds
  matrix_ref<int,8,3> r82 = m1.select<8,2,3,1>(2);                        // row offset causes out of bounds
  matrix_ref<int,8,3> r83 = m1.select<8,2,3,3>(0,2);                      // column offset causes out of bounds
  matrix_ref<int,8,3> r84 = m1.select<8,1,3,1>(15+two,0);                 // row offset out of bounds
  matrix_ref<int,8,3> r85 = m1.select<8,1,3,1>(0,3+two);                  // column offset out of bounds

  int r101 = s1.select;                                                   // no member select in s1
  int r102 = s1.select();                                                 // no member select in s1
  int r103 = s1.template select;                                          // select not a template
  int r104 = s2.select;                                                   // OK
  int r105 = s2.select<4;                                                 // OK
  int r106 = s2.select();                                                 // select not a function
  int r107 = s2.template select();                                        // select not a template

  v1.select<8,2>();                                                       // expression result unused
  m1.select<4,2,2,1>();                                                   // expression result unused,
  v2.select<4,1>() = 9;                                                   // OK
  m1.select<3,2,2,1>() = 1;                                               // OK
}
// CHECK: select.cpp(20,35):  error: expected '<'
// CHECK: select.cpp(21,36):  error: expected expression
// CHECK: select.cpp(22,36):  error: expected expression
// CHECK: select.cpp(23,37):  error: expected '>'
// CHECK: select.cpp(24,38):  error: expected expression
// CHECK: select.cpp(25,39):  error: expected '>'
// CHECK: select.cpp(26,40):  error: expected '('
// CHECK: select.cpp(27,41):  error: expected expression
// CHECK: select.cpp(29,37):  error: too few values: vector select expects 2 integer constant values
// CHECK: select.cpp(30,41):  error: too many values: vector select expects 2 integer constant values
// CHECK: select.cpp(31,39):  error: select size value must be a constant integer expression
// CHECK: select.cpp(32,39):  error: select stride value must be a constant integer expression
// CHECK: select.cpp(35,37):  error: select size must be greater than zero
// CHECK: select.cpp(36,39):  error: select stride must be greater than zero
// CHECK: select.cpp(37,37):  error: select size must be greater than zero
// CHECK: select.cpp(38,39):  error: select stride must be greater than zero
// CHECK: select.cpp(40,42):  error: expected expression
// CHECK: select.cpp(41,44):  error: too many offsets: vector select expects 1 integer offsets
// CHECK: select.cpp(42,44):  error: too many offsets: vector select expects 1 integer offsets
// CHECK: select.cpp(43,39):  error: when select size is 1, the stride must also be 1
// CHECK: select.cpp(44,42):  error: select offset cannot be negative (-2)
// CHECK: select.cpp(45,31):  warning: vector select out of bounds [-Wcm-bounds-check]
// CHECK: select.cpp(46,30):  warning: vector select out of bounds [-Wcm-bounds-check]
// CHECK: select.cpp(47,30):  warning: vector select out of bounds [-Wcm-bounds-check]
// CHECK: select.cpp(48,30):  warning: select offset out of bounds (offset 16, bounds 0..15) [-Wcm-bounds-check]
// CHECK: select.cpp(48,30):  warning: vector select out of bounds  [-Wcm-bounds-check]
// CHECK: select.cpp(50,38):  error: expected '<'
// CHECK: select.cpp(51,39):  error: expected expression
// CHECK: select.cpp(52,39):  error: expected expression
// CHECK: select.cpp(53,40):  error: expected '>'
// CHECK: select.cpp(54,41):  error: expected expression
// CHECK: select.cpp(55,42):  error: expected '>'
// CHECK: select.cpp(56,44):  error: expected '>'
// CHECK: select.cpp(57,46):  error: expected '>'
// CHECK: select.cpp(58,47):  error: expected '('
// CHECK: select.cpp(59,48):  error: expected expression
// CHECK: select.cpp(60,39):  error: too few values: matrix select expects 4 integer constant values
// CHECK: select.cpp(61,41):  error: too few values: matrix select expects 4 integer constant values
// CHECK: select.cpp(62,43):  error: too few values: matrix select expects 4 integer constant values
// CHECK: select.cpp(63,47):  error: too many values: matrix select expects 4 integer constant values
// CHECK: select.cpp(64,41):  error: select size value must be a constant integer expression
// CHECK: select.cpp(65,41):  error: too few values: matrix select expects 4 integer constant values
// CHECK: select.cpp(66,23):  error: cannot initialize a variable of type 'matrix_ref<int,8,3>' with an lvalue of type 'matrix_ref<int,2,3>'
// CHECK: select.cpp(70,39):  error: select v_size must be greater than zero
// CHECK: select.cpp(71,41):  error: select v_stride must be greater than zero
// CHECK: select.cpp(72,43):  error: select h_size must be greater than zero
// CHECK: select.cpp(73,45):  error: select h_stride must be greater than zero
// CHECK: select.cpp(74,39):  error: select v_size must be greater than zero
// CHECK: select.cpp(75,41):  error: select v_stride must be greater than zero
// CHECK: select.cpp(76,43):  error: select h_size must be greater than zero
// CHECK: select.cpp(77,45):  error: select h_stride must be greater than zero
// CHECK: select.cpp(79,48):  error: expected expression
// CHECK: select.cpp(81,52):  error: too many offsets: matrix select expects 2 integer offsets
// CHECK: select.cpp(82,41):  error: when select v_size is 1, the v_stride must also be 1
// CHECK: select.cpp(83,45):  error: when select h_size is 1, the h_stride must also be 1
// CHECK: select.cpp(84,48):  error: select row offset cannot be negative (-2)
// CHECK: select.cpp(85,51):  error: select column offset cannot be negative (-1)
// CHECK: select.cpp(86,33):  warning: matrix select out of bounds in rows [-Wcm-bounds-check]
// CHECK: select.cpp(87,32):  warning: matrix select out of bounds in columns [-Wcm-bounds-check]
// CHECK: select.cpp(88,32):  warning: matrix select out of bounds in rows [-Wcm-bounds-check]
// CHECK: select.cpp(89,32):  warning: matrix select out of bounds in columns [-Wcm-bounds-check]
// CHECK: select.cpp(90,32):  warning: matrix select out of bounds in rows [-Wcm-bounds-check]
// CHECK: select.cpp(91,32):  warning: matrix select out of bounds in columns [-Wcm-bounds-check]
// CHECK: select.cpp(92,32):  warning: select row offset out of bounds (offset 17, bounds 0..15) [-Wcm-bounds-check]
// CHECK: select.cpp(93,32):  warning: select column offset out of bounds (offset 5, bounds 0..3) [-Wcm-bounds-check]
// CHECK: select.cpp(95,17):  error: no member named 'select' in 'S1'
// CHECK: select.cpp(96,17):  error: no member named 'select' in 'S1'
// CHECK: select.cpp(97,26):  error: 'select' following the 'template' keyword does not refer to a template
// CHECK: select.cpp(100,23):  error: called object type 'int' is not a function or function pointer
// CHECK: select.cpp(101,26):  error: 'select' following the 'template' keyword does not refer to a template
// CHECK: select.cpp(103,3):  warning: expression result unused [-Wunused-value]
// CHECK: select.cpp(104,3):  warning: expression result unused [-Wunused-value]
// CHECK: 17 warnings and 57 errors generated.
