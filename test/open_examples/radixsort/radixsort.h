#pragma once

//#define x64_MODE

#ifdef x64_MODE
  #define SIMD_WIDTH 64
  #define KEY_DIGITS 64
  using KeyType = uint64_t;
#else
  #define SIMD_WIDTH 64
  #define KEY_DIGITS 32
  using KeyType = uint32_t;
#endif

const uint32_t  GPU_THREADS = 256;
const uint32_t  GPU_WORK_ITEMS = (GPU_THREADS * SIMD_WIDTH);

const uint32_t  RADIX_DIGITS = 4;
const uint32_t  RADIX_SIZE = 1 << RADIX_DIGITS;

const uint32_t  WG_SIZE = 4;

#define SLM_COALESCE
