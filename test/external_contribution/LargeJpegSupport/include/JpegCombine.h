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

#include <vector>

using namespace std;

#ifndef JPEGCOMBINE_H
#define JPEGCOMBINE_H

enum std_jpeg_markers {
   JPEG_DQT  = 0xDB, // Define Quantization Table
   JPEG_SOF  = 0xC0, // Start of Frame (size information)
   JPEG_DHT  = 0xC4, // Huffman Table
   JPEG_SOI  = 0xD8, // Start of Image
   JPEG_SOS  = 0xDA, // Start of Scan
   JPEG_EOI  = 0xD9, // End of Image 
   JPEG_DRI  = 0xDD, // Define Restart Interval 
   JPEG_APP0 = 0xE0,
};


class JpegCombine
{
public:
   JpegCombine(const unsigned int colblk, const unsigned int rowblk,
         const unsigned int mcuinterval, const unsigned int width, const unsigned int height);
   ~JpegCombine(void);
   int AddJpeg(unsigned char *imgBuf, unsigned int imgSize, const int rowIndex,
         const int colIndex, const int currentTileHeight);
   void FreeJpegBuffer(void);
   int GetJpegOutput(unsigned char *& coded_mem, unsigned int &size);
   int WriteOut(const char *filename);

private:
   int parse_JFIF(const int rowIndex, const int colIndex, const bool lastrow, const bool lastcol);
   int addStreamToCombineBuffer(vector<unsigned char> rowBuffer);
   int combineVertical(const int rowIndex, const int colIndex,
         const int curTileHeight);
   int combineVerticalHorizontal(const int rowIndex, const int colIndex,
         const int curTileHeight);

   unsigned int width;
   unsigned int height;
   unsigned int colparts;
   unsigned int rowparts;
   unsigned int mcuinterval;
   int last_restart_marker;

   unsigned char* stream;
   unsigned char *stream_begin, *stream_end, *stream_scan;
   unsigned int stream_length;

   vector<unsigned char> combineBuffer;
   vector<unsigned char> tempStreamBuffer;

};

#endif
