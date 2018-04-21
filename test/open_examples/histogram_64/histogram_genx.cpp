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

#include "cm/cm.h"

// Assuming grey-scale input image, with pixel value in the range of 0-63
// Set the number of histogram bins as 64
#define NUM_BINS 64
#define BLOCK_WIDTH  32
#define BLOCK_HEIGHT 64

const uint init_0_7[8] = { 0,1,2,3,4,5,6,7 };

// Histogram kernel: computes the distribution of pixel intensities
//
// SurfaceIndex INBUF: input surface index
// SurfaceIndex OUTBUF: output surface index
// uint total_num_threads: total number of threads
// uint max_h_pos: upper bound of horizontal block position
// uint max_v_pos: upper bound of vertical block position
//
extern "C" _GENX_MAIN_ void
histogram_atomic(SurfaceIndex INBUF, SurfaceIndex OUTBUF)
{
    // Get thread origin offsets
    uint h_pos = get_thread_origin_x() * BLOCK_WIDTH;
    uint v_pos = get_thread_origin_y() * BLOCK_HEIGHT;

    // Declare a 8x32 uchar matrix to store the input block pixel value
    matrix<uchar, 8, 32> in;

    // Declare a vector to store the local histogram
    vector<int, NUM_BINS>  histogram;

    // Declare a vector to store the source for atomic write operation
    vector<uint, 8> src;

    // Declare a vector to store the offset for atomic write operation
    vector<uint, 8> offset(init_0_7);

    // Declare a vector to store the return value from atomic write operation
    vector<uint, 8> ret;

    // Initialize other local varaibles
    int  i, j, sum = 0;
    histogram = 0;

    // Each thread handles 64x32 pixel block
    for (int y = 0; y < BLOCK_HEIGHT / 8; y++) {
        // Perform 2D media block read to load 8x32 pixel block
        read(INBUF, h_pos, v_pos, in);

        // Accumulate local histogram for each pixel value
#pragma unroll
        for (i = 0; i < 8; i++) {
#pragma unroll
          for (j = 0; j < 32; j++) {
            histogram(in(i, j) >> 2) += 1;
          }
        }

        // Update starting offset for the next work block
        v_pos += 8;
    }

    // Update global sum by atomically adding each local histogram
#pragma unroll
    for(i = 0; i < NUM_BINS; i += 8) {
        src =  histogram.select<8,1>(i);
#ifdef __ICL
        write<uint, 8>(OUTBUF, ATOMIC_ADD, i, offset, src, ret);
#else
        write_atomic<ATOMIC_ADD>(OUTBUF, offset, src, ret);
        offset += 8;
#endif
    }
}
