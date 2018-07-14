// RUN: %cmc -ferror-limit=999 %w 2>&1 | FileCheck %w

#include <cm/cm.h>

matrix<int,4,6> m;
vector<int,4> v1;
vector<short,15> v2;
struct S1 {
  int a;
} s1;
struct S2 {
  int replicate;
} s2;
int i;
const int two = 2;
const int minus_two = -2;

_GENX_MAIN_
void test() {
  vector<int,8> r1 = v1.replicate;                                       // expected '<'
  vector<int,8> r2 = v1.replicate<;                                      // expected expression
  vector<int,8> r3 = v1.replicate<2;                                     // expected '>'
  vector<int,8> r4 = v1.replicate<2>;                                    // expected '('
  vector<int,8> r5 = v1.replicate<2>(;                                   // expected expression
  vector<int,8> r6 = v1.replicate<2>();                                  // OK
  vector<int,8> r7 = v1.replicate<two>();                                // OK
  vector<int,8> r8 = v1.replicate<0>();                                  // zero REP
  vector<int,8> r9 = v1.replicate<two + minus_two>();                    // zero REP
  vector<int,8> r10 = v1.replicate<minus_two>();                         // negative REP
  vector<int,8> r11 = v1.replicate<2,>();                                // expected expression
  vector<int,8> r12 = v1.replicate<2,4>();                               // OK
  vector<int,8> r13 = v1.replicate<2,0>();                               // zero Width
  vector<int,8> r14 = v1.replicate<2,minus_two>();                       // negative Width
  vector<int,8> r15 = v1.replicate<2,4,>();                              // expected expression
  vector<int,4> r16 = v1.replicate<2,2,2>();                             // OK
  vector<int,8> r17 = v2.replicate<4,0,2>();                             // OK
  vector<int,8> r18 = v1.replicate<4,2,0>();                             // zero Width
  vector<int,8> r19 = v1.replicate<4,-8,2>();                            // negative VS
  vector<int,8> r20 = v1.replicate<4,2,-6>();                            // negative Width
  vector<int,8> r21 = v1.replicate<4,2,4,>();                            // expected expression
  vector<int,16> r22 = v1.replicate<4,0,4,0>();                          // OK
  vector<int,16> r23 = v1.replicate<4,2,0,2>();                          // zero width
  vector<int,16> r24 = v1.replicate<4,2,4,2,two>();                      // too many args
  vector<int,16> r25 = v1.replicate<4,2,4,2,2,3>();                      // too many args
  vector<int,16> r26 = v2.replicate<4,1+minus_two,4,2>();                // negative HS
  vector<int,16> r27 = v1.replicate<4,2,minus_two,2>();                  // negative Width
  vector<int,16> r28 = v1.replicate<4,2,4,minus_two-1>();                // negative VS
  vector<int,8> r29 = v1.replicate<4>(two+1);                            // offset not applicable
  vector<int,8> r30 = v1.replicate<4,2>(2);                              // OK
  vector<int,8> r31 = v1.template replicate<4,2>(i);                     // OK
  vector<int,8> r32 = v2.replicate<2,2>(2,1-two);                        // too many offsets
  vector<int,8> r33 = v1.replicate<4,2>(minus_two);                      // negative offset
  vector<int,8> r34 = v1.replicate<i>();                                 // REP not integer const
  vector<int,8> r35 = v1.replicate<2,i>();                               // Width not integer const
  vector<int,8> r36 = v1.replicate<2,i,4>();                             // VS not integer const
  vector<int,8> r37 = v1.replicate<2,2,i>();                             // Width not integer const
  vector<int,8> r38 = v1.replicate<2,2,i+two,2>();                       // Width not integer const
  vector<int,8> r39 = v1.replicate<2,2,4,(5*i)>();                       // HS not integer const
  vector<int,8> r40 = m.replicate;                                       // expected '<'
  vector<int,8> r41 = m.replicate<;                                      // expected expression
  vector<int,8> r42 = m.replicate<2;                                     // expected '>'
  vector<int,8> r43 = m.replicate<2>;                                    // expected '('
  vector<int,8> r44 = m.replicate<2>(;                                   // expected expression
  vector<int,48> r45 = m.template replicate<2>();                        // OK
  vector<int,48> r46 = m.replicate<two>();                               // OK
  vector<int,8> r47 = m.replicate<0>();                                  // zero REP
  vector<int,8> r48 = m.replicate<two + minus_two>();                    // zero REP
  vector<int,8> r59 = m.replicate<minus_two>();                          // negative REP
  vector<int,8> r60 = m.replicate<2,>();                                 // expected expression
  vector<int,8> r61 = m.replicate<2,4>();                                // OK
  vector<int,8> r62 = m.replicate<2,0>();                                // zero Width
  vector<int,8> r63 = m.replicate<2,minus_two>();                        // negative Width
  vector<int,8> r64 = m.replicate<2,4,>();                               // expected expression
  vector<int,8> r65 = m.replicate<4,2,2>();                              // OK
  vector<int,8> r66 = m.replicate<4,0,2>();                              // OK
  vector<int,8> r67 = m.replicate<4,2,0>();                              // zero Width
  vector<int,8> r68 = m.replicate<4,-8,2>();                             // negative VS
  vector<int,8> r69 = m.replicate<4,2,-6>();                             // negative Width
  vector<int,8> r70 = m.replicate<4,2,4,>();                             // expected expression
  vector<int,16> r71 = m.replicate<4,0,4,0>();                           // OK
  vector<int,16> r72 = m.replicate<4,2,0,2>();                           // zero width
  vector<int,16> r73 = m.replicate<4,2,4,2,two>();                       // too many args
  vector<int,16> r74 = m.replicate<4,2,4,2,2,3>();                       // too many args
  vector<int,16> r75 = m.replicate<4,1+minus_two,4,2>();                 // negative HS
  vector<int,16> r76 = m.replicate<4,2,minus_two,2>();                   // negative Width
  vector<int,16> r77 = m.replicate<4,2,4,minus_two-1>();                 // negative VS
  vector<int,8> r78 = m.replicate<4>(two+1);                             // offset not applicable
  vector<int,8> r79 = m.replicate<4,2>(2);                               // OK
  vector<int,8> r80 = m.template replicate<4,2>(i,two-i);                // OK
  vector<int,8> r81 = m.replicate<2,2>(2,1-two,5*i,0);                   // too many offsets
  vector<int,8> r82 = m.replicate<4,2>(minus_two);                       // negative offset
  vector<int,8> r83 = m.replicate<4,2>(0,minus_two);                     // negative offset
  vector<int,8> r84 = m.replicate<4.0f>();                               // REP not integer const
  vector<int,8> r85 = m.replicate<2,4.0f>();                             // Width not integer const
  vector<int,8> r86 = m.replicate<4,2>(1.0f);                            // offset not integer const
  vector<int,8> r87 = m.replicate<4,2>(0,0.0f);                          // offset not integer const

  vector<int,24> r88 = v1.replicate<2>().template replicate<3>();        // OK
  vector<short,8> r89 = m.replicate<4,2>(5).replicate<2,2,4,1>(4);       // OK

  int r90 = s1.replicate;                                                // no member replicate in s1 
  int r91 = s1.replicate();                                              // no member replicate in s1 
  int r92 = s1.template replicate;                                       // replicate not a template
  int r93 = s2.replicate;                                                // OK
  int r94 = s2.replicate<4;                                              // OK
  int r95 = s2.replicate();                                              // replicate not a function
  int r96 = s2.template replicate();                                     // replicate not a template

  vector<int,96> r100 = m.replicate<4,24>();                             // OK
  vector<int,96> r101 = m.replicate<4,24>(1);                            // out of bounds
  vector<int,96> r102 = m.replicate<4,24>(0,1);                          // out of bounds
  vector<int,24> r103 = m.replicate<1,24>(1,0);                          // out of bounds
  vector<int,24> r104 = m.replicate<1,24>(0,1);                          // out of bounds
  vector<int,6>  r105 = m.replicate<1,6>(3,0);                           // OK
  vector<int,6>  r106 = m.replicate<1,6>(3,1);                           // out of bounds
  vector<int,24> r107 = m.replicate<4,5,6>(0,1);                         // OK
  vector<int,24> r108 = m.replicate<4,7,6>();                            // out of bounds
  vector<int,24> r109 = m.replicate<4,5,6>(0,minus_two);                 // negative offset

  vector<int,24> r110 = v1.replicate<6,4>();                             // OK
  vector<int,4>  r111 = v1.replicate<1,4>(1);                            // out of bounds
  vector<int,4>  r112 = v1.replicate<2,2>(2);                            // OK
  vector<int,4>  r113 = v1.replicate<2,2,2>(2);                          // out of bounds
  vector<int,4>  r114 = v1.replicate<2,2>(-3);                           // negative offset

  vector<int,16> r120 = v1.replicate<4,1,4,0>();                         // OK
  vector<int,16> r121 = v1.replicate<4,1,4,0>(1);                        // out of bounds
  vector<int,16> r123 = v1.replicate<4,0,4,1>(0);                        // OK
  vector<int,16> r124 = v1.replicate<4,0,4,1>(1);                        // out of bounds
  vector<int,16> r125 = v1.replicate<4,1,4,1>(0);                        // out of bounds

  v1.replicate<16>();                                                    // expression result unused
  m.replicate<2,2>();                                                    // expression result unused
  v2.replicate<2,4,3,2>() = 9;                                           // expression not assignable
  m.replicate<2>() = 1;                                                  // expression not assignable
}

// CHECK: replicate.cpp(20,34):  error: expected '<'
// CHECK: replicate.cpp(21,35):  error: expected expression
// CHECK: replicate.cpp(22,36):  error: expected '>'
// CHECK: replicate.cpp(23,37):  error: expected '('
// CHECK: replicate.cpp(24,38):  error: expected expression
// CHECK: replicate.cpp(27,35):  error: replicate REP value cannot be zero
// CHECK: replicate.cpp(28,39):  error: replicate REP value cannot be zero
// CHECK: replicate.cpp(29,36):  error: replicate REP value cannot be negative (-2)
// CHECK: replicate.cpp(30,38):  error: expected expression
// CHECK: replicate.cpp(32,38):  error: replicate width value cannot be zero
// CHECK: replicate.cpp(33,38):  error: replicate width value cannot be negative (-2)
// CHECK: replicate.cpp(34,40):  error: expected expression
// CHECK: replicate.cpp(37,40):  error: replicate width value cannot be zero
// CHECK: replicate.cpp(38,38):  error: replicate vertical stride value cannot be negative (-8)
// CHECK: replicate.cpp(39,40):  error: replicate width value cannot be negative (-6)
// CHECK: replicate.cpp(40,42):  error: expected expression
// CHECK: replicate.cpp(42,41):  error: replicate width value cannot be zero
// CHECK: replicate.cpp(43,45):  error: too many arguments: replicate expects at most 4 constant integer values
// CHECK: replicate.cpp(44,45):  error: too many arguments: replicate expects at most 4 constant integer values
// CHECK: replicate.cpp(45,40):  error: replicate vertical stride value cannot be negative (-1)
// CHECK: replicate.cpp(46,41):  error: replicate width value cannot be negative (-2)
// CHECK: replicate.cpp(47,52):  error: replicate horizontal stride value cannot be negative (-3)
// CHECK: replicate.cpp(48,42):  error: replicate does not accept an offset when only the REP argument is specified
// CHECK: replicate.cpp(51,44):  error: too many offsets: replicate of a vector expects at most 1 integer offset
// CHECK: replicate.cpp(52,41):  error: replicate offset cannot be negative (-2)
// CHECK: replicate.cpp(53,36):  error: replicate value must be a constant integer expression
// CHECK: replicate.cpp(54,38):  error: replicate value must be a constant integer expression
// CHECK: replicate.cpp(55,38):  error: replicate value must be a constant integer expression
// CHECK: replicate.cpp(56,40):  error: replicate value must be a constant integer expression
// CHECK: replicate.cpp(57,40):  error: replicate value must be a constant integer expression
// CHECK: replicate.cpp(58,45):  error: replicate value must be a constant integer expression
// CHECK: replicate.cpp(59,34):  error: expected '<'
// CHECK: replicate.cpp(60,35):  error: expected expression
// CHECK: replicate.cpp(61,36):  error: expected '>'
// CHECK: replicate.cpp(62,37):  error: expected '('
// CHECK: replicate.cpp(63,38):  error: expected expression
// CHECK: replicate.cpp(66,35):  error: replicate REP value cannot be zero
// CHECK: replicate.cpp(67,39):  error: replicate REP value cannot be zero
// CHECK: replicate.cpp(68,35):  error: replicate REP value cannot be negative (-2)
// CHECK: replicate.cpp(69,37):  error: expected expression
// CHECK: replicate.cpp(71,37):  error: replicate width value cannot be zero
// CHECK: replicate.cpp(72,37):  error: replicate width value cannot be negative (-2)
// CHECK: replicate.cpp(73,39):  error: expected expression
// CHECK: replicate.cpp(76,39):  error: replicate width value cannot be zero
// CHECK: replicate.cpp(77,37):  error: replicate vertical stride value cannot be negative (-8)
// CHECK: replicate.cpp(78,39):  error: replicate width value cannot be negative (-6)
// CHECK: replicate.cpp(79,41):  error: expected expression
// CHECK: replicate.cpp(81,40):  error: replicate width value cannot be zero
// CHECK: replicate.cpp(82,44):  error: too many arguments: replicate expects at most 4 constant integer values
// CHECK: replicate.cpp(83,44):  error: too many arguments: replicate expects at most 4 constant integer values
// CHECK: replicate.cpp(84,39):  error: replicate vertical stride value cannot be negative (-1)
// CHECK: replicate.cpp(85,40):  error: replicate width value cannot be negative (-2)
// CHECK: replicate.cpp(86,51):  error: replicate horizontal stride value cannot be negative (-3)
// CHECK: replicate.cpp(87,41):  error: replicate does not accept an offset when only the REP argument is specified
// CHECK: replicate.cpp(90,49):  error: too many offsets: replicate of a matrix expects at most 2 integer offsets
// CHECK: replicate.cpp(91,40):  error: replicate offset cannot be negative (-2)
// CHECK: replicate.cpp(92,42):  error: replicate offset cannot be negative (-2)
// CHECK: replicate.cpp(93,35):  error: replicate value must be a constant integer expression
// CHECK: replicate.cpp(94,37):  error: replicate value must be a constant integer expression
// CHECK: replicate.cpp(95,40):  error: replicate offset must be an integer expression
// CHECK: replicate.cpp(96,42):  error: replicate offset must be an integer expression
// CHECK: replicate.cpp(99,45):  warning: replicate out of bounds - size of source vector exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(101,16):  error: no member named 'replicate' in 'S1'
// CHECK: replicate.cpp(102,16):  error: no member named 'replicate' in 'S1'
// CHECK: replicate.cpp(103,25):  error: 'replicate' following the 'template' keyword does not refer to a template
// CHECK: replicate.cpp(106,25):  error: called object type 'int' is not a function or function pointer
// CHECK: replicate.cpp(107,25):  error: 'replicate' following the 'template' keyword does not refer to a template
// CHECK: replicate.cpp(110,27):  warning: replicate out of bounds - size of source matrix exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(111,27):  warning: replicate out of bounds - size of source matrix exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(112,27):  warning: replicate out of bounds - size of source matrix exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(113,27):  warning: replicate out of bounds - size of source matrix exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(115,27):  warning: replicate out of bounds - size of source matrix exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(117,27):  warning: replicate out of bounds - size of source matrix exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(118,46):  error: replicate offset cannot be negative (-2)
// CHECK: replicate.cpp(121,28):  warning: replicate out of bounds - size of source vector exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(123,28):  warning: replicate out of bounds - size of source vector exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(124,43):  error: replicate offset cannot be negative (-3)
// CHECK: replicate.cpp(127,28):  warning: replicate out of bounds - size of source vector exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(129,28):  warning: replicate out of bounds - size of source vector exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(130,28):  warning: replicate out of bounds - size of source vector exceeded [-Wcm-bounds-check]
// CHECK: replicate.cpp(134,27):  error: expression is not assignable
// CHECK: replicate.cpp(135,20):  error: expression is not assignable
// CHECK: replicate.cpp(132,3):  warning: expression result unused [-Wunused-value]
// CHECK: replicate.cpp(133,3):  warning: expression result unused [-Wunused-value]
// CHECK: 14 warnings and 70 errors generated.
