/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

unsigned globalSize[2] = { n / 32, 1 };
unsigned localSize[2] = { 1, 1 };

clEnqueueNDRangeKernel(queue,      // command queue
                       kernel,     // kernel object
                       2,          // work dimension
                       nullptr,    // global work offsets
                       globalSize, // global work sizes
                       localSize,  // local work sizes
                       0,          // number of events in wait list
                       nullptr,    // event wait list
                       nullptr     // event
                       );



const char *program_text = "<read CM kernel from source file>";
cl_program prog = clCreateProgramWithSource(context,       // OpenCL context
                                            1,             // the number of source code strings
                                            &program_text, // the source code strings
                                            nullptr,
                                            nullptr);
const char *opts = "-cmc";
clBuildProgram(prog,    // OpenCL program
               1,       // number of devices,
               &device, // device list
               opts,    // compile options
               nullptr, // notification routine pointer
               nullptr  // argument to notification routine
               );

cl_program prog = clCreateProgramWithIL(context, // OpenCL context
                                        binary,  // a pointer to length-byte block of memory
                                                 // containing SPIR-V
                                        length,  // length of SPIR-V binary in bytes
                                        &err);

const char *opts = "-vc-codegen";
clBuildProgram(prog,    // OpenCL program
               1,       // number of devices
               &device, // device list
               opts,    // compile options
               nullptr, // notification routine pointer
               nullptr  // argument to notification routine
               );

cl_program prog = clCreateProgramWithBinary(context,   // OpenCL context
                                            1,         // number of devices,
                                            &device,   // device list
                                            &bin_size, // the binary size
                                            binary,    // the binary string
                                            nullptr,   // binary status,
                                            nullptr    // error code
                                            );

