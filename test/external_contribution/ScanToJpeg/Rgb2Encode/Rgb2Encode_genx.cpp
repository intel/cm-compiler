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
#define BLKH 8

#define X 0
#define Y 1

extern "C" _GENX_MAIN_ void
Rgb2YCbCr_GENX (
      SurfaceIndex SrcSIR,
      SurfaceIndex SrcSIG,
      SurfaceIndex SrcSIB,
      SurfaceIndex DstSIYCbCr,
      vector<float, 9> Coeffs,
      vector<float, 3> Offsets
      )
{
   vector<short, 2> pos;

   pos(X) = get_thread_origin_x() * BLKW;
   pos(Y) = get_thread_origin_y() * BLKH;

   matrix<uchar, BLKH, BLKW> inr;
   matrix<uchar, BLKH, BLKW> ing;
   matrix<uchar, BLKH, BLKW> inb;

   read(SrcSIR, pos(X), pos(Y), inr);
   read(SrcSIG, pos(X), pos(Y), ing);
   read(SrcSIB, pos(X), pos(Y), inb);

   matrix<float, BLKH, BLKW> y, cb, cr;

   // Y
   y = inr*Coeffs(0) + ing*Coeffs(1) + inb*Coeffs(2) + Offsets(0);
   // Cb
   cb = inr*Coeffs(3) + ing*Coeffs(4) + inb*Coeffs(5) + Offsets(1);
   // Cr
   cr = inr*Coeffs(6) + ing*Coeffs(7) + inb*Coeffs(8) + Offsets(2);

   matrix<uchar, BLKH, BLKW*4> ycbcr;

   ycbcr.select<BLKH, 1, BLKW, 4>(0,0) = matrix<uchar, BLKH, BLKW>(y, SAT);
   ycbcr.select<BLKH, 1, BLKW, 4>(0,1) = matrix<uchar, BLKH, BLKW>(cr, SAT);
   ycbcr.select<BLKH, 1, BLKW, 4>(0,2) = matrix<uchar, BLKH, BLKW>(cb, SAT);
   ycbcr.select<BLKH, 1, BLKW, 4>(0,3) = 0;

   write(DstSIYCbCr, pos(X)*4, pos(Y), ycbcr);

}

extern "C" _GENX_MAIN_ void
Rgb2Y8_GENX (
      SurfaceIndex SrcSIR,
      SurfaceIndex SrcSIG,
      SurfaceIndex SrcSIB,
      SurfaceIndex DstSIY,
      vector<float, 9> Coeffs,
      vector<float, 3> Offsets
      )
{
   vector<short, 2> pos;

   pos(X) = get_thread_origin_x() * BLKW;
   pos(Y) = get_thread_origin_y() * BLKH;

   matrix<uchar, BLKH, BLKW> inr;
   matrix<uchar, BLKH, BLKW> ing;
   matrix<uchar, BLKH, BLKW> inb;

   read(SrcSIR, pos(X), pos(Y), inr);
   read(SrcSIG, pos(X), pos(Y), ing);
   read(SrcSIB, pos(X), pos(Y), inb);

   matrix<float, BLKH, BLKW> y;

   // Y
   y = inr*Coeffs(0) + ing*Coeffs(1) + inb*Coeffs(2) + Offsets(0);

   matrix<uchar, BLKH, BLKW> yc;

   yc = matrix<uchar, BLKH, BLKW>(y, SAT);

   write(DstSIY, pos(X), pos(Y), yc);

}
