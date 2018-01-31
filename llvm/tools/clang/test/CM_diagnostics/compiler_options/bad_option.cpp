// RUN: %cmc -bad-option %w 2>&1 | FileCheck %w

#include <cm/cm.h>

_GENX_MAIN_
void test() {
}

// CHECK: cmc.exe: error: unsupported option '-b ad-option'
