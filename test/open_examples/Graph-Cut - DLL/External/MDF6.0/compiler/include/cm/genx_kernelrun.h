/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: genx_kernelrun.h 24080 2010-09-24 18:25:04Z kchen24 $"
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
***                      Oleg Mirochnik
***                      
***                      
***
*** Description: Interface for running GenX kernels
***
*** -----------------------------------------------------------------------------------------------
**/


#ifndef CM_KERNELRUN_H
#define CM_KERNELRUN_H
extern int num_threads;
CM_API void CM_execute_kernels(void *kernel, ...);
CM_API void CM_set_thread_count(int n);

#ifndef CM_EMU
typedef
struct CM_GENX_MAIN_PARM_S
{
    int esize;
    int length;
    int offset;
    int type; //added to distnguish between scalar and pointer data
    union {
        __int64 d64bit;
        int     d32bit;
        short   d16bit;
        char    d8bit;
        void *  dptr;
    } data;
}
CM_GENX_MAIN_PARM;
#endif /* CM_EMU */

#endif /* CM_KERNELRUN_H */
