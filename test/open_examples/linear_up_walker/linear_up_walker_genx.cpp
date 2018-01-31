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

extern "C" _GENX_MAIN_ void
linear(SurfaceIndex ibuf, SurfaceIndex obuf)
{
    matrix<uchar, 8, 32> in;
    matrix<uchar, 6, 24> out;
    matrix<float, 6, 24> m;

    uint h_pos = get_thread_origin_x();
    uint v_pos = get_thread_origin_y();

    read(ibuf, h_pos*24, v_pos*6, in);

    m  = in.select<6,1,24,1>(1,3);

    m += in.select<6,1,24,1>(0,0);
    m += in.select<6,1,24,1>(0,3);
    m += in.select<6,1,24,1>(0,6);

    m += in.select<6,1,24,1>(1,0);
    m += in.select<6,1,24,1>(1,6);

    m += in.select<6,1,24,1>(2,0);
    m += in.select<6,1,24,1>(2,3);
    m += in.select<6,1,24,1>(2,6);

    out = m * 0.111f;

    write(obuf, h_pos*24, v_pos*6, out);
}
