#include <cm/cm.h>

_GENX_MAIN_ void printf_demo_genx()
{
  int tx = get_thread_origin_x();
  int ty = get_thread_origin_y();

  printf("Number of bytes returned: %d\n", printf("Hello CM from %dx%d\n", tx, ty));

  double foo = 4.2;
  printf("Print a double: %f\n", (double)foo);

  printf("Print a string: %s\n", "Hello");

  if (tx == 0 && ty == 0) // Only print on tid 0
  {
    printf("Hello from tid 0 with tx/ty %d/%d \n", tx, ty);
  }
}

// output a warning just to have some output from the compiler to check
#warning 4_17_1_a.cpp

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: 4_17_1_a.cpp
// CHECK: 1 warning generated
