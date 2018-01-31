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


// Assuming grey-scale input image, with pixel value in the range of 0-255
// Set the number of histogram bins as 256
#define NUM_BINS 256

// Histogram kernel: computes the distribution of pixel intensities
//
// SurfaceIndex INBUF: input surface index
// SurfaceIndex OUTBUF: output surface index
// uint total_num_threads: total number of threads
// uint max_h_pos: upper bound of horizontal block position
// uint max_v_pos: upper bound of vertical block position
//
extern "C" _GENX_MAIN_ void
histogram_atomic(SurfaceIndex INBUF, SurfaceIndex OUTBUF,
                 uint total_num_threads, uint max_h_pos, uint max_v_pos)
{
    // Get current thread id
    uint id = get_thread_origin_x();

    // Declare a 8x32 uchar matrix to store the input block pixel value
    matrix<uchar, 8, 32> in;

    // Declare a vector to store the local histogram
    vector<int, NUM_BINS>  histogram;

    // Declare a vector to store the source for atomic write operation
    vector<uint, 8> src;

    // Declare a vector to store the offset for atomic write operation
    vector<uint, 8> offset;

    // Declare a vector to store the return value from atomic write operation
    vector<uint, 8> ret;

    // Initialize other local varaibles
    int  i, j, sum = 0;
    histogram = 0;
    int h_pos = id;
    int v_pos = 0;

    // Each thread process a column of 8x32 pixel blocks
    while (1) {

      // Adjust the starting offsets of current block
        while (h_pos >= max_h_pos) {
            v_pos++;
            h_pos -= max_h_pos;
        }

        // Stop current thread if no more blocks to process
        if ((h_pos >= max_h_pos) || (v_pos >= max_v_pos)) {
            break;
        }

        // Perform 2D media block read to load 8x32 pixel block
        read(INBUF, h_pos*32, v_pos*8, in);

        // Accumulate local histogram for each pixel value
#pragma unroll
        for (i = 0; i < 8; i++) {
#pragma unroll
            for (j = 0; j < 32; j++) {
                histogram(in(i,j)) += 1;
            }
        }

        // Update starting offset for the next work block
        h_pos += total_num_threads;
    }

    // Initialize the offset for atomic write operation
#pragma unroll
    for(i = 0; i < 8; i ++){
        offset(i) = i;
    }

    // Update global sum by atomically adding each local histogram
#pragma unroll
    for(i = 0; i < 256; i += 8) {
        src =  histogram.select<8,1>(i);
#ifdef __ICL
        write<uint, 8>(OUTBUF, ATOMIC_ADD, i, offset, src, ret);
#else
        write_atomic<ATOMIC_ADD>(OUTBUF, offset, src, ret);
        offset += 8;
#endif
    }
}
