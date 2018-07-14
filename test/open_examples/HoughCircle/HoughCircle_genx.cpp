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
#include "HoughCircle.h"

const float pi = 3.1415926f;
const float init_theta[8] = {0,            1 * pi / 180, 2 * pi / 180,
                             3 * pi / 180, 4 * pi / 180, 5 * pi / 180,
                             6 * pi / 180, 7 * pi / 180};

_GENX_MAIN_ void
cmk_hough_circle_acc(SurfaceIndex input,      // edge pixels image
                     SurfaceIndex output,     // hough circle accumulator counts
                     unsigned int img_height, // edge pixel image's height
                     unsigned int img_width,  // edge pixel image's width
                     unsigned int radius) { // radius of circle we are detecting
  // h_pos indicates which 256-element chunk the kernel is processing
  uint x = get_thread_origin_x();
  uint y = get_thread_origin_y();
  // starting pixel poistion of this block
  uint startX = x * BLK_W;
  uint startY = y * BLK_H;

  // read in BLK_H x VLK_W pixel block
  matrix<uchar, BLK_H, BLK_W> edges;
  read(input, startX, startY, edges);

  // return if there is no edge pixel
  if ((edges == 0).all())
    return;

  // prepare theta values
  vector<float, 96> theta;
  vector<float, 8> theta8(init_theta);

  theta.select<8, 1>(0) = theta8;
  theta.select<8, 1>(8) = theta8 + (8 * pi / 180);
  theta.select<16, 1>(16) = theta.select<16, 1>(0) + (16 * pi / 180);
  theta.select<32, 1>(32) = theta.select<32, 1>(0) + (32 * pi / 180);
  theta.select<32, 1>(64) = theta.select<32, 1>(0) + (64 * pi / 180);

  vector<float, 96> r_times_sin;
  vector<float, 96> r_times_cos;
  // compute radius times sin cos
  r_times_sin = radius * cm_sin<96>(theta);
  r_times_cos = radius * cm_cos<96>(theta);

  for (int i = 0; i < BLK_H; i++) {
    vector<uchar, 8> row = edges.row(i);
    if ((row == 0).all())
      continue;

    // look for edge pixel
    for (int j = 0; j < BLK_W; j++) {
      if (row(j)) { // detect edge pixel

        // compute positions of the points on circumference of the circle
        // (center (i,j))
        vector<uint, 8>
            ret; // for atomic return value even though we don't need it
        vector<uint, 96> xpos;
        vector<uint, 96> ypos;
        // compute x & y positions for 0 - 89 degrees
        xpos = startX + j + r_times_sin + 0.5f; // + 0.5f for rounding
        ypos = startY + i + r_times_cos + 0.5f; // + 0.5f for rounding

// HW discards out-of-bound accesses so there is no need to
// check if xpos and ypos are out of bound, e.g., ypos.select<8, 1>(k) <
// img_height
#pragma unroll
        for (auto k = 0; k < 88; k += 8) {
          write_typed_atomic<ATOMIC_INC, uint, 8>(
              output, ret, xpos.select<8, 1>(k), ypos.select<8, 1>(k));
        }
        write_typed_atomic<ATOMIC_INC, uint, 2>(output, ret.select<2, 1>(0),
                                                xpos.select<2, 1>(88),
                                                ypos.select<2, 1>(88));
        //} SIMD_IF_END;
        // compute x & y positions for 90 - 179 degrees
        // sin(90+x) = cos(x)
        // cos(90+x) = -sin(x)
        xpos = startX + j + r_times_cos + 0.5f;
        ypos = startY + i - r_times_sin + 0.5f;
#pragma unroll
        for (auto k = 0; k < 88; k += 8) {
          write_typed_atomic<ATOMIC_INC, uint, 8>(
              output, ret, xpos.select<8, 1>(k), ypos.select<8, 1>(k));
        }
        write_typed_atomic<ATOMIC_INC, uint, 2>(output, ret.select<2, 1>(0),
                                                xpos.select<2, 1>(88),
                                                ypos.select<2, 1>(88));
        // compute x & y positions for 180 - 269 degrees
        // sin(180+x) = -sin(x)
        // cos(180+x) = -cos(x)
        xpos = startX + j - r_times_sin + 0.5f;
        ypos = startY + i - r_times_cos + 0.5f;
#pragma unroll
        for (auto k = 0; k < 88; k += 8) {
          write_typed_atomic<ATOMIC_INC, uint, 8>(
              output, ret, xpos.select<8, 1>(k), ypos.select<8, 1>(k));
        }
        write_typed_atomic<ATOMIC_INC, uint, 2>(output, ret.select<2, 1>(0),
                                                xpos.select<2, 1>(88),
                                                ypos.select<2, 1>(88));
        // sin(270+x) = -cos(x)
        // cos(270+x) = sin(x)
        xpos = startX + j - r_times_cos + 0.5f;
        ypos = startY + i + r_times_sin + 0.5f;
#pragma unroll
        for (auto k = 0; k < 88; k += 8) {
          write_typed_atomic<ATOMIC_INC, uint, 8>(
              output, ret, xpos.select<8, 1>(k), ypos.select<8, 1>(k));
        }
        write_typed_atomic<ATOMIC_INC, uint, 2>(output, ret.select<2, 1>(0),
                                                xpos.select<2, 1>(88),
                                                ypos.select<2, 1>(88));
      }
    }
  }
}
