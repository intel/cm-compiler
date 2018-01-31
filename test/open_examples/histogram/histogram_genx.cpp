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

// Histogram kernel: computes the distribution of pixel intensities
// Assuming grey-scale input image, with pixel value in the range of 0-255

// Set the number of histogram bins as 256
#define NUM_BINS    256

// Stage-1: computes the local histogram of 8x32 pixel blocks
//
// SurfaceIndex INBUF: input surface index
// SurfaceIndex OUTBUF: output surface index
// uint id: current histogram thread id
// uint total_num_threads: total number of threads
// uint max_h_pos: upper bound of horizontal block position
// uint max_v_pos: upper bound of vertical block position
//
extern "C" _GENX_MAIN_ void
histogram(SurfaceIndex INBUF, SurfaceIndex OUTBUF, uint total_num_threads, uint max_h_pos, uint max_v_pos)
{
    // Declare a 8x32 uchar matrix to store the input block pixel value
    matrix<uchar, 8, 32> in;
    // Declare a vector to store the local histogram
    vector<int, NUM_BINS> histogram;

    // Initialize local variables
    histogram = 0;
    int i, j, sum = 0;
    int id = get_thread_origin_x();
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
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 32; j+=8) {
                histogram(in(i,j)) += 1;
                histogram(in(i,j+1)) += 1;
                histogram(in(i,j+2)) += 1;
                histogram(in(i,j+3)) += 1;
                histogram(in(i,j+4)) += 1;
                histogram(in(i,j+5)) += 1;
                histogram(in(i,j+6)) += 1;
                histogram(in(i,j+7)) += 1;
            }
        }

        // Update starting offset for the next work block
        h_pos += total_num_threads;
    }

    // Set starting offset for saving local histogram in output buffer
    int init_offset = id*32;

    // Perform 2D media block write to save local histogram
    write(OUTBUF, 0, init_offset, histogram.select<64,1>(0).format<int,8,8>());
    write(OUTBUF, 0, init_offset+8, histogram.select<64,1>(64).format<int,8,8>());
    write(OUTBUF, 0, init_offset+16, histogram.select<64,1>(128).format<int,8,8>());
    write(OUTBUF, 0, init_offset+24, histogram.select<64,1>(192).format<int,8,8>());
}

// Stage-2: performs summation of a subset of the local histograms
//
// SurfaceIndex OUTBUF: surface index for ouptut from stage-1
// uint id: current sum thread id
// uint total_num_threads: total number of sum threads
// uint num_histograms: total number of histograms computed in stage-1
//
extern "C" _GENX_MAIN_ void
sum(SurfaceIndex OUTBUF, uint total_num_threads, uint num_histograms)
{
    // Declare a vector to store the sum of local histograms
    vector<int, NUM_BINS> histogram;

    // Set starting offset for the first saved local histogram
    int id = get_thread_origin_x();
    int init_offset = id*32;

    // Perform 2D media block read to load local histogram
    read(OUTBUF, 0, init_offset, histogram.select<64,1>(0).format<int,8,8>());
    read(OUTBUF, 0, init_offset+8, histogram.select<64,1>(64).format<int,8,8>());
    read(OUTBUF, 0, init_offset+16, histogram.select<64,1>(128).format<int,8,8>());
    read(OUTBUF, 0, init_offset+24, histogram.select<64,1>(192).format<int,8,8>());

    // Set the id for the next histogram to be processed
    int next = id+total_num_threads;
    while (next < num_histograms) {
        // Declare a vector to store the next local histograms
        vector<int, NUM_BINS> histogram2;

        // Set starting offset for the next saved local histogram
        int next_offset = next*32;
        read(OUTBUF, 0, next_offset, histogram2.select<64,1>(0).format<int,8,8>());
        read(OUTBUF, 0, next_offset+8, histogram2.select<64,1>(64).format<int,8,8>());
        read(OUTBUF, 0, next_offset+16, histogram2.select<64,1>(128).format<int,8,8>());
        read(OUTBUF, 0, next_offset+24, histogram2.select<64,1>(192).format<int,8,8>());

        // Update the sume for local histograms
        histogram += histogram2;

        // Update the id for the next histogram to be processed
        next += total_num_threads;
    }

    // Perform 2D media block write to save the sum computed by current thread
    write(OUTBUF, 0, init_offset, histogram.select<64,1>(0).format<int,8,8>());
    write(OUTBUF, 0, init_offset+8, histogram.select<64,1>(64).format<int,8,8>());
    write(OUTBUF, 0, init_offset+16, histogram.select<64,1>(128).format<int,8,8>());
    write(OUTBUF, 0, init_offset+24, histogram.select<64,1>(192).format<int,8,8>());
}

// Stage-3: performs final summation of all local histogram sums
//
// SurfaceIndex OUTBUF: surface index for ouptut from stage-2
// uint total_num_threads: total number of sum threads
//
extern "C" _GENX_MAIN_ void
final_sum(SurfaceIndex OUTBUF, uint total_num_threads)
{
    // Declare a vector to store the sum of all histograms
    vector<int, NUM_BINS> histogram;

    // Perform 2D media block read to load the first local sum
    read(OUTBUF, 0, 0, histogram.select<64,1>(0).format<int,8,8>());
    read(OUTBUF, 0, 8, histogram.select<64,1>(64).format<int,8,8>());
    read(OUTBUF, 0, 16, histogram.select<64,1>(128).format<int,8,8>());
    read(OUTBUF, 0, 24, histogram.select<64,1>(192).format<int,8,8>());

    // Set the id for the next local sum to be processed
    int next = 1;
    while (next < total_num_threads) {
        // Declare a vector to store the next local sum
        vector<int, NUM_BINS> histogram2;

        // Set starting offset for the next saved local sum
        int next_offset = next*32;

        // Perform 2D media block read to load the next local sum
        read(OUTBUF, 0, next_offset, histogram2.select<64,1>(0).format<int,8,8>());
        read(OUTBUF, 0, next_offset+8, histogram2.select<64,1>(64).format<int,8,8>());
        read(OUTBUF, 0, next_offset+16, histogram2.select<64,1>(128).format<int,8,8>());
        read(OUTBUF, 0, next_offset+24, histogram2.select<64,1>(192).format<int,8,8>());

        // Update the final sum
        histogram += histogram2;

        // Update the id for the next histogram to be processed
        next++;
    }

    // Perform 2D media block write to save the final sum of all histograms
    write(OUTBUF, 0, 0, histogram.select<64,1>(0).format<int,8,8>());
    write(OUTBUF, 0, 8, histogram.select<64,1>(64).format<int,8,8>());
    write(OUTBUF, 0, 16, histogram.select<64,1>(128).format<int,8,8>());
    write(OUTBUF, 0, 24, histogram.select<64,1>(192).format<int,8,8>());
}

