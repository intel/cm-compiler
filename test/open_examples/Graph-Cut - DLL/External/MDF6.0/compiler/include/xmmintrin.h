/**
*** Copyright (C) 1985-2016 Intel Corporation.  All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
*** static char cvs_id[] = "$Id: xmmintrin.h 27360 2012-09-07 22:42:27Z plotfi $";
**/

/*
 * xmmintrin.h
 *
 * Principal header file for Streaming SIMD Extensions intrinsics
 */

#ifndef _INCLUDED_MM2
#define _INCLUDED_MM2

/*
 * The m64 type is required for the integer Streaming SIMD Extensions
 * intrinsics.
 */
#include <mmintrin.h>

#ifdef _MM2_FUNCTIONALITY
/* support old notation */
#ifndef _MM_FUNCTIONALITY
#define _MM_FUNCTIONALITY
#endif
#endif

#define _MM_ALIGN16 __declspec(align(16))

#if defined(__INTEL_COMPILER) && defined(_MM_FUNCTIONALITY)
# include "xmm_func.h"
#else
# if defined(_MSC_VER) && _MSC_FULL_VER >= 140040310
   typedef union  _MMINTRIN_TYPE(16) __m128 {
    /*
     * Although we do not recommend using these directly, they are here
     * for better MS compatibility.
     */
    float               m128_f32[4];
    unsigned __int64    m128_u64[2];
    __int8              m128_i8[16];
    __int16             m128_i16[8];
    __int32             m128_i32[4];
    __int64             m128_i64[2];
    unsigned __int8     m128_u8[16];
    unsigned __int16    m128_u16[8];
    unsigned __int32    m128_u32[4];

    /*
     * This is what we used to have here alone.
     * Leave for backward compatibility.
     */
    float f[4];
   } __m128;
# else
   typedef struct _MMINTRIN_TYPE(16) __m128 {
    float               m128_f32[4];
   } __m128;
# endif
#endif


/* Try to pick up _mm_malloc() and _mm_free() from malloc.h.  If the version
 * of malloc.h doesn't support these routines, use the functions in libirc.
 */
#if defined (_MSC_VER)
#include <malloc.h>
#endif

#if defined __cplusplus
extern "C" { /* Begin "C" */
  /* Intrinsics use C name-mangling. */
#endif /* __cplusplus */

#if !defined(_mm_malloc)
#include <stddef.h>  // defines size_t type
extern void* __ICL_INTRINCC _mm_malloc(size_t, size_t);
extern void  __ICL_INTRINCC _mm_free(void *);
#endif

/*******************************************************/
/* MACRO for shuffle parameter for _mm_shuffle_ps().   */
/* Argument fp3 is a digit[0123] that represents the fp*/
/* from argument "b" of mm_shuffle_ps that will be     */
/* placed in fp3 of result. fp2 is the same for fp2 in */
/* result. fp1 is a digit[0123] that represents the fp */
/* from argument "a" of mm_shuffle_ps that will be     */
/* places in fp1 of result. fp0 is the same for fp0 of */
/* result                                              */
/*******************************************************/
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | \
                                     ((fp1) << 2) | ((fp0)))


/*******************************************************/
/* MACRO for performing the transpose of a 4x4 matrix  */
/* of single precision floating point values.          */
/* Arguments row0, row1, row2, and row3 are __m128     */
/* values whose elements form the corresponding rows   */
/* of a 4x4 matrix.  The matrix transpose is returned  */
/* in arguments row0, row1, row2, and row3 where row0  */
/* now holds column 0 of the original matrix, row1 now */
/* holds column 1 of the original matrix, etc.         */
/*******************************************************/
#define _MM_TRANSPOSE4_PS(row0, row1, row2, row3) {                 \
            __m128 tmp3, tmp2, tmp1, tmp0;                          \
                                                                    \
            tmp0   = _mm_unpacklo_ps((row0), (row1));               \
            tmp2   = _mm_unpacklo_ps((row2), (row3));               \
            tmp1   = _mm_unpackhi_ps((row0), (row1));               \
            tmp3   = _mm_unpackhi_ps((row2), (row3));               \
                                                                    \
            (row0) = _mm_movelh_ps(tmp0, tmp2);                     \
            (row1) = _mm_movehl_ps(tmp2, tmp0);                     \
            (row2) = _mm_movelh_ps(tmp1, tmp3);                     \
            (row3) = _mm_movehl_ps(tmp3, tmp1);                     \
    }


/* constants for use with _mm_prefetch */
#define _MM_HINT_T0 1
#define _MM_HINT_T1 2
#define _MM_HINT_T2 3
#define _MM_HINT_NTA    0
#define _MM_HINT_ENTA   4
#define _MM_HINT_ET0    5
#define _MM_HINT_ET1    6
#define _MM_HINT_ET2    7

/* MACRO functions for setting and reading the MXCSR */
#define _MM_EXCEPT_MASK       0x003f
#define _MM_EXCEPT_INVALID    0x0001
#define _MM_EXCEPT_DENORM     0x0002
#define _MM_EXCEPT_DIV_ZERO   0x0004
#define _MM_EXCEPT_OVERFLOW   0x0008
#define _MM_EXCEPT_UNDERFLOW  0x0010
#define _MM_EXCEPT_INEXACT    0x0020

#define _MM_MASK_MASK         0x1f80
#define _MM_MASK_INVALID      0x0080
#define _MM_MASK_DENORM       0x0100
#define _MM_MASK_DIV_ZERO     0x0200
#define _MM_MASK_OVERFLOW     0x0400
#define _MM_MASK_UNDERFLOW    0x0800
#define _MM_MASK_INEXACT      0x1000

#define _MM_ROUND_MASK        0x6000
#define _MM_ROUND_NEAREST     0x0000
#define _MM_ROUND_DOWN        0x2000
#define _MM_ROUND_UP          0x4000
#define _MM_ROUND_TOWARD_ZERO 0x6000

#define _MM_FLUSH_ZERO_MASK   0x8000
#define _MM_FLUSH_ZERO_ON     0x8000
#define _MM_FLUSH_ZERO_OFF    0x0000

#define _MM_SET_EXCEPTION_STATE(mask)                               \
            _mm_setcsr((_mm_getcsr() & ~_MM_EXCEPT_MASK) | (mask))
#define _MM_GET_EXCEPTION_STATE()                                   \
            (_mm_getcsr() & _MM_EXCEPT_MASK)

#define _MM_SET_EXCEPTION_MASK(mask)                                \
            _mm_setcsr((_mm_getcsr() & ~_MM_MASK_MASK) | (mask))
#define _MM_GET_EXCEPTION_MASK()                                    \
            (_mm_getcsr() & _MM_MASK_MASK)

#define _MM_SET_ROUNDING_MODE(mode)                                 \
            _mm_setcsr((_mm_getcsr() & ~_MM_ROUND_MASK) | (mode))
#define _MM_GET_ROUNDING_MODE()                                     \
            (_mm_getcsr() & _MM_ROUND_MASK)

#define _MM_SET_FLUSH_ZERO_MODE(mode)                               \
            _mm_setcsr((_mm_getcsr() & ~_MM_FLUSH_ZERO_MASK) | (mode))
#define _MM_GET_FLUSH_ZERO_MODE(mode)                               \
            (_mm_getcsr() & _MM_FLUSH_ZERO_MASK)

/*****************************************************/
/*     INTRINSICS FUNCTION PROTOTYPES START HERE     */
/*****************************************************/

/*
 * FP, arithmetic
 */

extern __m128 __ICL_INTRINCC _mm_add_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_add_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_sub_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_sub_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_mul_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_mul_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_div_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_div_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_sqrt_ss(__m128);
extern __m128 __ICL_INTRINCC _mm_sqrt_ps(__m128);
extern __m128 __ICL_INTRINCC _mm_rcp_ss(__m128);
extern __m128 __ICL_INTRINCC _mm_rcp_ps(__m128);
extern __m128 __ICL_INTRINCC _mm_rsqrt_ss(__m128);
extern __m128 __ICL_INTRINCC _mm_rsqrt_ps(__m128);
extern __m128 __ICL_INTRINCC _mm_min_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_min_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_max_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_max_ps(__m128, __m128);

/*
 * FP, logical
 */

extern __m128 __ICL_INTRINCC _mm_and_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_andnot_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_or_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_xor_ps(__m128, __m128);

/*
 * FP, comparison
 */

extern __m128 __ICL_INTRINCC _mm_cmpeq_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpeq_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmplt_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmplt_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmple_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmple_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpgt_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpgt_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpge_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpge_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpneq_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpneq_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpnlt_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpnlt_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpnle_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpnle_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpngt_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpngt_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpnge_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpnge_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpord_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpord_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpunord_ss(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_cmpunord_ps(__m128, __m128);
extern int __ICL_INTRINCC _mm_comieq_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_comilt_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_comile_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_comigt_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_comige_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_comineq_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_ucomieq_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_ucomilt_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_ucomile_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_ucomigt_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_ucomige_ss(__m128, __m128);
extern int __ICL_INTRINCC _mm_ucomineq_ss(__m128, __m128);

/*
 * FP, conversions
 */

extern int    __ICL_INTRINCC _mm_cvt_ss2si(__m128);
extern __m64  __ICL_INTRINCC _mm_cvt_ps2pi(__m128);
extern int    __ICL_INTRINCC _mm_cvtt_ss2si(__m128);
extern __m64  __ICL_INTRINCC _mm_cvtt_ps2pi(__m128);
extern __m128 __ICL_INTRINCC _mm_cvt_si2ss(__m128, int);
extern __m128 __ICL_INTRINCC _mm_cvt_pi2ps(__m128, __m64);

/*
 * FP, misc
 */

extern __m128 __ICL_INTRINCC _mm_shuffle_ps(__m128, __m128, unsigned int);
extern __m128 __ICL_INTRINCC _mm_unpackhi_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_unpacklo_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_loadh_pi(__m128, __m64 const*);
extern __m128 __ICL_INTRINCC _mm_movehl_ps(__m128, __m128);
extern __m128 __ICL_INTRINCC _mm_movelh_ps(__m128, __m128);
extern void   __ICL_INTRINCC _mm_storeh_pi(__m64 *, __m128);
extern __m128 __ICL_INTRINCC _mm_loadl_pi(__m128, __m64 const*);
extern void   __ICL_INTRINCC _mm_storel_pi(__m64 *, __m128);
extern int    __ICL_INTRINCC _mm_movemask_ps(__m128);


/*
 * Integer extensions
 */
extern int   __ICL_INTRINCC _m_pextrw(__m64, int);
extern __m64 __ICL_INTRINCC _m_pinsrw(__m64, int, int);
extern __m64 __ICL_INTRINCC _m_pmaxsw(__m64, __m64);
extern __m64 __ICL_INTRINCC _m_pmaxub(__m64, __m64);
extern __m64 __ICL_INTRINCC _m_pminsw(__m64, __m64);
extern __m64 __ICL_INTRINCC _m_pminub(__m64, __m64);
extern int   __ICL_INTRINCC _m_pmovmskb(__m64);
extern __m64 __ICL_INTRINCC _m_pmulhuw(__m64, __m64);
extern __m64 __ICL_INTRINCC _m_pshufw(__m64, int);
extern void  __ICL_INTRINCC _m_maskmovq(__m64, __m64, char *);
extern __m64 __ICL_INTRINCC _m_pavgb(__m64, __m64);
extern __m64 __ICL_INTRINCC _m_pavgw(__m64, __m64);
extern __m64 __ICL_INTRINCC _m_psadbw(__m64, __m64);


/*
 * memory & initialization
 */

extern __m128 __ICL_INTRINCC _mm_set_ss(float);
extern __m128 __ICL_INTRINCC _mm_set_ps1(float);
extern __m128 __ICL_INTRINCC _mm_set_ps(float, float, float, float);
extern __m128 __ICL_INTRINCC _mm_setr_ps(float, float, float, float);
extern __m128 __ICL_INTRINCC _mm_setzero_ps(void);
extern __m128 __ICL_INTRINCC _mm_load_ss(float const*);
extern __m128 __ICL_INTRINCC _mm_load_ps1(float const*);
extern __m128 __ICL_INTRINCC _mm_load_ps(float const*);
extern __m128 __ICL_INTRINCC _mm_loadr_ps(float const*);
extern __m128 __ICL_INTRINCC _mm_loadu_ps(float const*);
extern void __ICL_INTRINCC _mm_store_ss(float *, __m128);
extern void __ICL_INTRINCC _mm_store_ps1(float *, __m128);
extern void __ICL_INTRINCC _mm_store_ps(float *, __m128);
extern void __ICL_INTRINCC _mm_storer_ps(float *, __m128);
extern void __ICL_INTRINCC _mm_storeu_ps(float *, __m128);
extern void __ICL_INTRINCC _mm_prefetch(char const*, int);
extern void __ICL_INTRINCC _mm_stream_pi(__m64 *, __m64);
extern void __ICL_INTRINCC _mm_stream_ps(float *, __m128);
extern __m128 __ICL_INTRINCC _mm_move_ss(__m128, __m128);
extern float  __ICL_INTRINCC _mm_cvtss_f32(__m128);

extern void __ICL_INTRINCC _mm_sfence(void);
extern unsigned int __ICL_INTRINCC _mm_getcsr(void);
extern void __ICL_INTRINCC _mm_setcsr(unsigned int);

/*
 * Support for 64-bit extension intrinsics
 */
#if defined(__x86_64) || defined(_M_X64)
extern __int64 __ICL_INTRINCC _mm_cvtss_si64(__m128);
extern __int64 __ICL_INTRINCC _mm_cvttss_si64(__m128);
extern __m128  __ICL_INTRINCC _mm_cvtsi64_ss(__m128, __int64);
#endif

/* Alternate intrinsic names definition */
#define _mm_cvtss_si32    _mm_cvt_ss2si
#define _mm_cvtps_pi32    _mm_cvt_ps2pi
#define _mm_cvttss_si32   _mm_cvtt_ss2si
#define _mm_cvttps_pi32   _mm_cvtt_ps2pi
#define _mm_cvtsi32_ss    _mm_cvt_si2ss
#define _mm_cvtpi32_ps    _mm_cvt_pi2ps
#define _mm_extract_pi16  _m_pextrw
#define _mm_insert_pi16   _m_pinsrw
#define _mm_max_pi16      _m_pmaxsw
#define _mm_max_pu8       _m_pmaxub
#define _mm_min_pi16      _m_pminsw
#define _mm_min_pu8       _m_pminub
#define _mm_movemask_pi8  _m_pmovmskb
#define _mm_mulhi_pu16    _m_pmulhuw
#define _mm_shuffle_pi16  _m_pshufw
#define _mm_maskmove_si64 _m_maskmovq
#define _mm_avg_pu8       _m_pavgb
#define _mm_avg_pu16      _m_pavgw
#define _mm_sad_pu8       _m_psadbw
#define _mm_set1_ps       _mm_set_ps1
#define _mm_load1_ps      _mm_load_ps1
#define _mm_store1_ps     _mm_store_ps1


/******************************************************/
/* UTILITY INTRINSICS FUNCTION DEFINITIONS START HERE */
/******************************************************/

/*********************************************************/
/*  NAME : _mm_cvtpi16_ps                                */
/*  DESCRIPTION : Convert 4 16-bit signed integer values */
/*                to 4 single-precision float values     */
/*  IN : __m64 a                                         */
/*  OUT : none                                           */
/*  RETURN : __m128 : (float)a                           */
/*********************************************************/
#define _mm_cvtpi16_ps(a)                                                    \
    _mm_cvtpi32_ps(                                                          \
        _mm_movelh_ps(                                                       \
            _mm_setzero_ps(),                                                \
            _mm_cvtpi32_ps(_mm_setzero_ps(),                                 \
                           _mm_unpackhi_pi16(                                \
                               (a),                                          \
                               _mm_cmpgt_pi16(_mm_setzero_si64(), (a))))),   \
        _mm_unpacklo_pi16((a), _mm_cmpgt_pi16(_mm_setzero_si64(), (a))))


/***********************************************************/
/*  NAME : _mm_cvtpu16_ps                                  */
/*  DESCRIPTION : Convert 4 16-bit unsigned integer values */
/*                to 4 single-precision float values       */
/*  IN : __m64 a                                           */
/*  OUT : none                                             */
/*  RETURN : __m128 : (float)a                             */
/***********************************************************/
#define _mm_cvtpu16_ps(a)                                                    \
    _mm_cvtpi32_ps(                                                          \
        _mm_movelh_ps(                                                       \
            _mm_setzero_ps(),                                                \
            _mm_cvtpi32_ps(_mm_setzero_ps(),                                 \
                           _mm_unpackhi_pi16((a), _mm_setzero_si64()))),     \
        _mm_unpacklo_pi16((a), _mm_setzero_si64()))

/******************************************************/
/*  NAME : _mm_cvtps_pi16                             */
/*  DESCRIPTION : Convert 4 single-precision float    */
/*                values to 4 16-bit integer values   */
/*  IN : __m128 a                                     */
/*  OUT : none                                        */
/*  RETURN : __m64 : (short)a                         */
/******************************************************/
#define _mm_cvtps_pi16(a)                                                    \
    _mm_packs_pi32(_mm_cvtps_pi32(a),                                        \
                   _mm_cvtps_pi32(_mm_movehl_ps((a), (a))))


/******************************************************/
/*  NAME : _mm_cvtpi8_ps                              */
/*  DESCRIPTION : Convert 4 8-bit integer values to 4 */
/*                single-precision float values       */
/*  IN : __m64 a                                      */
/*  OUT : none                                        */
/*  RETURN : __m128 : (float)a                        */
/******************************************************/
#define _mm_cvtpi8_ps(a)                                                    \
    _mm_cvtpi16_ps(                                                         \
        _mm_unpacklo_pi8((a), _mm_cmpgt_pi8(_mm_setzero_si64(), (a))))


/******************************************************/
/*  NAME : _mm_cvtpu8_ps                              */
/*  DESCRIPTION : Convert 4 8-bit unsigned integer    */
/*                values to 4 single-precision float  */
/*                values                              */
/*  IN : __m64 a                                      */
/*  OUT : none                                        */
/*  RETURN : __m128 : (float)a                        */
/******************************************************/
#define _mm_cvtpu8_ps(a)                                                    \
    _mm_cvtpu16_ps(_mm_unpacklo_pi8((a), _mm_setzero_si64()))


/******************************************************/
/*  NAME : _mm_cvtps_pi8                              */
/*  DESCRIPTION : Convert 4 single-precision float    */
/*                values to 4 8-bit integer values    */
/*  IN : __m128 a                                     */
/*  OUT : none                                        */
/*  RETURN : __m64 : (char)a                          */
/******************************************************/
#define _mm_cvtps_pi8(a)                                                    \
    _mm_packs_pi16(_mm_cvtps_pi16(a), _mm_setzero_si64())


/******************************************************/
/*  NAME : _mm_cvtpi32x2_ps                           */
/*  DESCRIPTION : Convert 4 32-bit integer values     */
/*                to 4 single-precision float values  */
/*  IN : __m64 a : operand 1                          */
/*       __m64 b : operand 2                          */
/*  OUT : none                                        */
/*  RETURN : __m128 : (float)a,(float)b               */
/******************************************************/
#define _mm_cvtpi32x2_ps(a, b)                                                \
    _mm_movelh_ps(_mm_cvt_pi2ps(_mm_setzero_ps(), (a)),                       \
                  _mm_cvt_pi2ps(_mm_setzero_ps(), (b)))


#if defined __cplusplus
}; /* End "C" */
#endif /* __cplusplus */

/*
 * Include the SSE2 definitions for compatibility with gcc.
 */
#if defined (__linux__) || defined (__QNX__) ||     \
    defined (__VXWORKS__) || defined(__APPLE__) ||  \
    defined(__NetBSD__) || defined(__FreeBSD__)
#include <emmintrin.h>
#endif

#endif  /* _INCLUDED_MM2 */
