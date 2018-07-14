#include <cm/cm.h>

_GENX_ matrix<float, 2, 2> M;                        // global data declaration

_GENX_ matrix<float, 4, 4> plusone(matrix<float, 4, 4> m)
{
  return m+1.0f;
}

_GENX_ float foo(matrix<float, 4, 4> m)              // user defined GenX function
{
  matrix<float, 4, 4> newm = plusone(m);
  return cm_sum<float>(newm);
}

_GENX_ void bar_value(vector<float, 16> v, float f)  // user defined GenX function
                                                     // using pass-by-value for "v"
{
  matrix<float, 4, 4> m1;
  m1 = v + f;
  float s = foo(m1);
  M = m1.select<2, 2, 2, 2>(0, 0) * s;
}

_GENX_ void bar_ref(vector<float, 16> v, float f,    // user defined GenX function
                    matrix_ref<float, 2, 2> m)       // using pass-by-reference for "m"
{
  matrix<float, 4, 4> m1;
  m1 = v + f;
  float s = foo(m1);
  m = m1.select<2, 2, 2, 2>(0, 0) * s;
}

_GENX_MAIN_ void kernel(SurfaceIndex inbuf, SurfaceIndex outbuf, // GenX kernel function
                        int x_pos, int y_pos)
{
  matrix<float, 4, 4> m;
  vector<float, 16>   v;

  read(inbuf, x_pos, y_pos, m);
  v = m;
  bar_value(v, 0.5f);
  write(outbuf, x_pos, y_pos, M);
  bar_ref(v, 1.0f, M);
  write(outbuf, x_pos, y_pos + 2, M);
}

// output a warning just to have some output from the compiler to check
#warning 4_2_a.cpp

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: warning: 4_2_a.cpp
// CHECK: 1 warning generated
