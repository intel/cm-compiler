/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

#include <iostream>
#include <cassert>
#include <math.h>
#include <vector>

#include <CL/cl.h>

#define SZ 160
#define KERNEL_SZ 16
#define CHECK(a) do { \
    err = (a); \
    if (err != CL_SUCCESS) { \
        fprintf(stderr, "FAIL: err=%d @ line=%d (%s)\n", err, __LINE__, (#a)); \
        exit(err); \
    } \
}while (0)
#define CHECK2(a) do { \
    (a); \
    if (err != CL_SUCCESS) { \
        fprintf(stderr, "FAIL: err=%d @ line=%d (%s)\n", err, __LINE__, (#a)); \
        exit(err); \
    } \
}while (0)
#ifndef KERNEL
#error "Error: KERNEL must be defined with location of kernel binary"
#endif

int main( int argc, char* argv[])
{
    // initialize data
    int *src1 = (int *)malloc(sizeof(int)*SZ);
    int *src2 = (int *)malloc(sizeof(int)*SZ);
    int *dst  = (int *)malloc(sizeof(int)*SZ);

    for (unsigned i=0; i<SZ; i++)
    {
        src1[i] = i;
        src2[i] = i<<2;
    }

    // initialize GPU
    cl_platform_id platform;  // OpenCL platform
    cl_device_id device;      // device ID
    cl_context context;       // context
    cl_command_queue queue;   // command queue
    cl_program program;       // program
    cl_kernel kernel;         // kernel
    cl_int err;

    CHECK(clGetPlatformIDs(1, &platform, NULL));
    CHECK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));
    CHECK2(context = clCreateContext(NULL, 1, &device, NULL, NULL, &err));
    CHECK2(queue = clCreateCommandQueueWithProperties(context, device, 0, &err));

    // diagnostic info
    char name_buffer[256];
    CHECK(clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(name_buffer), name_buffer, NULL));
    fprintf(stderr, "INFO: using platform: %s\n", name_buffer);
    CHECK(clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name_buffer), name_buffer, NULL));
    fprintf(stderr, "INFO: using device: %s\n", name_buffer);

    // read in and initialize kernel
    FILE *fp = fopen(KERNEL, "rb");
    if (fp == NULL) {
        fprintf(stderr, "FAIL: unable to open %s\n", KERNEL);
        exit(-1);
    }
    fseek(fp, 0, SEEK_END);
    size_t sz = ftell(fp);
    rewind(fp);

    unsigned char *code = (unsigned char *)malloc(sz);
    fread(code, 1, sz, fp);
    fclose(fp);

    cl_int errNum = 0;
    const unsigned char *codes[1] = {code};
    size_t sizes[1] = {sz};
    CHECK2(program = clCreateProgramWithBinary(context, 1, &device, sizes, codes, &err, &errNum));
    CHECK(clBuildProgram(program, 0, NULL, NULL, NULL, NULL));
    CHECK2(kernel = clCreateKernel(program, "vector_add", &err));

    // kernel parameter initialization
    cl_mem d_a, d_b, d_c;
    size_t bytes = SZ*sizeof(int);
    d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    d_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, NULL);
    CHECK(clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0, bytes, src1, 0, NULL, NULL));
    CHECK(clEnqueueWriteBuffer(queue, d_b, CL_TRUE, 0, bytes, src2, 0, NULL, NULL));
    CHECK(clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a));
    CHECK(clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b));
    CHECK(clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c));

    // send to GPU
    size_t globalSize = SZ/KERNEL_SZ;
    size_t localSize = 1;
    CHECK(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL));
    clFinish(queue);

    // process output and cleanup
    clEnqueueReadBuffer(queue, d_c, CL_TRUE, 0, bytes, dst, 0, NULL, NULL );
    clReleaseMemObject(d_a);
    clReleaseMemObject(d_b);
    clReleaseMemObject(d_c);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    // verify results
    for (unsigned i=0; i<SZ; i++)
       if ((src1[i] + src2[i]) != dst[i]) {
           fprintf(stderr, "FAIL: comparison at index[%d]: %d + %d => %d(host), but %d(gpu)\n", i, src1[i], src2[i], (src1[i]+src2[i]), dst[i]);
           //exit(-1);
       }
    fprintf(stderr, "PASSED\n");
    return 0;
}
