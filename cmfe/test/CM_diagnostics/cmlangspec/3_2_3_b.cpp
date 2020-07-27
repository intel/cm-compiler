#include <cm/cm.h>
#include <cm/cmtl.h>

_GENX_ void test1()
{
  cm_matrix(m, ushort, 4, 8, 10, 5);
  // ...
}

// RUN: %cmc -mCM_old_asm_name %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
