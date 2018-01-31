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

#define max_ch_value (255)

// Horizontal blur.
extern "C" _GENX_MAIN_
void horizontal_blur(SurfaceIndex ibuf, SurfaceIndex obuf,
                     uint width, vector<float, 2 * 4 + 1> K) {
    int i, row, k;

    // The following matrices contain up to 4 x 16 pixels (in RGB), which are
    // split into in_r (the right side 4 x 4), in_c (the middle 4 x 8), and
    // in_l (the left side 4 x 4).
    matrix<uchar, 4, 8 * 3> in_c;
    matrix<uchar, 4, 4 * 3> in_l;
    matrix<uchar, 4, 4 * 3> in_r;

    matrix<uchar, 4, (4 + 4) * 3> tmp;
    vector<float, (4 * 2 + 1) * 4> tmp1;
    vector<float, 8> sum1;

    matrix<uchar, 4, (2 * 4 + 8) * 4> in_unp;
    matrix<uchar, 4, 8 * 3> out;

    uint h_pos = 0;
    uint v_pos = get_thread_origin_y() * 4;
    // Preload 4 x 8 pixels into in_c (left half) and in_l.
    read(ibuf, (h_pos + 0) * 3, v_pos, in_c.select<4, 1, 4 * 3, 1>(0, 4 * 3));
    read(ibuf, (h_pos + 4) * 3, v_pos, in_l);

    for (h_pos = 4; h_pos < width - 4; h_pos += 8) {
        // Shift-right 4 x 8 pixels from in_c (left half) to in_r and in_l to
        // in_c (right half).
        in_r = in_c.select<4, 1, 4 * 3, 1>(0, 4 * 3);
        in_c.select<4, 1, 4 * 3, 1>(0, 0) = in_l;
        // Load the next 4 x 8 pixels.
        read(ibuf, (h_pos + 4) * 3, v_pos, tmp);
        // Pack them into in_c (left half) and in_l.
        in_c.select<4, 1, 4 * 3, 1>(0, 4 * 3) =
            tmp.select<4, 1, 4 * 3, 1>(0, 0);
        in_l = tmp.select<4, 1, 4 * 3, 1>(0, 4 * 3);

        // Unpack 3-component pixel into 4-element data. Per 2 pixels, unpack
        // them from
        //
        //  (r0, g0, b0, r1, g1, b1)
        //
        // into
        //
        //  (r0, g0, b0, r1, b0, r1, g1, b1)
        //
        // to avoid unaligned accesses.

        // Unpack the middle 4 x 8 pixels first.
        #pragma unroll
        for (row = 0; row < 4; row++) {
            #pragma unroll
            for (i = 0; i < 8 / 2; i++) {
                in_unp.select<1, 1, 8, 1>(row, i * 8 + 16) =
                    in_c.row(row).replicate<2, 2, 4, 1>(i * 6);
            }
        }

        // Unpack the right and left 4 x 4 pixels.
        #pragma unroll
        for (row = 0; row < 4; row++) {
            #pragma unroll
            for (i = 0; i < 4/2; i++) {
                in_unp.select<1, 1, 8, 1>(row, i * 8)
                    = in_r.row(row).replicate<2, 2, 4, 1>(i * 6);
                in_unp.select<1, 1, 8, 1>(row, i * 8 + 48)
                    = in_l.row(row).replicate<2, 2, 4, 1>(i * 6);
            }
        }

        // Calculate the 9-point convolution from 4 x 16 inputs into 4 x 8
        // outputs.
        #pragma unroll
        for (row = 0; row < 4; row++) {
            #pragma unroll
            for (i = 0; i < 8; i++) {
                tmp1 = in_unp.select<1, 1, 36, 1>(row, i * 4) *
                       K.replicate<9, 1, 4, 0>();

                sum1 = tmp1.select<8,1>(0);
                #pragma unroll
                for (k = 1; k < 36 / 8; k++) {
                    sum1 += tmp1.select<8, 1>(k * 8);
                }
                sum1.select<4, 1>() += tmp1.select<4, 1>(36 - 4);
                sum1.select<3, 1>(i & 1) += sum1.select<3, 1>(5 - (i & 1));
                out.select<1, 1, 3, 1>(row, i * 3) = sum1.select<3,1>(i & 1);
            }
        }
        write(obuf, h_pos * 3, v_pos, out);
    }

    // Fill borders with original 4 x 4 pixels.
    read(ibuf, 0 * 3, v_pos, in_l);
    write(obuf, 0 * 3, v_pos, in_l);
    read(ibuf, (width - 4) * 3, v_pos, in_l);
    write(obuf, (width - 4) * 3, v_pos, in_l);
}

// Vertical blur
extern "C" _GENX_MAIN_ void
vertical_blur(SurfaceIndex ibuf, SurfaceIndex obuf,
             uint height, vector<float, 2 * 4 + 1> K) {
    int i, k;

    matrix<uchar, 16, 16> in_unp;
    matrix<uchar, 8, 16> out;
    vector<float, 16> tmp1;

    uint v_pos = 0;
    uint h_pos = get_thread_origin_x() * 4;

    read(MODIFIED(ibuf), h_pos * 3, v_pos,
         in_unp.select<8, 1, 16, 1>(4 + 4, 0));

    for (v_pos = 4; v_pos < height - 4; v_pos += 8) {
        in_unp.select<8, 1, 16, 1>(0, 0) = in_unp.select<8, 1, 16, 1>(4 + 4, 0);
        read(MODIFIED(ibuf), h_pos * 3, v_pos + 4,
             in_unp.select<8, 1, 16, 1>(4 + 4, 0));

        #pragma unroll
        for (k = 0; k < 8; k++) {
            tmp1 = in_unp.row(k) * K(0);
            #pragma unroll
            for (i = 1; i < 2 * 4 + 1; i++) {
                tmp1 += in_unp.row(k + i) * K(i);
            }
            out.select<1, 1, 12, 1>(k, 0) = tmp1.select<12, 1>();
        }
        write(obuf, h_pos * 3, v_pos, out.select<8, 1, 12, 1>());
    }

    read(MODIFIED(ibuf), h_pos * 3, 0, in_unp.select<4, 1, 12, 1>());
    write(obuf, h_pos * 3, 0, in_unp.select<4, 1, 12, 1>());

    read(MODIFIED(ibuf), h_pos * 3, height - 4, in_unp.select<4, 1, 12, 1>());
    write(obuf, h_pos * 3, height - 4, in_unp.select<4, 1, 12, 1>());
}

_GENX_ matrix<uchar, 4, 16> fbitmap1;
_GENX_ matrix<uchar, 4, 16> fbitmap2;

_GENX_ void
blend_darken(short opacity) {
    matrix_ref<uchar, 4, 16> Lval = fbitmap1;
    matrix_ref<uchar, 4, 16> Hval = fbitmap2;

    matrix<short, 4, 16> tmp1;
    tmp1 = (Lval * (128 - opacity) + Hval * opacity) >> 7;
    fbitmap1.merge(tmp1, Hval < Lval);
}

_GENX_ void
blend_lighten(short opacity) {
    matrix_ref<uchar, 4, 16> Lval = fbitmap1;
    matrix_ref<uchar, 4, 16> Hval = fbitmap2;

    matrix<short, 4, 16> tmp1;
    tmp1 = (Lval * (128 - opacity) + Hval * opacity) >> 7;
    fbitmap1.merge(tmp1, Hval > Lval);
}


_GENX_ void
blend_screen(short opacity) {
    matrix_ref<uchar, 4, 16> Lval = fbitmap1;
    matrix<short, 4, 16> Hval = fbitmap2;

    Hval = max_ch_value - cm_quot((max_ch_value - Lval) * (max_ch_value - Hval),
                                  max_ch_value);
    fbitmap1 = (Lval * (128 - opacity) + Hval * opacity) >> 7;
}


_GENX_ void
blend_multiply(short opacity) {
    matrix_ref<uchar, 4, 16> Lval = fbitmap1;
    matrix<short, 4, 16> Hval = fbitmap2;

    Hval = cm_quot(Hval * Lval, max_ch_value);
    // Combine the original image with the result of multiplication.
    fbitmap1 = (Lval * (128 - opacity) + Hval * opacity) >> 7;
}

_GENX_ void
blend_difference(short opacity) {
    matrix_ref<uchar, 4, 16> Lval = fbitmap1;
    matrix<short, 4, 16> Hval = fbitmap2;

    // Get absolute difference.
    Hval = Lval - Hval;
    Hval.merge(-Hval, Hval < 0);

    // combine original image with the result of difference.
    fbitmap1 = (Lval * (128 - opacity) + Hval * opacity) >> 7;
}

// Unsharpen mask
extern "C" _GENX_MAIN_ void
simple_USM(SurfaceIndex ibuf, SurfaceIndex ibuf2, SurfaceIndex obuf,
           uint width, short Lamount, short Damount) {
    matrix<uchar, 4, 16> fbitmap_original;
    matrix<uchar, 4, 16> fbitmap_blur;
    matrix<uchar, 4, 16> fbitmap_light;
    matrix<uchar, 4, 16> fbitmap_dark;

    matrix<uchar, 4, 16> buffer;
    matrix<uchar, 4, 16> buffer2;

    uint h_pos;
    uint v_pos = get_thread_origin_y() * 4;

    for (h_pos = 0; h_pos < width * 3; h_pos += 16) {
        // Read original image.
        read(ibuf, h_pos, v_pos, fbitmap_original);
        // Read blurred image.
        read(MODIFIED(ibuf2), h_pos, v_pos, fbitmap_blur);

        if (Lamount > 0) {
            // Leave pixels which are darker in original bitmap unchanged.
            fbitmap1 = fbitmap_blur; fbitmap2 = fbitmap_original;
            blend_darken(128);
            buffer = fbitmap1;

            // Find a difference from an original bitmap.
            fbitmap1 = buffer; fbitmap2 = fbitmap_original;
            blend_difference(128);
            buffer = fbitmap1;

            // Amplify the effect.
            buffer2 = buffer;
            fbitmap1 = buffer; fbitmap2 = buffer2;
            blend_screen(128);
            blend_screen(128);
            buffer = fbitmap1;

            // Apply to original image.
            buffer2 = fbitmap_original;
            fbitmap1 = buffer2; fbitmap2 = buffer;
            blend_screen(Lamount);
            fbitmap_light = fbitmap1;
        } else {
            fbitmap_light = fbitmap_original;
        }

        if (Damount > 0) {
            buffer = fbitmap_blur;

            // Leave pixels which are lighter in original bitmap unchanged.
            fbitmap1 = buffer; fbitmap2 = fbitmap_original;
            blend_lighten(128);
            buffer = fbitmap1;

            // Find a difference from an original bitmap.
            fbitmap1 = buffer; fbitmap2 = fbitmap_original;
            blend_difference(128);
            buffer = fbitmap1;

            // Invert the difference.
            buffer = max_ch_value - buffer;

            fbitmap_original = fbitmap_light;
            buffer2 = buffer;

            // Amplify the difference.
            fbitmap1 = buffer; fbitmap2 = buffer2;
            blend_multiply(128);
            blend_multiply(128);
            buffer = fbitmap1;

            // Apply darkening halos to form the result image.
            fbitmap1 = fbitmap_original; fbitmap2 = buffer;
            blend_multiply(Damount);
            fbitmap_dark = fbitmap1;
        } else {
            fbitmap_dark = fbitmap_light;
        }

        write(obuf, h_pos, v_pos, fbitmap_dark);
    }
}
