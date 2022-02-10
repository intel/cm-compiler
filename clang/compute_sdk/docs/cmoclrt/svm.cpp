/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

float *inputs = (float*)clSVMAlloc(context,              // ocl context
                                   CL_MEM_READ_WRITE,    // flags
                                   size * sizeof(float), // amount of memory to allocate in bytes
                                   0);                   // alignment in bytes

clSetKernelArgSVMPointer(kernel,  // kernel object
                         0,       // argument number
                         inputs); // pointer to SVM memory
...
clSVMFree(context, inputs);
