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
#include "WarpAffine.h"

/*********************************************************************************************************************************/

#define BLOCK_WIDTH     16
#define BLOCK_HEIGHT    8


const float coeffs[128] = { 0.0f, 1.0f, 0.0f, 0.0f, -0.014663696f, 0.99760437f, 0.017532349f, -0.000473022f, 
   -0.02746582f, 0.990600586f, 0.038696289f, -0.001831055f, -0.038497925f, 0.979263306f, 0.063217163f, -0.003982544f,
   -0.047851563f, 0.963867188f, 0.090820313f, -0.006835938f, -0.055618286f, 0.94468689f, 0.121231079f, -0.010299683f,
   -0.061889648f, 0.92199707f, 0.154174805f, -0.014282227f, -0.066757202f, 0.896072388f, 0.189376831f, -0.018692017f,
   -0.0703125f, 0.8671875f, 0.2265625f, -0.0234375f, -0.072647095f, 0.835617065f, 0.265457153f, -0.028427124f,
   -0.073852539f, 0.801635742f, 0.305786133f, -0.033569336f, -0.074020386f, 0.765518188f, 0.34727478f, -0.038772583f,
   -0.073242188f, 0.727539063f, 0.389648438f, -0.043945313f, -0.071609497f, 0.687973022f, 0.432632446f, -0.048995972f,
   -0.069213867f, 0.647094727f, 0.475952148f, -0.053833008f, -0.066146851f, 0.605178833f, 0.519332886f, -0.058364868f,
   -0.0625f, 0.5625f, 0.5625f, -0.0625f, -0.058364868f, 0.519332886f, 0.605178833f, -0.066146851f,
   -0.053833008f, 0.475952148f, 0.647094727f, -0.069213867f, -0.048995972f, 0.432632446f, 0.687973022f, -0.071609497f,
   -0.043945313f, 0.389648438f, 0.727539063f, -0.073242188f, -0.038772583f, 0.34727478f, 0.765518188f, -0.074020386f,
   -0.033569336f, 0.305786133f, 0.801635742f, -0.073852539f, -0.028427124f, 0.265457153f, 0.835617065f, -0.072647095f,
   -0.0234375f, 0.2265625f, 0.8671875f, -0.0703125f, -0.018692017f, 0.189376831f, 0.896072388f, -0.066757202f,
   -0.014282227f, 0.154174805f, 0.92199707f, -0.061889648f, -0.010299683f, 0.121231079f, 0.94468689f, -0.055618286f,
   -0.006835938f, 0.090820313f, 0.963867188f, -0.047851563f, -0.003982544f, 0.063217163f, 0.979263306f, -0.038497925f,
   -0.001831055f, 0.038696289f, 0.990600586f, -0.02746582f, -0.000473022f, 0.017532349f, 0.99760437f, -0.014663696f };

/*********************************************************************************************************************************/

void GetRotationMatrix(
   double center_x,
   double center_y,
   double angle,
   double scale,
   float Matrix[3][2]
   )
{
   const double PI = 3.1415926535897932384626433832795;
   angle *= PI/180;

   double alpha = cos(angle)*scale;
   double beta = sin(angle)*scale;

   Matrix[0][0] = (float) alpha;
   Matrix[0][1] = (float) -beta;
   Matrix[1][0] = (float) beta;
   Matrix[1][1] = (float) alpha;
   Matrix[2][0] = (float) ((1-alpha)*center_x - beta*center_y);
   Matrix[2][1] = (float) (beta*center_x + (1-alpha)*center_y);
}

/*********************************************************************************************************************************/

void InvertMatrix(float Matrix[3][2])
{
   float alpha = Matrix[0][0];
   float beta = Matrix[0][1];
   float Tx = Matrix[2][0];
   float Ty = Matrix[2][1];

   Matrix[1][0] = beta;
   Matrix[0][1] = -beta;

   Matrix[2][0] = -1.0f*Tx*alpha - Ty*beta;
   Matrix[2][1] = -1.0f*Ty*alpha + Tx*beta;
}

/*********************************************************************************************************************************/

void GetRotateBound(
   uint64_t angle,
   uint32_t input_width,
   uint32_t input_height,
   float Matrix[3][2],
   uint32_t &output_width,
   uint32_t &output_height
   )
{
   float center_x = input_width / 2.0f;
   float center_y = input_height / 2.0f;

   GetRotationMatrix(center_x, center_y, angle, 1.0f, Matrix);

   float cos = fabs(Matrix[0][0]);
   float sin = fabs(Matrix[1][0]);

   output_width = (int)(input_height*sin + input_width*cos);
   output_height = (int)(input_height*cos + input_width*sin);

   Matrix[2][0] += (output_width / 2.0f) - center_x;
   Matrix[2][1] += (output_height / 2.0f) - center_y;

   InvertMatrix(Matrix);
}

/*********************************************************************************************************************************/

WarpAffine::WarpAffine(
   CmDevice *pdevice
   )
{
   m_pCmDev    = pdevice;
   m_pKernel   = NULL;
}

/*********************************************************************************************************************************/

int WarpAffine::GetSkew(
   SkewType skewType,
   unsigned int inputWidth,
   unsigned int inputHeight,
   unsigned int *outputWidth,
   unsigned int *outputHeight,
   int skew
   )
{
   unsigned int skewedWidth = 0;
   unsigned int skewedHeight = 0;
   double skewAngle = (double) skew;

   if( skewType == SKEW )
   {
      GetRotateBound(skewAngle, inputWidth, inputHeight, Matrix, skewedWidth, skewedHeight);

      unsigned int interpitch, intersize;
      m_pCmDev->GetSurface2DInfo(skewedWidth, skewedHeight, CM_SURFACE_FORMAT_A8R8G8B8, interpitch, intersize);

      skewedWidth = interpitch/4;
      *outputWidth   = skewedWidth;
      *outputHeight  = skewedHeight;
   }

   else if( skewType == DESKEW )
   {
      GetRotateBound(skewAngle, inputWidth, inputHeight, Matrix, skewedWidth, skewedHeight); 

      InvertMatrix(Matrix);

      *outputWidth   = inputWidth;
      *outputHeight  = inputHeight;
   }
}

/*********************************************************************************************************************************/

int WarpAffine::PreRun(
   CmKernel *pKernel,
   SurfaceIndex *pSI_Src,
   SurfaceIndex *pSI_DstR,
   SurfaceIndex *pSI_DstG,
   SurfaceIndex *pSI_DstB,
   unsigned int inputWidth,
   unsigned int inputHeight,
   unsigned int outputWidth,
   unsigned int outputHeight
   )
{
   CmThreadSpace *pTS = NULL;
   m_pKernel = pKernel;

   int nPicWidthInBlk, nPicHeightInBlk;
   int nKernelInput;
   int result;

   nPicWidthInBlk = (outputWidth + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
   nPicHeightInBlk = (outputHeight + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT;

   CM_Error_Handle(m_pCmDev->CreateThreadSpace(nPicWidthInBlk, nPicHeightInBlk, pTS), "CreateThreadGroupSpace Error");

   CmSampler *sampler = nullptr;
   CM_SAMPLER_STATE_EX sampler_state;
   sampler_state.magFilterType = CM_TEXTURE_FILTER_TYPE_POINT;
   sampler_state.minFilterType = CM_TEXTURE_FILTER_TYPE_POINT;
   sampler_state.addressU = CM_TEXTURE_ADDRESS_BORDER;
   sampler_state.addressV = CM_TEXTURE_ADDRESS_BORDER;
   sampler_state.addressW = CM_TEXTURE_ADDRESS_BORDER;
   sampler_state.BorderColorRedU = 0x0;
   sampler_state.BorderColorGreenU = 0x0;
   sampler_state.BorderColorBlueU = 0x0;
   sampler_state.SurfaceFormat = CM_PIXEL_OTHER;
   result = m_pCmDev->CreateSamplerEx(sampler_state, sampler);
   CM_Error_Handle(result, "Create Sampler Error");

   SamplerIndex *sampler_index = nullptr;
   result = sampler->GetIndex(sampler_index);
   CM_Error_Handle(result, "Create SamplerIndex Error");

   CmBuffer *coeff_surface = nullptr;
   SurfaceIndex *coeff_surface_index = nullptr;
   result = m_pCmDev->CreateBuffer(128*4, coeff_surface);
   result = coeff_surface->WriteSurface((uchar *)&coeffs[0], nullptr);
   result = coeff_surface->GetIndex(coeff_surface_index);
   CM_Error_Handle(result, "Create Coeffs Error");

   float coord_unit_u = 1.0f / static_cast<float>(inputWidth);
   float coord_unit_v = 1.0f / static_cast<float>(inputHeight);

   nKernelInput = 0;
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SamplerIndex), sampler_index);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_Src);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstR);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstG);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), pSI_DstB);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(SurfaceIndex), coeff_surface_index);
   result = m_pKernel->SetKernelArg(nKernelInput++, sizeof(float)*3*2, Matrix);
   result = m_pKernel->SetKernelArg(nKernelInput++, 4, &coord_unit_u);
   result = m_pKernel->SetKernelArg(nKernelInput++, 4, &coord_unit_v);
   result = m_pKernel->AssociateThreadSpace(pTS);

   CM_Error_Handle(result, "SetKernelArg Error");

   return CM_SUCCESS;
}
