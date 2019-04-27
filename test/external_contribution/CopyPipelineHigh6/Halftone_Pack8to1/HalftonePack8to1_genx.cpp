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

//
//  Generate Halftone imnage and Pack8to1 for picture in U8 pixel format.
//
//////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ _GENX_ROUNDING_MODE_(CM_RTZ) void
HalftonePack8to1_GENX(
      SurfaceIndex SrcSI,            // Source picture surface index
      SurfaceIndex DstCSI,           // Destination picture surface index
      SurfaceIndex DstMSI,           // Destination picture surface index
      SurfaceIndex DstYSI,           // Destination picture surface index
      SurfaceIndex DstKSI,           // Destination picture surface index
      SurfaceIndex MapCSI,           // C Mapping surface index
      SurfaceIndex MapMSI,           // M Mapping surface index
      SurfaceIndex MapYSI,           // Y Mapping surface index
      SurfaceIndex MapKSI,           // K Mapping surface index
      int widthC,
      int heightC,
      int widthM,
      int heightM,
      int widthY,
      int heightY,
      int widthK,
      int heightK
     )
{

   matrix<uchar, BLKH, BLKW*4> input;
   vector<short, 2> pos4, pos;
   short posXout;

   pos(X) = cm_local_id(X) + cm_group_id(X) * cm_local_size(X);
   pos(Y) = cm_local_id(Y) + cm_group_id(Y) * cm_local_size(Y);

   posXout = pos(X) * 4;
   pos4(X) = pos(X) * 32 * 4;
   pos4(Y) = pos(Y) * BLKH;
   pos(X) = pos(X) * 32;
   pos(Y) = pos(Y) * BLKH;

   // CM doesn't support 16x2 output, need to process 16x32 to generate 16x4 output
   matrix<uchar, BLKH, 4> outC1bit;
   matrix<uchar, BLKH, 4> outM1bit;
   matrix<uchar, BLKH, 4> outY1bit;
#pragma unroll
   for (int loop = 0; loop < 2; loop++)
   {
      read(SrcSI, pos4(X), pos4(Y), input.select<8,1,32,1>(0,0));
      read(SrcSI, pos4(X)+32, pos4(Y), input.select<8,1,32,1>(0,32));
      read(SrcSI, pos4(X), pos4(Y)+8, input.select<8,1,32,1>(8,0));
      read(SrcSI, pos4(X)+32, pos4(Y)+8, input.select<8,1,32,1>(8,32));

      matrix<uchar, BLKH, BLKW> kinput(input.select<16,1,16,4>(0,3));;
      matrix<uchar, BLKH, BLKW> yinput(input.select<16,1,16,4>(0,2));;
      matrix<uchar, BLKH, BLKW> minput(input.select<16,1,16,4>(0,1));;
      matrix<uchar, BLKH, BLKW> cinput(input.select<16,1,16,4>(0,0));;

      matrix<uchar, BLKH, BLKW> mapInput;
      vector<short, 2> mapPos;
      matrix<uchar, BLKH, BLKW> mask;
      matrix<uchar, BLKH, 2> out1bit;
      matrix<uchar, BLKH, BLKW> out8;

      // Channel C
      mapPos(X) = pos(X) % widthC;
      mapPos(Y) = pos(Y) % heightC;
      read(MapCSI, mapPos(X), mapPos(Y), mapInput);

      mask = (cinput > mapInput);
      out8 = mask * 0xff;
      Pack8to1(out8, out1bit);
      outC1bit.select<16,1,2,1>(0, loop*2) = out1bit;

      // Channel M
      mapPos(X) = pos(X) % widthM;
      mapPos(Y) = pos(Y) % heightM;
      read(MapMSI, mapPos(X), mapPos(Y), mapInput);

      mask = (minput > mapInput);
      out8 = mask * 0xff;
      Pack8to1(out8, out1bit);
      outM1bit.select<16,1,2,1>(0, loop*2) = out1bit;

      // Channel Y
      mapPos(X) = pos(X) % widthY;
      mapPos(Y) = pos(Y) % heightY;
      read(MapYSI, mapPos(X), mapPos(Y), mapInput);

      mask = (yinput > mapInput);
      out8 = mask * 0xff;
      Pack8to1(out8, out1bit);
      outY1bit.select<16,1,2,1>(0, loop*2) = out1bit;

      // Channel K
      mapPos(X) = pos(X) % widthK;
      mapPos(Y) = pos(Y) % heightK;
      read(MapKSI, mapPos(X), mapPos(Y), mapInput);

      mask = (kinput > mapInput);
      out8 = mask * 0xff;
      write(DstKSI, pos(X), pos(Y), out8);

      pos4(X) = pos4(X) + 64;
      pos(X) = pos(X) + 16;
   }

   write(DstCSI, posXout, pos(Y), outC1bit);
   write(DstMSI, posXout, pos(Y), outM1bit);
   write(DstYSI, posXout, pos(Y), outY1bit);

}
