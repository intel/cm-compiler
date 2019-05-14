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

#include "ScanPreProcess.h"

/*********************************************************************************************************************************/

class GainOffset : public ScanPreProcess
{
   public:
      GainOffset(CmDevice *device);
      ~GainOffset(void);

      int Convert(
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
      );

      int PreRun(
            CmKernel *pKernel,
            SurfaceIndex *pSI_SrcRSurf,
            SurfaceIndex *pSI_SrcGSurf,
            SurfaceIndex *pSI_SrcBSurf,
            SurfaceIndex *pSI_DstSurf,
            int nPicWidth,
            int nPicHeight
      );

      char* GetISA() { return "GainOffset/GainOffset_genx.isa"; }
      char* GetKernelName0() { return "GainOffset10_GENX"; }
      char* GetKernelName1() { return "GainOffset12_GENX"; }

   private:
      int   m_nPicWidth;
      int   m_nPicHeight;
      CmDevice *m_pCmDev;
      CmKernel *m_pKernel;
      float m_agcR;
      float m_agcG;
      float m_agcB;
      float *m_pGainR;
      float *m_pGainG;
      float *m_pGainB;
      float *m_pOffsetR;
      float *m_pOffsetG;
      float *m_pOffsetB;
};

