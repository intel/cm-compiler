#include <cm/cm.h>

                matrix<float, 2, 2> M;      // global variable declaration
_GENX_VOLATILE_ matrix<float, 4, 8> G;      // volatile global variable declaration

inline float foo(matrix<float, 4, 4> m)
{
  return cm_sum<float>(m);
}

inline void bar(vector<float, 16> v, float f)
{
  matrix<float, 4, 4> m1;
  m1 = v + f;
  float s = foo(m1);
  M = m1.select<2, 2, 2, 2>(0, 0) * s;
}

inline void baz(int i) {
  G += i;
}

_GENX_MAIN_ void kernel(SurfaceIndex inbuf, SurfaceIndex outbuf,
                        int x_pos, int y_pos)
{
  matrix<float, 4, 4> m;
  vector<float, 16>   v;

  read(inbuf, x_pos, y_pos, m);
  v = m;
  bar(v, 0.5f);
  write(outbuf, x_pos, y_pos, M);

  G = 0;
  baz(x_pos);
  write(outbuf, x_pos, y_pos, G);
}

// output a warning just to have some output from the compiler to check
#warning 2_4_a.cpp

// RUN: %cmc %w 2>&1 | FileCheck --implicit-check-not error %w
// RUN: rm %W.isa
// CHECK: 1 warning generated
