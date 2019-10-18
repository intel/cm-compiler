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
////////////////////////////////////////////////////////////////////////////////////
#include<vector>
using namespace std;

////////////////////////////////////////////////////////////////////////////
#include "Scan2FilePipeline_Defs.h"
#include "cm_rt.h"
#include "JpegEncoder.h"
#include "JpegDecoder.h"
#include "JpegCombine.h"
#include <va/va.h>


/////////////////////////////////////////////////////////////////////////////
#ifndef __SCAN2FILEPIPELINE_H__
#define __SCAN2FILEPIPELINE_H__

typedef unsigned char   U8;
typedef unsigned int    U32;
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;

#ifdef _DEBUG
#define DEBUG_printf(message, result)	printf("%s: %d\n", message, result)
#else
#define DEBUG_printf(message, result)
#endif


#define CM_Error_Handle(result, msg)	\
		if (result != CM_SUCCESS) {		\
			DEBUG_printf(msg, result);			\
			exit(1);				\
		}


//////////////////////////////////////////////////////////////////////////////////////////////
class Scan2FilePipeline
{
public:
    ////////////////////////////////////////////////////////////////////////////////
   Scan2FilePipeline(void);
	~Scan2FilePipeline(void);

   //////////////////////////////////////////////////////////////////////////////
   int Init();
   int ExecuteCompressGraph(int iterations);
   int ExecuteDecompressGraph(int iterations);
   int GetRawInputImage(const char *filename, const int width, const int height,
         const int yuvformat);
   int GetJpegInputImage(const char *filename, const int yuvformat);
   void Save2JPEG(const char *filename);
   void Save2Raw(const char *outfile);
   void AssemblerCompressGraph(int jpegQuality, const int tileHeight);
   void AssemblerDecompressGraph(const int tileHeight);

protected:
   void AddKernel(CmTask *pTask, CmKernel *pKernel);
   int GetSurface2DInfo(int width, int height, CM_SURFACE_FORMAT format,
         unsigned int * pitch, unsigned int * surface_size);
   int CreateKernel(char *isaFile, char *kernelName, CmKernel *& pKernel);
   int CompressGraphSetup(int width, int height, int yuvformat, int jpegquality, CmTask *&pTask,
         JpegEncoder *pJpegEncoder, int blkindx);
   int DeCompressGraphSetup(int width, int height, int yuvformat, CmTask *&pTask,
         JpegDecoder *pJpegDecoder, int blkindx);
   void copyToTile(unsigned char *pSrc, int tileWidth, int tileHeight, int imageWidth,  
         unsigned char *pDst0, unsigned char *pDst1, unsigned char *pDst2);
   void copyToTile(unsigned char *pSrc, int imageWidth, JpegEncoder *pJpegEncoder,
         VASurfaceID outSurfaceID);


private:
   CmDevice*    m_pCmDev;
   CmQueue      *m_pCmQueue;
   CmEvent      *e_Pipeline;
   CmTask       *m_pCompressTask[2];
   CmTask       *m_pDeCompressTask[2];
   VADisplay    m_vaDpy;
   JpegEncoder  *m_jpegencoder[2];
   JpegDecoder  *m_jpegdecoder[2];
   JpegCombine  *m_jpegcombine;
   int          m_fd;
   int          m_yuvFormat;

   uint         m_PicWidth;
   uint         m_PicHeight;
   uint         m_PicTileHeight;
   uint         m_PicTileWidth;
   uint         m_McuRestartInterval;

   uchar        *m_pSrc0[2];
   uchar        *m_pSrc1[2];
   uchar        *m_pSrc2[2];
   uchar        *m_pRawSrc;
   uchar        *m_pRawDst;
   vector<char> m_pJpegSrc;
   uchar        *m_compressJpegData;
   uint         m_compressJpegDataSize;
   VASurfaceID  m_VAEncodedSurfaceID[2];
   VASurfaceID  m_VADecodedSurfaceID[2];
   CmSurface2D  *m_pDstRgb[2];

};
#endif //__SCAN2FILEPIPELINE_H__
