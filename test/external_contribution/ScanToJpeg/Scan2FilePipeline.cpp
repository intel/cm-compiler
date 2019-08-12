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
#include <fstream>
//#include "va_display.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Scan2FilePipeline.h"
#include "common/cm_rt_helpers.h"

#include "Rgb2Encode.h"
#include "YCbCr2Rgb.h"

#define CM_Error_Handle(x) cm_result_check(x)

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

   m_pSrc0 = NULL;
   m_pSrc1 = NULL;
   m_pSrc2 = NULL;
   m_pDst  = NULL;
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

   free(m_pSrc0);
   free(m_pSrc1);
   free(m_pSrc2);
   free(m_pDst);

   close(m_fd);
   vaTerminate(m_vaDpy);
}

int Scan2FilePipeline::GetInputImage(const char* filename, const int width,
      const int height, const int yuv_format)
{

   CM_SURFACE_FORMAT cm_output_surface_format;
   unsigned int outpitch = 0;
   unsigned int outsize = 0;

   m_PicWidth = width;
   m_PicHeight = height;
   m_yuvFormat = yuv_format;

   unsigned int pitch_size = 0;
   unsigned int surface_size = 0;
   GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8,
         &pitch_size, &surface_size);

   std::ifstream infile(filename, std::ios::in | std::ios::binary);
   if (!infile)
   {
      return -1;
   }

   unsigned char *pTmpInput = 0;


   if (m_yuvFormat == 0) // RGB input 24bit 8bpp
   {
      m_pSrc0 = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
      m_pSrc1 = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
      m_pSrc2 = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);

      pTmpInput = (uchar *) malloc(m_PicWidth*3);

      memset(m_pSrc0, 0, surface_size);
      memset(m_pSrc1, 0, surface_size);
      memset(m_pSrc2, 0, surface_size);

      for (int y = 0; y < m_PicHeight; y++)
      {
         infile.read((char*)pTmpInput, m_PicWidth*3);

         for (int x = 0; x < m_PicWidth; x++)
         {
            m_pSrc0[x + y*pitch_size] = pTmpInput[x*3];
            m_pSrc1[x + y*pitch_size] = pTmpInput[x*3 + 1];
            m_pSrc2[x + y*pitch_size] = pTmpInput[x*3 + 2];
         }
      }

      cm_output_surface_format = CM_SURFACE_FORMAT_A8R8G8B8;
   }
   else  // Grayscale input
   {
      m_pSrc0 = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
      pTmpInput = (uchar *) malloc(m_PicWidth);
      memset(m_pSrc0, 0, surface_size);

      for (int y = 0; y < m_PicHeight; y++)
      {
         infile.read((char*)pTmpInput, m_PicWidth);

         for (int x = 0; x < m_PicWidth; x++)
         {
            m_pSrc0[x + y*pitch_size] = pTmpInput[x];
         }
         //pTmpInput +=m_PicWidth;
      }
      cm_output_surface_format = CM_SURFACE_FORMAT_A8;
   }

   infile.close();

   free(pTmpInput);

   GetSurface2DInfo(m_PicWidth, m_PicHeight, cm_output_surface_format,
      &outpitch, &outsize);
   m_pDst = (uchar *)CM_ALIGNED_MALLOC(outsize, 0x1000);

   return CM_SUCCESS;

}

void Scan2FilePipeline::Save2JPEG(const char* filename)
{
   if (m_jpegencoder)
      m_jpegencoder->WriteOut(m_VAEncodedSurfaceID, filename);
}

void Scan2FilePipeline::Save2Raw(const char* out_filename)
{
   if (out_filename != NULL)
   {
      if (m_yuvFormat == 0)
      {
         // RGB input image require to use CM kernel to do conversion from
         // YUV444 to RGB after HW JPEG decode
         // m_pDst is output surface after CM kernel conversion
         if (m_pDst)
         {
            unsigned int outpitch =0;
            unsigned int outsize = 0;

            std::ofstream outfile(out_filename, std::ios::out | std::ios::binary);

            GetSurface2DInfo(m_PicWidth, m_PicHeight,
                  CM_SURFACE_FORMAT_A8R8G8B8, &outpitch, &outsize);

            uchar *tmp_dst1 = (uchar *) malloc (outpitch);
            uchar *tmp_dst2 = m_pDst;
            int bytes_per_line = m_PicWidth * 3;

            for (int y = 0; y < m_PicHeight; y++)
            {
               for (int x = 0; x < m_PicWidth; x++)
               {
                  tmp_dst1[x*3] = tmp_dst2[x*4];
                  tmp_dst1[x*3+1] = tmp_dst2[x*4+1];
                  tmp_dst1[x*3+2] = tmp_dst2[x*4+2];
               }

               outfile.write((char *) tmp_dst1, bytes_per_line);
               tmp_dst2 += outpitch;
            }

            free(tmp_dst1);
         }

      }
      else if (m_yuvFormat == 1)
      {
         // No format conversion after HW JPEG decode
         // The decoded file can save out directly from Jpeg decoded surface
         m_jpegdecoder->WriteOut(m_VADecodedSurfaceID, out_filename);
      }
   }
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

void Scan2FilePipeline::AddKernel(CmTask *pTask, CmKernel *pKernel)
{
   // Add kernel to task
   cm_result_check(pTask-> AddKernel(pKernel));
   cm_result_check(pTask-> AddSync());
}

void Scan2FilePipeline::AssemblerCompressGraph(CmTask *& pTask, int jpegQuality)
{
   CM_SURFACE_FORMAT cm_surface_format;
   unsigned int pitch_size = 0;
   unsigned int surface_size = 0;
   GetSurface2DInfo(m_PicWidth, m_PicHeight, CM_SURFACE_FORMAT_A8,
         &pitch_size, &surface_size);

   m_jpegencoder = new JpegEncoder(m_pCmDev, m_vaDpy, m_yuvFormat);

   if (m_yuvFormat == 0)  // YUV444
   {
      // Source surface
      CmSurface2DUP *pSrcR = NULL;
      CmSurface2DUP *pSrcG = NULL;
      CmSurface2DUP *pSrcB = NULL;
      SurfaceIndex *pSI_SrcR = NULL;
      SurfaceIndex *pSI_SrcG = NULL;
      SurfaceIndex *pSI_SrcB = NULL;

      cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pSrc0, pSrcR));
      cm_result_check(pSrcR->GetIndex(pSI_SrcR));
      cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pSrc1, pSrcG));
      cm_result_check(pSrcG->GetIndex(pSI_SrcG));
      cm_result_check(m_pCmDev->CreateSurface2DUP(m_PicWidth, m_PicHeight,
            CM_SURFACE_FORMAT_A8, m_pSrc2, pSrcB));
      cm_result_check(pSrcB->GetIndex(pSI_SrcB));

      CmSurface2D *pSrcJpegEncode = NULL;
      SurfaceIndex *pSI_SrcJpegEncode = NULL;

      m_jpegencoder->CreateSurfaces(m_PicWidth, m_PicHeight, pitch_size,
            NULL, &m_VAEncodedSurfaceID);

      cm_result_check(m_pCmDev->CreateSurface2D(m_VAEncodedSurfaceID,
            pSrcJpegEncode));
      cm_result_check(pSrcJpegEncode->GetIndex(pSI_SrcJpegEncode));

      if (pTask == NULL)
         cm_result_check(m_pCmDev->CreateTask(pTask));

      // Pixel conversion using CM from RGB to YCbCr or RGB to grayscale
      Rgb2Encode *rgbnode = new Rgb2Encode(m_pCmDev);
      CmKernel *pRgb2EncodeKernel;
      cm_result_check(CreateKernel(rgbnode->GetIsa(), rgbnode->GetKernelName(m_yuvFormat), pRgb2EncodeKernel));
      AddKernel(pTask, pRgb2EncodeKernel);

      rgbnode->PreRun(pRgb2EncodeKernel, pSI_SrcR, pSI_SrcG, pSI_SrcB, pSI_SrcJpegEncode, m_PicWidth, m_PicHeight);

      cm_surface_format = CM_SURFACE_FORMAT_444P;
   }
   else
   {
      /* There is issue wrap a VA Surface for single channel 8bit format like
       * YUV400 to CM surface.  As a result require extra CPU copy to get
       * the input for grayscale.  
       *
       * The proper way for different pixel format is like
       * 1) Use vaCreateSurfaces to create a VA Surface
       * 2) VA Surface -> CM Surface
       *     CreateSurfac2D(vaSurfaceID, CmSurface2D)
       *     CmSurface2D->GetIndex(cmSurfaceIndex)
       */
      m_jpegencoder->CreateSurfaces(m_PicWidth, m_PicHeight, pitch_size,
            m_pSrc0, &m_VAEncodedSurfaceID);

      cm_surface_format = CM_SURFACE_FORMAT_A8;
   }


   unsigned int outpitch = 0;
   unsigned int outsize = 0;

   GetSurface2DInfo(m_PicWidth, m_PicHeight, cm_surface_format,
         &outpitch, &outsize);
   /* Convert to JPEG file */
   m_jpegencoder->PreRun(m_VAEncodedSurfaceID, m_PicWidth, m_PicHeight, jpegQuality,
         outsize);
}

void Scan2FilePipeline::AssemblerDecompressGraph(CmTask *& pTask)
{
   VASurfaceAttrib fourcc;
   int surface_type;
   unsigned int imgWidth, imgHeight;

   m_jpegdecoder = new JpegDecoder(m_pCmDev, m_vaDpy);

   assert(m_compressJpegData && m_compressJpegDataSize);

   m_jpegdecoder->ParseHeader(m_compressJpegData, m_compressJpegDataSize);

   imgWidth = m_jpegdecoder->GetPicWidth();
   imgHeight = m_jpegdecoder->GetPicHeight();

   /* Convert to JPEG file */
   /* Need to use vaCreateSurface instead of CM CreateSurface2D due to 
    * CM CreteSurface2D won't able to create an RGBA surface with YUV444 render
    * target
    */

   m_jpegdecoder->CreateSurfaces(imgWidth, imgHeight, &m_VADecodedSurfaceID);

   /*
   m_jpegdecoder->GetVASurfaceAttrib(&fourcc, &surface_type);

   vaCreateSurfaces(m_vaDpy, surface_type, imgWidth,
         imgHeight, &m_VADecodedSurfaceID, 1, &fourcc, 2);
   */
   m_jpegdecoder->PreRun(m_VADecodedSurfaceID);

   if (m_yuvFormat == 0)
   {
      if (pTask == NULL)
         cm_result_check(m_pCmDev->CreateTask(pTask));

      CmSurface2D *pSrcYCbCr = NULL;
      SurfaceIndex *pSI_SrcYCbCr = NULL;

      cm_result_check(m_pCmDev->CreateSurface2D(m_VADecodedSurfaceID, pSrcYCbCr));
      cm_result_check(pSrcYCbCr->GetIndex(pSI_SrcYCbCr));

      /* YCbCr to RGB node */
      CmSurface2DUP *pDstRgb = NULL;
      SurfaceIndex *pSI_DstRgb = NULL;

      /* Output surface to store after convert from YCbCr to RGB */
      cm_result_check(m_pCmDev->CreateSurface2DUP(imgWidth, imgHeight,
            CM_SURFACE_FORMAT_A8R8G8B8, m_pDst, pDstRgb));
      cm_result_check(pDstRgb->GetIndex(pSI_DstRgb));

      YCbCr2Rgb *ycbcrnode = new YCbCr2Rgb(m_pCmDev);
      CmKernel *pYCbCr2RgbKernel;
      cm_result_check(CreateKernel(ycbcrnode->GetIsa(), ycbcrnode->GetKernelName(), pYCbCr2RgbKernel));
      AddKernel(pTask, pYCbCr2RgbKernel);

      /* The input surface to the kernel is the output from JPEG decoder */
      ycbcrnode->PreRun(pYCbCr2RgbKernel, pSI_SrcYCbCr,  pSI_DstRgb, m_PicWidth, m_PicHeight);
   }

}


// Execute GPU Graph
int Scan2FilePipeline::ExecuteCompressGraph(CmTask *pCMTask, int iteration)
{
   DWORD dwTimeOutMs = -1;
   UINT64 executionTime=0;

   unsigned long start = GetCurrentTimeInMilliseconds();
   for (int loop = 0; loop < iteration; loop++)
   {
      unsigned long starte = GetCurrentTimeInMilliseconds();
      /* Run RGBtoYCbCr CM kernel */
      if (pCMTask != NULL)
      {
         cm_result_check(m_pCmQueue->Enqueue(pCMTask, e_Pipeline));
         cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
         e_Pipeline->GetExecutionTime(executionTime);
      }

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

   if (pCMTask != NULL)
   {
      cm_result_check(m_pCmDev->FlushPrintBuffer());

      cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));

      cm_result_check(m_pCmQueue->DestroyEvent(e_Pipeline));

      cm_result_check(m_pCmDev->DestroyTask(pCMTask));
   }

   return CM_SUCCESS;
}

// Execute GPU Graph
int Scan2FilePipeline::ExecuteDecompressGraph(CmTask *pCMTask, int iteration)
{
   DWORD dwTimeOutMs = -1;
   UINT64 executionTime=0;

   unsigned long start = GetCurrentTimeInMilliseconds();
   for (int loop = 0; loop < iteration; loop++)
   {
      unsigned long starte = GetCurrentTimeInMilliseconds();

      /* Run JPEG decoder.  This is blocking call until decoder complete */
      m_jpegdecoder->Run(m_VADecodedSurfaceID);

      if (pCMTask != NULL)
      {
         /* Run YCbCrtoRGB CM kernel */
         cm_result_check(m_pCmQueue->Enqueue(pCMTask, e_Pipeline));
         cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
         e_Pipeline->GetExecutionTime(executionTime);
      }

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

   if (pCMTask != NULL)
   {
      cm_result_check(m_pCmDev->FlushPrintBuffer());

      cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));

      cm_result_check(m_pCmQueue->DestroyEvent(e_Pipeline));

      cm_result_check(m_pCmDev->DestroyTask(pCMTask));
   }

   return CM_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
