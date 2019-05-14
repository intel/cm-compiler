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
#include <string.h>

/*********************************************************************************************************************************/
#include "common/bitmap_helpers.h"
#include "common/cm_rt_helpers.h"

#include "ScanPreProcess.h"
#include "GainOffset.h"
#include "WarpAffine.h"

#define CM_Error_Handle(x) cm_result_check(x)

// Common interface
static unsigned long GetCurrentTimeInMilliseconds()
{
   struct timeval time;
   gettimeofday(&time, NULL);

   return (time.tv_sec * 1000.0) + time.tv_usec/1000.0;
}

/*********************************************************************************************************************************/
// Initialize CM Device //
ScanPreProcess::ScanPreProcess()
{
   m_PicWidth     = 0;
   m_PicHeight    = 0;
   m_SkewedWidth  = 0;
   m_SkewedHeight = 0;

   m_pCmDev     = NULL;
   m_pCmQueue   = NULL;
   e_Pipeline   = NULL;

   m_pSrc         = NULL;
   m_pSkewedR     = NULL;
   m_pSkewedG     = NULL;
   m_pSkewedB     = NULL;
   m_pRawGainOffsetR    = NULL;
   m_pRawGainOffsetG    = NULL;
   m_pRawGainOffsetB    = NULL;
   m_pCorrectedGainOffsetDst = NULL;
   m_pDstR  = NULL;
   m_pDstG  = NULL;
   m_pDstB  = NULL;
}

/*********************************************************************************************************************************/

int ScanPreProcess::Init()
{
   UINT version = 0;
   int result;

   // Create a CM Device
   result = cm_result_check(CreateCmDevice( m_pCmDev, version ));

   // For testing only - collect platform information
   int nGPUPlatform     = 0;
   int nGTPlatform      = 0;
   int nGPUFrequency    = 0;
   char* sGPUPlatform[] = {"Unk", "SNB", "IVB", "HSW", "BDW", "VLV", "CHV", "SKL", "BXT", "CNL", "KBL", "GLK"};
   char* sGT[]          = {"Unk", "GT1", "GT2", "GT3", "GT4"};
   size_t size          = 4;

   cm_result_check(m_pCmDev->GetCaps( CAP_GPU_PLATFORM, size, &nGPUPlatform ));
   cm_result_check(m_pCmDev->GetCaps( CAP_GT_PLATFORM, size, &nGTPlatform ));
   cm_result_check(m_pCmDev->GetCaps( CAP_GPU_CURRENT_FREQUENCY, size, &nGPUFrequency ));

   if ((nGPUPlatform < sizeof(sGPUPlatform)/sizeof(sGPUPlatform[0])) &&
         (nGTPlatform < sizeof(sGT)/sizeof(sGT[0])))
   {
      printf
      (
         "The test is running on Intel %s %s platform with frequency at %d MHz.\n", 
         sGPUPlatform[nGPUPlatform], sGT[nGTPlatform], nGPUFrequency
      );
   }
   ///////////////

   // Create a task queue
   cm_result_check(m_pCmDev->CreateQueue(m_pCmQueue));

   // Inits the print buffer.
   cm_result_check(m_pCmDev->InitPrintBuffer());

   return CM_SUCCESS;
}

/*********************************************************************************************************************************/

ScanPreProcess::~ScanPreProcess()
{

   //cm_result_check(m_pCmDev->DestroyTask(pTask));
   ::DestroyCmDevice(m_pCmDev);

   free(m_pSrc);
   free(m_pSkewedR);
   free(m_pSkewedG);
   free(m_pSkewedB);
   free(m_pRawGainOffsetR);
   free(m_pRawGainOffsetG);
   free(m_pRawGainOffsetB);
   free(m_pCorrectedGainOffsetDst);
   free(m_pDstR);
   free(m_pDstG);
   free(m_pDstB);
}

/*********************************************************************************************************************************/

int ScanPreProcess::GetInputImage(const char* filename)
{
   auto input_image = cm::util::bitmap::BitMap::load(filename);

   // Gets the width and height of the bitmap file.
   m_PicWidth    = input_image.getWidth();
   m_PicHeight   = input_image.getHeight();

   unsigned int pitch_size   = 0;
   unsigned int surface_size = 0;

   GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8R8G8B8, &pitch_size, &surface_size);

   m_pSrc = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
   memset(m_pSrc, 0, surface_size);

   uchar *pSrc = input_image.getData();

   uchar *tmpsrc2 = m_pSrc;

   for (int xx = 0; xx < m_PicHeight*m_PicWidth; xx++)
   {
      tmpsrc2[0] = pSrc[0];
      tmpsrc2[1] = pSrc[1];
      tmpsrc2[2] = pSrc[2];
      tmpsrc2[3] = 0;
      tmpsrc2 += 4;
      pSrc += 3;
   }

   GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8, &pitch_size, &surface_size);

   m_pDstR = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
   m_pDstG = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
   m_pDstB = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);

   return CM_SUCCESS;
}

/*********************************************************************************************************************************/

int ScanPreProcess::SaveOutputImage(const char* filename, int skewangle)
{
   unsigned int outpitch;
   unsigned int outsize;

   GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8R8G8B8, &outpitch, &outsize);

   uchar *pDst = (uchar *)malloc(outsize*3/4);

   for (int y = 0; y < m_PicHeight; y++)
   {
      for (int x = 0; x < m_PicWidth; x++)
      {
         pDst[x*3 + y*m_PicWidth*3]     = m_pDstB[x + y*m_PicWidth];
         pDst[x*3 + 1 + y*m_PicWidth*3] = m_pDstG[x + y*m_PicWidth];
         pDst[x*3 + 2 + y*m_PicWidth*3] = m_pDstR[x + y*m_PicWidth];
      }
   }

   auto input_image = cm::util::bitmap::BitMap::load(filename);

   ///////////Output in BMP format////////////   
   auto output_image(input_image);
   const char* out_filename;

   std::string str;
   int len = strlen(filename);

   if (filename)
   {
      for(int i = 0; i < len; i++)
      {
         str += filename[i];
      }

      str += "_output.bmp";
      out_filename = str.c_str();
   }

   unsigned output_image_size = m_PicWidth * m_PicHeight * output_image.getBPP()/8;
   output_image.setData(new unsigned char[output_image_size]);

   unsigned char *tmp_dst1 = output_image.getData();
   unsigned char *tmp_dst2 = pDst;

   for (int i = 0; i < m_PicWidth * m_PicHeight;)
   {
      tmp_dst1[0] = tmp_dst2[0];
      tmp_dst1[1] = tmp_dst2[1];
      tmp_dst1[2] = tmp_dst2[2];
      tmp_dst1 += 3;
      tmp_dst2 += 3;
      i++;
      if ((i % m_PicWidth) == 0)
         tmp_dst1 = &output_image.getData()[(i / m_PicWidth) * m_PicWidth * 3];

   }
   output_image.save(out_filename);
   free(pDst);

   ///////////Output Skewed Image in RAW format////////////
   const char* skewed_filename;

   std::string sstr;
   int slen = strlen(out_filename);

   char a[3];
   sprintf(a, "%d", skewangle);

   if (out_filename)
   {
      for(int i = 0; i < slen-4; i++)
      {
         sstr += out_filename[i];
      }

      sstr += "_skewed_";
      sstr += a;
      sstr += "degree.raw";
      skewed_filename = sstr.c_str();
   }

   uchar *pSkewed = (uchar *)malloc(m_SkewedWidth*m_SkewedHeight*3);

   for (int y = 0; y < m_SkewedHeight; y++)
   {
      for (int x = 0; x < m_SkewedWidth; x++)
      {
         pSkewed[x*3 + y*m_SkewedWidth*3]     = m_pSkewedB[x + y*m_SkewedWidth];
         pSkewed[x*3 + 1 + y*m_SkewedWidth*3] = m_pSkewedG[x + y*m_SkewedWidth];
         pSkewed[x*3 + 2 + y*m_SkewedWidth*3] = m_pSkewedR[x + y*m_SkewedWidth];
      }
   }

   FILE *fpOut;
   if (pSkewed)
   {
      fpOut = fopen(skewed_filename, "wb");
      if (fpOut != NULL)
      {
         fwrite(pSkewed, sizeof(uchar), m_SkewedWidth*m_SkewedHeight*3 , fpOut);
         fclose(fpOut);
      }
   }

   free(pSkewed);
   return 0;
}

/*********************************************************************************************************************************/

int ScanPreProcess::GetSurface2DInfo(
   int width,
   int height,
   CM_SURFACE_FORMAT format,
   unsigned int * pitch,
   unsigned int * surface_size
   )
{
   m_pCmDev->GetSurface2DInfo(width, height, format, *pitch, *surface_size);
}

/*********************************************************************************************************************************/

int ScanPreProcess::CreateKernel(char *isaFile, char* kernelName, CmKernel *& pKernel)
{
   int nKernelSize;
   uchar       *pCommonISA; // Common ISA kernel
   CmProgram   *pProgram;

   // Create kernel
   FILE* pISA = fopen(isaFile, "rb");
   if (pISA == NULL)
   {
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

   if (fread(pCommonISA, 1, nKernelSize, pISA) != nKernelSize)
   {
      perror("Read fail\n");
      return -1;
   }
   fclose(pISA);

   cm_result_check(m_pCmDev->LoadProgram(pCommonISA, nKernelSize, pProgram));
   cm_result_check(m_pCmDev->CreateKernel(pProgram, kernelName, pKernel));

   free(pCommonISA);

   return CM_SUCCESS;
}

/*********************************************************************************************************************************/

int ScanPreProcess::AddKernel(CmTask *pTask, CmKernel *pKernel)
{
   // Add kernel to task
   cm_result_check(pTask-> AddKernel(pKernel));
   cm_result_check(pTask-> AddSync());

   return CM_SUCCESS;
}

/*********************************************************************************************************************************/

int ScanPreProcess::AssemblerGraph(CmTask *&SppTask, int skewAngle, int bits)
{
   CmTask *pTask = NULL;
   cm_result_check(m_pCmDev->CreateTask(pTask));

   //Input
   CmSurface2DUP  *m_pSrcSurf    = NULL;
   SurfaceIndex *m_pSI_SrcSurf = NULL;

   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8R8G8B8, m_pSrc, m_pSrcSurf));
   cm_result_check(m_pCmDev->CreateSamplerSurface2DUP(m_pSrcSurf, m_pSI_SrcSurf));

   //WarpAffine(Skew Image)
   WarpAffine *warpnode = new WarpAffine(m_pCmDev);
   CmKernel *pWarpKernel;
   cm_result_check(CreateKernel(warpnode->GetISA(), warpnode->GetKernelName(), pWarpKernel));
   AddKernel(pTask, pWarpKernel);
   warpnode->GetSkew(SKEW, m_PicWidth, m_PicHeight, &m_SkewedWidth, &m_SkewedHeight, skewAngle);

   uint skewed_size = 0;
   uint pit_size = 0;

   GetSurface2DInfo(m_SkewedWidth, m_SkewedHeight, CM_SURFACE_FORMAT_A8, &pit_size, &skewed_size);
   m_SkewedWidth = pit_size;
   std::cout << "Skewed Image Geometry = " << m_SkewedWidth << "x" << m_SkewedHeight
      << ", Skew Angle = " << skewAngle << std::endl;

   m_pSkewedR = (uchar *)CM_ALIGNED_MALLOC(skewed_size, 0x1000);
   m_pSkewedG = (uchar *)CM_ALIGNED_MALLOC(skewed_size, 0x1000);
   m_pSkewedB = (uchar *)CM_ALIGNED_MALLOC(skewed_size, 0x1000);

   //Output
   CmSurface2DUP *m_pSkewedRSurf    = NULL;
   SurfaceIndex  *m_pSI_SkewedRSurf = NULL;
   CmSurface2DUP *m_pSkewedGSurf    = NULL;
   SurfaceIndex  *m_pSI_SkewedGSurf = NULL;
   CmSurface2DUP *m_pSkewedBSurf    = NULL;
   SurfaceIndex  *m_pSI_SkewedBSurf = NULL;

   cm_result_check(m_pCmDev->CreateSurface2DUP(m_SkewedWidth, m_SkewedHeight,
            CM_SURFACE_FORMAT_A8, m_pSkewedR, m_pSkewedRSurf));
   cm_result_check(m_pSkewedRSurf->GetIndex(m_pSI_SkewedRSurf));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_SkewedWidth, m_SkewedHeight,
            CM_SURFACE_FORMAT_A8, m_pSkewedG, m_pSkewedGSurf));
   cm_result_check(m_pSkewedGSurf->GetIndex(m_pSI_SkewedGSurf));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_SkewedWidth, m_SkewedHeight,
            CM_SURFACE_FORMAT_A8, m_pSkewedB, m_pSkewedBSurf));
   cm_result_check(m_pSkewedBSurf->GetIndex(m_pSI_SkewedBSurf));

   warpnode->PreRun(pWarpKernel, m_pSI_SrcSurf, m_pSI_SkewedRSurf, m_pSI_SkewedGSurf,
         m_pSI_SkewedBSurf,  m_PicWidth, m_PicHeight, m_SkewedWidth, m_SkewedHeight);

   ExecuteGraph(pTask, 1);

   //GainOffset node
   GainOffset *offnode = new GainOffset(m_pCmDev);
   CmKernel *pGainOffsetKernel;

   uint validbytes = 0;
   uint p_size = 0;
   uint raw_size = 0;

   if( bits == 10 || bits == 12)
   {
      std::cout << bits << " bits Gain Offset is chosen" << std::endl;

      uint rawwidthbits = m_SkewedWidth * bits;
      validbytes   = rawwidthbits / 8;

      uint gainOffsetPitch;
      GetSurface2DInfo(validbytes, m_SkewedHeight, CM_SURFACE_FORMAT_A8, &gainOffsetPitch, &raw_size);

      m_pRawGainOffsetR = (uchar *)CM_ALIGNED_MALLOC(raw_size, 0x1000);
      m_pRawGainOffsetG = (uchar *)CM_ALIGNED_MALLOC(raw_size, 0x1000);
      m_pRawGainOffsetB = (uchar *)CM_ALIGNED_MALLOC(raw_size, 0x1000);

      offnode->Convert(m_pSkewedR, m_pSkewedG, m_pSkewedB, m_pRawGainOffsetR, m_pRawGainOffsetG,
            m_pRawGainOffsetB, m_SkewedWidth, m_SkewedHeight, gainOffsetPitch, bits);
   }
   else if (bits != 10 || bits != 12)
   {
      std::cout << "Bits option are not correct" << std::endl;
      return -1;
   }

   cm_result_check(m_pCmDev->DestroyTask(pTask));

   if (SppTask == NULL)
      cm_result_check(m_pCmDev->CreateTask(SppTask));

   // GO Source Surface
   CmSurface2DUP *m_pRawGainOffsetRSurf    = NULL;
   SurfaceIndex  *m_pSI_RawGainOffsetRSurf = NULL;
   CmSurface2DUP *m_pRawGainOffsetGSurf    = NULL;
   SurfaceIndex  *m_pSI_RawGainOffsetGSurf = NULL;
   CmSurface2DUP *m_pRawGainOffsetBSurf    = NULL;
   SurfaceIndex  *m_pSI_RawGainOffsetBSurf = NULL;

   cm_result_check(m_pCmDev->CreateSurface2DUP(validbytes, m_SkewedHeight,
            CM_SURFACE_FORMAT_A8, m_pRawGainOffsetR, m_pRawGainOffsetRSurf));
   m_pRawGainOffsetRSurf->GetIndex(m_pSI_RawGainOffsetRSurf);
   cm_result_check(m_pCmDev->CreateSurface2DUP(validbytes, m_SkewedHeight,
            CM_SURFACE_FORMAT_A8, m_pRawGainOffsetG, m_pRawGainOffsetGSurf));
   m_pRawGainOffsetGSurf->GetIndex(m_pSI_RawGainOffsetGSurf);
   cm_result_check(m_pCmDev->CreateSurface2DUP(validbytes, m_SkewedHeight,
            CM_SURFACE_FORMAT_A8, m_pRawGainOffsetB, m_pRawGainOffsetBSurf));
   m_pRawGainOffsetBSurf->GetIndex(m_pSI_RawGainOffsetBSurf);

   GetSurface2DInfo(m_SkewedWidth, m_SkewedHeight, CM_SURFACE_FORMAT_A8R8G8B8,
         &pit_size, &skewed_size);

   m_pCorrectedGainOffsetDst = (uchar *)CM_ALIGNED_MALLOC(skewed_size, 0x1000);

   //GO Destination Surface
   CmSurface2DUP *m_pCorrectedGainOffsetDstSurf   = NULL;
   SurfaceIndex *m_pSI_CorrectedGainOffsetDstSurf = NULL;

   cm_result_check(m_pCmDev->CreateSurface2DUP(m_SkewedWidth, m_SkewedHeight,
            CM_SURFACE_FORMAT_A8R8G8B8, m_pCorrectedGainOffsetDst,  m_pCorrectedGainOffsetDstSurf));
   m_pCorrectedGainOffsetDstSurf->GetIndex(m_pSI_CorrectedGainOffsetDstSurf);

   if ( bits == 10 )
      cm_result_check(CreateKernel( offnode->GetISA(), offnode->GetKernelName0(), pGainOffsetKernel));
   else
   if ( bits == 12 )
      cm_result_check(CreateKernel( offnode->GetISA(), offnode->GetKernelName1(), pGainOffsetKernel));

   AddKernel(SppTask, pGainOffsetKernel);

   offnode->PreRun(pGainOffsetKernel, m_pSI_RawGainOffsetRSurf, m_pSI_RawGainOffsetGSurf,
         m_pSI_RawGainOffsetBSurf, m_pSI_CorrectedGainOffsetDstSurf, m_SkewedWidth, m_SkewedHeight);

   //Destination Surface
   CmSurface2DUP *m_pDstRSurf    = NULL;
   SurfaceIndex  *m_pSI_DstRSurf = NULL;
   CmSurface2DUP *m_pDstGSurf    = NULL;
   SurfaceIndex  *m_pSI_DstGSurf = NULL;
   CmSurface2DUP *m_pDstBSurf    = NULL;
   SurfaceIndex  *m_pSI_DstBSurf = NULL;

   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstR, m_pDstRSurf));
   cm_result_check(m_pDstRSurf->GetIndex(m_pSI_DstRSurf));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstG, m_pDstGSurf));
   cm_result_check(m_pDstGSurf->GetIndex(m_pSI_DstGSurf));
   cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pDstB, m_pDstBSurf));
   cm_result_check(m_pDstBSurf->GetIndex(m_pSI_DstBSurf));

   //Warp Affine
   WarpAffine *affinenode = new WarpAffine(m_pCmDev);
   CmKernel *pAffineKernel;
   cm_result_check(CreateKernel(affinenode->GetISA(), affinenode->GetKernelName(), pAffineKernel));
   AddKernel(SppTask, pAffineKernel);
   affinenode->GetSkew(DESKEW, m_PicWidth, m_PicHeight, &m_PicWidth, &m_PicHeight, skewAngle);
   affinenode->PreRun(pAffineKernel, m_pSI_CorrectedGainOffsetDstSurf , m_pSI_DstRSurf,
         m_pSI_DstGSurf, m_pSI_DstBSurf, m_SkewedWidth, m_SkewedHeight, m_PicWidth, m_PicHeight); 

}

/*********************************************************************************************************************************/

//Execute Graph
int ScanPreProcess::ExecuteGraph(CmTask *pTask, int iterations)
{
   if (pTask == NULL)
   return CM_FAILURE;

   DWORD dwTimeOutMs = -1;
   UINT64 executionTime;

   cm_result_check(m_pCmQueue->Enqueue(pTask, e_Pipeline));
   cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
   unsigned long s = GetCurrentTimeInMilliseconds();

   for (int loop = 0; loop < iterations; loop++)
   {
      unsigned long start = GetCurrentTimeInMilliseconds();
      cm_result_check(m_pCmQueue->Enqueue(pTask, e_Pipeline));
      cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
      unsigned long end = GetCurrentTimeInMilliseconds();
      e_Pipeline->GetExecutionTime(executionTime);
      printf("Execution time=%dms, Kernel time=%.2fms\n", (end-start),
            executionTime/1000000.0);
   }
   unsigned long e = GetCurrentTimeInMilliseconds();

   if (iterations > 1)
   {
      unsigned long total = e - s;
      double average_time = total / (double) iterations;
      int PPM = 60000.0 / average_time;
      std::cout << "PPM = " << PPM << std::endl;
   }
   cm_result_check(m_pCmDev->FlushPrintBuffer());

   cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));

   return CM_SUCCESS;
}

/*****************************************************************************************************************************/
