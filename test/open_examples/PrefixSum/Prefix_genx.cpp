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
#include "Prefix.h"

// Code copied from ../cmut/cmutility_genx.h so we can disable loop unrolling.

template<typename ty, unsigned int size>
inline _GENX_ void cmk_read (SurfaceIndex index, unsigned int offset, vector_ref<ty, size> v)
{

#pragma unroll
  for (unsigned int i = 0; i < size; i += 32) {
    read (index, offset + i * sizeof (ty), v.template select<32, 1> (i));
  }
}

template<typename ty, unsigned int size>
inline _GENX_ void cmk_write(SurfaceIndex index, unsigned int offset, vector_ref<ty, size> v)
{
#pragma unroll
  for (unsigned int i = 0; i < size; i += 32) {
    write(index, offset + i * sizeof(ty), v.template select<32, 1>(i));
  }
}

const short init_0_3[4] = {0, 1, 2, 3};

// Local count : the local count stage partitions the table into chunks.
// Each chunk is 256*TUPLE_SZ.Each HW thread sums up values for each column
// within one chunk and stores the results in the last entry of the chunk
// All data chunks can be executed in parallel in this stage.
_GENX_MAIN_ void cmk_sum_tuple_count(SurfaceIndex table)
{
  // h_pos indicates which 256-element chunk the kernel is processing
  uint h_pos = get_thread_origin_x() + get_thread_origin_y()*MAX_TS_WIDTH;
  // each thread handles PREFIX_ENTRIES entries. Each entry has 4 bins
  unsigned int offset = (h_pos * PREFIX_ENTRIES*TUPLE_SZ) << 2;

  vector<unsigned int, 32*TUPLE_SZ> S, T;
  cmk_read<unsigned int, 32*TUPLE_SZ>(table, offset, S);
#pragma unroll
  for (int i = 1; i < PREFIX_ENTRIES / 32; i++) {
    cmk_read<unsigned int, 32*TUPLE_SZ>(table, offset + i*32*TUPLE_SZ* 4, T);
    S += T;
  }
  matrix_ref<unsigned int, 32, TUPLE_SZ> cnt_table = S.format<unsigned int, 32, TUPLE_SZ>();
  // sum reduction for each bin
  cnt_table.select<16, 1, TUPLE_SZ, 1>(0, 0) += cnt_table.select<16, 1, TUPLE_SZ, 1>(16, 0);
  cnt_table.select<8, 1, TUPLE_SZ, 1>(0, 0) += cnt_table.select<8, 1, TUPLE_SZ, 1>(8, 0);
  cnt_table.select<4, 1, TUPLE_SZ, 1>(0, 0) += cnt_table.select<4, 1, TUPLE_SZ, 1>(4, 0);
  cnt_table.select<2, 1, TUPLE_SZ, 1>(0, 0) += cnt_table.select<2, 1, TUPLE_SZ, 1>(2, 0);
  cnt_table.select<1, 1, TUPLE_SZ, 1>(0, 0) += cnt_table.select<1, 1, TUPLE_SZ, 1>(1, 0);

  vector<uint, 4> wrt_addr(init_0_3);
  wrt_addr += ((h_pos + 1) * PREFIX_ENTRIES *TUPLE_SZ - TUPLE_SZ);
  // write only sum to the last entry of the current chunk
  write(table, 0, wrt_addr.select<TUPLE_SZ, 1>(0), cnt_table.row(0));
}

// Global Prefix sum : For a given chunk i, the last entry of chunk(i - 1)
// tells the prefix sum up to chunk i.Computing prefix sum for all input
// entries can be done in parallel. Each HW thread handles a 256-entry chunk.
// It reads the last entry of its previous chunk as the initial value and
// computes prefix sum for all entries within the chunk.
_GENX_MAIN_ void cmk_prefix(SurfaceIndex table)
{
  // h_pos indicates which 256-element chunk the kernel is processing
  uint h_pos = get_thread_origin_x() + get_thread_origin_y()*MAX_TS_WIDTH;
  // each thread handles PREFIX_ENTRIES entries. Each entry has 4 bins
  unsigned int offset = (h_pos * PREFIX_ENTRIES*TUPLE_SZ) << 2;

  vector<unsigned int, TUPLE_SZ> prev_BIN;
  if (h_pos == 0)
    prev_BIN = 0;
  else {
    vector<uint, 4> rd_addr(init_0_3);
    rd_addr += ((offset>>2) - TUPLE_SZ);
    read(table, 0, rd_addr.select<TUPLE_SZ, 1>(0), prev_BIN);
  }

  vector<unsigned int, 32 * TUPLE_SZ> T;
  matrix_ref<unsigned int, 32, TUPLE_SZ> cnt = T.format<unsigned int, 32, TUPLE_SZ>();
  int iter = PREFIX_ENTRIES / 32;
  for (int i = 0; i < iter - 1; i++) {
    cmk_read<unsigned int, 32 * TUPLE_SZ>(table, offset + i*(32 * TUPLE_SZ) * 4, T);
    // calculate prefix
    cnt.row(0) += prev_BIN;
#pragma unroll
    for (int j = 1; j < 32; j++) {
      cnt.row(j) += cnt.row(j - 1);
    }
    // write only sum to the last entry of the current chunk
    cmk_write<unsigned int, 32 * TUPLE_SZ>(table, offset + ((i * 32 * TUPLE_SZ) << 2), T);
    prev_BIN = cnt.row(31);
  }
  // handle last iteration
  cmk_read<unsigned int, 32 * TUPLE_SZ>(table, offset + (iter-1)*(32 * TUPLE_SZ) * 4, T);
  // calculate prefix
  cnt.row(0) += prev_BIN;
#pragma unroll
  for (int j = 1; j < 31; j++) {
    cnt.row(j) += cnt.row(j - 1);
  }
  // write only sum to the last entry of the current chunk
  cmk_write<unsigned int, 32 * TUPLE_SZ>(table, offset + (((iter - 1) * 32 * TUPLE_SZ) << 2), T);
}
