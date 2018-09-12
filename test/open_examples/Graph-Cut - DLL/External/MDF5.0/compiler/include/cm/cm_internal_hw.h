/**             
*** ---------------------------------------------------------------------------
*** Copyright  (C) 1985-2010 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
*** Authors:             Biju George
***                      
*** Description:
***     CM internal functions (not to be called directly in CM programs).
*** ---------------------------------------------------------------------------
**/

#ifndef CM_INTERNAL_HW_H
#define CM_INTERNAL_HW_H

namespace __CMInternal__ {

    // ------------------------------------------------------------------------
    // SIMD condition expression annotators
    // ------------------------------------------------------------------------

    template <typename T, uint R, uint C>
    CM_API extern ushort
    __cm_internal_simd(const matrix<T,R,C> &cond);

    template <typename T, uint R, uint C>
    CM_API extern ushort 
    __cm_internal_simd(const matrix_ref<T,R,C> cond);

    template <typename T, uint S>
    CM_API extern ushort 
    __cm_internal_simd(const vector<T, S> &cond);

    template <typename T, uint S>
    CM_API extern ushort 
    __cm_internal_simd(const vector_ref<T, S> cond);

    CM_API extern ushort
    __cm_internal_simd(char cond);

    CM_API extern ushort
    __cm_internal_simd(unsigned char cond);

    CM_API extern ushort
    __cm_internal_simd(short cond);

    CM_API extern ushort
    __cm_internal_simd(unsigned short cond);

    CM_API extern ushort
    __cm_internal_simd(int cond);

    CM_API extern ushort
    __cm_internal_simd(unsigned int cond);

    // ------------------------------------------------------------------------
    // SIMD control flow annotators
    // ------------------------------------------------------------------------

    CM_API extern ushort
    __cm_internal_simd_if_begin();

    CM_API extern ushort
    __cm_internal_simd_elseif_begin();

    CM_API extern ushort
    __cm_internal_simd_then_end();

    CM_API extern ushort
    __cm_internal_simd_else_begin();

    CM_API extern ushort
    __cm_internal_simd_if_end();

    CM_API extern ushort
    __cm_internal_simd_if_join();

    CM_API extern ushort
    __cm_internal_simd_do_while_begin();

    CM_API extern ushort
    __cm_internal_simd_do_while_end();

    CM_API extern ushort
    __cm_internal_simd_break();

    CM_API extern ushort
    __cm_internal_simd_continue();
};

#endif /* CM_INTERNAL_HW_H */
