/*========================== begin_copyright_notice ============================

Copyright (C) 2017-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#define GEN_KERNEL

#undef CM_DEBUG

#include <cm/cm.h>

#ifdef CMRT_EMU
# include <shim_support.h>
#endif  //  CMRT_EMU

#include "share.h"

// Process 32x16 block of result matrix:

extern "C" _GENX_MAIN_ void sgemm_kernel (
                                int m, int n, int k,
                                int ib, int jb, int kb,
                                SurfaceIndex indxA [[type("image2d_t float")]],
                                SurfaceIndex indxB [[type("image2d_t float")]],
                                SurfaceIndex indxC [[type("image2d_t float")]])
{
  enum { Tm = 32, Tn = 16, Tk = 8 };
  matrix<float, Tm, Tk> a;
  matrix<float, Tn, Tk> b;
  matrix<float, Tm, Tn> c;

  int idx = cm_group_id(0)*cm_local_size(0) + cm_local_id(0);
  int idy = cm_group_id(1)*cm_local_size(1) + cm_local_id(1);
  int dst_col = (jb + idx * Tn) * sizeof(float);
  int dst_row = ib + idy * Tm;
  int kk;

  kk = k;
  if (kk > GEMM_BLOCK) kk = GEMM_BLOCK;

#ifdef CM_DEBUG
  printf(">>> M = %d N = %d K = %d  idx = %d  idy = %d  ib = %d  jb = %d  kb = %d\n",
	 m, n, k, idx, idy, ib, jb, kb);
#endif

  // Read the earlier value of C matrix (Read 32x16 block of C)
  read (indxC, dst_col,    dst_row   , c.select<8, 1, 8, 1> (0, 0));
  read (indxC, dst_col+32, dst_row   , c.select<8, 1, 8, 1> (0, 8));
  read (indxC, dst_col,    dst_row+8 , c.select<8, 1, 8, 1> (8, 0));
  read (indxC, dst_col+32, dst_row+8 , c.select<8, 1, 8, 1> (8, 8));
  read (indxC, dst_col,    dst_row+16, c.select<8, 1, 8, 1> (16, 0));
  read (indxC, dst_col+32, dst_row+16, c.select<8, 1, 8, 1> (16, 8));
  read (indxC, dst_col,    dst_row+24, c.select<8, 1, 8, 1> (24, 0));
  read (indxC, dst_col+32, dst_row+24, c.select<8, 1, 8, 1> (24, 8));

  int kb_m4 = kb * sizeof(float);

  for (int l = kb_m4; l < kb_m4 + kk * sizeof(float); l += Tk * sizeof(float)) {

    // Read 8x16 block of B matrix
      int k_d4 = l / sizeof(float);
      read (indxB, dst_col,    k_d4, b.select<8,1,8,1>(0,0));
      read (indxB, dst_col+32, k_d4, b.select<8,1,8,1>(8,0));

      // Read 32x8 block of A matrix
      read (indxA, l, dst_row   , a.select<8, 1, 8, 1> (0, 0));
      read (indxA, l, dst_row+8 , a.select<8, 1, 8, 1> (8, 0));
      read (indxA, l, dst_row+16, a.select<8, 1, 8, 1> (16, 0));
      read (indxA, l, dst_row+24, a.select<8, 1, 8, 1> (24, 0));

      // Compute a 32x8 block of C
      #pragma  unroll
      for(int ll = 0; ll < Tk; ll ++)  {
        #pragma unroll
        for(int ii = 0; ii < Tm; ii ++) {
            c.select<1,1,8,1>(ii,0) += a(ii,ll) * b.select<1,1,8,1>(ll,0);
        }
      }

      // Compute the next 32x8 block of C
      #pragma  unroll
      for(int ll = 0; ll < Tk; ll ++)  {
        #pragma unroll
        for(int ii = 0; ii < Tm; ii ++) {
            c.select<1,1,8,1>(ii,8) += a(ii,ll) * b.select<1,1,8,1>(ll+8,0);
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

#ifdef CMRT_EMU
EXPORT_SIGNATURE(sgemm_kernel);
#endif  //  CMRT_EMU
