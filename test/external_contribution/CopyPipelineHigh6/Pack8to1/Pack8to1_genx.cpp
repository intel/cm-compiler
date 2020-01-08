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

#define BLKW 16
#define BLKH 16

#define X 0
#define Y 1

inline _GENX_ void Pack8to1
   (
      matrix_ref<uchar, 16, 16> in,
      matrix_ref<uchar, 16, 2> out
   )
{
   matrix<uchar, 16, 16> transposed_in;
   cmtl::Transpose_16x16(in.select_all(), transposed_in.select_all());

#pragma unroll
   for (int j = 0; j < 2; j++)
   {
      out.select<16,1,1,1>(0,j) =
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
Pack8to1_4C_GENX(
   SurfaceIndex SrcSI,
   SurfaceIndex DstCSI,
   SurfaceIndex DstMSI,
   SurfaceIndex DstYSI,
   SurfaceIndex DstKSI
   )
{
   matrix<uchar, BLKH, BLKW * 4> cmykIn;
   vector<short, 2> pos, pos4, posout;

   pos(X) = cm_local_id(X) + cm_group_id(X) * cm_local_size(X);
   pos(Y) = cm_local_id(Y) + cm_group_id(Y) * cm_local_size(Y);

   // Even through the block size is 16x16, but as two loops, the actual block
   // size is 32x16
   pos4(X) = pos(X) * 32 * 4;
   pos4(Y) = pos(Y) * BLKH;
   posout(X) = pos(X) * 4;
   pos(X) = pos(X) * 32;
   pos(Y) = pos(Y) * BLKH;

   matrix<uchar, BLKH, 2> out1bit;
   matrix<uchar, BLKH, 4> outC1bit, outM1bit, outY1bit;

#pragma unroll
   for (int loop = 0; loop < 2; loop++)
   {
      read(SrcSI, pos4(X), pos4(Y), cmykIn.select<8,1,32,1>(0,0));
      read(SrcSI, pos4(X)+32, pos4(Y), cmykIn.select<8,1,32,1>(0,32));
      read(SrcSI, pos4(X), pos4(Y)+8, cmykIn.select<8,1,32,1>(8,0));
      read(SrcSI, pos4(X)+32, pos4(Y)+8, cmykIn.select<8,1,32,1>(8,32));

      matrix<uchar,BLKH, BLKW> kinput(cmykIn.select<16,1,16,4>(0,3));
      matrix<uchar,BLKH, BLKW> yinput(cmykIn.select<16,1,16,4>(0,2));
      matrix<uchar,BLKH, BLKW> minput(cmykIn.select<16,1,16,4>(0,1));
      matrix<uchar,BLKH, BLKW> cinput(cmykIn.select<16,1,16,4>(0,0));

      Pack8to1(cinput, out1bit);
      outC1bit.select<16,1,2,1>(0,loop*2) = out1bit;

      Pack8to1(minput, out1bit);
      outM1bit.select<16,1,2,1>(0,loop*2) = out1bit;

      Pack8to1(yinput, out1bit);
      outY1bit.select<16,1,2,1>(0,loop*2) = out1bit;

      write(DstKSI, pos(X), pos(Y), kinput);

      pos4(X) += 64;
      pos(X) += 16;
   }

   write(DstCSI, posout(X), pos(Y), outC1bit);
   write(DstMSI, posout(X), pos(Y), outM1bit);
   write(DstYSI, posout(X), pos(Y), outY1bit);

}
