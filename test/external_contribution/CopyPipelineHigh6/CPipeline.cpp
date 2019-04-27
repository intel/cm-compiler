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
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////
#include "CPipeline.h"
#include "common/bitmap_helpers.h"
#include "common/cm_rt_helpers.h"

#include "RgbToLab.h"
#include "SyBgLc.h"
#include "SoCgDiAO.h"
#include "BoxNem.h"
#include "DilateAnd.h"
#include "CmykRF.h"
#include "HalftonePack8to1.h"
#include "ThresholdProduceEdgeK.h"
#include "Unpack1to8Pack8to1.h"

#define CM_Error_Handle(x) cm_result_check(x)

// Common interface
static unsigned long GetCurrentTimeInMilliseconds()
{
    struct timeval time;
    gettimeofday(&time, NULL);

    return (time.tv_sec * 1000.0) + time.tv_usec/1000.0;
}

// Compare two pictures
int Compare (uchar* pRef, uchar* pOut, int nPicWidth, int nPicHeight)
{
   int nError = 0;
   int nRow, nCol;
   uchar *pRef_Row, *pOut_Row;

   pRef_Row = pRef;
   pOut_Row = pOut;

   for (nRow = 0; nRow < nPicHeight; nRow++)
   {
      for (nCol = 0; nCol < nPicWidth; nCol++)
      {
         if (pOut_Row[nCol] != pRef_Row[nCol])
         {
               if (((pOut_Row[nCol] - pRef_Row[nCol]) > 1) ||
                   ((pRef_Row[nCol] - pOut_Row[nCol]) > 1))
                {
                  printf("Pixel differs at[%d, %d] ! Ref = %d, Out = %d\n",
                        nRow, nCol, pRef_Row[nCol], pOut_Row[nCol]);
                  nError++;
                  if (nError > 10)
                     return nError;

               }
         }
      }
      pRef_Row += nPicWidth;
      pOut_Row += nPicWidth;
   }

   return nError;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Initialize CM device
CopyPipeline::CopyPipeline()
{

   m_PicWidth = 0;
   m_PicHeight = 0;
   m_Max_Thread_Count = 0;

   m_pSrcR = NULL;
   m_pSrcG = NULL;
   m_pSrcB = NULL;
   m_pDstC = NULL;
   m_pDstM = NULL;
   m_pDstY = NULL;
   m_pDstK = NULL;
   m_pDstCMYK = NULL;
   m_pCmDev = NULL;
   m_pCmQueue = NULL;
   e_Pipeline = NULL;

}

int CopyPipeline::Init()
{
   UINT version = 0;
   int result;

   // Create a CM Device
   cm_result_check(CreateCmDevice( m_pCmDev, version ));

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

   // Inits the print buffer.
   cm_result_check(m_pCmDev->InitPrintBuffer());

   cm_result_check(m_pCmDev->GetCaps(
            CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP,
            size,
            &m_Max_Thread_Count));

   return CM_SUCCESS;
}

CopyPipeline::~CopyPipeline()
{
   //free(m_pCommonISA);
   ::DestroyCmDevice(m_pCmDev);

   free(m_pSrcR);
   free(m_pSrcG);
   free(m_pSrcB);
   free(m_pDstC);
   free(m_pDstM);
   free(m_pDstY);
   free(m_pDstK);
   free(m_pDstCMYK);
}

int CopyPipeline::GetInputImage(const char* filename)
{
   auto input_image = cm::util::bitmap::BitMap::load(filename);

   // Gets the width and height of the bitmap file.
   m_PicWidth = input_image.getWidth();
   m_PicHeight = input_image.getHeight();

   unsigned int pitch_size = 0;
   unsigned int surface_size = 0;
   GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8,
         &pitch_size, &surface_size);

   m_pSrcR = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
   m_pSrcG = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
   m_pSrcB = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);

   memset(m_pSrcR, 0, surface_size);
   memset(m_pSrcG, 0, surface_size);
   memset(m_pSrcB, 0, surface_size);

   uchar *pSrc  = input_image.getData();

   for (int y = 0; y < m_PicHeight; y++)
   {
      for (int x = 0; x < m_PicWidth; x++)
      {
         m_pSrcR[x + y*pitch_size] = pSrc[x*3 + y*m_PicWidth*3];
         m_pSrcG[x + y*pitch_size] = pSrc[x*3 + 1 + y*m_PicWidth*3];
         m_pSrcB[x + y*pitch_size] = pSrc[x*3 + 2 + y*m_PicWidth*3];
      }
   }

   unsigned int outpitch = 0;
   unsigned int outsize = 0;
   GetSurface2DInfo(m_PicWidth/8, m_PicHeight, CM_SURFACE_FORMAT_A8,
         &outpitch, &outsize);

   m_pDstC = (uchar *)CM_ALIGNED_MALLOC(outsize, 0x1000);
   m_pDstM = (uchar *)CM_ALIGNED_MALLOC(outsize, 0x1000);
   m_pDstY = (uchar *)CM_ALIGNED_MALLOC(outsize, 0x1000);
   m_pDstK = (uchar *)CM_ALIGNED_MALLOC(outsize, 0x1000);

   GetSurface2DInfo(m_PicWidth/8, m_PicHeight, CM_SURFACE_FORMAT_A8R8G8B8,
         &outpitch, &outsize);

   m_pDstCMYK = (uchar *)CM_ALIGNED_MALLOC(outsize, 0x1000);

   return CM_SUCCESS;

}

int CopyPipeline::SaveOutputImage(const char* filename)
{
   // Take 4 planars of 1bpp surface, unpack1to8, then combine to interleave
   // CMYK, then do pack8to1 for interleave image
   cout << "Reformatting output to save to " << filename << endl;

   CmTask *pTask = NULL;
   cm_result_check(m_pCmDev->CreateTask(pTask));

   CmSurface2DUP *pImageC = NULL;
   CmSurface2DUP *pImageM = NULL;
   CmSurface2DUP *pImageY = NULL;
   CmSurface2DUP *pImageK = NULL;
   CmSurface2DUP *pImageCMYK = NULL;
   SurfaceIndex *pSI_ImageC = NULL;
   SurfaceIndex *pSI_ImageM = NULL;
   SurfaceIndex *pSI_ImageY = NULL;
   SurfaceIndex *pSI_ImageK = NULL;
   SurfaceIndex *pSI_ImageCMYK = NULL;

   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth/8, m_PicHeight,
            CM_SURFACE_FORMAT_A8R8G8B8, m_pDstCMYK, pImageCMYK));
   cm_result_check(pImageCMYK->GetIndex(pSI_ImageCMYK));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth/8, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstC, pImageC));
   cm_result_check(pImageC->GetIndex(pSI_ImageC));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth/8, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstM, pImageM));
   cm_result_check(pImageM->GetIndex(pSI_ImageM));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth/8, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstY, pImageY));
   cm_result_check(pImageY->GetIndex(pSI_ImageY));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth/8, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstK, pImageK));
   cm_result_check(pImageK->GetIndex(pSI_ImageK));

   Unpack1to8Pack8to1 *unpackpacknode = new Unpack1to8Pack8to1(m_pCmDev, m_Max_Thread_Count);
   CmKernel *pUnpackAndPackKernel;
   cm_result_check(CreateKernel(unpackpacknode->GetIsa(), unpackpacknode->GetKernelName(), pUnpackAndPackKernel));
   AddKernel(pTask, pUnpackAndPackKernel);
   unpackpacknode->PreRun(pUnpackAndPackKernel, pSI_ImageC, pSI_ImageM, pSI_ImageY, pSI_ImageK, pSI_ImageCMYK, m_PicWidth, m_PicHeight);

   ExecuteGraph(pTask, 1);

   FILE *fpOut;
   //CMYK output is 1bpp
   unsigned int outpitch = 0;
   unsigned int outsize = 0;
   GetSurface2DInfo(m_PicWidth/8, m_PicHeight, CM_SURFACE_FORMAT_A8R8G8B8,
         &outpitch, &outsize);

   if (m_pDstCMYK)
   {
      fpOut = fopen(filename, "wb");
      unsigned char *pTmp = m_pDstCMYK;
      if (fpOut != NULL)
      {
         for (int yy = 0; yy < m_PicHeight; yy++)
         {
            fwrite(pTmp, sizeof(uchar), m_PicWidth/8*4, fpOut);
            pTmp += outpitch;
         }
         fclose(fpOut);

      }
   }
}

int CopyPipeline::SetLightnessContrast(const int lightness, const int contrast) 
{
   m_lightness = lightness;
   m_contrast = contrast;
}

int CopyPipeline::GetSurface2DInfo(int width, int height, CM_SURFACE_FORMAT format,
        unsigned int * pitch, unsigned int * surface_size)
{
    m_pCmDev->GetSurface2DInfo(width, height, format, *pitch, *surface_size);
}

int CopyPipeline::CreateKernel(char *isaFile, char* kernelName, CmKernel *& pKernel)
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
      return -1;
   }

   pCommonISA = (uchar*) malloc(nKernelSize);
   if( !pCommonISA )
   {
      return -1;
   }

   if (fread(pCommonISA, 1, nKernelSize, pISA) != nKernelSize) {
      perror("Read fail\n");
      return -1;
   }
   fclose(pISA);

   //cm_result_check(m_pCmDev->LoadProgram(pCommonISA, nKernelSize, pProgram, "nojitter"));
   cm_result_check(m_pCmDev->LoadProgram(pCommonISA, nKernelSize, pProgram));

   cm_result_check(m_pCmDev->CreateKernel(pProgram, kernelName, pKernel));

   free(pCommonISA);

   return CM_SUCCESS;
}

int CopyPipeline::AddKernel(CmTask *pTask, CmKernel *pKernel)
{
   // Add kernel to task
   cm_result_check(pTask-> AddKernel(pKernel));
   cm_result_check(pTask-> AddSync());

   return CM_SUCCESS;
}

int CopyPipeline::AssemblerHigh6Graph(CmTask *& pTask)
{
   if (pTask == NULL)
      cm_result_check(m_pCmDev->CreateTask(pTask));

   // Source surface
   CmSurface2DUP *pSrcR = NULL;
   CmSurface2DUP *pSrcG = NULL;
   CmSurface2DUP *pSrcB = NULL;
   SurfaceIndex *pSI_SrcR = NULL;
   SurfaceIndex *pSI_SrcG = NULL;
   SurfaceIndex *pSI_SrcB = NULL;
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pSrcR, pSrcR));
   cm_result_check(pSrcR->GetIndex(pSI_SrcR));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pSrcG, pSrcG));
   cm_result_check(pSrcG->GetIndex(pSI_SrcG));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pSrcB, pSrcB));
   cm_result_check(pSrcB->GetIndex(pSI_SrcB));

   // Create 6 intermediate U8 buffers 
   CmSurface2D  *u8buf0 = NULL;
   CmSurface2D  *u8buf1 = NULL;
   CmSurface2D  *u8buf2 = NULL;
   CmSurface2D  *u8buf3 = NULL;
   CmSurface2D  *u8buf4 = NULL;
   CmSurface2D  *u8buf5 = NULL;
   SurfaceIndex *pSI_u8buf0, *pSI_u8buf1, *pSI_u8buf2;
   SurfaceIndex *pSI_u8buf3, *pSI_u8buf4, *pSI_u8buf5;

   cm_result_check(m_pCmDev->CreateSurface2D(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8, u8buf0));
   cm_result_check(u8buf0->GetIndex(pSI_u8buf0));
   cm_result_check(m_pCmDev->CreateSurface2D(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8, u8buf1));
   cm_result_check(u8buf1->GetIndex(pSI_u8buf1));
   cm_result_check(m_pCmDev->CreateSurface2D(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8, u8buf2));
   cm_result_check(u8buf2->GetIndex(pSI_u8buf2));
   cm_result_check(m_pCmDev->CreateSurface2D(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8, u8buf3));
   cm_result_check(u8buf3->GetIndex(pSI_u8buf3));
   cm_result_check(m_pCmDev->CreateSurface2D(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8, u8buf4));
   cm_result_check(u8buf4->GetIndex(pSI_u8buf4));
   cm_result_check(m_pCmDev->CreateSurface2D(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8, u8buf5));
   cm_result_check(u8buf5->GetIndex(pSI_u8buf5));

   // Create one intermediate X8R8G8B8 buffer
   CmSurface2D *u32buf = NULL;
   SurfaceIndex *pSI_u32buf;

   cm_result_check(m_pCmDev->CreateSurface2D(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_X8R8G8B8, u32buf));
   cm_result_check(u32buf->GetIndex(pSI_u32buf));

   // Destination surface
   CmSurface2DUP *pDstImageC = NULL;
   CmSurface2DUP *pDstImageM = NULL;
   CmSurface2DUP *pDstImageY = NULL;
   CmSurface2DUP *pDstImageK = NULL;
   SurfaceIndex *pSI_DstImageC = NULL;
   SurfaceIndex *pSI_DstImageM = NULL;
   SurfaceIndex *pSI_DstImageY = NULL;
   SurfaceIndex *pSI_DstImageK = NULL;

   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth/8, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstC, pDstImageC));
   cm_result_check(pDstImageC->GetIndex(pSI_DstImageC));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth/8, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstM, pDstImageM));
   cm_result_check(pDstImageM->GetIndex(pSI_DstImageM));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth/8, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstY, pDstImageY));
   cm_result_check(pDstImageY->GetIndex(pSI_DstImageY));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth/8, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstK, pDstImageK));
   cm_result_check(pDstImageK->GetIndex(pSI_DstImageK));

   // RGB To LAB node
   RgbToLab *rgbnode = new RgbToLab(m_pCmDev, m_Max_Thread_Count);
   CmKernel *pRgb2LabKernel;
   cm_result_check(CreateKernel(rgbnode->GetIsa(), rgbnode->GetKernelName(), pRgb2LabKernel));
   AddKernel(pTask, pRgb2LabKernel);

   rgbnode->PreRun(pRgb2LabKernel, pSI_SrcR, pSI_SrcG, pSI_SrcB, pSI_u8buf0, pSI_u8buf1, pSI_u8buf2, m_PicWidth, m_PicHeight);

   // Symmetric7x7, Background Suppresion, Lightness contrast combination node
   SyBgLc *sybglcnode = new SyBgLc(m_pCmDev, m_Max_Thread_Count, m_lightness,
         m_contrast);
   CmKernel *pSyBgLcKernel;
   cm_result_check(CreateKernel(sybglcnode->GetIsa(),
            sybglcnode->GetKernelName(), pSyBgLcKernel));
   AddKernel(pTask, pSyBgLcKernel);
   sybglcnode->PreRun(pSyBgLcKernel, pSI_u8buf0, pSI_u8buf1, pSI_u8buf2,
         pSI_u8buf3, pSI_u8buf4, pSI_u8buf5, m_PicWidth, m_PicHeight);

   // Sobel3x3, CalculateGradient, Dilate, And, Or combination node
   SoCgDiAO *socgdiaonode = new SoCgDiAO(m_pCmDev, m_Max_Thread_Count);
   CmKernel *pSoCgDiAOKernel;
   cm_result_check(CreateKernel(socgdiaonode->GetIsa(), socgdiaonode->GetKernelName(), pSoCgDiAOKernel));
   AddKernel(pTask, pSoCgDiAOKernel);

   socgdiaonode->PreRun(pSoCgDiAOKernel, pSI_u8buf3, pSI_u8buf0, m_PicWidth, m_PicHeight);

   // Box3x3 and Neutral Edge Detection combination node
   BoxNem *boxnemnode = new BoxNem(m_pCmDev, m_Max_Thread_Count);
   CmKernel *pBoxNemKernel;
   cm_result_check(CreateKernel(boxnemnode->GetIsa(),
            boxnemnode->GetKernelName(), pBoxNemKernel));
   AddKernel(pTask, pBoxNemKernel);

   boxnemnode->PreRun(pBoxNemKernel, pSI_u8buf4, pSI_u8buf5, pSI_u8buf1, m_PicWidth,
         m_PicHeight);

   // Dilate and And to produce neutral edge mask
   DilateAnd *dilateandnode = new DilateAnd(m_pCmDev, m_Max_Thread_Count);
   CmKernel *pDilateAndKernel;
   cm_result_check(CreateKernel(dilateandnode->GetIsa(),
            dilateandnode->GetKernelName(), pDilateAndKernel));
   AddKernel(pTask, pDilateAndKernel);

   dilateandnode->PreRun(pDilateAndKernel, pSI_u8buf0, pSI_u8buf1, pSI_u8buf2,
         m_PicWidth, m_PicHeight);

   // Lab2CMYK + Remove fringe
   CmykRF *cmykrfnode = new CmykRF(m_pCmDev, m_Max_Thread_Count);
   CmKernel *pCmykRFKernel;
   cm_result_check(CreateKernel(cmykrfnode->GetIsa(),
            cmykrfnode->GetKernelName(), pCmykRFKernel));
   AddKernel(pTask, pCmykRFKernel);

   cmykrfnode->PreRun(pCmykRFKernel, pSI_u8buf3, pSI_u8buf4, pSI_u8buf5, pSI_u8buf2, pSI_u32buf,
         pSI_u8buf0, m_PicWidth, m_PicHeight);

   // Halftone + Pack8to1
   HalftonePack8to1 *halftonenode = new HalftonePack8to1(m_pCmDev,
         m_Max_Thread_Count);
   CmKernel *pHalftoneKernel;
   cm_result_check(CreateKernel(halftonenode->GetIsa(),
            halftonenode->GetKernelName(), pHalftoneKernel));
   AddKernel(pTask, pHalftoneKernel);

   halftonenode->PreRun(pHalftoneKernel, pSI_u32buf, pSI_DstImageC, pSI_DstImageM,
         pSI_DstImageY, pSI_u8buf1, m_PicWidth, m_PicHeight);

   // SimpleThreshold + Dilate3x3 + ProduceEdgeK + Pack8to1
   ThresholdProduceEdgeK *thproduceknode = new ThresholdProduceEdgeK(m_pCmDev,
         m_Max_Thread_Count);
   CmKernel *pThproducekKernel;
   cm_result_check(CreateKernel(thproduceknode->GetIsa(),
            thproduceknode->GetKernelName(), pThproducekKernel));
   AddKernel(pTask, pThproducekKernel);

   thproduceknode->PreRun(pThproducekKernel, pSI_u8buf0, pSI_u8buf2, pSI_u8buf1,
         pSI_DstImageK, m_PicWidth, m_PicHeight);
}

// Execute GPU RGBToC1ELab
int CopyPipeline::ExecuteGraph(CmTask *pTask, int iteration)
{
   if (pTask == NULL)
      return CM_FAILURE;

   DWORD dwTimeOutMs = -1;
   UINT64 executionTime;

   unsigned long start = GetCurrentTimeInMilliseconds();
   for (int loop = 0; loop < iteration; loop++)
   {
      unsigned long starte = GetCurrentTimeInMilliseconds();
      cm_result_check(m_pCmQueue->EnqueueWithGroup(pTask, e_Pipeline));
      cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
      unsigned long ende = GetCurrentTimeInMilliseconds();
      e_Pipeline->GetExecutionTime(executionTime);
      printf("Execution time=%dms, Kernel time: %.2fms\n", (ende-starte),
            executionTime/1000000.0);
   }
   unsigned long end = GetCurrentTimeInMilliseconds();

   if (iteration > 1)
   {
      unsigned long total = end - start;
      double average_time = total / (double) iteration;
      int PPM = 60000.0 / average_time;
      std::cout << "PPM = " << PPM << std::endl;
   }

   cm_result_check(m_pCmDev->FlushPrintBuffer());

   cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));

   cm_result_check(m_pCmQueue->DestroyEvent(e_Pipeline));

   cm_result_check(m_pCmDev->DestroyTask(pTask));

   return CM_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
