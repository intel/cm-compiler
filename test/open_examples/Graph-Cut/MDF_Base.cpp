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

#ifdef WIN32
#include "stdafx.h"
#endif

#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>

//#include <cm/cm.h>

#include "MDF_Base.h"

/////////////////////////////////////////////////////////////////////////////////////////////
MDF_Base::MDF_Base() {
  // Create a CM Device
  unsigned int Option = 1; // Disable scratch space, no need for that, no spill
                           // happens in real sense kernels.
  int result;

  pCmProgram = 0;
  pCmKernel = 0;
  pThrdSpace = 0;

  pEvent = 0;
  pCmQueue = 0;
  pKernelArray = 0;

#ifdef CMRT_EMU
  result = ::CreateCmDevice(pCmDev, Version);
#else
  result = ::CreateCmDeviceEx(pCmDev, Version, NULL, Option);
#endif

  if (result != CM_SUCCESS) {
    printf("CmDevice creation error, error code %d.\n", result);
    return;
  }
  if (Version < CM_1_0) {
    printf(" The runtime API version is later than runtime DLL version.\n");
    return;
  } else {
    printf("\nMDF Runtime version: %.2f\n", ((float)Version) / 100);
  }

  InitGPU("Graph_Cut_genx.isa");

  size_t CapValueSize = 4;
  pCmDev->GetCaps(CAP_GPU_PLATFORM, CapValueSize, &GpuPlatform);
  pCmDev->GetCaps(CAP_GT_PLATFORM, CapValueSize, &GtPlatform);
  pCmDev->GetCaps(CAP_MIN_FREQUENCY, CapValueSize, &GpuMinFreq);
  pCmDev->GetCaps(CAP_MAX_FREQUENCY, CapValueSize, &GpuMaxFreq);
  pCmDev->GetCaps(CAP_GPU_CURRENT_FREQUENCY, CapValueSize, &GpuCurFreq);

  /*
      pCmDev->GetCaps(CAP_HW_THREAD_COUNT, CapValueSize, &HWThreadCount);
      pCmDev->GetCaps(CAP_KERNEL_COUNT_PER_TASK, CapValueSize,
     &KernelCountPerTask); pCmDev->GetCaps(CAP_SURFACE2D_COUNT, CapValueSize,
     &Surface2DCount); pCmDev->GetCaps(CAP_SURFACE_COUNT_PER_KERNEL,
     CapValueSize, &SurfaceCountPerKernel); pCmDev->GetCaps(CAP_SAMPLER_COUNT,
     CapValueSize, &SamplerCount); pCmDev->GetCaps(CAP_SAMPLER_COUNT_PER_KERNEL,
     CapValueSize, &SamplerCountPerKernel);
      pCmDev->GetCaps(CAP_ARG_COUNT_PER_KERNEL, CapValueSize,
     &ArgCountPerKernel); pCmDev->GetCaps(CAP_ARG_SIZE_PER_KERNEL, CapValueSize,
     &ArgSizePerKernel);
   */

  printf("Platform: ");
  if (GpuPlatform == PLATFORM_INTEL_SNB)
    printf("SNB ");
  else if (GpuPlatform == PLATFORM_INTEL_IVB)
    printf("IVB ");
  else if (GpuPlatform == PLATFORM_INTEL_HSW)
    printf("HSW ");
  else if (GpuPlatform == PLATFORM_INTEL_BDW)
    printf("BDW ");
  else if (GpuPlatform == PLATFORM_INTEL_VLV)
    printf("VLV ");
  else if (GpuPlatform == PLATFORM_INTEL_CHV)
    printf("CHV ");
#ifdef _MDF5
  else if (GpuPlatform == PLATFORM_INTEL_SKL)
    printf("SKL ");
  else if (GpuPlatform == PLATFORM_INTEL_BXT)
    printf("BXT ");
#endif
#ifdef _MDF6
  else if (GpuPlatform == PLATFORM_INTEL_SKL)
    printf("SKL ");
  else if (GpuPlatform == PLATFORM_INTEL_BXT)
    printf("BXT ");
  else if (GpuPlatform == PLATFORM_INTEL_KBL)
    printf("KBL ");
#endif

  if (GtPlatform == PLATFORM_INTEL_GT1)
    printf("GT1\n");
  else if (GtPlatform == PLATFORM_INTEL_GT2)
    printf("GT2\n");
  else if (GtPlatform == PLATFORM_INTEL_GT3)
    printf("GT3\n");
  else if (GtPlatform == PLATFORM_INTEL_GT4)
    printf("GT4\n");

  printf("GPU min frequency: %d MHz\n", GpuMinFreq);
  printf("GPU max frequency: %d MHz\n", GpuMaxFreq);
/*
    printf("GPU current frequency: %d MHz\n\n", GpuCurFreq);
    printf("Max HW thread count: %d\n", HWThreadCount);
    printf("Kernel count per task: %d\n", KernelCountPerTask);
    printf("Max Surface2D count: %d\n", Surface2DCount);
    printf("Max surface count per kernel: %d\n", SurfaceCountPerKernel);
    printf("Sampler count: %d\n", SamplerCount);
    printf("Sampler count per kernel: %d\n", SamplerCountPerKernel);
    printf("Arg count per kernel: %d\n", ArgCountPerKernel);
    printf("Arg size per kernel: %d\n\n", ArgSizePerKernel);
*/
#ifdef _DEBUG
  pCmDev->InitPrintBuffer();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////
MDF_Base::~MDF_Base() {
#ifdef _DEBUG
  pCmDev->FlushPrintBuffer();
#endif

  free(pCommonISACode);

  if (pCmProgram) {
    pCmDev->DestroyProgram(pCmProgram);
    pCmProgram = 0;
  }
  if (pThrdSpace) {
    pCmDev->DestroyThreadSpace(pThrdSpace);
    pThrdSpace = 0;
  }
  if (pCmKernel) {
    pCmDev->DestroyKernel(pCmKernel);
    pCmKernel = 0;
  }

  if (pCmDev) {
    int rtn = ::DestroyCmDevice(pCmDev);
    pCmDev = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::InitGPU(char *isa_file) {
  int result;

  // Open isa file
  pISA = fopen(isa_file, "rb");
  if (pISA == NULL) {
    printf("isa file: %s\n", isa_file);
    perror("Error openning isa file.");
    return -1;
  }

  // Find the kernel code size
  fseek(pISA, 0, SEEK_END);
  int codeSize = ftell(pISA);
  rewind(pISA);

  if (codeSize == 0) {
    perror("Kernel code size is 0");
    return -1;
  }

  // Allocate memory to hold isa kernel from kernel file
  pCommonISACode = (BYTE *)malloc(codeSize);
  if (!pCommonISACode) {
    perror("Failed allocate memeory for kernel");
    return -1;
  }

  // Read kernel file
  if (fread(pCommonISACode, 1, codeSize, pISA) != codeSize) {
    perror("Error reading isa file");
    return -1;
  }
  fclose(pISA);

  // Load kernel into CmProgram obj
  pCmProgram = NULL;
  result = pCmDev->LoadProgram(pCommonISACode, codeSize, pCmProgram);
  if (result != CM_SUCCESS) {
    perror("CM LoadProgram error");
    return -1;
  }

  // Create a task queue
  pCmQueue = NULL;
  result = pCmDev->CreateQueue(pCmQueue);
  if (result != CM_SUCCESS) {
    perror("CM CreateQueue error");
    return -1;
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::AllocGPUSurface(int FrameWidth, int FrameHeight,
                              CM_SURFACE_FORMAT format, CmSurface2D **pSurf,
                              SurfaceIndex **pIndex) {
  int result;
  CmSurface2D *pSurf2 = NULL;
  SurfaceIndex *pIndex2 = NULL;

  // Create a 2D surface
  result = pCmDev->CreateSurface2D(FrameWidth, FrameHeight, format, pSurf2);
  if (result != CM_SUCCESS) {
    perror("CM CreateSurface2D error");
    return -1;
  }
  pSurf2->GetIndex(pIndex2);

  *pSurf = pSurf2;
  *pIndex = pIndex2;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::AllocGPUSurfaceUP(int FrameWidth, int FrameHeight,
                                CM_SURFACE_FORMAT format,
                                unsigned char *pBuffer, CmSurface2DUP **pSurf,
                                SurfaceIndex **pIndex) {
  int result;
  CmSurface2DUP *pSurf2 = NULL;
  SurfaceIndex *pIndex2 = NULL;

  // Create a 2D surface
  result = pCmDev->CreateSurface2DUP(FrameWidth, FrameHeight, format, pBuffer,
                                     pSurf2);

  if (result != CM_SUCCESS) {
    perror("CM CreateSurface2DUP error");
    return -1;
  }
  pSurf2->GetIndex(pIndex2);

  *pSurf = pSurf2;
  *pIndex = pIndex2;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::AllocGPUBuffer(unsigned int size, CmBuffer **pBuffer,
                             SurfaceIndex **pIndex) {
  int result;
  CmBuffer *pBuffer2 = NULL;
  SurfaceIndex *pIndex2 = NULL;

  // Create a 1D buffer
  result = pCmDev->CreateBuffer(size, pBuffer2);
  if (result != CM_SUCCESS) {
    perror("CM CreateSurface2D error");
    return -1;
  }
  pBuffer2->GetIndex(pIndex2);

  *pBuffer = pBuffer2;
  *pIndex = pIndex2;
  return 0;
}

////////////////////////////////////////////////////////////////////////
int MDF_Base::AllocGPUBufferUP(unsigned int size, unsigned char *pSysMem,
                               CmBufferUP **pBuffer, SurfaceIndex **pIndex) {
  int result;
  CmBufferUP *pBuffer2 = NULL;
  SurfaceIndex *pIndex2 = NULL;

  // Create 1D UP buffer
  result = pCmDev->CreateBufferUP(size, pSysMem, pBuffer2);

  if (result != CM_SUCCESS) {
    perror("CM CreateSurface2D error");
    return -1;
  }
  pBuffer2->GetIndex(pIndex2);

  *pBuffer = pBuffer2;
  *pIndex = pIndex2;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////
void *MDF_Base::AlignedMalloc(unsigned int size, int alignment) {
  void *pBuffer;

#ifdef WIN32
  pBuffer = (void *)_aligned_malloc(size, alignment);
  if (pBuffer == NULL) {
    perror("MS _aligned_malloc error");
    pBuffer = 0;
  }
#elif ANDROID
  pBuffer = (void *)memalign(size, alignment);
  if (pBuffer == NULL) {
    perror("ANDROID memalign error");
    pBuffer = 0;
  }
#else
  perror("Unknown OS flag error");
  pBuffer = 0;
#endif

  return pBuffer;
}

////////////////////////////////////////////////////////////
void MDF_Base::AlignedFree(void *pBuffer) {
  if (pBuffer != 0) {
#ifdef WIN32
    _aligned_free(pBuffer);
#elif ANDROID
    free(pBuffer);
#endif
    pBuffer = 0;
  }
}

#ifdef WIN32

#ifdef CM_DX9
/////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::ImportDX9Surface(IDirect3DSurface9 *pD3DSurf, CmSurface2D **pSurf,
                               SurfaceIndex **pIndex) {
  int result;
  CmSurface2D *pSurf2 = NULL;
  SurfaceIndex *pIndex2 = NULL;

  // Bind to a D3D surface
  result = pCmDev->CreateSurface2D(pD3DSurf, pSurf2);

  if (result != CM_SUCCESS) {
    perror("CM CreateSurface2D error");
    return -1;
  }
  pSurf2->GetIndex(pIndex2);

  *pSurf = pSurf2;
  *pIndex = pIndex2;
  return 0;
}
#endif

#ifdef CM_DX11
////////////////////////////////////////////////////////////////////////
int MDF_Base::ImportDX11Surface(ID3D11Texture2D *pD3DSurf, CmSurface2D **pSurf,
                                SurfaceIndex **pIndex) {
  int result;
  CmSurface2D *pSurf2 = NULL;
  SurfaceIndex *pIndex2 = NULL;

  // Bind to a D3D surface
  result = pCmDev->CreateSurface2D(pD3DSurf, pSurf2);

  if (result != CM_SUCCESS) {
    perror("CM CreateSurface2D error");
    return -1;
  }
  pSurf2->GetIndex(pIndex2);

  *pSurf = pSurf2;
  *pIndex = pIndex2;
  return 0;
}
#endif

#endif

//////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::AllocatGPUSampler(CmSurface2D *pInputSurf,
                                SurfaceIndex **pSurfIndex,
                                SamplerIndex **pSamplerIndex) {
  int result;
  SurfaceIndex *pSurfIndex2 = NULL;
  SamplerIndex *pSamplerIndex2 = NULL;

  // Define sampler state
  CM_SAMPLER_STATE SamplerState;
  SamplerState.minFilterType = SamplerState.magFilterType =
      CM_TEXTURE_FILTER_TYPE_POINT; // CM_TEXTURE_FILTER_TYPE_LINEAR;
  SamplerState.addressU = SamplerState.addressV = SamplerState.addressW =
      CM_TEXTURE_ADDRESS_CLAMP;

  // Create sampler state
  CmSampler *pSampler = NULL;
  result = pCmDev->CreateSampler(SamplerState, pSampler);
  if (result != CM_SUCCESS) {
    printf("CM CreateSampler error.");
    return -1;
  }

  // Bind the actual 2D surface with a virtual sampler surface index
  // Get surface index for input surface used by sampler
  pCmDev->CreateSamplerSurface2D(pInputSurf, pSurfIndex2); // Since CM2.4

  // Get sampler index for input surface
  pSampler->GetIndex(pSamplerIndex2);

  *pSurfIndex = pSurfIndex2;
  *pSamplerIndex = pSamplerIndex2;
  return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::AllocatGPUSamplerUP(CmSurface2DUP * pInputSurfUP, SurfaceIndex **
pSurfIndex, SamplerIndex ** pSamplerIndex )
{
    int result;
    SurfaceIndex * pSurfIndex2 = NULL;
    SamplerIndex * pSamplerIndex2 = NULL;

    // Define sampler state
    CM_SAMPLER_STATE SamplerState;
    SamplerState.minFilterType = SamplerState.magFilterType =
CM_TEXTURE_FILTER_TYPE_POINT; // CM_TEXTURE_FILTER_TYPE_LINEAR;
    SamplerState.addressU = SamplerState.addressV = SamplerState.addressW =
CM_TEXTURE_ADDRESS_CLAMP;

    // Create sampler state
    CmSampler * pSampler = NULL;
    result = pCmDev->CreateSampler(SamplerState, pSampler);
    if (result != CM_SUCCESS ) {
        printf("CM CreateSampler error.");
        return -1;
    }

    // Bind the actual 2D surface with a virtual sampler surface index
    // Get surface index for input surface used by sampler
    result = pCmDev->CreateSamplerSurface2DUP(pInputSurfUP, pSurfIndex2);

    // Get sampler index for input surface
    pSampler->GetIndex( pSamplerIndex2 );

    *pSurfIndex = pSurfIndex2;
    *pSamplerIndex = pSamplerIndex2;
    return 0;
}
*/
