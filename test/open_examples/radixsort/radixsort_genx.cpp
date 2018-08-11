/*
* Copyright (c) 2018, Intel Corporation
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
#include "radixsort.h"

/*
Inclusive Prefix Sum - 16 Wide
In the style of Kogge-Stone
*/
template <typename T> inline vector<T, 16> PrefixSumIn(vector<T, 16> in) {
  vector<T, 32> d = 0;
  d.select<16, 1>(16) = in;

  d.select<16, 1>(16) = d.select<16, 1>(16) + d.select<16, 1>(15);
  d.select<16, 1>(16) = d.select<16, 1>(16) + d.select<16, 1>(14);
  d.select<16, 1>(16) = d.select<16, 1>(16) + d.select<16, 1>(12);
  d.select<16, 1>(16) = d.select<16, 1>(16) + d.select<16, 1>(8);

  return d.select<16, 1>(16);
}

/*
Inclusive Prefix Sum - 32 Wide
In the style of Kogge-Stone
*/
template <typename T> inline vector<T, 32> PrefixSumIn(vector<T, 32> in) {
  vector<T, 48> d = 0;
  d.select<32, 1>(16) = in;

  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(15);
  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(14);
  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(12);
  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(8);
  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(0);

  return d.select<32, 1>(16);
}

/*
Exclusive Prefix Sum - 16 Wide
In the style of Kogge-Stone
*/
template <typename T> inline vector<T, 16> PrefixSumEx(vector<T, 16> in) {
  vector<T, 32> d = 0;
  d.select<16, 1>(16) = in;

  d.select<16, 1>(16) = d.select<16, 1>(16) + d.select<16, 1>(15);
  d.select<16, 1>(16) = d.select<16, 1>(16) + d.select<16, 1>(14);
  d.select<16, 1>(16) = d.select<16, 1>(16) + d.select<16, 1>(12);
  d.select<16, 1>(16) = d.select<16, 1>(16) + d.select<16, 1>(8);
  return d.select<16, 1>(15);
}


/*
Exclusive Prefix Sum - 32 Wide
In the style of Kogge-Stone
*/
template <typename T> inline vector<T, 32> PrefixSumEx(vector<T, 32> in) {
  vector<T, 48> d = 0;
  d.select<32, 1>(16) = in;

  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(15);
  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(14);
  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(12);
  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(8);
  d.select<32, 1>(16) = d.select<32, 1>(16) + d.select<32, 1>(0);

  return d.select<32, 1>(15);
}

/*
Inclusive Prefix Sum - 64 Wide
In the style of Kogge-Stone
*/
template <typename T> inline vector<T, 64> PrefixSumIn(vector<T, 64> in) {
  vector<T, 96> d = 0;
  d.select<64, 1>(32) = in;

  d.select<64, 1>(32) = d.select<64, 1>(32) + d.select<64, 1>(31);
  d.select<64, 1>(32) = d.select<64, 1>(32) + d.select<64, 1>(30);
  d.select<64, 1>(32) = d.select<64, 1>(32) + d.select<64, 1>(28);
  d.select<56, 1>(40) = d.select<56, 1>(40) + d.select<56, 1>(32);
  d.select<48, 1>(48) = d.select<48, 1>(48) + d.select<48, 1>(32);
  d.select<32, 1>(64) = d.select<32, 1>(64) + d.select<32, 1>(32);

  return d.select<64, 1>(32);
}

/*
Inclusive Prefix Sum - 128 Wide
In the style of Kogge-Stone
*/
template <typename T>
inline vector<T, 128> PrefixSumIn(vector<T, 128> in) {
  vector<T, 128> r;
  vector<T, 64> firstNibble = in.select<64, 1>(0);
  vector<T, 64> secondNibble = in.select<64, 1>(64);

  r.select<64, 1>(0) = PrefixSumIn(firstNibble);
  r.select<64, 1>(64) = PrefixSumIn(secondNibble);
  r.select<64, 1>(64) += r(63);
  return r;
}

/*
Returns the global id of the current thread taking
into account the workgroup id and size
*/
inline uint GetGlobalId() {
#ifdef CMRT_EMU
  return get_thread_origin_x();
#else
  return (cm_local_id(0) + cm_group_id(0) * WG_SIZE);
#endif
}

const ushort RADIX_INDEX[RADIX_SIZE] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

#if SIMD_WIDTH == 32
const ushort LANE_INDEX[32] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
#elif SIMD_WIDTH == 64
const ushort LANE_INDEX[64] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63 };
#else
const ushort LANE_INDEX[128] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127 };
#endif

/*
PerThreadHist is the first stage of the pipeline.

Its job is to take in its list of keys and output the
histogram.

Three cm functions make our job easy.
select - allows us to select all keys that match a mask (in this case the bin number)
cm_pack_mask - packs a vector into a integral datatype, we use this to pack the vector of flags for each bin
cm_cbit - counts the number of bits set in a integral datatype, this would return the number of keys in each bin
*/
extern "C" _GENX_MAIN_ void
PerThreadHist(
  uint numKeys,   // total number of keys to sort
  uint radixPos,  // identifier to which chunk of bits we are processing
  svmptr_t ibuf,  // full list of keys to sort
  svmptr_t obuf   // histogram of each bin.
) {
  uint global_id = GetGlobalId();

  const KeyType radixMask = (RADIX_SIZE - 1);
  vector<uint, RADIX_SIZE> hist = 0;

  // Read 
  uint keysPerThread = numKeys / GPU_THREADS;
  uint numIter = keysPerThread / SIMD_WIDTH;
  uint keyOffset = global_id * keysPerThread;

  for (uint k = 0; k < keysPerThread; k += SIMD_WIDTH) {
    svmptr_t keyAddr = ibuf + (keyOffset + k) * sizeof(KeyType);
    vector<KeyType, SIMD_WIDTH> keys;

#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 16; i++) {
      cm_svm_block_read(keyAddr, keys.select<16, 1>(i * 16));
      keyAddr += 16 * sizeof(KeyType);
    }

    vector<ushort, SIMD_WIDTH> keys16;
    keys16 = (keys >> KeyType(radixPos)) & radixMask;

#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 32; i++) {
      vector<uint, RADIX_SIZE> mask;

#pragma unroll
      for (uint bin = 0; bin < RADIX_SIZE; bin++) {
        vector<ushort, 32> keyMask = (keys16.select<32, 1>(i * 32) == bin);
        mask(bin) = cm_pack_mask(keyMask);
      }

      hist += cm_cbit(mask);
    }

  }

  svmptr_t histAddr = obuf + global_id * sizeof(uint) * RADIX_SIZE;
  cm_svm_block_write(histAddr, hist);
}


/*
ScanHist is the second phase of the pipeline.

This phase is intended to be ran in one thread. We need
to read all of the histograms from the previous phase and
output the global bin offsets and the individual thread
offsets.
*/
extern "C" _GENX_MAIN_ void
ScanHist(
  svmptr_t ibuf, // input buffer that contains the histograms of phase 1. 
  svmptr_t obuf  // output buffer contains the thread offsets and the last row is the global hist.
) {
  vector<uint, RADIX_SIZE> ghist = 0;

  for (uint i = 0; i < GPU_THREADS; i++) {
    vector<uint, RADIX_SIZE> histr;
    svmptr_t histRdAddr = ibuf + i * sizeof(uint) * RADIX_SIZE;
    cm_svm_block_read(histRdAddr, histr);
    ghist += histr;

    // Output thread hist
    svmptr_t histWrAddr = obuf + i * sizeof(uint) * RADIX_SIZE;
    cm_svm_block_write(histWrAddr, ghist);
  }

  ghist = PrefixSumEx(ghist);

  // Output global hist
  svmptr_t histWrAddr = obuf + GPU_THREADS * sizeof(uint) * RADIX_SIZE;
  cm_svm_block_write(histWrAddr, ghist);
}

/*
ScanKeysUncoalesced is a variant of the third phase.

This version of the third phase is designed to work for platforms that dont support SLM operations.
Its task is to calculate the final position and write out each key.

This output of this phase is a list of keys sorted by the radixPos
*/
extern "C" _GENX_MAIN_ void
ScanKeysUncoalesced(
  uint numKeys,       // total number of keys to sort
  uint radixPos,      // identifier to show which chunk of bits we are processing
  svmptr_t ibuf,      // full list of keys 
  svmptr_t ibufHist,  // the indivial thread offsets and global offset
  svmptr_t obuf       // sorted list of keys
) {
  uint global_id = GetGlobalId();

  vector<uint, RADIX_SIZE> ghist;

  svmptr_t histAddr = ibufHist + GPU_THREADS * sizeof(uint) * RADIX_SIZE;
  cm_svm_block_read(histAddr, ghist);

  vector<uint, RADIX_SIZE> hist = 0;
  if (global_id > 0) {
    histAddr = ibufHist + (global_id - 1) * sizeof(uint) * RADIX_SIZE;
    cm_svm_block_read(histAddr, hist);
  }

  vector<uint, RADIX_SIZE> binBaseIndex = ghist + hist;

  const KeyType radixMask = (RADIX_SIZE - 1);

  uint keysPerThread = numKeys / GPU_THREADS;
  uint numIter = keysPerThread / SIMD_WIDTH;
  uint keyOffset = global_id * keysPerThread;

  for (uint k = 0; k < keysPerThread; k += SIMD_WIDTH) {
    svmptr_t keyAddr = ibuf + (keyOffset + k) * sizeof(KeyType);

    vector<KeyType, SIMD_WIDTH> keys;
#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 16; i++) {
      cm_svm_block_read(keyAddr, keys.select<16, 1>(i * 16));
      keyAddr += 16 * sizeof(KeyType);
    }

    vector<uchar, SIMD_WIDTH> keys16;
    keys16 = (keys >> KeyType(radixPos)) & radixMask;

    vector<uint, SIMD_WIDTH> wrIndex = 0;

#pragma unroll
    for (ushort bin = 0; bin < RADIX_SIZE; bin += 2) {
      vector<ushort, SIMD_WIDTH> keyMask;

      vector_ref<uchar, 2 * SIMD_WIDTH> keyMaskAll = keyMask.format<uchar>();
      vector_ref<uchar, SIMD_WIDTH> keyMaskEvn = keyMaskAll.select<SIMD_WIDTH, 2>(0);
      vector_ref<uchar, SIMD_WIDTH> keyMaskOdd = keyMaskAll.select<SIMD_WIDTH, 2>(1);

      keyMaskEvn = (keys16 == bin);
      keyMaskOdd = (keys16 == (bin + 1));

      vector<ushort, SIMD_WIDTH> keyScanIn = PrefixSumIn(keyMask);
      vector<ushort, SIMD_WIDTH> keyScanEx = keyScanIn - keyMask;

      vector_ref<uchar, 2 * SIMD_WIDTH> keyScanInAll = keyScanIn.format<uchar>();
      vector_ref<uchar, 2 * SIMD_WIDTH> keyScanExAll = keyScanEx.format<uchar>();

      vector_ref<uchar, SIMD_WIDTH> keyScanExEvn = keyScanExAll.select<SIMD_WIDTH, 2>(0);
      vector_ref<uchar, SIMD_WIDTH> keyScanExOdd = keyScanExAll.select<SIMD_WIDTH, 2>(1);

      uint baseIndex;
      vector<uint, SIMD_WIDTH> keyDestAddr;

      baseIndex = binBaseIndex(bin);
      keyDestAddr = keyScanExEvn + baseIndex;
      wrIndex.merge(keyDestAddr, keyMaskEvn);

      baseIndex = binBaseIndex(bin + 1);
      keyDestAddr = keyScanExOdd + baseIndex;
      wrIndex.merge(keyDestAddr, keyMaskOdd);

      binBaseIndex.select<2, 1>(bin) += keyScanInAll.select<2, 1>(SIMD_WIDTH * 2 - 2);
    }

    vector <svmptr_t, SIMD_WIDTH> wrAddr = obuf + wrIndex * sizeof(KeyType);

#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 16; i++) {
      vector <svmptr_t, 16> wrAddrh = wrAddr.select<16, 1>(i * 16);
      cm_svm_scatter_write(wrAddrh, keys.select<16, 1>(i * 16));
    }
  }
}

/*
ScanKeys is a variant of the third phase.

This version of the third phase is designed to work for platforms that support SLM operations.
Its task is to calculate the final position and write out each key.

This output of this phase is a list of keys sorted by the radixPos
*/
extern "C" _GENX_MAIN_ void
ScanKeys(
  uint numKeys,       // total number of keys to sort
  uint radixPos,      // identifier to show which chunk of bits we are processing
  svmptr_t ibuf,      // full list of keys 
  svmptr_t ibufHist,  // the indivial thread offsets and global offset
  svmptr_t obuf       // sorted list of keys
) {
  uint global_id = GetGlobalId();
  vector<ushort, SIMD_WIDTH> laneIndex = LANE_INDEX;

#ifdef CMRT_EMU
  const uint SLM_SIZE = SIMD_WIDTH * sizeof(KeyType);
  ushort slmBase = 0;
#else
  const uint SLM_SIZE = SIMD_WIDTH * sizeof(KeyType) * WG_SIZE;
  ushort slmBase = SIMD_WIDTH * cm_local_id(0);
#endif

  cm_slm_init(SLM_SIZE);
  uint slm = cm_slm_alloc(SLM_SIZE);

  // Read
  const KeyType radixMask = (RADIX_SIZE - 1);

  vector<uint, RADIX_SIZE> ghist;

  svmptr_t histAddr = ibufHist + GPU_THREADS * sizeof(uint) * RADIX_SIZE;
  cm_svm_block_read(histAddr, ghist);

  vector<uint, RADIX_SIZE> hist = 0;
  if (global_id > 0) {
    histAddr = ibufHist + (global_id - 1) * sizeof(uint) * RADIX_SIZE;
    cm_svm_block_read(histAddr, hist);
  }

  vector<uint, RADIX_SIZE> binBaseIndex = ghist + hist;

  uint keysPerThread = numKeys / GPU_THREADS;
  uint numIter = keysPerThread / SIMD_WIDTH;
  uint keyOffset = global_id * keysPerThread;

  for (uint k = 0; k < keysPerThread; k += SIMD_WIDTH) {
    svmptr_t keyAddr = ibuf + (keyOffset + k) * sizeof(KeyType);

    vector<KeyType, SIMD_WIDTH> keys;
#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 16; i++) {
      cm_svm_block_read(keyAddr, keys.select<16, 1>(i * 16));
      keyAddr += 16 * sizeof(KeyType);
    }

    vector<uchar, SIMD_WIDTH> keys16;
    keys16 = (keys >> KeyType(radixPos)) & radixMask;

    vector<uint, SIMD_WIDTH> wrIndexMem;
    vector<ushort, SIMD_WIDTH> wrIndexSLM;
    ushort baseIndexSLM = 0;

#pragma unroll
    for (ushort bin = 0; bin < RADIX_SIZE; bin += 2) {
      vector<ushort, SIMD_WIDTH> keyMask;
      vector_ref<uchar, 2 * SIMD_WIDTH> keyMaskAll = keyMask.format<uchar>();
      vector_ref<uchar, SIMD_WIDTH> keyMaskEvn = keyMaskAll.select<SIMD_WIDTH, 2>(0);
      vector_ref<uchar, SIMD_WIDTH> keyMaskOdd = keyMaskAll.select<SIMD_WIDTH, 2>(1);

      keyMaskEvn = (keys16 == bin);
      keyMaskOdd = (keys16 == (bin + 1));

      vector<ushort, SIMD_WIDTH> keyScanIn = PrefixSumIn(keyMask);
      vector<ushort, SIMD_WIDTH> keyScanEx = keyScanIn - keyMask;

      vector_ref<uchar, 2 * SIMD_WIDTH> keyScanInAll = keyScanIn.format<uchar>();
      vector_ref<uchar, 2 * SIMD_WIDTH> keyScanExAll = keyScanEx.format<uchar>();
      vector_ref<uchar, SIMD_WIDTH> keyScanExEvn = keyScanExAll.select<SIMD_WIDTH, 2>(0);
      vector_ref<uchar, SIMD_WIDTH> keyScanExOdd = keyScanExAll.select<SIMD_WIDTH, 2>(1);

      vector<ushort, SIMD_WIDTH> keyAddrSLM;
      vector<uint, SIMD_WIDTH> keyAddrMem;
      vector<ushort, SIMD_WIDTH> maskMem;

      keyAddrSLM = keyScanExEvn + baseIndexSLM;
      wrIndexSLM.merge(keyAddrSLM, keyMaskEvn);

      maskMem = (laneIndex >= baseIndexSLM);
      keyAddrMem = laneIndex - baseIndexSLM + binBaseIndex(bin);
      wrIndexMem.merge(keyAddrMem, maskMem);

      baseIndexSLM += keyScanInAll(SIMD_WIDTH * 2 - 2);
      keyAddrSLM = keyScanExOdd + baseIndexSLM;
      wrIndexSLM.merge(keyAddrSLM, keyMaskOdd);

      maskMem = (laneIndex >= baseIndexSLM);
      keyAddrMem = laneIndex - baseIndexSLM + binBaseIndex(bin + 1);
      wrIndexMem.merge(keyAddrMem, maskMem);

      baseIndexSLM += keyScanInAll(SIMD_WIDTH * 2 - 1);
      binBaseIndex.select<2, 1>(bin) += keyScanInAll.select<2, 1>(SIMD_WIDTH * 2 - 2);
    }

#if KEY_DIGITS == 64
    wrIndexSLM = (wrIndexSLM + slmBase) * 2;
    // try changing number of iterations, try 4 iterations and 2
#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 16; i++) {
      vector<uint, 32> keysDW = keys.select<16, 1>(i * 16).format<uint>();
      vector<uint, 32> keysSoA = keysDW.replicate<2, 1, 16, 2>(0);
      cm_slm_write4(slm, wrIndexSLM.select<16, 1>(i * 16), keysSoA, SLM_GR_ENABLE);
    }
    vector <ushort, SIMD_WIDTH> rdIndexSLM = (laneIndex + slmBase) * 2;
#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 16; i++) {
      vector<uint, 32> keysSoA;
      cm_slm_read4(slm, rdIndexSLM.select<16, 1>(i * 16), keysSoA, SLM_GR_ENABLE);

      vector<uint, 32> keysDW;
      keysDW.select<16, 2>(0) = keysSoA.select<16, 1>(0);
      keysDW.select<16, 2>(1) = keysSoA.select<16, 1>(16);

      keys.select<16, 1>(i * 16) = keysDW.format<KeyType>();
    }

#else
    wrIndexSLM = wrIndexSLM + slmBase;
#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 16; i++) {
      cm_slm_write(slm, wrIndexSLM.select<16, 1>(i * 16), keys.select<16, 1>(i * 16));
    }

    vector <ushort, SIMD_WIDTH> rdIndexSLM = laneIndex + slmBase;
#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 16; i++) {
      cm_slm_read(slm, rdIndexSLM.select<16, 1>(i * 16), keys.select<16, 1>(i * 16));
    }
#endif

    vector <svmptr_t, SIMD_WIDTH> wrAddr = obuf + wrIndexMem * sizeof(KeyType);
#pragma unroll
    for (uint i = 0; i < SIMD_WIDTH / 16; i++) {
      vector <svmptr_t, 16> wrAddrh = wrAddr.select<16, 1>(i * 16);
      cm_svm_scatter_write(wrAddrh, keys.select<16, 1>(i * 16));
    }
  }
}
