// RUN: %cmc -ferror-limit=99 %w 2>&1 | FileCheck %w

#include <cm/cm.h>

matrix<int,4,6> m1, m2, m3;
vector<int,8> v1, v2, v3;
matrix<float,4,6> m4;
vector<float,8> v4;
matrix<int,6,4> m5;
vector<int,24> v5;
matrix<int,6,6> m6, m7;
vector<ushort,32> v6, v7;
vector<SurfaceIndex,10> v8, v9;
int mask1;
char mask2;
float mask3;
SurfaceIndex mask4;
vector<int,6> mask5;
vector<int,24> mask6;
matrix<char,4,6> mask7;
matrix<char,6,4> mask8;
matrix<char,4,4> mask9;
matrix<float,4,6> mask10;
matrix<SurfaceIndex,4,6> mask11;
struct S1 {
  int a;
} s1;
struct S2 {
  int merge;
} s2;

_GENX_MAIN_
void test() {
  m1.merge;                                // expected '('
  m1.merge(;                               // expected expression
  m1.merge();                              // expected expression
  m1.merge(m2;                             // expected ')'
  m1.merge(m2);                            // not enough args
  m1.merge(m1,mask1);                      // OK
  m1.template merge(m1,mask1);             // OK
  m1.merge(m2,m3,mask1);                   // OK
  m1.template merge(m2,m3,mask1);          // OK
  m1.merge(m1,m2,m3,mask1);                // too many args
  m1.merge(m2,mask7).merge(m3,mask1);      // void not a structure
  m1.merge(m4,mask1);                      // OK - with type conversion
  m1.merge(m5,mask1);                      // OK - matrix different shape
  m1.merge(m6,mask1);                      // matrix size mismatch
  m1.merge(m6,m7,mask1);                   // matrix size mismatch
  m1.merge(m2,mask7);                      // OK
  m1.merge(m2,mask8);                      // OK
  m1.merge(m5,v5,mask6);                   // OK
  m1.merge(m2,mask4);                      // invalid mask type
  m1.merge(m2,mask10);                     // invalid mask type
  m1.merge(m2,mask11);                     // invalid mask type
  m1.merge(m5,v5,mask3);                   // invalid mask type
  m1.merge(m2,mask9);                      // mask size mismatch
  m1.merge(v5,mask1);                      // OK
  m1.merge(v5,mask2);                      // warning mask insufficient bits
  m1.merge(m2,mask6);                      // OK - mask different shape

  v1.merge;                                // expected '('
  v1.merge(;                               // expected expression
  v1.merge();                              // expected expression
  v1.merge(v2;                             // expected ')'
  v1.merge(v2);                            // not enough args
  v1.merge(v1,mask1);                      // OK
  v1.template merge(v1,mask1);             // OK
  v1.merge(v2,v3,mask1);                   // OK
  v1.template merge(v2,v3,mask1);          // OK
  v1.merge(v1,v2,v3,mask1);                // too many args
  v1.merge(v2,mask2).merge(v3,mask1);      // void not a structure
  v1.merge(v4,mask1);                      // OK - with type conversion
  v5.merge(m4,mask1);                      // OK - matrix different shape
  v1.merge(v6,mask1);                      // matrix size mismatch
  v1.merge(m7,v6,mask1);                   // matrix size mismatch
  v5.merge(m5,mask7);                      // OK
  v5.merge(m5,mask8);                      // OK
  v5.merge(m5,v5,mask6);                   // OK
  v1.merge(v2,v3,mask3);                   // invalid mask type float
  v1.merge(v2,mask9);                      // mask size mismatch
  v5.merge(m5,mask1);                      // OK
  v6.merge(v7,mask2);                      // warning mask insufficient bits
  v5.merge(m1,mask7);                      // OK - mask different shape
  v8.merge(v9,mask1);                      // invalid element type for merge

  s1.merge;                                // no member merge in s1
  s1.merge();                              // no member merge in s1
  s1.template merge;                       // merge not a template
  s2.merge = 7;                            // OK
  s2.merge();                              // merge not a function
  s2.template merge();                     // merge not a template

  m1.merge(m2,mask1) = 1;                  // not assignable
  v1.merge(v2,mask1) = 9;                  // not assignable
}
// CHECK: merge.cpp(34,11):  error: expected '('
// CHECK: merge.cpp(35,12):  error: expected expression
// CHECK: merge.cpp(36,12):  error: expected expression
// CHECK: merge.cpp(37,14):  error: expected ')'
// CHECK: merge.cpp(38,14):  error: too few arguments: merge() expects at least two arguments
// CHECK: merge.cpp(43,21):  error: too many arguments: merge() expects at most three arguments
// CHECK: merge.cpp(44,21):  error: member reference base type 'void' is not a structure or union
// CHECK: merge.cpp(47,3):  error: cannot convert to type 'matrix<int,4,6>' from type 'matrix<int,6,6>'
// CHECK: merge.cpp(48,3):  error: cannot convert to type 'matrix<int,4,6>' from type 'matrix<int,6,6>'
// CHECK: merge.cpp(52,15):  error: invalid merge mask type 'SurfaceIndex' for type 'matrix<int,4,6>'
// CHECK: merge.cpp(53,15):  error: invalid merge mask element type 'float'
// CHECK: merge.cpp(54,15):  error: invalid merge mask element type 'SurfaceIndex'
// CHECK: merge.cpp(55,18):  error: invalid merge mask type 'float' for type 'matrix<int,4,6>'
// CHECK: merge.cpp(56,15):  error: invalid merge mask type 'matrix<char,4,4>' for type 'matrix<int,4,6>'
// CHECK: merge.cpp(58,15):  warning: merge mask has fewer bits (8) than the number of merge elements (24)
// CHECK: merge.cpp(61,11):  error: expected '('
// CHECK: merge.cpp(62,12):  error: expected expression
// CHECK: merge.cpp(63,12):  error: expected expression
// CHECK: merge.cpp(64,14):  error: expected ')'
// CHECK: merge.cpp(65,14):  error: too few arguments: merge() expects at least two arguments
// CHECK: merge.cpp(70,21):  error: too many arguments: merge() expects at most three arguments
// CHECK: merge.cpp(71,21):  error: member reference base type 'void' is not a structure or union
// CHECK: merge.cpp(74,3):  error: cannot convert to type 'vector<int,8>' from type 'vector<unsigned short,32>'
// CHECK: merge.cpp(75,3):  error: cannot convert to type 'vector<int,8>' from type 'matrix<int,6,6>'
// CHECK: merge.cpp(79,18):  error: invalid merge mask type 'float' for type 'vector<int,8>'
// CHECK: merge.cpp(80,15):  error: invalid merge mask type 'matrix<char,4,4>' for type 'vector<int,8>'
// CHECK: merge.cpp(82,15):  warning: merge mask has fewer bits (8) than the number of merge elements (32)
// CHECK: merge.cpp(84,3):  error: invalid element type 'SurfaceIndex'
// CHECK: merge.cpp(86,6):  error: no member named 'merge' in 'S1'
// CHECK: merge.cpp(87,6):  error: no member named 'merge' in 'S1'
// CHECK: merge.cpp(88,15):  error: 'merge' following the 'template' keyword does not refer to a template
// CHECK: merge.cpp(90,11):  error: called object type 'int' is not a function or function pointer
// CHECK: merge.cpp(91,15):  error: 'merge' following the 'template' keyword does not refer to a template
// CHECK: merge.cpp(93,22):  error: expression is not assignable
// CHECK: merge.cpp(94,22):  error: expression is not assignable
// CHECK: 2 warnings and 33 errors generated.
