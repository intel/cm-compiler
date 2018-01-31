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

/*
 * To avoid loading/storing excessive data from/to memory, the
 * implementation of the bitonic sort here tries to take advantage of GRF
 * space and do as much work as possible locally without going through memory.
 * The algorithm is implemented using 2 kernels, cmk_bitonic_sort_256 and
 * cmk_bitonic_merge. Given an input, the algorithm first divides the data
 * into 256-element chunks sorted by each HW threads. Since 256 elements are
 * loaded into GRFs, swapping elements within a chunk leverages
 * the expressiveness of Gen register regioning. Once cm_bitonic_sort_256
 * is complete, two neighboring segments of 256-element form a bitonic order.
 * Cmk_bitonic_merge takes two 256-chunks and performs swapping elements
 * based on the sorting order directions, ascending or descending.
 */

template<typename ty, unsigned int size>
inline _GENX_ void cmk_read (SurfaceIndex index, unsigned int offset, vector_ref<ty, size> v)
{

#pragma unroll
  for (unsigned int i = 0; i < size; i += 32) {
    read (index, offset + i * sizeof (ty), v.template select<32, 1> (i));
  }
}

template<typename ty, unsigned int size>
inline _GENX_ void cmk_write (SurfaceIndex index, unsigned int offset, vector_ref<ty, size> v)
{
#pragma unroll
  for (unsigned int i = 0; i < size; i += 32) {
    write (index, offset + i * sizeof (ty), v.template select<32, 1> (i));
  }
}

const uchar init_mask1[32] = {
  0, 1, 1, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 1, 1, 0
};
const uchar init_mask2[32] = {
  0, 0, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0
};
const uchar init_mask3[32] = {
  0, 1, 0, 1, 1, 0, 1, 0,
  0, 1, 0, 1, 1, 0, 1, 0,
  0, 1, 0, 1, 1, 0, 1, 0,
  0, 1, 0, 1, 1, 0, 1, 0
};
const uchar init_mask4[32] = {
  0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 0, 0, 0, 0
};
const uchar init_mask5[32] = {
  0, 0, 1, 1, 0, 0, 1, 1,
  1, 1, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 1, 1,
  1, 1, 0, 0, 1, 1, 0, 0
};
const uchar init_mask6[32] = {
  0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0
};
const uchar init_mask7[32] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0
};
const uchar init_mask8[32] = {
  0, 0, 0, 0, 1, 1, 1, 1,
  0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0
};
const uchar init_mask9[32] = {
  0, 0, 1, 1, 0, 0, 1, 1,
  0, 0, 1, 1, 0, 0, 1, 1,
  1, 1, 0, 0, 1, 1, 0, 0,
  1, 1, 0, 0, 1, 1, 0, 0
};
const uchar init_mask10[32] = {
  0, 1, 0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0
};
const uchar init_mask11[32] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1
};
const uchar init_mask12[32] = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};
const uchar init_mask13[32] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1
};
const uchar init_mask14[32] = {
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,

};
const uchar init_mask15[32] = {
  0, 0, 0, 0, 1, 1, 1, 1,
  0, 0, 0, 0, 1, 1, 1, 1,
  0, 0, 0, 0, 1, 1, 1, 1,
  0, 0, 0, 0, 1, 1, 1, 1
};
const uchar init_mask16[32] = {
  1, 1, 1, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0
};
const uchar init_mask17[32] = {
  0, 0, 1, 1, 0, 0, 1, 1,
  0, 0, 1, 1, 0, 0, 1, 1,
  0, 0, 1, 1, 0, 0, 1, 1,
  0, 0, 1, 1, 0, 0, 1, 1
};
const uchar init_mask18[32] = {
  1, 1, 0, 0, 1, 1, 0, 0,
  1, 1, 0, 0, 1, 1, 0, 0,
  1, 1, 0, 0, 1, 1, 0, 0,
  1, 1, 0, 0, 1, 1, 0, 0
};
const uchar init_mask19[32] = {
  0, 1, 0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1, 0, 1
};
const uchar init_mask20[32] = {
  1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0
};
#define BASE_SZ 256
#define MAX_TS_WIDTH 512


// Function bitonic_exchange{1,2,4,8} compares and swaps elements with
// the particular strides
inline _GENX_ void bitonic_exchange8(vector_ref<unsigned int, BASE_SZ> A, vector_ref<unsigned int, BASE_SZ> B, vector_ref<uchar, 32> flip) {
#pragma unroll
  for (int i = 0; i < BASE_SZ; i += 32) {
    B.select<8, 1>(i) = A.select<8, 1>(i + 8);
    B.select<8, 1>(i + 8) = A.select<8, 1>(i);
    B.select<8, 1>(i + 16) = A.select<8, 1>(i + 24);
    B.select<8, 1>(i + 24) = A.select<8, 1>(i + 16);
    B.select<32, 1>(i).merge(A.select<32, 1>(i), (A.select<32, 1>(i) < B.select<32, 1>(i)) ^ flip);
  }
}
inline _GENX_ void bitonic_exchange4(vector_ref<unsigned int, BASE_SZ> A, vector_ref<unsigned int, BASE_SZ> B, vector_ref<uchar, 32> flip) {
#pragma unroll
  for (int i = 0; i < BASE_SZ; i += 32) {
    matrix_ref<unsigned int, 4, 8> MA = A.select<32, 1>(i).format<unsigned int, 4, 8>();
    matrix_ref<unsigned int, 4, 8> MB = B.select<32, 1>(i).format<unsigned int, 4, 8>();
    MB.select<4, 1, 4, 1>(0, 0) = MA.select<4, 1, 4, 1>(0, 4);
    MB.select<4, 1, 4, 1>(0, 4) = MA.select<4, 1, 4, 1>(0, 0);
    B.select<32, 1>(i).merge(A.select<32, 1>(i), (A.select<32, 1>(i) < B.select<32, 1>(i)) ^ flip);
  }
}

// The implementation of bitonic_exchange2 is similar to bitonic_exchange1.
// The only difference is stride distance and different flip vector.
// However the shuffling data patterns for stride 2 cannot be expressed
// concisely with Gen register regioning.
// we format vector A and B into matrix_ref<long long, 4, 4>.
// MB.select<4, 1, 2, 2>(0, 0) can be mapped to destination region well.
//   MB.select<4, 1, 2, 2>(0, 0) = MA.select<4, 1, 2, 2>(0, 1);
// is compiled to
//   mov(4) r34.0<2>:q r41.1<2; 1, 0>:q { Align1, Q1 }
//   mov(4) r36.0<2>:q r8.1<2; 1, 0>:q { Align1, Q1 }
//   mov(4) r34.1<2>:q r41.0<2; 1, 0>:q { Align1, Q1 }
//   mov(4) r36.1<2>:q r8.0<2; 1, 0>:q { Align1, Q1 }
// each mov copies four 64-bit data, which is 4X SIMD efficiency
// improvement over the straightforward implementation.
inline _GENX_ void bitonic_exchange2(vector_ref<unsigned int, BASE_SZ> A, vector_ref<unsigned int, BASE_SZ> B,  vector_ref<uchar, 32> flip) {
#pragma unroll
  for (int i = 0; i < BASE_SZ; i += 32) {
    matrix_ref<long long, 4, 4> MB = B.select<32, 1>(i).format<long long, 4, 4>();
    matrix_ref<long long, 4, 4> MA = A.select<32, 1>(i).format<long long, 4, 4>();

    MB.select<4, 1, 2, 2>(0, 0) = MA.select<4, 1, 2, 2>(0, 1);
    MB.select<4, 1, 2, 2>(0, 1) = MA.select<4, 1, 2, 2>(0, 0);
    B.select<32, 1>(i).merge(A.select<32, 1>(i), (A.select<32, 1>(i) < B.select<32, 1>(i)) ^ flip);
  }
}

inline _GENX_ void bitonic_exchange1(vector_ref<unsigned int, BASE_SZ> A, vector_ref<unsigned int, BASE_SZ> B,  vector_ref<uchar, 32> flip) {
#pragma unroll
  // each thread is handling 256-element chunk. Each iteration
  // compares and swaps two 32 elements
  for (int i = 0; i < BASE_SZ; i += 32) {
    // The first step is to select A's odd-position elements,
    // indicated by A.select<16,2>(i), which selects 16 elements
    // with stride 2 starting from location A[i], and copies
    // the selected elements to B[i] location with stride 2.
    vector_ref<unsigned, 32>T = B.select<32, 1>(i);
    T.select<16, 2>(0) = A.select<16, 2>(i + 1);
    // The next step selects 16 even-position elements starting
    // from A[i+1] and copies them over to B's odd positions
    // starting at B[i+1]. After the first two steps,
    // all even-odd pair elements are swapped.
    T.select<16, 2>(1) = A.select<16, 2>(i);
    // The final step determines if the swapped pairs in B are
    // the desired order and should be preserved. If not, their values
    // are overwritten by their corresponding original values
    // (before swapping). The comparisons determine which elements
    // in B already meet the sorting order requirement and which are not.
    // Consider the first two elements of A & B, B[0] and B[1] is
    // the swap of A[0] and A[1]. Element-wise < comparison tells
    // that A[0] < B[0], i.e., A[0] < A[1]. Since the desired sorting
    // order is A[0] < A[1], however, we already swap the two values
    // as we copy A to B. The XOR operation is to set the condition to
    // indicate which elements in original vector A have the right sorting
    // order. Those elements are then merged from A to B based on their
    // corresponding conditions. Consider B[2] and B[3] in this case.
    // The order already satisfies the sorting order. The flip vector
    // passed to this stage is [0,1,1,0,0,1,1,0]. The flip bit of B[2]
    // resets the condition so that the later merge operation preserves
    // B[2] and won't copy from A[2].
    B.select<32, 1>(i).merge(A.select<32, 1>(i), (A.select<32, 1>(i) < T) ^ flip);
  }
}

// bitonic_merge for stage m has recursive steps to compare and swap
// elements with stride 1 << m, 1 << (m - 1), ... , 8, 4, 2, 1.
// bitonic_merge is GRF based implementation that handles stride
// 1 to 128 compare - and - swap steps.For stride <= 128, 256 data
// items are kept in GRF. All compare-and-swap can all be completely
// done with GRF locally. Doing so avoids global synchronizations
// and repeating loads/stores. Parameter n indicates that bitonic_merge
// is handling stride 1 << n for bitonic stage m.
inline _GENX_ void bitonic_merge(unsigned int offset, vector_ref<unsigned int, BASE_SZ> A, unsigned int n, unsigned int m)
{
  // dist is the stride distance for compare-and-swap
  unsigned int dist = 1 << n;
  // number of exchange passes we need
  // this loop handles stride distance 128 down to 16. Each iteration
  // the distance is halved. Due to data access patterns of stride
  // 8, 4, 2 and 1 are within one GRF, those stride distance are handled
  // by custom tailored code to take advantage of register regioning.
  for (int k = 0; k < n-3; k++, dist >>= 1) {
    // Each HW thread process 256 data elements. For a given stride
    // distance S, 256 elements are divided into 256/(2*S) groups.
    // within each group, two elements with distance S apart are
    // compared and swapped based on sorting direction.
    // This loop basically iterates through each group.
    for (int i = 0; i < BASE_SZ; i += dist * 2) {
      // Every bitonic stage, we need to maintain bitonic sorting order.
      // Namely, data are sorted into alternating ascending and descending
      // fashion. As show in Figure 9, the light blue background regions
      // are in ascending order, the light green background regions in
      // descending order. Whether data are in ascending or descending
      // regions depends on their position and the current bitonic stage
      // they are in. "offset+i" the position. For stage m, data of
      // chunks of 1<<(m+1) elements in all the stride steps have the
      // same order.
      bool dir_up = (((offset + i) >> (m + 1)) & 1) == 0;
      // each iteration swap 2 16-element chunks
      for (int j = 0; j < dist>>4; j++) {
        vector<unsigned int, 16> T = A.select<16, 1>(i + j * 16);
        vector_ref<unsigned int, 16> T1 = A.select<16, 1>(i + j * 16);
        vector_ref<unsigned int, 16> T2 = A.select<16, 1>(i + j * 16 + dist);
        if (dir_up) {
          T1.merge(T2, T2 < T1);
          T2.merge(T, T > T2);
        }
        else {
          T1.merge(T2, T2 > T1);
          T2.merge(T, T < T2);
        }
      }
    }
  }

  // Stride 1, 2, 4, and 8 in bitonic_merge are custom tailored to
  // take advantage of register regioning. The implementation is
  // similar to bitonic_exchange{1,2,4,8}.

  // exchange 8
  vector<uchar, 32> flip13(init_mask13);
  vector<uchar, 32> flip14(init_mask14);
  vector<unsigned int, BASE_SZ> B;
  for (int i = 0; i < BASE_SZ; i += 32) {
    B.select<8, 1>(i) = A.select<8, 1>(i + 8);
    B.select<8, 1>(i + 8) = A.select<8, 1>(i);
    B.select<8, 1>(i + 16) = A.select<8, 1>(i + 24);
    B.select<8, 1>(i + 24) = A.select<8, 1>(i + 16);
    bool dir_up = (((offset + i) >> (m + 1)) & 1) == 0;
    if (dir_up)
      B.select<32, 1>(i).merge(A.select<32, 1>(i), (A.select<32, 1>(i) < B.select<32, 1>(i)) ^ flip13);
    else
      B.select<32, 1>(i).merge(A.select<32, 1>(i), (A.select<32, 1>(i) < B.select<32, 1>(i)) ^ flip14);
  }

  // exchange 4
  vector<uchar, 32> flip15(init_mask15);
  vector<uchar, 32> flip16(init_mask16);
#pragma unroll
  for (int i = 0; i < BASE_SZ; i += 32) {
    matrix_ref<unsigned int, 4, 8> MA = A.select<32, 1>(i).format<unsigned int, 4, 8>();
    matrix_ref<unsigned int, 4, 8> MB = B.select<32, 1>(i).format<unsigned int, 4, 8>();
    MA.select<4, 1, 4, 1>(0, 0) = MB.select<4, 1, 4, 1>(0, 4);
    MA.select<4, 1, 4, 1>(0, 4) = MB.select<4, 1, 4, 1>(0, 0);
    bool dir_up = (((offset + i) >> (m + 1)) & 1) == 0;
    if (dir_up)
      A.select<32, 1>(i).merge(B.select<32, 1>(i), (B.select<32, 1>(i) < A.select<32, 1>(i)) ^ flip15);
    else
      A.select<32, 1>(i).merge(B.select<32, 1>(i), (B.select<32, 1>(i) < A.select<32, 1>(i)) ^ flip16);

  }

  // exchange 2
  vector<uchar, 32> flip17(init_mask17);
  vector<uchar, 32> flip18(init_mask18);
#pragma unroll
  for (int i = 0; i < BASE_SZ; i += 32) {
    matrix_ref<long long, 4, 4> MB = B.select<32, 1>(i).format<long long, 4, 4>();
    matrix_ref<long long, 4, 4> MA = A.select<32, 1>(i).format<long long, 4, 4>();

    MB.select<4, 1, 2, 2>(0, 0) = MA.select<4, 1, 2, 2>(0, 1);
    MB.select<4, 1, 2, 2>(0, 1) = MA.select<4, 1, 2, 2>(0, 0);
    bool dir_up = (((offset + i) >> (m + 1)) & 1) == 0;
    if (dir_up)
      B.select<32, 1>(i).merge(A.select<32, 1>(i), (A.select<32, 1>(i) < B.select<32, 1>(i)) ^ flip17);
    else
      B.select<32, 1>(i).merge(A.select<32, 1>(i), (A.select<32, 1>(i) < B.select<32, 1>(i)) ^ flip18);
  }
  // exchange 1
  vector<uchar, 32> flip19(init_mask19);
  vector<uchar, 32> flip20(init_mask20);
#pragma unroll
  // Each iteration compares and swaps 2 32-element chunks
  for (int i = 0; i < BASE_SZ; i += 32) {
    // As aforementioned in bitonic_exchange1.
    // switch even and odd elements of B and put them in A.
    vector_ref<unsigned, 32> T = A.select<32, 1>(i);
    T.select<16, 2>(0) = B.select<16, 2>(i + 1);
    T.select<16, 2>(1) = B.select<16, 2>(i);
    // determine whether data are in ascending or descending regions
    // depends on their position and the current bitonic stage
    // they are in. "offset+i" is the position. For stage m,
    // data of chunks of 1<<(m+1) elements in all the stride steps
    // have the same order. For instance, in stage 4, all first 32 elements
    // are in ascending order and the next 32 elements are in descending
    // order. "&1" determines the alternating ascending and descending order.
    bool dir_up = (((offset + i) >> (m + 1)) & 1) == 0;
    // choose flip vector based on the direction (ascending or descending).
    // Compare and swap
    if (dir_up)
      A.select<32, 1>(i).merge(B.select<32, 1>(i), (B.select<32, 1>(i) < T) ^ flip19);
    else
      A.select<32, 1>(i).merge(B.select<32, 1>(i), (B.select<32, 1>(i) < T) ^ flip20);
  }
}

// sorting 256 elements in ascending or descending order
_GENX_MAIN_ void cmk_bitonic_sort_256 (SurfaceIndex index1, SurfaceIndex index2)
{
  uint h_pos = get_thread_origin_x() + get_thread_origin_y()*MAX_TS_WIDTH;
  unsigned int offset = (h_pos * BASE_SZ) << 2;

  // The first few stages are implemented with double buffers, A and B,
  // which reside in GRF.The output of a stride exchange step is fed
  // into the next exchange step as the input.cmk_read loads a 256-element
  // chunk starting at offset into vector A. The flip vectors basically
  // indicate what the desired sorting order for swapping.
  vector<unsigned int, BASE_SZ> A;
  vector<unsigned int, BASE_SZ> B;
  cmk_read<unsigned int, BASE_SZ> (index1, offset, A);

  vector<uchar, 32> flip1(init_mask1);

  vector<unsigned short, 32> mask;
  // stage 0
  bitonic_exchange1(A, B, flip1);
  // stage 1
  vector<uchar, 32> flip2(init_mask2);
  vector<uchar, 32> flip3(init_mask3);
  bitonic_exchange2(B, A, flip2);
  bitonic_exchange1(A, B, flip3);
  // stage 2
  vector<uchar, 32> flip4(init_mask4);
  vector<uchar, 32> flip5(init_mask5);
  vector<uchar, 32> flip6(init_mask6);
  bitonic_exchange4(B, A, flip4);
  bitonic_exchange2(A, B, flip5);
  bitonic_exchange1(B, A, flip6);
  // stage 3
  vector<uchar, 32> flip7(init_mask7);
  vector<uchar, 32> flip8(init_mask8);
  vector<uchar, 32> flip9(init_mask9);
  vector<uchar, 32> flip10(init_mask10);
  bitonic_exchange8(A, B, flip7);
  bitonic_exchange4(B, A, flip8);
  bitonic_exchange2(A, B, flip9);
  bitonic_exchange1(B, A, flip10);
  // stage 4,5,6,7 use generic bitonic_merge routine
  for (int i = 4; i < 8; i++)
    bitonic_merge(h_pos*BASE_SZ, A, i, i);

  // cmk_write writes out sorted data to the output buffer.
  cmk_write<unsigned int, BASE_SZ>(index2, offset, A);

}

_GENX_MAIN_ void cmk_bitonic_merge(SurfaceIndex index, unsigned int n, unsigned int m)
{
  // threads are mapped to a 2D space. take 2D origin (x,y) and unfold them
  // to get the thread position in 1D space. use tid read the data chunks
  // the thread needs to read from the index surface
  uint tid = get_thread_origin_x() + get_thread_origin_y()*MAX_TS_WIDTH;
  // which 2-to-(n+1) segment the thread needs to work on
  // each thread swap two 256-element blocks.
  unsigned int seg = tid / (1 << (n - 8));
  unsigned int seg_sz = 1 << (n + 1);
  // calculate the offset of the data this HW is reading. seg*seg_sz is
  // the starting address of the segment the thread is in. As aforementioned,
  // each segment needs 1<<(n-8) threads. tid%(1<<(n-8) which 256-element
  // chunk within the segment this HW thread is processing.
  unsigned int offset = (seg * seg_sz + (tid % (1 << (n - 8))*BASE_SZ));
  // stride distance
  unsigned int dist = 1 << n;
  // determine whether data are in ascending or descending regions depends on
  // their position and the current bitonic stage they are in.
  // "offset" is the position. For stage m, data of chunks of 1<<(m+1)
  // elements in all the stride steps have the same order.
  // "&1" determines the alternating ascending and descending order.
  bool dir_up = ((offset >> (m + 1)) & 1) == 0;
  // read oword 32 elements each time
  vector<unsigned int, BASE_SZ> A;
  vector<unsigned int, BASE_SZ> B;

#pragma unroll
  for (int i = 0; i < BASE_SZ; i += 32) {
    // byte offset
    cmk_read<unsigned int, 32>(index, (offset + i) << 2, A.select<32,1>(i));
    cmk_read<unsigned int, 32>(index, (offset + i + dist) << 2, B.select<32, 1>(i));
    // compare 32 elements at a time and merge the result based on
    // the sorting direction
    vector<unsigned int, 32> T = A.select<32, 1>(i);
    if (dir_up) {
      A.select<32, 1>(i).merge(B.select<32, 1>(i), B.select<32, 1>(i) < A.select<32, 1>(i));
      B.select<32, 1>(i).merge(T, T > B.select<32, 1>(i));
    }
    else {
      A.select<32, 1>(i).merge(B.select<32, 1>(i), B.select<32, 1>(i) > A.select<32, 1>(i));
      B.select<32, 1>(i).merge(T, T < B.select<32, 1>(i));
    }
  }
  // Once stride distance 256 is reached, all subsequent recursive steps
  // (n = 7, 6, ..., 1) can be resolved locally as all data reside
  // in vector A and B. Thus, reduce the overhead of returning back to
  // the host side and relaunch tasks. Also writing data back to
  // memory and reading it back is avoided. bitonic_merge is
  // the routine explained earlier.
  if (n == 8) {
    // Vector A has 256 elements. Call bitonic_merge to process
    // the remaining stride distance for A. A's sorted result is
    // immediately written out to memory. Doing so avoids spilling
    // because A's lifetime ends without interfering with
    // bitonic_merge(... B ...)
    bitonic_merge(offset, A, 7, m);
    cmk_write<unsigned int, BASE_SZ>(index, offset << 2, A);
    bitonic_merge(offset + dist, B, 7, m);
    cmk_write<unsigned int, BASE_SZ>(index, (offset + dist) << 2, B);
  }
  else {
    cmk_write<unsigned int, BASE_SZ>(index, offset << 2, A);
    cmk_write<unsigned int, BASE_SZ>(index, (offset + dist) << 2, B);
  }
}
