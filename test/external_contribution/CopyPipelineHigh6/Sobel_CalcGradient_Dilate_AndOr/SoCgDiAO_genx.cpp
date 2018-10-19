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

inline _GENX_ void CalculateGradient_GENX(
   matrix_ref<short, 18, 18> in_Gx,
   matrix_ref<short, 18, 18> in_Gy,
   matrix_ref<uchar, 18, 18> out_high,
   matrix_ref<uchar, 18, 18> out_low
   )
{
   short threshLow = 400;
   short threshHigh = 400;
   uchar vmask     = 0xff;

   matrix<ushort, 18, 18> uGx = cm_abs(in_Gx);
   matrix<ushort, 18, 18> uGy = cm_abs(in_Gy);

   matrix<ushort, 18, 18> gradient = uGx + uGy;

   matrix<ushort, 18, 18> threshlowoutput = (gradient > threshLow) * vmask;
   matrix<ushort, 18, 18> threshhighoutput = (gradient > threshHigh) * vmask;

   out_low = threshlowoutput;
   out_high = threshhighoutput;
}

inline _GENX_ void Dilate0_GENX(
   matrix_ref<uchar, 18, 18> in,
   matrix_ref<uchar, 16, 16> dilate_out
   )
{
   matrix<short, 18, 16> m;
   matrix<short, 16, 16> m_out;

   m = cm_max<short>(in.select<18,1,16,1>(0,0), in.select<18,1,16,1>(0,1));
   m = cm_max<short>(m, in.select<18,1,16,1>(0,2));

   m_out = cm_max<short>(m.select<16,1,16,1>(0,0), m.select<16,1,16,1>(1,0));
   m_out = cm_max<short>(m_out, m.select<16,1,16,1>(2,0));

   dilate_out = m_out.format<uchar,16,32>().select<16,1,16,2>(0,0);
}

inline _GENX_ void And_GENX(
   matrix_ref<uchar, 16, 16> inA,
   matrix_ref<uchar, 16, 16> inB,
   matrix_ref<uchar, 16, 16> out
   )
{
   out = (inA & inB);
}

inline _GENX_ void Or_GENX(
   matrix_ref<uchar, 16, 16> inA,
   matrix_ref<uchar, 16, 16> inB,
   matrix_ref<uchar, 16, 16> out
   )
{
   out = (inA | inB);
}

//
//  Combination kernels of Sobel3x3 + And + Or and Dilate3x3 for picture in U8 pixel format.
//
//////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
SoCgDiAO_GENX(
      SurfaceIndex SrcSI,    // Source picture surface index
      SurfaceIndex DstSI
     )
{
   matrix<uchar, 20, 32> input;
   matrix<short, 20, 18> subtractX;
   matrix<short, 18, 20> subtractY;
   matrix<short, 18, 18> sumX, sumY;

   uint hin_pos = cm_local_id(X) + cm_group_id(X) * cm_local_size(X);
   uint hout_pos = cm_local_id(X) + cm_group_id(X) * cm_local_size(X);
   uint v_pos = cm_local_id(Y) + cm_group_id(Y) * cm_local_size(Y);

   hin_pos = hin_pos << 4;
   hout_pos = hout_pos << 4;
   v_pos = v_pos << 4;

   read(SrcSI, hin_pos -2, v_pos -2 , input.select<8,1,32,1>(0,0));
   read(SrcSI, hin_pos -2, v_pos +6 , input.select<8,1,32,1>(8,0));
   read(SrcSI, hin_pos -2, v_pos +14 , input.select<4,1,32,1>(16,0));

   subtractX.select<20,1,18,1>(0, 0) = input.select<20,1,18,1>(0, 2) - input.select<20,1,18,1>(0, 0);
   subtractY.select<18,1,20,1>(0, 0) = input.select<18,1,20,1>(2, 0) - input.select<18,1,20,1>(0, 0);

   sumX.select<18,1,18,1>(0, 0) = subtractX.select<18,1,18,1>(0, 0) + subtractX.select<18,1,18,1>(1, 0)*2 +
                                       subtractX.select<18,1,18,1>(2, 0);
   sumY.select<18,1,18,1>(0, 0) = subtractY.select<18,1,18,1>(0, 0) + subtractY.select<18,1,18,1>(0, 1)*2 +
                                       subtractY.select<18,1,18,1>(0, 2);

   matrix<uchar, 18, 18> out_high;
   matrix<uchar, 18, 18> out_low;
   matrix<uchar, 16, 16> out_dilate0;
   matrix<uchar, 16, 16> out_and;
   matrix<uchar, 16, 16> out_or;

   CalculateGradient_GENX(sumX, sumY, out_high, out_low);

   Dilate0_GENX(out_high, out_dilate0);

   And_GENX(out_low.select<16,1,16,1>(1,1), out_dilate0, out_and);

   Or_GENX(out_and, out_high.select<16,1,16,1>(1,1), out_or);

   write(DstSI, hout_pos, v_pos, out_or);

}
