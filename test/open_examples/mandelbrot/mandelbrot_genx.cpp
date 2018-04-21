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

_GENX_MAIN_ void
mandelbrot(SurfaceIndex output_index, int crunch,
           float xOff, float yOff, float scale)
{
    int ix_start = cm_group_id(0) * 8;
    int iy_start = cm_group_id(1) * 2;

    cm_vector(vn, int, 8, 0, 1);
    matrix<int, 2, 8> ix;
    ix.row(0) = ix_start + vn;
    ix.row(1) = ix_start + vn;
    matrix<int, 2, 8> iy;
    iy.row(0) = iy_start;
    iy.row(1) = iy_start + 1;

    matrix<float, 2, 8> xPos = ix * scale + xOff;
    matrix<float, 2, 8> yPos = iy * scale + yOff;
    matrix<float, 2, 8> x = 0.0f;
    matrix<float, 2, 8> y = 0.0f;
    matrix<float, 2, 8> xx = 0.0f;
    matrix<float, 2, 8> yy = 0.0f;
    matrix<int, 2, 8>  m = 0;

    SIMD_DO_WHILE_BEGIN {
        y  = x * y * 2.0f + yPos;
        x  = xx - yy + xPos;
        yy = y * y;
        xx = x * x;
        m += 1;
    } SIMD_DO_WHILE_END ((m < crunch) & (xx + yy < 4.0f));

    matrix<int, 2, 8> color =
        ((((m << 4) - m) & 0xff)      ) +
        ((((m << 3) - m) & 0xff) << 8 ) +
        ((((m << 2) - m) & 0xff) << 16);

    // because the output is a y-tile 2D surface
    // we can only write 32-byte wide 
    write(output_index,
         ix_start * sizeof(int), iy_start,
         color);
}
