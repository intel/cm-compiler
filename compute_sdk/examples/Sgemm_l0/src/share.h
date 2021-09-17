/*========================== begin_copyright_notice ============================

Copyright (C) 2017-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef share_h
#define share_h

#define HETER

#if !defined(GEN_KERNEL)
#define fptype float
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#ifdef __GNUC__
#include <unistd.h>

static __inline__ unsigned long long __rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#else
#include <windows.h>
#endif

using namespace std;

static float randData(float low, float high)
{
    float t = (float)rand() / (float)RAND_MAX;
    return (1.0f - t) * low + t * high;
}

inline double get_cpu_freq() {
    unsigned long long t0, t1;
    t0 = __rdtsc();
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
    t1 = __rdtsc();
    return (double)(t1-t0);
}

#endif

#define TILE_m 32
#define TILE_n 16
#define TILE_k 8

#define GEMM_BLOCK 1024

#endif
