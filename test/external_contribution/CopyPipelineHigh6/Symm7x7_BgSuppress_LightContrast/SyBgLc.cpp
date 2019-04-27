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
#include "SyBgLc.h"
#include "lightnessdarknesscontrast_params.h"

#define BLOCK_WIDTH     16
#define BLOCK_HEIGHT    16

// Indices for 10 unique filter coefficients
// These 10 coefficients are used to construct the symmetric 7x7 filter shown below
// 
// J  I  H  G  H  I  J
// I  F  E  D  E  F  I
// H  E  C  B  C  E  H
// G  D  B  A  B  D  G
// H  E  C  B  C  E  H
// I  F  E  D  E  F  I
// J  I  H  G  H  I  J

#define  A 0
#define  B 1
#define  C 2
#define  D 3
#define  E 4
#define  F 5
#define  G 6
#define  H 7
#define  I 8
#define  J 9

//using namespace cm:util;

void InitializeLightnessDarknessContrastLUT(int lightness, int contrast, uchar * pLUT)
{
   if( (lightness > 3) || (lightness < -3) )
      lightness = 0;

   if( (contrast > 2) || (contrast < -2) )
      contrast = 0;

   //normalize the parameters
   lightness += 3;
   contrast += 2;

   //calculate the start index into our params table
   int start_index = (lightness*5*256) + contrast*256;

   for( int i = 0; i < 256; i++ )
   {
      pLUT[i] = LIGHT_DARK_CONSTRAST[i + start_index];
   }
}

SyBgLc::SyBgLc(CmDevice *pdevice, int max_Thread_Count, int lightness_, int
      contrast_)
{
   m_pCmDev = pdevice;
   m_pKernel   = NULL;
   m_max_Thread_Count = max_Thread_Count;

   // coefficient
   int coeffs[10] = {1140, -118, 526, 290, -236, 64, -128, -5, -87, -7};
   std::copy(coeffs, coeffs + 10, m_coeffs);

   m_shift = 10;

   int lightness = lightness_;
   int contrast = contrast_;

   m_pLUT = (uchar *)CM_ALIGNED_MALLOC(256, 0x1000);
   InitializeLightnessDarknessContrastLUT(lightness, contrast, m_pLUT);
}

SyBgLc::~SyBgLc(void)
{
   free(m_pLUT);
}

int SyBgLc::PreRun(
      CmKernel      *pKernel,
      SurfaceIndex  *pSI_SrcSurfL,
      SurfaceIndex  *pSI_SrcSurfA,
      SurfaceIndex  *pSI_SrcSurfB,
      SurfaceIndex   *pSI_DstSurfSymm,
      SurfaceIndex   *pSI_DstSurfA,
      SurfaceIndex   *pSI_DstSurfB,
      int            nPicWidth,
      int            nPicHeight
      )
{
   int   result;
   short wCoeffs[10];

   CmThreadGroupSpace *pTS          = NULL;
   SurfaceIndex   *pSI_LUTSurf      = NULL;
   CmBuffer       *pLUTSurf         = NULL;

   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;

   m_pKernel = pKernel;

   nPicWidthInBlk = (nPicWidth + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
   nPicHeightInBlk = (nPicHeight + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT;

   int threads = nPicWidthInBlk * nPicHeightInBlk;
   int max_thread_count_per_thread_group = m_max_Thread_Count;

   if (max_thread_count_per_thread_group < 4*2)
   {
      printf("Max thread count is too low\n");
   }
   CM_Error_Handle(m_pCmDev->CreateThreadGroupSpace(4 , 2, nPicWidthInBlk/4,
            nPicHeightInBlk/2, pTS), "CreateThreadGroupSpace Error");

   result = m_pCmDev->CreateBuffer(256 * sizeof(unsigned char) , pLUTSurf);
   result = pLUTSurf->GetIndex(pSI_LUTSurf);
   result = pLUTSurf->WriteSurface(m_pLUT, nullptr);

   CM_Error_Handle(result, "Create LUT Error");

   for (int i = 0; i < 10; i++)
      wCoeffs[i] = m_coeffs[i];


   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfL);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfA);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfB);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstSurfSymm);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstSurfA);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstSurfB);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_LUTSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, 10*sizeof(short), &wCoeffs[0]);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &m_shift);
   result = m_pKernel->AssociateThreadGroupSpace(pTS);

   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;

}
