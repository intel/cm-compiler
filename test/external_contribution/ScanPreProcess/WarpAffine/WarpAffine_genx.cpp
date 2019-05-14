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

/*********************************************************************************************************************************/

#define BLKW   16
#define BLKH   8

#define X   0
#define Y   1

#define INTER_BITS   5
#define INTER_TAB_SIZE  (1 << INTER_BITS)
#define INTER_SCALE  1.f/INTER_TAB_SIZE
#define AB_BITS      cm_max<int>(10, (int)INTER_BITS)
#define AB_SCALE  (1 << AB_BITS)
#define ROUND_DELTA  (1 << (AB_BITS - INTER_BITS - 1))

extern "C" _GENX_MAIN_  void
WarpAffine_GENX(
   SamplerIndex SAMPLER_IDX,
   SurfaceIndex INBUF_SAMPLER_IDX,
   SurfaceIndex DstRSI,
   SurfaceIndex DstGSI,
   SurfaceIndex DstBSI,
   SurfaceIndex COEFF_IDX,
   matrix<float, 3, 2> coeffs,
   float coord_unit_u,
   float coord_unit_v)
{
   vector<short, 2> pos;
   short posXout;

   pos(X) = get_thread_origin_x(); 
   pos(Y) = get_thread_origin_y();
   short posXin = pos(X) * BLKW;
   pos(Y) = pos(Y) * BLKH;
   posXout = pos(X) * 16;

   vector<float, 3> MXdata = coeffs.select<3,1,1,1>(0,0);
   vector<float, 3> MYdata = coeffs.select<3,1,1,1>(0,1);

   vector<int, 16> posXindex;
   cmtl::cm_vector_assign(posXindex.select<16,1>(0), posXin, 1);
   vector<int, 16> tmp = posXindex << AB_BITS;

   vector<int, 16> X0_ = cm_rnde<int>(MXdata[0] * tmp);
   vector<int, 16> Y0_ = cm_rnde<int>(MYdata[0] * tmp);
   matrix<uchar, 8, 64> outc = 0;

   for (int j=0; j < BLKH; j++)
   {
      int xAbFactor = cm_rnde<int>((MXdata[1] * (pos(Y)+j) + MXdata[2]) * AB_SCALE) + ROUND_DELTA;
      int yAbFactor = cm_rnde<int>((MYdata[1] * (pos(Y)+j) + MYdata[2]) * AB_SCALE) + ROUND_DELTA;

      vector<int, 16> X0 = X0_ + xAbFactor;
      vector<int, 16> Y0 = Y0_ + yAbFactor;

      X0 = X0 >> (AB_BITS - INTER_BITS);
      Y0 = Y0 >> (AB_BITS - INTER_BITS);

      vector<short, 16> sx = vector<short, 16>((X0 >> INTER_BITS),SAT) - 1;
      vector<short, 16> sy = vector<short, 16>((Y0 >> INTER_BITS), SAT) - 1;
      vector<uint, 16> ax = vector<uint, 16>((X0 & (INTER_TAB_SIZE-1)));
      vector<uint, 16> ay = vector<uint, 16>((Y0 & (INTER_TAB_SIZE-1)));

      matrix<float, 4, 16> coeffsX, coeffsY;

      read_untyped(COEFF_IDX, CM_ABGR_ENABLE, coeffsX, ax*4);
      read_untyped(COEFF_IDX, CM_ABGR_ENABLE, coeffsY, ay*4);

      vector<float, 16> uCoord;
      vector<float, 16> vCoord;

      uCoord = sx;

      matrix<float, 4, 16> uCoord16, vCoord16;
      uCoord16.row(0)   = uCoord.select<16,1>(0);
      uCoord16.row(1)   = uCoord.select<16,1>(0)+1;
      uCoord16.row(2)   = uCoord.select<16,1>(0)+2;
      uCoord16.row(3)   = uCoord.select<16,1>(0)+3;
      uCoord16 *= coord_unit_u;

      matrix<float,4, 16> xsum = 0.0f;
      matrix<float,4, 16> sum = 0.0f;

      #pragma unroll
      for (int k=0; k<4; k++)
      {
         matrix<float, 4, 16> sampledData;
         vCoord  = sy + k;

         vector<float, 16> vCoordnom = (vCoord)*coord_unit_v;

         sample16(sampledData, CM_BGR_ENABLE, INBUF_SAMPLER_IDX, SAMPLER_IDX, uCoord16.row(0), vCoordnom);
         matrix<float, 4, 16> CX = coeffsX.row(0).replicate<4>();
         sampledData = cm_mul<float>(sampledData, CX);
         xsum += sampledData;

         sample16(sampledData, CM_BGR_ENABLE, INBUF_SAMPLER_IDX, SAMPLER_IDX, uCoord16.row(1), vCoordnom);
         CX = coeffsX.row(1).replicate<4>();
         sampledData = cm_mul<float>(sampledData, CX);
         xsum += sampledData;

         sample16(sampledData, CM_BGR_ENABLE, INBUF_SAMPLER_IDX, SAMPLER_IDX, uCoord16.row(2), vCoordnom);
         CX = coeffsX.row(2).replicate<4>();
         sampledData = cm_mul<float>(sampledData, CX);
         xsum += sampledData;

         sample16(sampledData, CM_BGR_ENABLE, INBUF_SAMPLER_IDX, SAMPLER_IDX, uCoord16.row(3), vCoordnom);
         CX = coeffsX.row(3).replicate<4>();
         sampledData = cm_mul<float>(sampledData, CX);
         xsum += sampledData;

         matrix<float, 4, 16> CY = coeffsY.row(k).replicate<4>();
         sum += cm_mul<float>(xsum, CY);

         xsum = 0.0f;
      }        // for k loop

      sum *= 255.0f;
      sum = matrix<uchar,4,16> (sum, SAT);

      outc.select<1,1,16,4>(j,0) = sum.row(2);
      outc.select<1,1,16,4>(j,1) = sum.row(1);
      outc.select<1,1,16,4>(j,2) = sum.row(0);
      outc.select<1,1,16,4>(j,3) = 0;
   }

   matrix<uchar, BLKH, BLKW> R, G, B, O;

   O = outc.select<BLKH, 1, BLKW, 4>(0,3);
   R = outc.select<BLKH, 1, BLKW, 4>(0,2);
   G = outc.select<BLKH, 1, BLKW, 4>(0,1);
   B = outc.select<BLKH, 1, BLKW, 4>(0,0);

   write(DstRSI, posXout, pos(Y), R);
   write(DstGSI, posXout, pos(Y), G);
   write(DstBSI, posXout, pos(Y), B);
}
