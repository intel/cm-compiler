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
#define NUM_BINS 256
#define SLM_SIZE NUM_BINS * 4
#define BLOCK_WIDTH  32
#define BLOCK_HEIGHT 512

static const ushort init_offset[16] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
static const uint init_offset2[8] = { 0,1,2,3,4,5,6,7 };

// Histogram kernel: computes the distribution of pixel intensities
//
// SurfaceIndex INBUF: input surface index
// SurfaceIndex OUTBUF: output surface index
//
extern "C" _GENX_MAIN_ void
histogram_atomic(SurfaceIndex INBUF, SurfaceIndex OUTBUF)
{
    // Get thread origin offsets
    uint h_pos = cm_group_id(0) * BLOCK_WIDTH;
    uint v_pos = cm_group_id(1) * BLOCK_HEIGHT;

    // Declare and initialize SLM
    cm_slm_init(SLM_SIZE);
    uint slm_bins = cm_slm_alloc(SLM_SIZE);
    vector<uint, 16> slm_offset(init_offset);
    slm_offset *= 4;
    vector<uint, 64> slm_data = 0;
#pragma unroll
    for (int i = 0; i < NUM_BINS; i += 64) {
      cm_slm_write4(slm_bins, slm_offset, slm_data, SLM_ABGR_ENABLE);
      slm_offset += 64;
    }

    // Each thread handles BLOCK_HEIGHTxBLOCK_WIDTH pixel block
    for (int y = 0; y < BLOCK_HEIGHT / 8; y++) {
        // Declare a 8x32 uchar matrix to store the input block pixel value
        matrix<uchar, 8, 32> in;

        // Perform 2D media block read to load 8x32 pixel block
        read(INBUF, h_pos, v_pos, in);
#pragma unroll
        for (int i = 0; i < 8; i += 1) {
#pragma unroll
          for (int j = 0; j < 32; j += 16) {
            // Accumulate local histogram for each pixel value
            vector<ushort, 16> data = in.select<1, 1, 16, 1>(i, j);
            vector<uint, 16>   slm_AtomicResult;
            cm_slm_atomic(slm_bins, ATOMIC_INC, data, slm_AtomicResult);
          }
        }

        // Update starting offset for the next work block
        v_pos += 8;
    }

    // Update global sum by atomically adding each local histogram
    vector<uint, NUM_BINS> local_histogram;
    vector<ushort, 16> local_offset(init_offset);
#pragma unroll
    for (int i = 0; i < NUM_BINS; i += 16) {
      cm_slm_read(slm_bins, local_offset, local_histogram.select<16, 1>(i));
      local_offset += 16;
    }

    vector<uint, 8> offset(init_offset2);
    vector<uint, 8> src, ret;
#pragma unroll
    for(int i = 0; i < NUM_BINS; i += 8) {
        src = local_histogram.select<8,1>(i);
        write_atomic<ATOMIC_ADD>(OUTBUF, offset, src, ret);
        offset += 8;
    }
}
