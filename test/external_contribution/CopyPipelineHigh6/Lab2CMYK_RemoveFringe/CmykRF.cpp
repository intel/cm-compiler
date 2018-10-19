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
#include "CmykRF.h"
#include "lab2cmykparams_17x17x17.h"

#define BLOCK_WIDTH     16
#define BLOCK_HEIGHT    8

#define NLP             17

//using namespace cm:util;
void InitMapDefault(uint32_t nlatticepoints, float* mapout)
{
   for (int i=0; i< 256; i++)
   {
      float realx =( i * (nlatticepoints - 1)) / 255;
      uint32_t x0    = (uint32_t)realx;
      uint32_t prevx = x0 * 255 / (nlatticepoints - 1);
      uint32_t nextx = (x0 + 1) * 255 / (nlatticepoints -1);
      double xd = (double)(i - prevx) / (double)(nextx - prevx);
      mapout[i] = (float)(x0 + xd);
   }
}

CmykRF::CmykRF(CmDevice *pdevice, int max_Thread_Count)
{
   m_pCmDev = pdevice;
   m_pKernel   = NULL;
   m_max_Thread_Count = max_Thread_Count;

   m_pXMap = (float *)CM_ALIGNED_MALLOC(256*sizeof(float), 0x1000);
   m_pYMap = (float *)CM_ALIGNED_MALLOC(256*sizeof(float), 0x1000);
   m_pZMap = (float *)CM_ALIGNED_MALLOC(256*sizeof(float), 0x1000);

   /* Retrieve the right LUT */
   float defaultMap[256];
   InitMapDefault(NLP, &defaultMap[0]);

   for (int i = 0; i < 256; i++)
   {
      m_pXMap[i] = defaultMap[i];
      m_pYMap[i] = defaultMap[i];
      m_pZMap[i] = defaultMap[i];
   }

   m_pLatticePoints = (uchar *)CM_ALIGNED_MALLOC(NLP*NLP*NLP*4*sizeof(unsigned char),0x1000);
   for (int i=0; i < 17*17*17; i++)
   {
      m_pLatticePoints[i*4 + 0] = Y_NODE_VALUES17[i];
      m_pLatticePoints[i*4 + 1] = M_NODE_VALUES17[i];
      m_pLatticePoints[i*4 + 2] = C_NODE_VALUES17[i];
      m_pLatticePoints[i*4 + 3] = K_NODE_VALUES17[i];
   }

   unsigned char LtoK_array_values[256] =
   {
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xfe, 0xfd, 0xfc, 0xfc, 0xfb, 0xfa, 0xf9, 0xf9, 0xf8, 0xf7, 0xf6, 0xf6, 0xf5, 0xf4,
      0xf3, 0xf3, 0xf2, 0xf1, 0xf0, 0xf0, 0xef, 0xee, 0xed, 0xed, 0xec, 0xeb, 0xea, 0xea, 0xe9, 0xe8,
      0xe7, 0xe7, 0xe6, 0xe5, 0xe4, 0xe4, 0xe3, 0xe2, 0xe1, 0xe1, 0xe0, 0xdf, 0xde, 0xde, 0xdd, 0xdc,
      0xdb, 0xdb, 0xda, 0xd9, 0xd8, 0xd8, 0xd7, 0xd6, 0xd5, 0xd5, 0xd4, 0xd3, 0xd2, 0xd2, 0xd1, 0xd0,
      0xcf, 0xcf, 0xce, 0xcd, 0xcc, 0xcc, 0xcb, 0xca, 0xc9, 0xc9, 0xc8, 0xc7, 0xc6, 0xc6, 0xc5, 0xc4,
      0xc3, 0xc3, 0xc2, 0xc1, 0xc0, 0xc0, 0xbf, 0xbe, 0xbd, 0xbd, 0xbc, 0xbb, 0xba, 0xba, 0xb9, 0xb8,
      0xb7, 0xb7, 0xb6, 0xb5, 0xb4, 0xb4, 0xb3, 0xb2, 0xb1, 0xb1, 0xb0, 0xaf, 0xae, 0xae, 0xad, 0xac,
      0xab, 0xa9, 0xa6, 0xa3, 0xa1, 0x9e, 0x9b, 0x99, 0x96, 0x93, 0x91, 0x8e, 0x8b, 0x89, 0x86, 0x83,
      0x80, 0x78, 0x70, 0x68, 0x60, 0x58, 0x50, 0x48, 0x40, 0x38, 0x30, 0x28, 0x20, 0x18, 0x10, 0x08,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };

   std::copy(LtoK_array_values, LtoK_array_values + 256, m_pLtoK);
}

CmykRF::~CmykRF(void)
{
   free(m_pXMap);
   free(m_pYMap);
   free(m_pZMap);
   free(m_pLatticePoints);
}

int CmykRF::PreRun(
      CmKernel      *pKernel,
      SurfaceIndex  *pSI_SrcSurfL,
      SurfaceIndex  *pSI_SrcSurfA,
      SurfaceIndex  *pSI_SrcSurfB,
      SurfaceIndex  *pSI_SrcSurfNEM,
      SurfaceIndex  *pSI_DstSurfCMYK,
      SurfaceIndex  *pSI_DstSurfK,
      int            nPicWidth,
      int            nPicHeight
      )
{
   int   result;

   CmThreadGroupSpace *pTS          = NULL;
   CmSurface3D    *pLatticeSurf     = NULL;     // Lattice Point 3D surface
   CmBuffer     *pXMapSurf        = NULL;
   CmBufferUP     *pYMapSurf        = NULL;
   CmBufferUP     *pZMapSurf        = NULL;
   CmBuffer       *pLtoKSurf        = NULL;
   CmSampler      *pSampler         = NULL;
   SurfaceIndex   *pSI_LatticeSurf  = NULL;
   SurfaceIndex   *pSI_LtoKSurf     = NULL;
   SurfaceIndex   *pSI_XMapSurf     = NULL;
   SurfaceIndex   *pSI_YMapSurf     = NULL;
   SurfaceIndex   *pSI_ZMapSurf     = NULL;
   SamplerIndex   *pSI_Sampler      = NULL;

   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;
   float normalize = 1.0f/NLP;

   m_pKernel = pKernel;

   nPicWidthInBlk = nPicWidth / BLOCK_WIDTH;
   nPicHeightInBlk = nPicHeight / BLOCK_HEIGHT;

   if (m_max_Thread_Count < 4*2)
   {
      printf("Max thread count is too low\n");
   }
   CM_Error_Handle(m_pCmDev->CreateThreadGroupSpace(4 , 2, nPicWidthInBlk/4,
            nPicHeightInBlk/2, pTS), "CreateThreadGroupSpace Error");

   result = m_pCmDev->CreateSurface3D(NLP,NLP,NLP, CM_SURFACE_FORMAT_A8R8G8B8, pLatticeSurf);
   result = pLatticeSurf->WriteSurface(m_pLatticePoints, nullptr);
   result = m_pCmDev->CreateSamplerSurface3D(pLatticeSurf, pSI_LatticeSurf);
   CM_Error_Handle(result, "CreateSurface3D Error");

   result = m_pCmDev->CreateBuffer(256*sizeof(float), pXMapSurf);
   pXMapSurf->GetIndex(pSI_XMapSurf);
   pXMapSurf->WriteSurface((uchar*)m_pXMap, nullptr);
   result = m_pCmDev->CreateBufferUP(256*sizeof(float), m_pYMap, pYMapSurf);
   pYMapSurf->GetIndex(pSI_YMapSurf);
   result = m_pCmDev->CreateBufferUP(256*sizeof(float), m_pZMap, pZMapSurf);
   pZMapSurf->GetIndex(pSI_ZMapSurf);
   CM_Error_Handle(result, "CreateBuffer for XYZ Map Error");

   result = m_pCmDev->CreateBuffer(256*sizeof(uchar), pLtoKSurf);
   result = pLtoKSurf->WriteSurface(m_pLtoK, nullptr);
   pLtoKSurf->GetIndex(pSI_LtoKSurf);
   CM_Error_Handle(result, "CreateBuffer for LtoK Error");

   CM_SAMPLER_STATE  sampler_state;
   sampler_state.magFilterType = CM_TEXTURE_FILTER_TYPE_LINEAR;
   sampler_state.minFilterType = CM_TEXTURE_FILTER_TYPE_LINEAR;
   sampler_state.addressU = CM_TEXTURE_ADDRESS_CLAMP;
   sampler_state.addressV = CM_TEXTURE_ADDRESS_CLAMP;
   sampler_state.addressW = CM_TEXTURE_ADDRESS_CLAMP;
   result = m_pCmDev->CreateSampler(sampler_state, pSampler);
   pSampler->GetIndex(pSI_Sampler);

   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfL);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfA);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfB);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfNEM);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstSurfCMYK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstSurfK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_XMapSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_YMapSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_ZMapSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SamplerIndex), pSI_Sampler);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(float), &normalize);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_LtoKSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_LatticeSurf);
   result = m_pKernel->AssociateThreadGroupSpace(pTS);

   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;

}
