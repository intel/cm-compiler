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
#include "Unpack1to8Pack8to1.h"

#define BLOCK_WIDTH     32
#define BLOCK_HEIGHT    16

Unpack1to8Pack8to1::Unpack1to8Pack8to1(CmDevice *pdevice, int max_Thread_Count)
{
   m_pCmDev = pdevice;
   m_pKernel   = NULL;
   m_max_Thread_Count = max_Thread_Count;
}

Unpack1to8Pack8to1::~Unpack1to8Pack8to1(void)
{
}

int Unpack1to8Pack8to1::PreRun(
      CmKernel      *pKernel,
      SurfaceIndex  *pSI_SrcSurfC,
      SurfaceIndex  *pSI_SrcSurfM,
      SurfaceIndex  *pSI_SrcSurfY,
      SurfaceIndex  *pSI_SrcSurfK,
      SurfaceIndex  *pSI_DstSurfCMYK,
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

   if (m_max_Thread_Count < 4*2)
   {
      printf("Max thread count is too low\n");
   }
   CM_Error_Handle(m_pCmDev->CreateThreadGroupSpace(4 , 2, nPicWidthInBlk/4,
            nPicHeightInBlk/2, pTS), "CreateThreadGroupSpace Error");

   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfC);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfM);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfY);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstSurfCMYK);
   result = m_pKernel->AssociateThreadGroupSpace(pTS);

   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;

}
