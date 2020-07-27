// RUN: %cmc -mCM_old_asm_name -mdump_asm -bad-option %w 2>&1 | FileCheck %w

#include <cm/cm.h>

_GENX_MAIN_
void test() {
}

// CHECK: cmc: error: unsupported option '-b ad-option'
