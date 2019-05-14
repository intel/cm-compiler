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
#include <unistd.h>
#include <fcntl.h>
#include <va/va.h>
#include <va/va_drm.h>
#include <assert.h>
//#include "va_display.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Scan2FilePipeline.h"
#include "common/bitmap_helpers.h"
#include "common/cm_rt_helpers.h"

#include "Rgb2YCbCr.h"
#include "YCbCr2Rgb.h"

#define CM_Error_Handle(x) cm_result_check(x)

#define JPEG_YUV444OUT 0

// Common interface
static unsigned long GetCurrentTimeInMilliseconds()
{
    struct timeval time;
    gettimeofday(&time, NULL);

    return (time.tv_sec * 1000.0) + time.tv_usec/1000.0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Initialize CM device
Scan2FilePipeline::Scan2FilePipeline()
{
   m_PicWidth = 0;
   m_PicHeight = 0;

   m_pSrcR = NULL;
   m_pSrcG = NULL;
   m_pSrcB = NULL;
   m_pDstRGB = NULL;
   m_pCmDev = NULL;
   m_pCmQueue = NULL;
   e_Pipeline = NULL;
}

int Scan2FilePipeline::Init()
{
   UINT version = 0;
   int majorVer, minorVer;
   VAStatus vaStatus;

   /* Initialize the va driver */
   m_fd = open("/dev/dri/renderD128", O_RDWR);
   if (m_fd < 0)
   {
      cout<<"Error reading dri card0"<<endl;
      return -1;
   }

   m_vaDpy = vaGetDisplayDRM(m_fd);
   if (!m_vaDpy)
   {
      close(m_fd);
      cout<<"Error getting display"<<endl;
      return -1;
   }

   vaStatus = vaInitialize(m_vaDpy, &majorVer, &minorVer);
   assert(vaStatus == VA_STATUS_SUCCESS);

   // Create a CM Device
   cm_result_check(CreateCmDevice( m_pCmDev, version, m_vaDpy ));

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

   return CM_SUCCESS;
}

Scan2FilePipeline::~Scan2FilePipeline()
{
   //free(m_pCommonISA);
   delete m_jpegencoder;
   delete m_jpegdecoder;
   /* FIXME: rgbnode, and ycbcrnode haven't destroy */
   ::DestroyCmDevice(m_pCmDev);

   free(m_pSrcR);
   free(m_pSrcG);
   free(m_pSrcB);
   free(m_pDstRGB);

   close(m_fd);
   vaTerminate(m_vaDpy);
}

int Scan2FilePipeline::GetInputImage(const char* filename)
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

   GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8R8G8B8,
         &outpitch, &outsize);

   m_pDstRGB = (uchar *)CM_ALIGNED_MALLOC(outsize, 0x1000);

   return CM_SUCCESS;

}

int Scan2FilePipeline::Save2JPEG(const char* filename)
{
   if (m_jpegencoder)
      m_jpegencoder->WriteOut(m_VAEncodedSurfaceID, filename);
}

int Scan2FilePipeline::Save2Raw(const char* in_filename, const char* out_filename)
{
#if JPEG_YUV444OUT
   if (m_jpegdecoder)
      m_jpegdecoder->WriteOut(m_VADecodedSurfaceID, out_filename);
#else
   if (m_pDstRGB)
   {
      unsigned int outpitch = 0;
      unsigned int outsize = 0;

      auto input_image = cm::util::bitmap::BitMap::load(in_filename);
      auto output_image(input_image);
      unsigned output_image_size = m_PicWidth * m_PicHeight * output_image.getBPP()/8;
      output_image.setData(new unsigned char[output_image_size]);

      GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8R8G8B8,
         &outpitch, &outsize);

      unsigned char *tmp_dst1 = output_image.getData();
      unsigned char *tmp_dst2 = m_pDstRGB;

      for (int i = 0; i < m_PicWidth * m_PicHeight;) {
         tmp_dst1[0] = tmp_dst2[0];
         tmp_dst1[1] = tmp_dst2[1];
         tmp_dst1[2] = tmp_dst2[2];
         tmp_dst1 += 3;
         tmp_dst2 += 4;
         i++;
         if ((i % m_PicWidth) == 0)
            tmp_dst1 = &output_image.getData()[(i / m_PicWidth) * m_PicWidth * 3];

      }
      output_image.save(out_filename);
   }
#endif

}

int Scan2FilePipeline::GetSurface2DInfo(int width, int height, CM_SURFACE_FORMAT format,
        unsigned int * pitch, unsigned int * surface_size)
{
    m_pCmDev->GetSurface2DInfo(width, height, format, *pitch, *surface_size);
}

int Scan2FilePipeline::CreateKernel(char *isaFile, char* kernelName, CmKernel *& pKernel)
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

int Scan2FilePipeline::AddKernel(CmTask *pTask, CmKernel *pKernel)
{
   // Add kernel to task
   cm_result_check(pTask-> AddKernel(pKernel));
   cm_result_check(pTask-> AddSync());

   return CM_SUCCESS;
}

int Scan2FilePipeline::AssemblerCompressGraph(CmTask *& pTask, int jpegQuality)
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

   CmSurface2D *pDstYCbCr = NULL;
   SurfaceIndex *pSI_DstYCbCr = NULL;

   /* Need to retrieve fourcc from jpegencoder where it supported */
   VASurfaceAttrib fourcc[2];
   m_jpegencoder = new JpegEncoder(m_pCmDev, m_vaDpy);
   m_jpegencoder->GetVASurfaceAttrib(fourcc);

   /* Need to use vaCreateSurface instead of CM CreateSurface2D due to 
    * CM CreteSurface2D won't able to create an RGBA surface with YUV444 render
    * target
    */
   int status = vaCreateSurfaces(m_vaDpy, VA_RT_FORMAT_YUV444, m_PicWidth,
         m_PicHeight, &m_VAEncodedSurfaceID, 1, &fourcc[0], 2);
   cm_result_check(m_pCmDev->CreateSurface2D(m_VAEncodedSurfaceID, pDstYCbCr));
   cm_result_check(pDstYCbCr->GetIndex(pSI_DstYCbCr));

   // RGB To YCbCr node
   Rgb2YCbCr *rgbnode = new Rgb2YCbCr(m_pCmDev);
   CmKernel *pRgb2YCbCrKernel;
   cm_result_check(CreateKernel(rgbnode->GetIsa(), rgbnode->GetKernelName(), pRgb2YCbCrKernel));
   AddKernel(pTask, pRgb2YCbCrKernel);

   rgbnode->PreRun(pRgb2YCbCrKernel, pSI_SrcR, pSI_SrcG, pSI_SrcB, pSI_DstYCbCr, m_PicWidth, m_PicHeight);

   unsigned int outpitch = 0;
   unsigned int outsize = 0;

   /* Render target should be YUV444, as a result, the coded buffer size after
    * running jpeg encoded need to use YUV444 format size
    */
   GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_444P,
         &outpitch, &outsize);
   /* Convert to JPEG file */
   m_jpegencoder->PreRun(m_VAEncodedSurfaceID, m_PicWidth, m_PicHeight, jpegQuality,
         outsize);
}

int Scan2FilePipeline::AssemblerDecompressGraph(CmTask *& pTask)
{
   VASurfaceAttrib fourcc;
   int surface_type;
   unsigned int imgWidth, imgHeight;

   if (pTask == NULL)
      cm_result_check(m_pCmDev->CreateTask(pTask));

   m_jpegdecoder = new JpegDecoder(m_pCmDev, m_vaDpy);

   assert(m_compressJpegData && m_compressJpegDataSize);

   m_jpegdecoder->ParseHeader(m_compressJpegData, m_compressJpegDataSize);

   imgWidth = m_jpegdecoder->GetPicWidth();
   imgHeight = m_jpegdecoder->GetPicHeight();

   m_jpegdecoder->GetVASurfaceAttrib(&fourcc, &surface_type);

   /* Convert to JPEG file */
   /* Need to use vaCreateSurface instead of CM CreateSurface2D due to 
    * CM CreteSurface2D won't able to create an RGBA surface with YUV444 render
    * target
    */
   CmSurface2D *pSrcYCbCr = NULL;
   SurfaceIndex *pSI_SrcYCbCr = NULL;
   int status = vaCreateSurfaces(m_vaDpy, surface_type, imgWidth,
         imgHeight, &m_VADecodedSurfaceID, 1, &fourcc, 2);
   cm_result_check(m_pCmDev->CreateSurface2D(m_VADecodedSurfaceID, pSrcYCbCr));
   cm_result_check(pSrcYCbCr->GetIndex(pSI_SrcYCbCr));

   m_jpegdecoder->PreRun();

   /* YCbCr to RGB node */
   CmSurface2DUP *pDstRgb = NULL;
   SurfaceIndex *pSI_DstRgb = NULL;

   /* Output surface to store after convert from YCbCr to RGB */
   cm_result_check(m_pCmDev->CreateSurface2DUP(imgWidth, imgHeight,
            CM_SURFACE_FORMAT_A8R8G8B8, m_pDstRGB, pDstRgb));
   cm_result_check(pDstRgb->GetIndex(pSI_DstRgb));

   YCbCr2Rgb *ycbcrnode = new YCbCr2Rgb(m_pCmDev);
   CmKernel *pYCbCr2RgbKernel;
   cm_result_check(CreateKernel(ycbcrnode->GetIsa(), ycbcrnode->GetKernelName(), pYCbCr2RgbKernel));
   AddKernel(pTask, pYCbCr2RgbKernel);

   /* The input surface to the kernel is the output from JPEG decoder */
   ycbcrnode->PreRun(pYCbCr2RgbKernel, pSI_SrcYCbCr,  pSI_DstRgb, m_PicWidth, m_PicHeight);

}


// Execute GPU Graph
int Scan2FilePipeline::ExecuteCompressGraph(CmTask *pTask, int iteration)
{
   if (pTask == NULL)
      return CM_FAILURE;

   DWORD dwTimeOutMs = -1;
   UINT64 executionTime;

   unsigned long start = GetCurrentTimeInMilliseconds();
   for (int loop = 0; loop < iteration; loop++)
   {
      unsigned long starte = GetCurrentTimeInMilliseconds();
      /* Run RGBtoYCbCr CM kernel */
      cm_result_check(m_pCmQueue->Enqueue(pTask, e_Pipeline));
      cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
      e_Pipeline->GetExecutionTime(executionTime);

      /* Run JPEG encoder */
      m_jpegencoder->Run(m_VAEncodedSurfaceID);
      m_jpegencoder->GetCodedBufferAddress(m_VAEncodedSurfaceID, m_compressJpegData,
            m_compressJpegDataSize);

      unsigned long ende = GetCurrentTimeInMilliseconds();
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

// Execute GPU Graph
int Scan2FilePipeline::ExecuteDecompressGraph(CmTask *pTask, int iteration)
{
   if (pTask == NULL)
      return CM_FAILURE;

   DWORD dwTimeOutMs = -1;
   UINT64 executionTime;

   unsigned long start = GetCurrentTimeInMilliseconds();
   for (int loop = 0; loop < iteration; loop++)
   {
      unsigned long starte = GetCurrentTimeInMilliseconds();

      /* Run JPEG decoder.  This is blocking call until decoder complete */
      m_jpegdecoder->Run(m_VADecodedSurfaceID);

      /* Run YCbCrtoRGB CM kernel */
      cm_result_check(m_pCmQueue->Enqueue(pTask, e_Pipeline));
      cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
      e_Pipeline->GetExecutionTime(executionTime);

      unsigned long ende = GetCurrentTimeInMilliseconds();
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
