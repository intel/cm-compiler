// RUN: %cmc --help | FileCheck --check-prefix=CHECK-HELP %s
// RUN: %cmc -help | FileCheck --check-prefix=CHECK-HELP %s
// RUN: %cmc --help123123 2>&1 | FileCheck --check-prefix=CHECK-NO-HELP %s
// RUN: %cmc -help123123 2>&1 | FileCheck --check-prefix=CHECK-NO-HELP-SHORT %s
// CHECK-HELP: CMOC-specific help
// CHECK-NO-HELP: error:
// CHECK-NO-HELP-NOT: CMOC-specific help
// CHECK-NO-HELP-SHORT: error:
// CHECK-NO-HELP-SHORT-NOT: CMOC-specific help


#include <cm/cm.h>

extern "C" _GENX_MAIN_
void test_kernel() {
}

