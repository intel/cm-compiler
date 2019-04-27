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
#include "RgbToLab.h"

#define BLOCK_WIDTH     8
#define BLOCK_HEIGHT    8

RgbToLab::RgbToLab(CmDevice *pdevice, int max_Thread_Count)
{
   m_pCmDev = pdevice;
   m_pKernel   = NULL;
   m_Max_Thread_Count = max_Thread_Count;

   // D65 conversion coefficient
   float coeffs[9] = {0.412453, 0.357580, 0.180423, 0.212671, 0.715160, 0.072169, 0.019334, 0.119193,0.950227};
   std::copy(coeffs, coeffs + 9, m_coeffs);

   float offsets[3] = {1.0/0.950455, 1.0/1.000000, 1.0/1.088753};
   std::copy(offsets, offsets + 3, m_offsets);
}


int RgbToLab::PreRun(
      CmKernel *pKernel,
      SurfaceIndex *pSI_SrcRSurf,
      SurfaceIndex *pSI_SrcGSurf,
      SurfaceIndex *pSI_SrcBSurf,
      SurfaceIndex *pSI_DstLSurf,
      SurfaceIndex *pSI_DstASurf,
      SurfaceIndex *pSI_DstBSurf,
      int nPicWidth,
      int nPicHeight
      )
{
   int result;

   CmThreadGroupSpace *pTS           = NULL;
   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;

   m_pKernel = pKernel;

   nPicWidthInBlk = nPicWidth / BLOCK_WIDTH;
   nPicHeightInBlk = nPicHeight / BLOCK_HEIGHT;

   int max_thread_count_per_thread_group = m_Max_Thread_Count;

   if (max_thread_count_per_thread_group < 8*4)
   {
      printf("Max thread count is too low\n");
   }

   CM_Error_Handle(m_pCmDev->CreateThreadGroupSpace(8, 4, nPicWidthInBlk/8,
               nPicHeightInBlk/4, pTS), "CreateThreadGroupSpace Error");

   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcRSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcGSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcBSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstLSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstASurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstBSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, 9*sizeof(float), &m_coeffs[0]);
   result = m_pKernel->SetKernelArg(nKernelInput++, 3*sizeof(float), &m_offsets[0]);
   result = m_pKernel->AssociateThreadGroupSpace(pTS);

   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;

}
