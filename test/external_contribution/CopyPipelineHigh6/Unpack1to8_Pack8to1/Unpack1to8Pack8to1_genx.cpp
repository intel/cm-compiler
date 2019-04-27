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
#include <cm/cm.h>
#include <cm/cmtl.h>

#define BLKW 4
#define BLKH 16

#define X 0
#define Y 1

static const uchar shift[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };

void Unpack1to8_GENX (
      matrix_ref<uchar, BLKH, BLKW> packIn,
      matrix_ref<uchar, BLKH, BLKW*8> unpackOut
      )
{
   vector<uchar, 8> sh(shift);
   vector<uchar, BLKW*8> sh64 = sh.replicate<BLKW>();

#pragma unroll
   for (int i = 0; i < BLKH; i++)
      unpackOut.select<1,1,BLKW*8,1>(i, 0) =
         cm_shr<uchar>(packIn.replicate<4,1,8,0>(i,0), sh64);

   unpackOut &= 0x01;
   unpackOut *= 0xff;
}

void Pack8to1_GENX (
      matrix_ref<uchar, 16, 16> packIn,
      matrix_ref<uchar, 16, 2> unpackOut
      )
{
   matrix<uchar, 16, 16> transposed_in;

   cmtl::Transpose_16x16(packIn.select_all(), transposed_in.select_all());

#pragma unroll
   for (int j = 0; j < 2; j++)
   {
      unpackOut.select<16,1,1,1>(0, j) =
         (transposed_in.row(0+j*8) & 0x80) >> 0 |
         (transposed_in.row(1+j*8) & 0x80) >> 1 |
         (transposed_in.row(2+j*8) & 0x80) >> 2 |
         (transposed_in.row(3+j*8) & 0x80) >> 3 |
         (transposed_in.row(4+j*8) & 0x80) >> 4 |
         (transposed_in.row(5+j*8) & 0x80) >> 5 |
         (transposed_in.row(6+j*8) & 0x80) >> 6 |
         (transposed_in.row(7+j*8) & 0x80) >> 7;
   }

}


extern "C" _GENX_MAIN_ void
Unpack1to8Pack8to1_GENX (
      SurfaceIndex SrcSIC,
      SurfaceIndex SrcSIM,
      SurfaceIndex SrcSIY,
      SurfaceIndex SrcSIK,
      SurfaceIndex DstSICMYK
      )
{
   vector<short, 2> pos;
   matrix<uchar, BLKH, BLKW> packC, packM, packY, packK;
   matrix<uchar, BLKH, BLKW*8> unpackC, unpackM, unpackY, unpackK;

   pos(X) = cm_local_id(X) + cm_group_id(X)*cm_local_size(X);
   pos(Y) = cm_local_id(Y) + cm_group_id(Y)*cm_local_size(Y);
   pos(Y) *= BLKH;
   pos(X) *= BLKW;
   short posXout = pos(X) *  4;

   read(SrcSIC, pos(X), pos(Y), packC);
   Unpack1to8_GENX(packC, unpackC);

   read(SrcSIM, pos(X), pos(Y), packM);
   Unpack1to8_GENX(packM, unpackM);

   read(SrcSIY, pos(X), pos(Y), packY);
   Unpack1to8_GENX(packY, unpackY);

   read(SrcSIK, pos(X), pos(Y), packK);
   Unpack1to8_GENX(packK, unpackK);

   matrix<uchar, BLKH, BLKW*8*4> unpackTmp;
   matrix<uchar, BLKH, BLKW*4> unpackOut;

   unpackTmp.select<BLKH,1,32,4>(0,0) = unpackC;
   unpackTmp.select<BLKH,1,32,4>(0,1) = unpackM;
   unpackTmp.select<BLKH,1,32,4>(0,2) = unpackY;
   unpackTmp.select<BLKH,1,32,4>(0,3) = unpackK;

#pragma unroll
   for (int i = 0; i < 8; i++)
   {
      Pack8to1_GENX(unpackTmp.select<BLKH,1,16,1>(0,i<<4), unpackOut.select<16,1,2,1>(0, i<<1));
   }

   write(DstSICMYK, posXout, pos(Y), unpackOut.select<16,1,16,1>(0,0));

}
