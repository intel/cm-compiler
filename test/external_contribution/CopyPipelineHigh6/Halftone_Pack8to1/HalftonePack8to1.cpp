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
#include "HalftonePack8to1.h"
#include "cmykhalftone.h"

#define BLOCK_WIDTH     32
#define BLOCK_HEIGHT    16

HalftonePack8to1::HalftonePack8to1(CmDevice *pdevice, int max_Thread_Count)
{
   m_pCmDev = pdevice;
   m_pKernel   = NULL;
   m_max_Thread_Count = max_Thread_Count;

   m_mapWidthC = C_WIDTH; m_mapHeightC = C_HEIGHT;
   m_mapWidthM = M_WIDTH; m_mapHeightM = M_HEIGHT;
   m_mapWidthY = Y_WIDTH; m_mapHeightY = Y_HEIGHT;
   m_mapWidthK = K_WIDTH; m_mapHeightK = K_HEIGHT;

   uchar *pTmp = NULL;
   unsigned int map_size = 0;
   unsigned int pitch_size = 0;

   m_pCmDev->GetSurface2DInfo(m_mapWidthC, m_mapHeightC, CM_SURFACE_FORMAT_A8,
         pitch_size, map_size);
   m_pCMap = (uchar *)CM_ALIGNED_MALLOC(map_size, 0x1000);
   pTmp = m_pCMap;
   for (int y = 0; y < m_mapHeightC; y++)
   {
      for (int x = 0; x < m_mapWidthC; x++)
      {
         pTmp[x] = C_SCREEN_DATA[y*m_mapWidthC + x];
      }
      pTmp += m_mapWidthC;
   }

   m_pCmDev->GetSurface2DInfo(m_mapWidthM, m_mapHeightM, CM_SURFACE_FORMAT_A8,
         pitch_size, map_size);
   m_pMMap = (uchar *)CM_ALIGNED_MALLOC(map_size, 0x1000);
   pTmp = m_pMMap;
   for (int y = 0; y < m_mapHeightM; y++)
   {
      for (int x = 0; x < m_mapWidthM; x++)
      {
         pTmp[x] = M_SCREEN_DATA[y*m_mapWidthM+ x];
      }
      pTmp += m_mapWidthM;
   }

   m_pCmDev->GetSurface2DInfo(m_mapWidthY, m_mapHeightY, CM_SURFACE_FORMAT_A8,
         pitch_size, map_size);
   m_pYMap = (uchar *)CM_ALIGNED_MALLOC(map_size, 0x1000);
   pTmp = m_pYMap;
   for (int y = 0; y < m_mapHeightY; y++)
   {
      for (int x = 0; x < m_mapWidthY; x++)
      {
         pTmp[x] = Y_SCREEN_DATA[y*m_mapWidthY+ x];
      }
      pTmp += m_mapWidthY;
   }

   m_pCmDev->GetSurface2DInfo(m_mapWidthK, m_mapHeightK, CM_SURFACE_FORMAT_A8,
         pitch_size, map_size);
   m_pKMap = (uchar *)CM_ALIGNED_MALLOC(map_size, 0x1000);
   pTmp = m_pKMap;
   for (int y = 0; y < m_mapHeightK; y++)
   {
      for (int x = 0; x < m_mapWidthK; x++)
      {
         pTmp[x] = K_SCREEN_DATA[y*m_mapWidthK+ x];
      }
      pTmp += m_mapWidthK;
   }
}

HalftonePack8to1::~HalftonePack8to1(void)
{
   free(m_pCMap);
   free(m_pMMap);
   free(m_pYMap);
   free(m_pKMap);
}

int HalftonePack8to1::PreRun(
      CmKernel      *pKernel,
      SurfaceIndex  *pSI_SrcSurfCMYK,
      SurfaceIndex  *pSI_DstBit1C,
      SurfaceIndex  *pSI_DstBit1M,
      SurfaceIndex  *pSI_DstBit1Y,
      SurfaceIndex  *pSI_DstK,
      int            nPicWidth,
      int            nPicHeight
      )
{
   int   result;

   CmThreadGroupSpace *pTS       = NULL;
   SurfaceIndex   *pSI_MapCSurf  = NULL;
   SurfaceIndex   *pSI_MapMSurf  = NULL;
   SurfaceIndex   *pSI_MapYSurf  = NULL;
   SurfaceIndex   *pSI_MapKSurf  = NULL;
   CmSurface2D    *pMapCSurf     = NULL;
   CmSurface2D    *pMapMSurf     = NULL;
   CmSurface2D    *pMapYSurf     = NULL;
   CmSurface2D    *pMapKSurf     = NULL;

   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;

   m_pKernel = pKernel;

   nPicWidthInBlk = nPicWidth / BLOCK_WIDTH;
   nPicHeightInBlk = nPicHeight / BLOCK_HEIGHT;

   if (m_max_Thread_Count < 8*4)
   {
      printf("Max thread count is too low\n");
   }
   CM_Error_Handle(m_pCmDev->CreateThreadGroupSpace(4 , 2, nPicWidthInBlk/4,
            nPicHeightInBlk/2, pTS), "CreateThreadGroupSpace Error");


   result = m_pCmDev->CreateSurface2D(m_mapWidthC, m_mapHeightC, CM_SURFACE_FORMAT_A8, pMapCSurf);
   pMapCSurf->GetIndex(pSI_MapCSurf);
   result = pMapCSurf->WriteSurface(m_pCMap, nullptr);
   result = m_pCmDev->CreateSurface2D(m_mapWidthM, m_mapHeightM, CM_SURFACE_FORMAT_A8, pMapMSurf);
   pMapMSurf->GetIndex(pSI_MapMSurf);
   result = pMapMSurf->WriteSurface(m_pMMap, nullptr);
   result = m_pCmDev->CreateSurface2D(m_mapWidthY, m_mapHeightY, CM_SURFACE_FORMAT_A8, pMapYSurf);
   pMapYSurf->GetIndex(pSI_MapYSurf);
   result = pMapYSurf->WriteSurface(m_pYMap, nullptr);
   result = m_pCmDev->CreateSurface2D(m_mapWidthK, m_mapHeightK, CM_SURFACE_FORMAT_A8, pMapKSurf);
   pMapKSurf->GetIndex(pSI_MapKSurf);
   result = pMapKSurf->WriteSurface(m_pKMap, nullptr);
   CM_Error_Handle(result, "CreateSurface2D for Map surface Error");

   // Set up kernel args for Pipeline filter
   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_SrcSurfCMYK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstBit1C);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstBit1M);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstBit1Y);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_MapCSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_MapMSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_MapYSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_MapKSurf);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &m_mapWidthC);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &m_mapHeightC);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &m_mapWidthM);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &m_mapHeightM);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &m_mapWidthY);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &m_mapHeightY);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &m_mapWidthK);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(int), &m_mapHeightK);
   result = m_pKernel->AssociateThreadGroupSpace(pTS);

   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;

}
