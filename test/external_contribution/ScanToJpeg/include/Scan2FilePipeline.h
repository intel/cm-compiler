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
#include <va/va.h>


/////////////////////////////////////////////////////////////////////////////
#ifndef __SCAN2FILEPIPELINE_H__
#define __SCAN2FILEPIPELINE_H__

typedef unsigned char   U8;
typedef unsigned int    U32;
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;

enum YUVFormat
{
   YUV444 = 0,
   YUV420,
   YUV422,
   YUV400
};

#define CHECK_STATUS(status)                                  \
   if (status < 0) {                                   \
      fprintf(stderr,"%s:(%d) failed,exit\n", __func__, __LINE__); \
      exit(1);                                                    \
   }

#ifdef _DEBUG
#define DEBUG_printf(message, result)	printf("%s: %d\n", message, result)
#else
#define DEBUG_printf(message, result)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
class Scan2FilePipeline
{
public:
    ////////////////////////////////////////////////////////////////////////////////
   Scan2FilePipeline(void);
	~Scan2FilePipeline(void);

   int VAInit(void);

   //////////////////////////////////////////////////////////////////////////////
   int EncodeInit(const char *filename, const int width, const int height,
         const int yuv_format);
   int DecodeInit(const char *filename);
   int ExecuteCompressGraph( int iterations);
   int ExecuteDecompressGraph(int iterations);
   void Save2JPEG(const char *filename);
   void Save2Raw(const char *outfile);
   int AssemblerCompressGraph(int jpegQuality);
   void AssemblerDecompressGraph();
   int CMSurface2DInfo(int width, int height, CM_SURFACE_FORMAT format,
         unsigned int & pitch, unsigned int & surface_size);

   virtual int Execute(VASurfaceID inputSurfID) { };
   virtual void Destroy(VASurfaceID surfID) { };
   virtual int WriteOut(VASurfaceID surfureID, const char * filename) { };

protected:
   int UploadInputImage(const char* filename, unsigned char *pDst,
      const int width, const int height, const int pitch, bool grayscale);

   int UploadJpegImage(const char *filename, unsigned char *& pDst,
      uint &jpegDataSize);

   int WriteToFile(const char *filename, unsigned char *pSrc,
      const int width, const int height, const int pitch, bool grayscale);

private:
   VADisplay    m_vaDpy;
   int          m_fd;
   int          m_yuvFormat;

   int          m_PicWidth;
   int          m_PicHeight;
   const char  *m_filename;
   uchar       *m_compressJpegData;
   uint         m_compressJpegDataSize;
   VASurfaceID  m_VAEncodedSurfaceID;
   VASurfaceID  m_VADecodedSurfaceID;
//   vector<GPUGraph *> m_GpuTask1;
};

struct GPUGraph
{
   Scan2FilePipeline *node;
   VASurfaceID       nodeSurfParam;
};
#endif //__SCAN2FILEPIPELINE_H__
