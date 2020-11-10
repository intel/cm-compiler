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

// Author: Elaine Wang
#include <cm/cm.h>
#include <cm/cmtl.h>
#include "rgb_cvt_def.h"

const uint ELE_SIZE = 64;
const uint ELE_RGB4 = ELE_SIZE * 4;
const uint ELE_RGB3 = ELE_SIZE * 3;
const uint BLOCK_SIZE = 32;
const uint BLOCK_SIZE_S = BLOCK_SIZE * 4;
const uint BLOCK_SIZE_D = BLOCK_SIZE * 3;
const uint BLOCK_N = 2;
const uint LOOP_N = LOC_SIZE / BLOCK_N / BLOCK_SIZE;
//#define DUMP_DATA
// _GENX_MAIN_ attribute means this function is a kernel entry
// SurfaceIndex ibuf is input surface
// SurfaceIndex obuf is output surface
extern "C" _GENX_MAIN_ void
rgb_cvt(SurfaceIndex ibuf, SurfaceIndex obuf)
{
    vector<char, BLOCK_SIZE_S > v;
    vector<char, BLOCK_SIZE_D> w;
    const uint s_step = LOC_SIZE * 4;
    const uint d_step = LOC_SIZE * 3;
    
#if 0
    if ((get_thread_origin_x() > 0 && get_thread_origin_y() > 0))
        return;
#endif
    // using the following intrinsic instead of using
    // per-thread arguments
    uint s_pos = (get_thread_origin_y() * THREAD_W + get_thread_origin_x()) * s_step;
    uint d_pos = (get_thread_origin_y() * THREAD_W + get_thread_origin_x()) * d_step;

    for (int k = 0; k < LOOP_N; k++) {

        uint es  = s_pos + ELE_RGB4  * k;
        uint ds = d_pos + ELE_RGB3  * k;

#pragma unroll
        for (int t = 0; t < BLOCK_N; t++)
        {
            int s_off = t * BLOCK_SIZE_S;
            int d_off = t * BLOCK_SIZE_D;
            read(ibuf, es + s_off, v.select<BLOCK_SIZE_S, 1>(0));
#pragma unroll
            for (int i = 0; i < BLOCK_SIZE; i++)
            {
                w.select<3,1>(i * 3) = v.select<3,1>(i * 4);
            }

#if 0
            if (get_thread_origin_y() ==4 && get_thread_origin_x() == 3 && k == 0)
            {
                printf("loop %d \n", k);
                for (int x = 0; x < 32; x++)
                {
                    printf("%d ", v[x]);
                }
                printf("\n");
                for (int x = 0; x < 32; x++)
                {
                    printf("%d ", w[x]);
                }
                printf("\n");
            }
#endif

#pragma unroll
            for (int i = 0; i < 3; i++)
            {
                write(obuf, ds + d_off + i * BLOCK_SIZE, w.select<BLOCK_SIZE,1>(i * BLOCK_SIZE));
            }
        }
    }

    return;
}


