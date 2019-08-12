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
YCbCr2Rgb_GENX (
      SurfaceIndex SrcSIYCbCr,
      SurfaceIndex DstSIRGB,
      unsigned int picHeight,
      vector<float, 4> Coeffs,
      vector<float, 3> Offsets
      )
{
   vector<short, 2> pos;

   pos(X) = get_thread_origin_x() * BLKW;
   pos(Y) = get_thread_origin_y() * BLKH;

   matrix<uchar, BLKH, BLKW> inY;
   matrix<uchar, BLKH, BLKW> inCb;
   matrix<uchar, BLKH, BLKW> inCr;

   /* YCbCr output surface from JPEG decoder contains single 2D surface 
    * with 3 planars of Y, U, and V arrange like below.  To read each plane,
    * use the read_plane function. 
    *
    * void read_plane(SurfaceIndex IND, CmSurfacePlaneIndex plane_index, int X,
    * int Y, matrix_ref m);
    *
    * 
    *               width
    *            -----------
    *  0         |    Y    |
    *            -----------
    *  height    |    U    |
    *            -----------
    *  height*2  |    V    |
    *            -----------
    */

   read_plane(SrcSIYCbCr, GENX_SURFACE_Y_PLANE, pos(X), pos(Y), inY);
   read_plane(SrcSIYCbCr, GENX_SURFACE_U_PLANE, pos(X), pos(Y), inCb);
   read_plane(SrcSIYCbCr, GENX_SURFACE_V_PLANE, pos(X), pos(Y), inCr);

   matrix<float, BLKH, BLKW> r, g, b;

   matrix<float, BLKH, BLKW> inCr_offset_1 = inCr - Offsets(1);
   matrix<float, BLKH, BLKW> inCb_offset_2 = inCb - Offsets(2);

   r = inY + Coeffs(0) * inCr_offset_1;
   g = inY + Coeffs(1) * inCb_offset_2 + Coeffs(2) * inCr_offset_1;
   b = inY + Coeffs(3) * inCb_offset_2;

   matrix<uchar, BLKH, BLKW*4> rgb;

   rgb.select<BLKH, 1, BLKW, 4>(0,0) = cm_rndu<uchar>(b, SAT);
   rgb.select<BLKH, 1, BLKW, 4>(0,1) = cm_rndu<uchar>(g, SAT);
   rgb.select<BLKH, 1, BLKW, 4>(0,2) = cm_rndu<uchar>(r, SAT);
   rgb.select<BLKH, 1, BLKW, 4>(0,3) = 0xff;

   /* Generate a 32bit RGBA interleave output */
   write(DstSIRGB, pos(X)*4, pos(Y), rgb);
}
