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

#define BLOCK_HEIGHT 16
#define BLOCK_WIDTH 16

#define X 0
#define Y 1

inline _GENX_ void NeutralPixel_GENX(
      matrix_ref<uchar, 16, 16> in_boxA,
      matrix_ref<uchar, 16, 16> in_boxB,
      matrix_ref<uchar, 16, 16> neutralOut
      )
{
   uchar threshLow   = 54;
   uchar threshHigh  = 74;
   uchar bias        = 64;

   in_boxA -= bias;
   in_boxB -= bias;

   matrix<uchar, 16, 16> agtVec = in_boxA > threshLow;
   matrix<uchar, 16, 16> bgtVec = in_boxB > threshLow;

   matrix<uchar, 16, 16> altVec = in_boxA < threshHigh;
   matrix<uchar, 16, 16> bltVec = in_boxB < threshHigh;

   matrix<uchar, 16, 16> anVec = agtVec & altVec;
   matrix<uchar, 16, 16> bnVec = bgtVec & bltVec;

   neutralOut = (anVec & bnVec) * 0xff;

}

//
//  Process two Box3x3 filter for picture in U8 pixel format.
//
//////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
BoxNem_GENX(
      SurfaceIndex SrcASI,    // Source picture surface index
      SurfaceIndex SrcBSI,    // Source picture surface index
      SurfaceIndex DstSI,     // Destination picture surface index
      float        scale
     )
{
   matrix<uchar, 18, 32> inA;
   matrix<uchar, 18, 32> inB;
   matrix<uchar, 16, 16> outX;
   matrix<uchar, 16, 16> outY;
   matrix<short, 18, 16> mX;
   matrix<short, 18, 16> mY;
   matrix<short, 16, 16> mX_out;
   matrix<short, 16, 16> mY_out;
   vector<short, 2> pos;

   pos(X) = cm_local_id(X) + cm_group_id(X)*cm_local_size(X);
   pos(Y) = cm_local_id(Y) + cm_group_id(Y)*cm_local_size(Y);  // Location of the processing pixel block 
   pos = pos << 4;

   read(SrcASI, pos(X) -1, pos(Y) -1 , inA.select<8,1,32,1>(0,0));
   read(SrcASI, pos(X) -1, pos(Y) +7 , inA.select<8,1,32,1>(8,0));
   read(SrcASI, pos(X) -1, pos(Y) +15 , inA.select<2,1,32,1>(16,0));

   read(SrcBSI, pos(X) -1, pos(Y) -1 , inB.select<8,1,32,1>(0,0));
   read(SrcBSI, pos(X) -1, pos(Y) +7 , inB.select<8,1,32,1>(8,0));
   read(SrcBSI, pos(X) -1, pos(Y) +15 , inB.select<2,1,32,1>(16,0));

   mX = inA.select<18,1,16,1>(0,0) + inA.select<18,1,16,1>(0,1) + inA.select<18,1,16,1>(0,2);
   mX_out = mX.select<16,1,16,1>(0,0) + mX.select<16,1,16,1>(1,0) + mX.select<16,1,16,1>(2,0);
   outX = mX_out * scale;

   mY = inB.select<18,1,16,1>(0,0) + inB.select<18,1,16,1>(0,1) + inB.select<18,1,16,1>(0,2);
   mY_out = mY.select<16,1,16,1>(0,0) + mY.select<16,1,16,1>(1,0) + mY.select<16,1,16,1>(2,0);
   outY = mY_out * scale;

   matrix<uchar, 16, 16> neutralOut;

   NeutralPixel_GENX(outX, outY, neutralOut);

   write(DstSI, pos(X), pos(Y), neutralOut);

}

