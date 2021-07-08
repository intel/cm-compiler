// XFAIL: *

// RUN: not %cmc -emit-llvm -- %s 2>&1 | FileCheck %s

#include <cm/cm.h>

_GENX_ void test_func() {
  vector<unsigned short, 4> v1 = {};                    // OK
  vector<unsigned short, 4> v2 = {1, 2};                // OK
  vector<unsigned short, 4> v3 = {1, 2, 3, 5};          // OK
  vector<unsigned short, 4> v4 = {1, 2, 3, 5, 7, 11};   // error: excess elements
  vector<unsigned short, 4> v5 = {{1}};                 // warning: braces around scalar
  vector<unsigned short, 4> v6 = {{1}, 2, 3, 5, 7, 11}; // warning: braces around scalar; error: excess elements
  vector<unsigned short, 4> v7 = {(void*) 1, 2, 3, 4};   // error: initializing element with void*
// CHECK: vec_init_diag.cpp:9:47: error: excess elements in CM vector initializer
// CHECK: vec_init_diag.cpp:10:35: warning: braces around scalar initializer
// CHECK: vec_init_diag.cpp:11:35: warning: braces around scalar initializer
// CHECK: vec_init_diag.cpp:11:49: error: excess elements in CM vector initializer
// CHECK: vec_init_diag.cpp:12:35: error: cannot initialize a CM vector element of type 'unsigned short' with an rvalue of type 'void *'
}
