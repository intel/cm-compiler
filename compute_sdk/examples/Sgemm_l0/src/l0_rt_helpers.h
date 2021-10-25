/*========================== begin_copyright_notice ============================

Copyright (C) 2017-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// vim:ts=2:sw=2:et:

#pragma once

#ifndef L0_RT_HELPERS_H
#define L0_RT_HELPERS_H

#include <cstdio>
#include <cstdlib>
#include <assert.h>
#include <chrono>
#include <initializer_list>
#include <limits>
#include <tuple>

#include <level_zero/ze_api.h>

#include "cm_utils.h"

#define L0_SAFE_CALL(call)                                                     \
  {                                                                            \
    auto status = (call);                                                      \
    if (status != 0) {                                                         \
      fprintf(stderr, "%s:%d: L0 error %d\n", __FILE__, __LINE__,              \
              (int)status);                                                    \
      exit(1);                                                                 \
    }                                                                          \
  }

inline double getTimeStamp()
{
  namespace sc = std::chrono;
  sc::system_clock::duration d = sc::system_clock::now().time_since_epoch();
  sc::seconds s = sc::duration_cast<sc::seconds>(d);
  return s.count() + (sc::duration_cast<sc::microseconds>(d - s).count()) / 1e6;
}

inline auto findDevice()
{
    L0_SAFE_CALL(zeInit(ZE_INIT_FLAG_GPU_ONLY));

    // Discover all the driver instances
    uint32_t driverCount = 0;
    L0_SAFE_CALL(zeDriverGet(&driverCount, nullptr));
    fprintf(stderr, "driverCount = %d\n", (int)driverCount);

    ze_driver_handle_t *allDrivers =
        (ze_driver_handle_t *)malloc(driverCount * sizeof(ze_driver_handle_t));
    L0_SAFE_CALL(zeDriverGet(&driverCount, allDrivers));

    ze_driver_handle_t hDriver = nullptr;
    ze_device_handle_t hDevice = nullptr;
    for (uint32_t i = 0; i < driverCount; ++i) {
        uint32_t deviceCount = 0;
        hDriver = allDrivers[i];
        L0_SAFE_CALL(zeDeviceGet(hDriver, &deviceCount, nullptr));
        fprintf(stderr, "driver = %d: deviceCount= %d\n", (int)i, (int)deviceCount);
        ze_device_handle_t *allDevices =
            (ze_device_handle_t *)malloc(deviceCount * sizeof(ze_device_handle_t));
        L0_SAFE_CALL(zeDeviceGet(hDriver, &deviceCount, allDevices));
        for (uint32_t d = 0; d < deviceCount; ++d) {
            ze_device_properties_t device_properties;
            L0_SAFE_CALL(zeDeviceGetProperties(allDevices[d], &device_properties));
            if (ZE_DEVICE_TYPE_GPU == device_properties.type) {
                hDevice = allDevices[d];
                break;
            }
        }
        free(allDevices);
        if (nullptr != hDevice) {
            break;
        }
    }
    free(allDrivers);
    assert(hDriver);
    assert(hDevice);

    ze_context_desc_t contextDesc = {ZE_STRUCTURE_TYPE_CONTEXT_DESC, nullptr, 0};
    ze_context_handle_t hContext = nullptr;
    L0_SAFE_CALL(zeContextCreate(hDriver, &contextDesc, &hContext));

    return std::make_tuple(hDriver, hDevice, hContext);
}

inline auto findDriverAndDevice() {
  return findDevice();
}

inline ze_command_queue_handle_t createCommandQueue(ze_context_handle_t hContext, ze_device_handle_t hDevice)
{
    // Create a command queue
    ze_command_queue_desc_t commandQueueDesc = {
        ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC, nullptr, 0, 0, 0,
        ZE_COMMAND_QUEUE_MODE_DEFAULT, ZE_COMMAND_QUEUE_PRIORITY_NORMAL};
    ze_command_queue_handle_t hCommandQueue;
    L0_SAFE_CALL(zeCommandQueueCreate(hContext, hDevice, &commandQueueDesc, &hCommandQueue));
    return hCommandQueue;
}

inline ze_command_list_handle_t createCommandList(ze_context_handle_t hContext, ze_device_handle_t hDevice)
{
    // Create a command list
    ze_command_list_desc_t commandListDesc = {
        ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC, 0, 0, 0};
    ze_command_list_handle_t hCommandList;
    L0_SAFE_CALL(zeCommandListCreate(hContext, hDevice, &commandListDesc, &hCommandList));
    return hCommandList;
}

inline auto createCommandQueueAndList(ze_context_handle_t hContext, ze_device_handle_t hDevice)
{
    return std::tuple<ze_command_queue_handle_t, ze_command_list_handle_t>{
        createCommandQueue(hContext, hDevice), createCommandList(hContext, hDevice)};
}

inline ze_command_list_handle_t createImmCommandList(ze_context_handle_t hContext, ze_device_handle_t hDevice)
{
    ze_command_queue_desc_t desc = {
        ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC, nullptr, 0, 0,
        0,
        ZE_COMMAND_QUEUE_MODE_DEFAULT,
        ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
    };
    ze_command_list_handle_t hCommandList = nullptr;
    L0_SAFE_CALL(zeCommandListCreateImmediate(hContext, hDevice, &desc, &hCommandList));
    return hCommandList;
}

inline ze_image_handle_t createImage2D(ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_command_list_handle_t hCommandList, ze_image_format_t &fmt,
    unsigned int width, unsigned int height, const void *pData = nullptr)
{
    //printf("createImage: %u x %u\n", width, height);
    ze_image_handle_t hImage;
    ze_image_desc_t desc = {
        ZE_STRUCTURE_TYPE_IMAGE_DESC, nullptr,
        pData ? 0u : ZE_IMAGE_FLAG_KERNEL_WRITE,
        ZE_IMAGE_TYPE_2D,
        fmt, width, height, 0, 0, 0};
    L0_SAFE_CALL(zeImageCreate(hContext, hDevice, &desc, &hImage));
    if (pData)
        L0_SAFE_CALL(zeCommandListAppendImageCopyFromMemory(hCommandList, hImage,
                                                            pData, nullptr, nullptr, 0, nullptr));
    return hImage;
}

inline ze_image_handle_t createImage3D(ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_command_list_handle_t hCommandList, ze_image_format_t &fmt,
    unsigned int width, unsigned int height, unsigned int depth, const void *pData = nullptr)
{
    ze_image_handle_t hImage;
    ze_image_desc_t desc = {
        ZE_STRUCTURE_TYPE_IMAGE_DESC, nullptr,
        pData ? 0u : ZE_IMAGE_FLAG_KERNEL_WRITE,
        ZE_IMAGE_TYPE_3D,
        fmt, width, height, depth, 0, 0};
    L0_SAFE_CALL(zeImageCreate(hContext, hDevice, &desc, &hImage));
    if (pData)
        L0_SAFE_CALL(zeCommandListAppendImageCopyFromMemory(hCommandList, hImage,
                                                            pData, nullptr, nullptr, 0, nullptr));
    return hImage;
}

inline ze_sampler_handle_t createSampler(ze_context_handle_t hContext, ze_device_handle_t hDevice, const ze_sampler_desc_t* kdesc)
{
    ze_sampler_handle_t hSampler;
    L0_SAFE_CALL(zeSamplerCreate(hContext, hDevice, kdesc, &hSampler));
    return hSampler;
}

inline void destroy(ze_context_handle_t hContext)
{
#ifndef CMRT_EMU
    L0_SAFE_CALL(zeContextDestroy(hContext));
#endif
}

inline void destroy(ze_command_list_handle_t hCommandList)
{
#ifndef CMRT_EMU
    L0_SAFE_CALL(zeCommandListDestroy(hCommandList));
#endif
}

inline void destroy(ze_command_queue_handle_t hCommandQueue)
{
#ifndef CMRT_EMU
    L0_SAFE_CALL(zeCommandQueueDestroy(hCommandQueue));
#endif
}

inline void destroy(ze_module_handle_t hModule)
{
#ifndef CMRT_EMU
    L0_SAFE_CALL(zeModuleDestroy(hModule));
#endif
}
inline void destroy(ze_image_handle_t img)
{
    L0_SAFE_CALL(zeImageDestroy(img));
}

inline void destroy(ze_sampler_handle_t sampler)
{
    L0_SAFE_CALL(zeSamplerDestroy(sampler));
}

inline void destroy(ze_event_pool_handle_t pool)
{
#ifndef CMRT_EMU
    L0_SAFE_CALL(zeEventPoolDestroy(pool));
#endif
}

inline void* allocDeviceMemory(ze_context_handle_t hContext, ze_device_handle_t hDevice,
    size_t size, size_t alignment = 4096)
{
    ze_device_mem_alloc_desc_t device_desc = {
        ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC, nullptr,
        0,
        0
    };
    void *ptr = nullptr;
    L0_SAFE_CALL(zeMemAllocDevice(hContext, &device_desc, size,
        alignment, hDevice, &ptr));
    return ptr;
}

inline void* allocSharedMemory(ze_context_handle_t hContext, ze_device_handle_t hDevice,
    size_t size, size_t alignment = 4096)
{
    ze_device_mem_alloc_desc_t device_desc = {
      ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC, nullptr, 0, 0
    };
    ze_host_mem_alloc_desc_t host_desc = {
      ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC, nullptr, 0
    };
    void *ptr = nullptr;
    L0_SAFE_CALL(zeMemAllocShared(hContext, &device_desc, &host_desc, size,
        alignment, hDevice, &ptr));
    return ptr;
}

inline void freeMemory(ze_context_handle_t hContext, void *ptr)
{
    L0_SAFE_CALL(zeMemFree(hContext, ptr));
#ifdef TEST_DEBUG
    std::cout << __func__ << " is OK\n";
#endif
}

inline ze_module_handle_t createModule(ze_context_handle_t hContext, ze_device_handle_t hDevice, const uint8_t* krn, size_t krn_size)
{
    fprintf(stderr, "kernel size: %g KB\n", krn_size / 1024.0);

    ze_module_desc_t moduleDesc = {
        ZE_STRUCTURE_TYPE_MODULE_DESC, nullptr, //
#if defined(CM_COMPILE_SPIRV) || defined(OCLOC_SPIRV)
        ZE_MODULE_FORMAT_IL_SPIRV, //
#else // !CM_COMPILE_SPIRV
        ZE_MODULE_FORMAT_NATIVE, //
#endif // CM_COMPILE_SPIRV
        krn_size,  //
        krn, //
        "-vc-codegen",
        nullptr
    };

    ze_module_handle_t hModule;
    L0_SAFE_CALL(zeModuleCreate(hContext, hDevice, &moduleDesc, &hModule, nullptr));
    return hModule;
}


inline ze_module_handle_t createModule(ze_context_handle_t hContext, ze_device_handle_t hDevice, const char *fn)
{
    const auto krn = cm_utils::read_binary_file<uint8_t>(fn);
    auto hModule = createModule(hContext, hDevice, krn.data(), krn.size());
    fprintf(stderr, "create module from \"%s\"\n", fn);
    return hModule;
}

inline ze_kernel_handle_t createKernel(ze_module_handle_t hModule, const char* kname)
{
    ze_kernel_desc_t kernelDesc = {ZE_STRUCTURE_TYPE_KERNEL_DESC, nullptr, 0, kname};
    ze_kernel_handle_t hKernel;
    L0_SAFE_CALL(zeKernelCreate(hModule, &kernelDesc, &hKernel));
    //fprintf(stderr, "create kernel \"%s\"\n", kname);
    return hKernel;
}

inline ze_kernel_handle_t createKernel(ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const char *fn, const char* kname)
{
    ze_kernel_desc_t kernelDesc = {ZE_STRUCTURE_TYPE_KERNEL_DESC, nullptr, 0, kname};
    ze_kernel_handle_t hKernel;
    L0_SAFE_CALL(zeKernelCreate(createModule(hContext, hDevice, fn), &kernelDesc, &hKernel));
    //fprintf(stderr, "create kernel \"%s\"\n", kname);
    return hKernel;
}

// createKernel overload, which takes binary (or SPIRV) buffer (krn)
inline ze_kernel_handle_t createKernel(ze_context_handle_t hContext, ze_device_handle_t hDevice,
                                       const uint8_t* krn, size_t krn_size, const char* kname)
{
    ze_kernel_desc_t kernelDesc = {ZE_STRUCTURE_TYPE_KERNEL_DESC, nullptr, 0, kname};
    ze_kernel_handle_t hKernel;
    L0_SAFE_CALL(zeKernelCreate(createModule(hContext, hDevice, krn, krn_size), &kernelDesc, &hKernel));
    //fprintf(stderr, "create kernel \"%s\"\n", kname);
    return hKernel;
}

inline void appendBarrier(ze_command_list_handle_t hCommandList)
{
    L0_SAFE_CALL(zeCommandListAppendBarrier(hCommandList, nullptr, 0, nullptr));
}

inline void appendLaunchKernel(ze_command_list_handle_t hCommandList,
    ze_kernel_handle_t hKernel, ze_group_count_t *args,
    ze_event_handle_t hEvent = nullptr, unsigned numWait = 0,
    ze_event_handle_t *pWaitEvents = nullptr)
{
   L0_SAFE_CALL(zeCommandListAppendLaunchKernel(hCommandList, hKernel,
        args, hEvent, numWait, pWaitEvents));
}

inline void appendMemoryCopy(ze_command_list_handle_t hCommandList, void *dst,
    const void *src, size_t size, ze_event_handle_t hEvent = nullptr)
{
    L0_SAFE_CALL(zeCommandListAppendMemoryCopy(hCommandList, dst, src, size, hEvent, 0, nullptr));
}

inline void copyToMemory(ze_command_list_handle_t hCommandList, void *dst, ze_image_handle_t src,
    ze_event_handle_t hEvent = nullptr)
{
    L0_SAFE_CALL(zeCommandListAppendImageCopyToMemory(hCommandList, dst, src, nullptr, hEvent, 0, nullptr));
}

template<typename T>
void setArgById(ze_kernel_handle_t hKernel, unsigned id, T *t)
{
    L0_SAFE_CALL(zeKernelSetArgumentValue(hKernel, id, sizeof(T), t));
}

template<typename T, typename Func>
void setArg(ze_kernel_handle_t hKernel, Func get_id, T *t)
{
    setArgById(hKernel, get_id(), t);
}

template <typename ... Args>
inline void setKernelArgs(ze_kernel_handle_t hKernel, Args ... args)
{
    unsigned id = 0;
    auto get_id = [&](){ return id++;};
    (void)std::initializer_list<int>{(setArg(hKernel, get_id, args),0)...};
}

inline void execute(ze_command_queue_handle_t hCommandQueue,
    ze_command_list_handle_t &hCommandList)
{
    L0_SAFE_CALL(zeCommandListClose(hCommandList));
    L0_SAFE_CALL(zeCommandQueueExecuteCommandLists(hCommandQueue, 1,
        &hCommandList, nullptr));
    L0_SAFE_CALL(zeCommandQueueSynchronize(hCommandQueue,
        std::numeric_limits<uint32_t>::max()));
}

inline void reset(ze_command_list_handle_t hCommandList)
{
    L0_SAFE_CALL(zeCommandListReset(hCommandList));
}

inline void reset(ze_event_handle_t hEvent)
{
    L0_SAFE_CALL(zeEventHostReset(hEvent));
}

inline ze_event_pool_handle_t createPool(ze_context_handle_t hContext,
    ze_device_handle_t &hDevice, unsigned num = 1, unsigned count = 1)
{
    ze_event_pool_desc_t pool_desc = {
      ZE_STRUCTURE_TYPE_EVENT_POOL_DESC, nullptr,
      ZE_EVENT_POOL_FLAG_KERNEL_TIMESTAMP, count
    };

    ze_event_pool_handle_t hPool = nullptr;
    L0_SAFE_CALL(zeEventPoolCreate(hContext, &pool_desc, num, &hDevice, &hPool));
    return hPool;
}

inline ze_event_handle_t createEvent(ze_event_pool_handle_t hPool, unsigned index = 0)
{
    ze_event_desc_t desc = {ZE_STRUCTURE_TYPE_EVENT_DESC, nullptr, index, 0, 0};

    ze_event_handle_t hEvent = nullptr;
    L0_SAFE_CALL(zeEventCreate(hPool, &desc, &hEvent));
    return hEvent;
}

inline ze_event_handle_t createEvent(ze_context_handle_t hContext, ze_device_handle_t &hDevice)
{
    return createEvent(createPool(hContext, hDevice));
}

#endif //L0_RT_HELPERS_H
