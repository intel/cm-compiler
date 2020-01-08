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
#include "ErrorDiffusion.h"

#define BLOCK_WIDTH     8
#define BLOCK_HEIGHT    4

ErrorDiffusion::ErrorDiffusion(CmDevice *pdevice)
{
   m_pCmDev = pdevice;
   m_pKernel   = NULL;
}

ErrorDiffusion::~ErrorDiffusion(void)
{
   free(m_pErrRowBuf);
   free(m_pErrColBuf);
}

int ErrorDiffusion::PreRun(
      CmKernel      *pKernel,
      SurfaceIndex  *pSI_SrcSurfCMYK,
      SurfaceIndex  *pSI_DstSurfCMYK,
      int            nPicWidth,
      int            nPicHeight
      )
{
   int   result;

   CmThreadSpace *pTS       = NULL;
   CmBuffer *pErrRowBuf = NULL;
   CmBuffer *pErrColBuf = NULL;
   SurfaceIndex *pSI_ErrRowBuf = NULL;
   SurfaceIndex *pSI_ErrColBuf = NULL;

   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;

   m_pKernel = pKernel;

   nPicWidthInBlk = (nPicWidth + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
   nPicHeightInBlk = (nPicHeight + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT;

   CM_Error_Handle(m_pCmDev->CreateThreadSpace((nPicWidthInBlk+1), nPicHeightInBlk, pTS), "CreateThreadGroupSpace Error");

   CM_Error_Handle(pTS->SelectThreadDependencyPattern(CM_WAVEFRONT26), "Select thread dependency error");

   int rowErrBufferSize = (nPicWidthInBlk + 1) * 64;
   uchar *pRowErr = (uchar *) CM_ALIGNED_MALLOC(rowErrBufferSize, 0x1000);
   memset(pRowErr, 0, rowErrBufferSize);

   int colErrBufferSize = nPicHeightInBlk * 128;
   uchar *pColErr = (uchar *) CM_ALIGNED_MALLOC(colErrBufferSize, 0x1000);
   memset(pColErr, 0, colErrBufferSize);

   CM_Error_Handle(m_pCmDev->CreateBuffer(rowErrBufferSize, pErrRowBuf),
         "CreateBuffer Error");
   pErrRowBuf->GetIndex(pSI_ErrRowBuf);

   CM_Error_Handle(m_pCmDev->CreateBuffer(colErrBufferSize, pErrColBuf),
         "CreateBuffer Error");
   pErrColBuf->GetIndex(pSI_ErrColBuf);

   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfCMYK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_ErrRowBuf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_ErrColBuf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstSurfCMYK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &nPicWidthInBlk );
   result = m_pKernel->AssociateThreadSpace(pTS);

   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;

}
