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
#include "YCbCr2Rgb.h"

#define BLOCK_WIDTH     8
#define BLOCK_HEIGHT    8

YCbCr2Rgb::YCbCr2Rgb(CmDevice *pdevice)
{
   m_pCmDev = pdevice;
   m_pKernel   = NULL;

   float coefficients[4] = {1.402, -0.344136, -0.714136, 1.772};
   std::copy(coefficients, coefficients + 4, m_coeffs);

   float offsets[3] = {0, 128.0, 128.0};
   std::copy(offsets, offsets + 3, m_offsets);
}

int YCbCr2Rgb::PreRun(
      CmKernel      *pKernel,
      SurfaceIndex  *pSI_SrcSurfYCbCr,
      SurfaceIndex  *pSI_DstSurf,
      int            nPicWidth,
      int            nPicHeight
      )
{
   int   result;

   CmThreadSpace *pTS       = NULL;

   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;

   m_pKernel = pKernel;

   nPicWidthInBlk = (nPicWidth + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
   nPicHeightInBlk = (nPicHeight + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT;

   CM_Error_Handle(m_pCmDev->CreateThreadSpace(nPicWidthInBlk, nPicHeightInBlk, pTS), "CreateThreadSpace Error");

   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfYCbCr);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(unsigned int), &nPicHeight);
   result = m_pKernel->SetKernelArg(nKernelInput++, 4*sizeof(float), &m_coeffs[0]);
   result = m_pKernel->SetKernelArg(nKernelInput++, 3*sizeof(float), &m_offsets[0]);
   result = m_pKernel->AssociateThreadSpace(pTS);

   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;

}
