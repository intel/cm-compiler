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

#define BLKW 8
#define BLKH 4
#define ITERATION 4

#define X 0
#define Y 1

/* 
 * Error diffusion push error to neighbour pixels, and result the algorithm need
 * to run in sequence.  Using following pixel processing order allow parallelism
 * but not violate the dependency issue. 
 * Using pixelRowIndex and pixelColIndex to determine the pixel sequence to
 * process the error diffusion
 *
 * Some pixels can process together, but some can't.  To reduce the conditional
 * checking, if the pixel can't parallel with other pixel, repeat to process the
 * same pixel.  For SIMD, it isn't any performance penalty
 */

short pixelLocX[32] ={ 0,1,2,3,4,5,6,7,-2,-1,0,1,2,3,4,5,
                      -4,-3,-2,-1,0,1,2,3,-6,-5,-4,-3,-2,-1,0,1
                     };
short pixelLocY[32] ={ 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
                      2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3
                     };
uchar weight[4] = {1,5,3,7};

extern "C" _GENX_MAIN_ void
ErrorDiffusion_GENX(
   SurfaceIndex SrcSI,
   SurfaceIndex ErrBufRowSI,
   SurfaceIndex ErrBufColSI,
   SurfaceIndex DstSI,
   int widthBlk
)
{
   matrix<uint, BLKH, 8> input;
   matrix<uint, BLKH, 8> output;
   matrix<short, 5, 64> errBuf = 0;
   vector<short, 64> errRowTemp;
   vector<short, 2> pos, pos4;
   vector<uint, 32> inputVectorX(pixelLocX);
   vector<uint, 32> inputVectorY(pixelLocY);
   matrix<uint, 4, 16> tmpInput;

   cm_wait();

   pos(X) = get_thread_origin_x();
   pos(Y) = get_thread_origin_y();

   pos4(X) = pos(X) * BLKW;
   pos4(Y) = pos(Y) * BLKH;

   inputVectorX += pos4(X);
   inputVectorY += pos4(Y);

   read_typed(SrcSI, CM_R_ENABLE, tmpInput.select<1,1,16,1>(0,0),
         inputVectorX.select<16,1>(0), inputVectorY.select<16,1>(0));
   input.select<1,1,8,1>(0,0) = tmpInput.select<1,1,8,1>(0,0);
   input.select<1,1,8,1>(1,0) = tmpInput.select<1,1,8,1>(0,8);

   read_typed(SrcSI, CM_R_ENABLE, tmpInput.select<1,1,16,1>(0,0),
         inputVectorX.select<16,1>(16), inputVectorY.select<16,1>(16));
   input.select<1,1,8,1>(2,0) = tmpInput.select<1,1,8,1>(0,0);
   input.select<1,1,8,1>(3,0) = tmpInput.select<1,1,8,1>(0,8);

   // Need to update, try to use single column for column error buffer and
   // single row for row buffer (single row for row buffer may not possible)
   int colErrBufOffset = (pos(Y))*128;
   int rowErrBufOffset = (pos(X)+1)*64-24;


   if (pos(Y) != 0)
      read(DWALIGNED(ErrBufRowSI), rowErrBufOffset, errRowTemp.select<40,1>(0));
   else
      errRowTemp.select<40,1>(0) = 0;

   errBuf.select<1,1,40,1>(0,24) = errRowTemp.select<40,1>(0);

   if (pos(X) != 0)
      read(ErrBufColSI, colErrBufOffset , errRowTemp.select<40,1>(0));
   else
      errRowTemp.select<40,1>(0) = 0;

   errBuf.select<1,1,12,1>(1,16) = errRowTemp.select<12,1>(0);
   errBuf.select<1,1,12,1>(2,8) = errRowTemp.select<12,1>(12);
   errBuf.select<1,1,12,1>(3,0) = errRowTemp.select<12,1>(24);
   errBuf.select<1,1,4,1>(4,0) = errRowTemp.select<4,1>(36);

   // The read extra one column to left and right, so, it will directly map into
   // error buffer
   //
   // The input buffer is a diaganol box, but we read a square to wrap this box

   // The read extra one column to left and right, so, it will directly map into
   // error buffer
   // The input buffer is a diaganol box, but we read a square to wrap this box
   //       ________
   //     _|       _|
   //   _|       _|
   // _|       _|
   //|_______ |
   //
   vector<uint, 8> oldpixel, newpixel;

   vector_ref<uchar, 32> oldpixelc = oldpixel.format<uchar>();
   vector_ref<uchar, 32> newpixelc = newpixel.format<uchar>();
   vector<float, 32> fxs;

   vector<float, 4> weightf(weight);

   weightf =weightf/16.0f;

   // this is error buffer size matrix<short, 5, 64> errBuf = 0;
   int colIndex, rowIndex;
#pragma unroll
   for (int i=0; i < ITERATION; i++)
   {
      fxs = 0;
      // oldpixel should be from input one row
      oldpixel = input.select<1,1,8,1>(i,0);

      colIndex = 24 - i*8;
      rowIndex = i;
      if (pos(X) == widthBlk)
           errBuf.select<5,1,36,1>(0, 28) = 0;

      fxs += errBuf.select<1,1,32,1>(rowIndex,colIndex) * weightf[0];

      fxs += errBuf.select<1,1,32,1>(rowIndex,colIndex+4) * weightf[1];

      fxs += errBuf.select<1,1,32,1>(rowIndex,colIndex+8) * weightf[2];

      if (pos(X) == 0)
      {
         if (i == 1)
            fxs.select<8,1>(0)  = 0;
         else if (i == 2)
            fxs.select<16,1>(0) =  0;
         else if (i == 3)
            fxs.select<24,1>(0) = 0;
      }

      rowIndex += 1;

      vector<float, 4> pixelleft (errBuf.select<1,1,4,1>(rowIndex, colIndex));
      vector<float, 4> singlefxs;
      int rowi;

#pragma unroll
      for (int ii=0; ii < 8; ii++)
      {
         rowi = ii*4;

         singlefxs = pixelleft * weightf[3];

         singlefxs = singlefxs + fxs.select<4,1>(rowi) +
            oldpixelc.select<4,1>(rowi);

         newpixelc.select<4,1>(rowi) = (singlefxs > 127.0f)* 0xff;
         fxs.select<4,1>(rowi) = cm_rndd<float>(singlefxs - newpixelc.select<4,1>(rowi));
         pixelleft = fxs.select<4,1>(rowi);

      }

      // Temporary workaround the right border incorrect data
      if (pos(X) == widthBlk)
      {
         if (i == 1)
           fxs.select<24,1>(8) = 0;
         else if (i == 2)
            fxs.select<16,1>(16) = 0;
         else if (i == 3)
            fxs.select<8,1>(24) = 0;
      }
      errBuf.select<1,1,32,1>(rowIndex,colIndex+4) = fxs;

      output.select<1,1,8,1>(rowIndex-1, 0) = newpixel;

   }

   //output write need to use scatter write as it index is 
   matrix<uint, 4, 16> tmpOut;

   tmpOut.select<1,1,8,1>(0,0) = output.select<1,1,8,1>(0,0);
   tmpOut.select<1,1,8,1>(0,8) = output.select<1,1,8,1>(1,0);

   write_typed(DstSI, CM_R_ENABLE, tmpOut,
         inputVectorX.select<16,1>(0), inputVectorY.select<16,1>(0));

   tmpOut.select<1,1,8,1>(0,0) = output.select<1,1,8,1>(2,0);
   tmpOut.select<1,1,8,1>(0,8) = output.select<1,1,8,1>(3,0);
   write_typed(DstSI, CM_R_ENABLE, tmpOut,
         inputVectorX.select<16,1>(16), inputVectorY.select<16,1>(16));

   vector<short, 64> errBufColOut;

   errBufColOut.select<12,1>(0) = errBuf.select<1,1,12,1>(1,48);
   errBufColOut.select<12,1>(12) = errBuf.select<1,1,12,1>(2,40);
   errBufColOut.select<12,1>(24) = errBuf.select<1,1,12,1>(3,32);
   errBufColOut.select<4,1>(36) = errBuf.select<1,1,4,1>(4,32);

   colErrBufOffset = (pos(Y))*128;
   rowErrBufOffset = (pos(X))*64;

   write(ErrBufRowSI, rowErrBufOffset, errBuf.select<1,1,32,1>(4,4).format<short>());
   write(ErrBufColSI, colErrBufOffset, errBufColOut.select<64,1>(0));

   cm_fence();
   cm_signal();
}
