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

#ifndef MDF_BASE_H
#define MDF_BASE_H

#include "cm_rt.h"

#define R 0
#define G 1
#define B 2

#define FILTER_DILATE 0
#define FILTER_ERODE 1
/*
struct float4
{
  float x, y, z, w;
};

struct float3
{
  float x, y, z;
};

struct uchar4
{
  unsigned char x, y, z, w;
};
*/
/////////////////////////////////////////
class MDF_Base {
public:
  MDF_Base();
  virtual ~MDF_Base();

  UINT Version; // MDF version

  // Set in InitGPU()
  CmDevice *pCmDev;
  FILE *pISA;
  void *pCommonISACode;
  CmProgram *pCmProgram;
  CmKernel *pCmKernel;

  // Set in EnqueueFunc()
  CmThreadSpace *pThrdSpace;
  CmEvent *pEvent;
  CmQueue *pCmQueue;
  CmTask *pKernelArray;

  unsigned int GpuPlatform;
  unsigned int GtPlatform;
  unsigned int GpuMinFreq;
  unsigned int GpuMaxFreq;
  unsigned int GpuCurFreq;
  unsigned int HWThreadCount;
  unsigned int Surface2DCount;
  unsigned int SurfaceCountPerKernel;
  unsigned int SamplerCount;
  unsigned int SamplerCountPerKernel;
  unsigned int KernelCountPerTask;
  unsigned int ArgCountPerKernel;
  unsigned int ArgSizePerKernel;

  ////////////////////////////////////////////////
  // Init and alloc functions
  int InitGPU(char *isa_file);

  int AllocGPUSurface(int FrameWidth, int FrameHeight, CM_SURFACE_FORMAT format,
                      CmSurface2D **pSurf, SurfaceIndex **pIndex);
  int AllocGPUSurfaceUP(int FrameWidth, int FrameHeight,
                        CM_SURFACE_FORMAT format, unsigned char *pBuffer,
                        CmSurface2DUP **pSurf, SurfaceIndex **pIndex);
  int AllocGPUBuffer(unsigned int size, CmBuffer **pBuffer,
                     SurfaceIndex **pIndex);
  int AllocGPUBufferUP(unsigned int size, unsigned char *pSysMem,
                       CmBufferUP **pBuffer, SurfaceIndex **pIndex);

  void *AlignedMalloc(unsigned int size, int alignment);
  void AlignedFree(void *pBuffer);

#ifdef WIN32

#ifdef CM_DX9
  int ImportDX9Surface(IDirect3DSurface9 *pD3DSurf, CmSurface2D **pSurf,
                       SurfaceIndex **pIndex);
#endif

#ifdef CM_DX11
  int MDF_Base::ImportDX11Surface(ID3D11Texture2D *pD3DSurf,
                                  CmSurface2D **pSurf, SurfaceIndex **pIndex);
#endif

#endif

  int AllocatGPUSampler(CmSurface2D *pInputSurf, SurfaceIndex **pSurfIndex,
                        SamplerIndex **pSamplerIndex);
  int AllocatGPUSamplerUP(CmSurface2DUP *pInputSurfUP,
                          SurfaceIndex **pSurfIndex,
                          SamplerIndex **pSamplerIndex);

  // MDF 4.0 and up
  // int MDF_Base::AllocatGPUSamplerUP(CmSurface2DUP * pInputSurfUP,
  // SurfaceIndex ** pSurfIndex, SamplerIndex ** pSamplerIndex );

  ////////////////////////////////////////////////
  // Common enqueue functions
  int EnqueueSobel(int FrameWidth, int FrameHeight, SurfaceIndex *pInputIndex,
                   SurfaceIndex *pOutputIndex);

  int EnqueueScale(int FrameWidth, int FrameHeight, SamplerIndex *pSamplerIndex,
                   SurfaceIndex *pInputIndex, SurfaceIndex *pOutputIndex,
                   float fScalingFactor);

  int EnqueueRGBToNV12(int FrameWidth, int FrameHeight,
                       SurfaceIndex *pRGBXIndex, SurfaceIndex *pNV12Index);

  int Enqueue_NV12toYUY2(SurfaceIndex *pInputIndex, SurfaceIndex *pOutputIndex,
                         int FrameWidth, int FrameHeight);

  int Enqueue_ColorToGray(SurfaceIndex *pIndex, SurfaceIndex *pIndex2,
                          int FrameWidth, int FrameHeight, int InputFormat);

  int EnqueueMorphologicalFilter(SurfaceIndex *pInputIndex,
                                 SurfaceIndex *pOutputIndex, int FrameWidth,
                                 int FrameHeight, int FilterType);

  int EnqueueBinaryMedian(SurfaceIndex *pInputIndex, SurfaceIndex *pOutputIndex,
                          SurfaceIndex *pMaskIndex, int FrameWidth,
                          int FrameHeight, int winSize, bool bMaskOn);

  int EnqueueMergeRGBA(int FrameWidth, int FrameHeight, SurfaceIndex *pRGBIndex,
                       SurfaceIndex *pAlphaIndex, SurfaceIndex *pOutputIndex);

  int Enqueue_GPU_memcpy(SurfaceIndex *pInputIndex, SurfaceIndex *pOutputIndex,
                         int FrameWidth, int FrameHeight);
  int Enqueue_NULL_kernel(SurfaceIndex *pInputIndex, SurfaceIndex *pOutputIndex,
                          int FrameWidth, int FrameHeight);
  int Enqueue_GPU_memcpy_1D(SurfaceIndex *pInputIndex,
                            SurfaceIndex *pOutputIndex, unsigned int size);
  int Enqueue_GPU_memcpy_Scattered(SurfaceIndex *pInputIndex,
                                   SurfaceIndex *pOutputIndex,
                                   unsigned int size);

  int Enqueue_FlipX(SurfaceIndex *pInIndex, SurfaceIndex *pOutIndex,
                    int FrameWidth, int FrameHeight);
  int Enqueue_FlipX(SurfaceIndex *pInIndex, int FrameWidth, int FrameHeight);
  int Enqueue_RGBA2BGRA(SurfaceIndex *pInIndex, int FrameWidth,
                        int FrameHeight);
};

#endif
