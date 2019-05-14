/*
***
*** Copyright  (C) 1985-2018 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***----------------------------------------------------------------------------------------
*/

#include <cm/cm.h>

#define BLKH 8
#define BLKW 32
#define BLKWO 16

#define X 0
#define Y 1

static const uchar mask10[32] = { 0xff ,0xc0, 0xff, 0x30, 0xff, 0x0c, 0xff, 0x03, 0xff ,0xc0, 0xff, 0x30, 0xff, 0x0c, 0xff, 0x03,  0xff ,0xc0, 0xff, 0x30, 0xff, 0x0c, 0xff, 0x03, 0xff ,0xc0, 0xff, 0x30, 0xff, 0x0c, 0xff, 0x03 };
static const uchar shift10[32] = { 0, 6, 0, 4, 0, 2, 0, 0 ,0, 6, 0, 4, 0, 2, 0, 0, 0,6, 0, 4, 0, 2, 0, 0 ,0, 6, 0, 4, 0, 2, 0, 0};
static const uchar index10[32] = { 0, 4, 1, 4, 2, 4, 3, 4, 5, 9, 6, 9, 7, 9, 8, 9, 10, 14, 11, 14, 12, 14, 13, 14, 15, 19, 16, 19, 17, 19, 18, 19};
static const uchar mask12[32] = { 0xff ,0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff ,0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff ,0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff ,0x0f, 0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff };
static const uchar shift12[16] = { 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4 };
static const uchar index12[32] = { 0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10, 10, 11, 12, 13, 13, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 22, 22, 23};


inline _GENX_ void
GainOffset(
   matrix_ref<short, BLKH, BLKWO> unpacked,
   matrix_ref<float, BLKH, BLKWO > gain,
   matrix_ref<float, BLKH, BLKWO> offset,
   matrix_ref<float, BLKH, BLKWO> result
)
{
   result = unpacked * gain + offset;
}

inline _GENX_ void
Arrange10(
   matrix_ref<uchar, BLKH, BLKW > in,
   float agc,
   vector_ref<float, BLKWO> gain,
   vector_ref<float, BLKWO> offset,
   matrix_ref<uchar, BLKH, BLKWO> output
)
{
   matrix<short, BLKH, BLKWO> tempor;
   vector<short, 32>msk(mask10);
   vector<uchar, 32>sh(shift10);
   vector<ushort, 32> idx(index10);
   matrix<uchar, BLKH, 32> a;
   vector<uchar, 32> idx2;
   #pragma unroll
   for( int k = 0; k < BLKH; k++)
   {
      idx2 = k;
      a.row(k) = in.iselect(idx2,idx);
   }

   a = a & msk.replicate<BLKH>();;
   a = cm_shr<ushort>(a, sh.replicate<BLKH>());

   matrix<short, BLKH, BLKWO> unpacked = a.format<ushort>();
   matrix<float, BLKH, BLKWO> result;
   vector<float, BLKH* BLKWO> gains = gain.replicate<BLKH>();
   matrix<float, BLKH, BLKWO> gainss = gains;
   vector<float, BLKH* BLKWO> offsets = offset.replicate<BLKH>();
   matrix<float, BLKH, BLKWO> offsetss = offsets;
   GainOffset(unpacked, gainss, offsetss, result);
   result *= agc;
   tempor = cm_rndu<short>(result);

   matrix<uchar, BLKH, BLKWO> block0 = (tempor > 255 );
   tempor.merge(255, tempor, block0);

   matrix<uchar, BLKH, BLKWO> block1 = (tempor < 0);
   output.merge(0, tempor, block1);
}

extern "C" _GENX_MAIN_ void
GainOffset10_GENX(
   SurfaceIndex SrcRSI,
   SurfaceIndex SrcGSI,
   SurfaceIndex SrcBSI,
   SurfaceIndex DstSI,
   float agcR,
   float agcG,
   float agcB,
   SurfaceIndex GainRSI,
   SurfaceIndex GainGSI,
   SurfaceIndex GainBSI,
   SurfaceIndex OffsetRSI,
   SurfaceIndex OffsetGSI,
   SurfaceIndex OffsetBSI,
   int nPicWidth
)
{
   matrix<uchar, BLKH, BLKW> inputR, inputG, inputB;
   matrix<uchar, BLKH, BLKWO> outputR, outputG, outputB;
   vector<float, BLKWO> gainR, gainG, gainB;
   vector<float, BLKWO> offsetR, offsetG, offsetB;
   vector<short, 2> pos;
   short  posXout, posXin, posXgo;

   pos(X) = get_thread_origin_x();
   pos(Y) = get_thread_origin_y();
   posXin = pos(X) * 20;
   pos(Y) = pos(Y) * BLKH;
   posXout = pos(X) * BLKWO*4 ;
   posXgo = pos(X) * BLKWO*4;

   ///////// Channel R //////////         
   read(SrcRSI, posXin, pos(Y), inputR);
   read(GainRSI, posXgo, gainR);
   read(OffsetRSI, posXgo, offsetR);
   matrix<uchar, BLKH, BLKW> inR(inputR);

   Arrange10(inR, agcR, gainR, offsetR, outputR);

   ///////// Channel G //////////
   read(SrcGSI, posXin, pos(Y), inputG);
   read(GainGSI, posXgo, gainG);
   read(OffsetGSI, posXgo, offsetG);
   matrix<uchar, BLKH, BLKW> inG(inputG);

   Arrange10(inG, agcG, gainG, offsetG, outputG);

   ///////// Channel B //////////
   read(SrcBSI, posXin, pos(Y), inputB);
   read(GainBSI, posXgo, gainB);
   read(OffsetBSI, posXgo, offsetB);
   matrix<uchar, BLKH, BLKW> inB(inputB);

   Arrange10(inB, agcB, gainB, offsetB, outputB);

   //////// Write Output ////////   
   matrix<uchar, BLKH, BLKWO*4> Out;
   Out.select<BLKH, 1, BLKWO, 4>(0,3) = 0;
   Out.select<BLKH, 1, BLKWO, 4>(0,2) = outputR;
   Out.select<BLKH, 1, BLKWO, 4>(0,1) = outputG;
   Out.select<BLKH, 1, BLKWO, 4>(0,0) = outputB;

   write(DstSI, posXout, pos(Y), Out.select<8, 1, 32, 1>(0,0));
   write(DstSI, posXout+32, pos(Y), Out.select<8, 1, 32, 1>(0,32));
}

inline _GENX_ void
 GainOffset12(
   matrix_ref<short, BLKH, BLKWO> unpacked,
   matrix_ref<float, BLKH, BLKWO > gain,
   matrix_ref<float, BLKH, BLKWO> offset,
   matrix_ref<float, BLKH, BLKWO> result
 )
 {
   result = unpacked * gain + offset;
 }

inline _GENX_ void
Arrange12(
   matrix_ref<uchar, BLKH, BLKW> in,
   float agc,
   vector_ref<float, BLKWO> gain,
   vector_ref<float, BLKWO> offset,
   matrix_ref<uchar, BLKH, BLKWO> output
)
{
   matrix<short, BLKH, BLKWO> tempor;
   vector<short, BLKWO*2> msk(mask12);
   vector<uchar, BLKWO> sh(shift12);
   vector<ushort, BLKWO*2> idx(index12);
   matrix<uchar, BLKH, BLKWO*2> a;
   matrix<short, BLKH, BLKWO> unpacked;

   #pragma unroll
   for( int k = 0; k < BLKH; k++)
   {
      vector<uchar, 32> idx2 = k;
      a.row(k) = in.iselect(idx2,idx);
   }

   a = a & msk.replicate<BLKH>();
   unpacked = a.format<ushort>();

   unpacked = cm_shr<ushort>(unpacked, sh.replicate<BLKH>());
   matrix<float, BLKH, BLKWO> result;
   vector<float, 128> gains = gain.replicate<BLKH>();
   matrix<float, BLKH, BLKWO> gainss = gains;
   vector<float, 128> offsets = offset.replicate<BLKH>();
   matrix<float, BLKH, BLKWO> offsetss = offsets;
   GainOffset12(unpacked, gainss, offsetss, result);
   result *= agc;
   tempor = cm_rndu<short>(result);

   matrix<uchar, BLKH, BLKWO> block0 = (tempor > 255 );
   tempor.merge(255, tempor, block0);

   matrix<uchar, BLKH, BLKWO> block1 = (tempor < 0);
   output.merge(0, tempor, block1);
}

extern "C" _GENX_MAIN_ void
GainOffset12_GENX(
   SurfaceIndex SrcRSI,
   SurfaceIndex SrcGSI,
   SurfaceIndex SrcBSI,
   SurfaceIndex DstSI,
   float agcR,
   float agcG,
   float agcB,
   SurfaceIndex GainRSI,
   SurfaceIndex GainGSI,
   SurfaceIndex GainBSI,
   SurfaceIndex OffsetRSI,
   SurfaceIndex OffsetGSI,
   SurfaceIndex OffsetBSI,
   int nPicWidth
)
{
   matrix<uchar, BLKH, BLKW> inputR, inputG, inputB;
   matrix<uchar, BLKH, BLKWO> outputR, outputG, outputB;
   vector<float, BLKWO> gainR, gainG, gainB;
   vector<float, BLKWO> offsetR, offsetG, offsetB;
   vector<short, 2> pos;
   short  posXout, posXin, posXgo;

   pos(X) = get_thread_origin_x();
   pos(Y) = get_thread_origin_y();
   posXin = pos(X) * 24;
   pos(Y) = pos(Y) * BLKH;
   posXout = pos(X) * BLKWO*4 ;
   posXgo = pos(X) * BLKWO*4;

   ///////// Channel R //////////
   read(SrcRSI, posXin, pos(Y), inputR);
   read(GainRSI, posXgo, gainR);
   read(OffsetRSI, posXgo, offsetR);
   matrix<uchar, BLKH, BLKW> inR(inputR);
   Arrange12(inR, agcR, gainR, offsetR, outputR);

   ///////// Channel G //////////
   read(SrcGSI, posXin, pos(Y), inputG);
   read(GainGSI, posXgo, gainG);
   read(OffsetGSI, posXgo, offsetG);
   matrix<uchar, BLKH, BLKW> inG(inputG);
   Arrange12(inG, agcG, gainG, offsetG, outputG);

   ///////// Channel B //////////
   read(SrcBSI, posXin, pos(Y), inputB);
   read(GainBSI, posXgo, gainB);
   read(OffsetBSI, posXgo, offsetB);
   matrix<uchar, BLKH, BLKW> inB(inputB);
   Arrange12(inB, agcB, gainB, offsetB, outputB);

   //////// Write Output ////////
   matrix<uchar, BLKH, BLKWO*4> Out;
   Out.select<BLKH, 1, BLKWO, 4>(0,3) = 0;
   Out.select<BLKH, 1, BLKWO, 4>(0,2) = outputR;
   Out.select<BLKH, 1, BLKWO, 4>(0,1) = outputG;
   Out.select<BLKH, 1, BLKWO, 4>(0,0) = outputB;

   write(DstSI, posXout, pos(Y), Out.select<8, 1, 32, 1>(0,0));
   write(DstSI, posXout+32, pos(Y), Out.select<8, 1, 32, 1>(0,32));
}
