// XFAIL: *

// RUN: %cmc -emit-llvm -bad-option %s 2>&1 | FileCheck %s

#include <cm/cm.h>

_GENX_MAIN_
void test() {
}

// CHECK: cmc: error: unsupported option '-b ad-option'
