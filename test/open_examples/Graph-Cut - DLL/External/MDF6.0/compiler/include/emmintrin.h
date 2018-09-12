/**
***
*** Copyright (C) 1985-2016 Intel Corporation.  All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation and may not be disclosed, examined
*** or reproduced in whole or in part except as expressly provided
*** by the accompanying LICENSE AGREEMENT
***
*** static char cvs_id[] = "$Id: emmintrin.h 27360 2012-09-07 22:42:27Z plotfi $";
**/

/*
 * emmintrin.h
 *
 * Principal header file for Intel(R) Pentium(R) 4 processor SSE2 intrinsics
 */

#ifndef _INCLUDED_EMM
#define _INCLUDED_EMM

/*
 * Macro function for shuffle
 */
#define _MM_SHUFFLE2(x,y) (((x)<<1) | (y))

#if defined(_SSE2_INTEGER_FOR_PENTIUMIII)
#include <sse2mmx.h>
#define _MM_EMPTY() _mm_empty()
#elif defined(_SSE2_INTEGER_FOR_ITANIUM)
#include <sse2mmx.h>
#define _MM_EMPTY()
#else

#define _MM_EMPTY()

/*
 * the __m128 & __m64 types are required for the intrinsics
 */
#include <xmmintrin.h>

typedef struct _MMINTRIN_TYPE(16) __m128d {
    double              m128d_f64[2];
} __m128d;

typedef union  _MMINTRIN_TYPE(16) __m128i {
#if !defined(_MSC_VER)
     /*
      * To support GNU compatible intialization with initializers list,
      * make first union member to be of int64 type.
      */
     __int64             m128i_gcc_compatibility[2];
#endif
    /*
     * Although we do not recommend using these directly, they are here
     * for better MS compatibility.
     */
    __int8              m128i_i8[16];
    __int16             m128i_i16[8];
    __int32             m128i_i32[4];
    __int64             m128i_i64[2];
    unsigned __int8     m128i_u8[16];
    unsigned __int16    m128i_u16[8];
    unsigned __int32    m128i_u32[4];
    unsigned __int64    m128i_u64[2];

    /*
     * This is what we used to have here alone.
     * Leave for backward compatibility.
     */
    char c[16];
} __m128i;


/*****************************************************/
/*     INTRINSICS FUNCTION PROTOTYPES START HERE     */
/*****************************************************/

#if defined __cplusplus
extern "C" { /* Begin "C" */
  /* Intrinsics use C name-mangling. */
#endif /* __cplusplus */

/*
 * DP, arithmetic
 */

extern __m128d __ICL_INTRINCC _mm_add_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_add_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_sub_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_sub_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_mul_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_mul_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_sqrt_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_sqrt_pd(__m128d);
extern __m128d __ICL_INTRINCC _mm_div_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_div_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_min_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_min_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_max_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_max_pd(__m128d, __m128d);

/*
 * DP, logicals
 */

extern __m128d __ICL_INTRINCC _mm_and_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_andnot_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_or_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_xor_pd(__m128d, __m128d);

/*
 * DP, comparisons
 */

extern __m128d __ICL_INTRINCC _mm_cmpeq_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpeq_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmplt_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmplt_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmple_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmple_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpgt_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpgt_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpge_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpge_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpneq_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpneq_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpnlt_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpnlt_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpnle_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpnle_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpngt_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpngt_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpnge_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpnge_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpord_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpord_sd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpunord_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_cmpunord_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_comieq_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_comilt_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_comile_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_comigt_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_comige_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_comineq_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_ucomieq_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_ucomilt_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_ucomile_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_ucomigt_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_ucomige_sd(__m128d, __m128d);
extern int __ICL_INTRINCC _mm_ucomineq_sd(__m128d, __m128d);

/*
 * DP, converts
 */

extern __m128d __ICL_INTRINCC _mm_cvtepi32_pd(__m128i);
extern __m128i __ICL_INTRINCC _mm_cvtpd_epi32(__m128d);
extern __m128i __ICL_INTRINCC _mm_cvttpd_epi32(__m128d);
extern __m128  __ICL_INTRINCC _mm_cvtepi32_ps(__m128i);
extern __m128i __ICL_INTRINCC _mm_cvtps_epi32(__m128);
extern __m128i __ICL_INTRINCC _mm_cvttps_epi32(__m128);
extern __m128  __ICL_INTRINCC _mm_cvtpd_ps(__m128d);
extern __m128d __ICL_INTRINCC _mm_cvtps_pd(__m128);
extern __m128  __ICL_INTRINCC _mm_cvtsd_ss(__m128, __m128d);
extern double  __ICL_INTRINCC _mm_cvtsd_f64(__m128d);
extern __m128d __ICL_INTRINCC _mm_cvtss_sd(__m128d, __m128);

extern int     __ICL_INTRINCC _mm_cvtsd_si32(__m128d);
extern int     __ICL_INTRINCC _mm_cvttsd_si32(__m128d);
extern __m128d __ICL_INTRINCC _mm_cvtsi32_sd(__m128d, int);

extern __m64   __ICL_INTRINCC _mm_cvtpd_pi32(__m128d);
extern __m64   __ICL_INTRINCC _mm_cvttpd_pi32(__m128d);
extern __m128d __ICL_INTRINCC _mm_cvtpi32_pd(__m64);

/*
 * DP, misc
 */

extern __m128d __ICL_INTRINCC _mm_unpackhi_pd(__m128d, __m128d);
extern __m128d __ICL_INTRINCC _mm_unpacklo_pd(__m128d, __m128d);
extern int     __ICL_INTRINCC _mm_movemask_pd(__m128d);
extern __m128d __ICL_INTRINCC _mm_shuffle_pd(__m128d, __m128d, int);

/*
 * DP, loads
 */

extern __m128d __ICL_INTRINCC _mm_load_pd(double const*);
extern __m128d __ICL_INTRINCC _mm_load1_pd(double const*);
extern __m128d __ICL_INTRINCC _mm_loadr_pd(double const*);
extern __m128d __ICL_INTRINCC _mm_loadu_pd(double const*);
extern __m128d __ICL_INTRINCC _mm_load_sd(double const*);
extern __m128d __ICL_INTRINCC _mm_loadh_pd(__m128d, double const*);
extern __m128d __ICL_INTRINCC _mm_loadl_pd(__m128d, double const*);

/*
 * DP, sets
 */

extern __m128d __ICL_INTRINCC _mm_set_sd(double);
extern __m128d __ICL_INTRINCC _mm_set1_pd(double);
extern __m128d __ICL_INTRINCC _mm_set_pd(double, double);
extern __m128d __ICL_INTRINCC _mm_setr_pd(double, double);
extern __m128d __ICL_INTRINCC _mm_setzero_pd(void);
extern __m128d __ICL_INTRINCC _mm_move_sd(__m128d, __m128d);

/*
 * DP, stores
 */

extern void __ICL_INTRINCC _mm_store_sd(double *, __m128d);
extern void __ICL_INTRINCC _mm_store1_pd(double *, __m128d);
extern void __ICL_INTRINCC _mm_store_pd(double *, __m128d);
extern void __ICL_INTRINCC _mm_storeu_pd(double *, __m128d);
extern void __ICL_INTRINCC _mm_storer_pd(double *, __m128d);
extern void __ICL_INTRINCC _mm_storeh_pd(double *, __m128d);
extern void __ICL_INTRINCC _mm_storel_pd(double *, __m128d);

/*
 * Integer, arithmetic
 */

extern __m128i __ICL_INTRINCC _mm_add_epi8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_add_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_add_epi32(__m128i, __m128i);
extern __m64   __ICL_INTRINCC _mm_add_si64(__m64, __m64);
extern __m128i __ICL_INTRINCC _mm_add_epi64(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_adds_epi8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_adds_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_adds_epu8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_adds_epu16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_avg_epu8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_avg_epu16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_madd_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_max_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_max_epu8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_min_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_min_epu8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_mulhi_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_mulhi_epu16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_mullo_epi16(__m128i, __m128i);
extern __m64   __ICL_INTRINCC _mm_mul_su32(__m64, __m64);
extern __m128i __ICL_INTRINCC _mm_mul_epu32(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_sad_epu8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_sub_epi8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_sub_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_sub_epi32(__m128i, __m128i);
extern __m64   __ICL_INTRINCC _mm_sub_si64(__m64, __m64);
extern __m128i __ICL_INTRINCC _mm_sub_epi64(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_subs_epi8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_subs_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_subs_epu8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_subs_epu16(__m128i, __m128i);

/*
 * Integer, logicals
 */

extern __m128i __ICL_INTRINCC _mm_and_si128(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_andnot_si128(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_or_si128(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_xor_si128(__m128i, __m128i);

/*
 * Integer, shifts
 */

extern __m128i __ICL_INTRINCC _mm_bslli_si128(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_slli_si128(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_slli_epi16(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_sll_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_slli_epi32(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_sll_epi32(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_slli_epi64(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_sll_epi64(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_srai_epi16(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_sra_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_srai_epi32(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_sra_epi32(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_bsrli_si128(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_srli_si128(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_srli_epi16(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_srl_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_srli_epi32(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_srl_epi32(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_srli_epi64(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_srl_epi64(__m128i, __m128i);

/*
 * Integer, comparisons
 */

extern __m128i __ICL_INTRINCC _mm_cmpeq_epi8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_cmpeq_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_cmpeq_epi32(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_cmpgt_epi8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_cmpgt_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_cmpgt_epi32(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_cmplt_epi8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_cmplt_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_cmplt_epi32(__m128i, __m128i);

/*
 * Integer, converts
 */

extern __m128i __ICL_INTRINCC _mm_cvtsi32_si128(int);
extern int     __ICL_INTRINCC _mm_cvtsi128_si32(__m128i);

/*
 * Integer, misc
 */

extern __m128i __ICL_INTRINCC _mm_packs_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_packs_epi32(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_packus_epi16(__m128i, __m128i);
extern int     __ICL_INTRINCC _mm_extract_epi16(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_insert_epi16(__m128i, int, int);
extern int     __ICL_INTRINCC _mm_movemask_epi8(__m128i);
extern __m128i __ICL_INTRINCC _mm_shuffle_epi32(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_shufflehi_epi16(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_shufflelo_epi16(__m128i, int);
extern __m128i __ICL_INTRINCC _mm_unpackhi_epi8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_unpackhi_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_unpackhi_epi32(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_unpackhi_epi64(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_unpacklo_epi8(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_unpacklo_epi16(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_unpacklo_epi32(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_unpacklo_epi64(__m128i, __m128i);
extern __m128i __ICL_INTRINCC _mm_move_epi64(__m128i);
extern __m128i __ICL_INTRINCC _mm_movpi64_epi64(__m64);
extern __m64   __ICL_INTRINCC _mm_movepi64_pi64(__m128i);


/*
 * Integer, loads
 */

extern __m128i __ICL_INTRINCC _mm_load_si128(__m128i const*);
extern __m128i __ICL_INTRINCC _mm_loadu_si128(__m128i const*);
extern __m128i __ICL_INTRINCC _mm_loadl_epi64(__m128i const*);

/*
 * Integer, sets
 */

extern __m128i __ICL_INTRINCC _mm_set_epi64(__m64, __m64);
extern __m128i __ICL_INTRINCC _mm_set_epi32(int, int, int, int);
extern __m128i __ICL_INTRINCC _mm_set_epi16(short, short, short, short,
                                            short, short, short, short);
extern __m128i __ICL_INTRINCC _mm_set_epi8(char, char, char, char,
                                           char, char, char, char,
                                           char, char, char, char,
                                           char, char, char, char);
extern __m128i __ICL_INTRINCC _mm_set1_epi64(__m64);
extern __m128i __ICL_INTRINCC _mm_set1_epi32(int);
extern __m128i __ICL_INTRINCC _mm_set1_epi16(short);
extern __m128i __ICL_INTRINCC _mm_set1_epi8(char);
extern __m128i __ICL_INTRINCC _mm_setr_epi64(__m64, __m64);
extern __m128i __ICL_INTRINCC _mm_setr_epi32(int, int, int, int);
extern __m128i __ICL_INTRINCC _mm_setr_epi16(short, short, short, short,
                                             short, short, short, short);
extern __m128i __ICL_INTRINCC _mm_setr_epi8(char, char, char, char,
                                            char, char, char, char,
                                            char, char, char, char,
                                            char, char, char, char);
extern __m128i __ICL_INTRINCC _mm_setzero_si128();

/*
 * Integer, stores
 */

extern void __ICL_INTRINCC _mm_store_si128(__m128i *, __m128i);
extern void __ICL_INTRINCC _mm_storeu_si128(__m128i *, __m128i);
extern void __ICL_INTRINCC _mm_storel_epi64(__m128i *, __m128i);
extern void __ICL_INTRINCC _mm_maskmoveu_si128(__m128i, __m128i, char *);

/*
 * Cacheability support
 */

extern void __ICL_INTRINCC _mm_stream_pd(double *, __m128d);
extern void __ICL_INTRINCC _mm_stream_si128(__m128i *, __m128i);
extern void __ICL_INTRINCC _mm_clflush(void const*);
extern void __ICL_INTRINCC _mm_lfence(void);
extern void __ICL_INTRINCC _mm_mfence(void);
extern void __ICL_INTRINCC _mm_stream_si32(int *, int);
extern void __ICL_INTRINCC _mm_pause(void);

/*
 * Support for casting between various SP, DP, INT vector types.
 * Note that these do no conversion of values, they just change
 * the type.
 */
extern __m128  __ICL_INTRINCC _mm_castpd_ps(__m128d);
extern __m128i __ICL_INTRINCC _mm_castpd_si128(__m128d);
extern __m128d __ICL_INTRINCC _mm_castps_pd(__m128);
extern __m128i __ICL_INTRINCC _mm_castps_si128(__m128);
extern __m128  __ICL_INTRINCC _mm_castsi128_ps(__m128i);
extern __m128d __ICL_INTRINCC _mm_castsi128_pd(__m128i);

/*
 * Support for 64-bit extension intrinsics
 */
extern __m128i __ICL_INTRINCC _mm_cvtsi64_si128(__int64);
extern __int64 __ICL_INTRINCC _mm_cvtsi128_si64(__m128i);
extern __m128i __ICL_INTRINCC _mm_set1_epi64x(__int64);
extern __m128i __ICL_INTRINCC _mm_set_epi64x(__int64, __int64);

#if defined(__x86_64) || defined(_M_X64)
extern __int64 __ICL_INTRINCC _mm_cvtsd_si64(__m128d);
extern __int64 __ICL_INTRINCC _mm_cvttsd_si64(__m128d);
extern __m128d __ICL_INTRINCC _mm_cvtsi64_sd(__m128d, __int64);
extern    void __ICL_INTRINCC _mm_stream_si64(__int64 *, __int64);
#endif

/*
 * Support for half-float conversions to/from normal float.
 * Immediate argument is used for special MXCSR overrides.
 */
extern float          __ICL_INTRINCC _cvtsh_ss(unsigned short);
extern unsigned short __ICL_INTRINCC _cvtss_sh(float, int);

extern __m128  __ICL_INTRINCC _mm_cvtph_ps(__m128i);
extern __m128i __ICL_INTRINCC _mm_cvtps_ph(__m128, int);

// Alternate intrinsic names definition
#define _mm_load_pd1 _mm_load1_pd
#define _mm_set_pd1 _mm_set1_pd
#define _mm_store_pd1 _mm_store1_pd
#define _mm_cvtsi64x_si128 _mm_cvtsi64_si128
#define _mm_cvtsi128_si64x _mm_cvtsi128_si64

#if defined(__x86_64) || defined(_M_X64)
#define _mm_cvtsd_si64x _mm_cvtsd_si64
#define _mm_cvttsd_si64x _mm_cvttsd_si64
#define _mm_cvtsi64x_sd _mm_cvtsi64_sd
#endif

#if defined __cplusplus
}; /* End "C" */
#endif /* __cplusplus */

#endif /* _SSE2_INTEGER_FOR_PENTIUMIII */
#endif /* _INCLUDED_EMM */
