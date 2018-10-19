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

#include "CPipeline.h"

class CmykRF : public CopyPipeline
{
public:
   CmykRF(CmDevice *device, int threadCount);
   ~CmykRF(void);

   int PreRun(
         CmKernel       *pKernel,
         SurfaceIndex   *pSI_SrcSurfL,
         SurfaceIndex   *pSI_SrcSurfA,
         SurfaceIndex   *pSI_SrcSurfB,
         SurfaceIndex   *pSI_SrcSurfNEM,
         SurfaceIndex   *pSI_DstSurfCMYK,
         SurfaceIndex   *pSI_DstSurfK,
         int             nPicWidth,
         int             nPicHeight
         );
   char* GetIsa() { return "Lab2CMYK_RemoveFringe/CmykRF_genx.isa"; }
   char* GetKernelName() { return "CmykRF_GENX"; }

private:
   CmDevice*      m_pCmDev;
   CmKernel      *m_pKernel;
   int            m_max_Thread_Count;
   float         *m_pXMap;
   float         *m_pYMap;
   float         *m_pZMap;
   uchar         *m_pLatticePoints;
   uchar         m_pLtoK[256];
};

