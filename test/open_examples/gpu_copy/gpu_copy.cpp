/*
 * Copyright (c) 2018, Intel Corporation
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

#include <string>
#include <iostream>

// The only CM runtime header file that you need is cm_rt.h.
// It includes all of the CM runtime.
#include "cm_rt.h"

// Include cm_rt_helpers.h to convert the integer return code returned from
// the CM runtime to a meaningful string message.
#include "common/cm_rt_helpers.h"

// Copy data from graphics memory to system memory
// This function copies an ARGB surface and an NV12 surface
int CopyFromGPUToCPU(CmDevice *device, CmQueue *cmd_queue, unsigned int surf_width, unsigned int surf_height, unsigned int copy_width, unsigned int copy_height)
{
    // Sanity check
    if (copy_width % 16)
    {
        std::cerr << "Error: copy_width should be 16-aligned." << std::endl;
        return -1;
    }

    // Create the target system memory, one for ARGB and the other for NV12
    // 4 bytes per pixel in ARGB format, 3/2 bytes per pixel in NV12 format
    // The start address should be 16-aligned.
    unsigned char *dst_mem_argb = (unsigned char *)CM_ALIGNED_MALLOC(copy_width * copy_height * 4, 16);
    unsigned char *dst_mem_nv12 = (unsigned char *)CM_ALIGNED_MALLOC(copy_width * copy_height * 3/2, 16);
    if (dst_mem_argb == nullptr || dst_mem_nv12 == nullptr)
    {
        std::cerr << "Error: system memory allocation failed." << std::endl;
        return -1;
    }

    // Fill the destination memories with 0
    memset(dst_mem_argb, 0, copy_width * copy_height * 4);
    memset(dst_mem_nv12, 0, copy_width * copy_height * 3/2);

    // Creates source surface with given width and height.
    // Sets surface format as CM_SURFACE_FORMAT_A8R8G8B8 for ARGB, and each
    // pixel occupies 32 bits. Sets the surface format as CM_SURFACE_FORMAT_NV12
    // for NV12, and the surface will contain both Y and UV planes.
    CmSurface2D *src_surf_argb = nullptr;
    cm_result_check(device->CreateSurface2D(surf_width,
                                           surf_height,
                                           CM_SURFACE_FORMAT_A8R8G8B8,
                                           src_surf_argb));
    CmSurface2D *src_surf_nv12 = nullptr;
    cm_result_check(device->CreateSurface2D(surf_width,
                                           surf_height,
                                           CM_SURFACE_FORMAT_NV12,
                                           src_surf_nv12));

    // Fill the source surface with random data
    // The source data are kept for results verification later
    unsigned char *src_data_argb = (unsigned char *) malloc(surf_width * surf_height * 4);
    unsigned char *src_data_nv12 = (unsigned char *) malloc(surf_width * surf_height * 3/2);
    if (src_data_argb == nullptr || src_data_nv12 == nullptr)
    {
        std::cerr << "Error: system memory allocation failed." << std::endl;
        return -1;
    }
    for (unsigned int i = 0; i < surf_width * surf_height * 4; i++)
    {
        int temp = rand();
        src_data_argb[i] = temp & 0x000000FF;
    }
    for (unsigned int i = 0; i < surf_width * surf_height * 3/2; i++)
    {
        int temp = rand();
        src_data_nv12[i] = temp & 0x000000FF;
    }
    cm_result_check(src_surf_argb->WriteSurface(src_data_argb, NULL));
    cm_result_check(src_surf_nv12->WriteSurface(src_data_nv12, NULL));

    // Launches the gpu copy task on the GPU.
    // The taskes are executed sequentially, so only need to check the status of the
    // last task.
    // An event, "sync_event", is created to track the status of the second task.
    // "FullStride" in the function name means it has two arguments (3rd and 4th) to
    // set the horizontal and vertical stride of the system memory. The horizontal stride
    // is in unit of byte while the vertical one in unit of rows.
    CmEvent *sync_event = nullptr;
    CmEvent *no_event = CM_NO_EVENT;
    cm_result_check(cmd_queue->EnqueueCopyGPUToCPUFullStride(src_surf_argb, dst_mem_argb, copy_width * 4, copy_height, CM_FASTCOPY_OPTION_NONBLOCKING, no_event));
    cm_result_check(cmd_queue->EnqueueCopyGPUToCPUFullStride(src_surf_nv12, dst_mem_nv12, copy_width, copy_height, CM_FASTCOPY_OPTION_NONBLOCKING, sync_event));

    // Wait till the gpu copy task finished
    cm_result_check(sync_event->WaitForTaskFinished());

    // Destroys the CmEvent.
    // CmEvent must be destroyed by the user explicitly.
    cm_result_check(cmd_queue->DestroyEvent(sync_event));

    // Check the results
    // Check ARGB
    for (unsigned int i = 0; i < copy_height; i++)
    {
        for (unsigned int j = 0; j < copy_width * 4; j++)
        {
            unsigned int src_pos = i * surf_width * 4 + j;
            unsigned int dst_pos = i * copy_width * 4 + j;
            if (src_data_argb[src_pos] != dst_mem_argb[dst_pos])
            {
                std::cerr << "Error: Data mismatch in ARGB surface at position " << dst_pos << "." << std::endl;
                return -1;
            }
        }
    }
    std::cout << "ARGB data copying from GPU to CPU: PASS" << std::endl;
    // Check NV12
    // Check Y plane
    for (unsigned int i = 0; i < copy_height; i++)
    {
        for (unsigned int j = 0; j < copy_width; j++)
        {
            unsigned int src_pos = i * surf_width + j;
            unsigned int dst_pos = i * copy_width + j;
            if (src_data_nv12[src_pos] != dst_mem_nv12[dst_pos])
            {
                std::cerr << "Error: Y Data mismatch in NV12 surface at position " << dst_pos << "." << std::endl;
                return -1;
            }
        }
    }
    // Check NV plane
    for (unsigned int i = 0; i < copy_height/2; i++)
    {
        for (unsigned int j = 0; j < copy_width; j++)
        {
            unsigned int src_pos = surf_width * surf_height + i * surf_width + j;
            unsigned int dst_pos = copy_width * copy_height + i * copy_width + j;
            if (src_data_nv12[src_pos] != dst_mem_nv12[dst_pos])
            {
                std::cerr << "Error: UV Data mismatch in NV12 surface at position " << dst_pos << "." << std::endl;
                return -1;
            }
        }
    }
    std::cout << "NV12 data copying from GPU to CPU: PASS" << std::endl;

    // free resources
    free(src_data_argb);
    free(src_data_nv12);
    CM_ALIGNED_FREE(dst_mem_argb);
    CM_ALIGNED_FREE(dst_mem_nv12);

    return 0;
}

// Copy data from system memory to graphics memory
// This function copies an ARGB surface and an NV12 surface
int CopyFromCPUToGPU(CmDevice *device, CmQueue *cmd_queue, unsigned int surf_width, unsigned int surf_height, unsigned int copy_width, unsigned int copy_height)
{
    // Sanity check
    if (copy_width % 16)
    {
        std::cerr << "Error: copy_width should be 16-aligned." << std::endl;
        return -1;
    }

    // Create the source system memory, one for ARGB and the other for NV12
    // 4 bytes per pixel in ARGB format, 3/2 bytes per pixel in NV12 format
    // The start address should be 16-aligned.
    unsigned char *src_mem_argb = (unsigned char *)CM_ALIGNED_MALLOC(surf_width * surf_height * 4, 16);
    unsigned char *src_mem_nv12 = (unsigned char *)CM_ALIGNED_MALLOC(surf_width * surf_height * 3/2, 16);
    if (src_mem_argb == nullptr || src_mem_nv12 == nullptr)
    {
        std::cerr << "Error: system memory allocation failed." << std::endl;
        return -1;
    }

    // Fill the source memories with random data
    for (unsigned int i = 0; i < surf_width * surf_height * 4; i++)
    {
        int temp = rand();
        src_mem_argb[i] = temp & 0x000000FF;
    }
    for (unsigned int i = 0; i < surf_width * surf_height * 3/2; i++)
    {
        int temp = rand();
        src_mem_nv12[i] = temp & 0x000000FF;
    }

    // Creates target surface with given width and height.
    // Sets surface format as CM_SURFACE_FORMAT_A8R8G8B8 for ARGB, and each
    // pixel occupies 32 bits. Sets the surface format as CM_SURFACE_FORMAT_NV12
    // for NV12, and the surface will contain both Y and UV planes.
    CmSurface2D *dst_surf_argb = nullptr;
    cm_result_check(device->CreateSurface2D(copy_width,
                                           copy_height,
                                           CM_SURFACE_FORMAT_A8R8G8B8,
                                           dst_surf_argb));
    CmSurface2D *dst_surf_nv12 = nullptr;
    cm_result_check(device->CreateSurface2D(copy_width,
                                           copy_height,
                                           CM_SURFACE_FORMAT_NV12,
                                           dst_surf_nv12));

    // Initialize the dst surfaces with 0
    cm_result_check(dst_surf_argb->InitSurface(0, nullptr));
    cm_result_check(dst_surf_nv12->InitSurface(0, nullptr));

    // Launches the gpu copy task on the GPU.
    // The taskes are executed sequentially, so only need to check the status of the
    // last task.
    // An event, "sync_event", is created to track the status of the second task.
    // "FullStride" in the function name means it has two arguments (3rd and 4th) to
    // set the horizontal and vertical stride of the system memory. The horizontal stride
    // is in unit of byte while the vertical one in unit of rows. 
    CmEvent *sync_event = nullptr;
    CmEvent *no_event = CM_NO_EVENT;
    cm_result_check(cmd_queue->EnqueueCopyCPUToGPUFullStride(dst_surf_argb, src_mem_argb, surf_width * 4, surf_height, CM_FASTCOPY_OPTION_NONBLOCKING, no_event));
    cm_result_check(cmd_queue->EnqueueCopyCPUToGPUFullStride(dst_surf_nv12, src_mem_nv12, surf_width, surf_height, CM_FASTCOPY_OPTION_NONBLOCKING, sync_event));

    // Wait till the gpu copy task finished
    cm_result_check(sync_event->WaitForTaskFinished());

    // Destroys the CmEvent.
    // CmEvent must be destroyed by the user explicitly.
    cm_result_check(cmd_queue->DestroyEvent(sync_event));

    // Read the target surfaces
    unsigned char *dst_data_argb = (unsigned char *)malloc(copy_width * copy_height * 4);
    unsigned char *dst_data_nv12 = (unsigned char *)malloc(copy_width * copy_height * 3/2);
    cm_result_check(dst_surf_argb->ReadSurface(dst_data_argb, nullptr));
    cm_result_check(dst_surf_nv12->ReadSurface(dst_data_nv12, nullptr));

    // Check the results
    // Check ARGB
    for (unsigned int i = 0; i < copy_height; i++)
    {
        for (unsigned int j = 0; j < copy_width * 4; j++)
        {
            unsigned int src_pos = i * surf_width * 4 + j;
            unsigned int dst_pos = i * copy_width * 4 + j;
            if (src_mem_argb[src_pos] != dst_data_argb[dst_pos])
            {
                std::cerr << "Error: Data mismatch in ARGB surface at position " << dst_pos << "." << std::endl;
                return -1;
            }
        }
    }
    std::cout << "ARGB data copying from CPU to GPU: PASS" << std::endl;
    // Check NV12
    // Check Y plane
    for (unsigned int i = 0; i < copy_height; i++)
    {
        for (unsigned int j = 0; j < copy_width; j++)
        {
            unsigned int src_pos = i * surf_width + j;
            unsigned int dst_pos = i * copy_width + j;
            if (src_mem_nv12[src_pos] != dst_data_nv12[dst_pos])
            {
                std::cerr << "Error: Y Data mismatch in NV12 surface at position " << dst_pos << "." << std::endl;
                return -1;
            }
        }
    }
    // Check NV plane
    for (unsigned int i = 0; i < copy_height/2; i++)
    {
        for (unsigned int j = 0; j < copy_width; j++)
        {
            unsigned int src_pos = surf_width * surf_height + i * surf_width + j;
            unsigned int dst_pos = copy_width * copy_height + i * copy_width + j;
            if (src_mem_nv12[src_pos] != dst_data_nv12[dst_pos])
            {
                std::cerr << "Error: UV Data mismatch in NV12 surface at position " << dst_pos << "." << std::endl;
                return -1;
            }
        }
    }
    std::cout << "NV12 data copying from CPU to GPU: PASS" << std::endl;

    // free resources
    free(dst_data_argb);
    free(dst_data_nv12);
    CM_ALIGNED_FREE(src_mem_argb);
    CM_ALIGNED_FREE(src_mem_nv12);

    return 0;
}

// Copy data from grphics memory to graphics memory
// This function copies an ARGB surface and an NV12 surface
int CopyFromGPUToGPU(CmDevice *device, CmQueue *cmd_queue, unsigned int width, unsigned int height)
{
    // Creates source and target surfaces with given width and height.
    // Sets surface format as CM_SURFACE_FORMAT_A8R8G8B8 for ARGB, and each
    // pixel occupies 32 bits. Sets the surface format as CM_SURFACE_FORMAT_NV12
    // for NV12, and the surface will contain both Y and UV planes.
    CmSurface2D *src_surf_argb = nullptr;
    cm_result_check(device->CreateSurface2D(width,
                                           height,
                                           CM_SURFACE_FORMAT_A8R8G8B8,
                                           src_surf_argb));
    CmSurface2D *src_surf_nv12 = nullptr;
    cm_result_check(device->CreateSurface2D(width,
                                           height,
                                           CM_SURFACE_FORMAT_NV12,
                                           src_surf_nv12));
    CmSurface2D *dst_surf_argb = nullptr;
    cm_result_check(device->CreateSurface2D(width,
                                           height,
                                           CM_SURFACE_FORMAT_A8R8G8B8,
                                           dst_surf_argb));
    CmSurface2D *dst_surf_nv12 = nullptr;
    cm_result_check(device->CreateSurface2D(width,
                                           height,
                                           CM_SURFACE_FORMAT_NV12,
                                           dst_surf_nv12));

    // Initialize the dst surfaces with 0
    cm_result_check(dst_surf_argb->InitSurface(0, nullptr));
    cm_result_check(dst_surf_nv12->InitSurface(0, nullptr));

    // Fill the source surface with random data
    // The source data are kept for results verification later
    unsigned char *src_data_argb = (unsigned char *) malloc(width * height * 4);
    unsigned char *src_data_nv12 = (unsigned char *) malloc(width * height * 3/2);
    if (src_data_argb == nullptr || src_data_nv12 == nullptr)
    {
        std::cerr << "Error: system memory allocation failed." << std::endl;
        return -1;
    }
    for (unsigned int i = 0; i < width * height * 4; i++)
    {
        int temp = rand();
        src_data_argb[i] = temp & 0x000000FF;
    }
    for (unsigned int i = 0; i < width * height * 3/2; i++)
    {
        int temp = rand();
        src_data_nv12[i] = temp & 0x000000FF;
    }
    cm_result_check(src_surf_argb->WriteSurface(src_data_argb, NULL));
    cm_result_check(src_surf_nv12->WriteSurface(src_data_nv12, NULL));

    // Launches the gpu copy task on the GPU.
    // The taskes are executed sequentially, so only need to check the status of the
    // last task.
    // An event, "sync_event", is created to track the status of the second task.
    CmEvent *sync_event = nullptr;
    CmEvent *no_event = CM_NO_EVENT;
    cm_result_check(cmd_queue->EnqueueCopyGPUToGPU(dst_surf_argb, src_surf_argb, CM_FASTCOPY_OPTION_NONBLOCKING, no_event));
    cm_result_check(cmd_queue->EnqueueCopyGPUToGPU(dst_surf_nv12, src_surf_nv12, CM_FASTCOPY_OPTION_NONBLOCKING, sync_event));

    // Wait till the gpu copy task finished
    cm_result_check(sync_event->WaitForTaskFinished());

    // Destroys the CmEvent.
    // CmEvent must be destroyed by the user explicitly.
    cm_result_check(cmd_queue->DestroyEvent(sync_event));

    // Read the target surfaces
    unsigned char *dst_data_argb = (unsigned char *)malloc(width * height * 4);
    unsigned char *dst_data_nv12 = (unsigned char *)malloc(width * height * 3/2);
    cm_result_check(dst_surf_argb->ReadSurface(dst_data_argb, nullptr));
    cm_result_check(dst_surf_nv12->ReadSurface(dst_data_nv12, nullptr));

    // Check the results
    // Check ARGB
    for (unsigned int i = 0; i < width * height * 4; i++)
    {
        if (src_data_argb[i] != dst_data_argb[i])
        {
            std::cerr << "Error: Data mismatch in ARGB surface at position " << i << "." << std::endl;
            return -1;
        }
    }
    std::cout << "ARGB data copying from GPU to GPU: PASS" << std::endl;
    // Check NV12
    for (unsigned int i = 0; i < width * height * 3/2; i++)
    {
        if (src_data_nv12[i] != dst_data_nv12[i])
        {
            std::cerr << "Error: Data mismatch in NV12 surface at position " << i << "." << std::endl;
            return -1;
        }
    }
    std::cout << "NV12 data copying from GPU to GPU: PASS" << std::endl;

    // free resources
    free(dst_data_argb);
    free(dst_data_nv12);
    free(src_data_argb);
    free(src_data_nv12);

    return 0;
}

// Copy data from system memory to system memory
int CopyFromCPUToCPU(CmDevice *device, CmQueue *cmd_queue, unsigned int size)
{
    // Allocate source buffer and destination buffer, both should be 16-aligned
    unsigned char *src_buf = (unsigned char *)CM_ALIGNED_MALLOC(size, 16);
    unsigned char *dst_buf = (unsigned char *)CM_ALIGNED_MALLOC(size, 16);

    // Fill the source buffer with random data
    for (unsigned int i = 0; i < size; i++)
    {
        int temp = rand();
        src_buf[i] = temp & 0x000000FF;
    }

    // Init the destination buffer with 0
    memset(dst_buf, 0, size);

    // Launches the gpu copy task on the GPU.
    // An event, "sync_event", is created to track the status of the task.
    CmEvent *sync_event = nullptr;
    cm_result_check(cmd_queue->EnqueueCopyCPUToCPU(dst_buf, src_buf, size, CM_FASTCOPY_OPTION_NONBLOCKING, sync_event));

    // Wait till the gpu copy task finished
    cm_result_check(sync_event->WaitForTaskFinished());

    // Destroys the CmEvent.
    // CmEvent must be destroyed by the user explicitly.
    cm_result_check(cmd_queue->DestroyEvent(sync_event));

    // Check the results
    // Check ARGB
    for (unsigned int i = 0; i < size; i++)
    {
        if (src_buf[i] != dst_buf[i])
        {
            std::cerr << "Error: Data mismatch in Buffer at position " << i << "." << std::endl;
            return -1;
        }
    }
    std::cout << "Data copying from CPU to GPU: PASS" << std::endl;

    // free resources
    CM_ALIGNED_FREE(src_buf);
    CM_ALIGNED_FREE(dst_buf);

    return 0;
}

int main()
{
    // Creates a CmDevice from scratch.
    // Param device: pointer to the CmDevice object.
    // Param version: CM API version supported by the runtime library.
    CmDevice *device = nullptr;
    unsigned int version = 0;
    cm_result_check(::CreateCmDevice(device, version));

    // Creates a task queue.
    // The CmQueue is an in-order queue. Tasks get executed according to the
    // order they are enqueued. The next task does not start execution until the
    // current task finishes.
    CmQueue *cmd_queue = nullptr;
    cm_result_check(device->CreateQueue(cmd_queue));

    // Copy a 1280x720 region from a 1920x1080 container
    // From graphics memory to system memory
    CopyFromGPUToCPU(device, cmd_queue, 1920, 1080, 1280, 720);

    // Copy a 1280x720 region from a 1920x1080 container
    // From system memory to graphics memory
    CopyFromCPUToGPU(device, cmd_queue, 1920, 1080, 1280, 720);

    // Copy 1920x1080 data from graphics memory to graphics memory
    CopyFromGPUToGPU(device, cmd_queue, 1920, 1080);

    // Copy 1920x1080 data from system memory to system memory
    CopyFromCPUToCPU(device, cmd_queue, 1920*1080*4);

    // Destroys the CmDevice.
    // Also destroys surfaces, kernels, tasks, thread spaces, and queues that
    // were created using this device instance that have not explicitly been
    // destroyed by calling the respective destroy functions.
    cm_result_check(::DestroyCmDevice(device));

    return 0;
}
