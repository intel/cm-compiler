#include <cm/cm.h>

//A template function that multiplies an arbitrary matrix by 3
template <typename T, uint R, uint C>
_GENX_ inline void
mult3(matrix_ref<T, R, C> par)
{
  par = par * 3;
}
   
_GENX_MAIN_ void
kern(matrix<int, 4, 2> p)
{
  matrix<int, 4, 2> m(p); // m = p;
  mult3(m.select_all());  // m = p * 3;
}

// output a warning just to have some output from the compiler to check
#warning 3_2_9_g.cpp

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
