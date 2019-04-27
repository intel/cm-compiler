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

#define BLKW 16
#define BLKH 8

#define X 0
#define Y 1

inline _GENX_ void RemoveFringe_GENX(
      SurfaceIndex LtoKSI,
      matrix_ref<uchar, BLKH, BLKW> inputL,
      matrix_ref<uchar, BLKH, BLKW> inputNE,
      matrix_ref<uchar, BLKH, BLKW*4> inputCMYK,
      matrix_ref<uchar, BLKH, BLKW*4> outputCMYK,
      matrix_ref<uchar, BLKH, BLKW> outputK
      )
{
   vector<uint, BLKH*BLKW> lightInS(inputL);
   vector<uchar, BLKH*BLKW> koutPixel;

   #pragma unroll
   for (int i = 0; i < 8; i++)
      read(LtoKSI, 0, lightInS.select<16,1>(i*16), koutPixel.select<16,1>(i*16));

   matrix<uchar, BLKH, BLKW*4> edgeResult = 0;
   vector<uchar, BLKH*BLKW*4> neMask = 0;
   vector_ref<uchar, BLKH*BLKW> neIn = inputNE.format<uchar>();

   #pragma unroll
   for (int i = 0; i < 128; i++)
      neMask.select<4, 1>(i*4) = neIn.replicate<4,1>(i);

   edgeResult.select<BLKH, 1, BLKW, 4>(0,3) = koutPixel;

   outputCMYK.merge(edgeResult, inputCMYK, neMask);

   outputK = koutPixel;
}

//
//  Convert LAB to CMYK + Remove fringe combination kernels
//
//////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
CmykRF_GENX(
      SurfaceIndex SrcSIL,          // Source picture surface index
      SurfaceIndex SrcSIA,          // Source picture surface index
      SurfaceIndex SrcSIB,          // Source picture surface index
      SurfaceIndex SrcSINE,
      SurfaceIndex DstSICMYK,       // Destination picture surface index
      SurfaceIndex DstSIK,          // Destination picture surface index
      SurfaceIndex XMapSI,          // X Mapping surface index  //256 float
      SurfaceIndex YMapSI,          // Y Mapping surface index  //256 float
      SurfaceIndex ZMapSI,          // Z Mapping surface index  //256 float
      SamplerIndex SamplerSI,        // Sampler state  index
      float norm,
      SurfaceIndex LtoKSI,
      SurfaceIndex LatticePointSI  // Destination picture surface index
     )
{

   vector<short, 2> pos, pos4;

   pos(X) = cm_local_id(X) + cm_group_id(X) * cm_local_size(X);
   pos(Y) = cm_local_id(Y) + cm_group_id(Y) * cm_local_size(Y);

   pos(X) = pos(X) << 4;
   pos(Y) = pos(Y) << 3;

   pos4(X) = pos(X) << 2;
   pos4(Y) = pos(Y);

   matrix<uchar, BLKH, BLKW> inputL;
   matrix<uchar, BLKH, BLKW> inputA;
   matrix<uchar, BLKH, BLKW> inputB;

   // Read Input
   read(SrcSIL, pos(X), pos(Y), inputL);
   read(SrcSIA, pos(X), pos(Y), inputA);
   read(SrcSIB, pos(X), pos(Y), inputB);

   vector<uint, BLKH*BLKW> inL(inputL);
   vector<uint, BLKH*BLKW> inA(inputA);
   vector<uint, BLKH*BLKW> inB(inputB);

   vector<float, BLKW*BLKH> coordX, coordY, coordZ;

#pragma unroll
   for (int i=0; i < 8; i++)
   {
      read(XMapSI, 0, inL.select<16,1>(i*16), coordX.select<16,1>(i*16));
      read(YMapSI, 0, inA.select<16,1>(i*16), coordY.select<16,1>(i*16));
      read(ZMapSI, 0, inB.select<16,1>(i*16), coordZ.select<16,1>(i*16));
   }

   coordX += 0.5f;
   coordY += 0.5f;
   coordZ += 0.5f;

   // Divide with # of Lattice Points
   // OpenCL support normalized but not CM
   // float norm = 1.0f/(17.0f);

   coordX *= norm;
   coordY *= norm;
   coordZ *= norm;


   matrix<uchar, 8, 16> cout, mout, yout, kout;

   /* Use Sampler to do Trilinear interpolation 
    * Sampled Output format */
   /* X0X1X2
    * B0B1B2
    * G0G1G2
    * R0R1R2 
    */
   #pragma unroll
   for (int j=0; j <2; j++)
   {
      matrix<float, 16, 16> sampledData;
      matrix<float, 4, 16> c, m, y, k;

      #pragma unroll
      for (int i=0; i<4; i++)
         sample16(sampledData.select<4,1,16,1>(i*4,0), CM_ABGR_ENABLE, LatticePointSI, SamplerSI,
            coordZ.select<16,1>((j*4 + i)*16), coordY.select<16,1>((j*4 + i)*16), coordX.select<16,1>((j*4 + i)*16));

      sampledData *= 255.0f;
      c = sampledData.select<4,4,16,1>(0,0);
      m = sampledData.select<4,4,16,1>(1,0);
      y = sampledData.select<4,4,16,1>(2,0);
      k = sampledData.select<4,4,16,1>(3,0);

      cout.select<4,1,16,1>(j*4, 0) = c;
      mout.select<4,1,16,1>(j*4, 0) = m;
      yout.select<4,1,16,1>(j*4, 0) = y;
      kout.select<4,1,16,1>(j*4, 0) = k;
   }


   matrix<uchar, BLKH, BLKW*4> lab2cmykout;
   lab2cmykout.select<BLKH, 1, BLKW, 4>(0,3) = kout;
   lab2cmykout.select<BLKH, 1, BLKW, 4>(0,2) = yout;
   lab2cmykout.select<BLKH, 1, BLKW, 4>(0,1) = mout;
   lab2cmykout.select<BLKH, 1, BLKW, 4>(0,0) = cout;

   matrix<uchar, BLKH, BLKW*4> fringecmykout;
   matrix<uchar, BLKH, BLKW> fringekout;
   matrix<uchar, BLKH, BLKW> inputNE;

   read(SrcSINE, pos(X), pos(Y), inputNE);

   RemoveFringe_GENX(LtoKSI, inputL, inputNE, lab2cmykout, fringecmykout,
         fringekout);

   write(DstSICMYK, pos4(X), pos4(Y), fringecmykout.select<8,1,32,1>(0,0));
   write(DstSICMYK, pos4(X)+32, pos4(Y), fringecmykout.select<8,1,32,1>(0,32));
   write(DstSIK, pos(X), pos(Y), fringekout);

}
