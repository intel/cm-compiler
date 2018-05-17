/*
 * Copyright (c) 2017, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#define GEN_KERNEL

#include <cm/cm.h>
#include "share.h"

// Process 32x16 block of result matrix:

extern "C" _GENX_MAIN_ void sgemm_kernel (
                                int M, int N, int K,
                                int ib, int jb, int kb,
                                SurfaceIndex indxA,
                                SurfaceIndex indxB,
                                SurfaceIndex indxC)
{
  enum { Tm = 32, Tn = 16, Tk = 8 };
  matrix<float, Tm, Tk> a;
  matrix<float, Tn, Tk> b;
  matrix<float, Tm, Tn> c;

  int idx = cm_group_id(0)*cm_local_size(0) + cm_local_id(0);
  int idy = cm_group_id(1)*cm_local_size(1) + cm_local_id(1);
  int dst_col = (jb + idx * 16) * sizeof(float);
  int dst_row = ib + idy * 32;

  // Read the earlier value of C matrix (Read 32x16 block of C)
  read (indxC, dst_col,    dst_row   , c.select<8, 1, 8, 1> (0, 0));
  read (indxC, dst_col+32, dst_row   , c.select<8, 1, 8, 1> (0, 8));
  read (indxC, dst_col,    dst_row+8 , c.select<8, 1, 8, 1> (8, 0));
  read (indxC, dst_col+32, dst_row+8 , c.select<8, 1, 8, 1> (8, 8));
  read (indxC, dst_col,    dst_row+16, c.select<8, 1, 8, 1> (16, 0));
  read (indxC, dst_col+32, dst_row+16, c.select<8, 1, 8, 1> (16, 8));
  read (indxC, dst_col,    dst_row+24, c.select<8, 1, 8, 1> (24, 0));
  read (indxC, dst_col+32, dst_row+24, c.select<8, 1, 8, 1> (24, 8));

  int kb_m4 = kb << 2;

  for (int k = kb_m4; k < (kb_m4 + GEMM_BLOCK*4); k += 32) {

      // Read 8x16 block of B matrix
      int k_d4 = k >> 2;
      read (indxB, dst_col,    k_d4, b.select<8,1,8,1>(0,0));
      read (indxB, dst_col+32, k_d4, b.select<8,1,8,1>(8,0));

      // Read 32x8 block of A matrix
      read (indxA, k, dst_row   , a.select<8, 1, 8, 1> (0, 0));
      read (indxA, k, dst_row+8 , a.select<8, 1, 8, 1> (8, 0));
      read (indxA, k, dst_row+16, a.select<8, 1, 8, 1> (16, 0));
      read (indxA, k, dst_row+24, a.select<8, 1, 8, 1> (24, 0));

      // Compute a 32x8 block of C
      #pragma  unroll
      for(int kk = 0; kk < Tk; kk ++)  {
        #pragma unroll
        for(int ii = 0; ii < Tm; ii ++) {
            c.select<1,1,8,1>(ii,0) += a(ii,kk) * b.select<1,1,8,1>(kk,0);
        }
      }

      // Compute the next 32x8 block of C
      #pragma  unroll
      for(int kk = 0; kk < Tk; kk ++)  {
        #pragma unroll
        for(int ii = 0; ii < Tm; ii ++) {
            c.select<1,1,8,1>(ii,8) += a(ii,kk) * b.select<1,1,8,1>(kk+8,0);
        }
      }

  }

  // Write a 32x16 block of C
  write (indxC, dst_col,    dst_row   , c.select<8, 1, 8, 1> (0, 0));
  write (indxC, dst_col+32, dst_row   , c.select<8, 1, 8, 1> (0, 8));
  write (indxC, dst_col,    dst_row+8 , c.select<8, 1, 8, 1> (8, 0));
  write (indxC, dst_col+32, dst_row+8 , c.select<8, 1, 8, 1> (8, 8));
  write (indxC, dst_col,    dst_row+16, c.select<8, 1, 8, 1> (16, 0));
  write (indxC, dst_col+32, dst_row+16, c.select<8, 1, 8, 1> (16, 8));
  write (indxC, dst_col,    dst_row+24, c.select<8, 1, 8, 1> (24, 0));
  write (indxC, dst_col+32, dst_row+24, c.select<8, 1, 8, 1> (24, 8));
}
