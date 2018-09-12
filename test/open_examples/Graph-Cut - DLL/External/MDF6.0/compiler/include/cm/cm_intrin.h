/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: cm_intrin.h 26170 2011-10-29 00:48:24Z kchen24 $"
*** -----------------------------------------------------------------------------------------------
***
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:             Alexey V. Aliev
***                      
***                      
***                      
***
*** Description: Cm intrinsics
***
*** -----------------------------------------------------------------------------------------------
**/

//TODO: implement: rmd, line

#ifndef CM_INTRIN_H
#define CM_INTRIN_H

#include <stdio.h>
#include <math.h>

#include "cm_common_macros.h"

CM_API extern void cm_asm(const char *asm_line);
CM_API extern void *cm_emu_malloc(size_t memSize);
CM_API extern void cm_emu_free(void *memPtr);
CM_API extern void cm_cisa(const char *cisa_line);

/*
#ifdef CM_EMU
#ifdef CM_GENX
#define SIMDCF_WRAPPER(X, SZ, i) \
        if (__CMInternal__::getWorkingStack() && !__CMInternal__::getWorkingStack()->isEmpty() && (SZ > 1))  { \
            if ((short)(__CMInternal__::getSIMDMarker() << (i)) < 0) \
                X;\
        } else { \
            X; \
        }
#else
#define SIMDCF_WRAPPER(X, SZ, i) X
#endif
#else
#define SIMDCF_WRAPPER(X, SZ, i) X
#endif

#ifdef CM_EMU
#ifdef CM_GENX
#define SIMDCF_ELEMENT_SKIP(i) \
        if (__CMInternal__::getWorkingStack() && !__CMInternal__::getWorkingStack()->isEmpty()) \
            if ((short)(__CMInternal__::getSIMDMarker() << (i)) >= 0) \
                continue; \

#else
#define SIMDCF_ELEMENT_SKIP(i)
#endif
#else
#define SIMDCF_ELEMENT_SKIP(i)
#endif
*/

/* Some extras for float rounding support */
#ifdef CM_EMU
#ifdef __GNUC__
#include <fenv.h>
#else
#include <float.h>
#pragma fenv_access (on)
#endif
#endif

/* Half library support for use in emu mode */
#ifdef CM_EMU
#include "half.h"
#endif

/* Abs */
template<typename RT ,typename T, uint SZ>
CM_API extern vector<RT, SZ>
cm_abs(const stream<T,SZ>& src0, const uint flags = 0)
{
    int i;
    typename abstype<T>::type  ret;
    vector<RT, SZ> retv;
    
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) < 0) {
            ret = -(src0.get(i));
        } else {
            ret = (src0.get(i));
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

    return retv;
}

template<typename RT,typename T>
CM_API extern RT
cm_abs(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    vector<T, 1> v(src0);
    vector<RT, 1> ret;
    ret = cm_abs<RT>(v, flags);
    return ret(0);
}


/* Max */
template<typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_max(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1, const uint flags = 0)
{
    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) >= src1.get(i)) {
            ret = src0.get(i);
        } 
        else {
            ret = src1.get(i);
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

    return retv;
}

template<typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_max(const stream<T1,SZ>& src0, const T2 src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        v1(i) = src0.get(i);
    }

    retv = cm_max<RT>(v1, v2, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_max(const T1 src0, const stream<T2,SZ>& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        v2(i) = src1.get(i);
    }

    retv = cm_max<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API extern RT
cm_max(const T1& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;
    
    retv = cm_max<RT>(v1, v2, flags);

    return retv(0);
}


/* Min */
template<typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_min(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1, const uint flags = 0)
{
    int i;
    RT ret;
    vector<RT, SZ> retv;
    
    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) < src1.get(i)) {
            ret = RT(src0.get(i));
        } 
        else {
            ret = RT(src1.get(i));
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

    return retv;
}

template<typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_min(const stream<T1,SZ>& src0, const T2 src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        v1(i) = src0.get(i);
    }

    retv = cm_min<RT>(v1, v2, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_min(const T1 src0, const stream<T2,SZ>& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        v2(i) = src1.get(i);
    }

    retv = cm_min<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API extern RT
cm_min(const T1& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;
    
    retv = cm_min<RT>(v1, v2, flags);

    return retv(0);
}

//************************ Arithmetics ****************************
//CM QUOT
template <typename T, uint SZ>
CM_API extern vector<T, SZ>
cm_quot(const stream<T, SZ> &src0, const stream<T, SZ> &src1, 
        const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1.get(i);
    }
      
    return retv;
}   

template <typename T, uint SZ>
CM_API extern vector<T, SZ>
cm_quot(const stream<T, SZ> &src0, const T &src1, 
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1;
    }
      
    return retv;
}   

template <typename T, uint SZ>
CM_API extern vector<T, SZ>
cm_quot(const T &src0, const stream<T, SZ> &src1,  
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0 / src1.get(i);
    }
      
    return retv;
}   

//FIXME, scalar handling required
template <typename T>
CM_API extern T
cm_quot(const T &src0, const T &src1,  
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    T ret;

    ret = src0 / src1;
      
    return ret;
}   

//CM MOD
template <typename T, uint SZ>
CM_API extern vector<T, SZ>
cm_mod(const stream<T, SZ> &src0, const stream<T, SZ> &src1, 
        const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) % src1.get(i);
    }
      
    return retv;
}   

template <typename T, uint SZ>
CM_API extern vector<T, SZ>
cm_mod(const stream<T, SZ> &src0, const T &src1, 
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) % src1;
    }
      
    return retv;
}   

template <typename T, uint SZ>
CM_API extern vector<T, SZ>
cm_mod(const T &src0, const stream<T, SZ> &src1,  
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0 % src1.get(i);
    }
      
    return retv;
}   

//FIXME scalar handling for SIMDCF required
template <typename T>
CM_API extern T
cm_mod(const T &src0, const T &src1,  
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    T ret;

    ret = src0 % src1;
      
    return ret;
}   

//CM DIV
template <typename T, uint R, uint C>
CM_API extern vector<T, R * C>
cm_div(matrix<T, R, C> &rmd, const stream<T, R * C> &src0, 
       const stream<T, R * C> &src1, const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1.get(i);
        rmd(i / C, i % C) = src0.get(i) % src1.get(i);
    }
      
    return retv;
}   

template <typename T, uint R, uint C>
CM_API extern vector<T, R * C>
cm_div(matrix_ref<T, R, C> rmd, const stream<T, R * C> &src0, 
       const stream<T, R * C> &src1,  const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1.get(i);
        rmd(i / C, i % C) = src0.get(i) % src1.get(i);
    }
      
    return retv;
}   

template <typename T, uint R, uint C>
CM_API extern vector<T, R * C>
cm_div(matrix<T, R, C> &rmd, const stream<T, R * C> &src0, const T &src1, 
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1;
        rmd(i / C, i % C) = src0.get(i) % src1;
    }
      
    return retv;
}   

template <typename T, uint R, uint C>
CM_API extern vector<T, R * C>
cm_div(matrix_ref<T, R, C> rmd, const stream<T, R * C> &src0, const T &src1, 
       const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1;
        rmd(i / C, i % C) = src0.get(i) % src1;
    }
      
    return retv;
}   

template <typename T, uint R, uint C>
CM_API extern vector<T, R * C>
cm_div(matrix<T, R, C> &rmd, const T &src0, const stream<T, R * C> &src1,  
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0 / src1.get(i);
        rmd(i / C, i % C) = src0 % src1.get(i);
    }
      
    return retv;
}   

template <typename T, uint R, uint C>
CM_API extern vector<T, R * C>
cm_div(matrix_ref<T, R, C> rmd, const T &src0, const stream<T, R * C> &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0 / src1.get(i);
        rmd(i / C, i % C) = src0 % src1.get(i);
    }
      
    return retv;
}   

//FIXME scalar support required
template <typename T>
CM_API extern T
cm_div(T &rmd, const T &src0, const T &src1,  
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    T ret;

    ret = src0 / src1;
    rmd = src0 % src1;
      
    return ret;
}   

//CM IMUL
template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API extern vector<T3, R * C>
cm_imul(matrix<T3, R, C> &rmd, const stream<T1, R * C> &src0, 
       const stream<T2, R * C> &src1,  const uint flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<T3,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                      SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }else{
        for (i = 0; i < R * C; i++) {
            long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }
      
    return retv;
}   

template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API extern vector<T3, R * C>
cm_imul(matrix_ref<T3, R, C> rmd, const stream<T1, R * C> &src0, 
       const stream<T2, R * C> &src1,  const uint flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<T3,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }     
    }else{
        for (i = 0; i < R * C; i++) {
            long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }     
    }
    return retv;
}   

template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API extern vector<T3, R * C>
cm_imul(matrix<T3, R, C> &rmd, const T1 &src0, 
       const stream<T2, R * C> &src1, const typename uint_type<T1, T2>::type flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<int,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0 * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }      
    }else{
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0 * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }   
    }
    return retv;
}   

template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API extern vector<T3, R * C>
cm_imul(matrix_ref<T3, R, C> rmd, const T1 &src0, 
       const stream<T2, R * C> &src1, const typename uint_type<T1, T2>::type  flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<T3,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0 * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }      
    }else{
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0 * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }   
    }
    return retv;
}   

template <typename T1, typename T2, typename T3>
CM_API extern T3
cm_imul(T3 &rmd, const T1 &src0, 
       const T2 &src1, const typename uint_type<T1, T2>::type  flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    T3 retv;

    if( unsignedOpnd ){
        unsigned long long temp;
        temp = (long long)src0 * (long long)src1;
        retv = temp >> 32;
        rmd = temp & 0xFFFFFFFF;
    }else{
        long long temp;
        temp = (long long)src0 * (long long)src1;
        retv = temp >> 32;
        rmd = temp & 0xFFFFFFFF;
    }
    return retv;
}   


template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API extern vector<T3, R * C>
cm_imul(matrix<T3, R, C> &rmd, const stream<T1, R * C> &src0, 
       const T2 &src1, const typename uint_type<T1, T2>::type  flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<T3,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1;
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }      
    }else{
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1;
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }   
    }
    return retv;
}   

template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API extern vector<T3, R * C>
cm_imul(matrix_ref<T3, R, C> rmd, const stream<T1, R * C> &src0, 
       const T2 &src1, const typename uint_type<T1, T2>::type flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    vector<T3,R * C> retv;
    int i;
    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1;
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }      
    }else{
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1;
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }   
    }
    return retv;
}   

//Add
template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_add(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable = 
        check_true<!is_float_dword<T1, T2>::value>::value;

    static const bool conformable1 = 
        check_true<!(dftype<T1>::value || dftype<T2>::value || dftype<RT>::value) || 
                   (dftype<T1>::value && dftype<T2>::value && dftype<RT>::value)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();

    for (i = 0; i < SZ; i++) {
        if(flags | sat1) {
            ret = (typename restype_ex<T1,T2>::type) src0.get(i) + (typename restype_ex<T1,T2>::type) src1.get(i);
        } else {
            ret = src0.get(i) + src1.get(i);
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }
    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_add(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_add<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API extern RT
cm_add(const T1& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;
    
    v1(0) = src0;
    v2(0) = src1;
    retv = cm_add<RT>(v1, v2, flags);
    return retv(0);
}

//Mul
template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_mul(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable = 
        check_true<!is_float_dword<T1, T2>::value>::value;

    static const bool conformable1 = 
        check_true<!((is_dword_type<T1>::value | is_dword_type<T2>::value) &&
        is_fp_type<RT>::value)>::value;

    static const bool conformable2 = 
        check_true<!(dftype<T1>::value || dftype<T2>::value || dftype<RT>::value) || 
                   (dftype<T1>::value && dftype<T2>::value && dftype<RT>::value)>::value;

    int i;
    typename restype_sat<T1, T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if(flags | sat1) {
            ret = ((typename restype_sat<T1, T2>::type) src0.get(i)) * ((typename restype_sat<T1, T2>::type) src1.get(i));
        } else {
            ret = src0.get(i) * src1.get(i);
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_mul(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_mul<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API extern RT
cm_mul(const T1& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;
    
    retv = cm_mul<RT>(v1, v2, flags);

    return retv(0);
}

//*************** General Purpose instructions with SAT *************

/* Average */
template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_avg(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
   static const bool conformable1 = inttype<T1>::value;
   static const bool conformable2 = inttype<T2>::value;
   static const bool conformable3 = inttype<RT>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = (src0.get(i) + src1.get(i) + 1) >> 1;
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_avg(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v1(i) = src0.get(i);
    }

    retv = cm_avg<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API extern RT
cm_avg(const T1& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;
    
    retv = cm_avg<RT>(v1, v2, flags);

    return retv(0);
}

//Additional explanation from CG is needed
//Dot products
template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_dp2(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1, 
       const uint flags = 0)
{
//    static const bool conformable1 = fptype<T1>::value;
//    static const bool conformable2 = fptype<T2>::value;
//    static const bool conformable2 = fptype<T2>::value;
    static const bool conformable4 = check_true<!(SZ%4)>::value;
#if 0
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value && is_inttype<T2>::value)>::value;
    static const bool conformable6 = 
        check_true<!is_float_dword<T1, T2>::value>::value;
#else
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value || is_inttype<T2>::value)>::value;
#endif

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i += 4) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) * src1.get(i) + src0.get(i + 1) * src1.get(i + 1);
        retv(i) = retv(i + 1) = retv(i + 2) = retv(i + 3) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_dp2(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_dp2<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_dp3(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable4 = check_true<!(SZ%4)>::value;
#if 0
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value && is_inttype<T2>::value)>::value;
    static const bool conformable6 = 
        check_true<!is_float_dword<T1, T2>::value>::value;
#else
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value || is_inttype<T2>::value)>::value;
#endif

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i += 4) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) * src1.get(i) + src0.get(i + 1) * src1.get(i + 1) + src0.get(i + 2) * src1.get(i + 2);
        retv(i) = retv(i + 1) = retv(i + 2) = retv(i + 3) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_dp3(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_dp3<RT>(v1, v2, flags);

    return retv;
}


template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_dp4(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable4 = check_true<!(SZ%4)>::value;
#if 0
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value && is_inttype<T2>::value)>::value;
    static const bool conformable6 = 
        check_true<!is_float_dword<T1, T2>::value>::value;
#else
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value || is_inttype<T2>::value)>::value;
#endif

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i += 4) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) * src1.get(i) + src0.get(i + 1) * src1.get(i + 1) + src0.get(i + 2) * src1.get(i + 2) + src0.get(i + 3) * src1.get(i + 3);
        retv(i) = retv(i + 1) = retv(i + 2) = retv(i + 3) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_dp4(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_dp4<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_dph(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable4 = check_true<!(SZ%4)>::value;
#if 0
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value && is_inttype<T2>::value)>::value;
    static const bool conformable6 = 
        check_true<!is_float_dword<T1, T2>::value>::value;
#else
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value || is_inttype<T2>::value)>::value;
#endif

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i += 4) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) * src1.get(i) +
              src0.get(i + 1) * src1.get(i + 1) +
              src0.get(i + 2) * src1.get(i + 2) +
              1.0 * src1.get(i + 3);

        retv(i) = retv(i + 1) = retv(i + 2) = retv(i + 3) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}


template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_dph(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_dph<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_dph(const T1& src0, const stream<T2,SZ>& src1,  
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T1, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v2(i) = src1.get(i);
    }

    retv = cm_dph<RT>(v1, v2, flags);

    return retv;
}

//Fraction
template <typename T, uint SZ>
CM_API extern vector<T, SZ>
cm_frc(const stream<T, SZ>& src0,  const uint flags = 0)
{
    static const bool conformable1 = fptype<T>::value;

    int i;    
    float ret;
    vector<float, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) - floor(src0.get(i));
        retv(i) = CmEmulSys::satur<float>::saturate(ret, flags);
    }
        
    return retv;
}

template <typename T>
CM_API extern T
cm_frc(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;

    vector<float, 1> v(src0);
    v = cm_frc(v, flags);
    return v(0);
}

//Leading zero detection
template <typename T1, uint SZ>
CM_API extern vector<uint,SZ>
cm_lzd(const stream<T1,SZ>& src0, const uint flags = 0)
{
    static const bool conformable1 = uinttype<T1>::value; 

    int i;
    uint ret;
    vector<uint, SZ> retv;
    
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i); 
        uint cnt = 0;
        while ( (ret & 1u<<31u) == 0 && cnt != 32) {
            cnt ++;
            ret = ret << 1;
        }
        retv(i) = cnt;
    }

    return retv;
}

template <typename T1>
CM_API extern uint
cm_lzd(const T1& src0, const typename uint_type<T1, T1>::type flags = 0)
{
//    static const bool conformable1 = uinttype<T1>::value; 
    vector<T1, 1> v(src0);
    vector<uint, 1> retv;

    retv = cm_lzd(v, flags);

    return retv(0);
}


//Round Down
template <typename RT, uint SZ>
CM_API extern vector<RT, SZ>
cm_rndd(const stream<float,SZ>& src0, const uint flags = 0)
{
    
    int i;
    float ret;
    vector<RT, SZ> retv;
    
    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = floor(src0.get(i));
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT>
CM_API extern RT
cm_rndd(const float& src0, const uint flags = 0)
{
    
    vector<float, 1> v(src0);
    vector<RT, 1> retv;
    
    retv = cm_rndd<RT>(v, flags);

    return retv(0);
}

//Round up
template <typename RT, uint SZ>
CM_API extern vector<RT, SZ>
cm_rndu(const stream<float,SZ>& src0, const uint flags = 0)
{

    int i;
    float ret;
    vector<RT, SZ> retv;
    int increment;
    
    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) - floor(src0.get(i)) > 0.0f) {
            increment = 1;
        } else {
            increment = 0;
        }

        ret = floor(src0.get(i)) + increment;
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT>
CM_API extern RT
cm_rndu(const float& src0, const uint flags = 0)
{
    
    vector<float, 1> v(src0);
    vector<RT, 1> retv;
    
    retv = cm_rndu<RT>(v, flags);

    return retv(0);
}


//Round even
template <typename RT, uint SZ>
CM_API extern vector<RT,SZ>
cm_rnde(const stream<float,SZ>& src0, const uint flags = 0)
{
    int i;
    float ret;
    vector<RT, SZ> retv;
    int increment;

    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) - floor(src0.get(i)) > 0.5f) {
            increment = 1;
        } else if (src0.get(i) - floor(src0.get(i)) < 0.5f) {
            increment = 0;
        } else {
            increment = (int(floor(src0.get(i))) % 2 == 1);
        }

        ret = floor(src0.get(i)) + increment;
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}


template <typename RT>
CM_API extern RT
cm_rnde(const float& src0, const uint flags = 0)
{
    
    vector<float, 1> v(src0);
    vector<RT, 1> retv;
    
    retv = cm_rnde<RT>(v, flags);

    return retv(0);
}


//Round zero
template <typename RT, uint SZ>
CM_API extern vector<RT, SZ>
cm_rndz(const stream<float,SZ>& src0, const uint flags = 0)
{
    int i;
    float ret;
    vector<RT, SZ> retv;
    int increment;

    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (fabs(src0.get(i)) < fabs(floor(src0.get(i)))) {
            increment = 1;
        } else {
            increment = 0;
        }
        ret = floor(src0.get(i)) + increment;
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT>
CM_API extern RT
cm_rndz(const float& src0, const uint flags = 0)
{
    
    vector<float, 1> v(src0);
    vector<RT, 1> retv;
    
    retv = cm_rndz<RT>(v, flags);

    return retv(0);
}

//Sum of Absolute Difference 2
template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_sad2(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
        const uint flags = 0)
{
    static const bool conformable1 = bytetype<T1>::value;
    static const bool conformable2 = bytetype<T2>::value;
    static const bool conformable3 = wordtype<RT>::value;
    static const bool conformable4 = check_true<!(SZ&1)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i+=2) {
         SIMDCF_ELEMENT_SKIP(i);
        ret = CmEmulSys::abs(src0.get(i) - src1.get(i)) 
                + CmEmulSys::abs(src0.get(i+1) - src1.get(i+1));

        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
   }

   return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_sad2(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_sad2<RT>(v1, v2, flags);

    return retv;
}

//Logical shift left
template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_shl(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
        const uint flags = 0)
{
    static const bool conformable1 = inttype<T1>::value;
    static const bool conformable2 = unsignedtype<T2>::value;
    static const bool conformable3 = inttype<RT>::value;

    int i;
    typename maxtype<T1>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) << src1.get(i);
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

   return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_shl(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_shl<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_shl(const T1& src0, const stream<T2,SZ>& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v2(i) = src1.get(i);
    }

    retv = cm_shl<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API extern RT
cm_shl(const T1& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_shl<RT>(v1, v2, flags);

    return retv(0);
}


//Logical shift right
template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_shr(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
        const uint flags = 0)
{
    static const bool conformable1 = unsignedtype<T1>::value;
    static const bool conformable2 = unsignedtype<T2>::value;
    static const bool conformable3 = unsignedtype<RT>::value;

    int i;
    typename maxtype<T1>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) >> src1.get(i);
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
   }

   return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_shr(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_shr<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_shr(const T1& src0, const stream<T2,SZ>& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v2(i) = src1.get(i);
    }

    retv = cm_shr<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API extern RT
cm_shr(const T1& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_shr<RT>(v1, v2, flags);

    return retv(0);
}


//Ariphmetic shift right
template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_asr(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
        const uint flags = 0)
{
    static const bool conformable1 = inttype<T1>::value;
    static const bool conformable2 = unsignedtype<T2>::value;
    static const bool conformable3 = inttype<RT>::value;

    int i;
    typename maxtype<T1>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) >> src1.get(i);
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
   }

   return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_asr(const stream<T1,SZ>& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_asr<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_asr(const T1& src0, const stream<T2,SZ>& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v2(i) = src1.get(i);
    }

    retv = cm_asr<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API extern RT
cm_asr(const T1& src0, const T2& src1, 
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_asr<RT>(v1, v2, flags);

    return retv(0);
}


template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_line(const stream<T1, 4>& src0, const stream<T2,SZ>& src1, 
        const typename uint_type<T1, T2>::type flags = 0)
{
    int i;
    vector<RT, SZ> retv;
    typename restype_ex<T1,T2>::type ret;

    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
         ret = src0.get(0) * src1.get(i) + src0.get(3);
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
     }

    return retv;    
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API extern vector<RT, SZ>
cm_line(const T1& P, const T1& Q, const stream<T2,SZ>& src1, 
        const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 4> src0;
    vector<RT, SZ> retv;
    src0(0) = P;
    src0(3) = Q;

    retv = cm_line<RT>(src0, src1, flags); 

    return retv;
}

//*********************Reduction inrinsics***********************
template <typename RT, typename T, uint SZ>
CM_API extern RT
cm_sum(const stream<T,SZ>& src1, const uint flags = 0)
{
    int i;
    RT retv = 0;

    if (std::numeric_limits<RT>::is_integer) {
        for (i = 0; i < SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            retv = cm_add<RT>(retv, src1.get(i), flags);
        }
    } 
    else {
        for (i = 0; i < SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            retv = cm_add<RT>(retv, src1.get(i));
        }
        retv = CmEmulSys::satur<RT>::saturate(retv, flags);
    }

    return retv;
}

//*********************Reduction inrinsics***********************
template <typename RT, typename T, uint SZ>
CM_API extern RT
cm_reduced_max(const stream<T,SZ>& src1, const uint flags = 0)
{
    int i;
    RT retv = 0;
    T tmp = 0;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        tmp = src1.get(i);
    }

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src1.get(i) > tmp) {
            tmp = src1.get(i);
        }
    }

    retv = CmEmulSys::satur<RT>::saturate(tmp, flags);
    return retv;
}

//*********************Reduction inrinsics***********************
template <typename RT, typename T, uint SZ>
CM_API extern RT
cm_reduced_min(const stream<T,SZ>& src1, const uint flags = 0)
{
    int i;
    RT retv = 0;
    T tmp = 0;
      
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        tmp = src1.get(i);
    }

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src1.get(i) < tmp) {
            tmp = src1.get(i);
        }        
    }

    retv = CmEmulSys::satur<RT>::saturate(tmp, flags);
    return retv;
}

template <typename RT, typename T, uint SZ>
CM_API extern RT
cm_prod(const stream<T,SZ>& src1, const uint flags = 0)
{
    int i;
    RT retv = 1;

    if (std::numeric_limits<RT>::is_integer) {
        int tmp= 1;
        for (i = 0; i < SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            tmp *= src1.get(i);
        }
        retv = CmEmulSys::satur<RT>::saturate(tmp, flags);
    } 
    else {
        float tmp= 1.0;
        for (i = 0; i < SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            tmp *= src1.get(i);
        }
        retv = CmEmulSys::satur<RT>::saturate(tmp, flags);
    }
    return retv;
}

//***************** Extended Math Unit Intrinsics*****************
// dst = 1.0/src
template <uint SZ>
CM_API extern vector<float, SZ>
cm_inv(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(1. / src0.get(i), flags);
    }

    return retv;
}   
template <typename T>
CM_API extern float
cm_inv(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_inv(v, flags);
    return v(0);   
}

//dst = log_2(scr)
template <uint SZ>
CM_API extern vector<float, SZ>
cm_log(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(logf(src0.get(i)) / logf(2.), flags);
    }
    return retv;
}   
template <typename T>
CM_API extern float
cm_log(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_log(v, flags);
    return v(0);   
}


//dst = 2**src
template <uint SZ>
CM_API extern vector<float, SZ>
cm_exp(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(powf(2., src0.get(i)), flags);
    }
    return retv;
}   
template <typename T>
CM_API extern float
cm_exp(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_exp(v, flags);
    return v(0);   
}

//dst = sqrt(src)
template <uint SZ>
CM_API extern vector<float, SZ>
cm_sqrt(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(sqrt(src0.get(i)), flags);
    }
    return retv;
}   
template <typename T>
CM_API extern float
cm_sqrt(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_sqrt(v, flags);
    return v(0);   
}

//dst = 1.0/sqrt(src)
template <uint SZ>
CM_API extern vector<float, SZ>
cm_rsqrt(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(1. / sqrt(src0.get(i)), flags);
    }
    return retv;
}   
template <typename T>
CM_API extern float
cm_rsqrt(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_rsqrt(v, flags);
    return v(0);   
}
#if defined(CM_GEN4) || defined(CM_GEN5)
template <uint SZ>
CM_API extern vector<float, SZ>
cm_pow(const stream<float, SZ>& src0, const stream<float, SZ>& src1, const uint flags = 0) 
{
    std::cerr << "\ncm_pow intrinsic does not support stream power for ILK" << std::endl;
}
template <uint SZ>
CM_API extern vector<float, SZ>
cm_pow( const float& src0, const stream<float, SZ>& src1, const uint flags = 0) 
{
    std::cerr << "\ncm_pow intrinsic does not support stream power for ILK" << std::endl;
}
//dst = pow(src0,src1), src1 is always scalar
template <uint SZ>
CM_API extern vector<float, SZ>
cm_pow(const stream<float, SZ>& src0, const float& src1, const uint flags = 0) 
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        retv(i) = 
            CmEmulSys::satur<float>::saturate(powf(fabs(src0.get(i)), src1), flags);
    }
    return retv;
}
template <typename T>
CM_API extern float
cm_pow(const T& src0, const T& src1,
       const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_pow(v, src1, flags);
    return v(0);   
}
#else
template <uint SZ>
CM_API extern vector<float, SZ>
cm_pow(const stream<float, SZ>& src0, const stream<float, SZ>& src1, const uint flags = 0) 
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(powf(fabs(src0.get(i)), src1.get(i)), flags);
    }
    return retv;
}
template <uint SZ>
CM_API extern vector<float, SZ>
cm_pow( const float& src0, const stream<float, SZ>& src1, const uint flags = 0) 
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(powf(fabs(src0), src1.get(i)), flags);
    }
    return retv;
}
template <uint SZ>
CM_API extern vector<float, SZ>
cm_pow(const stream<float, SZ>& src0, const float& src1, const uint flags = 0) 
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(powf(fabs(src0.get(i)), src1), flags);
    }
    return retv;
}
template <typename T>
CM_API extern float
cm_pow(const T& src0, const T& src1,
       const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    vector<float, 1> v1(src1);
    v = cm_pow(v, v1, flags);
    return v(0);   
}
#endif
//dst = sin(src)
template <uint SZ>
CM_API extern vector<float, SZ>
cm_sin(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(sin(src0.get(i)), flags);
    }
    return retv;
}   
template <typename T>
CM_API extern float
cm_sin(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_sin(v, flags);
    return v(0);   
}

//dst = cos(src)
template <uint SZ>
CM_API extern vector<float, SZ>
cm_cos(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(cos(src0.get(i)), flags);
    }
    return retv;
}   
template <typename T>
CM_API extern float
cm_cos(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_cos(v, flags);
    return v(0);   
}


//sincos
template <uint SZ>
CM_API extern vector<float, SZ>
cm_sincos(vector<float, SZ> &cosv, const stream<float, SZ>& src0, 
          const uint flags = 0)
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(sin(src0.get(i)), flags);
        cosv(i) = CmEmulSys::satur<float>::saturate(cos(src0.get(i)), flags);
    }
    return retv;
}   

template <typename T>
CM_API extern float
cm_sincos(T& cosr, const T& src0, 
          const typename uint_type<T, T>::type flags = 0)
{
    vector<float, 1> v(src0);
    vector<float, 1> v1;
    v = cm_sincos(v1,v, flags);
    cosr = v1(0);
    return v(0);
}

template <uint SZ>
CM_API extern vector<float, SZ>
cm_acos(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    
    for (int i = 0; i < SZ; i++) {        
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(acos(src0.get(i)), flags);
    }
    return retv;
}   
template <typename T>
CM_API extern float
cm_acos(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_acos(v, flags);
    return v(0);   
}

template <uint SZ>
CM_API extern vector<float, SZ>
cm_asin(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    for (int i = 0; i < SZ; i++) {        
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(asin(src0.get(i)), flags);
    }
    return retv;
}   
template <typename T>
CM_API extern float
cm_asin(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_asin(v, flags);
    return v(0);   
}

template <uint SZ>
CM_API extern vector<float, SZ>
cm_atan(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    for (int i = 0; i < SZ; i++) {        
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(atan(src0.get(i)), flags);
    }
    return retv;
}   
template <typename T>
CM_API extern float
cm_atan(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_atan(v, flags);
    return v(0);   
}

template <typename T, uint R, uint C>
CM_API extern void
cm_input(matrix<T,R,C> &in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint R, uint C>
CM_API extern void
cm_input(matrix_ref<T,R,C> in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint S>
CM_API extern void
cm_input(vector<T, S> &in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint S>
CM_API extern void
cm_input(vector_ref<T, S> in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template<typename T>
CM_API extern void
cm_input(T& in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint R, uint C>
CM_API extern void 
cm_output(const matrix<T,R,C> &out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint R, uint C>
CM_API extern void 
cm_output(const matrix_ref<T,R,C> out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint S>
CM_API extern void 
cm_output(const vector<T, S> &out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint S>
CM_API extern void 
cm_output(const vector_ref<T, S> out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template<typename T>
CM_API extern void
cm_output(const T& out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint SZ>
CM_API extern uint
__cm_pack_mask(const vector_ref<T,SZ>& src0) {
    // We don't check the arguments here as this function is only invoked by wrapper code (which
    // does the checks already)
    // The way icl works, it will try and compile code inside if statements that is never reached
    // and produce compile time errors otherwise
    uint retv = 0;   
    for (int i = 0; i < SZ; i++) {
        if (src0.get(i) & 0x1) {
            retv |= 0x1 << i;
        } 
    }

    return retv;
}

template <typename T, uint R, uint C>
CM_API extern uint
__cm_pack_mask(const matrix_ref<T,R,C>& src0) {
    // We don't check the arguments here as this function is only invoked by wrapper code (which
    // does the checks already)
    // The way icl works, it will try and compile code inside if statements that is never reached
    // and produce compile time errors otherwise
    uint retv = 0;   
    for (int i = 0; i < R*C; i++) {
        if (src0(i / C, i % C) & 0x1) {
            retv |= 0x1 << i;
        } 
    }

    return retv;
}

template <typename T, uint SZ>
_GENX_ inline uint
cm_pack_mask(const vector_ref<T,SZ>& src0) {
    static const bool conformable1 = is_ushort_type<T>::value;
    static const bool conformable2 = check_true< SZ == 8 || SZ == 16 || SZ == 32>::value;
    CM_STATIC_ERROR(conformable1, "only ushort element type is supported");
    CM_STATIC_WARNING(conformable2, "argument isn't 8, 16 or 32 elements - temporary will be introduced");

    // We rely on the compiler doing the right thing with the SZ compile time constant to reduce
    // this code to the minimum required
    if ( ! conformable2 ) {
        vector<T, SZ < 8 ? 8 : ( SZ < 16 ? 16 : 32 ) > _Src0 = 0;
        _Src0.template select<SZ,1>() = src0;
        return __cm_pack_mask(_Src0.select_all());
    }

    return __cm_pack_mask(src0);
}
 
template <typename T, uint SZ>
_GENX_ inline uint
cm_pack_mask(const vector<T,SZ>& src0) {
    return cm_pack_mask(src0.select_all());
}
 
template <typename T, uint R, uint C>
_GENX_ inline uint
cm_pack_mask(const matrix_ref<T,R,C>& src0) {
    static const bool conformable1 = is_ushort_type<T>::value;
    static const bool conformable2 = check_true< R*C == 8 || R*C == 16 || R*C == 32>::value;
    CM_STATIC_ERROR(conformable1, "only ushort element type is supported");
    CM_STATIC_WARNING(conformable2, "argument isn't 8, 16 or 32 elements - temporary will be introduced");

    if ( ! conformable2 ) {
        vector<T, (R*C) < 8 ? 8 : ( (R*C) < 16 ? 16 : 32 ) > _Src0 = 0;
        _Src0.template select<R*C,1>() = src0.template format<T>();
        return __cm_pack_mask(_Src0.select_all());
    }

    return __cm_pack_mask(src0);
}

template <typename T, uint R, uint C>
_GENX_ inline uint
cm_pack_mask(const matrix<T,R,C>& src0) {
    return cm_pack_mask(src0.select_all());
}

template <typename RT, uint SZ>
CM_API extern vector<RT,SZ> 
cm_unpack_mask(const uint& src0) {
    static const bool conformable1 = is_ushort_type<RT>::value;
    static const bool conformable2 = check_true< SZ == 8 || SZ == 16 || SZ == 32>::value;
    CM_STATIC_ERROR(conformable1, "only ushort element type is supported");
    CM_STATIC_WARNING(conformable2, "argument isn't 8, 16 or 32 elements - temporary will be introduced");

    vector<RT,SZ> retv;
    for (int i = 0; i < SZ; i++) {
        if ((src0 >> i) & 0x1) {
            retv(i) = 1;
        } else {
            retv(i) = 0;
        }
    }

    return retv;
}

// Count component-wise the total bits set in source operand
template <typename T1, uint SZ>
CM_API extern vector<uint,SZ>
cm_cbit(const stream<T1,SZ>& src0, const uint flags = 0)
{
    static const bool conformable1 = inttype<T1>::value; 

    int i;
    uint ret;
    vector<uint, SZ> retv;
    
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i); 
        uint cnt = 0;
        for(int j = 0; j < sizeof(T1)*8; j++) {
            if((ret & 1u) == 1) {
                cnt ++;
            }
            ret = ret >> 1;
        }
        retv(i) = cnt;
    }

    return retv;
}

template <typename T1>
CM_API extern uint
cm_cbit(const T1& src0, const typename uint_type<T1, T1>::type flags = 0)
{
    static const bool conformable1 = inttype<T1>::value; 

    vector<T1, 1> v(src0);
    vector<uint, 1> retv;

    retv = cm_cbit(v, flags);

    return retv(0);
}

// Find component-wise the first bit from LSB side
template <typename T1, uint SZ>
CM_API extern vector<uint,SZ>
cm_fbl(const stream<T1,SZ>& src0, const uint flags = 0)
{
    static const bool conformable1 = uinttype<T1>::value; 

    int i;
    uint ret;
    vector<uint, SZ> retv;
    
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i); 
        uint cnt = 0;
        while ((ret & 1u) == 0 && cnt != 32) {
            cnt ++;
            ret = ret >> 1;
        }
        if(src0.get(i) == 0x0) {
            retv(i) = 0xFFFFFFFF;
        } else {
            retv(i) = cnt;
        }
    }

    return retv;
}

template <typename T1>
CM_API extern uint
cm_fbl(const T1& src0, const typename uint_type<T1, T1>::type flags = 0)
{
    static const bool conformable1 = uinttype<T1>::value; 
    vector<T1, 1> v(src0);
    vector<uint, 1> retv;

    retv = cm_fbl(v, flags);

    return retv(0);
}

// Find component-wise the first bit from MSB side
template <typename T1, uint SZ>
CM_API extern vector<T1,SZ>
cm_fbh(const stream<T1,SZ>& src0, const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T1>::value; 

    bool uintOpnd = check_true<(unsignedtype <T1>::value)>::value;

    int i, cval;
    T1 ret;
    vector<T1, SZ> retv;
    
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i); 
        uint cnt = 0;
        if (uintOpnd) {
            while ((ret & (1u << 31u)) == 0 && cnt != 32) {
                cnt ++;
                ret = ret << 1;
            }
            if (src0.get(i) == 0x00000000) {
                retv(i) = 0xFFFFFFFF;
            } else {
                retv(i) = cnt;
            }
        }
        else {
            if (((ret >> 31u) & 1u) == 1) {
                cval = 1;
            }
            else {
                cval = 0;
            }
            while (((ret >> 31u) & 1u) == cval && cnt != 32) {
                cnt ++;
                ret = ret << 1;
            }

            if ((src0.get(i) == 0xFFFFFFFF) || 
                 (src0.get(i) == 0x00000000)) {
                retv(i) = 0xFFFFFFFF;
            } else {
                retv(i) = cnt;
            }
        }
    }

    return retv;
}

template <typename T1>
CM_API extern T1
cm_fbh(const T1& src0, const typename uint_type<T1, T1>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T1>::value; 
    vector<T1, 1> v(src0);
    vector<T1, 1> retv;

    retv = cm_fbh(v, flags);

    return retv(0);
}

#if !defined __CM && defined _WIN32
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#endif

template <typename T>
CM_API extern vector<T, 4>
cm_rdtsc()
{
    static const bool conformable1 = uinttype<T>::value;
    vector<T, 4> dst = 0;
    #if !defined __CM && defined _WIN32
    unsigned __int64  ts      = __rdtsc();
    unsigned __int32* ptr     = (unsigned __int32*)&ts;
    unsigned __int32  ts_low  = *ptr;
    unsigned __int32  ts_high = *(ptr + 1);

    dst[0] = (T)ts_low;
    dst[1] = (T)ts_high;
    #endif
    return dst;
}

//------------------------------------------------------------------------------
//             Functions for Groups and SLM Support
//------------------------------------------------------------------------------

#ifndef CM_EMU
#define CMRT_VAR_API
#elif defined(LIBCM_TEST_EXPORTS)
    #ifndef __GNUC__
    #define CMRT_VAR_API __declspec(dllexport)
    #else
    #define CMRT_VAR_API __attribute__((visibility("default")))
    #endif
#else
    #ifndef __GNUC__
    #define CMRT_VAR_API __declspec(dllimport) 
    #else
    #define CMRT_VAR_API __attribute__((visibility("default")))
    #endif
#endif /* CM_EMU */

CMRT_VAR_API extern char *__cm_emu_slm;
CMRT_VAR_API extern uint __cm_slm_size;

#define MAX_SLM_ID 100
CMRT_VAR_API extern uint slmID;
CMRT_VAR_API extern char *__cm_emu_slm_ptr[MAX_SLM_ID];

#define __CM_SLM_CHUNK_SIZE 4096
#define __CM_SLM_MAX_SIZE   (16 * __CM_SLM_CHUNK_SIZE)

// Functions called by runtime in Emulation mode
//------------------------------------------------------------------------------

// Set Local Thread IDs in current group 
CM_API extern void __cm_set_local_id (uint dim, uint id);

// Set Number of threads per group
CM_API extern void __cm_set_local_size(uint dim, uint size);

// Set Group IDs
CM_API extern void __cm_set_group_id(uint dim, uint id);

// Set Number of groups
CM_API extern void __cm_set_group_count(uint dim, uint count);

// EMU SLM buffer
CM_API extern void __cm_init_emu_slm(void);
CM_API extern void __cm_free_emu_slm(void);

// Intrinsic Functions called from User functions:
//------------------------------------------------------------------------------


// Cm Size
CM_API extern void cm_slm_size (uint size);

// Local Thread IDs in current group 
CM_API extern uint cm_local_id (uint dim);

// Number of threads per group
CM_API extern uint cm_local_size(uint dim);

// Group IDs
CM_API extern uint cm_group_id(uint dim);

// Number of groups
CM_API extern uint cm_group_count(uint dim);

// Set the fp rounding mode
CM_API extern void cm_fsetround(CmRoundingMode val);

// Get the fp rounding mode
CM_API extern CmRoundingMode cm_fgetround(void);

// Set the fp mode (ALT/IEEE)
CM_API extern void cm_fsetmode(CmFPMode val);

// Get the fp mode (ALT/IEEE)
CM_API extern CmFPMode cm_fgetmode(void);

// Pause thread
CM_API extern void cm_pause(unsigned short length);

// Insert value into src bitfield of defined width at defined offset
// The instruction restricts the src and dst types to D or UD types
template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API extern vector<RT, SZ>
cm_bf_insert(const stream<T1,SZ>& width, const stream<T2, SZ>& offset,
             const stream<T3, SZ>& val, const stream<T4,SZ>& src,
             const uint flags = 0, const uint flags2 = 0) 
{
    static const bool conformable1 = dwordtype<T1>::value;
    static const bool conformable2 = dwordtype<T2>::value;
    static const bool conformable3 = dwordtype<T3>::value;
    static const bool conformable4 = dwordtype<T4>::value;
    static const bool conformable5 = dwordtype<RT>::value;
    static const bool is_unsigned  = unsignedtype<RT>::value;

    int i;
    typename maxtype<T4>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        const uint mask = ((1 << width.get(i)) - 1) << offset.get(i);
        const uint imask = ~mask;
        ret = (src.get(i) & imask) | ((val.get(i) << offset.get(i) & mask));
        // Sign extend if signed type
        if (!is_unsigned) {
            const int m = 1U << (width.get(i) - 1);
            ret = (ret ^ m) - m;
        }
        retv(i) = ret; 
    }

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API extern vector<RT, SZ>
cm_bf_insert(const stream<T1,SZ>& width, const stream<T2, SZ>& offset,
             const stream<T3, SZ>& val, const T4& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0) 
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2;
    vector<T3, SZ> v3;
    vector<T4, SZ> v4(src);
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v1(i) = width.get(i);
        v2(i) = offset.get(i);
        v3(i) = val.get(i);
    }
    
    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API extern vector<RT, SZ>
cm_bf_insert(const stream<T1,SZ>& width, const stream<T2, SZ>& offset,
             const T3& val, const stream<T4,SZ>& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0) 
{
    vector<T1, SZ> v1;
    vector<T3, SZ> v2;
    vector<T2, SZ> v3(val);
    vector<T4, SZ> v4;
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v1(i) = width.get(i);
        v2(i) = offset.get(i);
        v4(i) = src.get(i);
    }
    
    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API extern vector<RT, SZ>
cm_bf_insert(const T1& width, const T2& offset,
             const stream<T3, SZ>& val, const stream<T4,SZ>& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0) 
{
    vector<T1, SZ> v1(width);
    vector<T2, SZ> v2(offset);
    vector<T3, SZ> v3;
    vector<T4, SZ> v4;
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v3(i) = val.get(i);
        v4(i) = src.get(i);
    }
    
    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API extern vector<RT, SZ>
cm_bf_insert(const T1& width, const T2& offset,
             const stream<T3, SZ>& val, const T4& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0) 
{
    vector<T1, SZ> v1(width);
    vector<T2, SZ> v2(offset);
    vector<T3, SZ> v3;
    vector<T4, SZ> v4(src);
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v3(i) = val.get(i);
    }
    
    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API extern vector<RT, SZ>
cm_bf_insert(const T1& width, const T2& offset,
             const T3& val, const stream<T3,SZ>& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0) 
{
    vector<T1, SZ> v1(width);
    vector<T2, SZ> v2(offset);
    vector<T3, SZ> v3(val);
    vector<T4, SZ> v4;
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v4(i) = src.get(i);
    }
    
    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4>
CM_API extern RT
cm_bf_insert(const T1& width, const T2& offset,
             const T3& val, const T4& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0) 
{
    vector<T3, 1> v1(width);
    vector<T4, 1> v2(offset);
    vector<T2, 1> v3(val);
    vector<T1, 1> v4(src);
    vector<RT, 1> retv;

    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv(0);
}


// Extract value from src bitfield of defined width at defined offset
template<typename RT, typename T1, typename T2, typename T3, uint SZ>
CM_API extern vector<RT, SZ>
cm_bf_extract(const stream<T1,SZ>& width, const stream<T2,SZ>& offset,
              const stream<T3,SZ>& src, 
              const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T1>::value;
    static const bool conformable2 = dwordtype<T2>::value;
    static const bool conformable3 = dwordtype<T3>::value;
    static const bool conformable5 = dwordtype<RT>::value;

    int i;
    typename maxtype<T3>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        const uint mask = ((1 << width.get(i)) - 1) << offset.get(i);
        ret = (src.get(i) & mask) >> offset.get(i);
        retv(i) = ret;
    }

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, uint SZ>
CM_API extern vector<RT, SZ>
cm_bf_extract(const T1& width, const T2& offset,
              const stream<T3,SZ>& src, 
              const typename uint_type<T3, T1>::type flags = 0)
{
    vector<T1, SZ> v1(width);
    vector<T2, SZ> v2(offset);
    vector<T3, SZ> v3;
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v3(i) = src.get(i);
    }

    retv = cm_bf_extract<RT>(v1, v2, v3, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3>
CM_API extern RT
cm_bf_extract(const T1& width, const T2& offset,
              const T3& src, 
              const typename uint_type<T3, T1>::type flags = 0)
{
    vector<T1, 1> v1(width);
    vector<T2, 1> v2(offset);
    vector<T3, 1> v3(src);
    vector<RT, 1> retv;

    retv = cm_bf_extract<RT>(v1, v2, v3, flags);

    return retv;
}

// Reverse src bitfield
template<typename RT, typename T1, uint SZ>
CM_API extern vector<RT, SZ>
cm_bf_reverse(const stream<T1,SZ>& src, 
              const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T1>::value;
    static const bool conformable5 = dwordtype<RT>::value;

    int i,j;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        RT input = src.get(i);
        RT output = 0;
        for (j = 0; j < sizeof(RT) * 8; j++) {
            output |= input & 0x1;
            
            // Don't shift if this was the last one
            if ((j+1) < (sizeof(RT) * 8)) {
                output <<= 1;
                input >>= 1;
            }
        }
        retv(i) = output;
    }

    return retv;
}

// Convert from short to float32 (maps to hardware instruction)
// where the short is in the form of an IEEE float16 
// We use short to represent as we don't have a native half type
template<typename T1, uint SZ>
CM_API extern vector<float, SZ>
cm_f16tof32(const stream<T1, SZ>& src,
            const uint flags = 0)
{
    static const bool conformable1 = wordtype<T1>::value;

    int i;
    vector<float, SZ> retv;

#ifdef CM_EMU
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        T1 input = src.get(i);
        //float output = Float16Conversion::tof32(input);
        half_float::half val;
        val.setBits(input);
        float output = (float) val;
        retv(i) = output;
    }
#endif

    return retv;
}

// Convert from float32 to short (maps to hardware instruction)
// where the short is in the form of an IEEE float16 
// We use short to represent as we don't have a native half type
template<typename T1, uint SZ>
CM_API extern vector<ushort, SZ>
cm_f32tof16(const stream<T1, SZ>& src,
            const uint flags = 0)
{
    static const bool conformable2 = is_fp_type<T1>::value;

    int i;
    vector<ushort, SZ> retv;

#ifdef CM_EMU
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        T1 input = src.get(i);
        //ushort output = Float16Conversion::tof16(input);
        half_float::half output = half_float::half(input);
        retv(i) = output.getBits();
    }
#endif

    return retv;
}



// Functions implemented as user-level utility functions:
// _GENX_ inline cm_slm_init (uint slmSizePerGroup);
// _GENX_ inline cm_slm_alloc (uint bufferSizeInBytes);
// _GENX_ inline void cm_slm_free (uint bufferSizeInBytes);
// _GENX_ inline void cm_slm_load (uint         slmBuffer,    // SLM buffer
//                                 SurfaceIndex memSurfIndex, // Memory SurfaceIndex
//                                 uint         memOffset,    // Byte-Offset in Memory Surface
//                                 uint         loadSize      // Bytes to be Loaded from Memory
//                                 )

// Cm Barrier
CM_API extern void cm_barrier (void);

// Type-Checking Templates

template <typename T> struct Allowed_Type_Float_Or_Dword {
};
template <> struct Allowed_Type_Float_Or_Dword<int> {
    static const bool value = true;
};
template <> struct Allowed_Type_Float_Or_Dword<uint> {
    static const bool value = true;
};
template <> struct Allowed_Type_Float_Or_Dword<float> {
    static const bool value = true;
};


template <uint N> struct Allowed_Vector_Length_8_Or_16 {
};
template <> struct Allowed_Vector_Length_8_Or_16<8> {
    static const bool value = true;
};
template <> struct Allowed_Vector_Length_8_Or_16<16> {
    static const bool value = true;
};


// Templatized functions need to be defined in the header file:
//--------------------------------------------------------------

// Read from the SLM buffer 'slmBuffer' at the addresses given in 'v_Addr'
// and write it back to the destination vector 'v_Dst'
// Implemented with Byte-Scattered-Read in Gen7+

template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API extern void
cm_slm_read (uint  slmBuffer,           // SLM buffer
             vector<ushort, N> &v_Addr, // Byte-Offsets into SLM Buffer
             vector<T, N>      &v_Dst   // Data vector to be written from SLM
            )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_slm + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i); 
        v_Dst(i) = *( (T *)byteOffset );
    }
}
template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API extern void
cm_slm_read (uint  slmBuffer,              // SLM buffer
             vector_ref<ushort, N> v_Addr, // Byte-Offsets into SLM Buffer
             vector_ref<T, N>      v_Dst   // Data vector to be written from SLM
            )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_slm + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i); 
        v_Dst(i) = *( (T *)byteOffset );
    }
}
template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API extern void
cm_slm_read (uint  slmBuffer,              // SLM buffer
             vector<ushort, N>     &v_Addr,// Byte-Offsets into SLM Buffer
             vector_ref<T, N>      v_Dst   // Data vector to be written from SLM
            )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_slm + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i); 
        v_Dst(i) = *( (T *)byteOffset );
    }
}
template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API extern void
cm_slm_read (uint  slmBuffer,              // SLM buffer
             vector_ref<ushort, N> v_Addr, // Byte-Offsets into SLM Buffer
             vector<T, N>          &v_Dst   // Data vector to be written from SLM
            )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_slm + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i); 
        v_Dst(i) = *( (T *)byteOffset );
    }
}

typedef enum _SLM_ChannelMaskType_
{   SLM_R_ENABLE         = 14,
    SLM_G_ENABLE         = 13,
    SLM_GR_ENABLE        = 12,
    SLM_B_ENABLE         = 11,
    SLM_BR_ENABLE        = 10,
    SLM_BG_ENABLE        = 9,
    SLM_BGR_ENABLE       = 8,
    SLM_A_ENABLE         = 7,
    SLM_AR_ENABLE        = 6,
    SLM_AG_ENABLE        = 5,
    SLM_AGR_ENABLE       = 4,
    SLM_AB_ENABLE        = 3,
    SLM_ABR_ENABLE       = 2,
    SLM_ABG_ENABLE       = 1,
    SLM_ABGR_ENABLE      = 0
} SLM_ChannelMaskType;

template <typename T1, typename T, uint M>
inline extern void
cm_slm_read4_internal (uint slmBuffer, T1 v_Addr, vector_ref<T,M> v_Dst, SLM_ChannelMaskType mask, uint N, uint M1)
{
    static const bool conformable1 = Allowed_Type_Float_Or_Dword<T>::value;
    char *baseOffset, *byteOffset;
    char numColors=0, color[4]={0,0,0,0}, colorNext=0;
    baseOffset = __cm_emu_slm + slmBuffer;
    // mask = mask & 0x1111;
    if (!(mask & 0x1)) {color[0]=1; numColors++;}
    if (!(mask & 0x2)) {color[1]=1; numColors++;}
    if (!(mask & 0x4)) {color[2]=1; numColors++;}
    if (!(mask & 0x8)) {color[3]=1; numColors++;}
    if (numColors == 0) {
        fprintf(stderr, "cm_slm_read4 error: At least one"
                "destination vector has to be read!\n");
        exit(EXIT_FAILURE);
    }
    if (M1 < numColors*N) {
        fprintf(stderr, "cm_slm_read4 error: destination vector"
                "does not have enough space to hold data\n");
        exit(EXIT_FAILURE);
    }
    for (uint j=0; j<4; j++) {
        if (color[j] == 0) continue;
        for (uint i=0; i<N; i++) {
            byteOffset = baseOffset +  sizeof(T) * (v_Addr(i) + j);
            v_Dst(i+colorNext*N) = *( (T *)byteOffset );
        }
        colorNext++;
    }
}

template <typename T, uint N, uint M>
CM_API extern void
cm_slm_read4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector<T,M> &v_Dst, SLM_ChannelMaskType mask)
{
    vector_ref<T,M> dst = v_Dst;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_read4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector_ref<T,M> v_Dst, SLM_ChannelMaskType mask)
{
    vector_ref<T,M> dst = v_Dst;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_read4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector_ref<T,M> v_Dst, SLM_ChannelMaskType mask)
{
    vector_ref<T,M> dst = v_Dst;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_read4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector<T,M> &v_Dst, SLM_ChannelMaskType mask)
{
    vector_ref<T,M> dst = v_Dst;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}
//---------------------------------
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_read4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector<T,M> &v_Dst, int imask)
{
    vector_ref<T,M> dst = v_Dst;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_read4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector_ref<T,M> v_Dst, int imask)
{
    vector_ref<T,M> dst = v_Dst;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_read4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector_ref<T,M> v_Dst, int imask)
{
    vector_ref<T,M> dst = v_Dst;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_read4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector<T,M> &v_Dst, int imask)
{
    vector_ref<T,M> dst = v_Dst;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}

// Write the vector 'v_Src' to the SLM buffer 'slmBuffer' at the
// addresses given in 'v_Addr'
// Implement with Byte-Scattered-Write in Gen7+

template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API extern void                                                               
cm_slm_write (uint  slmBuffer,           // SLM buffer
              vector<ushort, N> &v_Addr, // Byte-Offsets into SLM Buffer
              vector<T, N>      &v_Src   // Data vector to be written to SLM
             )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_slm + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i); 
        *( (T *)byteOffset ) = v_Src(i);
    }
}
template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API extern void                                                               
cm_slm_write (uint  slmBuffer,              // SLM buffer
              vector_ref<ushort, N> v_Addr, // Byte-Offsets into SLM Buffer
              vector_ref<T, N>      v_Src   // Data vector to be written to SLM
             )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_slm + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i); 
        *( (T *)byteOffset ) = v_Src(i);
    }
}
template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API extern void                                                               
cm_slm_write (uint  slmBuffer,               // SLM buffer
              vector<ushort, N>     &v_Addr, // Byte-Offsets into SLM Buffer
              vector_ref<T, N>      v_Src    // Data vector to be written to SLM
             )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_slm + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i); 
        *( (T *)byteOffset ) = v_Src(i);
    }
}
template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API extern void                                                               
cm_slm_write (uint  slmBuffer,              // SLM buffer
              vector_ref<ushort, N> v_Addr, // Byte-Offsets into SLM Buffer
              vector<T, N>          &v_Src  // Data vector to be written to SLM
             )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_slm + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i); 
        *( (T *)byteOffset ) = v_Src(i);
    }
}

template <typename T1, typename T, uint M>
inline extern void
cm_slm_write4_internal (uint slmBuffer, T1 v_Addr, vector<T,M> &v_Src, SLM_ChannelMaskType mask, uint N, uint M1)
{
    static const bool conformable1 = Allowed_Type_Float_Or_Dword<T>::value;
    char *baseOffset, *byteOffset;
    char numColors=0, color[4]={0,0,0,0}, colorNext=0;
    baseOffset = __cm_emu_slm + slmBuffer;
    // mask = mask & 0x1111;
    if (!(mask & 0x1)) {color[0]=1; numColors++;}
    if (!(mask & 0x2)) {color[1]=1; numColors++;}
    if (!(mask & 0x4)) {color[2]=1; numColors++;}
    if (!(mask & 0x8)) {color[3]=1; numColors++;}
    if (numColors == 0) {
        fprintf(stderr, "cm_slm_read4 error: At least one"
                "destination vector has to be read!\n");
        exit(EXIT_FAILURE);
    }
    if (M1 < numColors*N) {
        fprintf(stderr, "cm_slm_read4 error: destination vector"
                "does not have enough space to hold data\n");
        exit(EXIT_FAILURE);
    }
    for (uint j=0; j<4; j++) {
        if (color[j] == 0) continue;
        for (uint i=0; i<N; i++) {
            byteOffset = baseOffset +  sizeof(T) * (v_Addr(i) + j);
            *( (T *)byteOffset ) = v_Src(i+colorNext*N);
        }
        colorNext++;
    }
}

template <typename T, uint N, uint M>
CM_API extern void
cm_slm_write4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector<T,M> &v_Src, SLM_ChannelMaskType mask)
{
    vector<T,M> src = v_Src;
    cm_slm_write4_internal(slmBuffer, v_Addr, src, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_write4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector_ref<T,M> v_Src, SLM_ChannelMaskType mask)
{
    vector<T,M> src = v_Src;
    cm_slm_write4_internal(slmBuffer, v_Addr, src, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_write4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector_ref<T,M> v_Src, SLM_ChannelMaskType mask)
{
    vector<T,M> src = v_Src;
    cm_slm_write4_internal(slmBuffer, v_Addr, src, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_write4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector<T,M> &v_Src, SLM_ChannelMaskType mask)
{
    vector<T,M> src = v_Src;
    cm_slm_write4_internal(slmBuffer, v_Addr, src, mask, N, M);
}

//-------------------------
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_write4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector<T,M> &v_Src, int imask)
{
    vector<T,M> src = v_Src;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_write4_internal(slmBuffer, v_Addr, src, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_write4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector_ref<T,M> v_Src, int imask)
{
    vector<T,M> src = v_Src;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_write4_internal(slmBuffer, v_Addr, src, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_write4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector_ref<T,M> v_Src, int imask)
{
    vector<T,M> src = v_Src;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_write4_internal(slmBuffer, v_Addr, src, mask, N, M);
}
template <typename T, uint N, uint M>
CM_API extern void
cm_slm_write4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector<T,M> &v_Src, int imask)
{
    vector<T,M> src = v_Src;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_write4_internal(slmBuffer, v_Addr, src, mask, N, M);
}

template <typename T1, uint R1, uint C1, typename T2, uint R2, uint C2>
CM_API extern void
cm_send(matrix<T1, R1, C1> &rspVar, matrix<T2, R2, C2> &msgVar, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc) 
{
    fprintf(stderr, "cm_send is not supported in emulation mode.\n");
    #ifdef CM_EMU
    exit(EXIT_FAILURE);
    #endif
}

template <typename T1, uint R1, uint C1, typename T2, uint R2, uint C2>
CM_API extern void
cm_send(matrix_ref<T1, R1, C1> rspVar, matrix_ref<T2, R2, C2> msgVar, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc) 
{
    fprintf(stderr, "cm_send is not supported in emulation mode.\n");
    #ifdef CM_EMU
    exit(EXIT_FAILURE);
    #endif
}

template <typename T, uint R, uint C>
CM_API extern void
cm_send(int rspVar, matrix_ref<T, R, C> msgVar, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc) 
{
    fprintf(stderr, "cm_send is not supported in emulation mode.\n");
    #ifdef CM_EMU
    exit(EXIT_FAILURE);
    #endif
}

template <typename T1, uint R1, uint C1, typename T2, uint R2, uint C2, typename T3, uint R3, uint C3>
CM_API extern void
cm_sends(matrix<T1, R1, C1> &rspVar, matrix<T2, R2, C2> &msgVar, matrix<T3, R3, C3> &msg2Var, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc) 
{
    fprintf(stderr, "cm_sends is not supported in emulation mode.\n");
    #ifdef CM_EMU
    exit(EXIT_FAILURE);
    #endif
}

template <typename T1, uint R1, uint C1, typename T2, uint R2, uint C2, typename T3, uint R3, uint C3>
CM_API extern void
cm_sends(matrix_ref<T1, R1, C1> rspVar, matrix_ref<T2, R2, C2> msgVar, matrix_ref<T3, R3, C3> msg2Var, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc) 
{
    fprintf(stderr, "cm_sends is not supported in emulation mode.\n");
    #ifdef CM_EMU
    exit(EXIT_FAILURE);
    #endif
}

template <typename T, uint R, uint C, typename T3, uint R3, uint C3>
CM_API extern void
cm_sends(int rspVar, matrix_ref<T, R, C> msgVar, matrix_ref<T3, R3, C3> msg2Var, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc) 
{
    fprintf(stderr, "cm_sends is not supported in emulation mode.\n");
    #ifdef CM_EMU
    exit(EXIT_FAILURE);
    #endif
}

template <typename T>
CM_API extern unsigned int
cm_get_value(T &index) 
{
    return index.get_data();
}

template <typename T>
CM_API extern vector<T, 8> 
cm_get_r0( ) 
{
    vector<T, 8> dummy;
    fprintf(stderr, "cm_get_r0 is not supported in emulation mode.\n");
    #ifdef CM_EMU
    exit(EXIT_FAILURE);
    #endif

    return dummy;
}

template <typename T>
CM_API extern vector<T, 4> 
cm_get_sr0( ) 
{
    vector<T, 4> dummy;
    fprintf(stderr, "cm_get_sr0 is not supported in emulation mode.\n");
    #ifdef CM_EMU
    exit(EXIT_FAILURE);
    #endif

    return dummy;
}

template <typename T>
CM_API extern void
cm_label(const char *pLabel) 
{
    fprintf(stderr, "\nInsert label %s:\n", pLabel);
}

template <typename T>
CM_API extern void
cm_label(const char *pLabel, int i) 
{
    fprintf(stderr, "\nInsert label %s_%d:\n", pLabel, i);
}

template <typename T>
CM_API extern void
cm_label(const char *pLabel, int i, int j) 
{
    fprintf(stderr, "\nInsert label %s_%d_%d\n", pLabel, i, j);
}

template <typename T>
CM_API extern void
cm_label(const char *pLabel, int i, int j, int k) 
{
    fprintf(stderr, "\nInsert label %s_%d_%d_%d:\n", pLabel, i, j, k);
}

// Implementation of SLM atomic operations

/*
typedef enum _AtomicOpType_
{
    CM_ATOMIC_ADD                     = 0x0,
    CM_ATOMIC_SUB                     = 0x1,
    CM_ATOMIC_INC                     = 0x2,
    CM_ATOMIC_DEC                     = 0x3,
    CM_ATOMIC_MIN                     = 0x4,
    CM_ATOMIC_MAX                     = 0x5,
    CM_ATOMIC_XCHG                    = 0x6,
    CM_ATOMIC_CMPXCHG                 = 0x7,
    CM_ATOMIC_AND                     = 0x8,
    CM_ATOMIC_OR                      = 0x9,
    CM_ATOMIC_XOR                     = 0xa,
    CM_ATOMIC_MINSINT                 = 0xb,
    CM_ATOMIC_MAXSINT                 = 0xc
} AtomicOpType;
*/

typedef enum _CmSLMAtomicOpType_
{
    // ATOMIC_CMPWR8B = 0x0 is not supported for SLM
    SLM_ATOMIC_AND                     = 0x1,
    SLM_ATOMIC_OR                      = 0x2,
    SLM_ATOMIC_XOR                     = 0x3,
    SLM_ATOMIC_MOV                     = 0x4,
    SLM_ATOMIC_INC                     = 0x5,
    SLM_ATOMIC_DEC                     = 0x6,
    SLM_ATOMIC_ADD                     = 0x7,
    SLM_ATOMIC_SUB                     = 0x8,
    SLM_ATOMIC_REVSUB                  = 0x9,
    SLM_ATOMIC_IMAX                    = 0xa,
    SLM_ATOMIC_IMIN                    = 0xb,
    SLM_ATOMIC_UMAX                    = 0xc,
    SLM_ATOMIC_UMIN                    = 0xd,
    SLM_ATOMIC_CMPWR                   = 0xe,
    SLM_ATOMIC_PREDEC                  = 0xf
} CmSLMAtomicOpType;

// This funtion performs atomic scattered DWord write to SLM

template <typename T, uint N>
inline extern void
cm_slm_atomic_internal(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr, 
                       vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    static const bool conformable1 = Allowed_Type_Float_Or_Dword<T>::value;
    static const bool conformable2 = Allowed_Vector_Length_8_Or_16<N>::value;

    int    i;
    char   *baseOffset, *byteOffset;
    uint   *uintPtr;
    int    *intPtr;

    baseOffset = __cm_emu_slm + slmBuffer;

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < N; i++) {

        SIMDCF_ELEMENT_SKIP(i);
        
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i);
        uintPtr    = (uint *) byteOffset;
        intPtr     = (int *) byteOffset;
        v_Dst(i)   = *uintPtr;

        // To Do: How to handle out-of-bound accesses to SLM for atomic writes?

        switch (op) {
            case SLM_ATOMIC_AND:
                *uintPtr = *uintPtr & v_Src0(i);
                break;
            case SLM_ATOMIC_OR:
                *uintPtr = *uintPtr | v_Src0(i);                 
                break;
            case SLM_ATOMIC_XOR:
                *uintPtr = *uintPtr ^ v_Src0(i);                
                break;
            case SLM_ATOMIC_MOV:
                *uintPtr = (uint) v_Src0(i);                 
                break;
            case SLM_ATOMIC_INC:
                *uintPtr = *uintPtr + 1;
                break;
            case SLM_ATOMIC_DEC:
                *uintPtr = *uintPtr - 1;
                break;
            case SLM_ATOMIC_ADD:
                *uintPtr = *uintPtr + (uint) v_Src0(i);
                break;
            case SLM_ATOMIC_SUB:
                *uintPtr = *uintPtr - (uint) v_Src0(i);
                break;
            case SLM_ATOMIC_REVSUB:
                *uintPtr = (uint) v_Src0(i) - *uintPtr;
                break;
            case SLM_ATOMIC_IMAX:
                *intPtr = (*intPtr > (int) v_Src0(i)) ? *intPtr : (int) v_Src0(i);   
                break;
            case SLM_ATOMIC_IMIN:
                *intPtr = (*intPtr > (int) v_Src0(i)) ? (int) v_Src0(i) : *intPtr;   
                break;
            case SLM_ATOMIC_UMAX:
                *uintPtr = (*uintPtr > (uint) v_Src0(i)) ? *uintPtr : (uint) v_Src0(i);
                break;
            case SLM_ATOMIC_UMIN:
                *uintPtr = (*uintPtr < (uint) v_Src0(i)) ? *uintPtr : (uint) v_Src0(i);
                break;
            case SLM_ATOMIC_CMPWR:
                *uintPtr = (*uintPtr == (uint) v_Src0(i)) ? (uint) v_Src1(i) : *uintPtr;
                break;
            case SLM_ATOMIC_PREDEC:
                *uintPtr = *uintPtr - 1;
                v_Dst(i) = *uintPtr;
                break;
            default:
                printf("Error writing SLM: invalid opcode for SLM atomic write!\n");
                exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);
}

//------------------------------------------------------------------------------
template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr, 
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, src0, src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr, 
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr, 
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr, 
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, src0, v_Src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr, 
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, v_Src0, src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr, 
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, v_Src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr, 
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, v_Src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr, 
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, v_Src0, v_Src1, v_Dst);
}
//-----------------------------------------------------------------------------

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr, 
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, src0, src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr, 
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr, 
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr, 
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, src0, v_Src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr, 
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src1 = v_Src1;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, v_Src0, src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr, 
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src1 = v_Src1;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, v_Src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr, 
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, v_Src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr, 
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, v_Src0, v_Src1, v_Dst);
}
//------------------------------------------------------------------------------

static CmSLMAtomicOpType Get_Gen_Atomic_Opcode( CmAtomicOpType aop ) 
{
    switch (aop) {
    case ATOMIC_ADD:
        return SLM_ATOMIC_ADD;
    case ATOMIC_SUB:
        return SLM_ATOMIC_SUB;
    case ATOMIC_INC:
        return SLM_ATOMIC_INC;
    case ATOMIC_DEC:
        return SLM_ATOMIC_DEC;
    case ATOMIC_MIN:
        return SLM_ATOMIC_UMIN;
    case ATOMIC_MAX:
        return SLM_ATOMIC_UMAX;
    case ATOMIC_XCHG:
        return SLM_ATOMIC_MOV;
    case ATOMIC_CMPXCHG:
        return SLM_ATOMIC_CMPWR;
    case ATOMIC_AND:
        return SLM_ATOMIC_AND;
    case ATOMIC_OR:
        return SLM_ATOMIC_OR;
    case ATOMIC_XOR:
        return SLM_ATOMIC_XOR;
    case ATOMIC_MINSINT:
        return SLM_ATOMIC_IMIN;
    case ATOMIC_MAXSINT:
        return SLM_ATOMIC_IMAX;
    default:
        exit(EXIT_FAILURE);
    }
}

// Top-level atomic functions:
// Either needs two sources, one source, or no sources
// Or two, one or no sources and NULL dst

template <typename T, typename T1, typename T2, typename T3>
CM_API extern void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr, 
              T1 v_Dst, T2 v_Src0, T3 v_Src1)
{
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (op != SLM_ATOMIC_CMPWR) {
        fprintf(stderr, "Two sources not allowed for the Atomic Operation! \n");
        exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, v_Src0, v_Src1, v_Dst);
}
template <typename T, typename T1, typename T2>
CM_API extern void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr, 
              T1 v_Dst, T2 v_Src0)
{
    T1 src1 = 0;
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    switch (op) {
        case SLM_ATOMIC_CMPWR:
            fprintf(stderr, "Two sources needed for ATOMIC_CMPWR! \n");
            exit(EXIT_FAILURE);
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            fprintf(stderr, "No sources allowed for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
        default:
        break;
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, v_Src0, src1, v_Dst);
}
template <typename T, typename T1>
CM_API extern void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr, 
              T1 v_Dst)
{
    T1 src0 = 0;
    T1 src1 = 0;
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    switch (op) {
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            break;
        default:
            fprintf(stderr, "Need source(s) for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, src0, src1, v_Dst);
}

template <typename T, typename T1, typename T2>
CM_API extern void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr, 
              int dst, T1 v_Src0, T2 v_Src1)
{
    T2 v_Dst = 0; // We don't actually care about the contents of this
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (dst != 0) {
        fprintf(stderr, "cm_slm_atomic passed destination vec as int but not NULL %x\n", dst);
        exit(EXIT_FAILURE);
    }

    if (op != SLM_ATOMIC_CMPWR) {
        fprintf(stderr, "Two sources not allowed for the Atomic Operation! \n");
        exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, v_Src0, v_Src1, v_Dst);
}
template <typename T, typename T1>
CM_API extern void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr, 
              int dst, T1 v_Src0)
{
    T1 src1 = 0;
    T1 v_Dst = 0; // We don't actually care about the contents of this
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (dst != 0) {
        fprintf(stderr, "cm_slm_atomic passed destination vec as int but not NULL %x\n", dst);
        exit(EXIT_FAILURE);
    }

    switch (op) {
        case SLM_ATOMIC_CMPWR:
            fprintf(stderr, "Two sources needed for ATOMIC_CMPWR! \n");
            exit(EXIT_FAILURE);
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            fprintf(stderr, "No sources allowed for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
        default:
        break;
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, v_Src0, src1, v_Dst);
}
template <typename T, uint N>
CM_API extern void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, vector<T,N> &v_Addr,
              int dst)
{
    // We can use the same type as the v_Addr as this will define the size of the vector correctly
    // and we don't actually care about the type otherwise (in this case it will be ushort) 
    vector<uint,N> src0 = 0;
    vector<uint,N> src1 = 0;
    vector<uint,N> v_Dst = 0; // We don't actually care about the contents of this
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (dst != 0) {
        fprintf(stderr, "cm_slm_atomic passed destination vec as int but not NULL %x\n", dst);
        exit(EXIT_FAILURE);
    }

    switch (op) {
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            break;
        default:
            fprintf(stderr, "Need source(s) for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, vector_ref<T,N> v_Addr,
              int dst)
{
    // We can use the same type as the v_Addr as this will define the size of the vector correctly
    // and we don't actually care about the type otherwise (in this case it will be ushort)
    vector<uint,N> src0 = 0;
    vector<uint,N> src1 = 0;
    vector<uint,N> v_Dst = 0; // We don't actually care about the contents of this
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (dst != 0) {
        fprintf(stderr, "cm_slm_atomic passed destination vec as int but not NULL %x\n", dst);
        exit(EXIT_FAILURE);
    }

    switch (op) {
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            break;
        default:
            fprintf(stderr, "Need source(s) for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, src0, src1, v_Dst);
}

template <typename RT, typename T1, typename T2, typename T3, uint SZ>
CM_API extern vector<RT, SZ>
cm_sada2(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1, const stream<T3,SZ>& src2,
        const uint flags = 0)
{
    static const bool conformable1 = bytetype<T1>::value;
    static const bool conformable2 = bytetype<T2>::value;
    static const bool conformable3 = wordtype<T3>::value;
    static const bool conformable4 = wordtype<RT>::value;
    static const bool conformable5 = check_true<!(SZ&1)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i+=1) {
        retv(i) = 0;
    }

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i+=2) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = CmEmulSys::abs(src0.get(i) - src1.get(i)) 
                + CmEmulSys::abs(src0.get(i+1) - src1.get(i+1))
                + src2.get(i);

        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
   }

   return retv;
}

template <typename RT, typename T1, typename T2, typename T3, uint SZ>
CM_API extern vector<RT, SZ>
cm_sada2(const stream<T1,SZ>& src0, const T2& src1, const stream<T3,SZ>& src2,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<T3, SZ> v3;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
        v3(i) = src2.get(i);
    }

    retv = cm_sada2<RT>(v1, v2, v3, flags);

    return retv;
}

//------------------------------------------------------------------------------
//             End of Groups and SLM Functions
//------------------------------------------------------------------------------

template <uint SZ>
CM_API extern vector<float, SZ>
cm_pln(const stream<float, 4>& src0, 
       const stream<float,SZ>& src1, 
       const stream<float,SZ>& src2,
       const uint flags = 0)
{
    int i;
    static const bool conformable2 = check_true<!(SZ%8)>::value;
    vector<float, SZ> retv;
    float ret;

    for (i = 0; i < SZ; i++) {
         ret = src0.get(0) * src1.get(i) + src0.get(1) * src2.get(i) + src0.get(3);
         SIMDCF_WRAPPER(retv(i) = CmEmulSys::satur<float>::saturate(ret, flags), SZ, i);
     }

    return retv;    
}

template <uint SZ>
CM_API extern vector<float, SZ>
cm_lrp(const stream<float,SZ>& src0, 
       const stream<float,SZ>& src1, 
       const stream<float,SZ>& src2, 
       const uint flags = 0)
{
    int i;
    vector<float, SZ> retv;
    float ret;

    for (i = 0; i < SZ; i++) {
         ret = src1.get(i) * src0.get(i) + src2.get(i) * (1.0 - src0.get(i));
         SIMDCF_WRAPPER(retv(i) = ret, SZ, i);
     }

    return retv;    
}

//------------------------------------------------------------------------------
// Temporary SVM functions that take svmptr_t address

// The /DCM_PTRSIZE=32 and /DCM_PTRSIZE=64 options allow the user to specify
// the size of svmptr_t in a CM program.
#ifdef CM_PTRSIZE
#if CM_PTRSIZE==32
typedef uint32_t svmptr_t;
#elif CM_PTRSIZE==64
typedef uint64_t svmptr_t;
#else
#error CM_PTRSIZE must be 32 or 64
#endif
#else
typedef uintptr_t svmptr_t;
#endif // def CM_PTRSIZE

// Read from SVM at the addresses starting at 'addr'
// and write it back to the destination vector 'v_Dst'
template <typename T, uint N>
CM_API extern void
cm_svm_block_read(uint64_t addr, // SVM pointer
                  vector_ref<T, N> v_Dst)   // Data vector to be written from SVM
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_read: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = ((T *)addr)[i];
    }
}

template <typename T, uint N>
CM_API extern void
cm_svm_block_read(uint64_t addr, // SVM pointer
                  vector<T, N> &v_Dst)   // Data vector to be written from SVM
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_read: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = ((T *)addr)[i];
    }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_block_read(uint64_t addr, // SVM pointer
                  matrix_ref<T, R, C> v_Dst)   // Data vector to be written from SVM
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_read: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = ((T *)addr)[i * C + j];
        }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_block_read(uint64_t addr, // SVM pointer
                  matrix<T, R, C> &v_Dst)   // Data vector to be written from SVM
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_read: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = ((T *)addr)[i * C + j];
        }
}

// Read from SVM at the addresses starting at 'addr'
// and write it back to the destination vector 'v_Dst'.
// The address does not need to be oword aligned.
template <typename T, uint N>
CM_API extern void
cm_svm_block_read_unaligned(uint64_t addr, // SVM pointer
                  vector_ref<T, N> v_Dst)   // Data vector to be written from SVM
{
    if (addr & 3) {
        fprintf(stderr, "cm_svm_block_read_unaligned: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = ((T *)addr)[i];
    }
}

template <typename T, uint N>
CM_API extern void
cm_svm_block_read_unaligned(uint64_t addr, // SVM pointer
                  vector<T, N> &v_Dst)   // Data vector to be written from SVM
{
    if (addr & 3) {
        fprintf(stderr, "cm_svm_block_read_unaligned: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = ((T *)addr)[i];
    }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_block_read_unaligned(uint64_t addr, // SVM pointer
                  matrix_ref<T, R, C> v_Dst)   // Data vector to be written from SVM
{
    if (addr & 3) {
        fprintf(stderr, "cm_svm_block_read_unaligned: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = ((T *)addr)[i * C + j];
        }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_block_read_unaligned(uint64_t addr, // SVM pointer
                  matrix<T, R, C> &v_Dst)   // Data vector to be written from SVM
{
    if (addr & 3) {
        fprintf(stderr, "cm_svm_block_read_unaligned: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = ((T *)addr)[i * C + j];
        }
}

// Read from SVM at the addresses given in 'v_Addr'
// and write it back to the destination vector 'v_Dst'
template <typename T, uint N>
CM_API extern void
cm_svm_scatter_read64(vector_ref<uint64_t, N> v_Addr, // vector of SVM pointers
                    vector_ref<T, N> v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = *(T *)v_Addr(i);
    }
}

template <typename T, uint N>
CM_API extern void
cm_svm_scatter_read64(vector<uint64_t, N> &v_Addr, // vector of SVM pointers
                    vector_ref<T, N> v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = *(T *)v_Addr(i);
    }
}

template <typename T, uint N>
CM_API extern void
cm_svm_scatter_read64(vector_ref<uint64_t, N> v_Addr, // vector of SVM pointers
                    vector<T, N> &v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = *(T *)v_Addr(i);
    }
}

template <typename T, uint N>
CM_API extern void
cm_svm_scatter_read64(vector<uint64_t, N> &v_Addr, // vector of SVM pointers
                    vector<T, N> &v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = *(T *)v_Addr(i);
    }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_scatter_read64(matrix_ref<uint64_t, R, C> v_Addr, // vector of SVM pointers
                    matrix_ref<T, R, C> v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = *(T *)v_Addr(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_scatter_read64(matrix<uint64_t, R, C> &v_Addr, // vector of SVM pointers
                    matrix_ref<T, R, C> v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = *(T *)v_Addr(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_scatter_read64(matrix_ref<uint64_t, R, C> v_Addr, // vector of SVM pointers
                    matrix<T, R, C> &v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = *(T *)v_Addr(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_scatter_read64(matrix<uint64_t, R, C> &v_Addr, // vector of SVM pointers
                    matrix<T, R, C> &v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = *(T *)v_Addr(i, j);
        }
}

// Write to SVM at the addresses starting at 'addr'
// from the source vector 'v_Src'
template <typename T, uint N>
CM_API extern void
cm_svm_block_write(uint64_t addr, // SVM pointer
                  vector<T, N> &v_Src   // Data vector to write into SVM
            )
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_write: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ((T *)addr)[i] = v_Src(i);
    }
}

template <typename T, uint N>
CM_API extern void
cm_svm_block_write(uint64_t addr, // SVM pointer
                  vector_ref<T, N> v_Src   // Data vector to write into SVM
            )
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_write: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ((T *)addr)[i] = v_Src(i);
    }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_block_write(uint64_t addr, // SVM pointer
                  matrix<T, R, C> &v_Src   // Data vector to write into SVM
            )
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_write: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            ((T *)addr)[i * C + j] = v_Src(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_block_write(uint64_t addr, // SVM pointer
                  matrix_ref<T, R, C> v_Src   // Data vector to write into SVM
            )
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_write: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            ((T *)addr)[i * C + j] = v_Src(i, j);
        }
}

// Write to SVM at the addresses given in 'v_Addr'
// from the source vector 'v_Src'
template <typename T, uint N>
CM_API extern void
cm_svm_scatter_write64(vector_ref<uint64_t, N> v_Addr, // vector of SVM pointers
                     vector<T, N> &v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        *(T *)v_Addr(i) = v_Src(i);
    }
}

template <typename T, uint N>
CM_API extern void
cm_svm_scatter_write64(vector<uint64_t, N> &v_Addr, // vector of SVM pointers
                     vector<T, N> &v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        *(T *)v_Addr(i) = v_Src(i);
    }
}

template <typename T, uint N>
CM_API extern void
cm_svm_scatter_write64(vector_ref<uint64_t, N> v_Addr, // vector of SVM pointers
                     vector_ref<T, N> v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        *(T *)v_Addr(i) = v_Src(i);
    }
}

template <typename T, uint N>
CM_API extern void
cm_svm_scatter_write64(vector<uint64_t, N> &v_Addr, // vector of SVM pointers
                     vector_ref<T, N> v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        *(T *)v_Addr(i) = v_Src(i);
    }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_scatter_write64(matrix_ref<uint64_t, R, C> v_Addr, // vector of SVM pointers
                     matrix<T, R, C> &v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            *(T *)v_Addr(i, j) = v_Src(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_scatter_write64(matrix<uint64_t, R, C> &v_Addr, // vector of SVM pointers
                     matrix<T, R, C> &v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            *(T *)v_Addr(i, j) = v_Src(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_scatter_write64(matrix_ref<uint64_t, R, C> v_Addr, // vector of SVM pointers
                     matrix_ref<T, R, C> v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            *(T *)v_Addr(i, j) = v_Src(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API extern void
cm_svm_scatter_write64(matrix<uint64_t, R, C> &v_Addr, // vector of SVM pointers
                     matrix_ref<T, R, C> v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            *(T *)v_Addr(i, j) = v_Src(i, j);
        }
}

// Wrappers for svm scatter/gather functions that take address vector in
// actual pointer size (32 or 64 bit) for the platform.
template <typename T, uint N> _GENX_ inline void cm_svm_scatter_read(vector<svmptr_t, N> v_Addr, vector_ref<T, N> v_Dst)
{   vector<uint64_t, N> v_Addr64(v_Addr);
    cm_svm_scatter_read64(v_Addr64, v_Dst);
}
template <typename T, uint N> _GENX_ inline void cm_svm_scatter_read(vector<svmptr_t, N> v_Addr, vector<T, N> &v_Dst)
{   vector<uint64_t, N> v_Addr64(v_Addr);
    cm_svm_scatter_read64(v_Addr64, v_Dst);
}
template <typename T, uint R, uint C> _GENX_ inline void cm_svm_scatter_read(matrix<svmptr_t, R, C> v_Addr, matrix_ref<T, R, C> v_Dst)
{   matrix<uint64_t, R, C> v_Addr64(v_Addr);
    cm_svm_scatter_read64(v_Addr64, v_Dst);
}
template <typename T, uint R, uint C> _GENX_ inline void cm_svm_scatter_read(matrix<svmptr_t, R, C> v_Addr, matrix<T, R, C> &v_Dst)
{   matrix<uint64_t, R, C> v_Addr64(v_Addr);
    cm_svm_scatter_read64(v_Addr64, v_Dst);
}
template <typename T, uint N> _GENX_ inline void cm_svm_scatter_write(vector<svmptr_t, N> v_Addr, vector_ref<T, N> v_Dst)
{   vector<uint64_t, N> v_Addr64(v_Addr);
    cm_svm_scatter_write64(v_Addr64, v_Dst);
}
template <typename T, uint N> _GENX_ inline void cm_svm_scatter_write(vector<svmptr_t, N> v_Addr, vector<T, N> &v_Dst)
{   vector<uint64_t, N> v_Addr64(v_Addr);
    cm_svm_scatter_write64(v_Addr64, v_Dst);
}
template <typename T, uint R, uint C> _GENX_ inline void cm_svm_scatter_write(matrix<svmptr_t, R, C> v_Addr, matrix_ref<T, R, C> v_Dst)
{   matrix<uint64_t, R, C> v_Addr64(v_Addr);
    cm_svm_scatter_write64(v_Addr64, v_Dst);
}
template <typename T, uint R, uint C> _GENX_ inline void cm_svm_scatter_write(matrix<svmptr_t, R, C> v_Addr, matrix<T, R, C> &v_Dst)
{   matrix<uint64_t, R, C> v_Addr64(v_Addr);
    cm_svm_scatter_write64(v_Addr64, v_Dst);
}

//------------------------------------------------------------------------------
// Implementation of SVM atomic operations

// This funtion performs atomic scattered DWord write to SVM
template <typename T, uint N>
inline extern void
cm_svm_atomic_internal(CmAtomicOpType op, vector<uint64_t,N> &v_Addr, 
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    static const bool conformable1 = Allowed_Type_Float_Or_Dword<T>::value;
    static const bool conformable2 = Allowed_Vector_Length_8_Or_16<N>::value;

    int    i;
    uint   *uintPtr;
    int    *intPtr;

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < N; i++) {

        SIMDCF_ELEMENT_SKIP(i);
        
        uintPtr    = (uint *)v_Addr(i);
        intPtr     = (int *)uintPtr;
        v_Dst(i)   = *uintPtr;

        // To Do: How to handle out-of-bound accesses to SVM for atomic writes?

        switch (op) {
            case ATOMIC_AND:
                *uintPtr = *uintPtr & v_Src0(i);
                break;
            case ATOMIC_OR:
                *uintPtr = *uintPtr | v_Src0(i);                 
                break;
            case ATOMIC_XOR:
                *uintPtr = *uintPtr ^ v_Src0(i);                
                break;
            case ATOMIC_INC:
                *uintPtr = *uintPtr + 1;
                break;
            case ATOMIC_DEC:
                *uintPtr = *uintPtr - 1;
                break;
            case ATOMIC_ADD:
                *uintPtr = *uintPtr + (uint) v_Src0(i);
                break;
            case ATOMIC_SUB:
                *uintPtr = *uintPtr - (uint) v_Src0(i);
                break;
            case ATOMIC_MAXSINT:
                *intPtr = (*intPtr > (int) v_Src0(i)) ? *intPtr : (int) v_Src0(i);   
                break;
            case ATOMIC_MINSINT:
                *intPtr = (*intPtr > (int) v_Src0(i)) ? (int) v_Src0(i) : *intPtr;   
                break;
            case ATOMIC_MAX:
                *uintPtr = (*uintPtr > (uint) v_Src0(i)) ? *uintPtr : (uint) v_Src0(i);
                break;
            case ATOMIC_MIN:
                *uintPtr = (*uintPtr < (uint) v_Src0(i)) ? *uintPtr : (uint) v_Src0(i);
                break;
            case ATOMIC_CMPXCHG:
                *uintPtr = (*uintPtr == (uint) v_Src0(i)) ? (uint) v_Src1(i) : *uintPtr;
                break;
            default:
                printf("Error writing SVM: invalid opcode for SVM atomic write!\n");
                exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);
}
//------------------------------------------------------------------------------
template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr, 
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, src0, src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr, 
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr, 
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr, 
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, src0, v_Src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr, 
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, v_Src0, src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr, 
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, v_Src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr, 
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, v_Src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr, 
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, v_Src0, v_Src1, v_Dst);
}
//-----------------------------------------------------------------------------

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr, 
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    cm_svm_atomic_internal(op, v_Addr, src0, src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr, 
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    cm_svm_atomic_internal(op, v_Addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr, 
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    cm_svm_atomic_internal(op, v_Addr, src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr, 
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    cm_svm_atomic_internal(op, v_Addr, src0, v_Src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr, 
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src1 = v_Src1;
    cm_svm_atomic_internal(op, v_Addr, v_Src0, src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr, 
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src1 = v_Src1;
    cm_svm_atomic_internal(op, v_Addr, v_Src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr, 
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    cm_svm_atomic_internal(op, v_Addr, v_Src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API extern void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr, 
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    cm_svm_atomic_internal(op, v_Addr, v_Src0, v_Src1, v_Dst);
}

// Top-level atomic functions:
// Either needs two sources, one source, or no sources

template <typename T1, typename T2, typename T3>
CM_API extern void
cm_svm_atomic64(CmAtomicOpType op, vector<uint64_t, 8> &v_Addr,
              T1 &v_Dst, T2 v_Src0, T3 v_Src1)
{
    if (op != ATOMIC_CMPXCHG) {
        fprintf(stderr, "Two sources not allowed for the Atomic Operation! \n");
        exit(EXIT_FAILURE);
    }
    cm_svm_atomic_generic(op, v_Addr, v_Src0, v_Src1, v_Dst);
}
template <typename T1, typename T2>
CM_API extern void
cm_svm_atomic64(CmAtomicOpType op, vector<uint64_t, 8> &v_Addr, 
              T1 &v_Dst, T2 v_Src0)
{
    T1 src1 = 0;
    switch (op) {
        case ATOMIC_CMPXCHG:
            fprintf(stderr, "Two sources needed for ATOMIC_CMPXCHG! \n");
            exit(EXIT_FAILURE);
        case ATOMIC_INC:
        case ATOMIC_DEC:
            fprintf(stderr, "No sources allowed for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
        default:
        break;
    }
    cm_svm_atomic_generic(op, v_Addr, v_Src0, src1, v_Dst);
}
template <typename T1>
CM_API extern void
cm_svm_atomic64(CmAtomicOpType op, vector<uint64_t, 8> &v_Addr, T1 &v_Dst)
{
    T1 src0 = 0;
    T1 src1 = 0;
    switch (op) {
        case ATOMIC_INC:
        case ATOMIC_DEC:
            break;
        default:
            fprintf(stderr, "Need source(s) for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
    }
    cm_svm_atomic_generic(op, v_Addr, src0, src1, v_Dst);
}

template <typename T1, typename T2, typename T3>
_GENX_ inline void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, 8> v_Addr, T1 &v_Dst, T2 v_Src0, T3 v_Src1)
{
#if 0
    // This doesn't seem to work on cm-icl:
    vector<uint64_t, 8> v_Addr64(v_Addr);
#else
    vector<uint64_t, 8> v_Addr64;
    v_Addr64[0] = v_Addr[0];
    v_Addr64[1] = v_Addr[1];
    v_Addr64[2] = v_Addr[2];
    v_Addr64[3] = v_Addr[3];
    v_Addr64[4] = v_Addr[4];
    v_Addr64[5] = v_Addr[5];
    v_Addr64[6] = v_Addr[6];
    v_Addr64[7] = v_Addr[7];
#endif
    cm_svm_atomic64(op, v_Addr64, v_Dst, v_Src0, v_Src1);
}
template <typename T1, typename T2>
_GENX_ inline void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, 8> v_Addr, T1 &v_Dst, T2 v_Src0)
{
#if 0
    // This doesn't seem to work on cm-icl:
    vector<uint64_t, 8> v_Addr64(v_Addr);
#else
    vector<uint64_t, 8> v_Addr64;
    v_Addr64[0] = v_Addr[0];
    v_Addr64[1] = v_Addr[1];
    v_Addr64[2] = v_Addr[2];
    v_Addr64[3] = v_Addr[3];
    v_Addr64[4] = v_Addr[4];
    v_Addr64[5] = v_Addr[5];
    v_Addr64[6] = v_Addr[6];
    v_Addr64[7] = v_Addr[7];
#endif
    cm_svm_atomic64(op, v_Addr64, v_Dst, v_Src0);
}
template <typename T1>
_GENX_ inline void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, 8> v_Addr, T1 &v_Dst)
{
#if 0
    // This doesn't seem to work on cm-icl:
    vector<uint64_t, 8> v_Addr64(v_Addr);
#else
    vector<uint64_t, 8> v_Addr64;
    v_Addr64[0] = v_Addr[0];
    v_Addr64[1] = v_Addr[1];
    v_Addr64[2] = v_Addr[2];
    v_Addr64[3] = v_Addr[3];
    v_Addr64[4] = v_Addr[4];
    v_Addr64[5] = v_Addr[5];
    v_Addr64[6] = v_Addr[6];
    v_Addr64[7] = v_Addr[7];
#endif
    cm_svm_atomic64(op, v_Addr64, v_Dst);
}

#if !defined(CM_EMU) && defined(CM_GEN10)
// for gen10 the following intrinsics are replaced by the
// software equivalents below:
#define cm_dp2 cm_dp2_gen10
#define cm_dp3 cm_dp3_gen10
#define cm_dp4 cm_dp4_gen10
#define cm_dph cm_dph_gen10
#define cm_lrp cm_lrp_gen10
#define cm_line cm_line_gen10

template <typename RT, typename T1, typename U, uint SZ>
_GENX_ inline vector<RT, SZ>
cm_dp2_gen10(const vector<T1,SZ> src0, U src1, const int flag = 0)
{
    vector<T1, SZ> _Src1 = src1;
    vector<RT, SZ> retv;
    CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
#pragma unroll
    for (int i = 0; i < SZ; i += 4) {
        retv.template select<4,1>(i) =
            cm_add<RT>(src0[i] * _Src1[i], src0[i + 1] * _Src1[i + 1],
                       flag);
    }
    return retv;
}

template <typename T0, typename T1, typename U, uint SZ>
_GENX_ inline vector<T0, SZ>
cm_dp2_gen10(vector_ref<T1, SZ> src0, U src1, const int flag = 0) {
  vector<T1, SZ> _Src0 = src0;
  return cm_dp2_gen10<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, uint N1, uint N2, typename U>
_GENX_ inline vector<T0, N1*N2>
cm_dp2_gen10(matrix<T1, N1, N2> src0, U src1, const int flag = 0) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_dp2_gen10<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, uint N1, uint N2, typename U>
_GENX_ inline vector<T0, N1*N2>
cm_dp2_gen10(matrix_ref<T1, N1, N2> src0, U src1, const int flag = 0) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_dp2_gen10<T0>(_Src0, src1, flag);
}

template <typename RT, typename T1, typename U, uint SZ>
_GENX_ inline vector<RT, SZ>
cm_dp3_gen10(const vector<T1,SZ> src0, U src1, const int flag = 0)
{
    vector<T1, SZ> _Src1 = src1;
    vector<RT, SZ> retv;
    CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
#pragma unroll
    for (int i = 0; i < SZ; i += 4) {
        retv.template select<4,1>(i) =
            cm_add<RT>(src0[i] * _Src1[i] + src0[i + 1] * _Src1[i + 1],
                       src0[i + 2] * _Src1[i + 2], flag);
    }
    return retv;
}

template <typename T0, typename T1, typename U, uint SZ>
_GENX_ inline vector<T0, SZ>
cm_dp3_gen10(vector_ref<T1, SZ> src0, U src1, const int flag = 0) {
  vector<T1, SZ> _Src0 = src0;
  return cm_dp3_gen10<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, uint N1, uint N2, typename U>
_GENX_ inline vector<T0, N1*N2>
cm_dp3_gen10(matrix<T1, N1, N2> src0, U src1, const int flag = 0) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_dp3_gen10<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, uint N1, uint N2, typename U>
_GENX_ inline vector<T0, N1*N2>
cm_dp3_gen10(matrix_ref<T1, N1, N2> src0, U src1, const int flag = 0) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_dp3_gen10<T0>(_Src0, src1, flag);
}

template <typename RT, typename T1, typename U, uint SZ>
_GENX_ inline vector<RT, SZ>
cm_dp4_gen10(const vector<T1,SZ> src0, U src1, const int flag = 0)
{
    vector<T1, SZ> _Src1 = src1;
    vector<RT, SZ> retv;
    CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
#pragma unroll
    for (int i = 0; i < SZ; i += 4) {
        retv.template select<4,1>(i) =
            cm_add<RT>(src0[i] * _Src1[i] + src0[i + 1] * _Src1[i + 1],
                       src0[i + 2] * _Src1[i + 2] + src0[i + 3] * _Src1[i + 3],
                       flag);
    }
    return retv;
}

template <typename T0, typename T1, typename U, uint SZ>
_GENX_ inline vector<T0, SZ>
cm_dp4_gen10(vector_ref<T1, SZ> src0, U src1, const int flag = 0) {
  vector<T1, SZ> _Src0 = src0;
  return cm_dp4_gen10<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, uint N1, uint N2, typename U>
_GENX_ inline vector<T0, N1*N2>
cm_dp4_gen10(matrix<T1, N1, N2> src0, U src1, const int flag = 0) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_dp4_gen10<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, uint N1, uint N2, typename U>
_GENX_ inline vector<T0, N1*N2>
cm_dp4_gen10(matrix_ref<T1, N1, N2> src0, U src1, const int flag = 0) {
  vector<T1, N1 *N2> _Src0 = src0;
  return cm_dp4_gen10<T0>(_Src0, src1, flag);
}

template <typename RT, typename T1, typename U, uint SZ>
_GENX_ inline vector<RT, SZ>
cm_dph_gen10(const vector<T1,SZ> src0, U src1, const int flag = 0)
{
    vector<T1, SZ> _Src1 = src1;
    vector<RT, SZ> retv;
    CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
#pragma unroll
    for (int i = 0; i < SZ; i += 4) {
        retv.template select<4,1>(i) =
            cm_add<RT>(src0[i] * _Src1[i] + src0[i + 1] * _Src1[i + 1],
                       src0[i + 2] * _Src1[i + 2] + _Src1[i + 3],
                       flag);
    }
    return retv;
}

template <typename T0, typename T1, typename U, uint SZ>
_GENX_ inline vector<T0, SZ>
cm_dph_gen10(vector_ref<T1, SZ> src0, U src1, const int flag = 0) {
    vector<T1, SZ> _Src0 = src0;
    return cm_dph_gen10<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, uint N1, uint N2, typename U>
_GENX_ inline vector<T0, N1*N2>
cm_dph_gen10(matrix<T1, N1, N2> src0, U src1, const int flag = 0) {
    vector<T1, N1*N2> _Src0 = src0;
    return cm_dph_gen10<T0>(_Src0, src1, flag);
}

template <typename T0, typename T1, uint N1, uint N2, typename U>
_GENX_ inline vector<T0, N1*N2>
cm_dph_gen10(matrix_ref<T1, N1, N2> src0, U src1, const int flag = 0) {
    vector<T1, N1*N2> _Src0 = src0;
    return cm_dph_gen10<T0>(_Src0, src1, flag);
}

template <typename T, uint SZ, typename U, typename V>
_GENX_ inline vector<T, SZ>
cm_lrp_gen10(vector<T, SZ> src0, U src1, V src2, const int flag = 0) {
    vector<float, SZ> _Src1 = src1;
    vector<float, SZ> _Src2 = src2;
    vector<float, SZ> _Result;
    return cm_add<float>(_Src1 * src0, _Src2 * (1.0f - src0), flag);
}

template <typename T, uint SZ, typename U, typename V>
_GENX_ inline vector<T, SZ>
cm_lrp_gen10(vector_ref<T, SZ> src0, U src1, V src2, const int flag = 0) {
    vector<float, SZ> _Src0 = src0;
    return cm_lrp_gen10(_Src0, src1, src2, flag);
}

template <uint N1, uint N2, typename U, typename V>
_GENX_ inline vector<float, N1*N2>
cm_lrp_gen10(matrix<float, N1, N2> src0, U src1, V src2, const int flag = 0) {
    vector<float, N1*N2> _Src0 = src0;
    return cm_lrp_gen10(_Src0, src1, src2, flag);
}

template <uint N1, uint N2, typename U, typename V>
_GENX_ inline vector<float, N1*N2>
cm_lrp_gen10(matrix_ref<float, N1, N2> src0, U src1, V src2, const int flag = 0) {
    vector<float, N1*N2> _Src0 = src0;
    return cm_lrp_gen10(_Src0, src1, src2, flag);
}

template <typename T, uint SZ>
_GENX_ inline vector<T, SZ>
cm_line_gen10(vector<T, 4> src0, vector<T, SZ> src1, const int flag = 0) {
    CM_STATIC_ERROR(SZ % 4 == 0, "result size is not a multiple of 4");
    vector<T, SZ> _Result;
#pragma unroll
    for (int i = 0; i < SZ; i += 4) {
      _Result.template select<4,1>(i) = cm_add<T>(src0[0] * src1[i], src0[3], flag);
    }
    return _Result;
}

template <typename T, uint SZ>
_GENX_ inline vector<T, SZ>
cm_line_gen10(vector<T, 4> src0, vector_ref<T, SZ> src1, const int flag = 0) {
    vector<T, SZ> _Src1 = src1;
    return cm_line<T>(src0, _Src1, flag);
}

template <typename T, uint N1, uint N2>
_GENX_ inline vector<T, N1*N2>
cm_line_gen10(vector<T, 4> src0, matrix<T, N1, N2> src1, const int flag = 0) {
    vector<T, N1*N2> _Src1 = src1;
    return cm_line<T>(src0, _Src1, flag);
}

template <typename T, uint N1, uint N2>
_GENX_ inline vector<T, N1*N2>
cm_line_gen10(vector<T, 4> src0, matrix_ref<T, N1, N2> src1, const int flag = 0) {
    vector<T, N1*N2> _Src1 = src1;
    return cm_line<T>(src0, _Src1, flag);
}

template <typename T, uint SZ>
_GENX_ inline vector<T, SZ>
cm_line_gen10(float P, float Q, vector<T, SZ> src1, const int flag = 0) {
    vector<T, 4> _Src0 = P;
    _Src0(3) = Q;
    return cm_line<float>(_Src0, src1, flag);
}

template <typename T, uint SZ>
_GENX_ inline vector<T, SZ>
cm_line_gen10(float P, float Q, vector<float, SZ> src1, const int flag = 0) {
    vector<T, 4> _Src0 = P;
    _Src0(3) = Q;
    return cm_line<float>(_Src0, src1, flag);
}

template <typename T, uint SZ>
_GENX_ inline vector<T, SZ>
cm_line_gen10(float P, float Q, vector_ref<float, SZ> src1, const int flag = 0) {
    vector<T, 4> _Src0 = P;
    _Src0(3) = Q;
    return cm_line<T>(_Src0, src1, flag);
}

template <typename T, uint N1, uint N2>
_GENX_ inline vector<T, N1*N2>
cm_line_gen10(float P, float Q, matrix<float, N1, N2> src1, const int flag = 0) {
    vector<float, N1*N2> _Src1 = src1;
    return cm_line<T>(P, Q, _Src1, flag);
}

template <typename T, uint N1, uint N2>
_GENX_ inline vector<T, N1*N2>
cm_line_gen10(float P, float Q, matrix_ref<float, N1, N2> src1, const int flag = 0) {
    vector<float, N1 *N2> _Src1 = src1;
    return cm_line<T>(P, Q, _Src1, flag);
}

#endif /* CM_GEN10 */

#endif /* CM_INTRIN_H */

