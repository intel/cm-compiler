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

#define ROWS    8

extern "C" _GENX_MAIN_ void
sepia (SurfaceIndex INBUF, SurfaceIndex OUTBUF, int height) {
  matrix<uchar, ROWS, 32> in;
  matrix<float, ROWS, 32> in_float;
  matrix<uchar, ROWS, 32> out;
  vector<float, 8> v;
  vector<float, 6> w;

  vector<float,8> r;
  vector<float,8> g;
  vector<float,8> b;

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
  int v_pos = get_thread_origin_y() * ROWS + height/2;

  //printf("(%d, %d)\n", h_pos, v_pos);

  read(INBUF, h_pos, v_pos, in);
  in_float = matrix<float, ROWS, 32>(in);

  int i, j;
  #pragma unroll
  for (i = 0; i < 8; i++) {
    #pragma unroll
    for (j = 0; j <= 18; j+=6) {
      v.merge(cm_dp4<float>(b, in_float.replicate<2,2,4,1>(i,j)),0x12);
      v.merge(cm_dp4<float>(g, in_float.replicate<2,2,4,1>(i,j)),0x24);
      v.merge(cm_dp4<float>(r, in_float.replicate<2,2,4,1>(i,j)),0x48);
      w = v.select<6,1>(1);
      out.select<1,1,6,1>(i,j) = vector<uchar,6>(w,SAT);
    }
  }

  write(OUTBUF, h_pos, v_pos, out.select<ROWS, 1, 24, 1>());
}

extern "C" _GENX_MAIN_ void
linear (SurfaceIndex ibuf, SurfaceIndex obuf) {
  matrix<uchar, 8, 32> in;
  matrix<uchar, 6, 24> out;
  matrix<float, 6, 24> m;

  int h_pos = (get_thread_origin_x()) * 24;
  int v_pos = get_thread_origin_y() * 6;

  read(ibuf, h_pos, v_pos, in);

  m  = in.select<6,1,24,1>(1,3);

  m += in.select<6,1,24,1>(0,0);
  m += in.select<6,1,24,1>(0,3);
  m += in.select<6,1,24,1>(0,6);

  m += in.select<6,1,24,1>(1,0);
  m += in.select<6,1,24,1>(1,6);

  m += in.select<6,1,24,1>(2,0);
  m += in.select<6,1,24,1>(2,3);
  m += in.select<6,1,24,1>(2,6);

  out = m * 0.111f;

  write(obuf, h_pos, v_pos, out);
}

