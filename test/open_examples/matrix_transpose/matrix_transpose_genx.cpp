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

// The basic idea of vecotrizing transposition can be illustrated by
// transposing a 2 x 2 matrix as follows:
//
// A B
// C D
// ==>
// merge([A, A, B, B], [C, C, D, D], 0b1010) = [A, C, B, D]
// ==>
// A C
// B D
//
template <typename T, int N>
_GENX_ inline void
transpose_matrix(matrix_ref<T, N, N> m1, matrix_ref<T, N, N> m2) {
    // 32 bit mask to control how to merge two vectors.
    uint32_t mask = 0x55555555;
    int k = 0;

    #pragma unroll
    for (int j = 1; j < N; j *= 2) {
        k++;
        if (k % 2) {
            #pragma unroll
            for (int i = 0; i < N; i++)
                m2.row(i).merge(
                    m1.replicate<N / 2, 1, 2, 0>(i / 2, (i % 2) * (N / 2)),
                    m1.replicate<N / 2, 1, 2, 0>(i / 2 + N / 2, (i % 2) * (N / 2)),
                    mask);
        } else {
            #pragma unroll
            for (int i = 0; i < N; i++)
                m1.row(i).merge(
                    m2.replicate<N / 2, 1, 2, 0>(i / 2, (i % 2) * (N / 2)),
                    m2.replicate<N / 2, 1, 2, 0>(i / 2 + N / 2, (i % 2) * (N / 2)),
                    mask);
        }
    }
}

template <typename T>
CM_NODEBUG _GENX_ inline void
transpose_matrix(matrix_ref<T, 8, 8> m1, matrix_ref<T, 8, 8> m2) {
    // 32 bit mask to control how to merge two vectors.
    uint32_t mask = 0x55555555;
    matrix_ref<T, 4, 16> t1 = m1.format<T, 4, 16>();
    matrix_ref<T, 4, 16> t2 = m2.format<T, 4, 16>();

    // j = 1
    t2.row(0).merge(t1.replicate<8, 1, 2, 0>(0, 0), t1.replicate<8, 1, 2, 0>(2, 0), mask);
    t2.row(1).merge(t1.replicate<8, 1, 2, 0>(0, 8), t1.replicate<8, 1, 2, 0>(2, 8), mask);
    t2.row(2).merge(t1.replicate<8, 1, 2, 0>(1, 0), t1.replicate<8, 1, 2, 0>(3, 0), mask);
    t2.row(3).merge(t1.replicate<8, 1, 2, 0>(1, 8), t1.replicate<8, 1, 2, 0>(3, 8), mask);

    // j = 2
    t1.row(0).merge(t2.replicate<8, 1, 2, 0>(0, 0), t2.replicate<8, 1, 2, 0>(2, 0), mask);
    t1.row(1).merge(t2.replicate<8, 1, 2, 0>(0, 8), t2.replicate<8, 1, 2, 0>(2, 8), mask);
    t1.row(2).merge(t2.replicate<8, 1, 2, 0>(1, 0), t2.replicate<8, 1, 2, 0>(3, 0), mask);
    t1.row(3).merge(t2.replicate<8, 1, 2, 0>(1, 8), t2.replicate<8, 1, 2, 0>(3, 8), mask);

    // j = 4
    t2.row(0).merge(t1.replicate<8, 1, 2, 0>(0, 0), t1.replicate<8, 1, 2, 0>(2, 0), mask);
    t2.row(1).merge(t1.replicate<8, 1, 2, 0>(0, 8), t1.replicate<8, 1, 2, 0>(2, 8), mask);
    t2.row(2).merge(t1.replicate<8, 1, 2, 0>(1, 0), t1.replicate<8, 1, 2, 0>(3, 0), mask);
    t2.row(3).merge(t1.replicate<8, 1, 2, 0>(1, 8), t1.replicate<8, 1, 2, 0>(3, 8), mask);
}

// Square matrix transposition on block of size 8x8
_GENX_MAIN_ void transpose8(SurfaceIndex buf) // input & output buffer
{
    static const int N = 8;
    int block_col = get_thread_origin_x();
    int block_row = get_thread_origin_y();

    if (block_row == block_col) {
        matrix<int, N, N> m1, t1;
        read(buf, N * block_col * sizeof(int), N * block_row, m1);
        transpose_matrix(m1, t1);
        write(buf, N * block_row * sizeof(int), N * block_col, t1);
    } else if (block_row < block_col) {
        // Read two blocks to be swapped.
        matrix<int, N, N> m1, t1;
        read(buf, N * block_col * sizeof(int), N * block_row, m1);
        matrix<int, N, N> m2, t2;
        read(buf, N * block_row * sizeof(int), N * block_col, m2);

        // Tranpose them.
        transpose_matrix(m1, t1);
        transpose_matrix(m2, t2);

        // Write them back to the transposed location.
        write(buf, N * block_row * sizeof(int), N * block_col, t1);
        write(buf, N * block_col * sizeof(int), N * block_row, t2);
    }
}

// Square matrix transposition on block of size 16x16.
// In this version, a thread handle a block of size 16x16 which allows
// to better latency hidding and subsentantially improve overall performance.
//
_GENX_MAIN_ void transpose16(SurfaceIndex buf) // input & output buffer
{
    static const int N = 16;
    int block_col = get_thread_origin_x();
    int block_row = get_thread_origin_y();

    if (block_row == block_col) {
        // Read a tile of 4 8x8 sub-blocks:
        //
        //  [ m00 m01 ]
        //  [ m10 m11 ]
        //
        matrix<int, 8, 8> m00, m01, m10, m11, t00, t01, t10, t11;
        read(buf, (N * block_col + 0) * sizeof(int), N * block_row + 0, m00);
        read(buf, (N * block_col + 8) * sizeof(int), N * block_row + 0, m01);
        read(buf, (N * block_col + 0) * sizeof(int), N * block_row + 8, m10);
        read(buf, (N * block_col + 8) * sizeof(int), N * block_row + 8, m11);

        // Tranpose sub-blocks.
        transpose_matrix(m00, t00);
        transpose_matrix(m01, t01);
        transpose_matrix(m10, t10);
        transpose_matrix(m11, t11);

        // write out as
        //
        // [t00 t10]
        // [t01 t11]
        //
        write(buf, (N * block_col + 0) * sizeof(int), N * block_row + 0, t00);
        write(buf, (N * block_col + 8) * sizeof(int), N * block_row + 0, t10);
        write(buf, (N * block_col + 0) * sizeof(int), N * block_row + 8, t01);
        write(buf, (N * block_col + 8) * sizeof(int), N * block_row + 8, t11);
    } else if (block_row < block_col) {
        // Read two tiles of 4 8x8 sub-blocks to be swapped.
        //
        matrix<int, 8, 8> a00, a01, a10, a11, t00, t01, t10, t11;
        read(buf, (N * block_col + 0) * sizeof(int), N * block_row + 0, a00);
        read(buf, (N * block_col + 8) * sizeof(int), N * block_row + 0, a01);
        read(buf, (N * block_col + 0) * sizeof(int), N * block_row + 8, a10);
        read(buf, (N * block_col + 8) * sizeof(int), N * block_row + 8, a11);

        matrix<int, 8, 8> b00, b01, b10, b11, r00, r01, r10, r11;
        read(buf, (N * block_row + 0) * sizeof(int), N * block_col + 0, b00);
        read(buf, (N * block_row + 8) * sizeof(int), N * block_col + 0, b01);
        read(buf, (N * block_row + 0) * sizeof(int), N * block_col + 8, b10);
        read(buf, (N * block_row + 8) * sizeof(int), N * block_col + 8, b11);

        // Tranpose the first tile.
        transpose_matrix(a00, t00);
        transpose_matrix(a01, t01);
        transpose_matrix(a10, t10);
        transpose_matrix(a11, t11);

        // Tranpose the second tile.
        transpose_matrix(b00, r00);
        transpose_matrix(b01, r01);
        transpose_matrix(b10, r10);
        transpose_matrix(b11, r11);

        // Write the first tile to the transposed location.
        write(buf, (N * block_row + 0) * sizeof(int), N * block_col + 0, t00);
        write(buf, (N * block_row + 8) * sizeof(int), N * block_col + 0, t10);
        write(buf, (N * block_row + 0) * sizeof(int), N * block_col + 8, t01);
        write(buf, (N * block_row + 8) * sizeof(int), N * block_col + 8, t11);

        // Write the second tile to the transposed location.
        write(buf, (N * block_col + 0) * sizeof(int), N * block_row + 0, r00);
        write(buf, (N * block_col + 8) * sizeof(int), N * block_row + 0, r10);
        write(buf, (N * block_col + 0) * sizeof(int), N * block_row + 8, r01);
        write(buf, (N * block_col + 8) * sizeof(int), N * block_row + 8, r11);
    }
}
