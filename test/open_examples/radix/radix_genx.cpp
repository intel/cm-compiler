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
#include "radix.h"

// There are three kernels:
// 1.	cmk_radix_count: which counts how many elements in each bin locally
//    within each HW thread.
// 2.	prefix sum : which cumulates the number of elements of bins
//    of all threads.
// 3.	cmk_radix_bucket : which reads a chunk of data, 256 elements, bins
//    them into buckets, finally writes elements in each bucket to
//    the output buffer based on the global positions calculated in step 2.
template<typename ty, unsigned int size>
inline _GENX_ void cmk_read (SurfaceIndex index, unsigned int offset, vector_ref<ty, size> v)
{
#pragma unroll
  for (unsigned int i = 0; i < size; i += 32) {
    read (index, offset + i * sizeof (ty), v.template select<32, 1> (i));
  }
}

#define TABLE_SZ 16
const uchar init16[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const uchar init_rev16[16] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
const ushort init_mask[BIN_NUM] = {0, BIN_SZ, 0, BIN_SZ};

// On GPU.We apply Map-Reduce mechanism to the bin counting process.
// First, we divide input data into chunks and Map computing local bin_cnt
// for a data chunk to each HW thread. Then apply Reduce operation to
// calculate prefix sum for all local bin_cnt.
// cmk_radix_count basically reads in one chunk of data, 256 elements,
// and counts how many elements in each bin.
_GENX_MAIN_ void cmk_radix_count(SurfaceIndex input, SurfaceIndex output, unsigned int n)
{
  // h_pos indicates which 256-element chunk the kernel is processing
  uint h_pos = get_thread_origin_x() + get_thread_origin_y()*MAX_TS_WIDTH;
  // byte offset of the data chunk
  unsigned int offset = (h_pos * BASE_SZ) << 2;
  // to take advantage of SIMD architecture, we process counting 32
  // elements as a batch rather than counting each element serially.
  // Here we create a 4x32 counters. Each time, 32 elements are read.
  // We can view them as 32 lanes of data. Each lane has its own
  // dedicated bin counters.
  matrix<unsigned short, 4, 32> counters;
  // after we are done with 32-element batch counting, we perform sum
  // reduction to calculate the bin counts for 256 elements. The results
  // are in bin_cnt[]
  vector<unsigned int, BIN_NUM> bin_cnt;
  counters = 0;
  unsigned int mask = 0x3 << n; // which 2 bits we want to extract

//#pragma unroll
  for (int i = 0; i < BASE_SZ; i += 32) {
    // read and process 32 elements each time
    vector<unsigned int, 32> A;
    cmk_read<unsigned int, 32>(input, offset + i * sizeof(unsigned int), A);
    // extract n-th and (n+1)-th bits out.
    // val is the bin number, data will be put. E.g., val[i] is bin # for A(i)
    vector<unsigned int, 32> val = (A & mask) >> n;
    // row(0) is for bin0 for all 32 lanes.
    // merge operation to increase its own corresponding counters
    // val == 0 indicate which lanes have 0. Only those channels are
    // incrementing.
    counters.row(0).merge(counters.row(0) + 1, val == 0);
    counters.row(1).merge(counters.row(1) + 1, val == 1);
    counters.row(2).merge(counters.row(2) + 1, val == 2);
    counters.row(3).merge(counters.row(3) + 1, val == 3);
  }
  // bin counters for 32 lanes are complete. Perform sum reduction to obtain
  // final bin counters for 256 elements. The most intuitive way is to
  // perform sum reduction for each row, e.g.,
  // bin_cnt[i] = cm_sum<unsigned int>(counters.row(i));
  // although cm_sum intrinsic is doing efficient sum reduction operations
  // however, there are more parallelism we can exploit when doing reduction
  // with all 4 rows together.
  matrix<unsigned short, 4, 16> tmp_sum16;
  // reduction from 32 to 16
  tmp_sum16 = counters.select<4, 1, 16, 1>(0, 0) + counters.select<4, 1, 16, 1>(0, 16);
  matrix<unsigned short, 4, 8> tmp_sum8;
  // reduction from 16 to 8
  tmp_sum8 = tmp_sum16.select<4, 1, 8, 1>(0, 0) + tmp_sum16.select<4, 1, 8, 1>(0, 8);
  matrix<unsigned short, 4,4> tmp_sum4;
  // reduction from 8 to 4
  tmp_sum4 = tmp_sum8.select<4, 1, 4, 1>(0, 0) + tmp_sum8.select<4, 1, 4, 1>(0, 4);
  matrix<unsigned short, 4, 2> tmp_sum2;
  // reduction from 4 to 2
  tmp_sum2 = tmp_sum4.select<4, 1, 2, 1>(0, 0) + tmp_sum4.select<4, 1, 2, 1>(0, 2);
  // reduction from 2 to 1
  bin_cnt = tmp_sum2.select<4, 1, 1, 1>(0, 0) + tmp_sum2.select<4, 1, 1, 1>(0, 1);
  //printf(" %d %d %d %d\n", bin_cnt[0], bin_cnt[1], bin_cnt[2], bin_cnt[3]);
  // write out count (number of elements in each bin) for each bin
  // bin_cnt[0] = bin_cnt[1] = 110;
  write(output, (h_pos * BIN_NUM) << 2, bin_cnt);
}

_GENX_MAIN_ void cmk_radix_bucket (
  SurfaceIndex input, // input data to be sorted
  SurfaceIndex table, // Prefix sum table
  SurfaceIndex output,  // output for binning result
  unsigned int bin0_cnt, // global bin0 count,
  unsigned int bin1_cnt, // global bin1 count
  unsigned int bin2_cnt, // global bin2 count
  unsigned int bin3_cnt, // global bin3 count
  unsigned int n)     // binning based n-th and (n+1)-th bits
{
  // h_pos indicates which 256-element chunk the kernel is processing
  uint h_pos = get_thread_origin_x() + get_thread_origin_y()*MAX_TS_WIDTH;
  // byte offset of the data chunk
  unsigned int offset = (h_pos * BASE_SZ) << 2;

  vector<unsigned int, BIN_NUM> prefix = 0;
  // loading PrefixSum[h_pos-1]
  // the information tells how many cumulated elements from thread 0 to
  // h_pos-1 in each bin. Thread0 has no previous prefix sum so 0 is
  // initialized.
  if (h_pos != 0) {
    read(table, ((h_pos-1)*BIN_NUM) << 2, prefix);
  }

  unsigned int mask = 0x3 << n;
  // the location where the next 32 elements can be put in each bin
  vector<unsigned int, BIN_NUM> next;
  next[0] = prefix[0];
  next[1] = bin0_cnt + prefix[1];
  next[2] = bin0_cnt + bin1_cnt + prefix[2];
  next[3] = bin0_cnt + bin1_cnt + bin2_cnt + prefix[3];

  for (int i = 0; i < BASE_SZ; i += 32) {
    // read and process 32 elements at a time
    vector<unsigned int, 32> A;
    cmk_read<unsigned int, 32>(input, offset + i * sizeof(unsigned int), A);
    // calculate bin # for each element
    vector<unsigned short, 32> val = (A & mask) >> n;
    vector<unsigned int, 4> bitset;
    // val has bin # for each element. val == 0 is a 32-element Boolean vector.
    // The ith element is 1 (true) if val[i] == 0, 0 (false) otherwise
    // cm_pack_mask(val == 0) turns the Boolean vector into one unsigned
    // 32-bit value. The i-th value is the corresponding i-th Boolean value.
    bitset(0) = cm_pack_mask(val == 0);
    bitset(1) = cm_pack_mask(val == 1);
    bitset(2) = cm_pack_mask(val == 2);
    bitset(3) = cm_pack_mask(val == 3);
    // calculate how many elements in each bin
    vector<unsigned short, 4> n_elems = cm_cbit<unsigned int>(bitset);

    // calculate prefix sum
    // For each bin, there is a corresponding "next" index pointing to
    // the next available slot. "val == 0" tells us which A elements
    // should be put into bin0. From position 0 to 31,
    // if val[i] == 0 then A[i] will be placed into bin0[next],
    // then bin0[next+1], bin0[next+2], etc.
    // For instance, "val == 0":   0 0 1 0 0 0 1 0 0 1 1 0 0 0 0 0 -- LSB
    // A[5] is placed in bin0[next], A[6] in bin0[next+1],
    // A[9] in bin0[next+2], A[13] in bin0[next+3]
    // Calculate prefix sum for "val == 0" we get
    // prefix_val0 = 4 4 4 3 3 3 3 2 2 2 1 0 0 0 0 0  --- LSB
    // the 5, 6, 9 and 13-th value of "next + prefix_val0 - 1" is the locations
    // where A[5], A[6], A[9] and A[13] will be stored in bin0.
    matrix<unsigned short, 4, 32> idx;
    idx.row(0) = (val == 0);
    idx.row(1) = (val == 1);
    idx.row(2) = (val == 2);
    idx.row(3) = (val == 3);
    // step 1 of prefix-sum. Sum up every pair of even and odd elements
    // and store the result in even position. In each step, we process 4 bins
    idx.select<4, 1, 16, 2>(0, 1) += idx.select<4, 1, 16, 2>(0, 0);
    // step 2
    idx.select<4, 1, 8, 4>(0, 2) += idx.select<4, 1, 8, 4>(0, 1);
    idx.select<4, 1, 8, 4>(0, 3) += idx.select<4, 1, 8, 4>(0, 1);
    // step 3
    // for a vector: 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
    // this step adds 3 to 4, 5, 6, 7 and adds 11 to 12, 13, 14, 15.
    // replicate<16,8,4,0>(0,3) duplicates 3rd, 11th, 19th, 27th 4 times each
    matrix<unsigned short, 4, 16> t;
    t = idx.replicate<16, 8, 4, 0>(0, 3);
    // Gen ISA describes only one destination stride. That is,
    // one instruction cannot write 4 consecutive elements and then write
    // another consecutive 4 elements with a stride distance. Due to this
    // restriction, a straightforward implementation has to break step 3
    // into 4 instructions, each adding 4 elements. we format matrix of
    // uw type into unsigned long long type. One unsigned long long has 4 uw.
    // The maximum value of prefix sum is 32, only i.e., every bit is set.
    // No overflow will happen during prefix sum computation. One long long
    // type add is equivalent to 4 uw additions. 16 additions of uw types
    // can be collapsed into 4 qword additions. What is more, one add
    // instruction can express those 4 qword additions without running into
    // the destination stride restriction.
    matrix_ref<unsigned long long, 4, 8> m1 = idx.format<unsigned long long, 4, 8>();
    matrix_ref<unsigned long long, 4, 4> t1 = t.format<unsigned long long, 4, 4>();
    m1.select<4, 1, 4, 2>(0, 1) += t1;

#pragma unroll
    for (int j = 0; j < 4; j++) {
      // step 4
      idx.select<1, 1, 8, 1>(j, 8) += idx(j, 7);
      idx.select<1, 1, 8, 1>(j, 24) += idx(j, 23);
      // step 5
      idx.select<1, 1, 16, 1>(j, 16) += idx(j, 15);
    }

    // calculate the positions of elements in their corresponding bins
    vector<unsigned int, 32> voff;
    // add bin0 element offsets to the bin0-batch-start
    voff.merge(idx.row(0) + next(0) - 1, val == 0);
    // add bin1 element offsets to the bin1-batch-start
    voff.merge(idx.row(1) + next(1) - 1, val == 1);
    // add bin2 element offsets to the bin2-batch-start
    voff.merge(idx.row(2) + next(2) - 1, val == 2);
    // add bin3 element offsets to the bin3-batch-start
    voff.merge(idx.row(3) + next(3) - 1, val == 3);

    // scatter write, 16-element each
    write(output, 0, voff.select<16, 1>(0), A.select<16,1>(0));
    write(output, 0, voff.select<16, 1>(16), A.select<16,1>(16));

    // update the next pointers, move onto the next 32 element
    next += n_elems;
  }
}
