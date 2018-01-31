#include <cm/cm.h>

const short init_v_B[2][4] = {1, 9, 17, 25, 33, 41, 49, 57};
const short init_Table[5][16] = {{4,6,8},{0,1,1,0,0},{8,9,7,6},{23},{45}};
const short init_0_7[8] = {0,1,2,3,4,5,6,7};

extern "C"
_GENX_MAIN_ void test1(SurfaceIndex OUT, int index)
{
  vector<ushort, 8>     v_0_7(init_0_7);
  matrix<uint, 5, 16>   m_Table(init_Table);
  vector<short, 8>      v_B(init_v_B);
  // ...
}

// output a warning just to have some output from the compiler to check
#warning 3_2_3.cpp

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: 1 warning generated
