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

// Calculate Integral Image:
// every output pixel is the summation of all pixels in the sub-image
// from (0, 0) to (x, y). Define the equation in the recursive form
//    S(x, y) = I(x, y) + S(x-1, y) + S(x, y-1) - S(x-1, y-1)
// This example does it in 16x16 block fashion in order to fully
// utilize the funtion-unit and registers of evry GEN execution-unit
_GENX_ void inline acc_matrix16x16(matrix_ref<uchar, 16, 16> m_in_uchar,
                                   matrix_ref<uint, 16, 16> m_out,
                                   matrix_ref<uint, 16, 1> v_16_vert,
                                   vector_ref<uint, 16> v_16_hori,
                                   vector_ref<uint, 1> upleft)
{
    matrix<uint, 16, 16> m_in = m_in_uchar;
    matrix<uint, 16, 1> v_16_vert_ini = v_16_vert;

    matrix<uint, 16, 8> m_16x8;
    matrix<uint, 16, 4> m_16x4;
    matrix<uint, 16, 2> m_16x2;

    // The following algorithm does a 2D prefix-sum of 16x16 matirx
    //1: From <16,16> to <16,8>
    m_16x8 = m_in.select<16,1,8,2>(0,0) + m_in.select<16,1,8,2>(0,1);

    //2: From <16,8> to <16,4>
    m_16x4 = m_16x8.select<16,1,4,2>(0,0) + m_16x8.select<16,1,4,2>(0,1);

    //3: From <16,4> to <16,2>
    m_16x2.select<16,1,1,1>(0,1) = m_16x4.select<16,1,1,1>(0,0) + m_16x4.select<16,1,1,1>(0,1);

    //CLEAR
    m_16x2.column(0) = 0;

    m_16x4.select<16,1,2,2>(0,1) = m_16x2 + m_16x4.select<16,1,2,2>(0,0);
    m_16x4.select<16,1,2,2>(0,0) = m_16x2;

    m_16x8.select<16,1,4,2>(0,1) = m_16x4 + m_16x8.select<16,1,4,2>(0,0);
    m_16x8.select<16,1,4,2>(0,0) = m_16x4;

    m_out.select<16,1,8,2>(0,1) = m_16x8 + m_in.select<16,1,8,2>(0,0);
    m_out.select<16,1,8,2>(0,0) = m_16x8;


#pragma unroll
    for(int i = 0; i < 16;  i++)
        m_out.row(i) += m_in.row(i);

    m_out.row(1)  += m_out.row(0) ;
    m_out.row(2)  += m_out.row(1) ;
    m_out.row(3)  += m_out.row(2) ;
    m_out.row(4)  += m_out.row(3) ;
    m_out.row(5)  += m_out.row(4) ;
    m_out.row(6)  += m_out.row(5) ;
    m_out.row(7)  += m_out.row(6) ;
    m_out.row(8)  += m_out.row(7) ;
    m_out.row(9)  += m_out.row(8) ;
    m_out.row(10) += m_out.row(9) ;
    m_out.row(11) += m_out.row(10);
    m_out.row(12) += m_out.row(11);
    m_out.row(13) += m_out.row(12);
    m_out.row(14) += m_out.row(13);
    m_out.row(15) += m_out.row(14);

    // after the local prefix-sum,
    // first add the global sum from the left neighbor
    vector<uint, 16> v_16_vertical = v_16_vert_ini;
#pragma unroll
    for(int i = 0; i < 16; i++){
        m_out.row(i) += v_16_vertical(i);
    }
#pragma unroll
    // then add the global sum from the up-neighbor
    for(int k = 0; k < 16; k++)
        m_out.row(k) += v_16_hori;
    // finally minus the global sum from the up-left corner:w
    m_out -= upleft(0);
}

extern "C" _GENX_MAIN_
void CalcIntImage(SurfaceIndex bufin,
                  SurfaceIndex bufout)
{
    uint rd_h_pos = get_thread_origin_x() * 16;
    uint wr_h_pos = get_thread_origin_x() * 64;//rd_h_pos * 4;
    uint rd_v_pos = get_thread_origin_y() * 16;
    uint wr_v_pos = get_thread_origin_y() * 16;//rd_v_pos;

    matrix<uint, 16, 1> v_16_vert;
    vector<uint, 16> v_16_hori;
    matrix<uchar, 16, 16> read_in_matrix;
    matrix<uint, 16, 16> calc_matrix;
    vector<uint, 1> upleft;

    // read the 16x16 pixel block from input image
    read(bufin, rd_h_pos, rd_v_pos, read_in_matrix);
    int wr_h_pos_minus_four = wr_h_pos - 4;
    int rd_v_pos_minus_one = rd_v_pos - 1;

    // wait for the up-thread, left thread, and the up-left thread to finish
    cm_wait();

    // get the sum from the left-neighbor
    if(rd_h_pos == 0)
        v_16_vert = 0;
    else{
        read(bufout, wr_h_pos_minus_four, rd_v_pos, v_16_vert);
    }

    // get the sum from the up-neighbor
    if(rd_v_pos == 0)
        v_16_hori = 0;
    else{
        read(bufout, wr_h_pos, rd_v_pos_minus_one,
             v_16_hori.select<8,1>(0));
        read(bufout, wr_h_pos+32, rd_v_pos_minus_one,
             v_16_hori.select<8,1>(8));
    }

    // get the sum from the up-left corner
    if(rd_v_pos != 0 && rd_h_pos != 0){
        read(bufout, wr_h_pos_minus_four, rd_v_pos_minus_one, upleft);
    }
    else
        upleft = 0;

    // compute the output for this 16x16 pixel block
    acc_matrix16x16 (read_in_matrix, calc_matrix, v_16_vert, v_16_hori, upleft);

    // write the output
    write(bufout, wr_h_pos, wr_v_pos, calc_matrix.select<8,1,8,1>(0,0));
    write(bufout, wr_h_pos+32, wr_v_pos, calc_matrix.select<8,1,8,1>(0,8));
    write(bufout, wr_h_pos, wr_v_pos+8, calc_matrix.select<8,1,8,1>(8,0));
    write(bufout, wr_h_pos+32, wr_v_pos+8, calc_matrix.select<8,1,8,1>(8,8));

    // cm_fence makes sure the writes are truely finished (in the memory)
    cm_fence();
    // inform dependents
    cm_signal();
}
