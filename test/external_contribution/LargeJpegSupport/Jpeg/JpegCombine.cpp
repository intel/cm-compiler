/*
 *
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

#include <assert.h>
#include <iostream>
#include "JpegCombine.h"

#define be16_to_cpu(x) (((x)[0] << 8) | (x)[1])

int update_SOF(unsigned char *istream,
      unsigned int width,
      unsigned int height)
{
   /* 
    * SOF remain the same except width and height
    * The older SOF width/height is based on tilewidth and tileheight
    */
   *(istream+3) = (height >> 8) & 0xff;
   *(istream+4) = height & 0xff;
   *(istream+5) = (width >> 8) & 0xff;
   *(istream+6) = width & 0xff;

   istream += 8;

   return 0;
}

int remove_EOI(
      unsigned char *& istream_end,
      unsigned int & stream_length
      )
{
   int ret = 0;
   /* Search the EOI from the stream_end -> stream */
   istream_end -= 1;
   if ((*(istream_end) == 0xd9)  && *(istream_end-1) == 0xff)
   {
      istream_end -= 2;
      stream_length -= 2;
   }
   else
   {
      /* Without EOI is invalid code */
      ret = -1;
   }

   return ret;
}

int find_SOI(unsigned char *& istream, unsigned char *istream_end)
{
   while (!(*istream == 0xff && *(istream+1) == 0xd8))  // searching for the start of image marker
   {
      if (istream <= istream_end)
      {
         istream++;
         continue;
      }
      else
         return 0;   // No more images in the file.
   }
   istream = istream + 2;
   return 1;
}

int get_last_restart_marker(const unsigned char *istream, unsigned char
      *istream_end)
{
   int last_restart_marker = 0;

   while (istream_end >= istream)
   {
      if ((*(istream_end -1)  == 0xff) && ((*(istream_end) & ~0xd7) == 0) && (*(istream_end ) != 0))
      {
         last_restart_marker = *(istream_end) & 0xf;
         break;
      }

      istream_end--;
   }

   return last_restart_marker;
}

inline int getMcuBlkCount(const int tileWidth, const int tileHeight, const int mcuRestartInterval)
{
   /* Assume surface format is YUV444 and MCU is 8x8 */
   int mcuBlkCount = 0;

   if (tileWidth % (mcuRestartInterval * 8) == 0)
      mcuBlkCount = tileWidth/(mcuRestartInterval * 8) * (tileHeight / 8);
   else
      mcuBlkCount = -1;

   return mcuBlkCount;
}


JpegCombine::JpegCombine(
      const unsigned int   totalCols,
      const unsigned int   totalRows,
      const unsigned int   mcuInterval,
      const unsigned int   imageWidth,
      const unsigned int   imageHeight
      )
{

   this->width = imageWidth;
   this->height = imageHeight;
   this->colparts = totalCols;
   this->rowparts = totalRows;
   this->mcuinterval = mcuInterval;

   /* Unknown size to reserve, need to calculate later */
   /* Just take a large enough size */
   this->combineBuffer.reserve(imageWidth);

   int mcuBlkInRow = getMcuBlkCount(this->width/this->colparts, 8, this->mcuinterval);
   int mcuBlkTotal = getMcuBlkCount(this->width/this->colparts, this->height/this->rowparts, 
                                 this->mcuinterval);

   if ((mcuBlkInRow <= 0) || (mcuBlkTotal <= 0))
      throw std::out_of_range("mcuinterval is not valid, can't combine JPEG using this interval");
}

JpegCombine::~JpegCombine(void)
{
   // Swap with empty vector to de-allocate the memory
   std::vector<unsigned char>().swap(this->combineBuffer);

}

void JpegCombine::FreeJpegBuffer(void)
{
   std::vector<unsigned char>().swap(this->combineBuffer);
}

int JpegCombine::parse_JFIF(
      const int rowIndex,
      const int colIndex,
      const bool rowlast,
      const bool collast)
{
   int chuck_len;
   int marker;
   int sos_marker_found = 0;
   unsigned char *next_chunck;
   int ret = 0;

   unsigned char *istream = this->stream_begin;
   unsigned char *istream_end = this->stream_end;

   find_SOI(istream, istream_end);

   while (!sos_marker_found && istream<= istream_end)
   {
      while((*istream == 0xff))
         istream++;
      marker = *istream++;
      chuck_len = be16_to_cpu(istream);
      next_chunck = istream + chuck_len;
      switch (marker)
      {
      case JPEG_SOF:
         if ((colIndex == 0) && (rowIndex == 0))
         {
            if (update_SOF(istream, this->width, this->height) < 0)
               return -1;
         }
         break;
      case JPEG_DQT:
         break;
      case JPEG_SOS:
         sos_marker_found = 1;
         break;
      case JPEG_DHT:
         break;
      case JPEG_DRI:
         /* For restart interval */
         break;
      case JPEG_APP0:
         break;
      default:
         cout << "Unknown marker "<<marker<<endl;
         break;
      }
      istream = next_chunck;
   }


   if (!rowlast || !collast)
      ret = remove_EOI(istream_end, this->stream_length);

   if ((colIndex == 0) && (rowIndex == 0))
   {
      this->stream_scan = this->stream_begin;
      this->stream_end = istream_end;
   }
   else
   {
      /* 
       * HW will never generate restart marker for first MCU blk
       * Insert a dummy value first, and eventually will update with a correct
       * sequence
       */
      *(istream - 1) = (this->last_restart_marker+1)%8 + 0xD0;
      *(istream - 2) = 0xff;
      this->stream_scan = istream - 2;
      this->stream_end = istream_end;
      this->stream_length += 2;
   }

   return ret;

}

int JpegCombine::addStreamToCombineBuffer(vector<unsigned char> rowBuffer)
{
   combineBuffer.insert(combineBuffer.end(), rowBuffer.begin(), rowBuffer.end());

   return 0;
}

int JpegCombine::combineVertical(
      const int rowIndex,
      const int colIndex,
      const int  curTileHeight
      )
{
   int ret = 0;

   int mcuBlkInRow = getMcuBlkCount(this->width/this->colparts, 8, this->mcuinterval);
   int mcuBlkTotal = getMcuBlkCount(this->width/this->colparts, curTileHeight, this->mcuinterval);

   vector<unsigned char> tempStreamBuffer;

   bool jpegheader = (rowIndex == 0) && (colIndex == 0) ? 1 : 0;

   int curMcuBlk = 0;
   unsigned int restartMarker = this->last_restart_marker;
   unsigned char *istream = this->stream_scan;
   unsigned char *istream_end = this->stream_end;

   int streamdone = 0;
   int streamMcuBlk = 0;

   if (jpegheader)
   {
      this->last_restart_marker = get_last_restart_marker(this->stream_scan, this->stream_end);
      this->last_restart_marker = (this->last_restart_marker + 1)%8;
   }
   else
   {
      while (curMcuBlk < mcuBlkTotal)
      {
         while ((istream < istream_end) && (!streamdone))
         {
            if ((*(istream) == 0xff) && ((*(istream+1) & ~0x7) == 0xd0) &&
                  (*(istream+1) != 0))
            {
               streamMcuBlk++;
               *(istream+1) = restartMarker + 0xD0;

               if (streamMcuBlk <= mcuBlkInRow)
                  restartMarker = (restartMarker + 1)%8;
            }

            if (streamMcuBlk > mcuBlkInRow)
               streamdone = 1;
            else
               istream++;
         }

         streamMcuBlk = 0;
         streamdone = 0;

         curMcuBlk += mcuBlkInRow;
      }
      this->last_restart_marker = restartMarker;
   }

   tempStreamBuffer.insert(tempStreamBuffer.begin(), this->stream_scan, this->stream_end+1);

   ret = addStreamToCombineBuffer(tempStreamBuffer);

   return ret;

}

int JpegCombine::combineVerticalHorizontal(
      const int  rowIndex,
      const int  colIndex,
      const int  curTileHeight
      )
{
   int ret = 0;

   int mcuBlkInRow = getMcuBlkCount(this->width/this->colparts, 8, this->mcuinterval);
   int mcuBlkTotal = getMcuBlkCount(this->width/this->colparts, curTileHeight, this->mcuinterval);

   if (colIndex != (this->colparts - 1))
   {
      /* add current stream buffer into a temporary buffer */
      tempStreamBuffer.insert(tempStreamBuffer.begin(), this->stream_scan, this->stream_end + 1);
   }
   else if (colIndex == (this->colparts - 1))
   {
      unsigned char *stream1 = reinterpret_cast<unsigned char *>(tempStreamBuffer.data());
      unsigned char *stream1_end = stream1 + tempStreamBuffer.size()-1;

      unsigned char *stream2 = this->stream_scan;
      unsigned char *stream2_end = this->stream_end;

      // Find out total of MCU blocks in a tile
      // Process each mcu block in a row for two buffers
      //
      int curMcuBlk = 0;
      int stream1McuBlk = 0;
      int stream2McuBlk = 0;
      unsigned int restartMarkerS1;
      unsigned int restartMarkerS2;

      if (rowIndex == 0)
      {
         stream1McuBlk = 1;
         restartMarkerS1 = 0;
         restartMarkerS2 = (restartMarkerS1 - 1 + mcuBlkInRow)%8;
      }
      else
      {
         restartMarkerS1 = this->last_restart_marker;
         restartMarkerS2 = (restartMarkerS1 + mcuBlkInRow)%8;
      }


      while (curMcuBlk < mcuBlkTotal)
      {
         unsigned char *stream1start = stream1;
         unsigned char *stream2start = stream2;
         vector<unsigned char> rowStreamBuffer;

         /* 
          * For JPEG header, first MCU block doesn't have restart marker
          */

         int stream1done = 0;
         int stream2done = 0;
         while ((stream1 < stream1_end) && (!stream1done))
         {
            //Restart marker need to updated
            if ((*(stream1) == 0xff) && ((*(stream1+1) & ~0x7) == 0xd0) &&
                  (*(stream1+1) != 0))
            {
               stream1McuBlk++;
               *(stream1+1) = restartMarkerS1 + 0xD0;
               restartMarkerS1 = (restartMarkerS1 + 1)%8;
            }

            if (stream1McuBlk > mcuBlkInRow)
               stream1done = 1;
            else
               stream1++;

         }

         while ((stream2 < stream2_end) && (!stream2done))
         {

            if ((*(stream2) == 0xff) && ((*(stream2+1) & ~0x7) == 0xd0) &&
                  (*(stream2+1) != 0))
            {
               stream2McuBlk++;
               *(stream2+1) = restartMarkerS2 + 0xD0;
               if (stream2McuBlk <= mcuBlkInRow)
               restartMarkerS2 = (restartMarkerS2 + 1)%8;
            }

            if (stream2McuBlk > mcuBlkInRow)
               stream2done = 1;

            else
               stream2++;

         }

         stream1McuBlk = 0;
         stream2McuBlk = 0;

         restartMarkerS1 = restartMarkerS2;
         restartMarkerS2 = (restartMarkerS1 + mcuBlkInRow)%8;

         // Ensure last byte not truncted for last row from rowblk
         if ((curMcuBlk == mcuBlkTotal - mcuBlkInRow))
         {
            stream1++;
            stream2++;
         }

         rowStreamBuffer.insert(rowStreamBuffer.begin(), stream1start, stream1);
         rowStreamBuffer.insert(rowStreamBuffer.end(), stream2start, stream2);

         addStreamToCombineBuffer(rowStreamBuffer);

         std::vector<unsigned char>().swap(rowStreamBuffer);

         curMcuBlk += mcuBlkInRow;
      }

      this->last_restart_marker = restartMarkerS1;

      /* 
       * Clear the tempStreamBuffer as not use anymore after process 2nd colblk
       */
      std::vector<unsigned char>().swap(this->tempStreamBuffer);

   } else {
      std::cout<<"Unsupported! Only support break image into two column block"<<std::endl;
      ret = -1;
   }

   return ret;
}

/* 
 * To support JPEG files combining using current method, all JPEG files expected
 * contain restart marker.  The restart marker can be controlled through MCU
 * restart interval, for example using retart interval 1, mean every MCU (8x8
 * pixels) will have single restart marker.  Lower restart interval will
 * increase the file size.
 *
 * Currently, it only supports two column blocks.  For example, for image width
 * of 5120, will break two blocks of 2560.  More than two blocks are not
 * supported.  
 *
 * For the first column block and first row block, it need to contains full JPEG
 * header, for subsequent tiles/blocks, all JPEG header will remove and only
 * contain the JPEG SOS. 
 *
 * For every first column block, it will add to tempStreamBuffer.  Combine only
 * done when receive the second column block.  All the restart marker in the
 * JPEG SOS will be reconstruct with order from D0 to D7.  Then, each MCU row
 * will insert into rowStreamBuffer, later into combineBuffer.
 * 
 * The combineBuffer stores the full JPEG image output   
 */

int JpegCombine::AddJpeg(
      unsigned char  *imgBuf,
      unsigned int   imgSize,
      const int      rowIndex,
      const int      colIndex,
      const int      curTileHeight
      )
{
   int ret;

   assert((imgBuf[0] == 0xFF) || (imgBuf[1] == JPEG_SOI));

   this->stream_begin = imgBuf;
   this->stream_length = imgSize;
   this->stream_end = this->stream_begin + this->stream_length;

   this->stream = this->stream_begin;

   bool jpegheader = (rowIndex == 0) && (colIndex == 0) ? 1 : 0;

   if (jpegheader)
      std::vector<unsigned char>().swap(this->combineBuffer);

   ret = parse_JFIF(rowIndex, colIndex, rowIndex == (this->rowparts - 1), colIndex == (this->colparts -1));

   if (this->width == this->width/this->colparts)
      ret = combineVertical(rowIndex, colIndex, curTileHeight);
   else
      ret = combineVerticalHorizontal(rowIndex, colIndex, curTileHeight);

   return ret;
}

int JpegCombine::WriteOut(const char* filename)
{
   FILE *jpeg_fp;
   jpeg_fp = fopen(filename, "wb");
   size_t w_items;

   unsigned char * jpegBuffer = reinterpret_cast<unsigned char*>(combineBuffer.data());
   int bufferSize = combineBuffer.size();

   if (jpeg_fp != NULL)
   {
      do {
         w_items = fwrite(jpegBuffer, bufferSize, 1, jpeg_fp);
      } while (w_items != 1);
      fclose(jpeg_fp);
   }

   return 0;
}

int JpegCombine::GetJpegOutput(unsigned char *& coded_mem, unsigned int &size)
{
   coded_mem = reinterpret_cast<unsigned char*>(combineBuffer.data());
   size = combineBuffer.size();

   return 0;
}

