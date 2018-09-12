/**             
*** ---------------------------------------------------------------------------
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
*** Authors:             Somnath Ghosh
***                      
*** Description:
***     Optimized functions in Cm - could be used by Cm developers just like
***     Cm intrinsic functions for better performance
***     These functions use optimized code idioms that have found to provide
***     the optimal performance on GenX for the desired functionality
*** ---------------------------------------------------------------------------
**/

//------------------------------------------------------------------------------
// Functions implemented as user-level Cm kernels
//------------------------------------------------------------------------------

#ifndef CM_SLM_USER_H
#define CM_SLM_USER_H

#include "cm_intrin.h"

// This needs to be set by the user if GPGPU_MODE is needed
#define GPGPU_MODE

#ifdef GPGPU_MODE

// Linearization functions for IDs and SIZEs:
//------------------------------------------------------------------------------

// Returns the linear thread ID in a group

_GENX_ inline uint cm_linear_local_id (void)
{
    return (cm_local_size(0) * cm_local_id(1) + cm_local_id(0));
}

// Returns the linear group ID

_GENX_ inline uint cm_linear_group_id (void)
{
    return (cm_group_count(0) * cm_group_id(1) + cm_group_id(0));
}

// Returns the total number of threads per group

_GENX_ inline uint cm_linear_local_size (void)
{
    return (cm_local_size(0) * cm_local_size(1));
}

// Returns the linear global ID of a thread

_GENX_ inline uint cm_linear_global_id (void)
{
    return (cm_linear_group_id() * cm_linear_local_size() + cm_linear_local_id());
}

// Returns the total number of groups

_GENX_ inline uint cm_linear_group_count (void)
{
    return (cm_group_count(0) * cm_group_count(1));
}

// Returns the total number of threads

_GENX_ inline uint cm_linear_global_size (void)
{
    return (cm_linear_group_count() * cm_linear_local_size());
}

// User-level utility functions to manage SLM:
//------------------------------------------------------------------------------

static const   ushort  __cm_init_seq[8]={0,1,2,3,4,5,6,7};
_GENX_ static  uint    __cm_slm_offset; 

// Initializes SLM for the current kernel

_GENX_ inline void cm_slm_init (uint size)
{
    // set the SLM size per group
    cm_slm_size(size);

    __cm_slm_offset = 0;

#ifdef CM_EMU
    if (size <= 0) {
        fprintf(stderr, "Error in SLM Emulation:  Specify SLM size!\n");
        exit(EXIT_FAILURE);
    }
    if (size > __CM_SLM_MAX_SIZE) {
        fprintf(stderr, "Error in SLM Emulation:  Max SLM size = %dK!\n", 
                       __CM_SLM_MAX_SIZE/1024);
        exit(EXIT_FAILURE);
    }

    slmID = 0;

    __cm_slm_size = ((int)ceil((double)size/__CM_SLM_CHUNK_SIZE))*__CM_SLM_CHUNK_SIZE;

    if (__cm_emu_slm == NULL) {
        __cm_emu_slm = (char *) cm_emu_malloc(sizeof(char) * __cm_slm_size);

        if (__cm_emu_slm == NULL) {
            fprintf(stderr, "Error in SLM Emulation: memory allocation failure in cm_slm_init()!\n");
            exit(EXIT_FAILURE);
        }

        __cm_emu_slm_ptr[0] = __cm_emu_slm;
    }
#endif
}

// Returns the integer identifier of the SLM buffer allocated

_GENX_ inline uint cm_slm_alloc (uint bufferSize // Size in Bytes
                                 )
{
    uint slmBuffer;

    slmBuffer       = __cm_slm_offset;
    __cm_slm_offset += bufferSize;

#ifdef CM_EMU
    if (__cm_slm_offset > __cm_slm_size) {
        fprintf(stderr, "Error in SLM Emulation: Exceeded SLM Size of %dK! Try increasing SLM size.\n",
                        __cm_slm_size/1024);
        exit(EXIT_FAILURE);
    }
#endif

    return slmBuffer;
}

// Frees 'bufferSize' bytes of SLM
// Just sets the pointer to the next available SLM location
// No intelligent free-ing here - can be improved later.

_GENX_ inline void cm_slm_free (uint bufferSize // Size in Bytes
                                 )
{
    __cm_slm_offset -= bufferSize;

#ifdef CM_EMU
    if (__cm_slm_offset < 0) {
        fprintf(stderr, "Error in SLM Emulation: Free-ing more data than allocated!\n");
        exit(EXIT_FAILURE);
    }
#endif

}


// LOAD 'loadSize' bytes from memory surface 'memSurfIndex' starting at 'memOffset'
// to the SLM buffer 'slmBuffer'
// Use all the threads in the current group to read from memory and write to SLM

template <typename T1, typename T2>
_GENX_ inline void cm_slm_load (uint         slmBuffer,    // SLM buffer
                                SurfaceIndex memSurfIndex, // Memory SurfaceIndex
                                T1           memOffset,    // Byte-Offset in Memory Surface
                                T2           loadSize      // Bytes to be Loaded from Memory
                                )
{
#ifdef CM_EMU

    if (__cm_emu_slm_ptr[slmID] == NULL) {
        __cm_emu_slm = (char *) cm_emu_malloc(sizeof(char) * __cm_slm_size);

        if (__cm_emu_slm == NULL) {
            fprintf(stderr, "Error in SLM Emulation: memory allocation failure in cm_slm_init()!\n");
            exit(EXIT_FAILURE);
        }

        __cm_emu_slm_ptr[slmID] = __cm_emu_slm;

    }
    else {
        __cm_emu_slm = __cm_emu_slm_ptr[slmID];
    }

    slmID++;

    if (cm_linear_local_id() != 0) {
        return;
    }

#endif

    vector<ushort, 16> v_Offset(__cm_init_seq);
    v_Offset.select<8,1>(8) = v_Offset.select<8,1>(0) + 8;

#ifdef CM_EMU
    
    //check that loadSize is a multiple of 256 * #_threads_in_a_group
    if( loadSize % 256 != 0 ) {
        fprintf(stderr, "Warning: load size for cm_slm_load() must be multiple of 256\n");
    }
    //thread 0 reads every block
    int numBlocks = loadSize / 256;
    int numGroups = 1;

#else
    int numTotalBlocks = loadSize / 256;
    int numGroups = cm_linear_local_size();
    int numBlocks = numTotalBlocks / numGroups;
    int numLeftOver = numTotalBlocks % numGroups;
    numBlocks += cm_linear_local_id() < numLeftOver ? 1 : 0;
   
#endif
    
    // We just need numBlocks and numGroups
    ushort elemSize = sizeof(float);
    int threadOffsetInSLM = cm_linear_local_id() * 256;
    // in bytes
    int threadOffsetInMemory = memOffset + threadOffsetInSLM;
    // in unit of elements 
    vector<ushort, 16> v_Offsets = (threadOffsetInSLM / elemSize) + v_Offset * 4;

    for( int block = 0; block < numBlocks; block++ ) {
        vector<uint,   32> row0; // 32 floats or 128 Bytes or 4 GRF-registers
        vector<uint,   32> row1;
        vector<uint,   64> rowTrans;
        read (memSurfIndex, threadOffsetInMemory, row0);
        read (memSurfIndex, threadOffsetInMemory + 128, row1);

        //Transpose
        rowTrans.select<8,1>(0)  = row0.select<8,4>(0);
        rowTrans.select<8,1>(16) = row0.select<8,4>(1);
        rowTrans.select<8,1>(32) = row0.select<8,4>(2);
        rowTrans.select<8,1>(48) = row0.select<8,4>(3);

        rowTrans.select<8,1>(8)  = row1.select<8,4>(0);
        rowTrans.select<8,1>(24) = row1.select<8,4>(1);
        rowTrans.select<8,1>(40) = row1.select<8,4>(2);
        rowTrans.select<8,1>(56) = row1.select<8,4>(3);

        cm_slm_write4 (slmBuffer, v_Offsets, rowTrans, SLM_ABGR_ENABLE);
        threadOffsetInMemory += numGroups * 256;
        v_Offsets += numGroups * 64;
    }

#ifndef CM_EMU
    cm_barrier();
#endif
}

#endif // GPGPU_MODE

#endif // CM_SLM_USER_H
