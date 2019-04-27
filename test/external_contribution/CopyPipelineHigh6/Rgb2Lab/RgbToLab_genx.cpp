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

//
//  Convert RGB to LAB format
//
//////////////////////////////////////////////////////////////////////////////////////////
//extern "C" _GENX_MAIN_ _GENX_ROUNDING_MODE_(CM_RTZ) void
extern "C" _GENX_MAIN_ void
RgbToLab_GENX(
       SurfaceIndex SrcRSI,       // Source picture surface index
       SurfaceIndex SrcGSI,       // Source picture surface index
       SurfaceIndex SrcBSI,       // Source picture surface index
       SurfaceIndex DstLSI,       // Destination picture surface index
       SurfaceIndex DstASI,       // Destination picture surface index
       SurfaceIndex DstBSI,       // Destination picture surface index
       vector<float,9> Coeffs,
       vector<float,3> Offsets
     )
{
   vector<short, 2> pos;
   pos(X) = cm_local_id(X) + cm_group_id(X) * cm_local_size(X);
   pos(Y) = cm_local_id(Y) + cm_group_id(Y) * cm_local_size(Y);
   pos(X) = pos(X) * BLKW;
   pos(Y) = pos(Y) * BLKH;

   matrix<uchar, BLKH, BLKW> InR, InG, InB;
   read(SrcRSI, pos(X), pos(Y), InR);
   read(SrcGSI, pos(X), pos(Y), InG);
   read(SrcBSI, pos(X), pos(Y), InB);

   matrix<ushort, BLKH, BLKW> inb = matrix<ushort, BLKH, BLKW>(InB);
   matrix<ushort, BLKH, BLKW> ing = matrix<ushort, BLKH, BLKW>(InG);
   matrix<ushort, BLKH, BLKW> inr = matrix<ushort, BLKH, BLKW>(InR);

   matrix<float, BLKH, BLKW> Rf, Gf, Bf;

   Rf = inr * (1.0f/255.0f);
   Gf = ing * (1.0f/255.0f);
   Bf = inb * (1.0f/255.0f);

   matrix<float, BLKH, BLKW> Xf, Yf, Zf;

   Xf = Rf*Coeffs(0) + Gf*Coeffs(1) + Bf*Coeffs(2);
   Yf = Rf*Coeffs(3) + Gf*Coeffs(4) + Bf*Coeffs(5);
   Zf = Rf*Coeffs(6) + Gf*Coeffs(7) + Bf*Coeffs(8);

   matrix<float, BLKH, BLKW> Xrf, Yrf, Zrf;
   Xrf = Xf * Offsets[0];
   Yrf = Yf * Offsets[1];
   Zrf = Zf * Offsets[2];

   matrix<float, BLKH, BLKW> cubedXrf, cubedYrf, cubedZrf;
   cubedXrf = cm_pow(Xrf, 1.0f/3.0f);
   cubedYrf = cm_pow(Yrf, 1.0f/3.0f);
   cubedZrf = cm_pow(Zrf, 1.0f/3.0f);

   matrix<float, BLKH, BLKW> otherwiseXrf, otherwiseYrf, otherwiseZrf;
   otherwiseXrf = (903.3f*Xrf + 16.0f) * (1.0f/116.0f);
   otherwiseYrf = (903.3f*Yrf + 16.0f) * (1.0f/116.0f);
   otherwiseZrf = (903.3f*Zrf + 16.0f) * (1.0f/116.0f);

   matrix<uchar, BLKH, BLKW> x_mask, y_mask, z_mask;
   x_mask = (Xrf > 0.008865f);
   y_mask = (Yrf > 0.008865f);
   z_mask = (Zrf > 0.008865f);

   matrix<float, BLKH, BLKW> Fxf, Fyf, Fzf;
   Fxf.merge(cubedXrf, otherwiseXrf, x_mask);
   Fyf.merge(cubedYrf, otherwiseYrf, y_mask);
   Fzf.merge(cubedZrf, otherwiseZrf, z_mask);

   matrix<float, BLKH, BLKW> Loutf, Aoutf, Boutf;
   Loutf = (116.0f*Fyf - 16.0f) * 2.55f;
   Aoutf = (500.0f*(Fxf - Fyf)) + 128.0f;
   Boutf = 200.0f*(Fyf - Fzf) + 128.0f;

   matrix<uchar, BLKH, BLKW> Lout(Loutf);
   matrix<uchar, BLKH, BLKW> Aout(Aoutf);
   matrix<uchar, BLKH, BLKW> Bout(Boutf);

   write(DstLSI, pos(X), pos(Y), Lout);
   write(DstASI, pos(X), pos(Y), Aout);
   write(DstBSI, pos(X), pos(Y), Bout);
}
