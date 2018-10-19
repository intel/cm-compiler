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

extern "C" _GENX_MAIN_ void
DilateAnd_GENX(
      SurfaceIndex SrcSobelSI,
      SurfaceIndex SrcBoxSI,
      SurfaceIndex DstSI
      )
{
   matrix<uchar, 18, 32> inA;
   matrix<uchar, 16, 16> inB;
   matrix<uchar, 16, 16> dilate_out;
   matrix<short, 18, 16> m;
   matrix<short, 16, 16> m_out;
   vector<short, 2> pos;

   pos(X) = cm_local_id(X) + cm_group_id(X)*cm_local_size(X);
   pos(Y) = cm_local_id(Y) + cm_group_id(Y)*cm_local_size(Y);  // Location of the processing pixel block 
   pos = pos << 4;

   read(SrcSobelSI, pos(X) -1, pos(Y) -1 , inA.select<8,1,32,1>(0,0));
   read(SrcSobelSI, pos(X) -1, pos(Y) +7 , inA.select<8,1,32,1>(8,0));
   read(SrcSobelSI, pos(X) -1, pos(Y) +15 , inA.select<2,1,32,1>(16,0));

   m = cm_max<short>(inA.select<18,1,16,1>(0,0), inA.select<18,1,16,1>(0,1));
   m = cm_max<short>(m, inA.select<18,1,16,1>(0,2));

   m_out = cm_max<short>(m.select<16,1,16,1>(0,0), m.select<16,1,16,1>(1,0));
   m_out = cm_max<short>(m_out, m.select<16,1,16,1>(2,0));

   dilate_out = m_out.format<uchar, 16, 32>().select<16,1,16,2>(0,0);

   read(SrcBoxSI, pos(X), pos(Y), inB);
   matrix<uchar, 16, 16>and_out (inB & dilate_out);

   write(DstSI, pos(X), pos(Y), and_out);
//   write(DstSI, pos(X), pos(Y), m_out.format<uchar, 16,
//         32>().select<16,1,16,2>(0,0));

}
