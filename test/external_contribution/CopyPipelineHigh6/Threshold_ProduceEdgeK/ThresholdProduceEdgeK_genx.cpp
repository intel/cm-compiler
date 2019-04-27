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

#define BLOCK_WIDTH 32
#define BLOCK_HEIGHT 16

#define X 0
#define Y 1

inline _GENX_ void SimpleThreshold
   (
      matrix_ref<uchar, 16, 16> in,
      matrix_ref<uchar, 16, 16> out,
      short threshold
   )
{
   matrix<uchar, 16, 16> max;
   max = cm_max<uchar>(in, threshold);
   out = (max == in);
   out *= 0xff;
}

inline _GENX_ void Dilate
   (
      matrix_ref<uchar, 18, 18>in,
      matrix_ref<uchar, 16, 16> out
   )
{
   matrix<short, 18, 16> m;
   matrix<short, 16, 16> m_out;

   m = cm_max<short>(in.select<18,1,16,1>(0,0), in.select<18,1,16,1>(0,1));
   m = cm_max<short>(m, in.select<18,1,16,1>(0,2));

   m_out = cm_max<short>(m.select<16,1,16,1>(0,0), m.select<16,1,16,1>(1,0));
   m_out = cm_max<short>(m_out, m.select<16,1,16,1>(2,0));

   out = m_out.format<uchar, 16, 32>().select<16,1,16,2>(0,0);
}

inline _GENX_ void ProduceEdgeK
   (
      matrix_ref<uchar, 16, 16> inNeutral,
      matrix_ref<uchar, 16, 16> inThresholdMaxFilter,
      matrix_ref<uchar, 16, 16> inThresholdK,
      matrix_ref<uchar, 16, 16> inErrorDiffusionK,
      matrix_ref<uchar, 16, 16> out
   )
{
   matrix<uchar, 16, 16> n_and_t1 = inNeutral & inThresholdMaxFilter;
   matrix<uchar, 16, 16> n_and_t1_and_t2 = n_and_t1 & inThresholdK;
   matrix<uchar, 16, 16> notn_and_t1_and_k = ~(n_and_t1) & inErrorDiffusionK;

   out = notn_and_t1_and_k | n_and_t1_and_t2;
}

inline _GENX_ void Pack8to1
   (
      matrix_ref<uchar, 16, 16> in,
      matrix_ref<uchar, 16, 2> out
   )
{
   matrix<uchar, 16, 16> transposed_in;
   cmtl::Transpose_16x16(in.select_all(), transposed_in.select_all());

#pragma unroll
   for (int j=0; j<2; j++)
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
//  Combine ProduceEdgeK + Dilate + Simple Threshold for picture in U8 pixel format.
//
//////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void ThresholdProduceEdgeK_GENX
   (
      SurfaceIndex SrcRemoveFringeKSI,    // Source picture surface index
      SurfaceIndex SrcNeutralEdgeMaskSI,  // Source picture surface index
      SurfaceIndex SrcErrorDiffusionKSI,  // Source picture surface index
      SurfaceIndex DstSI,                 // Destination picture surface index
      short thresholdK,
      short thresholdMaxFilter
   )
{
   matrix<uchar, 18, 32> inRF;
   matrix<uchar, 16, 4> out;
   vector<short, 2> pos;
   short posXout;

   pos(X)= cm_local_id(X) + cm_group_id(X) * cm_local_size(X);
   posXout = pos(X) * 4;
   pos(Y)= cm_local_id(Y) + cm_group_id(Y) * cm_local_size(Y);
   pos(X) = pos(X) * 32;
   pos(Y) = pos(Y) * 16;

   // CM doesn't support 16x2 output, as a result using 2x 16x16 to produce 16x4
   // output
#pragma unroll
   for (int loop = 0; loop<2; loop++)
   {

      read(SrcRemoveFringeKSI, pos(X) -1, pos(Y) -1 , inRF.select<8,1,32,1>(0,0));
      read(SrcRemoveFringeKSI, pos(X) -1, pos(Y) +7 , inRF.select<8,1,32,1>(8,0));
      read(SrcRemoveFringeKSI, pos(X) -1, pos(Y) +15 , inRF.select<2,1,32,1>(16,0));

      // Generate ThresholdK 
      matrix<uchar, 16, 16> outThresholdK;
      SimpleThreshold(inRF.select<16,1,16,1>(1,1), outThresholdK, thresholdK);

      // Dilate then ThresholdFilter
      matrix<uchar, 16, 16> outDilate;
      matrix<uchar, 16, 16> outMaxFilter;
      Dilate(inRF.select<18,1,18,1>(0,0), outDilate);
      SimpleThreshold(outDilate, outMaxFilter, thresholdMaxFilter);

      //ProduceK
      matrix<uchar, 16, 16> outProduceK;
      matrix<uchar, 16, 16> inNEM;
      matrix<uchar, 16, 16> inED;

      read(SrcNeutralEdgeMaskSI, pos(X), pos(Y), inNEM.select<16,1,16,1>(0,0));
      read(SrcErrorDiffusionKSI, pos(X), pos(Y), inED.select<16,1,16,1>(0,0));

      ProduceEdgeK(inNEM, outMaxFilter, outThresholdK, inED, outProduceK);

      //Pack8to1
      matrix<uchar, 16, 2> out1K;
      Pack8to1(outProduceK, out1K);
      out.select<16,1,2,1>(0,loop*2) = out1K;

      pos(X) = pos(X) + 16;
   }
   write(DstSI, posXout, pos(Y), out);
}
