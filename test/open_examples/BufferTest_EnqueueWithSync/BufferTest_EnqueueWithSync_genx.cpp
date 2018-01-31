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
BufferTest(SurfaceIndex ibuf, SurfaceIndex obuf)
{
    vector<uchar, 64> in;

    unsigned int h_pos = get_thread_origin_x() * 64;

    read(ibuf, h_pos, in);
	in += 1;

    write(obuf, h_pos, in);
}

extern "C" _GENX_MAIN_ void
BufferTest_GPGPU(SurfaceIndex ibuf, SurfaceIndex obuf)
{
    vector<uchar, 64> in;

    unsigned int h_pos = cm_linear_global_id() * 64;

    read(ibuf, h_pos, in);
    in += 1;

    write(obuf, h_pos, in);
}

