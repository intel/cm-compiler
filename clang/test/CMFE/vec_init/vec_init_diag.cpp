/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=SKL -emit-llvm -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

#include <cm/cm.h>

_GENX_ void test_func() {
  vector<unsigned short, 4> v1 = {};                    // OK
  vector<unsigned short, 4> v2 = {1, 2};                // OK
  vector<unsigned short, 4> v3 = {1, 2, 3, 5};          // OK
  vector<unsigned short, 4> v4 = {1, 2, 3, 5, 7, 11};   // error: excess elements // expected-error{{excess elements in CM vector initializer}}
  vector<unsigned short, 4> v5 = {{1}};                 // warning: braces around scalar // expected-warning{{braces around scalar initializer}}
  vector<unsigned short, 4> v6 = {{1}, 2, 3, 5, 7, 11}; // warning: braces around scalar; error: excess elements // expected-warning{{braces around scalar initializer}} // expected-error{{excess elements in CM vector initializer}}
  vector<unsigned short, 4> v7 = {(void*) 1, 2, 3, 4};   // error: initializing element with void* // expected-error{{cannot initialize a CM vector element of type 'unsigned short' with an rvalue of type 'void *'}}
}
