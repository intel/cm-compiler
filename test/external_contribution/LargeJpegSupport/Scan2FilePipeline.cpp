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
#include "common/cm_rt_helpers.h"

#include "Rgb2Encode.h"
#include "YCbCr2Rgb.h"

#define TILE_WIDTH_DIVIDER 2

#define CM_Error_Handle(x) cm_result_check(x)

#define GALIGN(v, alignment) (((v) + (alignment) - 1) & (~((alignment) - 1)))

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

   m_pRawDst  = NULL;
   m_pRawSrc  = NULL;
   m_pCmDev = NULL;
   m_pCmQueue = NULL;
   e_Pipeline = NULL;

   m_pCompressTask[0] = NULL;
   m_pCompressTask[1] = NULL;
   m_pDeCompressTask[0] = NULL;
   m_pDeCompressTask[1] = NULL;

   m_jpegcombine = NULL;
   m_jpegencoder[0] = NULL;
   m_jpegencoder[1] = NULL;
   m_jpegdecoder[0] = NULL;
   m_jpegdecoder[1] = NULL;
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

   /* Create a CM Device */
   cm_result_check(CreateCmDevice( m_pCmDev, version, m_vaDpy ));

   /* For testing only - collect platform information */
   int nGPUPlatform = 0;
   int nGTPlatform = 0;
   int nGPUFrequency = 0;
   char* sGPUPlatform[] = {"Unk", "SNB", "IVB", "HSW", "BDW", "VLV", "CHV", "SKL", "BXT", "CNL", "KBL", "GLK"};
   char* sGT[] = {"Unk", "GT1", "GT2", "GT3", "GT4"};
   size_t size = 4;

   cm_result_check(m_pCmDev->GetCaps( CAP_GPU_PLATFORM, size, &nGPUPlatform ));
   cm_result_check(m_pCmDev->GetCaps( CAP_GT_PLATFORM, size, &nGTPlatform ));
   cm_result_check(m_pCmDev->GetCaps( CAP_GPU_CURRENT_FREQUENCY, size, &nGPUFrequency ));

   /* Create a task queue */
   cm_result_check(m_pCmDev->CreateQueue(m_pCmQueue));

   /* Inits the print buffer */
   cm_result_check(m_pCmDev->InitPrintBuffer());

   return CM_SUCCESS;
}

Scan2FilePipeline::~Scan2FilePipeline()
{
   if (m_pDeCompressTask[0] != NULL)
      cm_result_check(m_pCmDev->DestroyTask(m_pDeCompressTask[0]));
   if (m_pDeCompressTask[1] != NULL)
      cm_result_check(m_pCmDev->DestroyTask(m_pDeCompressTask[1]));

   if (m_pCompressTask[0] != NULL)
      cm_result_check(m_pCmDev->DestroyTask(m_pCompressTask[0]));
   if (m_pCompressTask[1] != NULL)
      cm_result_check(m_pCmDev->DestroyTask(m_pCompressTask[1]));

   delete m_jpegencoder[0];
   delete m_jpegdecoder[0];
   if (m_jpegencoder[1]) delete m_jpegencoder[1];
   if (m_jpegdecoder[1]) delete m_jpegdecoder[1];

   free(m_pSrc0[0]);
   free(m_pSrc1[0]);
   free(m_pSrc2[0]);
   if (m_pSrc0[1]) free(m_pSrc0[1]);
   if (m_pSrc1[1]) free(m_pSrc1[1]);
   if (m_pSrc2[1]) free(m_pSrc2[1]);

   if (m_pRawDst) free(m_pRawDst);

   ::DestroyCmDevice(m_pCmDev);

   close(m_fd);
   vaTerminate(m_vaDpy);
}

int Scan2FilePipeline::GetRawInputImage(const char* filename, const int width,
      const int height, const int yuv_format)
{
   m_PicWidth = width;
   m_PicHeight = height;
   m_yuvFormat = yuv_format;

   std::ifstream infile(filename, std::ios::in | std::ios::binary);
   if (!infile)
   {
      return -1;
   }

   uint image_size;
   if (m_yuvFormat == 0)
   {
      /* RGB input 24bit */
      image_size = m_PicWidth * m_PicHeight * 3;
   }
   else
   {
      /* Grayscale input 8bpp */
      image_size = m_PicWidth * m_PicHeight;
   }

   m_pRawSrc = (uchar *)malloc(image_size);
   infile.read((char*)m_pRawSrc, image_size);
   infile.close();

   return 0;

}

int Scan2FilePipeline::GetJpegInputImage(const char *filename, const int yuv_format)
{
   m_yuvFormat = yuv_format;

   std::ifstream infile(filename, std::ios::in | std::ios::binary);
   if (!infile)
   {
      return -1;
   }

   std::streampos filesize;

   infile.seekg(0, std::ios::end);
   filesize = infile.tellg();
   infile.seekg(0, std::ios::beg);

   m_pJpegSrc.resize(filesize);
   infile.read((char*) &m_pJpegSrc[0], filesize);

   return 0;
}


void Scan2FilePipeline::Save2JPEG(const char* filename)
{
   /* 
    * If tileHeight != picture height, the input image will break into multiple
    * tiles.  And this will require jpegcombine function to combines all the
    * bitstream into single Jpeg file.
    * If tileHeight == picture height, no combine function, just have m_jpegencoder[0] 
    * to write out, and there won't have any jpegencoder[1]
    */
   if (filename != NULL)
   {
      if (m_PicHeight == m_PicTileHeight)
      {
         if (m_jpegencoder[0])
            m_jpegencoder[0]->WriteOut(m_VAEncodedSurfaceID[0], filename);
      }
      else
      {
         if (m_jpegcombine)
            m_jpegcombine->WriteOut(filename);
      }
   }
}

void Scan2FilePipeline::Save2Raw(const char* out_filename)
{
   if (out_filename != NULL)
   {
      /* 
       * For input image without divide into multiple tiles and grayscale, 
       * use the JpegDecoder WriteOut function to dump the decoded raw image 
       * into file. 
       * Otherwise, for non grayscale input image, will need to CM kernel YUV444
       * to RGB conversion, and will save it to m_pRawDst.
       * If input image divided into tiles, each tile raw image will combine
       * into m_pRawDst as well.
       */
      if (m_jpegdecoder[0])
      {
         uint imgWidth = m_jpegdecoder[0]->GetPicWidth();
         uint imgHeight = m_jpegdecoder[0]->GetPicHeight();

//         if (imgHeight == m_PicTileHeight)
         if ((m_yuvFormat == 1) && (imgHeight == m_PicTileHeight))
         {
            // 
            if (m_jpegdecoder[0])
               m_jpegdecoder[0]->WriteOut(m_VADecodedSurfaceID[0], out_filename);
         }
         else
         {
            std::ofstream outfile(out_filename, std::ios::out | std::ios::binary);
            uint imageSize;

            if (m_yuvFormat == 0)
               imageSize = imgWidth * imgHeight * 4;
            else
               imageSize = imgWidth * imgHeight;

            outfile.write((char *) m_pRawDst,  imageSize);

            outfile.close();
         }
      }
   }
}

int Scan2FilePipeline::GetSurface2DInfo(int width, int height, CM_SURFACE_FORMAT format,
        uint * pitch, uint * surface_size)
{
    m_pCmDev->GetSurface2DInfo(width, height, format, *pitch, *surface_size);
}

int Scan2FilePipeline::CreateKernel(char *isaFile, char* kernelName, CmKernel *& pKernel)
{
   int         nKernelSize;
   uchar       *pCommonISA;
   CmProgram   *pProgram;

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

   /* cm_result_check(m_pCmDev->LoadProgram(pCommonISA, nKernelSize, pProgram, "nojitter")); */
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

int Scan2FilePipeline::CompressGraphSetup(int width, int height, int yuvformat, int jpegquality, CmTask *& pTask, JpegEncoder *pJpegEncoder, int blkidx)
{
   CM_SURFACE_FORMAT cm_surface_format;
   uint pitch_size = 0;
   uint surface_size = 0;

   GetSurface2DInfo(width, height, CM_SURFACE_FORMAT_A8, &pitch_size, &surface_size);

   U8 *src0, *src1, *src2;

   if (yuvformat == 0)
   {
      /* YUV444 */
      src0 = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
      src1 = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);
      src2 = (uchar *)CM_ALIGNED_MALLOC(surface_size, 0x1000);

      memset(src0, 0, surface_size);
      memset(src1, 0, surface_size);
      memset(src2, 0, surface_size);

      CmSurface2DUP *pSrcR = NULL;
      CmSurface2DUP *pSrcG = NULL;
      CmSurface2DUP *pSrcB = NULL;
      SurfaceIndex *pSI_SrcR = NULL;
      SurfaceIndex *pSI_SrcG = NULL;
      SurfaceIndex *pSI_SrcB = NULL;

      cm_result_check(m_pCmDev->CreateSurface2DUP(width, height,
            CM_SURFACE_FORMAT_A8, src0, pSrcR));
      cm_result_check(pSrcR->GetIndex(pSI_SrcR));
      cm_result_check(m_pCmDev->CreateSurface2DUP(width, height,
            CM_SURFACE_FORMAT_A8, src1, pSrcG));
      cm_result_check(pSrcG->GetIndex(pSI_SrcG));
      cm_result_check(m_pCmDev->CreateSurface2DUP(width, height,
            CM_SURFACE_FORMAT_A8, src2, pSrcB));
      cm_result_check(pSrcB->GetIndex(pSI_SrcB));

      CmSurface2D *pSrcJpegEncode = NULL;
      SurfaceIndex *pSI_SrcJpegEncode = NULL;

      pJpegEncoder->CreateVASurfaces(width, height, &m_VAEncodedSurfaceID[blkidx]);

      cm_result_check(m_pCmDev->CreateSurface2D(m_VAEncodedSurfaceID[blkidx],
            pSrcJpegEncode));
      cm_result_check(pSrcJpegEncode->GetIndex(pSI_SrcJpegEncode));

      if (pTask == NULL)
         cm_result_check(m_pCmDev->CreateTask(pTask));

      /* Pixel conversion using CM from RGB to YCbCr */
      Rgb2Encode *rgbnode = new Rgb2Encode(m_pCmDev);
      CmKernel *pRgb2EncodeKernel;
      cm_result_check(CreateKernel(rgbnode->GetIsa(), rgbnode->GetKernelName(m_yuvFormat), pRgb2EncodeKernel));
      AddKernel(pTask, pRgb2EncodeKernel);

      rgbnode->PreRun(pRgb2EncodeKernel, pSI_SrcR, pSI_SrcG, pSI_SrcB, pSI_SrcJpegEncode, width, height);

      cm_surface_format = CM_SURFACE_FORMAT_444P;

      m_pSrc0[blkidx] = src0;
      m_pSrc1[blkidx] = src1;
      m_pSrc2[blkidx] = src2;
   }
   else
   {
      /* For grayscale input image, no pixel conversion required */

      pJpegEncoder->CreateVASurfaces(width, height, &m_VAEncodedSurfaceID[blkidx]);

      cm_surface_format = CM_SURFACE_FORMAT_A8;
   }

   uint outpitch = 0;
   uint outsize = 0;

   GetSurface2DInfo(width, height, cm_surface_format, &outpitch, &outsize);
   /* Setup function to convert to JPEG file */
   pJpegEncoder->PreRun(m_VAEncodedSurfaceID[blkidx], width, height, jpegquality,
         m_McuRestartInterval, outsize);
}

void Scan2FilePipeline::AssemblerCompressGraph(int jpegQuality,  const int tileHeight)
{
   m_jpegencoder[0] = new JpegEncoder(m_pCmDev, m_vaDpy, m_yuvFormat);

   m_PicTileHeight = tileHeight;
   m_PicTileWidth = m_PicWidth;


   if ((m_PicTileHeight != m_PicHeight) && (m_PicWidth > 16*1024))
   {
      m_PicTileWidth = m_PicWidth / 2;
   }
   else if (m_PicTileHeight == m_PicHeight)
   {
      if (m_PicWidth < 16*1024)
         m_PicTileWidth = m_PicWidth;
      else
         throw std::out_of_range("Please specific tileheight for image width > 16384");
   }

   if (m_PicTileHeight != m_PicHeight)
   {
      int rowBlock = (m_PicHeight + m_PicTileHeight - 1) / m_PicTileHeight;
      int colBlock = (m_PicWidth + m_PicTileWidth - 1) / m_PicTileWidth;

      m_McuRestartInterval = m_PicTileWidth/8;
      m_jpegcombine = new JpegCombine(colBlock, rowBlock, m_McuRestartInterval,
            m_PicWidth, m_PicHeight);
   }
   else
   {
      m_McuRestartInterval = 0;
   }

   CompressGraphSetup(m_PicTileWidth, m_PicTileHeight, m_yuvFormat, jpegQuality,
         m_pCompressTask[0], m_jpegencoder[0], 0);

   if (m_PicHeight % m_PicTileHeight)
   {
      int remaining_row = m_PicHeight % m_PicTileHeight;
      m_jpegencoder[1] = new JpegEncoder(m_pCmDev, m_vaDpy, m_yuvFormat);

      CompressGraphSetup(m_PicTileWidth, remaining_row, m_yuvFormat,
               jpegQuality, m_pCompressTask[1], m_jpegencoder[1], 1);
   }
}

int Scan2FilePipeline::DeCompressGraphSetup(int width, int height, int yuvformat, CmTask *& pTask, JpegDecoder *pJpegDecoder, int blkidx)
{
   m_jpegdecoder[blkidx]->CreateSurfaces(width, height, &m_VADecodedSurfaceID[blkidx]);
   m_jpegdecoder[blkidx]->PreRun(m_VADecodedSurfaceID[blkidx]);

   if (yuvformat == 0)  // YUV444
   {
      if (pTask == NULL)
         cm_result_check(m_pCmDev->CreateTask(pTask));

      CmSurface2D *pSrcYCbCr = NULL;
      SurfaceIndex *pSI_SrcYCbCr = NULL;

      cm_result_check(m_pCmDev->CreateSurface2D(m_VADecodedSurfaceID[blkidx], pSrcYCbCr));
      cm_result_check(pSrcYCbCr->GetIndex(pSI_SrcYCbCr));

      /* YCbCr to RGB node */
      SurfaceIndex *pSI_DstRgb = NULL;

      /* Output surface to store after convert from YCbCr to RGB */
      cm_result_check(m_pCmDev->CreateSurface2D(width, height, CM_SURFACE_FORMAT_X8R8G8B8, 
               m_pDstRgb[blkidx]));

      cm_result_check(m_pDstRgb[blkidx]->GetIndex(pSI_DstRgb));

      YCbCr2Rgb *ycbcrnode = new YCbCr2Rgb(m_pCmDev);
      CmKernel *pYCbCr2RgbKernel;
      cm_result_check(CreateKernel(ycbcrnode->GetIsa(), ycbcrnode->GetKernelName(), pYCbCr2RgbKernel));
      AddKernel(pTask, pYCbCr2RgbKernel);

      /* The input surface to the kernel is the output from JPEG decoder */
      ycbcrnode->PreRun(pYCbCr2RgbKernel, pSI_SrcYCbCr,  pSI_DstRgb, width, height);
   }

}

void Scan2FilePipeline::AssemblerDecompressGraph(const int tileHeight)
{
   VASurfaceAttrib fourcc;
   int surface_type;
   uint imgWidth, imgHeight, mcuRestartInterval;

   m_jpegdecoder[0] = new JpegDecoder(m_pCmDev, m_vaDpy);

   if (m_pJpegSrc.size() == 0)
   {
      /* For encode + decode path */
      U8 *pJpegInput;
      uint jpegSize;

      if (m_jpegcombine)
         m_jpegcombine->GetJpegOutput(pJpegInput, jpegSize);
      else if (m_jpegencoder[0])
         m_jpegencoder[0]->GetCodedBufferAddress(m_VAEncodedSurfaceID[0],
            pJpegInput, jpegSize);

      m_jpegdecoder[0]->ParseHeader(pJpegInput, jpegSize);
   }
   else
   {
      m_jpegdecoder[0]->ParseHeader((uchar *)&m_pJpegSrc[0], m_pJpegSrc.size());
   }

   imgWidth = m_jpegdecoder[0]->GetPicWidth();
   imgHeight = m_jpegdecoder[0]->GetPicHeight();
   mcuRestartInterval = m_jpegdecoder[0]->GetRestartInterval();

   cout << "JPEG decode image WIDTH=" <<imgWidth<< " HEIGHT="<<imgHeight<<endl;

   if (!(imgWidth && imgHeight))
      throw std::out_of_range("Unsupported JPEG format!");

   if (tileHeight == 0)
      m_PicTileHeight = imgHeight;
   else
      m_PicTileHeight = tileHeight;

   m_PicTileWidth = imgWidth;

   if ((m_PicTileHeight != imgHeight) && (imgWidth > 16 * 1024))
   {
      m_PicTileWidth = imgWidth / 2;
   }

   if (m_PicTileHeight != imgHeight)
   {

      assert(mcuRestartInterval != 0);

      /* 
       * If a tileheight is specific, the decode process will break JPEG input
       * images into multiple tiles, decode each tile, and combine back to
       * single raw output
       */
      int rowBlock = (imgHeight + m_PicTileHeight - 1) / m_PicTileHeight;
      int colBlock = (imgWidth + m_PicTileWidth - 1) / m_PicTileWidth;
      m_jpegdecoder[0]->SetTileInfo(m_PicTileHeight, m_PicTileWidth, rowBlock, colBlock);


   }
   uint image_size;
   if (m_yuvFormat == 0)
   {
      /* Output is 32bit bpp image */
      /* If need 24bit bpp image, will need additional conversion */
      image_size = imgWidth * imgHeight * 4;
   }
   else
   {
      /* Output is grayscale 8bpp image */
      image_size = imgWidth * imgHeight;
   }

   /* 
    * m_pRawDst mainly use to combine multiple raw files after decode
    */
   /* Need at least 16bytes align for using GPU to CPU copy */
   /* 64 bytes align or 4K bytes align has the best performance for gpu copy */
   m_pRawDst = (uchar *)CM_ALIGNED_MALLOC(image_size, 0x1000);

   uint outpitch = 0;
   uint outsize = 0;
   GetSurface2DInfo(m_PicTileWidth, m_PicTileHeight, CM_SURFACE_FORMAT_A8R8G8B8,
            &outpitch, &outsize);

   DeCompressGraphSetup(m_PicTileWidth, m_PicTileHeight, m_yuvFormat, m_pDeCompressTask[0], 
         m_jpegdecoder[0], 0);


   if (imgHeight % m_PicTileHeight)
   {
      int remaining_row = imgHeight % m_PicTileHeight;
      m_jpegdecoder[1] = new JpegDecoder(m_jpegdecoder[0]);

      GetSurface2DInfo(m_PicTileWidth, remaining_row, CM_SURFACE_FORMAT_A8R8G8B8,
            &outpitch, &outsize);

      DeCompressGraphSetup(m_PicTileWidth, remaining_row, m_yuvFormat, m_pDeCompressTask[1], 
            m_jpegdecoder[1], 1);
   }

}

/* For RGB input */
void Scan2FilePipeline::copyToTile(U8 *pSrc, int tileWidth, int tileHeight, int imageWidth,
      U8 *pDst0, U8 *pDst1, U8 *pDst2)
{
   int dstIndex, srcIndex;

   uint dst_pitch_size = 0;
   uint dst_surface_size = 0;
   GetSurface2DInfo(tileWidth, tileHeight, CM_SURFACE_FORMAT_A8,
         &dst_pitch_size, &dst_surface_size);

   for (int y = 0; y < tileHeight; y++)
   {
      srcIndex = y * imageWidth * 3;
      dstIndex = y * dst_pitch_size;
      for (int x = 0; x < tileWidth; x++)
      {
         pDst0[dstIndex] = pSrc[srcIndex];
         pDst1[dstIndex] = pSrc[srcIndex+1];
         pDst2[dstIndex] = pSrc[srcIndex+2];
         dstIndex += 1;
         srcIndex += 3;
      }
   }
}

void Scan2FilePipeline::copyToTile(U8 *pSrc, int imageWidth, JpegEncoder *pJpegEncoder,
      VASurfaceID outSurfaceID)
{
   pJpegEncoder->CopyToVASurface(outSurfaceID, pSrc, imageWidth);
}

/* Jpeg Encode Graph */
int Scan2FilePipeline::ExecuteCompressGraph(int iteration)
{
   DWORD dwTimeOutMs = -1;
   UINT64 executionTime=0;

   int rowBlock = (m_PicHeight + m_PicTileHeight - 1) / m_PicTileHeight;
   int colBlock = (m_PicWidth + m_PicTileWidth - 1) / m_PicTileWidth;

   U8 *srcOffset;
   U8 *srcTileOffset0, *srcTileOffset1, *srcTileOffset2;
   int taskBlk, rowToProcess;
   int src_bytes_per_line, src_bytes_per_tilewidth_line;

   int dstIndex, srcIndex;

   if (m_yuvFormat == 0)
   {
      src_bytes_per_line = m_PicWidth * 3;
      src_bytes_per_tilewidth_line = m_PicTileWidth * 3;
   } else { src_bytes_per_line = m_PicWidth;
      src_bytes_per_tilewidth_line = m_PicTileWidth;
   }

   unsigned long start = GetCurrentTimeInMilliseconds();
   for (int loop = 0; loop < iteration; loop++)
   {
      for (int rowi = 0; rowi < rowBlock; rowi++)
      {
         srcOffset = m_pRawSrc + rowi * m_PicTileHeight * src_bytes_per_line;

         for (int coli = 0; coli < colBlock; coli++)
         {
            srcOffset += coli * src_bytes_per_tilewidth_line;

            if (((rowi + 1) * m_PicTileHeight) <= m_PicHeight)
            {
               rowToProcess = m_PicTileHeight;
               taskBlk = 0;
            }
            else
            {
               rowToProcess = m_PicHeight - rowi * m_PicTileHeight;
               taskBlk = 1;
            }

            if (m_yuvFormat == 0)
            {
               srcTileOffset0 = m_pSrc0[taskBlk];
               srcTileOffset1 = m_pSrc1[taskBlk];
               srcTileOffset2 = m_pSrc2[taskBlk];

               copyToTile(srcOffset, m_PicTileWidth, rowToProcess, m_PicWidth,
                      srcTileOffset0, srcTileOffset1, srcTileOffset2);
            }
            else
            {
               copyToTile(srcOffset, m_PicWidth, m_jpegencoder[taskBlk], m_VAEncodedSurfaceID[taskBlk]);
            }

            /* Run RGBtoYCbCr CM kernel */
            if (m_pCompressTask[taskBlk] != NULL)
            {
               if (e_Pipeline)
                  cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
               cm_result_check(m_pCmQueue->Enqueue(m_pCompressTask[taskBlk], e_Pipeline));
            }

            /* Run JPEG encoder */
            m_jpegencoder[taskBlk]->Run(m_VAEncodedSurfaceID[taskBlk]);

            m_jpegencoder[taskBlk]->GetCodedBufferAddress(m_VAEncodedSurfaceID[taskBlk], m_compressJpegData,
                  m_compressJpegDataSize);

            if (m_PicHeight != m_PicTileHeight)
            {
               m_jpegcombine->AddJpeg(m_compressJpegData, m_compressJpegDataSize,
                     rowi, coli, rowToProcess);
            }

            m_jpegencoder[taskBlk]->ReleaseCodedBuffer();

         }
      }
   }

   unsigned long end = GetCurrentTimeInMilliseconds();

   if (iteration > 1)
   {
      unsigned long total = end - start;
      double average_time = total / (double) iteration;
      int PPM = 60000.0 / average_time;
      std::cout << "PPM = " << PPM << " Average time = " << average_time << "ms. "<< std::endl;
   }

   if (m_pCompressTask[0] != NULL)
   {
      cm_result_check(m_pCmDev->FlushPrintBuffer());

      cm_result_check(m_pCmQueue->DestroyEvent(e_Pipeline));

   }

   return CM_SUCCESS;
}

/* Execute JPEG decode Graph */
int Scan2FilePipeline::ExecuteDecompressGraph(int iteration)
{
   DWORD dwTimeOutMs = -1;
   UINT64 executionTime=0;

   uint imgWidth = m_jpegdecoder[0]->GetPicWidth();
   uint imgHeight = m_jpegdecoder[0]->GetPicHeight();

   int rowBlock = (imgHeight + m_PicTileHeight - 1) / m_PicTileHeight;
   int colBlock = (imgWidth + m_PicTileWidth - 1) / m_PicTileWidth;

   int taskBlk, rowToProcess;

   uint dummySize;
   uint tilePitch;
   GetSurface2DInfo(m_PicTileWidth, m_PicTileHeight, CM_SURFACE_FORMAT_A8R8G8B8,
         &tilePitch, &dummySize);

   unsigned long start = GetCurrentTimeInMilliseconds();
   for (int loop = 0; loop < iteration; loop++)
   {
      for (int rowi = 0; rowi < rowBlock; rowi++)
      {
         for (int coli = 0; coli < colBlock; coli++)
         {
            unsigned long starte = GetCurrentTimeInMilliseconds();

            if (((rowi + 1) * m_PicTileHeight) <= imgHeight)
            {
               rowToProcess = m_PicTileHeight;
               taskBlk = 0;
            }
            else
            {
               /* Retrieve the pointer to last decode JPEG buffer */
               const U8 *lastStream = m_jpegdecoder[0]->GetLastStream();
               m_jpegdecoder[1]->SetStartStream(lastStream);
               rowToProcess = imgHeight - rowi * m_PicTileHeight;
               taskBlk = 1;
            }

            /* Run JPEG decoder */

            m_jpegdecoder[taskBlk]->Run(m_VADecodedSurfaceID[taskBlk], rowi, coli);

            if (m_pDeCompressTask[taskBlk] != NULL)
            {
               /* Run YCbCrtoRGB CM kernel */
               if (e_Pipeline)
                  cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
               cm_result_check(m_pCmQueue->Enqueue(m_pDeCompressTask[taskBlk], e_Pipeline));
               //cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
            }

            /* Copy individual tile to destination memory location */
            if ((rowBlock > 1) || (colBlock > 1))
            {
               if (m_pDeCompressTask[taskBlk] != NULL)
               {
                  /* 
                   * Use GPU to transfer tile to CPU memory location
                   * CPU memcpy is slow, use the CM kernel GPUtoCPU transfer
                   */
                  U8 *pImgOutPtr;
                  uint imgOutOffset = rowi * m_PicTileHeight * imgWidth*4 + coli * m_PicTileWidth*4;
                  U8 *pSrcTilePtr;

                  pImgOutPtr = m_pRawDst + imgOutOffset;

                  cm_result_check(m_pCmQueue->EnqueueCopyGPUToCPUFullStride(m_pDstRgb[taskBlk],
                           pImgOutPtr,imgWidth*4,rowToProcess,CM_FASTCOPY_OPTION_NONBLOCKING,
                           e_Pipeline));

                  /* CPU memcpy
                  for (int y=0; y < rowToProcess; y++)
                  {
                     pSrcTilePtr = m_pTileDst[taskBlk] + y*tilePitch;

                     for (int x = 0; x < m_PicTileWidth; x++)
                     {
                        pImgOutPtr[0] = pSrcTilePtr[2];
                        pImgOutPtr[1] = pSrcTilePtr[1];
                        pImgOutPtr[2] = pSrcTilePtr[0];
                        pSrcTilePtr += 4;
                        pImgOutPtr  += 4;
                     }
                     pImgOutPtr += m_PicTileWidth*4;
                  }
                  */
               }
               else
               {
                  unsigned long start2 = GetCurrentTimeInMilliseconds();
                  m_jpegdecoder[taskBlk]->AppendTileToOutput(m_pRawDst, rowi, coli,
                        m_VADecodedSurfaceID[taskBlk]);
               }
            }
            else if (m_pDeCompressTask[taskBlk] != NULL)
            {
               if (e_Pipeline)
                  cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));
               cm_result_check(m_pCmQueue->EnqueueCopyGPUToCPUFullStride(m_pDstRgb[taskBlk],
                          m_pRawDst,imgWidth*4,rowToProcess,CM_FASTCOPY_OPTION_NONBLOCKING,
                           e_Pipeline));

            }
         }
      }
   }
   unsigned long end = GetCurrentTimeInMilliseconds();

   if (iteration > 1)
   {
      unsigned long total = end - start;
      double average_time = total / (double) iteration;
      int PPM = 60000.0 / average_time;
      std::cout << "PPM = " << PPM << std::endl;
   }

   if (m_pDeCompressTask[taskBlk] != NULL)
   {
      cm_result_check(m_pCmDev->FlushPrintBuffer());

      cm_result_check(e_Pipeline->WaitForTaskFinished(dwTimeOutMs));

      cm_result_check(m_pCmQueue->DestroyEvent(e_Pipeline));
   }

   return CM_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
