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

// Linear filter: average neighbors for each pixel
// For Each pixel
//     For Each RGB Channel
//         I(x,y)=[I(x-1, y-1) + I(x-1, y) + I(x-1, y+1) +
//                 I(x, y-1) + I(x, y) + I(x, y+1) +
//                 I(x+1, y-1) + I(x+1, y) + I(x+1, y+1)]/9
//
// Every thread outputs results for a 6x8 pixel-block
// Since the data is R8G8B8, therefore the output is
// a 6x24 matrix of uchar elements
//
// _GENX_MAIN_ attribute means this function is a kernel entry
// SurfaceIndex ibuf is input surface
// SurfaceIndex obuf is output surface
extern "C" _GENX_MAIN_ void
linear(SurfaceIndex ibuf, SurfaceIndex obuf)
{
    // declare 8x32 input matrix of uchar elements
    // Note: 8x30 is sufficient for the computation
    // however block-read only reads the multiple of dwords
    matrix<uchar, 8, 32> in;
    // declare 6x24 output matrix of uchar elements
    matrix<uchar, 6, 24> out;
    // declare intermediate matrix for summation
    matrix<float, 6, 24> m;

    // when we use media-walker, we can get thread-ids
    // using the following intrinsic instead of using
    // per-thread arguments
    uint h_pos = get_thread_origin_x();
    uint v_pos = get_thread_origin_y();

    // 2D media-block read from surface to input
    read(ibuf, h_pos * 24, v_pos * 6, in);
    // copy 6x24 block from in-matrix starting at corner (v1,h3)
    // to m-matrix
    // <6,1,24,1> means height is 6, vertical stride is 1
    // width is 24, horizontal stride is 1
    m = in.select<6, 1, 24, 1>(1, 3);
    // add 6x24 block from in-matrix starting at corner(v0,h0)
    // to m-matrix
    m += in.select<6, 1, 24, 1>(0, 0);
    // add 6x24 block from in-matrix starting at corner(v0,h3)
    // to m-matrix
    m += in.select<6, 1, 24, 1>(0, 3);
    // add 6x24 block from in-matrix starting at corner(v0,h6)
    // to m-matrix
    m += in.select<6, 1, 24, 1>(0, 6);
    // add 6x24 block from in-matrix starting at corner(v1,h0)
    // to m-matrix
    m += in.select<6, 1, 24, 1>(1, 0);
    // add 6x24 block from in-matrix starting at corner(v1,h6)
    // to m-matrix
    m += in.select<6, 1, 24, 1>(1, 6);
    // add 6x24 block from in-matrix starting at corner(v2,h0)
    // to m-matrix
    m += in.select<6, 1, 24, 1>(2, 0);
    // add 6x24 block from in-matrix starting at corner(v2,h3)
    // to m-matrix
    m += in.select<6, 1, 24, 1>(2, 3);
    // add 6x24 block from in-matrix starting at corner(v2,h6)
    // to m-matrix
    m += in.select<6, 1, 24, 1>(2, 6);
    // divide by 9 approximately, mul is faster
    // implicit type conversion from float to uchar
    out = m * 0.111f;
    // 2D media-block write to surface
    write(obuf, h_pos * 24, v_pos * 6, out);
}

// change the algorithm from 2-d convolution to 2 1-d convolution.
// This change saves computation yet requires more registers.
// So it is a trade-off between time and space. On GEN, every EU-thread
// gets 128x32 bytes of regiser space. As long as kernels can stay within
// this limit, we should strive for computation efficiency.
extern "C" _GENX_MAIN_ void
linear1d2(SurfaceIndex ibuf, SurfaceIndex obuf)
{
    matrix<uchar, 8, 32> in;
    matrix<uchar, 6, 24> out;
    matrix<short, 8, 24> m;
    matrix<short, 6, 24> m_out;

    // when we use media-walker, we can get thread-ids
    // using the following intrinsic instead of using
    // per-thread arguments
    uint h_pos = get_thread_origin_x();
    uint v_pos = get_thread_origin_y();

    read(ibuf, h_pos*24, v_pos*6, in);

    // sum up the input pixel values by columns
    m = in.select<8,1,24,1>(0,0) + in.select<8,1,24,1>(0,3);
    m += in.select<8,1,24,1>(0,6);

    // sum up the m values by rows
    m_out = m.select<6,1,24,1>(0,0) + m.select<6,1,24,1>(1,0);
    m_out += m.select<6,1,24,1>(2,0);

    out = m_out * 0.111f;

    write(obuf, h_pos*24, v_pos*6, out);
}

// this version also use 2 1-d convolution to save computation.
// Unlike linear1d2, it uses a sliding window scheme to minimize
// the storage: 3 rows for both input and intermediate result,
// and one row for output. However, in this way, it loads input one
// row at a time, and store output one row at a time.
extern "C" _GENX_MAIN_ void
linearslide(SurfaceIndex ibuf, SurfaceIndex obuf)
{
    matrix<uchar, 3, 32> in;
    vector<uchar, 24> out;
    matrix<short, 3, 24> m;
    vector<short, 24> m_out;

    // when we use media-walker, we can get thread-ids
    // using the following intrinsic instead of using
    // per-thread arguments
    uint h_pos = get_thread_origin_x();
    uint v_pos = get_thread_origin_y();

    // reads the first 3 rows
    read(ibuf, h_pos*24, v_pos*6, in);

    // sum up the input pixel values by columns
    m = in.select<3,1,24,1>(0,0)
        + in.select<3,1,24,1>(0,3)
        + in.select<3,1,24,1>(0,6);

#pragma unroll
    for (int i = 0; i < 5; ++i) {
        // sum up the m values by rows
        m_out = m.row(0) + m.row(1) + m.row(2);
        out = m_out * 0.111f;
        // write out one row
        write(obuf, h_pos*24, v_pos*6+i, out);
        // read in the next row
        read(ibuf, h_pos*24, v_pos*6+i+3, in.row(i%3));
        // sum up pixels by columns
        m.row(i%3) = in.select<1,1,24,1>(i%3,0)
            + in.select<1,1,24,1>(i%3,3)
            + in.select<1,1,24,1>(i%3,6);
    }
    // sum up the m values by rows
    m_out = m.row(0) + m.row(1) + m.row(2);
    out = m_out * 0.111f;
    // write out the last row
    write(obuf, h_pos*24, v_pos*6+5, out);
}

// This is another version of using sliding window.
// However it only minimizes the intermediate result to 3 rows.
// It keeps the entire input block and output block in registers
// in order to utilize the large media-block read and write.
extern "C" _GENX_MAIN_ void
linearslide2(SurfaceIndex ibuf, SurfaceIndex obuf)
{
    matrix<uchar, 8, 32> in;
    matrix<uchar, 6, 24> out;
    matrix<short, 3, 24> m;
    vector<short, 24> m_out;

    // when we use media-walker, we can get thread-ids
    // using the following intrinsic instead of using
    // per-thread arguments
    uint h_pos = get_thread_origin_x();
    uint v_pos = get_thread_origin_y();

    // read 8x32 block
    read(ibuf, h_pos*24, v_pos*6, in);

    // sum up the first 3-row input values by columns
    m = in.select<3,1,24,1>(0,0)
        + in.select<3,1,24,1>(0,3)
        + in.select<3,1,24,1>(0,6);

#pragma unroll
    for (int i = 0; i < 5; ++i) {
        // sum up the m values by rows
        m_out = m.row(0) + m.row(1) + m.row(2);
        out.row(i) = m_out * 0.111f;
        // update one row of m
        m.row(i%3) = in.select<1,1,24,1>(i+3,0)
            + in.select<1,1,24,1>(i+3,3)
            + in.select<1,1,24,1>(i+3,6);
    }
    // sum up the m values by rows
    m_out = m.row(0) + m.row(1) + m.row(2);
    out.row(5) = m_out * 0.111f;
    // write 6x24 block
    write(obuf, h_pos*24, v_pos*6, out);
}

