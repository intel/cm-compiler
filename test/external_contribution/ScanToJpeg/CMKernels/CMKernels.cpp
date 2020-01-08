/*
 * Copyright (c) 2017, Intel Corporation
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
#include <assert.h>
#include "cm_rt.h"
#include "CMKernels.h"
#include "common/cm_rt_helpers.h"

#define BLOCK_WIDTH     8
#define BLOCK_HEIGHT    8

///////////////////////////////////////////////////////////////
// 
// KernelBase class
//

KernelBase::KernelBase(VADisplay vaDpy)
{

   m_pTask     = NULL;
   m_pCmQueue  = NULL;
   e_Pipeline  = NULL;
   m_vaDpy     = vaDpy;

}

int KernelBase::Init()
{
   // For testing only - collect platform information
	////////
   int nGPUPlatform = 0;
   int nGTPlatform = 0;
   int nGPUFrequency = 0;
   char* sGPUPlatform[] = {"Unk", "SNB", "IVB", "HSW", "BDW", "VLV", "CHV", "SKL", "BXT", "CNL", "KBL", "GLK"};
   char* sGT[] = {"Unk", "GT1", "GT2", "GT3", "GT4"};
   size_t size = 4;

   cm_result_check(m_pCmDev->GetCaps( CAP_GPU_PLATFORM, size, &nGPUPlatform ));
   cm_result_check(m_pCmDev->GetCaps( CAP_GT_PLATFORM, size, &nGTPlatform ));
   cm_result_check(m_pCmDev->GetCaps( CAP_GPU_CURRENT_FREQUENCY, size, &nGPUFrequency ));
   //printf("The test is running on Intel %s %s platform with frequency at %d MHz.\n", sGPUPlatform[nGPUPlatform], sGT[nGTPlatform], nGPUFrequency);


   // Create a task queue
   cm_result_check(m_pCmDev->CreateQueue(m_pCmQueue));

   // Create a CM task
   cm_result_check(m_pCmDev->CreateTask(m_pTask));

}

int KernelBase::CreateKernel(char *isaFile, char* kernelName, CmKernel *& pKernel)
{
   int         nKernelSize;
   uchar       *pCommonISA; // Common ISA kernel
   CmProgram   *pProgram;

   // Create kernel
   FILE* pISA = fopen(isaFile, "rb");
   if (pISA == NULL) {
      perror("Open File failed\n");
      return -1;
   }

   fseek (pISA, 0, SEEK_END);
   nKernelSize = ftell (pISA);
   rewind(pISA);

   if(nKernelSize == 0)
   {
      fclose(pISA);
      return -1;
   }

   pCommonISA = (uchar*) malloc(nKernelSize);
   if( !pCommonISA )
   {
      fclose(pISA);
      return -1;
   }

   if (fread(pCommonISA, 1, nKernelSize, pISA) != nKernelSize) {
      perror("Read fail\n");
      fclose(pISA);
      free(pCommonISA);
      return -1;
   }
   fclose(pISA);

   //cm_result_check(m_pCmDev->LoadProgram(pCommonISA, nKernelSize, pProgram, "nojitter"));
   cm_result_check(m_pCmDev->LoadProgram(pCommonISA, nKernelSize, pProgram));

   cm_result_check(m_pCmDev->CreateKernel(pProgram, kernelName, pKernel));

   free(pCommonISA);

   return CM_SUCCESS;
}

void KernelBase::AddKernel(CmTask *pTask, CmKernel *pKernel)
{
   // Add kernel to task
   cm_result_check(pTask->AddKernel(pKernel));
   cm_result_check(pTask->AddSync());
}

void KernelBase::GetSurface2DInfo(int width, int height, CM_SURFACE_FORMAT format,
        unsigned int & pitch, unsigned int & surface_size)
{
      m_pCmDev->GetSurface2DInfo(width, height, format, pitch, surface_size);
}

int KernelBase::ExecuteInternal()
{
   DWORD dwTimeOutMs = -1;
   UINT64 executionTime=0;

   e_Pipeline = NULL;

   if (m_pTask)
   {
      cm_result_check(m_pCmQueue->Enqueue(m_pTask, e_Pipeline));
      cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
   }

   return CM_SUCCESS;
}

void KernelBase::DestroyInternal()
{
   //FIXME, need to destroy surface also
   if (m_pTask)
   {
      cm_result_check(m_pCmQueue->DestroyEvent(e_Pipeline));
      cm_result_check(m_pCmDev->DestroyTask(m_pTask));
   }

   e_Pipeline = NULL;
}

///////////////////////////////////////////////////////////////
// 
// Rgb2YCbCr class
//

Rgb2YCbCr::Rgb2YCbCr(VADisplay vaDpy) : Scan2FilePipeline(),  KernelBase(vaDpy)
{
   UINT version = 0;
   // Create a CM Device
   cm_result_check(CreateCmDevice( m_pCmDev, version,vaDpy ));

   Init();

   CreateKernel(this->GetIsa(), this->GetKernelName(), m_pKernel);

   AddKernel(m_pTask, m_pKernel);

   float coefficients[9] = {0.299, 0.587, 0.114, -0.168736, -0.331264, 0.5, 0.5, -0.418688, -0.081312};
   std::copy(coefficients, coefficients + 9, m_coeffs);

   float offsets[3] = {0.0, 128.0, 128.0};
   std::copy(offsets, offsets + 3, m_offsets);
}

Rgb2YCbCr::~Rgb2YCbCr()
{
   cm_result_check(m_pCmDev->DestroySurface2DUP(m_pSrcSurf));
   free(m_pSrc);
}

// Need to return a VA surface ID and not CM surface ID
int Rgb2YCbCr::CreateInputSurface(const char *filename, const int width, const int height,
      unsigned int & outPitch, unsigned int & outSurfaceSize)
{
   CM_SURFACE_FORMAT surfaceFormat = CM_SURFACE_FORMAT_A8R8G8B8;

   m_pCmDev->GetSurface2DInfo(width, height, surfaceFormat,
         outPitch, outSurfaceSize);

   m_pSrc = (uchar *) CM_ALIGNED_MALLOC(outSurfaceSize, 0x1000);
   memset(m_pSrc, 0, outSurfaceSize);

   assert(UploadInputImage(filename, m_pSrc, width, height, outPitch, false) >=
         0);

   cm_result_check(m_pCmDev->CreateSurface2DUP(width, height, surfaceFormat,
            m_pSrc, m_pSrcSurf));
   m_pSrcSurf->GetIndex(m_pSI_SrcSurf);

   return 0;

}

int Rgb2YCbCr::WrapSurface(VASurfaceID vaSurface)
{
   CmSurface2D *cmSurface = NULL;

   cm_result_check(m_pCmDev->CreateSurface2D(vaSurface, cmSurface));
   cmSurface->GetIndex(m_pSI_DstSurf);

   return 0;
}

int Rgb2YCbCr::PreRun(
      int            nPicWidth,
      int            nPicHeight
      )
{
   int   result;
   CmThreadSpace *pTS       = NULL;

   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;

   nPicWidthInBlk = (nPicWidth + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
   nPicHeightInBlk = (nPicHeight + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT;

   cm_result_check(m_pCmDev->CreateThreadSpace(nPicWidthInBlk, nPicHeightInBlk, pTS));

   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_SrcSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_DstSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, 9*sizeof(float), &m_coeffs[0]);
   result = m_pKernel->SetKernelArg(nKernelInput++, 3*sizeof(float), &m_offsets[0]);
   result = m_pKernel->AssociateThreadSpace(pTS);

   cm_result_check(result);

   return CM_SUCCESS;

}

int Rgb2YCbCr::Execute(VASurfaceID dummyID)
{
   return ExecuteInternal();
}

void Rgb2YCbCr::Destroy(VASurfaceID dummyID)
{
   DestroyInternal();
}


///////////////////////////////////////////////////////////////
// 
// YCbCr2Rgb class
//
//
YCbCr2Rgb::YCbCr2Rgb(VADisplay vaDpy) : Scan2FilePipeline(),  KernelBase(vaDpy)
{
   UINT version = 0;
   // Create a CM Device
   cm_result_check(CreateCmDevice( m_pCmDev, version,vaDpy ));

   Init();

   CreateKernel(this->GetIsa(), this->GetKernelName(), m_pKernel);

   AddKernel(m_pTask, m_pKernel);

   float coefficients[4] = {1.402, -0.344136, -0.714136, 1.772};
   std::copy(coefficients, coefficients + 4, m_coeffs);

   float offsets[3] = {0.0, 128.0, 128.0};
   std::copy(offsets, offsets + 3, m_offsets);
}

YCbCr2Rgb::~YCbCr2Rgb()
{
   cm_result_check(m_pCmDev->DestroySurface2DUP(m_pDstSurf));
   free(m_pDst);
}

int YCbCr2Rgb::WrapSurface(VASurfaceID vaSurface)
{
   CmSurface2D *cmSurface = NULL;

   cm_result_check(m_pCmDev->CreateSurface2D(vaSurface, cmSurface));
   cmSurface->GetIndex(m_pSI_SrcSurf);

   return 0;
}

int YCbCr2Rgb::CreateOutputSurfaces(const int width, const int height)
{
   CM_SURFACE_FORMAT surfaceFormat = CM_SURFACE_FORMAT_A8R8G8B8;
   unsigned int outPitch, outSurfaceSize;

   m_PicWidth = width;
   m_PicHeight = height;

   m_pCmDev->GetSurface2DInfo(width, height, surfaceFormat, outPitch, outSurfaceSize);

   m_pDst = (uchar *) CM_ALIGNED_MALLOC(outSurfaceSize, 0x1000);
   memset(m_pDst, 0, outSurfaceSize);

   cm_result_check(m_pCmDev->CreateSurface2DUP(width, height, CM_SURFACE_FORMAT_A8R8G8B8,
            m_pDst, m_pDstSurf));
   m_pDstSurf->GetIndex(m_pSI_DstSurf);

   return 0;
}

int YCbCr2Rgb::PreRun(
      int   nPicWidth,
      int   nPicHeight
      )
{
   int   result;
   CmThreadSpace *pTS       = NULL;

   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;

   nPicWidthInBlk = (nPicWidth + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
   nPicHeightInBlk = (nPicHeight + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT;

   cm_result_check(m_pCmDev->CreateThreadSpace(nPicWidthInBlk, nPicHeightInBlk, pTS));

   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_SrcSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_DstSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, 4*sizeof(float), &m_coeffs[0]);
   result = m_pKernel->SetKernelArg(nKernelInput++, 3*sizeof(float), &m_offsets[0]);
   result = m_pKernel->AssociateThreadSpace(pTS);

   cm_result_check(result);

   return CM_SUCCESS;


}

int YCbCr2Rgb::WriteOut(VASurfaceID surfaceID, const char *filename)
{
   unsigned int outPitch, outSurfaceSize;

   m_pCmDev->GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8R8G8B8,
         outPitch, outSurfaceSize);

   if (m_pDst)
   {
      WriteToFile(filename, m_pDst, m_PicWidth, m_PicHeight, outPitch, false); 
   }

   return 0;
}

int YCbCr2Rgb::Execute(VASurfaceID dummyID)
{
   return ExecuteInternal();
}

void YCbCr2Rgb::Destroy(VASurfaceID dummyID)
{
   DestroyInternal();
}
