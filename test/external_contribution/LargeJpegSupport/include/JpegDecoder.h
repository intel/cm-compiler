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
#include "tinyjpeg-internal.h"
#include <va/va.h>
#include <vector>

#ifndef JPEGDECODER_H
#define JPEGDECODER_H

class JpegDecoder
{
public:
   JpegDecoder(CmDevice *device, VADisplay va_dpy);
   JpegDecoder(JpegDecoder *&obj);
   ~JpegDecoder(void);

   int PreRun(VASurfaceID inputSurfID);
   unsigned int GetPicWidth();
   unsigned int GetPicHeight();
   unsigned int GetRestartInterval();
   int ParseHeader(const unsigned char *imgBuf, unsigned int imgSize);
   int CreateSurfaces(const int picture_width, const int picture_height,
         VASurfaceID *VASurfaceID);
   int Run(VASurfaceID inputSurfID, int rowBlkIdx, int colBlkIdx);
   int WriteOut(VASurfaceID inputSurfID, const char* filename);
   const unsigned char * GetLastStream();
   void SetStartStream(const unsigned char *startStream);
   void SetTileInfo(unsigned int tileHeight, unsigned int tileWidth,
         unsigned int totalRowBlock, unsigned int totalColumnBlock);
   int AppendTileToOutput(unsigned char *pDst, int rowIdx, int colIdx,
         VASurfaceID surfID);

private:
   int build_default_huffman_tables();
   int SplitTile(int rowBlkIdx, int colBlkIdx, int tileHeight, vector<unsigned char>& jpegBuffer);
   int SplitY(const unsigned char *stream_start, int curTileHeight,
         vector<unsigned char> &tileJpegDataOut);
   int SplitXY(const unsigned char *stream_start, int rowIdx, int colIdx, int curTileHeight,
         vector<unsigned char> &tileJpegDataOut);
   int CreateSliceParam(int tileHeight, vector<unsigned char> tileJpegBuffer);
   int parse_JFIF(const unsigned char *stream);
   int findSOI(const unsigned char *stream);
   int findEOI(const unsigned char *stream);
   int parse_DRI(const unsigned char *stream);
   int parse_DHT(const unsigned char *stream);
   int parse_SOS(const unsigned char *stream);
   int parse_SOF(const unsigned char *stream);
   int parse_DQT(const unsigned char *stream);
   int update_SOF(unsigned char *stream, int newWidth, int newHeight);

   CmDevice          *m_pCmDev;
   VADisplay         m_pVADpy;
   VASurfaceAttrib   m_fourcc;
   VAContextID       m_contextID;
   VAConfigID        m_configID;
   jdec_private      *m_jdecPriv;
   static int        m_scanNum;
   static int        m_nextImageFound;
   VABufferID        m_pic_param_buf;
   VABufferID        m_iqmatrix_buf;
   VABufferID        m_huffmantable_buf;
   VABufferID        m_slice_param_buf;
   VABufferID        m_slice_data_buf;
   bool              m_isTiled;
   unsigned int      m_tileWidth;
   unsigned int      m_tileHeight;
   unsigned int      m_curWidth;
   unsigned int      m_curHeight;   // m_tileHeight is overall image tile height,
                                    // if image height is not divisible by tile
                                    // height, use this m_curHeight to track the
                                    // remaining height
   unsigned int      m_totalColumnBlocks;
   unsigned int      m_totalRowBlocks;
   vector<unsigned char> tileJpegHeader;
   vector<unsigned char> tmpJpegDataOut;
   const unsigned char   *m_tileStreamPtr;

};

#endif
