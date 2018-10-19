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
#include "ThresholdProduceEdgeK.h"

#define BLOCK_WIDTH     32
#define BLOCK_HEIGHT    16

ThresholdProduceEdgeK::ThresholdProduceEdgeK(CmDevice *pdevice, int max_Thread_Count)
{
   m_pCmDev = pdevice;
   m_pKernel   = NULL;
   m_max_Thread_Count = max_Thread_Count;

   short m_thresholdK = 96;
   short m_thresholdMaxFilter = 130;
}

ThresholdProduceEdgeK::~ThresholdProduceEdgeK(void)
{
}

int ThresholdProduceEdgeK::PreRun(
      CmKernel      *pKernel,
      SurfaceIndex  *pSI_SrcRFK,
      SurfaceIndex  *pSI_SrcNEM,
      SurfaceIndex  *pSI_SrcHTK,
      SurfaceIndex  *pSI_DstBit1K,
      int            nPicWidth,
      int            nPicHeight
      )
{
   int   result;

   CmThreadGroupSpace *pTS       = NULL;

   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;

   m_pKernel = pKernel;

   nPicWidthInBlk = nPicWidth / BLOCK_WIDTH;
   nPicHeightInBlk = nPicHeight / BLOCK_HEIGHT;

   if (m_max_Thread_Count < 8*4)
   {
      printf("Max thread count is too low\n");
   }
   CM_Error_Handle(m_pCmDev->CreateThreadGroupSpace(8 , 4, nPicWidthInBlk/8,
            nPicHeightInBlk/4, pTS), "CreateThreadGroupSpace Error");

   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcRFK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcNEM);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcHTK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstBit1K);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(short), &m_thresholdK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(short), &m_thresholdMaxFilter);
   result = m_pKernel->AssociateThreadGroupSpace(pTS);

   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;

}
