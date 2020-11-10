/* * Copyright (c) 2020, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
// The only CM runtime header file that you need is cm_rt.h.
// It includes all of the CM runtime.
// Author: Elaine Wang
#include "cm_rt.h"

#include "common/isa_helpers.h"
#include "common/cm_rt_helpers.h"
#include "cm_cvt_color.h"
#include "rgb_cvt_def.h"

CmCvtColor::CmCvtColor():
        mDevice(nullptr),
        mTask(nullptr),
        mQueue(nullptr),
        mProgram(nullptr),
        mOut(nullptr),
        mLog(false)
{
    // Creates a CmDevice from scratch.
    // Param device: pointer to the CmDevice object.
    // Param version: CM API version supported by the runtime library.
    unsigned int version = 0;
    cm_result_check(::CreateCmDevice(mDevice, version));

    // The file linear_walker_genx.isa is generated when the kernels in the file
    // linear_walker_genx.cpp are compiled by the CM compiler.
    // Reads in the virtual ISA from "xxx_genx.isa" to the code
    // buffer.
    std::string isa_code = cm::util::isa::loadFile("rgb_cvt_genx.isa");
    if (isa_code.size() == 0) {
        std::cerr << "Error: empty ISA binary.\n";
        exit(1);
    }

    // Creates a CmProgram object consisting of the kernels loaded from the code
    // buffer.
    // Param isa_code.data(): Pointer to the code buffer containing the virtual
    // ISA.
    // Param isa_code.size(): Size in bytes of the code buffer containing the
    // virtual ISA.
    cm_result_check(mDevice->LoadProgram(const_cast<char*>(isa_code.data()),
                isa_code.size(),
                mProgram));

    // Creates the linear kernel.
    // Param mProgram: CM Program from which the kernel is created.
    // Param "xxx": The kernel name which should be no more than 256 bytes
    // including the null terminator.
    cm_result_check(mDevice->CreateKernel(mProgram,
                "rgb_cvt",
                mKernel));

    if (mLog)
    {
        cm_result_check(mDevice->InitPrintBuffer());
    }
 
}

int CmCvtColor::CreateOutputBuffer(char *pOut)
{
    cm_result_check(mDevice->CreateBufferUP(3 * IMG_W * IMG_H * sizeof(char), pOut, mOut));

}


int CmCvtColor::CreateKernel()
{
    // Each CmKernel can be executed by multiple concurrent threads.
    int thread_width = THREAD_W;
    int thread_height ;
    unsigned max_num =  IMG_W * IMG_H;
    thread_height =  IMG_W * IMG_H / LOC_SIZE / thread_width;

    // Creates a CmThreadSpace object.
    // There are two usage models for the thread space. One is to define the
    // dependency between threads to run in the GPU. The other is to define a
    // thread space where each thread can get a pair of coordinates during
    // mKernel execution. For this example, we use the latter usage model.
    if (mLog)
    {
       std::cout<<"Create thread space ("<<thread_width<<"X"<<thread_height<<")\n";
    }
    cm_result_check(mDevice->CreateThreadSpace(thread_width,
                thread_height,
                mThreadSpace));

    // The CmQueue is an in-order queue. Tasks get executed according to the
    // order they are enqueued. The next task does not start execution until the
    // current task finishes.
    cm_result_check(mDevice->CreateQueue(mCmdQueue));
    // Creates a CmTask object.
    // The CmTask object is a container for CmKernel pointers.
    // It is used to enqueue the kernels for execution.
    cm_result_check(mDevice->CreateTask(mTask));

    return 0;
} 


int CmCvtColor::Cvt2Rgb(char *input)
{
    CmEvent *sync_event = nullptr;
    CmBufferUP *mIn;
    // Creates a CmTask object.
    // The CmTask object is a container for CmKernel pointers. It is used to
    // enqueue the mKernels for execution.
    mTask->Reset();

    // When a surface is created by the CmDevice a SurfaceIndex object is
    // created. This object contains a unique index value that is mapped to the
    // surface.
    // Gets the input surface index.

    cm_result_check(mDevice->CreateBufferUP(4 * IMG_W * IMG_H * sizeof(char), input, mIn));

    SurfaceIndex *input_surf1_idx = nullptr;
    cm_result_check(mIn->GetIndex(input_surf1_idx));

    // Gets the output surface index.
    SurfaceIndex *output_surf_idx = nullptr;
    cm_result_check(mOut->GetIndex(output_surf_idx));
    // Sets a per mKernel argument.
    // Sets input surface index as the first argument of linear mKernel.
    cm_result_check(mKernel->SetKernelArg(0,
                sizeof(SurfaceIndex),
                input_surf1_idx));

    cm_result_check(mKernel->SetKernelArg(1,
                sizeof(SurfaceIndex),
                output_surf_idx));

    // Adds a CmKernel pointer to CmTask.
    // This task has one mKernel, "linear".
    cm_result_check(mTask->AddKernel(mKernel));

    // Launches the task on the GPU. Enqueue is a non-blocking call, i.e. the
    // function returns immediately without waiting for the GPU to start or
    // finish execution of the task. The runtime will query the HW status. If
    // the hardware is not busy, the runtime will submit the task to the
    // driver/HW; otherwise, the runtime will submit the task to the driver/HW
    // at another time.
    // An event, "sync_event", is created to track the status of the task.
    cm_result_check(mCmdQueue->Enqueue(mTask,
                sync_event,
                mThreadSpace));

    cm_result_check(sync_event->WaitForTaskFinished());

    // Queries the execution time of a task in the unit of nanoseconds.
    // The execution time is measured from the time the task started execution
    // in the GPU to the time when the task finished execution.
    if (mLog)
    {
        static int idx = 0;
        if ((idx++) % 30 == 0)
        {
            UINT64 execution_time = 0;
            cm_result_check(sync_event->GetExecutionTime(execution_time));
            std::cout << "Kernel linear execution time is " << execution_time / 1000
                << " nanoseconds" << std::endl;

            cm_result_check(mDevice->FlushPrintBuffer());
        }
    }
    cm_result_check(mCmdQueue->DestroyEvent(sync_event));
    cm_result_check(mDevice->DestroyBufferUP(mIn));

    return 0;
}

CmCvtColor::~CmCvtColor()
{
    // Destroys the CmDevice.
    // Also destroys surfaces, kernels, tasks, thread spaces, and queues that
    // were created using this device instance that have not explicitly been
    // destroyed by calling the respective destroy functions.
    cm_result_check(::DestroyCmDevice(mDevice));
}

