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

/*********************************************************************************************************************************/

#include<vector>
using namespace std;

/*********************************************************************************************************************************/

#include "ScanPreProcess_Defs.h"
#include "cm_rt.h"

/*********************************************************************************************************************************/

#ifndef __SCANPP_H__
#define __SCANPP_H__

typedef unsigned char   U8;
typedef unsigned int    U32;
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;

#ifdef _DEBUG
#define DEBUG_printf(message, result)   printf("%s: %d\n", message, result)
#else
#define DEBUG_printf(message, result)
#endif


#define CM_Error_Handle(result, msg)    \
   if (result != CM_SUCCESS)        \
   {             \
      DEBUG_printf(msg, result);                      \
      exit(1);                                \
   }

/*********************************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ScanPreProcess
{
   public:
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ScanPreProcess(void);
      ~ScanPreProcess(void);

      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      int Init();
      int ExecuteGraph(CmTask *pTask, int iterations);
      int GetInputImage(const char *filename);
      int SaveOutputImage(const char *filename, int skewangle);
      int AssemblerGraph(CmTask *&SppTask, int skew, int bits);

   protected:
      int AddKernel(CmTask *pTask, CmKernel *pKernel);
      int GetSurface2DInfo(
            int width,
            int height,
            CM_SURFACE_FORMAT format,
            unsigned int * pitch,
            unsigned int * surface_size
      );
      int CreateKernel(char *isaFile, char *kernelName, CmKernel *& pKernel);

   private:
      CmDevice   *m_pCmDev;
      CmQueue    *m_pCmQueue;
      CmEvent    *e_Pipeline;

      uint     m_PicWidth;
      uint     m_PicHeight;
      uint     m_SkewedWidth;
      uint     m_SkewedHeight;

      uchar    *m_pSrc;
      uchar    *m_pSkewedR;
      uchar    *m_pSkewedG;
      uchar    *m_pSkewedB;
      uchar    *m_pRawGainOffsetR;
      uchar    *m_pRawGainOffsetG;
      uchar    *m_pRawGainOffsetB;
      uchar    *m_pCorrectedGainOffsetDst;
      uchar    *m_pDstR;
      uchar    *m_pDstG;
      uchar    *m_pDstB;
};
#endif //__SCANPP_H__
