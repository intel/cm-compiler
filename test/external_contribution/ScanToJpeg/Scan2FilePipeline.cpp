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

///////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Scan2FilePipeline.h"

#include "CMKernels.h"
#include "JpegEncoder.h"
#include "JpegDecoder.h"
#include "ImageColorConversion.h"

// Common interface
static unsigned long GetCurrentTimeInMilliseconds()
{
    struct timeval time;
    gettimeofday(&time, NULL);

    return (time.tv_sec * 1000.0) + time.tv_usec/1000.0;
}

JpegEncoder             *jpegEncoderNode;
JpegDecoder             *jpegDecoderNode;
Rgb2YCbCr               *rgb2YCbCrNode;
YCbCr2Rgb               *yCbCr2RgbNode;
ImageColorConversion    *imageCSCNode;

vector<GPUGraph> gpuGraph;

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Initialize CM device
Scan2FilePipeline::Scan2FilePipeline (void)
{
   m_fd = 0;
   m_vaDpy = NULL;
   gpuGraph.reserve(3);
}

int Scan2FilePipeline::VAInit()
{
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
}

int Scan2FilePipeline::EncodeInit(const char *filename, const int width, const int height,
      const int yuv_format)
{

   jpegEncoderNode = new JpegEncoder(m_vaDpy, yuv_format);
   rgb2YCbCrNode = new Rgb2YCbCr(m_vaDpy);
   imageCSCNode = new ImageColorConversion(m_vaDpy);

   if ((!jpegEncoderNode) || (!rgb2YCbCrNode) || (!imageCSCNode))
      return -1;


   m_PicWidth = width;
   m_PicHeight = height;
   m_yuvFormat = yuv_format;
   m_filename = filename;

   return 0;
}

int Scan2FilePipeline::DecodeInit(const char *filename)
{
   jpegDecoderNode = new JpegDecoder(m_vaDpy);
   imageCSCNode = new ImageColorConversion(m_vaDpy);
   yCbCr2RgbNode = new YCbCr2Rgb(m_vaDpy);

   if ((!jpegDecoderNode) || (!imageCSCNode) || (!yCbCr2RgbNode))
      return -1;

   m_filename = filename;

   return 0;
}

Scan2FilePipeline::~Scan2FilePipeline(void)
{
   for (auto &it : gpuGraph)
      it.node->Destroy(it.nodeSurfParam);

   delete jpegEncoderNode;
   delete jpegDecoderNode;
   delete rgb2YCbCrNode;
   delete yCbCr2RgbNode;
   delete imageCSCNode;

   //m_GpuTask.clear();
   gpuGraph.clear();

   close(m_fd);
   vaTerminate(m_vaDpy);
}

int Scan2FilePipeline::UploadInputImage(const char* filename, unsigned char *pDst,
      const int width, const int height, const int pitch, bool grayscale)
{
   int bytes_per_line = width;

   std::ifstream infile(filename, std::ios::in | std::ios::binary);
   if (!infile)
   {
      cout<<"File open failed "<<filename<<endl;
      return -1;
   }

   if (grayscale)
      bytes_per_line = width;
   else
      bytes_per_line = width * 3;

   unsigned char *pTmpInput = (unsigned char *)malloc(bytes_per_line);

   if (!pTmpInput)
   {
      infile.close();
      return -1;
   }

   for (int y = 0; y < height; y++)
   {
      infile.read((char *) pTmpInput, bytes_per_line);

      if (grayscale)
      {
         memcpy(pDst, pTmpInput, bytes_per_line);
         pDst += pitch;
      }
      else
      {
         for (int x = 0; x < width; x++)
         {
            pDst[x*4 + y*pitch] = pTmpInput[x*3];
            pDst[x*4 + y*pitch + 1] = pTmpInput[x*3 + 1];
            pDst[x*4 + y*pitch + 2] = pTmpInput[x*3 + 2];
         }
      }
   }

   infile.close();
   free(pTmpInput);

   return 0;

}

int Scan2FilePipeline::UploadJpegImage(const char *filename, unsigned char *& pDst,
      uint &jpegDataSize)
{
   std::ifstream infile(filename, std::ios::in | std::ios::binary);
   if (!infile)
   {
      cout<<"File open failed "<<filename<<endl;
      return -1;
   }

   infile.seekg(0, std::ios::end);
   jpegDataSize = infile.tellg();
   infile.seekg(0, std::ios::beg);

   pDst = (uchar *) malloc(jpegDataSize);

   if (pDst)
   {
      infile.read((char *)pDst, jpegDataSize);
      infile.close();
   }
   else
   {
      infile.close();
      return -1;
   }

   /* pDst will free in calling function */
   return 0;
}

int Scan2FilePipeline::WriteToFile(const char *filename, unsigned char *pSrc,
      const int width, const int height, const int pitch, bool grayscale)
{
   int bytes_per_line = width;

   std::ofstream outfile(filename, std::ios::out | std::ios::binary);

   if (!outfile)
   {
      cout<<"File creation failed "<<filename<<endl;
      return -1;
   }

   if (grayscale)
      bytes_per_line = width;
   else
      bytes_per_line = width * 3;

   unsigned char *pTmpOutput = (unsigned char *)malloc(bytes_per_line);

   if (!pTmpOutput)
   {
      outfile.close();
      return -1;
   }

   for (int y = 0; y < height; y++)
   {
      if (grayscale)
      {
         outfile.write((char *) pSrc, bytes_per_line);
      }
      else
      {
         unsigned char *tmpSrc = pSrc;

         for (int x = 0; x < width; x++)
         {
            pTmpOutput[x*3] = tmpSrc[x*4];
            pTmpOutput[x*3 + 1] = tmpSrc[x*4 + 1];
            pTmpOutput[x*3 + 2] = tmpSrc[x*4 + 2];
         }
         outfile.write((char *) pTmpOutput, bytes_per_line);
      }

      pSrc += pitch;
   }

   outfile.close();
   free(pTmpOutput);

}

void Scan2FilePipeline::Save2JPEG(const char *filename)
{
   if (jpegEncoderNode)
      jpegEncoderNode->WriteOut(m_VAEncodedSurfaceID, filename);
}

void Scan2FilePipeline::Save2Raw(const char *filename)
{
   GPUGraph lastNode = gpuGraph.back();
   Scan2FilePipeline *elem;

   if (lastNode.node != NULL)
   {
      elem = lastNode.node;

      elem->WriteOut(lastNode.nodeSurfParam, filename);
   }
}

int Scan2FilePipeline::CMSurface2DInfo(int width, int height, CM_SURFACE_FORMAT format,
        unsigned int & pitch, unsigned int & surface_size)
{
   rgb2YCbCrNode->GetSurface2DInfo(width, height, format, pitch, surface_size);
}


int Scan2FilePipeline::AssemblerCompressGraph(const int jpegQuality)
{
   CM_SURFACE_FORMAT cm_surface_format;
   unsigned int pitch_size = 0;
   unsigned int surface_size = 0;
   int status;


   if (m_yuvFormat == YUV444)  // YUV444
   {
      /* 
       * Hardware color format conversion doesn't support BGRX/BGRA to YUV444
       * Use a CM kernel for conversion
       */

      GPUGraph rgbtask;

      CHECK_STATUS(rgb2YCbCrNode->CreateInputSurface(m_filename, m_PicWidth, m_PicHeight,
            pitch_size, surface_size));

      /* 
       * CM not able to create surface to meet VA surface requirement.
       * As a result, use the VACreateSurface for surface need to input to
       * JpegEncoder, and the same VASurface can wrap to CM surface for output 
       * surface for YUV444 conversion kernel.
       */
      CHECK_STATUS(jpegEncoderNode->CreateSurfaces(m_PicWidth, m_PicHeight, pitch_size, &m_VAEncodedSurfaceID));

      CHECK_STATUS(rgb2YCbCrNode->WrapSurface(m_VAEncodedSurfaceID));

      CHECK_STATUS(rgb2YCbCrNode->PreRun(m_PicWidth, m_PicHeight));

      cm_surface_format = CM_SURFACE_FORMAT_444P;

      rgbtask.node = rgb2YCbCrNode;
      rgbtask.nodeSurfParam = m_VAEncodedSurfaceID;

      gpuGraph.push_back(rgbtask);
   }
   else if ((m_yuvFormat == YUV420) || (m_yuvFormat == YUV422))
   {
      /* 
       * Use hardware color format conversion to convert RGB to YUV420 or YUV422
       */

      unsigned int fourcc;
      VASurfaceID vaInputSurfaceID;
      GPUGraph csctask;

      if (m_yuvFormat == YUV420)
      {
         fourcc = VA_FOURCC_NV12;
         cm_surface_format = CM_SURFACE_FORMAT_NV12;

      }
      else
      {
         fourcc = VA_FOURCC_YUY2;
         cm_surface_format = CM_SURFACE_FORMAT_YUY2;
      }

      CHECK_STATUS(imageCSCNode->CreateInputSurfaces(m_filename, m_PicWidth, m_PicHeight, VA_FOURCC_BGRX,
            VA_RT_FORMAT_YUV420, &vaInputSurfaceID));
      CHECK_STATUS(imageCSCNode->CreateSurfaces(m_PicWidth, m_PicHeight, fourcc,
            VA_RT_FORMAT_YUV420, &m_VAEncodedSurfaceID));
      CHECK_STATUS(imageCSCNode->PreRun(vaInputSurfaceID, m_PicWidth, m_PicHeight,
            m_VAEncodedSurfaceID, m_PicWidth, m_PicHeight));

      csctask.node = imageCSCNode;
      csctask.nodeSurfParam = m_VAEncodedSurfaceID;

      gpuGraph.push_back(csctask);

   }
   else if (m_yuvFormat = YUV400)
   {
      CHECK_STATUS(jpegEncoderNode->CreateInputSurfaces(m_filename, m_PicWidth, m_PicHeight,
            &m_VAEncodedSurfaceID));

      cm_surface_format = CM_SURFACE_FORMAT_A8;
   }


   unsigned int outpitch = 0;
   unsigned int outsize = 0;

   /* Use a CM API to retrieve pitch and image size */
   CMSurface2DInfo(m_PicWidth, m_PicHeight, cm_surface_format,
         outpitch, outsize);
   /* Convert to JPEG file */
   CHECK_STATUS(jpegEncoderNode->PreRun(m_VAEncodedSurfaceID, m_PicWidth, m_PicHeight, jpegQuality,
         outsize));

   GPUGraph encodetask;
   encodetask.node = jpegEncoderNode;
   encodetask.nodeSurfParam = m_VAEncodedSurfaceID;
   gpuGraph.push_back(encodetask);

   return 0;
}

void Scan2FilePipeline::AssemblerDecompressGraph()
{
   GPUGraph decodetask;

   CHECK_STATUS(jpegDecoderNode->CreateInputSurfaces(m_filename, m_PicWidth, m_PicHeight,
         m_yuvFormat, &m_VADecodedSurfaceID));

   CHECK_STATUS(jpegDecoderNode->PreRun(m_VADecodedSurfaceID));

   decodetask.node = jpegDecoderNode;
   decodetask.nodeSurfParam = m_VADecodedSurfaceID;

   gpuGraph.push_back(decodetask);

   if ((m_yuvFormat == YUV420) || (m_yuvFormat == YUV422))
   {
      VASurfaceID rgbSurfaceID;
      GPUGraph    csctask;

      CHECK_STATUS(imageCSCNode->CreateSurfaces(m_PicWidth, m_PicHeight, VA_FOURCC_BGRX,
            VA_RT_FORMAT_YUV420, &rgbSurfaceID));

      CHECK_STATUS(imageCSCNode->PreRun(m_VADecodedSurfaceID, m_PicWidth, m_PicHeight,
            rgbSurfaceID, m_PicWidth, m_PicHeight));

      csctask.node = imageCSCNode;
      csctask.nodeSurfParam = rgbSurfaceID;

      gpuGraph.push_back(csctask);
   }
   else if (m_yuvFormat == YUV444)
   {
      GPUGraph ycbcrtask;

      CHECK_STATUS(yCbCr2RgbNode->WrapSurface(m_VADecodedSurfaceID));

      CHECK_STATUS(yCbCr2RgbNode->CreateOutputSurfaces(m_PicWidth, m_PicHeight));

      CHECK_STATUS(yCbCr2RgbNode->PreRun(m_PicWidth, m_PicHeight));

      ycbcrtask.node = yCbCr2RgbNode;
      ycbcrtask.nodeSurfParam = 0;

      gpuGraph.push_back(ycbcrtask);
   }
   else if (m_yuvFormat == YUV400)
   {
      // No conversion require
   }

   return;

}

// Execute GPU Graph
int Scan2FilePipeline::ExecuteCompressGraph(int iteration)
{
   unsigned long start = GetCurrentTimeInMilliseconds();
   for (int loop = 0; loop < iteration; loop++)
   {
      unsigned long starte = GetCurrentTimeInMilliseconds();

      for (auto &it : gpuGraph)
         it.node->Execute(it.nodeSurfParam);
   }
   unsigned long end = GetCurrentTimeInMilliseconds();

   if (iteration > 1)
   {
      unsigned long total = end - start;
      double average_time = total / (double) iteration;
      int PPM = 60000.0 / average_time;
      std::cout << "PPM = " << PPM << std::endl;
   }

   return 0;
}


// Execute GPU Graph
int Scan2FilePipeline::ExecuteDecompressGraph(int iteration)
{
   unsigned long start = GetCurrentTimeInMilliseconds();
   for (int loop = 0; loop < iteration; loop++)
   {
      for (auto &it : gpuGraph)
         it.node->Execute(it.nodeSurfParam);
   }
   unsigned long end = GetCurrentTimeInMilliseconds();

   if (iteration > 1)
   {
      unsigned long total = end - start;
      double average_time = total / (double) iteration;
      int PPM = 60000.0 / average_time;
      std::cout << "PPM = " << PPM << std::endl;
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
