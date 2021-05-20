#include <cm/cm.h>

_GENX_MAIN_ void foo(SurfaceIndex S)
{
   vector<int, 4> v = cm_get_hwid();
   write(S, 0, 0, v);
}


// RUN: %cmc -emit-llvm -march=BDW %s | FileCheck %s
//
// CHECK: -platform BDW
// CHECK-NOT: error
// CHECK-NOT: warning

// We check that the generated asm contains an AND to mask the
// 10 bits from the r0.5 register that contain the hwid.
//
// RUN: FileCheck -input-file=%W_0.asm -check-prefix=ASM %s
// XFAIL: *
//
// ASM: and
// ASM-SAME: r0.5<0;1,0>:ud
// ASM-SAME: 0x3ff:ud

// tidy up the generated files
