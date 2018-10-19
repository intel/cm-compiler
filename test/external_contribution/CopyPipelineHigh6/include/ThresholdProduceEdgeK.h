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

class ThresholdProduceEdgeK : public CopyPipeline
{
public:
   ThresholdProduceEdgeK(CmDevice *device, int threadCount);
   ~ThresholdProduceEdgeK(void);

   int PreRun(
         CmKernel       *pKernel,
         SurfaceIndex   *pSI_SrcRFK,
         SurfaceIndex   *pSI_SrcNEM,
         SurfaceIndex   *pSI_SrcHTK,
         SurfaceIndex   *pSI_DstHTK,
         int             nPicWidth,
         int             nPicHeight
         );
   char* GetIsa() { return "Threshold_ProduceEdgeK/ThresholdProduceEdgeK_genx.isa"; }
   char* GetKernelName() { return "ThresholdProduceEdgeK_GENX"; }

private:
   CmDevice*      m_pCmDev;
   CmKernel      *m_pKernel;
   int            m_max_Thread_Count;
   short          m_thresholdK;
   short          m_thresholdMaxFilter;
};

