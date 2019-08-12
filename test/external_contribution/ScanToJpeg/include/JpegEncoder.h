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
using namespace std;

#include "cm_rt.h"
#include <va/va.h>

#ifndef JPEGENCODER_H
#define JPEGENCODER_H

class JpegEncoder
{
public:
   JpegEncoder(CmDevice *device, VADisplay va_dpy, const unsigned int yuv_type);
   ~JpegEncoder(void);

   int PreRun(
         VASurfaceID inputSurfID,
         int nPicWidth,
         int nPicHeight,
         int quality,
         int frameSize
         );

   int CreateSurfaces(int picture_width, int picture_height, int picture_pitch,
         unsigned char *gray_surface, VASurfaceID *outSurfaceID);
   int GetCodedBufferAddress(VASurfaceID inputSurfID, unsigned char *& coded_mem,
         unsigned int &slice_length);
   int GetVASurfaceAttrib(VASurfaceAttrib fourcc[], unsigned int & surface_format);
   int Run(VASurfaceID inputSurfID);
   int WriteOut(VASurfaceID inputSurfID, const char* filename);

private:
   CmDevice          *m_pCmDev;
   VADisplay         m_pVADpy;
   VAConfigAttrib    m_attrib[2];
   VASurfaceAttrib   m_fourcc[2];
   VAContextID       m_contextID;
   VAConfigID        m_configID;
   VABufferID        m_pic_paramBufID;                /* Picture parameter id*/
   VABufferID        m_slice_paramBufID;              /* Slice parameter id, only 1 slice per frame in jpeg encode */
   VABufferID        m_codedbufBufID;                 /* Output buffer id, compressed data */
   VABufferID        m_packed_raw_header_paramBufID;  /* Header parameter buffer id */
   VABufferID        m_packed_raw_headerBufID;        /* Header buffer id */
   VABufferID        m_qmatrixBufID;                  /* Quantization Matrix id */
   VABufferID        m_huffmantableBufID;             /* Huffman table id*/

   unsigned int      m_vaSurfaceType;
};

#endif
