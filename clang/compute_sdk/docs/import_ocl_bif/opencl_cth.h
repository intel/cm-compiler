/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _OPENCL_CTH_
#define _OPENCL_CTH_

#define FP_FAST_FMAF

#pragma GCC system_header //Allows for typedef redefinitions to be allowed in this file.

//typedef size_t uintptr_t;

#define NULL 0

#if (__OPENCL_C_VERSION__ >= CL_VERSION_1_2)
char __attribute__((overloadable)) popcount(char x);
uchar __attribute__((overloadable)) popcount(uchar x);
char2 __attribute__((overloadable)) popcount(char2 x);
uchar2 __attribute__((overloadable)) popcount(uchar2 x);
char3 __attribute__((overloadable)) popcount(char3 x);
uchar3 __attribute__((overloadable)) popcount(uchar3 x);
char4 __attribute__((overloadable)) popcount(char4 x);
uchar4 __attribute__((overloadable)) popcount(uchar4 x);
char8 __attribute__((overloadable)) popcount(char8 x);
uchar8 __attribute__((overloadable)) popcount(uchar8 x);
char16 __attribute__((overloadable)) popcount(char16 x);
uchar16 __attribute__((overloadable)) popcount(uchar16 x);
short __attribute__((overloadable)) popcount(short x);
ushort __attribute__((overloadable)) popcount(ushort x);
short2 __attribute__((overloadable)) popcount(short2 x);
ushort2 __attribute__((overloadable)) popcount(ushort2 x);
short3 __attribute__((overloadable)) popcount(short3 x);
ushort3 __attribute__((overloadable)) popcount(ushort3 x);
short4 __attribute__((overloadable)) popcount(short4 x);
ushort4 __attribute__((overloadable)) popcount(ushort4 x);
short8 __attribute__((overloadable)) popcount(short8 x);
ushort8 __attribute__((overloadable)) popcount(ushort8 x);
short16 __attribute__((overloadable)) popcount(short16 x);
ushort16 __attribute__((overloadable)) popcount(ushort16 x);
int __attribute__((overloadable)) popcount(int x);
uint __attribute__((overloadable)) popcount(uint x);
int2 __attribute__((overloadable)) popcount(int2 x);
uint2 __attribute__((overloadable)) popcount(uint2 x);
int3 __attribute__((overloadable)) popcount(int3 x);
uint3 __attribute__((overloadable)) popcount(uint3 x);
int4 __attribute__((overloadable)) popcount(int4 x);
uint4 __attribute__((overloadable)) popcount(uint4 x);
int8 __attribute__((overloadable)) popcount(int8 x);
uint8 __attribute__((overloadable)) popcount(uint8 x);
int16 __attribute__((overloadable)) popcount(int16 x);
uint16 __attribute__((overloadable)) popcount(uint16 x);
long __attribute__((overloadable)) popcount(long x);
ulong __attribute__((overloadable)) popcount(ulong x);
long2 __attribute__((overloadable)) popcount(long2 x);
ulong2 __attribute__((overloadable)) popcount(ulong2 x);
long3 __attribute__((overloadable)) popcount(long3 x);
ulong3 __attribute__((overloadable)) popcount(ulong3 x);
long4 __attribute__((overloadable)) popcount(long4 x);
ulong4 __attribute__((overloadable)) popcount(ulong4 x);
long8 __attribute__((overloadable)) popcount(long8 x);
ulong8 __attribute__((overloadable)) popcount(ulong8 x);
long16 __attribute__((overloadable)) popcount(long16 x);
ulong16 __attribute__((overloadable)) popcount(ulong16 x);
#endif

//#if (__OPENCL_C_VERSION__ > CL_VERSION_1_2)
char __attribute__((overloadable)) ctz(char x);
uchar __attribute__((overloadable)) ctz(uchar x);
char2 __attribute__((overloadable)) ctz(char2 x);
uchar2 __attribute__((overloadable)) ctz(uchar2 x);
char3 __attribute__((overloadable)) ctz(char3 x);
uchar3 __attribute__((overloadable)) ctz(uchar3 x);
char4 __attribute__((overloadable)) ctz(char4 x);
uchar4 __attribute__((overloadable)) ctz(uchar4 x);
char8 __attribute__((overloadable)) ctz(char8 x);
uchar8 __attribute__((overloadable)) ctz(uchar8 x);
char16 __attribute__((overloadable)) ctz(char16 x);
uchar16 __attribute__((overloadable)) ctz(uchar16 x);
short __attribute__((overloadable)) ctz(short x);
ushort __attribute__((overloadable)) ctz(ushort x);
short2 __attribute__((overloadable)) ctz(short2 x);
ushort2 __attribute__((overloadable)) ctz(ushort2 x);
short3 __attribute__((overloadable)) ctz(short3 x);
ushort3 __attribute__((overloadable)) ctz(ushort3 x);
short4 __attribute__((overloadable)) ctz(short4 x);
ushort4 __attribute__((overloadable)) ctz(ushort4 x);
short8 __attribute__((overloadable)) ctz(short8 x);
ushort8 __attribute__((overloadable)) ctz(ushort8 x);
short16 __attribute__((overloadable)) ctz(short16 x);
ushort16 __attribute__((overloadable)) ctz(ushort16 x);
int __attribute__((overloadable)) ctz(int x);
uint __attribute__((overloadable)) ctz(uint x);
int2 __attribute__((overloadable)) ctz(int2 x);
uint2 __attribute__((overloadable)) ctz(uint2 x);
int3 __attribute__((overloadable)) ctz(int3 x);
uint3 __attribute__((overloadable)) ctz(uint3 x);
int4 __attribute__((overloadable)) ctz(int4 x);
uint4 __attribute__((overloadable)) ctz(uint4 x);
int8 __attribute__((overloadable)) ctz(int8 x);
uint8 __attribute__((overloadable)) ctz(uint8 x);
int16 __attribute__((overloadable)) ctz(int16 x);
uint16 __attribute__((overloadable)) ctz(uint16 x);
long __attribute__((overloadable)) ctz(long x);
ulong __attribute__((overloadable)) ctz(ulong x);
long2 __attribute__((overloadable)) ctz(long2 x);
ulong2 __attribute__((overloadable)) ctz(ulong2 x);
long3 __attribute__((overloadable)) ctz(long3 x);
ulong3 __attribute__((overloadable)) ctz(ulong3 x);
long4 __attribute__((overloadable)) ctz(long4 x);
ulong4 __attribute__((overloadable)) ctz(ulong4 x);
long8 __attribute__((overloadable)) ctz(long8 x);
ulong8 __attribute__((overloadable)) ctz(ulong8 x);
long16 __attribute__((overloadable)) ctz(long16 x);
ulong16 __attribute__((overloadable)) ctz(ulong16 x);
//#endif

////////////////////////////////////////////////////////////////////////////////////
////              cl_khr_fp16 - extension support
////
////////////////////////////////////////////////////////////////////////////////////

#ifdef cl_khr_fp16

#define HALF_DIG 3
#define HALF_MANT_DIG 11
#define HALF_MAX_10_EXP +4
#define HALF_MAX_EXP +16
#define HALF_MIN_10_EXP -4
#define HALF_MIN_EXP -13
#define HALF_RADIX 2
#define HALF_MAX ((0x1.ffcp15h))
#define HALF_MIN ((0x1.0p-14h))
#define HALF_EPSILON ((0x1.0p-10h))

#define M_E_H         2.71828182845904523536028747135266250h
#define M_LOG2E_H     1.44269504088896340735992468100189214h
#define M_LOG10E_H    0.434294481903251827651128918916605082h
#define M_LN2_H       0.693147180559945309417232121458176568h
#define M_LN10_H      2.30258509299404568401799145468436421h
#define M_PI_H        3.14159265358979323846264338327950288h
#define M_PI_2_H      1.57079632679489661923132169163975144h
#define M_PI_4_H      0.785398163397448309615660845819875721h
#define M_1_PI_H      0.318309886183790671537767526745028724h
#define M_2_PI_H      0.636619772367581343075535053490057448h
#define M_2_SQRTPI_H  1.12837916709551257389615890312154517h
#define M_SQRT2_H     1.41421356237309504880168872420969808h
#define M_SQRT1_2_H   0.707106781186547524400844362104849039h

half __attribute__((overloadable)) native_exp(half x);
half2 __attribute__((overloadable)) native_exp(half2 x);
half3 __attribute__((overloadable)) native_exp(half3 x);
half4 __attribute__((overloadable)) native_exp(half4 x);
half8 __attribute__((overloadable)) native_exp(half8 x);
half16 __attribute__((overloadable)) native_exp(half16 x);
half __attribute__((overloadable)) native_exp10(half x);
half2 __attribute__((overloadable)) native_exp10(half2 x);
half3 __attribute__((overloadable)) native_exp10(half3 x);
half4 __attribute__((overloadable)) native_exp10(half4 x);
half8 __attribute__((overloadable)) native_exp10(half8 x);
half16 __attribute__((overloadable)) native_exp10(half16 x);
half __attribute__((overloadable)) native_exp2(half x);
half2 __attribute__((overloadable)) native_exp2(half2 x);
half3 __attribute__((overloadable)) native_exp2(half3 x);
half4 __attribute__((overloadable)) native_exp2(half4 x);
half8 __attribute__((overloadable)) native_exp2(half8 x);
half16 __attribute__((overloadable)) native_exp2(half16 x);
half __attribute__((overloadable)) native_log(half x);
half2 __attribute__((overloadable)) native_log(half2 x);
half3 __attribute__((overloadable)) native_log(half3 x);
half4 __attribute__((overloadable)) native_log(half4 x);
half8 __attribute__((overloadable)) native_log(half8 x);
half16 __attribute__((overloadable)) native_log(half16 x);
half __attribute__((overloadable)) native_log2(half x);
half2 __attribute__((overloadable)) native_log2(half2 x);
half3 __attribute__((overloadable)) native_log2(half3 x);
half4 __attribute__((overloadable)) native_log2(half4 x);
half8 __attribute__((overloadable)) native_log2(half8 x);
half16 __attribute__((overloadable)) native_log2(half16 x);
half __attribute__((overloadable)) native_powr(half x, half y);
half2 __attribute__((overloadable)) native_powr(half2 x, half2 y);
half3 __attribute__((overloadable)) native_powr(half3 x, half3 y);
half4 __attribute__((overloadable)) native_powr(half4 x, half4 y);
half8 __attribute__((overloadable)) native_powr(half8 x, half8 y);
half16 __attribute__((overloadable)) native_powr(half16 x, half16 y);
half __attribute__((overloadable)) native_recip(half x);
half2 __attribute__((overloadable)) native_recip(half2 x);
half3 __attribute__((overloadable)) native_recip(half3 x);
half4 __attribute__((overloadable)) native_recip(half4 x);
half8 __attribute__((overloadable)) native_recip(half8 x);
half16 __attribute__((overloadable)) native_recip(half16 x);
half __attribute__((overloadable)) native_rsqrt(half x);
half2 __attribute__((overloadable)) native_rsqrt(half2 x);
half3 __attribute__((overloadable)) native_rsqrt(half3 x);
half4 __attribute__((overloadable)) native_rsqrt(half4 x);
half8 __attribute__((overloadable)) native_rsqrt(half8 x);
half16 __attribute__((overloadable)) native_rsqrt(half16 x);
half __attribute__((overloadable)) native_sqrt(half x);
half2 __attribute__((overloadable)) native_sqrt(half2 x);
half3 __attribute__((overloadable)) native_sqrt(half3 x);
half4 __attribute__((overloadable)) native_sqrt(half4 x);
half8 __attribute__((overloadable)) native_sqrt(half8 x);
half16 __attribute__((overloadable)) native_sqrt(half16 x);


#endif

////////////////////////////////////////////////////////////////////////////////////
////              cl_khr_fp64 - extension support
////
////////////////////////////////////////////////////////////////////////////////////

#if defined(cl_khr_fp64)

#define DBL_DIG 15
#define DBL_MANT_DIG 53
#define DBL_MAX_10_EXP +308
#define DBL_MAX_EXP +1024
#define DBL_MIN_10_EXP -307
#define DBL_MIN_EXP -1021
#define DBL_RADIX 2
#define DBL_MAX 0x1.fffffffffffffp1023
#define DBL_MIN 0x1.0p-1022
#define DBL_EPSILON 0x1.0p-52

#define M_E           0x1.5bf0a8b145769p+1
#define M_LOG2E       0x1.71547652b82fep+0
#define M_LOG10E      0x1.bcb7b1526e50ep-2
#define M_LN2         0x1.62e42fefa39efp-1
#define M_LN10        0x1.26bb1bbb55516p+1
#define M_PI          0x1.921fb54442d18p+1
#define M_PI_2        0x1.921fb54442d18p+0
#define M_PI_4        0x1.921fb54442d18p-1
#define M_1_PI        0x1.45f306dc9c883p-2
#define M_2_PI        0x1.45f306dc9c883p-1
#define M_2_SQRTPI    0x1.20dd750429b6dp+0
#define M_SQRT2       0x1.6a09e667f3bcdp+0
#define M_SQRT1_2     0x1.6a09e667f3bcdp-1

// Conversions

char __attribute__((overloadable)) convert_char(double);
char __attribute__((overloadable)) convert_char_rte(double);
char __attribute__((overloadable)) convert_char_rtn(double);
char __attribute__((overloadable)) convert_char_rtp(double);
char __attribute__((overloadable)) convert_char_rtz(double);
char __attribute__((overloadable)) convert_char_sat(double);
char __attribute__((overloadable)) convert_char_sat_rte(double);
char __attribute__((overloadable)) convert_char_sat_rtn(double);
char __attribute__((overloadable)) convert_char_sat_rtp(double);
char __attribute__((overloadable)) convert_char_sat_rtz(double);
char2 __attribute__((overloadable)) convert_char2(double2);
char2 __attribute__((overloadable)) convert_char2_rte(double2);
char2 __attribute__((overloadable)) convert_char2_rtn(double2);
char2 __attribute__((overloadable)) convert_char2_rtp(double2);
char2 __attribute__((overloadable)) convert_char2_rtz(double2);
char2 __attribute__((overloadable)) convert_char2_sat(double2);
char2 __attribute__((overloadable)) convert_char2_sat_rte(double2);
char2 __attribute__((overloadable)) convert_char2_sat_rtn(double2);
char2 __attribute__((overloadable)) convert_char2_sat_rtp(double2);
char2 __attribute__((overloadable)) convert_char2_sat_rtz(double2);
char3 __attribute__((overloadable)) convert_char3(double3);
char3 __attribute__((overloadable)) convert_char3_rte(double3);
char3 __attribute__((overloadable)) convert_char3_rtn(double3);
char3 __attribute__((overloadable)) convert_char3_rtp(double3);
char3 __attribute__((overloadable)) convert_char3_rtz(double3);
char3 __attribute__((overloadable)) convert_char3_sat(double3);
char3 __attribute__((overloadable)) convert_char3_sat_rte(double3);
char3 __attribute__((overloadable)) convert_char3_sat_rtn(double3);
char3 __attribute__((overloadable)) convert_char3_sat_rtp(double3);
char3 __attribute__((overloadable)) convert_char3_sat_rtz(double3);
char4 __attribute__((overloadable)) convert_char4(double4);
char4 __attribute__((overloadable)) convert_char4_rte(double4);
char4 __attribute__((overloadable)) convert_char4_rtn(double4);
char4 __attribute__((overloadable)) convert_char4_rtp(double4);
char4 __attribute__((overloadable)) convert_char4_rtz(double4);
char4 __attribute__((overloadable)) convert_char4_sat(double4);
char4 __attribute__((overloadable)) convert_char4_sat_rte(double4);
char4 __attribute__((overloadable)) convert_char4_sat_rtn(double4);
char4 __attribute__((overloadable)) convert_char4_sat_rtp(double4);
char4 __attribute__((overloadable)) convert_char4_sat_rtz(double4);
char8 __attribute__((overloadable)) convert_char8(double8);
char8 __attribute__((overloadable)) convert_char8_rte(double8);
char8 __attribute__((overloadable)) convert_char8_rtn(double8);
char8 __attribute__((overloadable)) convert_char8_rtp(double8);
char8 __attribute__((overloadable)) convert_char8_rtz(double8);
char8 __attribute__((overloadable)) convert_char8_sat(double8);
char8 __attribute__((overloadable)) convert_char8_sat_rte(double8);
char8 __attribute__((overloadable)) convert_char8_sat_rtn(double8);
char8 __attribute__((overloadable)) convert_char8_sat_rtp(double8);
char8 __attribute__((overloadable)) convert_char8_sat_rtz(double8);
char16 __attribute__((overloadable)) convert_char16(double16);
char16 __attribute__((overloadable)) convert_char16_rte(double16);
char16 __attribute__((overloadable)) convert_char16_rtn(double16);
char16 __attribute__((overloadable)) convert_char16_rtp(double16);
char16 __attribute__((overloadable)) convert_char16_rtz(double16);
char16 __attribute__((overloadable)) convert_char16_sat(double16);
char16 __attribute__((overloadable)) convert_char16_sat_rte(double16);
char16 __attribute__((overloadable)) convert_char16_sat_rtn(double16);
char16 __attribute__((overloadable)) convert_char16_sat_rtp(double16);
char16 __attribute__((overloadable)) convert_char16_sat_rtz(double16);

uchar __attribute__((overloadable)) convert_uchar(double);
uchar __attribute__((overloadable)) convert_uchar_rte(double);
uchar __attribute__((overloadable)) convert_uchar_rtn(double);
uchar __attribute__((overloadable)) convert_uchar_rtp(double);
uchar __attribute__((overloadable)) convert_uchar_rtz(double);
uchar __attribute__((overloadable)) convert_uchar_sat(double);
uchar __attribute__((overloadable)) convert_uchar_sat_rte(double);
uchar __attribute__((overloadable)) convert_uchar_sat_rtn(double);
uchar __attribute__((overloadable)) convert_uchar_sat_rtp(double);
uchar __attribute__((overloadable)) convert_uchar_sat_rtz(double);
uchar2 __attribute__((overloadable)) convert_uchar2(double2);
uchar2 __attribute__((overloadable)) convert_uchar2_rte(double2);
uchar2 __attribute__((overloadable)) convert_uchar2_rtn(double2);
uchar2 __attribute__((overloadable)) convert_uchar2_rtp(double2);
uchar2 __attribute__((overloadable)) convert_uchar2_rtz(double2);
uchar2 __attribute__((overloadable)) convert_uchar2_sat(double2);
uchar2 __attribute__((overloadable)) convert_uchar2_sat_rte(double2);
uchar2 __attribute__((overloadable)) convert_uchar2_sat_rtn(double2);
uchar2 __attribute__((overloadable)) convert_uchar2_sat_rtp(double2);
uchar2 __attribute__((overloadable)) convert_uchar2_sat_rtz(double2);
uchar3 __attribute__((overloadable)) convert_uchar3(double3);
uchar3 __attribute__((overloadable)) convert_uchar3_rte(double3);
uchar3 __attribute__((overloadable)) convert_uchar3_rtn(double3);
uchar3 __attribute__((overloadable)) convert_uchar3_rtp(double3);
uchar3 __attribute__((overloadable)) convert_uchar3_rtz(double3);
uchar3 __attribute__((overloadable)) convert_uchar3_sat(double3);
uchar3 __attribute__((overloadable)) convert_uchar3_sat_rte(double3);
uchar3 __attribute__((overloadable)) convert_uchar3_sat_rtn(double3);
uchar3 __attribute__((overloadable)) convert_uchar3_sat_rtp(double3);
uchar3 __attribute__((overloadable)) convert_uchar3_sat_rtz(double3);
uchar4 __attribute__((overloadable)) convert_uchar4(double4);
uchar4 __attribute__((overloadable)) convert_uchar4_rte(double4);
uchar4 __attribute__((overloadable)) convert_uchar4_rtn(double4);
uchar4 __attribute__((overloadable)) convert_uchar4_rtp(double4);
uchar4 __attribute__((overloadable)) convert_uchar4_rtz(double4);
uchar4 __attribute__((overloadable)) convert_uchar4_sat(double4);
uchar4 __attribute__((overloadable)) convert_uchar4_sat_rte(double4);
uchar4 __attribute__((overloadable)) convert_uchar4_sat_rtn(double4);
uchar4 __attribute__((overloadable)) convert_uchar4_sat_rtp(double4);
uchar4 __attribute__((overloadable)) convert_uchar4_sat_rtz(double4);
uchar8 __attribute__((overloadable)) convert_uchar8(double8);
uchar8 __attribute__((overloadable)) convert_uchar8_rte(double8);
uchar8 __attribute__((overloadable)) convert_uchar8_rtn(double8);
uchar8 __attribute__((overloadable)) convert_uchar8_rtp(double8);
uchar8 __attribute__((overloadable)) convert_uchar8_rtz(double8);
uchar8 __attribute__((overloadable)) convert_uchar8_sat(double8);
uchar8 __attribute__((overloadable)) convert_uchar8_sat_rte(double8);
uchar8 __attribute__((overloadable)) convert_uchar8_sat_rtn(double8);
uchar8 __attribute__((overloadable)) convert_uchar8_sat_rtp(double8);
uchar8 __attribute__((overloadable)) convert_uchar8_sat_rtz(double8);
uchar16 __attribute__((overloadable)) convert_uchar16(double16);
uchar16 __attribute__((overloadable)) convert_uchar16_rte(double16);
uchar16 __attribute__((overloadable)) convert_uchar16_rtn(double16);
uchar16 __attribute__((overloadable)) convert_uchar16_rtp(double16);
uchar16 __attribute__((overloadable)) convert_uchar16_rtz(double16);
uchar16 __attribute__((overloadable)) convert_uchar16_sat(double16);
uchar16 __attribute__((overloadable)) convert_uchar16_sat_rte(double16);
uchar16 __attribute__((overloadable)) convert_uchar16_sat_rtn(double16);
uchar16 __attribute__((overloadable)) convert_uchar16_sat_rtp(double16);
uchar16 __attribute__((overloadable)) convert_uchar16_sat_rtz(double16);

short __attribute__((overloadable)) convert_short(double);
short __attribute__((overloadable)) convert_short_rte(double);
short __attribute__((overloadable)) convert_short_rtn(double);
short __attribute__((overloadable)) convert_short_rtp(double);
short __attribute__((overloadable)) convert_short_rtz(double);
short __attribute__((overloadable)) convert_short_sat(double);
short __attribute__((overloadable)) convert_short_sat_rte(double);
short __attribute__((overloadable)) convert_short_sat_rtn(double);
short __attribute__((overloadable)) convert_short_sat_rtp(double);
short __attribute__((overloadable)) convert_short_sat_rtz(double);
short2 __attribute__((overloadable)) convert_short2(double2);
short2 __attribute__((overloadable)) convert_short2_rte(double2);
short2 __attribute__((overloadable)) convert_short2_rtn(double2);
short2 __attribute__((overloadable)) convert_short2_rtp(double2);
short2 __attribute__((overloadable)) convert_short2_rtz(double2);
short2 __attribute__((overloadable)) convert_short2_sat(double2);
short2 __attribute__((overloadable)) convert_short2_sat_rte(double2);
short2 __attribute__((overloadable)) convert_short2_sat_rtn(double2);
short2 __attribute__((overloadable)) convert_short2_sat_rtp(double2);
short2 __attribute__((overloadable)) convert_short2_sat_rtz(double2);
short3 __attribute__((overloadable)) convert_short3(double3);
short3 __attribute__((overloadable)) convert_short3_rte(double3);
short3 __attribute__((overloadable)) convert_short3_rtn(double3);
short3 __attribute__((overloadable)) convert_short3_rtp(double3);
short3 __attribute__((overloadable)) convert_short3_rtz(double3);
short3 __attribute__((overloadable)) convert_short3_sat(double3);
short3 __attribute__((overloadable)) convert_short3_sat_rte(double3);
short3 __attribute__((overloadable)) convert_short3_sat_rtn(double3);
short3 __attribute__((overloadable)) convert_short3_sat_rtp(double3);
short3 __attribute__((overloadable)) convert_short3_sat_rtz(double3);
short4 __attribute__((overloadable)) convert_short4(double4);
short4 __attribute__((overloadable)) convert_short4_rte(double4);
short4 __attribute__((overloadable)) convert_short4_rtn(double4);
short4 __attribute__((overloadable)) convert_short4_rtp(double4);
short4 __attribute__((overloadable)) convert_short4_rtz(double4);
short4 __attribute__((overloadable)) convert_short4_sat(double4);
short4 __attribute__((overloadable)) convert_short4_sat_rte(double4);
short4 __attribute__((overloadable)) convert_short4_sat_rtn(double4);
short4 __attribute__((overloadable)) convert_short4_sat_rtp(double4);
short4 __attribute__((overloadable)) convert_short4_sat_rtz(double4);
short8 __attribute__((overloadable)) convert_short8(double8);
short8 __attribute__((overloadable)) convert_short8_rte(double8);
short8 __attribute__((overloadable)) convert_short8_rtn(double8);
short8 __attribute__((overloadable)) convert_short8_rtp(double8);
short8 __attribute__((overloadable)) convert_short8_rtz(double8);
short8 __attribute__((overloadable)) convert_short8_sat(double8);
short8 __attribute__((overloadable)) convert_short8_sat_rte(double8);
short8 __attribute__((overloadable)) convert_short8_sat_rtn(double8);
short8 __attribute__((overloadable)) convert_short8_sat_rtp(double8);
short8 __attribute__((overloadable)) convert_short8_sat_rtz(double8);
short16 __attribute__((overloadable)) convert_short16(double16);
short16 __attribute__((overloadable)) convert_short16_rte(double16);
short16 __attribute__((overloadable)) convert_short16_rtn(double16);
short16 __attribute__((overloadable)) convert_short16_rtp(double16);
short16 __attribute__((overloadable)) convert_short16_rtz(double16);
short16 __attribute__((overloadable)) convert_short16_sat(double16);
short16 __attribute__((overloadable)) convert_short16_sat_rte(double16);
short16 __attribute__((overloadable)) convert_short16_sat_rtn(double16);
short16 __attribute__((overloadable)) convert_short16_sat_rtp(double16);
short16 __attribute__((overloadable)) convert_short16_sat_rtz(double16);

ushort __attribute__((overloadable)) convert_ushort(double);
ushort __attribute__((overloadable)) convert_ushort_rte(double);
ushort __attribute__((overloadable)) convert_ushort_rtn(double);
ushort __attribute__((overloadable)) convert_ushort_rtp(double);
ushort __attribute__((overloadable)) convert_ushort_rtz(double);
ushort __attribute__((overloadable)) convert_ushort_sat(double);
ushort __attribute__((overloadable)) convert_ushort_sat_rte(double);
ushort __attribute__((overloadable)) convert_ushort_sat_rtn(double);
ushort __attribute__((overloadable)) convert_ushort_sat_rtp(double);
ushort __attribute__((overloadable)) convert_ushort_sat_rtz(double);
ushort2 __attribute__((overloadable)) convert_ushort2(double2);
ushort2 __attribute__((overloadable)) convert_ushort2_rte(double2);
ushort2 __attribute__((overloadable)) convert_ushort2_rtn(double2);
ushort2 __attribute__((overloadable)) convert_ushort2_rtp(double2);
ushort2 __attribute__((overloadable)) convert_ushort2_rtz(double2);
ushort2 __attribute__((overloadable)) convert_ushort2_sat(double2);
ushort2 __attribute__((overloadable)) convert_ushort2_sat_rte(double2);
ushort2 __attribute__((overloadable)) convert_ushort2_sat_rtn(double2);
ushort2 __attribute__((overloadable)) convert_ushort2_sat_rtp(double2);
ushort2 __attribute__((overloadable)) convert_ushort2_sat_rtz(double2);
ushort3 __attribute__((overloadable)) convert_ushort3(double3);
ushort3 __attribute__((overloadable)) convert_ushort3_rte(double3);
ushort3 __attribute__((overloadable)) convert_ushort3_rtn(double3);
ushort3 __attribute__((overloadable)) convert_ushort3_rtp(double3);
ushort3 __attribute__((overloadable)) convert_ushort3_rtz(double3);
ushort3 __attribute__((overloadable)) convert_ushort3_sat(double3);
ushort3 __attribute__((overloadable)) convert_ushort3_sat_rte(double3);
ushort3 __attribute__((overloadable)) convert_ushort3_sat_rtn(double3);
ushort3 __attribute__((overloadable)) convert_ushort3_sat_rtp(double3);
ushort3 __attribute__((overloadable)) convert_ushort3_sat_rtz(double3);
ushort4 __attribute__((overloadable)) convert_ushort4(double4);
ushort4 __attribute__((overloadable)) convert_ushort4_rte(double4);
ushort4 __attribute__((overloadable)) convert_ushort4_rtn(double4);
ushort4 __attribute__((overloadable)) convert_ushort4_rtp(double4);
ushort4 __attribute__((overloadable)) convert_ushort4_rtz(double4);
ushort4 __attribute__((overloadable)) convert_ushort4_sat(double4);
ushort4 __attribute__((overloadable)) convert_ushort4_sat_rte(double4);
ushort4 __attribute__((overloadable)) convert_ushort4_sat_rtn(double4);
ushort4 __attribute__((overloadable)) convert_ushort4_sat_rtp(double4);
ushort4 __attribute__((overloadable)) convert_ushort4_sat_rtz(double4);
ushort8 __attribute__((overloadable)) convert_ushort8(double8);
ushort8 __attribute__((overloadable)) convert_ushort8_rte(double8);
ushort8 __attribute__((overloadable)) convert_ushort8_rtn(double8);
ushort8 __attribute__((overloadable)) convert_ushort8_rtp(double8);
ushort8 __attribute__((overloadable)) convert_ushort8_rtz(double8);
ushort8 __attribute__((overloadable)) convert_ushort8_sat(double8);
ushort8 __attribute__((overloadable)) convert_ushort8_sat_rte(double8);
ushort8 __attribute__((overloadable)) convert_ushort8_sat_rtn(double8);
ushort8 __attribute__((overloadable)) convert_ushort8_sat_rtp(double8);
ushort8 __attribute__((overloadable)) convert_ushort8_sat_rtz(double8);
ushort16 __attribute__((overloadable)) convert_ushort16(double16);
ushort16 __attribute__((overloadable)) convert_ushort16_rte(double16);
ushort16 __attribute__((overloadable)) convert_ushort16_rtn(double16);
ushort16 __attribute__((overloadable)) convert_ushort16_rtp(double16);
ushort16 __attribute__((overloadable)) convert_ushort16_rtz(double16);
ushort16 __attribute__((overloadable)) convert_ushort16_sat(double16);
ushort16 __attribute__((overloadable)) convert_ushort16_sat_rte(double16);
ushort16 __attribute__((overloadable)) convert_ushort16_sat_rtn(double16);
ushort16 __attribute__((overloadable)) convert_ushort16_sat_rtp(double16);
ushort16 __attribute__((overloadable)) convert_ushort16_sat_rtz(double16);

int __attribute__((overloadable)) convert_int(double);
int __attribute__((overloadable)) convert_int_rte(double);
int __attribute__((overloadable)) convert_int_rtn(double);
int __attribute__((overloadable)) convert_int_rtp(double);
int __attribute__((overloadable)) convert_int_rtz(double);
int __attribute__((overloadable)) convert_int_sat(double);
int __attribute__((overloadable)) convert_int_sat_rte(double);
int __attribute__((overloadable)) convert_int_sat_rtn(double);
int __attribute__((overloadable)) convert_int_sat_rtp(double);
int __attribute__((overloadable)) convert_int_sat_rtz(double);
int2 __attribute__((overloadable)) convert_int2(double2);
int2 __attribute__((overloadable)) convert_int2_rte(double2);
int2 __attribute__((overloadable)) convert_int2_rtn(double2);
int2 __attribute__((overloadable)) convert_int2_rtp(double2);
int2 __attribute__((overloadable)) convert_int2_rtz(double2);
int2 __attribute__((overloadable)) convert_int2_sat(double2);
int2 __attribute__((overloadable)) convert_int2_sat_rte(double2);
int2 __attribute__((overloadable)) convert_int2_sat_rtn(double2);
int2 __attribute__((overloadable)) convert_int2_sat_rtp(double2);
int2 __attribute__((overloadable)) convert_int2_sat_rtz(double2);
int3 __attribute__((overloadable)) convert_int3(double3);
int3 __attribute__((overloadable)) convert_int3_rte(double3);
int3 __attribute__((overloadable)) convert_int3_rtn(double3);
int3 __attribute__((overloadable)) convert_int3_rtp(double3);
int3 __attribute__((overloadable)) convert_int3_rtz(double3);
int3 __attribute__((overloadable)) convert_int3_sat(double3);
int3 __attribute__((overloadable)) convert_int3_sat_rte(double3);
int3 __attribute__((overloadable)) convert_int3_sat_rtn(double3);
int3 __attribute__((overloadable)) convert_int3_sat_rtp(double3);
int3 __attribute__((overloadable)) convert_int3_sat_rtz(double3);
int4 __attribute__((overloadable)) convert_int4(double4);
int4 __attribute__((overloadable)) convert_int4_rte(double4);
int4 __attribute__((overloadable)) convert_int4_rtn(double4);
int4 __attribute__((overloadable)) convert_int4_rtp(double4);
int4 __attribute__((overloadable)) convert_int4_rtz(double4);
int4 __attribute__((overloadable)) convert_int4_sat(double4);
int4 __attribute__((overloadable)) convert_int4_sat_rte(double4);
int4 __attribute__((overloadable)) convert_int4_sat_rtn(double4);
int4 __attribute__((overloadable)) convert_int4_sat_rtp(double4);
int4 __attribute__((overloadable)) convert_int4_sat_rtz(double4);
int8 __attribute__((overloadable)) convert_int8(double8);
int8 __attribute__((overloadable)) convert_int8_rte(double8);
int8 __attribute__((overloadable)) convert_int8_rtn(double8);
int8 __attribute__((overloadable)) convert_int8_rtp(double8);
int8 __attribute__((overloadable)) convert_int8_rtz(double8);
int8 __attribute__((overloadable)) convert_int8_sat(double8);
int8 __attribute__((overloadable)) convert_int8_sat_rte(double8);
int8 __attribute__((overloadable)) convert_int8_sat_rtn(double8);
int8 __attribute__((overloadable)) convert_int8_sat_rtp(double8);
int8 __attribute__((overloadable)) convert_int8_sat_rtz(double8);
int16 __attribute__((overloadable)) convert_int16(double16);
int16 __attribute__((overloadable)) convert_int16_rte(double16);
int16 __attribute__((overloadable)) convert_int16_rtn(double16);
int16 __attribute__((overloadable)) convert_int16_rtp(double16);
int16 __attribute__((overloadable)) convert_int16_rtz(double16);
int16 __attribute__((overloadable)) convert_int16_sat(double16);
int16 __attribute__((overloadable)) convert_int16_sat_rte(double16);
int16 __attribute__((overloadable)) convert_int16_sat_rtn(double16);
int16 __attribute__((overloadable)) convert_int16_sat_rtp(double16);
int16 __attribute__((overloadable)) convert_int16_sat_rtz(double16);

uint __attribute__((overloadable)) convert_uint(double);
uint __attribute__((overloadable)) convert_uint_rte(double);
uint __attribute__((overloadable)) convert_uint_rtn(double);
uint __attribute__((overloadable)) convert_uint_rtp(double);
uint __attribute__((overloadable)) convert_uint_rtz(double);
uint __attribute__((overloadable)) convert_uint_sat(double);
uint __attribute__((overloadable)) convert_uint_sat_rte(double);
uint __attribute__((overloadable)) convert_uint_sat_rtn(double);
uint __attribute__((overloadable)) convert_uint_sat_rtp(double);
uint __attribute__((overloadable)) convert_uint_sat_rtz(double);
uint2 __attribute__((overloadable)) convert_uint2(double2);
uint2 __attribute__((overloadable)) convert_uint2_rte(double2);
uint2 __attribute__((overloadable)) convert_uint2_rtn(double2);
uint2 __attribute__((overloadable)) convert_uint2_rtp(double2);
uint2 __attribute__((overloadable)) convert_uint2_rtz(double2);
uint2 __attribute__((overloadable)) convert_uint2_sat(double2);
uint2 __attribute__((overloadable)) convert_uint2_sat_rte(double2);
uint2 __attribute__((overloadable)) convert_uint2_sat_rtn(double2);
uint2 __attribute__((overloadable)) convert_uint2_sat_rtp(double2);
uint2 __attribute__((overloadable)) convert_uint2_sat_rtz(double2);
uint3 __attribute__((overloadable)) convert_uint3(double3);
uint3 __attribute__((overloadable)) convert_uint3_rte(double3);
uint3 __attribute__((overloadable)) convert_uint3_rtn(double3);
uint3 __attribute__((overloadable)) convert_uint3_rtp(double3);
uint3 __attribute__((overloadable)) convert_uint3_rtz(double3);
uint3 __attribute__((overloadable)) convert_uint3_sat(double3);
uint3 __attribute__((overloadable)) convert_uint3_sat_rte(double3);
uint3 __attribute__((overloadable)) convert_uint3_sat_rtn(double3);
uint3 __attribute__((overloadable)) convert_uint3_sat_rtp(double3);
uint3 __attribute__((overloadable)) convert_uint3_sat_rtz(double3);
uint4 __attribute__((overloadable)) convert_uint4(double4);
uint4 __attribute__((overloadable)) convert_uint4_rte(double4);
uint4 __attribute__((overloadable)) convert_uint4_rtn(double4);
uint4 __attribute__((overloadable)) convert_uint4_rtp(double4);
uint4 __attribute__((overloadable)) convert_uint4_rtz(double4);
uint4 __attribute__((overloadable)) convert_uint4_sat(double4);
uint4 __attribute__((overloadable)) convert_uint4_sat_rte(double4);
uint4 __attribute__((overloadable)) convert_uint4_sat_rtn(double4);
uint4 __attribute__((overloadable)) convert_uint4_sat_rtp(double4);
uint4 __attribute__((overloadable)) convert_uint4_sat_rtz(double4);
uint8 __attribute__((overloadable)) convert_uint8(double8);
uint8 __attribute__((overloadable)) convert_uint8_rte(double8);
uint8 __attribute__((overloadable)) convert_uint8_rtn(double8);
uint8 __attribute__((overloadable)) convert_uint8_rtp(double8);
uint8 __attribute__((overloadable)) convert_uint8_rtz(double8);
uint8 __attribute__((overloadable)) convert_uint8_sat(double8);
uint8 __attribute__((overloadable)) convert_uint8_sat_rte(double8);
uint8 __attribute__((overloadable)) convert_uint8_sat_rtn(double8);
uint8 __attribute__((overloadable)) convert_uint8_sat_rtp(double8);
uint8 __attribute__((overloadable)) convert_uint8_sat_rtz(double8);
uint16 __attribute__((overloadable)) convert_uint16(double16);
uint16 __attribute__((overloadable)) convert_uint16_rte(double16);
uint16 __attribute__((overloadable)) convert_uint16_rtn(double16);
uint16 __attribute__((overloadable)) convert_uint16_rtp(double16);
uint16 __attribute__((overloadable)) convert_uint16_rtz(double16);
uint16 __attribute__((overloadable)) convert_uint16_sat(double16);
uint16 __attribute__((overloadable)) convert_uint16_sat_rte(double16);
uint16 __attribute__((overloadable)) convert_uint16_sat_rtn(double16);
uint16 __attribute__((overloadable)) convert_uint16_sat_rtp(double16);
uint16 __attribute__((overloadable)) convert_uint16_sat_rtz(double16);

long __attribute__((overloadable)) convert_long(double);
long __attribute__((overloadable)) convert_long_rte(double);
long __attribute__((overloadable)) convert_long_rtn(double);
long __attribute__((overloadable)) convert_long_rtp(double);
long __attribute__((overloadable)) convert_long_rtz(double);
long __attribute__((overloadable)) convert_long_sat(double);
long __attribute__((overloadable)) convert_long_sat_rte(double);
long __attribute__((overloadable)) convert_long_sat_rtn(double);
long __attribute__((overloadable)) convert_long_sat_rtp(double);
long __attribute__((overloadable)) convert_long_sat_rtz(double);
long2 __attribute__((overloadable)) convert_long2(double2);
long2 __attribute__((overloadable)) convert_long2_rte(double2);
long2 __attribute__((overloadable)) convert_long2_rtn(double2);
long2 __attribute__((overloadable)) convert_long2_rtp(double2);
long2 __attribute__((overloadable)) convert_long2_rtz(double2);
long2 __attribute__((overloadable)) convert_long2_sat(double2);
long2 __attribute__((overloadable)) convert_long2_sat_rte(double2);
long2 __attribute__((overloadable)) convert_long2_sat_rtn(double2);
long2 __attribute__((overloadable)) convert_long2_sat_rtp(double2);
long2 __attribute__((overloadable)) convert_long2_sat_rtz(double2);
long3 __attribute__((overloadable)) convert_long3(double3);
long3 __attribute__((overloadable)) convert_long3_rte(double3);
long3 __attribute__((overloadable)) convert_long3_rtn(double3);
long3 __attribute__((overloadable)) convert_long3_rtp(double3);
long3 __attribute__((overloadable)) convert_long3_rtz(double3);
long3 __attribute__((overloadable)) convert_long3_sat(double3);
long3 __attribute__((overloadable)) convert_long3_sat_rte(double3);
long3 __attribute__((overloadable)) convert_long3_sat_rtn(double3);
long3 __attribute__((overloadable)) convert_long3_sat_rtp(double3);
long3 __attribute__((overloadable)) convert_long3_sat_rtz(double3);
long4 __attribute__((overloadable)) convert_long4(double4);
long4 __attribute__((overloadable)) convert_long4_rte(double4);
long4 __attribute__((overloadable)) convert_long4_rtn(double4);
long4 __attribute__((overloadable)) convert_long4_rtp(double4);
long4 __attribute__((overloadable)) convert_long4_rtz(double4);
long4 __attribute__((overloadable)) convert_long4_sat(double4);
long4 __attribute__((overloadable)) convert_long4_sat_rte(double4);
long4 __attribute__((overloadable)) convert_long4_sat_rtn(double4);
long4 __attribute__((overloadable)) convert_long4_sat_rtp(double4);
long4 __attribute__((overloadable)) convert_long4_sat_rtz(double4);
long8 __attribute__((overloadable)) convert_long8(double8);
long8 __attribute__((overloadable)) convert_long8_rte(double8);
long8 __attribute__((overloadable)) convert_long8_rtn(double8);
long8 __attribute__((overloadable)) convert_long8_rtp(double8);
long8 __attribute__((overloadable)) convert_long8_rtz(double8);
long8 __attribute__((overloadable)) convert_long8_sat(double8);
long8 __attribute__((overloadable)) convert_long8_sat_rte(double8);
long8 __attribute__((overloadable)) convert_long8_sat_rtn(double8);
long8 __attribute__((overloadable)) convert_long8_sat_rtp(double8);
long8 __attribute__((overloadable)) convert_long8_sat_rtz(double8);
long16 __attribute__((overloadable)) convert_long16(double16);
long16 __attribute__((overloadable)) convert_long16_rte(double16);
long16 __attribute__((overloadable)) convert_long16_rtn(double16);
long16 __attribute__((overloadable)) convert_long16_rtp(double16);
long16 __attribute__((overloadable)) convert_long16_rtz(double16);
long16 __attribute__((overloadable)) convert_long16_sat(double16);
long16 __attribute__((overloadable)) convert_long16_sat_rte(double16);
long16 __attribute__((overloadable)) convert_long16_sat_rtn(double16);
long16 __attribute__((overloadable)) convert_long16_sat_rtp(double16);
long16 __attribute__((overloadable)) convert_long16_sat_rtz(double16);

ulong __attribute__((overloadable)) convert_ulong(double);
ulong __attribute__((overloadable)) convert_ulong_rte(double);
ulong __attribute__((overloadable)) convert_ulong_rtn(double);
ulong __attribute__((overloadable)) convert_ulong_rtp(double);
ulong __attribute__((overloadable)) convert_ulong_rtz(double);
ulong __attribute__((overloadable)) convert_ulong_sat(double);
ulong __attribute__((overloadable)) convert_ulong_sat_rte(double);
ulong __attribute__((overloadable)) convert_ulong_sat_rtn(double);
ulong __attribute__((overloadable)) convert_ulong_sat_rtp(double);
ulong __attribute__((overloadable)) convert_ulong_sat_rtz(double);
ulong2 __attribute__((overloadable)) convert_ulong2(double2);
ulong2 __attribute__((overloadable)) convert_ulong2_rte(double2);
ulong2 __attribute__((overloadable)) convert_ulong2_rtn(double2);
ulong2 __attribute__((overloadable)) convert_ulong2_rtp(double2);
ulong2 __attribute__((overloadable)) convert_ulong2_rtz(double2);
ulong2 __attribute__((overloadable)) convert_ulong2_sat(double2);
ulong2 __attribute__((overloadable)) convert_ulong2_sat_rte(double2);
ulong2 __attribute__((overloadable)) convert_ulong2_sat_rtn(double2);
ulong2 __attribute__((overloadable)) convert_ulong2_sat_rtp(double2);
ulong2 __attribute__((overloadable)) convert_ulong2_sat_rtz(double2);
ulong3 __attribute__((overloadable)) convert_ulong3(double3);
ulong3 __attribute__((overloadable)) convert_ulong3_rte(double3);
ulong3 __attribute__((overloadable)) convert_ulong3_rtn(double3);
ulong3 __attribute__((overloadable)) convert_ulong3_rtp(double3);
ulong3 __attribute__((overloadable)) convert_ulong3_rtz(double3);
ulong3 __attribute__((overloadable)) convert_ulong3_sat(double3);
ulong3 __attribute__((overloadable)) convert_ulong3_sat_rte(double3);
ulong3 __attribute__((overloadable)) convert_ulong3_sat_rtn(double3);
ulong3 __attribute__((overloadable)) convert_ulong3_sat_rtp(double3);
ulong3 __attribute__((overloadable)) convert_ulong3_sat_rtz(double3);
ulong4 __attribute__((overloadable)) convert_ulong4(double4);
ulong4 __attribute__((overloadable)) convert_ulong4_rte(double4);
ulong4 __attribute__((overloadable)) convert_ulong4_rtn(double4);
ulong4 __attribute__((overloadable)) convert_ulong4_rtp(double4);
ulong4 __attribute__((overloadable)) convert_ulong4_rtz(double4);
ulong4 __attribute__((overloadable)) convert_ulong4_sat(double4);
ulong4 __attribute__((overloadable)) convert_ulong4_sat_rte(double4);
ulong4 __attribute__((overloadable)) convert_ulong4_sat_rtn(double4);
ulong4 __attribute__((overloadable)) convert_ulong4_sat_rtp(double4);
ulong4 __attribute__((overloadable)) convert_ulong4_sat_rtz(double4);
ulong8 __attribute__((overloadable)) convert_ulong8(double8);
ulong8 __attribute__((overloadable)) convert_ulong8_rte(double8);
ulong8 __attribute__((overloadable)) convert_ulong8_rtn(double8);
ulong8 __attribute__((overloadable)) convert_ulong8_rtp(double8);
ulong8 __attribute__((overloadable)) convert_ulong8_rtz(double8);
ulong8 __attribute__((overloadable)) convert_ulong8_sat(double8);
ulong8 __attribute__((overloadable)) convert_ulong8_sat_rte(double8);
ulong8 __attribute__((overloadable)) convert_ulong8_sat_rtn(double8);
ulong8 __attribute__((overloadable)) convert_ulong8_sat_rtp(double8);
ulong8 __attribute__((overloadable)) convert_ulong8_sat_rtz(double8);
ulong16 __attribute__((overloadable)) convert_ulong16(double16);
ulong16 __attribute__((overloadable)) convert_ulong16_rte(double16);
ulong16 __attribute__((overloadable)) convert_ulong16_rtn(double16);
ulong16 __attribute__((overloadable)) convert_ulong16_rtp(double16);
ulong16 __attribute__((overloadable)) convert_ulong16_rtz(double16);
ulong16 __attribute__((overloadable)) convert_ulong16_sat(double16);
ulong16 __attribute__((overloadable)) convert_ulong16_sat_rte(double16);
ulong16 __attribute__((overloadable)) convert_ulong16_sat_rtn(double16);
ulong16 __attribute__((overloadable)) convert_ulong16_sat_rtp(double16);
ulong16 __attribute__((overloadable)) convert_ulong16_sat_rtz(double16);

float __attribute__((overloadable)) convert_float(double);
float __attribute__((overloadable)) convert_float_rte(double);
float __attribute__((overloadable)) convert_float_rtn(double);
float __attribute__((overloadable)) convert_float_rtp(double);
float __attribute__((overloadable)) convert_float_rtz(double);
float2 __attribute__((overloadable)) convert_float2(double2);
float2 __attribute__((overloadable)) convert_float2_rte(double2);
float2 __attribute__((overloadable)) convert_float2_rtn(double2);
float2 __attribute__((overloadable)) convert_float2_rtp(double2);
float2 __attribute__((overloadable)) convert_float2_rtz(double2);
float3 __attribute__((overloadable)) convert_float3(double3);
float3 __attribute__((overloadable)) convert_float3_rte(double3);
float3 __attribute__((overloadable)) convert_float3_rtn(double3);
float3 __attribute__((overloadable)) convert_float3_rtp(double3);
float3 __attribute__((overloadable)) convert_float3_rtz(double3);
float4 __attribute__((overloadable)) convert_float4(double4);
float4 __attribute__((overloadable)) convert_float4_rte(double4);
float4 __attribute__((overloadable)) convert_float4_rtn(double4);
float4 __attribute__((overloadable)) convert_float4_rtp(double4);
float4 __attribute__((overloadable)) convert_float4_rtz(double4);
float8 __attribute__((overloadable)) convert_float8(double8);
float8 __attribute__((overloadable)) convert_float8_rte(double8);
float8 __attribute__((overloadable)) convert_float8_rtn(double8);
float8 __attribute__((overloadable)) convert_float8_rtp(double8);
float8 __attribute__((overloadable)) convert_float8_rtz(double8);
float16 __attribute__((overloadable)) convert_float16(double16);
float16 __attribute__((overloadable)) convert_float16_rte(double16);
float16 __attribute__((overloadable)) convert_float16_rtn(double16);
float16 __attribute__((overloadable)) convert_float16_rtp(double16);
float16 __attribute__((overloadable)) convert_float16_rtz(double16);

double __attribute__((overloadable)) convert_double(char);
double __attribute__((overloadable)) convert_double(double);
double __attribute__((overloadable)) convert_double(float);
double __attribute__((overloadable)) convert_double(int);
double __attribute__((overloadable)) convert_double(long);
double __attribute__((overloadable)) convert_double(short);
double __attribute__((overloadable)) convert_double(uchar);
double __attribute__((overloadable)) convert_double(uint);
double __attribute__((overloadable)) convert_double(ulong);
double __attribute__((overloadable)) convert_double(ushort);
double __attribute__((overloadable)) convert_double_rte(char);
double __attribute__((overloadable)) convert_double_rte(double);
double __attribute__((overloadable)) convert_double_rte(float);
double __attribute__((overloadable)) convert_double_rte(int);
double __attribute__((overloadable)) convert_double_rte(long);
double __attribute__((overloadable)) convert_double_rte(short);
double __attribute__((overloadable)) convert_double_rte(uchar);
double __attribute__((overloadable)) convert_double_rte(uint);
double __attribute__((overloadable)) convert_double_rte(ulong);
double __attribute__((overloadable)) convert_double_rte(ushort);
double __attribute__((overloadable)) convert_double_rtn(char);
double __attribute__((overloadable)) convert_double_rtn(double);
double __attribute__((overloadable)) convert_double_rtn(float);
double __attribute__((overloadable)) convert_double_rtn(int);
double __attribute__((overloadable)) convert_double_rtn(long);
double __attribute__((overloadable)) convert_double_rtn(short);
double __attribute__((overloadable)) convert_double_rtn(uchar);
double __attribute__((overloadable)) convert_double_rtn(uint);
double __attribute__((overloadable)) convert_double_rtn(ulong);
double __attribute__((overloadable)) convert_double_rtn(ushort);
double __attribute__((overloadable)) convert_double_rtp(char);
double __attribute__((overloadable)) convert_double_rtp(double);
double __attribute__((overloadable)) convert_double_rtp(float);
double __attribute__((overloadable)) convert_double_rtp(int);
double __attribute__((overloadable)) convert_double_rtp(long);
double __attribute__((overloadable)) convert_double_rtp(short);
double __attribute__((overloadable)) convert_double_rtp(uchar);
double __attribute__((overloadable)) convert_double_rtp(uint);
double __attribute__((overloadable)) convert_double_rtp(ulong);
double __attribute__((overloadable)) convert_double_rtp(ushort);
double __attribute__((overloadable)) convert_double_rtz(char);
double __attribute__((overloadable)) convert_double_rtz(double);
double __attribute__((overloadable)) convert_double_rtz(float);
double __attribute__((overloadable)) convert_double_rtz(int);
double __attribute__((overloadable)) convert_double_rtz(long);
double __attribute__((overloadable)) convert_double_rtz(short);
double __attribute__((overloadable)) convert_double_rtz(uchar);
double __attribute__((overloadable)) convert_double_rtz(uint);
double __attribute__((overloadable)) convert_double_rtz(ulong);
double __attribute__((overloadable)) convert_double_rtz(ushort);
double __attribute__((overloadable)) convert_double_sat(char);
double __attribute__((overloadable)) convert_double_sat(double);
double __attribute__((overloadable)) convert_double_sat(float);
double __attribute__((overloadable)) convert_double_sat(int);
double __attribute__((overloadable)) convert_double_sat(long);
double __attribute__((overloadable)) convert_double_sat(short);
double __attribute__((overloadable)) convert_double_sat(uchar);
double __attribute__((overloadable)) convert_double_sat(uint);
double __attribute__((overloadable)) convert_double_sat(ulong);
double __attribute__((overloadable)) convert_double_sat(ushort);
double __attribute__((overloadable)) convert_double_sat_rte(char);
double __attribute__((overloadable)) convert_double_sat_rte(double);
double __attribute__((overloadable)) convert_double_sat_rte(float);
double __attribute__((overloadable)) convert_double_sat_rte(int);
double __attribute__((overloadable)) convert_double_sat_rte(long);
double __attribute__((overloadable)) convert_double_sat_rte(short);
double __attribute__((overloadable)) convert_double_sat_rte(uchar);
double __attribute__((overloadable)) convert_double_sat_rte(uint);
double __attribute__((overloadable)) convert_double_sat_rte(ulong);
double __attribute__((overloadable)) convert_double_sat_rte(ushort);
double __attribute__((overloadable)) convert_double_sat_rtn(char);
double __attribute__((overloadable)) convert_double_sat_rtn(double);
double __attribute__((overloadable)) convert_double_sat_rtn(float);
double __attribute__((overloadable)) convert_double_sat_rtn(int);
double __attribute__((overloadable)) convert_double_sat_rtn(long);
double __attribute__((overloadable)) convert_double_sat_rtn(short);
double __attribute__((overloadable)) convert_double_sat_rtn(uchar);
double __attribute__((overloadable)) convert_double_sat_rtn(uint);
double __attribute__((overloadable)) convert_double_sat_rtn(ulong);
double __attribute__((overloadable)) convert_double_sat_rtn(ushort);
double __attribute__((overloadable)) convert_double_sat_rtp(char);
double __attribute__((overloadable)) convert_double_sat_rtp(double);
double __attribute__((overloadable)) convert_double_sat_rtp(float);
double __attribute__((overloadable)) convert_double_sat_rtp(int);
double __attribute__((overloadable)) convert_double_sat_rtp(long);
double __attribute__((overloadable)) convert_double_sat_rtp(short);
double __attribute__((overloadable)) convert_double_sat_rtp(uchar);
double __attribute__((overloadable)) convert_double_sat_rtp(uint);
double __attribute__((overloadable)) convert_double_sat_rtp(ulong);
double __attribute__((overloadable)) convert_double_sat_rtp(ushort);
double __attribute__((overloadable)) convert_double_sat_rtz(char);
double __attribute__((overloadable)) convert_double_sat_rtz(double);
double __attribute__((overloadable)) convert_double_sat_rtz(float);
double __attribute__((overloadable)) convert_double_sat_rtz(int);
double __attribute__((overloadable)) convert_double_sat_rtz(long);
double __attribute__((overloadable)) convert_double_sat_rtz(short);
double __attribute__((overloadable)) convert_double_sat_rtz(uchar);
double __attribute__((overloadable)) convert_double_sat_rtz(uint);
double __attribute__((overloadable)) convert_double_sat_rtz(ulong);
double __attribute__((overloadable)) convert_double_sat_rtz(ushort);
double2 __attribute__((overloadable)) convert_double2(char2);
double2 __attribute__((overloadable)) convert_double2(double2);
double2 __attribute__((overloadable)) convert_double2(float2);
double2 __attribute__((overloadable)) convert_double2(int2);
double2 __attribute__((overloadable)) convert_double2(long2);
double2 __attribute__((overloadable)) convert_double2(short2);
double2 __attribute__((overloadable)) convert_double2(uchar2);
double2 __attribute__((overloadable)) convert_double2(uint2);
double2 __attribute__((overloadable)) convert_double2(ulong2);
double2 __attribute__((overloadable)) convert_double2(ushort2);
double2 __attribute__((overloadable)) convert_double2_rte(char2);
double2 __attribute__((overloadable)) convert_double2_rte(double2);
double2 __attribute__((overloadable)) convert_double2_rte(float2);
double2 __attribute__((overloadable)) convert_double2_rte(int2);
double2 __attribute__((overloadable)) convert_double2_rte(long2);
double2 __attribute__((overloadable)) convert_double2_rte(short2);
double2 __attribute__((overloadable)) convert_double2_rte(uchar2);
double2 __attribute__((overloadable)) convert_double2_rte(uint2);
double2 __attribute__((overloadable)) convert_double2_rte(ulong2);
double2 __attribute__((overloadable)) convert_double2_rte(ushort2);
double2 __attribute__((overloadable)) convert_double2_rtn(char2);
double2 __attribute__((overloadable)) convert_double2_rtn(double2);
double2 __attribute__((overloadable)) convert_double2_rtn(float2);
double2 __attribute__((overloadable)) convert_double2_rtn(int2);
double2 __attribute__((overloadable)) convert_double2_rtn(long2);
double2 __attribute__((overloadable)) convert_double2_rtn(short2);
double2 __attribute__((overloadable)) convert_double2_rtn(uchar2);
double2 __attribute__((overloadable)) convert_double2_rtn(uint2);
double2 __attribute__((overloadable)) convert_double2_rtn(ulong2);
double2 __attribute__((overloadable)) convert_double2_rtn(ushort2);
double2 __attribute__((overloadable)) convert_double2_rtp(char2);
double2 __attribute__((overloadable)) convert_double2_rtp(double2);
double2 __attribute__((overloadable)) convert_double2_rtp(float2);
double2 __attribute__((overloadable)) convert_double2_rtp(int2);
double2 __attribute__((overloadable)) convert_double2_rtp(long2);
double2 __attribute__((overloadable)) convert_double2_rtp(short2);
double2 __attribute__((overloadable)) convert_double2_rtp(uchar2);
double2 __attribute__((overloadable)) convert_double2_rtp(uint2);
double2 __attribute__((overloadable)) convert_double2_rtp(ulong2);
double2 __attribute__((overloadable)) convert_double2_rtp(ushort2);
double2 __attribute__((overloadable)) convert_double2_rtz(char2);
double2 __attribute__((overloadable)) convert_double2_rtz(double2);
double2 __attribute__((overloadable)) convert_double2_rtz(float2);
double2 __attribute__((overloadable)) convert_double2_rtz(int2);
double2 __attribute__((overloadable)) convert_double2_rtz(long2);
double2 __attribute__((overloadable)) convert_double2_rtz(short2);
double2 __attribute__((overloadable)) convert_double2_rtz(uchar2);
double2 __attribute__((overloadable)) convert_double2_rtz(uint2);
double2 __attribute__((overloadable)) convert_double2_rtz(ulong2);
double2 __attribute__((overloadable)) convert_double2_rtz(ushort2);
double2 __attribute__((overloadable)) convert_double2_sat(char2);
double2 __attribute__((overloadable)) convert_double2_sat(double2);
double2 __attribute__((overloadable)) convert_double2_sat(float2);
double2 __attribute__((overloadable)) convert_double2_sat(int2);
double2 __attribute__((overloadable)) convert_double2_sat(long2);
double2 __attribute__((overloadable)) convert_double2_sat(short2);
double2 __attribute__((overloadable)) convert_double2_sat(uchar2);
double2 __attribute__((overloadable)) convert_double2_sat(uint2);
double2 __attribute__((overloadable)) convert_double2_sat(ulong2);
double2 __attribute__((overloadable)) convert_double2_sat(ushort2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(char2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(double2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(float2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(int2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(long2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(short2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(uchar2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(uint2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(ulong2);
double2 __attribute__((overloadable)) convert_double2_sat_rte(ushort2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(char2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(double2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(float2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(int2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(long2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(short2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(uchar2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(uint2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(ulong2);
double2 __attribute__((overloadable)) convert_double2_sat_rtn(ushort2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(char2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(double2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(float2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(int2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(long2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(short2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(uchar2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(uint2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(ulong2);
double2 __attribute__((overloadable)) convert_double2_sat_rtp(ushort2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(char2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(double2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(float2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(int2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(long2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(short2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(uchar2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(uint2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(ulong2);
double2 __attribute__((overloadable)) convert_double2_sat_rtz(ushort2);
double3 __attribute__((overloadable)) convert_double3(char3);
double3 __attribute__((overloadable)) convert_double3(double3);
double3 __attribute__((overloadable)) convert_double3(float3);
double3 __attribute__((overloadable)) convert_double3(int3);
double3 __attribute__((overloadable)) convert_double3(long3);
double3 __attribute__((overloadable)) convert_double3(short3);
double3 __attribute__((overloadable)) convert_double3(uchar3);
double3 __attribute__((overloadable)) convert_double3(uint3);
double3 __attribute__((overloadable)) convert_double3(ulong3);
double3 __attribute__((overloadable)) convert_double3(ushort3);
double3 __attribute__((overloadable)) convert_double3_rte(char3);
double3 __attribute__((overloadable)) convert_double3_rte(double3);
double3 __attribute__((overloadable)) convert_double3_rte(float3);
double3 __attribute__((overloadable)) convert_double3_rte(int3);
double3 __attribute__((overloadable)) convert_double3_rte(long3);
double3 __attribute__((overloadable)) convert_double3_rte(short3);
double3 __attribute__((overloadable)) convert_double3_rte(uchar3);
double3 __attribute__((overloadable)) convert_double3_rte(uint3);
double3 __attribute__((overloadable)) convert_double3_rte(ulong3);
double3 __attribute__((overloadable)) convert_double3_rte(ushort3);
double3 __attribute__((overloadable)) convert_double3_rtn(char3);
double3 __attribute__((overloadable)) convert_double3_rtn(double3);
double3 __attribute__((overloadable)) convert_double3_rtn(float3);
double3 __attribute__((overloadable)) convert_double3_rtn(int3);
double3 __attribute__((overloadable)) convert_double3_rtn(long3);
double3 __attribute__((overloadable)) convert_double3_rtn(short3);
double3 __attribute__((overloadable)) convert_double3_rtn(uchar3);
double3 __attribute__((overloadable)) convert_double3_rtn(uint3);
double3 __attribute__((overloadable)) convert_double3_rtn(ulong3);
double3 __attribute__((overloadable)) convert_double3_rtn(ushort3);
double3 __attribute__((overloadable)) convert_double3_rtp(char3);
double3 __attribute__((overloadable)) convert_double3_rtp(double3);
double3 __attribute__((overloadable)) convert_double3_rtp(float3);
double3 __attribute__((overloadable)) convert_double3_rtp(int3);
double3 __attribute__((overloadable)) convert_double3_rtp(long3);
double3 __attribute__((overloadable)) convert_double3_rtp(short3);
double3 __attribute__((overloadable)) convert_double3_rtp(uchar3);
double3 __attribute__((overloadable)) convert_double3_rtp(uint3);
double3 __attribute__((overloadable)) convert_double3_rtp(ulong3);
double3 __attribute__((overloadable)) convert_double3_rtp(ushort3);
double3 __attribute__((overloadable)) convert_double3_rtz(char3);
double3 __attribute__((overloadable)) convert_double3_rtz(double3);
double3 __attribute__((overloadable)) convert_double3_rtz(float3);
double3 __attribute__((overloadable)) convert_double3_rtz(int3);
double3 __attribute__((overloadable)) convert_double3_rtz(long3);
double3 __attribute__((overloadable)) convert_double3_rtz(short3);
double3 __attribute__((overloadable)) convert_double3_rtz(uchar3);
double3 __attribute__((overloadable)) convert_double3_rtz(uint3);
double3 __attribute__((overloadable)) convert_double3_rtz(ulong3);
double3 __attribute__((overloadable)) convert_double3_rtz(ushort3);
double3 __attribute__((overloadable)) convert_double3_sat(char3);
double3 __attribute__((overloadable)) convert_double3_sat(double3);
double3 __attribute__((overloadable)) convert_double3_sat(float3);
double3 __attribute__((overloadable)) convert_double3_sat(int3);
double3 __attribute__((overloadable)) convert_double3_sat(long3);
double3 __attribute__((overloadable)) convert_double3_sat(short3);
double3 __attribute__((overloadable)) convert_double3_sat(uchar3);
double3 __attribute__((overloadable)) convert_double3_sat(uint3);
double3 __attribute__((overloadable)) convert_double3_sat(ulong3);
double3 __attribute__((overloadable)) convert_double3_sat(ushort3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(char3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(double3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(float3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(int3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(long3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(short3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(uchar3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(uint3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(ulong3);
double3 __attribute__((overloadable)) convert_double3_sat_rte(ushort3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(char3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(double3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(float3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(int3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(long3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(short3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(uchar3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(uint3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(ulong3);
double3 __attribute__((overloadable)) convert_double3_sat_rtn(ushort3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(char3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(double3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(float3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(int3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(long3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(short3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(uchar3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(uint3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(ulong3);
double3 __attribute__((overloadable)) convert_double3_sat_rtp(ushort3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(char3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(double3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(float3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(int3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(long3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(short3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(uchar3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(uint3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(ulong3);
double3 __attribute__((overloadable)) convert_double3_sat_rtz(ushort3);
double4 __attribute__((overloadable)) convert_double4(char4);
double4 __attribute__((overloadable)) convert_double4(double4);
double4 __attribute__((overloadable)) convert_double4(float4);
double4 __attribute__((overloadable)) convert_double4(int4);
double4 __attribute__((overloadable)) convert_double4(long4);
double4 __attribute__((overloadable)) convert_double4(short4);
double4 __attribute__((overloadable)) convert_double4(uchar4);
double4 __attribute__((overloadable)) convert_double4(uint4);
double4 __attribute__((overloadable)) convert_double4(ulong4);
double4 __attribute__((overloadable)) convert_double4(ushort4);
double4 __attribute__((overloadable)) convert_double4_rte(char4);
double4 __attribute__((overloadable)) convert_double4_rte(double4);
double4 __attribute__((overloadable)) convert_double4_rte(float4);
double4 __attribute__((overloadable)) convert_double4_rte(int4);
double4 __attribute__((overloadable)) convert_double4_rte(long4);
double4 __attribute__((overloadable)) convert_double4_rte(short4);
double4 __attribute__((overloadable)) convert_double4_rte(uchar4);
double4 __attribute__((overloadable)) convert_double4_rte(uint4);
double4 __attribute__((overloadable)) convert_double4_rte(ulong4);
double4 __attribute__((overloadable)) convert_double4_rte(ushort4);
double4 __attribute__((overloadable)) convert_double4_rtn(char4);
double4 __attribute__((overloadable)) convert_double4_rtn(double4);
double4 __attribute__((overloadable)) convert_double4_rtn(float4);
double4 __attribute__((overloadable)) convert_double4_rtn(int4);
double4 __attribute__((overloadable)) convert_double4_rtn(long4);
double4 __attribute__((overloadable)) convert_double4_rtn(short4);
double4 __attribute__((overloadable)) convert_double4_rtn(uchar4);
double4 __attribute__((overloadable)) convert_double4_rtn(uint4);
double4 __attribute__((overloadable)) convert_double4_rtn(ulong4);
double4 __attribute__((overloadable)) convert_double4_rtn(ushort4);
double4 __attribute__((overloadable)) convert_double4_rtp(char4);
double4 __attribute__((overloadable)) convert_double4_rtp(double4);
double4 __attribute__((overloadable)) convert_double4_rtp(float4);
double4 __attribute__((overloadable)) convert_double4_rtp(int4);
double4 __attribute__((overloadable)) convert_double4_rtp(long4);
double4 __attribute__((overloadable)) convert_double4_rtp(short4);
double4 __attribute__((overloadable)) convert_double4_rtp(uchar4);
double4 __attribute__((overloadable)) convert_double4_rtp(uint4);
double4 __attribute__((overloadable)) convert_double4_rtp(ulong4);
double4 __attribute__((overloadable)) convert_double4_rtp(ushort4);
double4 __attribute__((overloadable)) convert_double4_rtz(char4);
double4 __attribute__((overloadable)) convert_double4_rtz(double4);
double4 __attribute__((overloadable)) convert_double4_rtz(float4);
double4 __attribute__((overloadable)) convert_double4_rtz(int4);
double4 __attribute__((overloadable)) convert_double4_rtz(long4);
double4 __attribute__((overloadable)) convert_double4_rtz(short4);
double4 __attribute__((overloadable)) convert_double4_rtz(uchar4);
double4 __attribute__((overloadable)) convert_double4_rtz(uint4);
double4 __attribute__((overloadable)) convert_double4_rtz(ulong4);
double4 __attribute__((overloadable)) convert_double4_rtz(ushort4);
double4 __attribute__((overloadable)) convert_double4_sat(char4);
double4 __attribute__((overloadable)) convert_double4_sat(double4);
double4 __attribute__((overloadable)) convert_double4_sat(float4);
double4 __attribute__((overloadable)) convert_double4_sat(int4);
double4 __attribute__((overloadable)) convert_double4_sat(long4);
double4 __attribute__((overloadable)) convert_double4_sat(short4);
double4 __attribute__((overloadable)) convert_double4_sat(uchar4);
double4 __attribute__((overloadable)) convert_double4_sat(uint4);
double4 __attribute__((overloadable)) convert_double4_sat(ulong4);
double4 __attribute__((overloadable)) convert_double4_sat(ushort4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(char4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(double4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(float4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(int4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(long4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(short4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(uchar4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(uint4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(ulong4);
double4 __attribute__((overloadable)) convert_double4_sat_rte(ushort4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(char4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(double4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(float4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(int4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(long4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(short4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(uchar4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(uint4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(ulong4);
double4 __attribute__((overloadable)) convert_double4_sat_rtn(ushort4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(char4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(double4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(float4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(int4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(long4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(short4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(uchar4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(uint4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(ulong4);
double4 __attribute__((overloadable)) convert_double4_sat_rtp(ushort4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(char4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(double4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(float4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(int4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(long4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(short4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(uchar4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(uint4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(ulong4);
double4 __attribute__((overloadable)) convert_double4_sat_rtz(ushort4);
double8 __attribute__((overloadable)) convert_double8(char8);
double8 __attribute__((overloadable)) convert_double8(double8);
double8 __attribute__((overloadable)) convert_double8(float8);
double8 __attribute__((overloadable)) convert_double8(int8);
double8 __attribute__((overloadable)) convert_double8(long8);
double8 __attribute__((overloadable)) convert_double8(short8);
double8 __attribute__((overloadable)) convert_double8(uchar8);
double8 __attribute__((overloadable)) convert_double8(uint8);
double8 __attribute__((overloadable)) convert_double8(ulong8);
double8 __attribute__((overloadable)) convert_double8(ushort8);
double8 __attribute__((overloadable)) convert_double8_rte(char8);
double8 __attribute__((overloadable)) convert_double8_rte(double8);
double8 __attribute__((overloadable)) convert_double8_rte(float8);
double8 __attribute__((overloadable)) convert_double8_rte(int8);
double8 __attribute__((overloadable)) convert_double8_rte(long8);
double8 __attribute__((overloadable)) convert_double8_rte(short8);
double8 __attribute__((overloadable)) convert_double8_rte(uchar8);
double8 __attribute__((overloadable)) convert_double8_rte(uint8);
double8 __attribute__((overloadable)) convert_double8_rte(ulong8);
double8 __attribute__((overloadable)) convert_double8_rte(ushort8);
double8 __attribute__((overloadable)) convert_double8_rtn(char8);
double8 __attribute__((overloadable)) convert_double8_rtn(double8);
double8 __attribute__((overloadable)) convert_double8_rtn(float8);
double8 __attribute__((overloadable)) convert_double8_rtn(int8);
double8 __attribute__((overloadable)) convert_double8_rtn(long8);
double8 __attribute__((overloadable)) convert_double8_rtn(short8);
double8 __attribute__((overloadable)) convert_double8_rtn(uchar8);
double8 __attribute__((overloadable)) convert_double8_rtn(uint8);
double8 __attribute__((overloadable)) convert_double8_rtn(ulong8);
double8 __attribute__((overloadable)) convert_double8_rtn(ushort8);
double8 __attribute__((overloadable)) convert_double8_rtp(char8);
double8 __attribute__((overloadable)) convert_double8_rtp(double8);
double8 __attribute__((overloadable)) convert_double8_rtp(float8);
double8 __attribute__((overloadable)) convert_double8_rtp(int8);
double8 __attribute__((overloadable)) convert_double8_rtp(long8);
double8 __attribute__((overloadable)) convert_double8_rtp(short8);
double8 __attribute__((overloadable)) convert_double8_rtp(uchar8);
double8 __attribute__((overloadable)) convert_double8_rtp(uint8);
double8 __attribute__((overloadable)) convert_double8_rtp(ulong8);
double8 __attribute__((overloadable)) convert_double8_rtp(ushort8);
double8 __attribute__((overloadable)) convert_double8_rtz(char8);
double8 __attribute__((overloadable)) convert_double8_rtz(double8);
double8 __attribute__((overloadable)) convert_double8_rtz(float8);
double8 __attribute__((overloadable)) convert_double8_rtz(int8);
double8 __attribute__((overloadable)) convert_double8_rtz(long8);
double8 __attribute__((overloadable)) convert_double8_rtz(short8);
double8 __attribute__((overloadable)) convert_double8_rtz(uchar8);
double8 __attribute__((overloadable)) convert_double8_rtz(uint8);
double8 __attribute__((overloadable)) convert_double8_rtz(ulong8);
double8 __attribute__((overloadable)) convert_double8_rtz(ushort8);
double8 __attribute__((overloadable)) convert_double8_sat(char8);
double8 __attribute__((overloadable)) convert_double8_sat(double8);
double8 __attribute__((overloadable)) convert_double8_sat(float8);
double8 __attribute__((overloadable)) convert_double8_sat(int8);
double8 __attribute__((overloadable)) convert_double8_sat(long8);
double8 __attribute__((overloadable)) convert_double8_sat(short8);
double8 __attribute__((overloadable)) convert_double8_sat(uchar8);
double8 __attribute__((overloadable)) convert_double8_sat(uint8);
double8 __attribute__((overloadable)) convert_double8_sat(ulong8);
double8 __attribute__((overloadable)) convert_double8_sat(ushort8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(char8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(double8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(float8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(int8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(long8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(short8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(uchar8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(uint8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(ulong8);
double8 __attribute__((overloadable)) convert_double8_sat_rte(ushort8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(char8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(double8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(float8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(int8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(long8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(short8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(uchar8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(uint8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(ulong8);
double8 __attribute__((overloadable)) convert_double8_sat_rtn(ushort8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(char8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(double8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(float8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(int8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(long8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(short8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(uchar8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(uint8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(ulong8);
double8 __attribute__((overloadable)) convert_double8_sat_rtp(ushort8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(char8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(double8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(float8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(int8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(long8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(short8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(uchar8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(uint8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(ulong8);
double8 __attribute__((overloadable)) convert_double8_sat_rtz(ushort8);
double16 __attribute__((overloadable)) convert_double16(char16);
double16 __attribute__((overloadable)) convert_double16(double16);
double16 __attribute__((overloadable)) convert_double16(float16);
double16 __attribute__((overloadable)) convert_double16(int16);
double16 __attribute__((overloadable)) convert_double16(long16);
double16 __attribute__((overloadable)) convert_double16(short16);
double16 __attribute__((overloadable)) convert_double16(uchar16);
double16 __attribute__((overloadable)) convert_double16(uint16);
double16 __attribute__((overloadable)) convert_double16(ulong16);
double16 __attribute__((overloadable)) convert_double16(ushort16);
double16 __attribute__((overloadable)) convert_double16_rte(char16);
double16 __attribute__((overloadable)) convert_double16_rte(double16);
double16 __attribute__((overloadable)) convert_double16_rte(float16);
double16 __attribute__((overloadable)) convert_double16_rte(int16);
double16 __attribute__((overloadable)) convert_double16_rte(long16);
double16 __attribute__((overloadable)) convert_double16_rte(short16);
double16 __attribute__((overloadable)) convert_double16_rte(uchar16);
double16 __attribute__((overloadable)) convert_double16_rte(uint16);
double16 __attribute__((overloadable)) convert_double16_rte(ulong16);
double16 __attribute__((overloadable)) convert_double16_rte(ushort16);
double16 __attribute__((overloadable)) convert_double16_rtn(char16);
double16 __attribute__((overloadable)) convert_double16_rtn(double16);
double16 __attribute__((overloadable)) convert_double16_rtn(float16);
double16 __attribute__((overloadable)) convert_double16_rtn(int16);
double16 __attribute__((overloadable)) convert_double16_rtn(long16);
double16 __attribute__((overloadable)) convert_double16_rtn(short16);
double16 __attribute__((overloadable)) convert_double16_rtn(uchar16);
double16 __attribute__((overloadable)) convert_double16_rtn(uint16);
double16 __attribute__((overloadable)) convert_double16_rtn(ulong16);
double16 __attribute__((overloadable)) convert_double16_rtn(ushort16);
double16 __attribute__((overloadable)) convert_double16_rtp(char16);
double16 __attribute__((overloadable)) convert_double16_rtp(double16);
double16 __attribute__((overloadable)) convert_double16_rtp(float16);
double16 __attribute__((overloadable)) convert_double16_rtp(int16);
double16 __attribute__((overloadable)) convert_double16_rtp(long16);
double16 __attribute__((overloadable)) convert_double16_rtp(short16);
double16 __attribute__((overloadable)) convert_double16_rtp(uchar16);
double16 __attribute__((overloadable)) convert_double16_rtp(uint16);
double16 __attribute__((overloadable)) convert_double16_rtp(ulong16);
double16 __attribute__((overloadable)) convert_double16_rtp(ushort16);
double16 __attribute__((overloadable)) convert_double16_rtz(char16);
double16 __attribute__((overloadable)) convert_double16_rtz(double16);
double16 __attribute__((overloadable)) convert_double16_rtz(float16);
double16 __attribute__((overloadable)) convert_double16_rtz(int16);
double16 __attribute__((overloadable)) convert_double16_rtz(long16);
double16 __attribute__((overloadable)) convert_double16_rtz(short16);
double16 __attribute__((overloadable)) convert_double16_rtz(uchar16);
double16 __attribute__((overloadable)) convert_double16_rtz(uint16);
double16 __attribute__((overloadable)) convert_double16_rtz(ulong16);
double16 __attribute__((overloadable)) convert_double16_rtz(ushort16);
double16 __attribute__((overloadable)) convert_double16_sat(char16);
double16 __attribute__((overloadable)) convert_double16_sat(double16);
double16 __attribute__((overloadable)) convert_double16_sat(float16);
double16 __attribute__((overloadable)) convert_double16_sat(int16);
double16 __attribute__((overloadable)) convert_double16_sat(long16);
double16 __attribute__((overloadable)) convert_double16_sat(short16);
double16 __attribute__((overloadable)) convert_double16_sat(uchar16);
double16 __attribute__((overloadable)) convert_double16_sat(uint16);
double16 __attribute__((overloadable)) convert_double16_sat(ulong16);
double16 __attribute__((overloadable)) convert_double16_sat(ushort16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(char16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(double16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(float16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(int16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(long16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(short16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(uchar16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(uint16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(ulong16);
double16 __attribute__((overloadable)) convert_double16_sat_rte(ushort16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(char16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(double16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(float16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(int16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(long16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(short16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(uchar16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(uint16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(ulong16);
double16 __attribute__((overloadable)) convert_double16_sat_rtn(ushort16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(char16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(double16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(float16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(int16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(long16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(short16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(uchar16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(uint16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(ulong16);
double16 __attribute__((overloadable)) convert_double16_sat_rtp(ushort16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(char16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(double16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(float16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(int16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(long16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(short16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(uchar16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(uint16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(ulong16);
double16 __attribute__((overloadable)) convert_double16_sat_rtz(ushort16);

double __attribute__((overloadable)) acos(double);
double2 __attribute__((overloadable)) acos(double2);
double3 __attribute__((overloadable)) acos(double3);
double4 __attribute__((overloadable)) acos(double4);
double8 __attribute__((overloadable)) acos(double8);
double16 __attribute__((overloadable)) acos(double16);

double __attribute__((overloadable)) acosh(double);
double2 __attribute__((overloadable)) acosh(double2);
double3 __attribute__((overloadable)) acosh(double3);
double4 __attribute__((overloadable)) acosh(double4);
double8 __attribute__((overloadable)) acosh(double8);
double16 __attribute__((overloadable)) acosh(double16);

double __attribute__((overloadable)) acospi(double x);
double2 __attribute__((overloadable)) acospi(double2 x);
double3 __attribute__((overloadable)) acospi(double3 x);
double4 __attribute__((overloadable)) acospi(double4 x);
double8 __attribute__((overloadable)) acospi(double8 x);
double16 __attribute__((overloadable)) acospi(double16 x);

double __attribute__((overloadable)) asin(double);
double2 __attribute__((overloadable)) asin(double2);
double3 __attribute__((overloadable)) asin(double3);
double4 __attribute__((overloadable)) asin(double4);
double8 __attribute__((overloadable)) asin(double8);
double16 __attribute__((overloadable)) asin(double16);

double __attribute__((overloadable)) asinh(double);
double2 __attribute__((overloadable)) asinh(double2);
double3 __attribute__((overloadable)) asinh(double3);
double4 __attribute__((overloadable)) asinh(double4);
double8 __attribute__((overloadable)) asinh(double8);
double16 __attribute__((overloadable)) asinh(double16);

double __attribute__((overloadable)) asinpi(double x);
double2 __attribute__((overloadable)) asinpi(double2 x);
double3 __attribute__((overloadable)) asinpi(double3 x);
double4 __attribute__((overloadable)) asinpi(double4 x);
double8 __attribute__((overloadable)) asinpi(double8 x);
double16 __attribute__((overloadable)) asinpi(double16 x);

double __attribute__((overloadable)) atan(double y_over_x);
double2 __attribute__((overloadable)) atan(double2 y_over_x);
double3 __attribute__((overloadable)) atan(double3 y_over_x);
double4 __attribute__((overloadable)) atan(double4 y_over_x);
double8 __attribute__((overloadable)) atan(double8 y_over_x);
double16 __attribute__((overloadable)) atan(double16 y_over_x);

double __attribute__((overloadable)) atan2(double y, double x);
double2 __attribute__((overloadable)) atan2(double2 y, double2 x);
double3 __attribute__((overloadable)) atan2(double3 y, double3 x);
double4 __attribute__((overloadable)) atan2(double4 y, double4 x);
double8 __attribute__((overloadable)) atan2(double8 y, double8 x);
double16 __attribute__((overloadable)) atan2(double16 y, double16 x);

double __attribute__((overloadable)) atanh(double);
double2 __attribute__((overloadable)) atanh(double2);
double3 __attribute__((overloadable)) atanh(double3);
double4 __attribute__((overloadable)) atanh(double4);
double8 __attribute__((overloadable)) atanh(double8);
double16 __attribute__((overloadable)) atanh(double16);

double __attribute__((overloadable)) atanpi(double x);
double2 __attribute__((overloadable)) atanpi(double2 x);
double3 __attribute__((overloadable)) atanpi(double3 x);
double4 __attribute__((overloadable)) atanpi(double4 x);
double8 __attribute__((overloadable)) atanpi(double8 x);
double16 __attribute__((overloadable)) atanpi(double16 x);

double __attribute__((overloadable)) atan2pi(double y, double x);
double2 __attribute__((overloadable)) atan2pi(double2 y, double2 x);
double3 __attribute__((overloadable)) atan2pi(double3 y, double3 x);
double4 __attribute__((overloadable)) atan2pi(double4 y, double4 x);
double8 __attribute__((overloadable)) atan2pi(double8 y, double8 x);
double16 __attribute__((overloadable)) atan2pi(double16 y, double16 x);

double __attribute__((overloadable)) cbrt(double);
double2 __attribute__((overloadable)) cbrt(double2);
double3 __attribute__((overloadable)) cbrt(double3);
double4 __attribute__((overloadable)) cbrt(double4);
double8 __attribute__((overloadable)) cbrt(double8);
double16 __attribute__((overloadable)) cbrt(double16);

double __attribute__((overloadable)) ceil(double);
double2 __attribute__((overloadable)) ceil(double2);
double3 __attribute__((overloadable)) ceil(double3);
double4 __attribute__((overloadable)) ceil(double4);
double8 __attribute__((overloadable)) ceil(double8);
double16 __attribute__((overloadable)) ceil(double16);

double __attribute__((overloadable)) copysign(double x, double y);
double2 __attribute__((overloadable)) copysign(double2 x, double2 y);
double3 __attribute__((overloadable)) copysign(double3 x, double3 y);
double4 __attribute__((overloadable)) copysign(double4 x, double4 y);
double8 __attribute__((overloadable)) copysign(double8 x, double8 y);
double16 __attribute__((overloadable)) copysign(double16 x, double16 y);

double __attribute__((overloadable)) cos(double);
double2 __attribute__((overloadable)) cos(double2);
double3 __attribute__((overloadable)) cos(double3);
double4 __attribute__((overloadable)) cos(double4);
double8 __attribute__((overloadable)) cos(double8);
double16 __attribute__((overloadable)) cos(double16);

double __attribute__((overloadable)) cosh(double);
double2 __attribute__((overloadable)) cosh(double2);
double3 __attribute__((overloadable)) cosh(double3);
double4 __attribute__((overloadable)) cosh(double4);
double8 __attribute__((overloadable)) cosh(double8);
double16 __attribute__((overloadable)) cosh(double16);

double __attribute__((overloadable)) cospi(double x);
double2 __attribute__((overloadable)) cospi(double2 x);
double3 __attribute__((overloadable)) cospi(double3 x);
double4 __attribute__((overloadable)) cospi(double4 x);
double8 __attribute__((overloadable)) cospi(double8 x);
double16 __attribute__((overloadable)) cospi(double16 x);

double __attribute__((overloadable)) erfc(double);
double2 __attribute__((overloadable)) erfc(double2);
double3 __attribute__((overloadable)) erfc(double3);
double4 __attribute__((overloadable)) erfc(double4);
double8 __attribute__((overloadable)) erfc(double8);
double16 __attribute__((overloadable)) erfc(double16);

double __attribute__((overloadable)) erf(double);
double2 __attribute__((overloadable)) erf(double2);
double3 __attribute__((overloadable)) erf(double3);
double4 __attribute__((overloadable)) erf(double4);
double8 __attribute__((overloadable)) erf(double8);
double16 __attribute__((overloadable)) erf(double16);

double __attribute__((overloadable)) exp(double x);
double2 __attribute__((overloadable)) exp(double2 x);
double3 __attribute__((overloadable)) exp(double3 x);
double4 __attribute__((overloadable)) exp(double4 x);
double8 __attribute__((overloadable)) exp(double8 x);
double16 __attribute__((overloadable)) exp(double16 x);

double __attribute__((overloadable)) exp2(double);
double2 __attribute__((overloadable)) exp2(double2);
double3 __attribute__((overloadable)) exp2(double3);
double4 __attribute__((overloadable)) exp2(double4);
double8 __attribute__((overloadable)) exp2(double8);
double16 __attribute__((overloadable)) exp2(double16);

double __attribute__((overloadable)) exp10(double);
double2 __attribute__((overloadable)) exp10(double2);
double3 __attribute__((overloadable)) exp10(double3);
double4 __attribute__((overloadable)) exp10(double4);
double8 __attribute__((overloadable)) exp10(double8);
double16 __attribute__((overloadable)) exp10(double16);

double __attribute__((overloadable)) expm1(double x);
double2 __attribute__((overloadable)) expm1(double2 x);
double3 __attribute__((overloadable)) expm1(double3 x);
double4 __attribute__((overloadable)) expm1(double4 x);
double8 __attribute__((overloadable)) expm1(double8 x);
double16 __attribute__((overloadable)) expm1(double16 x);

double __attribute__((overloadable)) fabs(double);
double2 __attribute__((overloadable)) fabs(double2);
double3 __attribute__((overloadable)) fabs(double3);
double4 __attribute__((overloadable)) fabs(double4);
double8 __attribute__((overloadable)) fabs(double8);
double16 __attribute__((overloadable)) fabs(double16);

double __attribute__((overloadable)) fdim(double x, double y);
double2 __attribute__((overloadable)) fdim(double2 x, double2 y);
double3 __attribute__((overloadable)) fdim(double3 x, double3 y);
double4 __attribute__((overloadable)) fdim(double4 x, double4 y);
double8 __attribute__((overloadable)) fdim(double8 x, double8 y);
double16 __attribute__((overloadable)) fdim(double16 x, double16 y);

double __attribute__((overloadable)) floor(double);
double2 __attribute__((overloadable)) floor(double2);
double3 __attribute__((overloadable)) floor(double3);
double4 __attribute__((overloadable)) floor(double4);
double8 __attribute__((overloadable)) floor(double8);
double16 __attribute__((overloadable)) floor(double16);

double __attribute__((overloadable)) fma(double a, double b, double c);
double2 __attribute__((overloadable)) fma(double2 a, double2 b, double2 c);
double3 __attribute__((overloadable)) fma(double3 a, double3 b, double3 c);
double4 __attribute__((overloadable)) fma(double4 a, double4 b, double4 c);
double8 __attribute__((overloadable)) fma(double8 a, double8 b, double8 c);
double16 __attribute__((overloadable)) fma(double16 a, double16 b, double16 c);

double __attribute__((overloadable)) fmax(double x, double y);
double2 __attribute__((overloadable)) fmax(double2 x, double2 y);
double3 __attribute__((overloadable)) fmax(double3 x, double3 y);
double4 __attribute__((overloadable)) fmax(double4 x, double4 y);
double8 __attribute__((overloadable)) fmax(double8 x, double8 y);
double16 __attribute__((overloadable)) fmax(double16 x, double16 y);
double2 __attribute__((overloadable)) fmax(double2 x, double y);
double3 __attribute__((overloadable)) fmax(double3 x, double y);
double4 __attribute__((overloadable)) fmax(double4 x, double y);
double8 __attribute__((overloadable)) fmax(double8 x, double y);
double16 __attribute__((overloadable)) fmax(double16 x, double y);

double __attribute__((overloadable)) fmin(double x, double y);
double2 __attribute__((overloadable)) fmin(double2 x, double2 y);
double3 __attribute__((overloadable)) fmin(double3 x, double3 y);
double4 __attribute__((overloadable)) fmin(double4 x, double4 y);
double8 __attribute__((overloadable)) fmin(double8 x, double8 y);
double16 __attribute__((overloadable)) fmin(double16 x, double16 y);
double2 __attribute__((overloadable)) fmin(double2 x, double y);
double3 __attribute__((overloadable)) fmin(double3 x, double y);
double4 __attribute__((overloadable)) fmin(double4 x, double y);
double8 __attribute__((overloadable)) fmin(double8 x, double y);
double16 __attribute__((overloadable)) fmin(double16 x, double y);

double __attribute__((overloadable)) fmod(double x, double y);
double2 __attribute__((overloadable)) fmod(double2 x, double2 y);
double3 __attribute__((overloadable)) fmod(double3 x, double3 y);
double4 __attribute__((overloadable)) fmod(double4 x, double4 y);
double8 __attribute__((overloadable)) fmod(double8 x, double8 y);
double16 __attribute__((overloadable)) fmod(double16 x, double16 y);

double __attribute__((overloadable)) hypot(double x, double y);
double2 __attribute__((overloadable)) hypot(double2 x, double2 y);
double3 __attribute__((overloadable)) hypot(double3 x, double3 y);
double4 __attribute__((overloadable)) hypot(double4 x, double4 y);
double8 __attribute__((overloadable)) hypot(double8 x, double8 y);
double16 __attribute__((overloadable)) hypot(double16 x, double16 y);

int __attribute__((overloadable)) ilogb(double x);
int2 __attribute__((overloadable)) ilogb(double2 x);
int3 __attribute__((overloadable)) ilogb(double3 x);
int4 __attribute__((overloadable)) ilogb(double4 x);
int8 __attribute__((overloadable)) ilogb(double8 x);
int16 __attribute__((overloadable)) ilogb(double16 x);

double __attribute__((overloadable)) ldexp(double x, int n);
double2 __attribute__((overloadable)) ldexp(double2 x, int2 n);
double3 __attribute__((overloadable)) ldexp(double3 x, int3 n);
double4 __attribute__((overloadable)) ldexp(double4 x, int4 n);
double8 __attribute__((overloadable)) ldexp(double8 x, int8 n);
double16 __attribute__((overloadable)) ldexp(double16 x, int16 n);
double2 __attribute__((overloadable)) ldexp(double2 x, int n);
double3 __attribute__((overloadable)) ldexp(double3 x, int n);
double4 __attribute__((overloadable)) ldexp(double4 x, int n);
double8 __attribute__((overloadable)) ldexp(double8 x, int n);
double16 __attribute__((overloadable)) ldexp(double16 x, int n);

double __attribute__((overloadable)) lgamma(double x);
double2 __attribute__((overloadable)) lgamma(double2 x);
double3 __attribute__((overloadable)) lgamma(double3 x);
double4 __attribute__((overloadable)) lgamma(double4 x);
double8 __attribute__((overloadable)) lgamma(double8 x);
double16 __attribute__((overloadable)) lgamma(double16 x);

double __attribute__((overloadable)) log(double);
double2 __attribute__((overloadable)) log(double2);
double3 __attribute__((overloadable)) log(double3);
double4 __attribute__((overloadable)) log(double4);
double8 __attribute__((overloadable)) log(double8);
double16 __attribute__((overloadable)) log(double16);

double __attribute__((overloadable)) log2(double);
double2 __attribute__((overloadable)) log2(double2);
double3 __attribute__((overloadable)) log2(double3);
double4 __attribute__((overloadable)) log2(double4);
double8 __attribute__((overloadable)) log2(double8);
double16 __attribute__((overloadable)) log2(double16);

double __attribute__((overloadable)) log10(double);
double2 __attribute__((overloadable)) log10(double2);
double3 __attribute__((overloadable)) log10(double3);
double4 __attribute__((overloadable)) log10(double4);
double8 __attribute__((overloadable)) log10(double8);
double16 __attribute__((overloadable)) log10(double16);

double __attribute__((overloadable)) log1p(double x);
double2 __attribute__((overloadable)) log1p(double2 x);
double3 __attribute__((overloadable)) log1p(double3 x);
double4 __attribute__((overloadable)) log1p(double4 x);
double8 __attribute__((overloadable)) log1p(double8 x);
double16 __attribute__((overloadable)) log1p(double16 x);

double __attribute__((overloadable)) logb(double x);
double2 __attribute__((overloadable)) logb(double2 x);
double3 __attribute__((overloadable)) logb(double3 x);
double4 __attribute__((overloadable)) logb(double4 x);
double8 __attribute__((overloadable)) logb(double8 x);
double16 __attribute__((overloadable)) logb(double16 x);

double __attribute__((overloadable)) mad(double a, double b, double c);
double2 __attribute__((overloadable)) mad(double2 a, double2 b, double2 c);
double3 __attribute__((overloadable)) mad(double3 a, double3 b, double3 c);
double4 __attribute__((overloadable)) mad(double4 a, double4 b, double4 c);
double8 __attribute__((overloadable)) mad(double8 a, double8 b, double8 c);
double16 __attribute__((overloadable)) mad(double16 a, double16 b, double16 c);

double __attribute__((overloadable)) maxmag(double x, double y);
double2 __attribute__((overloadable)) maxmag(double2 x, double2 y);
double3 __attribute__((overloadable)) maxmag(double3 x, double3 y);
double4 __attribute__((overloadable)) maxmag(double4 x, double4 y);
double8 __attribute__((overloadable)) maxmag(double8 x, double8 y);
double16 __attribute__((overloadable)) maxmag(double16 x, double16 y);

double __attribute__((overloadable)) minmag(double x, double y);
double2 __attribute__((overloadable)) minmag(double2 x, double2 y);
double3 __attribute__((overloadable)) minmag(double3 x, double3 y);
double4 __attribute__((overloadable)) minmag(double4 x, double4 y);
double8 __attribute__((overloadable)) minmag(double8 x, double8 y);
double16 __attribute__((overloadable)) minmag(double16 x, double16 y);

double __attribute__((overloadable)) nan(ulong nancode);
double2 __attribute__((overloadable)) nan(ulong2 nancode);
double3 __attribute__((overloadable)) nan(ulong3 nancode);
double4 __attribute__((overloadable)) nan(ulong4 nancode);
double8 __attribute__((overloadable)) nan(ulong8 nancode);
double16 __attribute__((overloadable)) nan(ulong16 nancode);

double __attribute__((overloadable)) nextafter(double x, double y);
double2 __attribute__((overloadable)) nextafter(double2 x, double2 y);
double3 __attribute__((overloadable)) nextafter(double3 x, double3 y);
double4 __attribute__((overloadable)) nextafter(double4 x, double4 y);
double8 __attribute__((overloadable)) nextafter(double8 x, double8 y);
double16 __attribute__((overloadable)) nextafter(double16 x, double16 y);

double __attribute__((overloadable)) pow(double x, double y);
double2 __attribute__((overloadable)) pow(double2 x, double2 y);
double3 __attribute__((overloadable)) pow(double3 x, double3 y);
double4 __attribute__((overloadable)) pow(double4 x, double4 y);
double8 __attribute__((overloadable)) pow(double8 x, double8 y);
double16 __attribute__((overloadable)) pow(double16 x, double16 y);

double __attribute__((overloadable)) pown(double x, int y);
double2 __attribute__((overloadable)) pown(double2 x, int2 y);
double3 __attribute__((overloadable)) pown(double3 x, int3 y);
double4 __attribute__((overloadable)) pown(double4 x, int4 y);
double8 __attribute__((overloadable)) pown(double8 x, int8 y);
double16 __attribute__((overloadable)) pown(double16 x, int16 y);

double __attribute__((overloadable)) powr(double x, double y);
double2 __attribute__((overloadable)) powr(double2 x, double2 y);
double3 __attribute__((overloadable)) powr(double3 x, double3 y);
double4 __attribute__((overloadable)) powr(double4 x, double4 y);
double8 __attribute__((overloadable)) powr(double8 x, double8 y);
double16 __attribute__((overloadable)) powr(double16 x, double16 y);

double __attribute__((overloadable)) remainder(double x, double y);
double2 __attribute__((overloadable)) remainder(double2 x, double2 y);
double3 __attribute__((overloadable)) remainder(double3 x, double3 y);
double4 __attribute__((overloadable)) remainder(double4 x, double4 y);
double8 __attribute__((overloadable)) remainder(double8 x, double8 y);
double16 __attribute__((overloadable)) remainder(double16 x, double16 y);

double __attribute__((overloadable)) rint(double);
double2 __attribute__((overloadable)) rint(double2);
double3 __attribute__((overloadable)) rint(double3);
double4 __attribute__((overloadable)) rint(double4);
double8 __attribute__((overloadable)) rint(double8);
double16 __attribute__((overloadable)) rint(double16);

double __attribute__((overloadable)) rootn(double x, int y);
double2 __attribute__((overloadable)) rootn(double2 x, int2 y);
double3 __attribute__((overloadable)) rootn(double3 x, int3 y);
double4 __attribute__((overloadable)) rootn(double4 x, int4 y);
double8 __attribute__((overloadable)) rootn(double8 x, int8 y);
double16 __attribute__((overloadable)) rootn(double16 x, int16 y);

double __attribute__((overloadable)) round(double x);
double2 __attribute__((overloadable)) round(double2 x);
double3 __attribute__((overloadable)) round(double3 x);
double4 __attribute__((overloadable)) round(double4 x);
double8 __attribute__((overloadable)) round(double8 x);
double16 __attribute__((overloadable)) round(double16 x);

double __attribute__((overloadable)) rsqrt(double);
double2 __attribute__((overloadable)) rsqrt(double2);
double3 __attribute__((overloadable)) rsqrt(double3);
double4 __attribute__((overloadable)) rsqrt(double4);
double8 __attribute__((overloadable)) rsqrt(double8);
double16 __attribute__((overloadable)) rsqrt(double16);

double __attribute__((overloadable)) sin(double);
double2 __attribute__((overloadable)) sin(double2);
double3 __attribute__((overloadable)) sin(double3);
double4 __attribute__((overloadable)) sin(double4);
double8 __attribute__((overloadable)) sin(double8);
double16 __attribute__((overloadable)) sin(double16);

double __attribute__((overloadable)) sinh(double);
double2 __attribute__((overloadable)) sinh(double2);
double3 __attribute__((overloadable)) sinh(double3);
double4 __attribute__((overloadable)) sinh(double4);
double8 __attribute__((overloadable)) sinh(double8);
double16 __attribute__((overloadable)) sinh(double16);

double __attribute__((overloadable)) sinpi(double x);
double2 __attribute__((overloadable)) sinpi(double2 x);
double3 __attribute__((overloadable)) sinpi(double3 x);
double4 __attribute__((overloadable)) sinpi(double4 x);
double8 __attribute__((overloadable)) sinpi(double8 x);
double16 __attribute__((overloadable)) sinpi(double16 x);

double __attribute__((overloadable)) sqrt(double);
double2 __attribute__((overloadable)) sqrt(double2);
double3 __attribute__((overloadable)) sqrt(double3);
double4 __attribute__((overloadable)) sqrt(double4);
double8 __attribute__((overloadable)) sqrt(double8);
double16 __attribute__((overloadable)) sqrt(double16);

double __attribute__((overloadable)) tan(double);
double2 __attribute__((overloadable)) tan(double2);
double3 __attribute__((overloadable)) tan(double3);
double4 __attribute__((overloadable)) tan(double4);
double8 __attribute__((overloadable)) tan(double8);
double16 __attribute__((overloadable)) tan(double16);

double __attribute__((overloadable)) tanh(double);
double2 __attribute__((overloadable)) tanh(double2);
double3 __attribute__((overloadable)) tanh(double3);
double4 __attribute__((overloadable)) tanh(double4);
double8 __attribute__((overloadable)) tanh(double8);
double16 __attribute__((overloadable)) tanh(double16);

double __attribute__((overloadable)) tanpi(double x);
double2 __attribute__((overloadable)) tanpi(double2 x);
double3 __attribute__((overloadable)) tanpi(double3 x);
double4 __attribute__((overloadable)) tanpi(double4 x);
double8 __attribute__((overloadable)) tanpi(double8 x);
double16 __attribute__((overloadable)) tanpi(double16 x);

double __attribute__((overloadable)) tgamma(double);
double2 __attribute__((overloadable)) tgamma(double2);
double3 __attribute__((overloadable)) tgamma(double3);
double4 __attribute__((overloadable)) tgamma(double4);
double8 __attribute__((overloadable)) tgamma(double8);
double16 __attribute__((overloadable)) tgamma(double16);

double __attribute__((overloadable)) trunc(double);
double2 __attribute__((overloadable)) trunc(double2);
double3 __attribute__((overloadable)) trunc(double3);
double4 __attribute__((overloadable)) trunc(double4);
double8 __attribute__((overloadable)) trunc(double8);
double16 __attribute__((overloadable)) trunc(double16);

double __attribute__((overloadable)) native_cos(double x);
double2 __attribute__((overloadable)) native_cos(double2 x);
double3 __attribute__((overloadable)) native_cos(double3 x);
double4 __attribute__((overloadable)) native_cos(double4 x);
double8 __attribute__((overloadable)) native_cos(double8 x);
double16 __attribute__((overloadable)) native_cos(double16 x);

double __attribute__((overloadable)) native_divide(double x, double y);
double2 __attribute__((overloadable)) native_divide(double2 x, double2 y);
double3 __attribute__((overloadable)) native_divide(double3 x, double3 y);
double4 __attribute__((overloadable)) native_divide(double4 x, double4 y);
double8 __attribute__((overloadable)) native_divide(double8 x, double8 y);
double16 __attribute__((overloadable)) native_divide(double16 x, double16 y);

double __attribute__((overloadable)) native_exp(double x);
double2 __attribute__((overloadable)) native_exp(double2 x);
double3 __attribute__((overloadable)) native_exp(double3 x);
double4 __attribute__((overloadable)) native_exp(double4 x);
double8 __attribute__((overloadable)) native_exp(double8 x);
double16 __attribute__((overloadable)) native_exp(double16 x);



double __attribute__((overloadable)) native_exp10(double x);
double2 __attribute__((overloadable)) native_exp10(double2 x);
double3 __attribute__((overloadable)) native_exp10(double3 x);
double4 __attribute__((overloadable)) native_exp10(double4 x);
double8 __attribute__((overloadable)) native_exp10(double8 x);
double16 __attribute__((overloadable)) native_exp10(double16 x);


double __attribute__((overloadable)) native_exp2(double x);
double2 __attribute__((overloadable)) native_exp2(double2 x);
double3 __attribute__((overloadable)) native_exp2(double3 x);
double4 __attribute__((overloadable)) native_exp2(double4 x);
double8 __attribute__((overloadable)) native_exp2(double8 x);
double16 __attribute__((overloadable)) native_exp2(double16 x);


double __attribute__((overloadable)) native_log(double x);
double2 __attribute__((overloadable)) native_log(double2 x);
double3 __attribute__((overloadable)) native_log(double3 x);
double4 __attribute__((overloadable)) native_log(double4 x);
double8 __attribute__((overloadable)) native_log(double8 x);
double16 __attribute__((overloadable)) native_log(double16 x);


double __attribute__((overloadable)) native_log10(double x);
double2 __attribute__((overloadable)) native_log10(double2 x);
double3 __attribute__((overloadable)) native_log10(double3 x);
double4 __attribute__((overloadable)) native_log10(double4 x);
double8 __attribute__((overloadable)) native_log10(double8 x);
double16 __attribute__((overloadable)) native_log10(double16 x);

double __attribute__((overloadable)) native_log2(double x);
double2 __attribute__((overloadable)) native_log2(double2 x);
double3 __attribute__((overloadable)) native_log2(double3 x);
double4 __attribute__((overloadable)) native_log2(double4 x);
double8 __attribute__((overloadable)) native_log2(double8 x);
double16 __attribute__((overloadable)) native_log2(double16 x);


double __attribute__((overloadable)) native_powr(double x, double y);
double2 __attribute__((overloadable)) native_powr(double2 x, double2 y);
double3 __attribute__((overloadable)) native_powr(double3 x, double3 y);
double4 __attribute__((overloadable)) native_powr(double4 x, double4 y);
double8 __attribute__((overloadable)) native_powr(double8 x, double8 y);
double16 __attribute__((overloadable)) native_powr(double16 x, double16 y);


double __attribute__((overloadable)) native_recip(double x);
double2 __attribute__((overloadable)) native_recip(double2 x);
double3 __attribute__((overloadable)) native_recip(double3 x);
double4 __attribute__((overloadable)) native_recip(double4 x);
double8 __attribute__((overloadable)) native_recip(double8 x);
double16 __attribute__((overloadable)) native_recip(double16 x);


double __attribute__((overloadable)) native_rsqrt(double x);
double2 __attribute__((overloadable)) native_rsqrt(double2 x);
double3 __attribute__((overloadable)) native_rsqrt(double3 x);
double4 __attribute__((overloadable)) native_rsqrt(double4 x);
double8 __attribute__((overloadable)) native_rsqrt(double8 x);
double16 __attribute__((overloadable)) native_rsqrt(double16 x);


double __attribute__((overloadable)) native_sin(double x);
double2 __attribute__((overloadable)) native_sin(double2 x);
double3 __attribute__((overloadable)) native_sin(double3 x);
double4 __attribute__((overloadable)) native_sin(double4 x);
double8 __attribute__((overloadable)) native_sin(double8 x);
double16 __attribute__((overloadable)) native_sin(double16 x);

double __attribute__((overloadable)) native_sqrt(double x);
double2 __attribute__((overloadable)) native_sqrt(double2 x);
double3 __attribute__((overloadable)) native_sqrt(double3 x);
double4 __attribute__((overloadable)) native_sqrt(double4 x);
double8 __attribute__((overloadable)) native_sqrt(double8 x);
double16 __attribute__((overloadable)) native_sqrt(double16 x);


double __attribute__((overloadable)) native_tan(double x);
double2 __attribute__((overloadable)) native_tan(double2 x);
double3 __attribute__((overloadable)) native_tan(double3 x);
double4 __attribute__((overloadable)) native_tan(double4 x);
double8 __attribute__((overloadable)) native_tan(double8 x);
double16 __attribute__((overloadable)) native_tan(double16 x);

// Common Functions

double __attribute__((overloadable)) clamp(double x, double minval, double maxval);
double2 __attribute__((overloadable)) clamp(double2 x, double2 minval, double2 maxval);
double3 __attribute__((overloadable)) clamp(double3 x, double3 minval, double3 maxval);
double4 __attribute__((overloadable)) clamp(double4 x, double4 minval, double4 maxval);
double8 __attribute__((overloadable)) clamp(double8 x, double8 minval, double8 maxval);
double16 __attribute__((overloadable)) clamp(double16 x, double16 minval, double16 maxval);
double2 __attribute__((overloadable)) clamp(double2 x, double minval, double maxval);
double3 __attribute__((overloadable)) clamp(double3 x, double minval, double maxval);
double4 __attribute__((overloadable)) clamp(double4 x, double minval, double maxval);
double8 __attribute__((overloadable)) clamp(double8 x, double minval, double maxval);
double16 __attribute__((overloadable)) clamp(double16 x, double minval, double maxval);

double __attribute__((overloadable)) degrees(double radians);
double2 __attribute__((overloadable)) degrees(double2 radians);
double3 __attribute__((overloadable)) degrees(double3 radians);
double4 __attribute__((overloadable)) degrees(double4 radians);
double8 __attribute__((overloadable)) degrees(double8 radians);
double16 __attribute__((overloadable)) degrees(double16 radians);

double __attribute__((overloadable)) max(double x, double y);
double2 __attribute__((overloadable)) max(double2 x, double2 y);
double3 __attribute__((overloadable)) max(double3 x, double3 y);
double4 __attribute__((overloadable)) max(double4 x, double4 y);
double8 __attribute__((overloadable)) max(double8 x, double8 y);
double16 __attribute__((overloadable)) max(double16 x, double16 y);
double2 __attribute__((overloadable)) max(double2 x, double y);
double3 __attribute__((overloadable)) max(double3 x, double y);
double4 __attribute__((overloadable)) max(double4 x, double y);
double8 __attribute__((overloadable)) max(double8 x, double y);
double16 __attribute__((overloadable)) max(double16 x, double y);

double __attribute__((overloadable)) min(double x, double y);
double2 __attribute__((overloadable)) min(double2 x, double2 y);
double3 __attribute__((overloadable)) min(double3 x, double3 y);
double4 __attribute__((overloadable)) min(double4 x, double4 y);
double8 __attribute__((overloadable)) min(double8 x, double8 y);
double16 __attribute__((overloadable)) min(double16 x, double16 y);
double2 __attribute__((overloadable)) min(double2 x, double y);
double3 __attribute__((overloadable)) min(double3 x, double y);
double4 __attribute__((overloadable)) min(double4 x, double y);
double8 __attribute__((overloadable)) min(double8 x, double y);
double16 __attribute__((overloadable)) min(double16 x, double y);

double __attribute__((overloadable)) mix(double x, double y, double a);
double2 __attribute__((overloadable)) mix(double2 x, double2 y, double2 a);
double3 __attribute__((overloadable)) mix(double3 x, double3 y, double3 a);
double4 __attribute__((overloadable)) mix(double4 x, double4 y, double4 a);
double8 __attribute__((overloadable)) mix(double8 x, double8 y, double8 a);
double16 __attribute__((overloadable)) mix(double16 x, double16 y, double16 a);
double2 __attribute__((overloadable)) mix(double2 x, double2 y, double a);
double3 __attribute__((overloadable)) mix(double3 x, double3 y, double a);
double4 __attribute__((overloadable)) mix(double4 x, double4 y, double a);
double8 __attribute__((overloadable)) mix(double8 x, double8 y, double a);
double16 __attribute__((overloadable)) mix(double16 x, double16 y, double a);

double __attribute__((overloadable)) radians(double degrees);
double2 __attribute__((overloadable)) radians(double2 degrees);
double3 __attribute__((overloadable)) radians(double3 degrees);
double4 __attribute__((overloadable)) radians(double4 degrees);
double8 __attribute__((overloadable)) radians(double8 degrees);
double16 __attribute__((overloadable)) radians(double16 degrees);

double __attribute__((overloadable)) step(double edge, double x);
double2 __attribute__((overloadable)) step(double2 edge, double2 x);
double3 __attribute__((overloadable)) step(double3 edge, double3 x);
double4 __attribute__((overloadable)) step(double4 edge, double4 x);
double8 __attribute__((overloadable)) step(double8 edge, double8 x);
double16 __attribute__((overloadable)) step(double16 edge, double16 x);
double2 __attribute__((overloadable)) step(double edge, double2 x);
double3 __attribute__((overloadable)) step(double edge, double3 x);
double4 __attribute__((overloadable)) step(double edge, double4 x);
double8 __attribute__((overloadable)) step(double edge, double8 x);
double16 __attribute__((overloadable)) step(double edge, double16 x);

double __attribute__((overloadable)) smoothstep(double edge0, double edge1, double x);
double2 __attribute__((overloadable)) smoothstep(double2 edge0, double2 edge1, double2 x);
double3 __attribute__((overloadable)) smoothstep(double3 edge0, double3 edge1, double3 x);
double4 __attribute__((overloadable)) smoothstep(double4 edge0, double4 edge1, double4 x);
double8 __attribute__((overloadable)) smoothstep(double8 edge0, double8 edge1, double8 x);
double16 __attribute__((overloadable)) smoothstep(double16 edge0, double16 edge1, double16 x);
double2 __attribute__((overloadable)) smoothstep(double edge0, double edge1, double2 x);
double3 __attribute__((overloadable)) smoothstep(double edge0, double edge1, double3 x);
double4 __attribute__((overloadable)) smoothstep(double edge0, double edge1, double4 x);
double8 __attribute__((overloadable)) smoothstep(double edge0, double edge1, double8 x);
double16 __attribute__((overloadable)) smoothstep(double edge0, double edge1, double16 x);

double __attribute__((overloadable)) sign(double x);
double2 __attribute__((overloadable)) sign(double2 x);
double3 __attribute__((overloadable)) sign(double3 x);
double4 __attribute__((overloadable)) sign(double4 x);
double8 __attribute__((overloadable)) sign(double8 x);
double16 __attribute__((overloadable)) sign(double16 x);

// Geometric Functions

double4 __attribute__((overloadable)) cross(double4 p0, double4 p1);
double3 __attribute__((overloadable)) cross(double3 p0, double3 p1);

double __attribute__((overloadable)) dot(double p0, double p1);
double __attribute__((overloadable)) dot(double2 p0, double2 p1);
double __attribute__((overloadable)) dot(double3 p0, double3 p1);
double __attribute__((overloadable)) dot(double4 p0, double4 p1);

double __attribute__((overloadable)) distance(double p0, double p1);
double __attribute__((overloadable)) distance(double2 p0, double2 p1);
double __attribute__((overloadable)) distance(double3 p0, double3 p1);
double __attribute__((overloadable)) distance(double4 p0, double4 p1);

double __attribute__((overloadable)) length(double p);
double __attribute__((overloadable)) length(double2 p);
double __attribute__((overloadable)) length(double3 p);
double __attribute__((overloadable)) length(double4 p);

double __attribute__((overloadable)) normalize(double p);
double2 __attribute__((overloadable)) normalize(double2 p);
double3 __attribute__((overloadable)) normalize(double3 p);
double4 __attribute__((overloadable)) normalize(double4 p);

// TODO: fp64 fast_distance, fast_length, fast_normalize?

// Relational Functions

int __attribute__((overloadable)) isequal(double x, double y);
long2 __attribute__((overloadable)) isequal(double2 x, double2 y);
long3 __attribute__((overloadable)) isequal(double3 x, double3 y);
long4 __attribute__((overloadable)) isequal(double4 x, double4 y);
long8 __attribute__((overloadable)) isequal(double8 x, double8 y);
long16 __attribute__((overloadable)) isequal(double16 x, double16 y);

int __attribute__((overloadable)) isnotequal(double x, double y);
long2 __attribute__((overloadable)) isnotequal(double2 x, double2 y);
long3 __attribute__((overloadable)) isnotequal(double3 x, double3 y);
long4 __attribute__((overloadable)) isnotequal(double4 x, double4 y);
long8 __attribute__((overloadable)) isnotequal(double8 x, double8 y);
long16 __attribute__((overloadable)) isnotequal(double16 x, double16 y);

int __attribute__((overloadable)) isgreater(double x, double y);
long2 __attribute__((overloadable)) isgreater(double2 x, double2 y);
long3 __attribute__((overloadable)) isgreater(double3 x, double3 y);
long4 __attribute__((overloadable)) isgreater(double4 x, double4 y);
long8 __attribute__((overloadable)) isgreater(double8 x, double8 y);
long16 __attribute__((overloadable)) isgreater(double16 x, double16 y);

int __attribute__((overloadable)) isgreaterequal(double x, double y);
long2 __attribute__((overloadable)) isgreaterequal(double2 x, double2 y);
long3 __attribute__((overloadable)) isgreaterequal(double3 x, double3 y);
long4 __attribute__((overloadable)) isgreaterequal(double4 x, double4 y);
long8 __attribute__((overloadable)) isgreaterequal(double8 x, double8 y);
long16 __attribute__((overloadable)) isgreaterequal(double16 x, double16 y);

int __attribute__((overloadable)) isless(double x, double y);
long2 __attribute__((overloadable)) isless(double2 x, double2 y);
long3 __attribute__((overloadable)) isless(double3 x, double3 y);
long4 __attribute__((overloadable)) isless(double4 x, double4 y);
long8 __attribute__((overloadable)) isless(double8 x, double8 y);
long16 __attribute__((overloadable)) isless(double16 x, double16 y);

int __attribute__((overloadable)) islessequal(double x, double y);
long2 __attribute__((overloadable)) islessequal(double2 x, double2 y);
long3 __attribute__((overloadable)) islessequal(double3 x, double3 y);
long4 __attribute__((overloadable)) islessequal(double4 x, double4 y);
long8 __attribute__((overloadable)) islessequal(double8 x, double8 y);
long16 __attribute__((overloadable)) islessequal(double16 x, double16 y);

int __attribute__((overloadable)) islessgreater(double x, double y);
long2 __attribute__((overloadable)) islessgreater(double2 x, double2 y);
long3 __attribute__((overloadable)) islessgreater(double3 x, double3 y);
long4 __attribute__((overloadable)) islessgreater(double4 x, double4 y);
long8 __attribute__((overloadable)) islessgreater(double8 x, double8 y);
long16 __attribute__((overloadable)) islessgreater(double16 x, double16 y);

int __attribute__((overloadable)) isfinite(double);
long2 __attribute__((overloadable)) isfinite(double2);
long3 __attribute__((overloadable)) isfinite(double3);
long4 __attribute__((overloadable)) isfinite(double4);
long8 __attribute__((overloadable)) isfinite(double8);
long16 __attribute__((overloadable)) isfinite(double16);

int __attribute__((overloadable)) isinf(double);
long2 __attribute__((overloadable)) isinf(double2);
long3 __attribute__((overloadable)) isinf(double3);
long4 __attribute__((overloadable)) isinf(double4);
long8 __attribute__((overloadable)) isinf(double8);
long16 __attribute__((overloadable)) isinf(double16);

int __attribute__((overloadable)) isnan(double);
long2 __attribute__((overloadable)) isnan(double2);
long3 __attribute__((overloadable)) isnan(double3);
long4 __attribute__((overloadable)) isnan(double4);
long8 __attribute__((overloadable)) isnan(double8);
long16 __attribute__((overloadable)) isnan(double16);

int __attribute__((overloadable)) isnormal(double);
long2 __attribute__((overloadable)) isnormal(double2);
long3 __attribute__((overloadable)) isnormal(double3);
long4 __attribute__((overloadable)) isnormal(double4);
long8 __attribute__((overloadable)) isnormal(double8);
long16 __attribute__((overloadable)) isnormal(double16);

int __attribute__((overloadable)) isordered(double x, double y);
long2 __attribute__((overloadable)) isordered(double2 x, double2 y);
long3 __attribute__((overloadable)) isordered(double3 x, double3 y);
long4 __attribute__((overloadable)) isordered(double4 x, double4 y);
long8 __attribute__((overloadable)) isordered(double8 x, double8 y);
long16 __attribute__((overloadable)) isordered(double16 x, double16 y);

int __attribute__((overloadable)) isunordered(double x, double y);
long2 __attribute__((overloadable)) isunordered(double2 x, double2 y);
long3 __attribute__((overloadable)) isunordered(double3 x, double3 y);
long4 __attribute__((overloadable)) isunordered(double4 x, double4 y);
long8 __attribute__((overloadable)) isunordered(double8 x, double8 y);
long16 __attribute__((overloadable)) isunordered(double16 x, double16 y);

int __attribute__((overloadable)) signbit(double);
long2 __attribute__((overloadable)) signbit(double2);
long3 __attribute__((overloadable)) signbit(double3);
long4 __attribute__((overloadable)) signbit(double4);
long8 __attribute__((overloadable)) signbit(double8);
long16 __attribute__((overloadable)) signbit(double16);

double __attribute__((overloadable)) bitselect(double a, double b, double c);
double2 __attribute__((overloadable)) bitselect(double2 a, double2 b, double2 c);
double3 __attribute__((overloadable)) bitselect(double3 a, double3 b, double3 c);
double4 __attribute__((overloadable)) bitselect(double4 a, double4 b, double4 c);
double8 __attribute__((overloadable)) bitselect(double8 a, double8 b, double8 c);
double16 __attribute__((overloadable)) bitselect(double16 a, double16 b, double16 c);

double __attribute__((overloadable)) select(double a, double b, long c);
double2 __attribute__((overloadable)) select(double2 a, double2 b, long2 c);
double3 __attribute__((overloadable)) select(double3 a, double3 b, long3 c);
double4 __attribute__((overloadable)) select(double4 a, double4 b, long4 c);
double8 __attribute__((overloadable)) select(double8 a, double8 b, long8 c);
double16 __attribute__((overloadable)) select(double16 a, double16 b, long16 c);
double __attribute__((overloadable)) select(double a, double b, ulong c);
double2 __attribute__((overloadable)) select(double2 a, double2 b, ulong2 c);
double3 __attribute__((overloadable)) select(double3 a, double3 b, ulong3 c);
double4 __attribute__((overloadable)) select(double4 a, double4 b, ulong4 c);
double8 __attribute__((overloadable)) select(double8 a, double8 b, ulong8 c);
double16 __attribute__((overloadable)) select(double16 a, double16 b, ulong16 c);

// Miscellaneous Vector Instructions

double2   __attribute__((overloadable)) shuffle(double2 x, ulong2 mask);
double2   __attribute__((overloadable)) shuffle(double4 x, ulong2 mask);
double2   __attribute__((overloadable)) shuffle(double8 x, ulong2 mask);
double2   __attribute__((overloadable)) shuffle(double16 x, ulong2 mask);

double4   __attribute__((overloadable)) shuffle(double2 x, ulong4 mask);
double4   __attribute__((overloadable)) shuffle(double4 x, ulong4 mask);
double4   __attribute__((overloadable)) shuffle(double8 x, ulong4 mask);
double4   __attribute__((overloadable)) shuffle(double16 x, ulong4 mask);

double8   __attribute__((overloadable)) shuffle(double2 x, ulong8 mask);
double8   __attribute__((overloadable)) shuffle(double4 x, ulong8 mask);
double8   __attribute__((overloadable)) shuffle(double8 x, ulong8 mask);
double8   __attribute__((overloadable)) shuffle(double16 x, ulong8 mask);

double16   __attribute__((overloadable)) shuffle(double2 x, ulong16 mask);
double16   __attribute__((overloadable)) shuffle(double4 x, ulong16 mask);
double16   __attribute__((overloadable)) shuffle(double8 x, ulong16 mask);
double16   __attribute__((overloadable)) shuffle(double16 x, ulong16 mask);

double2   __attribute__((overloadable)) shuffle2(double2 x, double2 y, ulong2 mask);
double2   __attribute__((overloadable)) shuffle2(double4 x, double4 y, ulong2 mask);
double2   __attribute__((overloadable)) shuffle2(double8 x, double8 y, ulong2 mask);
double2   __attribute__((overloadable)) shuffle2(double16 x, double16 y, ulong2 mask);

double4   __attribute__((overloadable)) shuffle2(double2 x, double2 y, ulong4 mask);
double4   __attribute__((overloadable)) shuffle2(double4 x, double4 y, ulong4 mask);
double4   __attribute__((overloadable)) shuffle2(double8 x, double8 y, ulong4 mask);
double4   __attribute__((overloadable)) shuffle2(double16 x, double16 y, ulong4 mask);

double8   __attribute__((overloadable)) shuffle2(double2 x, double2 y, ulong8 mask);
double8   __attribute__((overloadable)) shuffle2(double4 x, double4 y, ulong8 mask);
double8   __attribute__((overloadable)) shuffle2(double8 x, double8 y, ulong8 mask);
double8   __attribute__((overloadable)) shuffle2(double16 x, double16 y, ulong8 mask);

double16   __attribute__((overloadable)) shuffle2(double2 x, double2 y, ulong16 mask);
double16   __attribute__((overloadable)) shuffle2(double4 x, double4 y, ulong16 mask);
double16   __attribute__((overloadable)) shuffle2(double8 x, double8 y, ulong16 mask);
double16   __attribute__((overloadable)) shuffle2(double16 x, double16 y, ulong16 mask);

#endif

////////////////////////////////////////////////////////////////////////////////////
////              fp16 / fp64 conversions
////
////////////////////////////////////////////////////////////////////////////////////

#if defined(cl_khr_fp64) && defined(cl_khr_fp16)
half __attribute__((overloadable)) convert_half(double);
half __attribute__((overloadable)) convert_half_rte(double);
half __attribute__((overloadable)) convert_half_rtn(double);
half __attribute__((overloadable)) convert_half_rtp(double);
half __attribute__((overloadable)) convert_half_rtz(double);
half2 __attribute__((overloadable)) convert_half2(double2);
half2 __attribute__((overloadable)) convert_half2_rte(double2);
half2 __attribute__((overloadable)) convert_half2_rtn(double2);
half2 __attribute__((overloadable)) convert_half2_rtp(double2);
half2 __attribute__((overloadable)) convert_half2_rtz(double2);
half3 __attribute__((overloadable)) convert_half3(double3);
half3 __attribute__((overloadable)) convert_half3_rte(double3);
half3 __attribute__((overloadable)) convert_half3_rtn(double3);
half3 __attribute__((overloadable)) convert_half3_rtp(double3);
half3 __attribute__((overloadable)) convert_half3_rtz(double3);
half4 __attribute__((overloadable)) convert_half4(double4);
half4 __attribute__((overloadable)) convert_half4_rte(double4);
half4 __attribute__((overloadable)) convert_half4_rtn(double4);
half4 __attribute__((overloadable)) convert_half4_rtp(double4);
half4 __attribute__((overloadable)) convert_half4_rtz(double4);
half8 __attribute__((overloadable)) convert_half8(double8);
half8 __attribute__((overloadable)) convert_half8_rte(double8);
half8 __attribute__((overloadable)) convert_half8_rtn(double8);
half8 __attribute__((overloadable)) convert_half8_rtp(double8);
half8 __attribute__((overloadable)) convert_half8_rtz(double8);
half16 __attribute__((overloadable)) convert_half16(double16);
half16 __attribute__((overloadable)) convert_half16_rte(double16);
half16 __attribute__((overloadable)) convert_half16_rtn(double16);
half16 __attribute__((overloadable)) convert_half16_rtp(double16);
half16 __attribute__((overloadable)) convert_half16_rtz(double16);

double __attribute__((overloadable)) convert_double(half);
double __attribute__((overloadable)) convert_double_rte(half);
double __attribute__((overloadable)) convert_double_rtn(half);
double __attribute__((overloadable)) convert_double_rtp(half);
double __attribute__((overloadable)) convert_double_rtz(half);
double2 __attribute__((overloadable)) convert_double2(half2);
double2 __attribute__((overloadable)) convert_double2_rte(half2);
double2 __attribute__((overloadable)) convert_double2_rtn(half2);
double2 __attribute__((overloadable)) convert_double2_rtp(half2);
double2 __attribute__((overloadable)) convert_double2_rtz(half2);
double3 __attribute__((overloadable)) convert_double3(half3);
double3 __attribute__((overloadable)) convert_double3_rte(half3);
double3 __attribute__((overloadable)) convert_double3_rtn(half3);
double3 __attribute__((overloadable)) convert_double3_rtp(half3);
double3 __attribute__((overloadable)) convert_double3_rtz(half3);
double4 __attribute__((overloadable)) convert_double4(half4);
double4 __attribute__((overloadable)) convert_double4_rte(half4);
double4 __attribute__((overloadable)) convert_double4_rtn(half4);
double4 __attribute__((overloadable)) convert_double4_rtp(half4);
double4 __attribute__((overloadable)) convert_double4_rtz(half4);
double8 __attribute__((overloadable)) convert_double8(half8);
double8 __attribute__((overloadable)) convert_double8_rte(half8);
double8 __attribute__((overloadable)) convert_double8_rtn(half8);
double8 __attribute__((overloadable)) convert_double8_rtp(half8);
double8 __attribute__((overloadable)) convert_double8_rtz(half8);
double16 __attribute__((overloadable)) convert_double16(half16);
double16 __attribute__((overloadable)) convert_double16_rte(half16);
double16 __attribute__((overloadable)) convert_double16_rtn(half16);
double16 __attribute__((overloadable)) convert_double16_rtp(half16);
double16 __attribute__((overloadable)) convert_double16_rtz(half16);
#endif

// end of build workaround for clang separation
// Disable any extensions we may have enabled previously.
#pragma OPENCL EXTENSION all : disable

#if defined(cl_khr_fp64) && ( __OPENCL_C_VERSION__ >= CL_VERSION_1_2 )
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

#ifdef __CLANG_50__
// Clang requires this pragma to be enabled if subgroup functions are to be used.
// Not all tests follow this requirement, leave it enabled for transition period until they are fixed.
#if defined(cl_khr_subgroups)
#pragma OPENCL EXTENSION cl_khr_subgroups : enable
#endif
#endif // __CLANG_50__

#endif // #ifndef _OPENCL_CTH_

 
