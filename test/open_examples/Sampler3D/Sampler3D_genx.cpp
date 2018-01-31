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

/*----------------------------------------------------------------------*/

#define BLOCK_PIXEL_WIDTH (8)
#define BLOCK_HEIGHT (2)

// samplerExample: sampler message usage example
//
// SamplerIndex SAMPLER_IDX: sampler state index
// SurfaceIndex INBUF_SAMPLER_IDX: sampler surface index
// SurfaceIndex OUTBUF_IDX: output surface index
// float coord_unit_u: multiply coefficient for U coordinate
// float coord_unit_v: multiply coefficient for V coordinate
//
extern "C" _GENX_MAIN_  void
samplerExample(SamplerIndex SAMPLER_IDX,
    SurfaceIndex INBUF_SAMPLER_IDX,
    SurfaceIndex OUTBUF_IDX,
    float coord_unit_u,
    float coord_unit_v) {
    // Declares a vector with 16 U coordinates
    vector<float, 16> uCoord;
    // Declares a vector with 16 V coordinates
    vector<float, 16> vCoord;

    // Computes the horizontal offset of current thread
    int horizOffset = get_thread_origin_x() * BLOCK_PIXEL_WIDTH;
    // Compute the vertical offset of current thread
    int vertOffset = get_thread_origin_y() * BLOCK_HEIGHT;

    // Initializes coordinates
    for (int i = 0; i < 8; i++) {
        uCoord(i) = i;
        uCoord(i + 8) = i;
    }

    // Adjusts U coordinates based on horizontal offset and coefficient
    uCoord += horizOffset;
    uCoord *= coord_unit_u;

    // Adjusts V coordinates based on vertical offset and coefficient
    vCoord.select<8, 1>() = vertOffset;
    vCoord.select<8, 1>(8) = vertOffset + 1.0f;
    vCoord *= coord_unit_v;

    // Declares a 4x16 float matrix to store sampled data
    matrix<float, 4, 16> sampledData;

    // Performs sample16 operation for the given sampler surface, state
    // and coordinates
    sample16(sampledData, CM_BGR_ENABLE, INBUF_SAMPLER_IDX, SAMPLER_IDX, uCoord,
        vCoord);

    // Scales up by 255
    sampledData *= 255;

    // Declares a 2X32 uchar matrix to store output data
    matrix<uchar, BLOCK_HEIGHT, BLOCK_PIXEL_WIDTH * 4> outData = 0;

    // Computes output data for R channel
    outData.select<BLOCK_HEIGHT, 1, BLOCK_PIXEL_WIDTH, 4>(0, 2) =
        sampledData.row(0);

    // Computes output data for G channel
    outData.select<BLOCK_HEIGHT, 1, BLOCK_PIXEL_WIDTH, 4>(0, 1) =
        sampledData.row(1);

    // Computes output data for B channel
    outData.select<BLOCK_HEIGHT, 1, BLOCK_PIXEL_WIDTH, 4>(0, 0) =
        sampledData.row(2);

    // Performs 2D media block write to save output data
    write(OUTBUF_IDX, horizOffset * 4, vertOffset, outData);
}
