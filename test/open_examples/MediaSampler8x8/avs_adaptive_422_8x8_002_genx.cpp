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

//////////////////////////////////////////////////////////////////////////////
// void PA_444AVS_PA(SamplerIndex s_index, SurfaceIndex i_index, SurfaceIndex
//              o_index, uint group_id, uint vert_block_num,
//              float pix0_u, float pix0_v,
//              float delta_u, float delta_v,
//              float u2d, float v2d,
//              uint dest_x, uint dest_y)
//
//  Description:
//    This funciton calls sample8x8 to carry out 1x AVS scaling on one
//    16x4 block on a YUY2 surface.
//
//  Parameters:
//    s_index - Sampler s_index of the input surface
//    i_index - Binding table index of the input surface.
//    o_index - Binding table index of the output surface.
//    group_id - Group ID.  In this test, 1 16x4 sampling block per group
//    vert_block_num -  Vertical block # within the group
//    pix0_u, pix0_v -  pixel 0 addr relative to group origin
//    delta_u, delta_v - Scaling step size
//    u2d, v2d - For NLAS
//    dest_x, dest_y - Destination block addr
//
//  Return:
//    Write scaled block to dest surface.
//
//////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
PA_444AVS_PA(SamplerIndex s_index, SurfaceIndex i_index, SurfaceIndex o_index,
             int block_width, int block_height, int frame_width,
             int frame_height) {
  int j = get_thread_origin_x();
  int i = get_thread_origin_y();
  int group_id = j;
  int vert_block_num = 0;

  // Normlized block origin
  float pix0_u = 2.34 * block_width * j / static_cast<float>(frame_width);
  float pix0_v = 0.546 * block_height * i / static_cast<float>(frame_height);
  // Normlized step size
  float delta_u = 2.34 / frame_width;
  float delta_v = 0.546 / frame_height;
  float u2d = 0;    // No NLAS
  float v2d = 0;
  int dest_x = 2 * block_width * j;   // yuy2 in byte addr
  int dest_y = block_height * i;

  matrix<ushort, 4, 64> write_back;
  matrix_ref<uchar, 4, 128> b_write_back = write_back.format<uchar, 4, 128>();
  matrix<uchar, 4, 32> YUY2_422;

  // Old
  // 1x non NLAS AVS scaling, 444 output, block size 16x4, IEFbypass is true
  // sample8x8AVS(write_back, CM_BGR_ENABLE, i_index, s_index, pix0_u, pix0_v,
  // delta_u, delta_v, u2d, CM_16_FULL, v2d, CM_AVS_16x4, true);

  // New
  // sample8x8AVS(matrix<RT, N, 64> &m, ChannelMaskType channelMask,
  // SurfaceIndex surfIndex, SamplerIndex samplerIndex, const float &u,
  // const float &v, const float &delta_u, const float &delta_v,
  // const float &u2d, int group_id = -1, short verticalBlockNumber = -1,
  // OutputFormatControl outControl=CM_16_FULL, float v2d=0, AVSExecMode
  // execMode=CM_AVS_16x4, bool IEFBypass=false )
  cm_avs_sampler(write_back, CM_BGR_ENABLE, i_index, s_index, pix0_u, pix0_v,
                 delta_u, delta_v, u2d, group_id, vert_block_num, CM_16_FULL,
                 v2d, CM_AVS_16x4, true);

  // Returns 16bits YUV444, 3*64 ushort in 12 GRFs, alpha channel is ignored.

  // 444 -> 422.  Both solutions work.
  // Shift right to low byte
  // YUY2_422.select<4,1,8,4>(0,3) = write_back.select<1,1,32,2>(0,0) >> 8; //V
  // YUY2_422.select<4,1,16,2>(0,0) = write_back.select<1,1,64,1>(1,0) >> 8;//Y
  // YUY2_422.select<4,1,8,4>(0,1) = write_back.select<1,1,32,2>(2,0) >> 8; //U

  // Take high byte
  YUY2_422.select<4,1,8,4>(0,3) = b_write_back.select<1,1,32,4>(0,1);  // V
  YUY2_422.select<4,1,16,2>(0,0) = b_write_back.select<1,1,64,2>(1,1); // Y
  YUY2_422.select<4,1,8,4>(0,1) = b_write_back.select<1,1,32,4>(2,1);  // U

  // Outputs AVSed block 16x4 pixels = 32x4 bytes.  8bits per pixel.
  write(o_index, dest_x, dest_y, YUY2_422);
}
