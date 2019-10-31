/*
 * Copyright (c) 2017-2019, Intel Corporation
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

const float W8_init[] = {
    +0.000000000000000f,
    +0.707106781186547f,
    +1.000000000000000f,
};

const float W16_init[] = {
    +0.000000000000000f,
    +0.382683432365090f,
    +0.707106781186547f,
    +0.923879532511287f,
    +1.000000000000000f,
};

const float W32_init[] = {
    +0.000000000000000f,
    +0.195090322016128f,
    +0.382683432365090f,
    +0.555570233019602f,
    +0.707106781186547f,
    +0.831469612302545f,
    +0.923879532511287f,
    +0.980785280403230f,
    +1.000000000000000f,
};

const unsigned short C8_init[] = {0, 1, 2, 3, 4, 5, 6, 7};

// ------------------------------------------------------------------
// Bit reversal procedure required by the Radix-2 DIF FFT algorithm.
// ------------------------------------------------------------------
template <int E>
inline int BitReverse(int value)
{
    int reversed = 0;
    #pragma unroll
    for (int j = 0; j < E; j++)
    {
        reversed <<= 1; reversed += value & 1; value >>= 1;
    }
    return reversed;
}

template <typename T, int N> inline
void swap(vector_ref<T, N> a, vector_ref<T, N> b)
{
    vector<T, N> t;
    t = a;
    a = b;
    b = t;
}

// ------------------------------
// Multiply complex numbers.
// ------------------------------
template <typename T, int N> inline
void cmul(vector_ref<T, N> a0,
          vector_ref<T, N> a1,
          vector<T, N> b0,
          vector<T, N> b1)
{
    vector<T, N> t0, t1;
    t0 = a0 * b0 - a1 * b1;
    t1 = a0 * b1 + a1 * b0;
    a0 = t0;
    a1 = t1;
}

template <typename T, int N> inline
void cmul(vector_ref<T, N> a0,
          vector_ref<T, N> a1,
          T b0,
          T b1)
{
    vector<T, N> t0 = b0;
    vector<T, N> t1 = b1;
    cmul(a0, a1, t0, t1);
}

// ---------------------
// 2-Point Radix-2 FFT.
// ---------------------
template <typename T, int N> inline
void fft2(vector_ref<T, N> a0,
          vector_ref<T, N> a1,
          vector_ref<T, N> b0,
          vector_ref<T, N> b1)
{
    vector<T, N> t0 = a0;
    vector<T, N> t1 = a1;
    a0 = a0 + b0;
    a1 = a1 + b1;
    b0 = t0 - b0;
    b1 = t1 - b1;
}

// ----------------------------------
// N-Point Radix-2 FFT butterflies.
// ----------------------------------
template <typename T, int N, int SIMD, int K> inline
void butterfly(matrix_ref<T, N, SIMD> x0,
               matrix_ref<T, N, SIMD> x1,
               vector<T, K> W,
               int crtfft,
               int offset)
{
    // apply twiddle factors
    #pragma unroll
    for (int i = 0; i < crtfft / 4; i++) {
        T w0 = +W[N / 4 - N / crtfft * i];
        T w1 = -W[N / crtfft * i];
        int j = 2 * crtfft / 4 + i + offset;
        cmul(x0.row(j), x1.row(j), w0, w1);

        w0 = -W[N / crtfft * i];
        w1 = -W[N / 4 - N / crtfft * i];
        j = 3 * crtfft / 4 + i + offset;
        cmul(x0.row(j), x1.row(j), w0, w1);
    }

    // perform butterflies
    #pragma unroll
    for (int i = 0; i < crtfft / 2; i++) {
        int j = offset + i + crtfft / 2;
        fft2(x0.row(offset + i), x1.row(offset + i), x0.row(j), x1.row(j));
    }
}

template <typename T, // Element type float/half
          int SIMD,   // SIMD size (8 or 16)
          int E>      // E = log2(N) with fft size N
inline void fftbase_io(SurfaceIndex buf, unsigned width, vector<T, (1 << E) / 4 + 1> W)
{
    const unsigned N = 1 << E;
    unsigned idx = cm_group_id(0) * cm_local_size(0) + cm_local_id(0);
    unsigned idy = cm_group_id(1) * cm_local_size(1) + cm_local_id(1);
    unsigned offset_x = idx * SIMD;
    unsigned offset_y = idy * N;
    unsigned data_offset = idy * width * N + idx * SIMD;

    // Generate element offsets <0,1,...,SIMD-1>
    vector<ushort, 8> Offset8(C8_init);
    vector<ushort, SIMD> Offset;
    Offset.select<8, 1>() = Offset8;
    #pragma unroll
    for (int i = 8; i < SIMD; i+= 8) {
        Offset.select<8, 1>(i) = Offset8 + i;
    }

    // Read data to compute DFT.
    matrix<float, N * 2, SIMD> data;

    #pragma unroll
    for (int i = 0; i < N; i++) {
        int j = BitReverse<E>(i);
        matrix_ref<float, 2, SIMD> data_row = data.select<2, 1, SIMD, 1>(2 * j, 0);
        vector<unsigned, SIMD> elt_offset = (data_offset + i * width + Offset) * 2;
        read_untyped(buf, CM_GR_ENABLE, data_row, elt_offset);
    }

    matrix_ref<float, N, SIMD> data0 = data.select<N, 2, SIMD, 1>(0, 0);
    matrix_ref<float, N, SIMD> data1 = data.select<N, 2, SIMD, 1>(1, 0);

    // Do butterflies.
    #pragma unroll
    for (int crtfft = 2; crtfft <= N; crtfft <<= 1) {
        #pragma unroll
        for (int offset = 0; offset < N; offset += crtfft) {
            butterfly(data0, data1, W, crtfft, offset);
        }
    }

    #pragma unroll
    for (int i = 0; i < N; i++) {
        matrix_ref<float, 2, SIMD> data_row = data.select<2, 1, SIMD, 1>(2 * i, 0);
        vector<unsigned, SIMD> elt_offset = (data_offset + i * width + Offset) * 2;
        write_untyped(buf, CM_GR_ENABLE, data_row, elt_offset);
    }
}

_GENX_MAIN_ void fft8_simd8_io(SurfaceIndex buf, unsigned width)
{
    vector<float, 3> W(W8_init);
    fftbase_io<float, 8, 3>(buf, width, W);
}

_GENX_MAIN_ void fft8_simd16_io(SurfaceIndex buf, unsigned width)
{
    vector<float, 3> W(W8_init);
    fftbase_io<float, 16, 3>(buf, width, W);
}

_GENX_MAIN_ void fft16_simd8_io(SurfaceIndex buf, unsigned width)
{
    vector<float, 5> W(W16_init);
    fftbase_io<float, 8, 4>(buf, width, W);
}

_GENX_MAIN_ void fft16_simd16_io(SurfaceIndex buf, unsigned width)
{
    vector<float, 5> W(W16_init);
    fftbase_io<float, 16, 4>(buf, width, W);
}

_GENX_MAIN_ void fft32_simd8_io(SurfaceIndex buf, unsigned width)
{
    vector<float, 9> W(W32_init);
    fftbase_io<float, 8, 5>(buf, width, W);
}

