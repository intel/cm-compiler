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

//----------------------------------------------------------------------
// Algorithm
//
// Sepia tone conversion:
// OutR = (InR * .411) + (InG *.741) + (InB * .201)
// OutG = (InR * .161) + (InG *.691) + (InB * .161)
// OutB = (InR * .291) + (InG *.531) + (InB * .101)
//
// 8x8 pixels are processed together
//----------------------------------------------------------------------

#define ROWS    8

extern "C" _GENX_MAIN_ void
sepia(SurfaceIndex INBUF, SurfaceIndex OUTBUF)
{
    matrix<uchar, ROWS, 32> in;
    matrix<float, ROWS, 32> in_float;
    matrix<uchar, ROWS, 32> out;
    vector<float, 8> v;
    vector<float, 6> w;

    vector<float, 8> r;
    vector<float, 8> g;
    vector<float, 8> b;

    // initialize the coefficients
    // The coefficients are set for doing dot-product on two pixels
    // [0.201, 0.741, 0.411, 0, 0, 0.201, 0.741, 0.411]
    // refer to the usage in the loop
    r(0) = r(5) = 0.201f;
    r(1) = r(6) = 0.741f;
    r(2) = r(7) = 0.411f;
    r(3) = r(4) = 0.0f;

    g(0) = g(5) = 0.161f;
    g(1) = g(6) = 0.691f;
    g(2) = g(7) = 0.361f;
    g(3) = g(4) = 0.0f;

    b(0) = b(5) = 0.101f;
    b(1) = b(6) = 0.531f;
    b(2) = b(7) = 0.291f;
    b(3) = b(4) = 0.0f;

    int h_pos = get_thread_origin_x() * 24;
    int v_pos = get_thread_origin_y() * ROWS;
    // read the input-image pixel block
    read(INBUF, h_pos, v_pos, in);
    // convert them to float values for computation
    in_float = matrix<float, ROWS, 32>(in);

    int i, j;
    #pragma unroll
    for (i = 0; i < 8; i++) {
        #pragma unroll
        for (j = 0; j <= 18; j+=6) {
             // every iteration works on two pixels, six input values
             // [B1, G1, R1, B2, G2, R2]
             // replicate operation expands them into a vector<float, 8>
             // [B1, G1, R1, B2, R1, B2, G2, R2],
             //
             // Explanation of the replicate operation:
             //     According to the spec of replicate<REP, VS, W, HS>,
             // here the REP is 2, W is 4, HS is 1, therefore, first time
             // it picks 4 elements starting from B1, so B1, G1, R1, B2;
             // second time, VS=2, move two element ahead, pick
             // R1, B2, G2, R2;
             //
             // The middle elements [B2, R1] do not contribute to the result
             // because the corresponding coefficients are zero.
             //
             // Definition of cm_dp4:
             //     cm_dp4 performs the 4-wide dot product operation for
             // each 4-tuple of elements in the input vector/matrix
             // parameters, and sets the same scalar product result to each
             // element of the corresponding 4-tuple in the return value.
             // so in this case, cm_dp4 produces two scalar values, one for
             // each pixel, they are replicated four times,
             // so we get [ob1, ob1, ob1, ob1, ob2, ob2, ob2, ob2]
             //
             // Explanation of the merge:
             //     The 2nd parameter of the merge is a bit-mask,
             // 0x12 is 00010010, so we end with v[1] = ob1; v[4] = ob2;
             v.merge(cm_dp4<float>(b, in_float.replicate<2, 2, 4, 1>(i, j)),
                                   0x12);
             // 0x24 is 00100100, so we end with v[2] = og1; v[5] = og2;
             v.merge(cm_dp4<float>(g, in_float.replicate<2, 2, 4, 1>(i, j)),
                                   0x24);
             // 0x48 is 01001000, so we end with v[3] = or1; v[6] = or2;
             v.merge(cm_dp4<float>(r, in_float.replicate<2, 2, 4, 1>(i, j)),
                                   0x48);
             // pick v[1-6] and convert them to uchar
             w = v.select<6, 1>(1);
             // write two pixels to the output matrix
             out.select<1, 1, 6, 1>(i, j) = vector<uchar, 6>(w, SAT);
        }
    }

    // write the 8x8 pixels to the output image
    write(OUTBUF, h_pos, v_pos, out.select<ROWS, 1, 24, 1>());
}
