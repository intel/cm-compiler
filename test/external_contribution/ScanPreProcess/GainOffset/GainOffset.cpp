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
#include "cm_rt.h"
#include "GainOffset.h"

#define BLOCK_WIDTH     16
#define BLOCK_HEIGHT    8

/*********************************************************************************************************************************/

float randFloat(float min, float max)
{
   return min + (max - min) * (((float)rand())/(float) RAND_MAX);
}

/*********************************************************************************************************************************/

static void Convert8bppto10or12(
   uchar* pSrc,
   uchar* pDst,
   float *agcf,
   float *gain,
   float *offset,
   int image8width,
   int image8height,
   int image10or12width,
   int bits
   )
{
   float acgf = randFloat(0.9f, 1.1f);
   *agcf = acgf;

   for (int i = 0; i < image8width; i++)
   {
      gain[i] = randFloat(0.9f, 3.0f);

      offset[i] = randFloat(-64.0, 0.0f);;
   }

   unsigned char *pUnpackedLine, *pPackedLine, byte0, byte1, byte2, byte3, byte4, byte5;
   float p0, p2, p3, p1;
   unsigned short topack0, topack1, topack2, topack3;

   for (int y = 0; y < image8height; y++)
   {
      pUnpackedLine = pSrc + y*image8width;
      pPackedLine = pDst + y*image10or12width;

      for( int x = 0; x < image8width/4; x++)
      {

         p0 = (((float)pUnpackedLine[x*4 + 0] * (1.0f/acgf) - offset[x*4 + 0]) * (1.0f/gain[x*4 + 0]));
         p1 = (((float)pUnpackedLine[x*4 + 1] * (1.0f/acgf) - offset[x*4 + 1]) * (1.0f/gain[x*4 + 1]));
         p2 = (((float)pUnpackedLine[x*4 + 2] * (1.0f/acgf) - offset[x*4 + 2]) * (1.0f/gain[x*4 + 2]));
         p3 = (((float)pUnpackedLine[x*4 + 3] * (1.0f/acgf) - offset[x*4 + 3]) * (1.0f/gain[x*4 + 3]));

         if (bits == 10)
         {
            p0 = (p0 > 1023.0f) ? 1023.0f : p0;
            p1 = (p1 > 1023.0f) ? 1023.0f : p1;
            p2 = (p2 > 1023.0f) ? 1023.0f : p2;
            p3 = (p3 > 1023.0f) ? 1023.0f : p3;
         }
         else if(bits == 12)
         {
            p0 = (p0 > 4095.0f) ? 4095.0f : p0;
            p1 = (p1 > 4095.0f) ? 4095.0f : p1;
            p2 = (p2 > 4095.0f) ? 4095.0f : p2;
            p3 = (p3 > 4095.0f) ? 4095.0f : p3;
         }

         p0 = (p0 < 0.0f) ? 0.0f : p0;
         p1 = (p1 < 0.0f) ? 0.0f : p1;
         p2 = (p2 < 0.0f) ? 0.0f : p2;
         p3 = (p3 < 0.0f) ? 0.0f : p3;

         topack0 = (unsigned short)p0;
         topack1 = (unsigned short)p1;
         topack2 = (unsigned short)p2;
         topack3 = (unsigned short)p3;

         if (bits == 10)
         {
            byte0 = topack0 & 0xff;
            byte1 = topack1 & 0xff;
            byte2 = topack2 & 0xff;
            byte3 = topack3 & 0xff;
            byte4 = (topack0 & 0x300) >> 2;
            byte4 = byte4 | (topack1 & 0x300) >> 4;
            byte4 = byte4 | (topack2 & 0x300) >> 6;
            byte4 = byte4 | (topack3 & 0x300) >> 8;

            *pPackedLine++ = byte0;
            *pPackedLine++ = byte1;
            *pPackedLine++ = byte2;
            *pPackedLine++ = byte3;
            *pPackedLine++ = byte4;
         }
         else if(bits == 12)
         {
            byte0 = topack0 & 0x0ff;
            byte1 = (topack0 & 0xf00) >> 8;
            byte1 = byte1 | ((topack1 & 0x00f) << 4);
            byte2 = (topack1 & 0xff0) >> 4;
            byte3 = topack2 & 0x0ff;
            byte4 = (topack2 & 0xf00) >> 8;
            byte4 = byte4 | ((topack3 & 0x00f) << 4);
            byte5 = (topack3 & 0xff0) >> 4;

            *pPackedLine++ = byte0;
            *pPackedLine++ = byte1;
            *pPackedLine++ = byte2;
            *pPackedLine++ = byte3;
            *pPackedLine++ = byte4;
            *pPackedLine++ = byte5;
         }
      }
   }
}

/*********************************************************************************************************************************/

GainOffset::GainOffset(CmDevice *pdevice)
{
   m_pCmDev    = pdevice;
   m_pKernel   = NULL;

   m_pGainR    = NULL;
   m_pGainG    = NULL;
   m_pGainB    = NULL;

   m_pOffsetR  = NULL;
   m_pOffsetG  = NULL;
   m_pOffsetB  = NULL;

   srand(time(NULL));
}

GainOffset::~GainOffset(void)
{
   free (m_pGainR);
   free (m_pGainG);
   free (m_pGainB);
   free (m_pOffsetR);
   free (m_pOffsetG);
   free (m_pOffsetB);
}

/*********************************************************************************************************************************/

int GainOffset::Convert(
   uchar* pR,
   uchar* pG,
   uchar* pB,
   uchar* pDstR,
   uchar* pDstG,
   uchar* pDstB,
   int inputWidth,
   int inputHeight,
   unsigned int outputWidth,
   int bits
   )
{
   m_pGainR    = (float *)CM_ALIGNED_MALLOC(inputWidth*sizeof(float), 0x1000);
   m_pOffsetR = (float *)CM_ALIGNED_MALLOC(inputWidth*sizeof(float), 0x1000);

   m_pGainG    = (float *)CM_ALIGNED_MALLOC(inputWidth*sizeof(float), 0x1000);
   m_pOffsetG = (float *)CM_ALIGNED_MALLOC(inputWidth*sizeof(float), 0x1000);

   m_pGainB    = (float *)CM_ALIGNED_MALLOC(inputWidth*sizeof(float), 0x1000);
   m_pOffsetB = (float *)CM_ALIGNED_MALLOC(inputWidth*sizeof(float), 0x1000);

   memset(m_pGainR, 0.0f, sizeof(float) * inputWidth);
   memset(m_pOffsetR, 0.0f, sizeof(float) * inputWidth);
   memset(m_pGainG, 0.0f, sizeof(float) * inputWidth);
   memset(m_pOffsetG, 0.0f, sizeof(float) * inputWidth);
   memset(m_pGainB, 0.0f, sizeof(float) * inputWidth);
   memset(m_pOffsetB, 0.0f, sizeof(float) * inputWidth);

   Convert8bppto10or12(pB, pDstB, &m_agcB, m_pGainB, m_pOffsetB, inputWidth,
         inputHeight, outputWidth, bits);
   Convert8bppto10or12(pG, pDstG, &m_agcG, m_pGainG, m_pOffsetG, inputWidth,
         inputHeight, outputWidth, bits);
   Convert8bppto10or12(pR, pDstR, &m_agcR, m_pGainR, m_pOffsetR, inputWidth,
         inputHeight, outputWidth, bits);

}

/*********************************************************************************************************************************/

int GainOffset::PreRun(
   CmKernel *pKernel,
   SurfaceIndex *pSI_SrcRSurf,
   SurfaceIndex *pSI_SrcGSurf,
   SurfaceIndex *pSI_SrcBSurf,
   SurfaceIndex *pSI_DstSurf,
   int nPicWidth,
   int nPicHeight
   )
{
   CmThreadSpace *pTS = NULL;
   m_pKernel = pKernel;
   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;
   int result;

   nPicWidthInBlk = (nPicWidth + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
   nPicHeightInBlk = (nPicHeight + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT;

   CM_Error_Handle(m_pCmDev->CreateThreadSpace(nPicWidthInBlk, nPicHeightInBlk, pTS), "Create pTS Error");  

   CmBufferUP     *m_pGainRSurf;
   CmBufferUP     *m_pGainGSurf;
   CmBufferUP     *m_pGainBSurf;
   CmBufferUP     *m_pOffsetRSurf;
   CmBufferUP     *m_pOffsetGSurf;
   CmBufferUP     *m_pOffsetBSurf;
   SurfaceIndex   *m_pSI_GainRSurf;
   SurfaceIndex   *m_pSI_GainGSurf;
   SurfaceIndex   *m_pSI_GainBSurf;
   SurfaceIndex   *m_pSI_OffsetRSurf;
   SurfaceIndex   *m_pSI_OffsetGSurf;
   SurfaceIndex   *m_pSI_OffsetBSurf;

   result = m_pCmDev->CreateBufferUP(nPicWidth*sizeof(float), m_pGainR, m_pGainRSurf);
   m_pGainRSurf->GetIndex(m_pSI_GainRSurf);
   result = m_pCmDev->CreateBufferUP(nPicWidth*sizeof(float), m_pGainG, m_pGainGSurf);
   m_pGainGSurf->GetIndex(m_pSI_GainGSurf);
   result = m_pCmDev->CreateBufferUP(nPicWidth*sizeof(float), m_pGainB, m_pGainBSurf);
   m_pGainBSurf->GetIndex(m_pSI_GainBSurf);
   result = m_pCmDev->CreateBufferUP(nPicWidth*sizeof(float), m_pOffsetR, m_pOffsetRSurf);
   m_pOffsetRSurf->GetIndex(m_pSI_OffsetRSurf);
   result = m_pCmDev->CreateBufferUP(nPicWidth*sizeof(float), m_pOffsetG, m_pOffsetGSurf);
   m_pOffsetGSurf->GetIndex(m_pSI_OffsetGSurf);
   result = m_pCmDev->CreateBufferUP(nPicWidth*sizeof(float), m_pOffsetB, m_pOffsetBSurf);
   m_pOffsetBSurf->GetIndex(m_pSI_OffsetBSurf);
   CM_Error_Handle(result, "Create Buffer Error");

   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcRSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcGSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcBSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(float), &m_agcR);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(float), &m_agcG);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(float), &m_agcB);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_GainRSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_GainGSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_GainBSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_OffsetRSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_OffsetGSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), m_pSI_OffsetBSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &nPicWidth);
   result = m_pKernel->AssociateThreadSpace(pTS);
   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;
}
