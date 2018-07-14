#include <cm/cm.h>

_GENX_MAIN_ void foo(SurfaceIndex S)
{
   vector<int, 4> v = cm_get_hwid();
   write(S, 0, 0, v);
}


// RUN: %cmc -Qxcm_jit_target=BDW %w | FileCheck %w
//
// CHECK: -platform BDW
// CHECK-NOT: error
// CHECK-NOT: warning

// We check that the generated asm contains an AND to mask the
// 10 bits from the r0.5 register that contain the hwid.
//
// RUN: FileCheck -input-file=%W_0.asm -check-prefix=ASM %w
//
// ASM: and
// ASM-SAME: r0.5<0;1,0>:ud
// ASM-SAME: 0x3ff:ud

// tidy up the generated files
// RUN: rm %W_0.dat %W_0.visaasm %W_0.asm %W.isa
