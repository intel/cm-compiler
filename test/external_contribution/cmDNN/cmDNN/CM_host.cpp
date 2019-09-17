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

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>

#include <cm_rt.h>
//#include "half.h"
#include "cmvm.h"

#include "cmDNN.h"

//using namespace half_float;

extern void LinearToTiledImage(float * pSrc, float * pDst, int DstPitch, int Images, int HoriTiles, 
						int TileWidth, int TileHeight, int BorderWidth, int ImgWidth, int ImgHeight, int ImgDepth, int UseFP16);

extern int SaveBMPFile(char fn[], void * Buffer, int Pitch, int Width, int Height, int UseFP16);

#ifdef CMRT_EMU
// FP32
extern "C" void InputProc(SurfaceIndex iIndex, SurfaceIndex iIndex2, SurfaceIndex oIndex);
extern "C" void Convol3x3_IPT16_2D(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex kIndex, 
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
									uint oTileHoriCount, uint Border, uint NumGroups, uint EnableReLU);
extern "C" void Convol1x1_IPT16_2D(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex kIndex,
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileWidth, uint iTileHeight,
									uint oTileHoriCount, uint Border, uint NumGroups, uint EnableReLU);
extern "C" void Convol3x3_BPT_8x8_MB(SurfaceIndex iIndex, SurfaceIndex iIndex2, int NumInputs,
									SurfaceIndex oIndex, SurfaceIndex kIndex,
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileVertCount, uint iTileWidth, uint iTileHeight, 
									uint oTileHoriCount, uint oTileVertCount, uint oTileWidth, uint oTileHeight, 
									uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
									uint Border, uint EnableReLU);
extern "C" void Convol1x1_BPT_8x8_MB(SurfaceIndex iIndex, SurfaceIndex iIndex2, int NumInputs,
									SurfaceIndex oIndex, SurfaceIndex kIndex,
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileVertCount, uint iTileWidth, uint iTileHeight,
									uint oTileHoriCount, uint oTileVertCount, uint oTileWidth, uint oTileHeight,
									uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
									uint Border, uint EnableReLU);
extern "C" void Convol1x1_BPT_8x8_MB_S2(SurfaceIndex iIndex, SurfaceIndex iIndex2, int NumInputs,
									SurfaceIndex oIndex, SurfaceIndex kIndex,
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileVertCount, uint iTileWidth, uint iTileHeight,
									uint oTileHoriCount, uint oTileVertCount, uint oTileWidth, uint oTileHeight,
									uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
									uint Border, uint EnableReLU);
extern "C" void Convol5x5_BPT27x8(	SurfaceIndex iIndex, SurfaceIndex iIndex2, int NumInputs,
									SurfaceIndex oIndex, SurfaceIndex kIndex,
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
									uint oTileHoriCount, uint oTileWidth, uint oTileHeight, 
									uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
									uint Border, uint NumGroups, uint EnableReLU); 
extern "C" void Convol7x7_BPT(SurfaceIndex iIndex, SurfaceIndex iIndex2, int NumInputs,
									SurfaceIndex oIndex, SurfaceIndex kIndex,
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileWidth, uint iTileHeight,
									uint oTileHoriCount, uint oTileWidth, uint oTileHeight,
									uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
									uint Border, uint NumGroups, uint EnableReLU);
extern "C" void Convol11x11_BPT(SurfaceIndex iIndex, SurfaceIndex iIndex2, int NumInputs,
									SurfaceIndex oIndex, SurfaceIndex kIndex,
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
									uint oTileHoriCount, uint oTileWidth, uint oTileHeight, 
									uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
									uint Border, uint NumGroups, uint EnableReLU); 
extern "C" void LRN(SurfaceIndex INBUF, SurfaceIndex OUTBUF, float Alpha, float Beta, float K, 
									int Width, int Height, int Blocks, int Images, int Channel);
extern "C" void MaxPooling2D(SurfaceIndex INBUF, SurfaceIndex OUTBUF, vector<int, 2> ThreadDim,	vector<int, 2> ImageDim8, 
									vector<int, 2> ImageDim, vector<int, 2> ImageBorder, int PoolSize);
extern "C" void MaxPooling1D(SurfaceIndex INBUF, SurfaceIndex OUTBUF, vector<int, 2> ImageBorder, int Images, int Input2D, int PoolSize);
extern "C" void AvgPooling2D(SurfaceIndex INBUF, SurfaceIndex OUTBUF, int PoolSize, int iTileHoriCount);
extern "C" void FullyConnection_NPatch(SurfaceIndex INBUF, SurfaceIndex OUTBUF,	SurfaceIndex COEFF,	SurfaceIndex BIAS, 
									int OneSrcBatchSize,	int OneDestBatchSize, uint HoriThreads,	int Batches, int ReLU);
extern "C" void FullyConnection_NPatch_QWT(SurfaceIndex INBUF, SurfaceIndex OUTBUF, SurfaceIndex COEFF,	SurfaceIndex BIAS,	
									int OneSrcBatchSize, int OneDestBatchSize, uint HoriThreads, int Batches, int ReLU, float fA, float fB);
extern "C" void ReLU(SurfaceIndex iIndex, SurfaceIndex oIndex, int InputMemType, int OutputMemType, int ImagesPerRow);
extern "C" void SurfConv(SurfaceIndex iIndex, SurfaceIndex oIndex, int ImagesPerRow, int DstWidth);
extern "C" void Softmax(SurfaceIndex INBUF,	SurfaceIndex OUTBUF, int oDepth, int FeaturesPerThread);

// FP16
extern "C" void InputProc_HF(SurfaceIndex iIndex, SurfaceIndex iIndex2, SurfaceIndex oIndex);

extern "C" void Convol3x3_IPT_2D_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex kIndex, 
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iBorder, uint oBorder, uint NumGroups, uint EnableReLU, 
									uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
									uint oTileHoriCount, uint oTileWidth, uint oTileHeight );

extern "C" void Convol3x3_BPT_8x8_MB_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex kIndex, 
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iBorder, uint oBorder, uint EnableReLU,
									uint iTileHoriCount, uint iTileVertCount, uint iTileWidth, uint iTileHeight, 
									uint oTileHoriCount, uint oTileVertCount, uint oTileWidth, uint oTileHeight, 
									uint ThreadsWidthPerTile, uint ThreadsHeightPerTile); 

extern "C" void Convol5x5_BPT27x16_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex kIndex, 
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
									uint oTileHoriCount, uint oTileWidth, uint oTileHeight, 
									uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
									uint Border, uint NumGroups, uint EnableReLU); 

extern "C" void Convol11x11_BPT_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex kIndex, 
									uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
									uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
									uint oTileHoriCount, uint oTileWidth, uint oTileHeight, 
									uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
									uint oBorder, uint NumGroups, uint EnableReLU); 

extern "C" void ReLU_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, int InputMemType, int OutputMemType, int ImagesPerRow);

extern "C" void SurfConv_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, int ImagesPerRow, int DstWidth);

extern "C" void LRN_HF(SurfaceIndex INBUF, SurfaceIndex OUTBUF, float Alpha, float Beta, float K,									
									int Width, int Height, int Blocks, int Images, int Channel);
extern "C" void MaxPooling2D_HF(SurfaceIndex INBUF,	SurfaceIndex OUTBUF, vector<int, 2> ThreadDim, vector<int, 2> ImageDim8, 
									vector<int, 2> ImageDim, vector<int, 2> ImageBorder, int PoolSize);
extern "C" void MaxPooling1D_HF(SurfaceIndex INBUF, SurfaceIndex OUTBUF, vector<int, 2> ImageBorder, int Images, int Input2D, int PoolSize);
extern "C" void AvgPooling2D_HF(SurfaceIndex INBUF, SurfaceIndex OUTBUF, int PoolSize, int iTileHoriCount);
extern "C" void FullyConnection_NPatch_HF(SurfaceIndex INBUF, SurfaceIndex OUTBUF, SurfaceIndex COEFF, SurfaceIndex BIAS, 
									int OneSrcBatchSize, int OneDestBatchSize, uint HoriThreads, int Batches, int ReLU);
extern "C" void FullyConnection_NPatch_HF_QWT(SurfaceIndex INBUF, SurfaceIndex OUTBUF, SurfaceIndex COEFF, SurfaceIndex BIAS, 
									int OneSrcBatchSize, int OneDestBatchSize, uint HoriThreads, int Batches, int ReLU, float fA, float fB);

extern "C" void Softmax_HF(SurfaceIndex INBUF, SurfaceIndex OUTBUF, int oDepth, int FeaturesPerThread);
#endif

///////////////////////////////////////////////////////////////////////////
int CM_LAYER::CreateKernel_InputProc()
{
//	return 0;

	int result;

	if (!UseFP16)
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(InputProc),  pCmKernel);
	else
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(InputProc_HF),  pCmKernel);

	if (result != CM_SUCCESS ) {
		printf("CreateKernel error, %d\n", result);
		return -1;
	}

	pCmKernel->SetThreadCount( ThreadsWidth * ThreadsHeight );

	int ParaIdx = 0;
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex2);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.OutputBorderWidth);

	pThrdSpace = NULL;
	result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth, ThreadsHeight, pThrdSpace);
//	pTGS = NULL;
//	result = pCm->pCmDev->CreateThreadGroupSpace(1, 1, ThreadsWidth, ThreadsHeight, pTGS);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// IPT = Image Per Thread
// for image Param.SrcWidth < 16 && Param.SrcHeight < 16
int CM_LAYER::CreateKernel_Convol_IPT(uint KernelWidth)
{
	if (!IsActiveLayer())
		return 0;

	int result;
	switch (KernelWidth) {
/*		case 1:
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol1x1_IPT16_2D), pCmKernel);
			printf("Kernel->Convol1x1_IPT16_2D\n");
			break;*/
		case 3:	
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol3x3_IPT16_2D) , pCmKernel);
			printf("Kernel->Convol3x3_IPT16_2D\n");
			break;
		default: 
			printf("Invalid kernel size %d\n", KernelWidth);
			return -1;
	}
	if (result != CM_SUCCESS ) {
		printf("CreateKernel error, %d\n", result);
		return -1;
	}

    // Thread space is the same as the output tile dimension. 
	int threadswidth  = oTileHoriCount;  
	int threadsheight = oTileVertCount; 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);

//	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightBiasIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pBiasIndex);

	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.SrcDepth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestDepth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestWidth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestHeight);

	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileWidth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHeight);

	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHoriCount);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.OutputBorderWidth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.NumGroups);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.EnableReLU);

	pThrdSpace = NULL;
	result = pCm->pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);
//	pTGS = NULL;
//	result = pCm->pCmDev->CreateThreadGroupSpace(ThreadsWidth, 1, 1, ThreadsHeight, pTGS);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
int CM_LAYER::CreateKernel_Convol_BPT(uint KernelWidth)
{
	if (!IsActiveLayer())
		return 0;

	int result;
	switch (KernelWidth) {
		case 1:
			if (Param.KernelStride == 1) {
				result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol1x1_BPT_8x8_MB), pCmKernel);
				printf("Kernel->Convol1x1_BPT_8x8_MB\n");
			} else if (Param.KernelStride == 2) {
				result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol1x1_BPT_8x8_MB_S2), pCmKernel);
				printf("Kernel->Convol1x1_BPT_8x8_MB_S2\n");
			}
			else {
				printf("Invalid stirde %d in Convol1x1_BPT_8x8_MB_S2\n", Param.KernelStride);
				return -1;
			}
			break;
		case 3:	
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol3x3_BPT_8x8_MB), pCmKernel);
			printf("Kernel->Convol3x3_BPT_8x8_MB\n");
			break;
		case 5:	
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol5x5_BPT27x8), pCmKernel);
			printf("Convol5x5_BPT27x8\n");
			break;
		case 7:
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol7x7_BPT), pCmKernel);
			printf("Convol7x7_BPT\n");
			if (Param.NumGroups > 1) {
				printf("NumGroups = %d. Convol7x7_BPT kernel does not support NumGroups > 1.\n", Param.NumGroups);
				return -1;
			}
			break;
		case 11:
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol11x11_BPT), pCmKernel);
			printf("Convol11x11_BPT\n");
			break;
		default: 
			printf("Invalid kernel size %d\n", KernelWidth);
			return -1;
	}
	if (result != CM_SUCCESS ) {
		printf("pCmDev->CreateKernel error, %d\n", result);
		return -1;
	}

    // Find # of threads based on output frame size in pixel
    pCmKernel->SetThreadCount( ThreadsWidth * ThreadsHeight );

	// Set curbe data
	int ParaIdx = 0;
	pThrdSpace = NULL;

	if (KernelWidth == 1) {				// for multiple output blocks, and 1 and 2 inputs
		pCmKernel->SetThreadCount(ThreadsWidth / MULTIBLOCKS * ThreadsHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), (NumInputs == 2) ? pInputIndex2 : pInputIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &NumInputs);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);

//		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightBiasIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pBiasIndex);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.SrcDepth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestDepth);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileVertCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileVertCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsWidthPerTile);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsHeightPerTile);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.OutputBorderWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), (NumInputs == 2) ? &pPrevLayer2->Param.OutputBorderWidth : &Param.OutputBorderWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.EnableReLU);

		result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth / MULTIBLOCKS, ThreadsHeight, pThrdSpace);

	} else if (KernelWidth == 3) {		// for multiple output blocks and 1 input
		pCmKernel->SetThreadCount( ThreadsWidth / MULTIBLOCKS * ThreadsHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);

//		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightBiasIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pBiasIndex);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.SrcDepth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestDepth);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileVertCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileVertCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsWidthPerTile);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsHeightPerTile);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.OutputBorderWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.EnableReLU);

		result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth / MULTIBLOCKS, ThreadsHeight, pThrdSpace);

	} else {							// For the rest of kernels with kernel size = 5, 7, and 11.
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);

//		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightBiasIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pBiasIndex);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.SrcDepth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestDepth);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsWidthPerTile);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsHeightPerTile);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.OutputBorderWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.NumGroups);	
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.EnableReLU);

		result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth, ThreadsHeight, pThrdSpace);
	}

	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}

///////////////////////////////////////////////////////////////////////////
int CM_LAYER::CreateKernel_LRN()
{
	if (!IsActiveLayer())
		return 0;
    int result;
	if(!UseFP16)
	    result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(LRN),  pCmKernel);
	else 
	    result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(LRN_HF),  pCmKernel);
	if (result != CM_SUCCESS ) {
		printf("pCmDev->CreateKernel error, %d\n", result);
		return -1;
	}

	// To bo modified
	// Currently all output images are put on the same row in thread space.  Can run over the max horizontal thread space for large images.
	// Thread space is equally spaced regardless if the block is on the border.   
//	ThreadsWidth = oTileWidth/oBlockWidth*Param.DestDepth;				// 64/8*96;		32/8*256
//	ThreadsHeight = oTileHeight/oBlockHeight;							// 64/8;		32/8
	ThreadsWidth = oTileWidth/oBlockWidth*oTileHoriCount;				// 64/8*12
	ThreadsHeight = oTileHeight/oBlockHeight*oTileVertCount;			// 64/8*8
	pCmKernel->SetThreadCount(ThreadsWidth * ThreadsHeight);

	float Alpha = Param.Alpha/Param.WindowChannelSize;	//input, alpha /= 25 spatial; alpha /= 5, cross channel
	float Beta = Param.Beta;							//input,
	float K = Param.K;									//input,
	int Width = oTileWidth/oBlockWidth;					//input, image width of block
	int Height = oTileHeight/oBlockHeight;				//input, image height of block
	int Blocks = oTileWidth/oBlockWidth*oTileHoriCount;	//input, block number per row, 64/8*12
//	int MaxBlock = ThreadsWidth * ThreadsHeight;		//input, total block number, 6144
	int Images = Param.DestDepth;						//input, total image per batch
	int Channel	= 1;									//input, accross channel

	int ParaIdx = 0;
 	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);

	pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &Alpha);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &Beta);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &K);
	
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int),  &Width);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int),  &Height);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int),  &Blocks);
//	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int),  &MaxBlock);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int),  &Images);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int),  &Channel);

	result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth, ThreadsHeight, pThrdSpace);
//	pTGS = NULL;
//	result = pCm->pCmDev->CreateThreadGroupSpace(1, 1, ThreadsWidth, ThreadsHeight, pTGS);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}

///////////////////////////////////////////////////////////////////////////
int CM_LAYER::CreateKernel_MaxPool()
{
	if (!IsActiveLayer())
		return 0;
    int result;

	if (!UseFP16)
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(MaxPooling2D),  pCmKernel);
	else
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(MaxPooling2D_HF),  pCmKernel);
	if (result != CM_SUCCESS ) {
		printf("pCmDev->CreateKernel error, %d\n", result);
		return -1;
	}

	pCmKernel->SetThreadCount(ThreadsWidth * ThreadsHeight);

	vector<int, 2> ThreadsPerTile;		//input, thread width/height per tile
	vector<int, 2> oTileSize;			//input, otile width/height
	vector<int, 2> oDestSize;			//input, oDest width/height
	vector<int, 2> ImageBorder;			//input, left/top border for one image

	ThreadsPerTile[0] = ThreadsWidthPerTile;
	ThreadsPerTile[1] = ThreadsHeightPerTile;
	oTileSize[0] = oTileWidth;
	oTileSize[1] = oTileHeight;
	oDestSize[0] = Param.DestWidth;
	oDestSize[1] = Param.DestHeight;
	ImageBorder[0] = ImageBorder[1] = Param.InputBorderWidth;

	int ParaIdx = 0;
 	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, ThreadsPerTile.get_size_data(), ThreadsPerTile.get_addr_data());
	pCmKernel->SetKernelArg(ParaIdx++, oTileSize.get_size_data(), oTileSize.get_addr_data());
	pCmKernel->SetKernelArg(ParaIdx++, oDestSize.get_size_data(),  oDestSize.get_addr_data());
	pCmKernel->SetKernelArg(ParaIdx++, ImageBorder.get_size_data(), ImageBorder.get_addr_data());
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.KernelWidth);

	result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth, ThreadsHeight, pThrdSpace);
//	pTGS = NULL;
//	result = pCm->pCmDev->CreateThreadGroupSpace(1, 1, ThreadsWidth, ThreadsHeight, pTGS);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}

///////////////////////////////////////////////////////////
int CM_LAYER::CreateKernel_AvgPool()
{
	if (!IsActiveLayer())
		return 0;
	int result;

	if (!UseFP16)
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(AvgPooling2D), pCmKernel);
	else
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(AvgPooling2D_HF), pCmKernel);
	if (result != CM_SUCCESS) {
		printf("pCmDev->CreateKernel error, %d\n", result);
		return -1;
	}

	// Use input tiles as thread space.
	ThreadsWidth = iTileHoriCount;
	ThreadsHeight = iTileVertCount;
	pCmKernel->SetThreadCount(ThreadsWidth * ThreadsHeight);

	int ParaIdx = 0;
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.KernelWidth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);

	result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth, ThreadsHeight, pThrdSpace);
	//	pTGS = NULL;
	//	result = pCm->pCmDev->CreateThreadGroupSpace(1, 1, ThreadsWidth, ThreadsHeight, pTGS);
	if (result != CM_SUCCESS) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}

///////////////////////////////////////////////////////////
// Process multi-batch
int CM_LAYER::CreateKernel_FC_NPatch()
{
	if (!IsActiveLayer())
		return 0;

	int result;
	if (!UseFP16)
		if(!qFCWt)
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(FullyConnection_NPatch), pCmKernel);
		else
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(FullyConnection_NPatch_QWT), pCmKernel);
	else
		if(!qFCWt)
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(FullyConnection_NPatch_HF), pCmKernel);
		else
			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(FullyConnection_NPatch_HF_QWT), pCmKernel);
	if (result != CM_SUCCESS ) {
		printf("pCmDev->CreateKernel error, %d\n", result);
		return -1;
	}

	if (Param.BatchSize > 100) {
		printf("Batch size exceeds the max limit 100.\n");
		return -1;
	}

	int BlockWidth = 8; //32;
	int BlockHeight = 1;
	if(UseFP16)
		BlockWidth = 16;

	// Calculate thread space for sample per batch.  Multiple samples upto 64 is processed in the kernel with Param.BatchSize.
	ThreadsWidth = (int) ceil((float) oDepth/BlockWidth);							// One sample per batch
	ThreadsHeight = BlockHeight;

	uint HoriThreads, VertThreads;
	FindImageTile(ThreadsWidth, 1, HoriThreads, VertThreads);						// Find thread space for one batch.

	pCmKernel->SetThreadCount( HoriThreads * VertThreads);

	int OneSrcBatchSize = Param.SrcWidth * Param.SrcHeight * Param.SrcDepth;		// One sample per batch. Input size for FC per batch, 6x6x256 
	int OneDestBatchSize = Param.DestWidth * Param.DestHeight * oDepth;		

	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pBiasIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &OneSrcBatchSize);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &OneDestBatchSize);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &HoriThreads);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.BatchSize);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.EnableReLU);
	if(qFCWt) {			// Compute inverse quantization parameters
		// Compute decompress parameter used by kernels
		char bMin, bMax;
		float m_fA, m_fB;

		bMin = -128;
		bMax = 127;		// Range of INT8 data type
		m_fA = (fWMax - fWMin) / (bMax - bMin);
		m_fB = fWMin - m_fA * bMin;
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &m_fA);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &m_fB);
	}

	result = pCm->pCmDev->CreateThreadSpace(HoriThreads, VertThreads, pThrdSpace);
//	pTGS = NULL;
//	result = pCm->pCmDev->CreateThreadGroupSpace(1, 1, HoriThreads, VertThreads, pTGS);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}

///////////////////////////////////////////////////////////////////////////
int CM_LAYER::CreateKernel_ReLU()
{
	if (!IsActiveLayer())
		return 0;

	int result;
	if (!UseFP16)
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(ReLU), pCmKernel);
	else
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(ReLU_HF), pCmKernel);
	if (result != CM_SUCCESS ) {
		printf("pCmDev->CreateKernel error, %d\n", result);
		return -1;
	}

	if (InputMemType == MEMORY_1D) {
		int BlockWidth_1D = (!UseFP16) ? 64 : 256;		// Map to 8x8 block size for FP32, 16x16 block size for FP16
		ThreadsWidth = (int) ceil((float) oDepth/BlockWidth_1D);
		ThreadsHeight = 1;
	}

	pCmKernel->SetThreadCount( ThreadsWidth * ThreadsHeight );

	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &InputMemType);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &OutputMemType);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);		// ImagesPerRow

	 
	result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth, ThreadsHeight, pThrdSpace);
//	pTGS = NULL;
//	result = pCm->pCmDev->CreateThreadGroupSpace(1, 1, ThreadsWidth, ThreadsHeight, pTGS);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}

///////////////////////////////////////////////////////////////////////////
// 2D to 1D data conversion
int CM_LAYER::CreateKernel_SurfConv()
{
	if (!IsActiveLayer())
		return 0;

	int result;
	if (!UseFP16)
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(SurfConv), pCmKernel);
	else
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(SurfConv_HF), pCmKernel);
	if (result != CM_SUCCESS ) {
		printf("pCmDev->CreateKernel error, %d\n", result);
		return -1;
	}

	// Use input tile info to calculate thread space.
	ThreadsWidth  = ThreadsWidthPerTile * iTileHoriCount;
	ThreadsHeight = ThreadsHeightPerTile * iTileVertCount;

	pCmKernel->SetThreadCount( ThreadsWidth * ThreadsHeight );

	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);		// ImagesPerRow
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestWidth);
		 
	result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth, ThreadsHeight, pThrdSpace);
//	result = pCm->pCmDev->CreateThreadGroupSpace(1, 1, ThreadsWidth, ThreadsHeight, pTGS);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}

///////////////////////////////////////////////////////////////////////////
int CM_LAYER::CreateKernel_SoftMax()
{
	if (!IsActiveLayer())
		return 0;

	int result;
	if (!UseFP16)
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Softmax), pCmKernel);
	else
		result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Softmax_HF), pCmKernel);
	if (result != CM_SUCCESS ) {
		printf("pCmDev->CreateKernel error, %d\n", result);
		return -1;
	}

	int FeaturesPerThread = 8;
	ThreadsWidth = (int) ceil((float) Param.DestDepth / FeaturesPerThread);
	ThreadsHeight = Param.BatchSize;

	pCmKernel->SetThreadCount( ThreadsWidth * ThreadsHeight);	// 4096

	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oDepth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &FeaturesPerThread);

	result = pCm->pCmDev->CreateThreadSpace(ThreadsWidth, ThreadsHeight, pThrdSpace);
//	result = pCm->pCmDev->CreateThreadGroupSpace(1, 1, ThreadsWidth, ThreadsHeight, pTGS);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateThreadSpace error = %d\n", result);
		return -1;
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}

////////// Host code for FP16 //////////

//////////////////////////////////////////////////////////
int CM_LAYER::CreateKernel_Convol_IPT_HF(uint KernelWidth)
{
	if (!IsActiveLayer())
		return 0;

	int result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol3x3_IPT_2D_HF) , pCmKernel);
	printf("Convol3x3_IPT_2D_HF\n");
	if (result != CM_SUCCESS ) {
		printf("CreateKernel error, %d\n", result);
		return -1;
	}

    // Thread space is the same as the output tile dimension. 
	int threadswidth  = oTileHoriCount;  
	int threadsheight = oTileVertCount; 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);

//	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightBiasIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightIndex);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pBiasIndex);

	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.SrcDepth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestDepth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestWidth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestHeight);

	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileWidth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHeight);

	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHoriCount);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileWidth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHeight);

	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.InputBorderWidth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.OutputBorderWidth);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.NumGroups);
	pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.EnableReLU);

	pThrdSpace = NULL;
	pCm->pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);
	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
int CM_LAYER::CreateKernel_Convol_BPT_HF(uint KernelWidth)
{
	if (!IsActiveLayer())
		return 0;

	int result;
	switch (KernelWidth) {
//		case 1: // Kernel Convol1x1_BPT8x8 processes 8 collocated blocks in 8 output tiles. Thread count is reduced by 8 times.
//			result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol1x1_BPT_8x8_MB_HF), pCmKernel);
//			printf("Convol3x3_BPT_1x1_MB_HF\n");
//			break;
		case 3:	// Kernel Convol3x3_BPT8x8 processes 8 collocated blocks in 8 output tiles. Thread count is reduced by 8 times.
//				result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol3x3_BPT8x8_HF), pCmKernel);
//				printf("Convol3x3_BPT_8x8_HF\n");
				result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol3x3_BPT_8x8_MB_HF), pCmKernel);
				printf("Convol3x3_BPT_8x8_MB_HF\n");
				break;
		case 5:	result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol5x5_BPT27x16_HF), pCmKernel);
				printf("Convol5x5_BPT27x16_HF\n");
				break;
		case 11: result = pCm->pCmDev->CreateKernel(pCm->pCmProgram, _NAME(Convol11x11_BPT_HF), pCmKernel);
				printf("Convol11x11_BPT_HF\n");
				break;
		default: 
			printf("Invalid kernel size %d\n", KernelWidth);
			return -1;
	}
	if (result != CM_SUCCESS ) {
		printf("pCmDev->CreateKernel error, %d\n", result);
		return -1;
	}

	pThrdSpace = NULL;
	int ParaIdx = 0;

	if (KernelWidth == 3) {		// for multiple output blocks
	    // Find # of threads based on output frame size in pixel
		pCmKernel->SetThreadCount( ThreadsWidth / MULTIBLOCKS * ThreadsHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);

//		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightBiasIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pBiasIndex);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.SrcDepth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestDepth);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileVertCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileVertCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsWidthPerTile);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsHeightPerTile);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.InputBorderWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.OutputBorderWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.EnableReLU);

		pCm->pCmDev->CreateThreadSpace(ThreadsWidth / MULTIBLOCKS, ThreadsHeight, pThrdSpace);

	} else {			// For the rest of kernels with kernel size = 5, 7, and 11.
	    // Find # of threads based on output frame size in pixel
		pCmKernel->SetThreadCount( ThreadsWidth * ThreadsHeight );

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);

//		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightBiasIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWeightIndex);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pBiasIndex);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.SrcDepth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestDepth);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.DestHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &iTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHoriCount);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &oTileHeight);

		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsWidthPerTile);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadsHeightPerTile);
	
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.InputBorderWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.OutputBorderWidth);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.NumGroups);
		pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &Param.EnableReLU);

		pCm->pCmDev->CreateThreadSpace(ThreadsWidth, ThreadsHeight, pThrdSpace);
	}

	if (MULTI_KERNEL_ENQUEUE)
		pCmKernel->AssociateThreadSpace(pThrdSpace);

	return 0;
}


////////////////////////////////////////////////////////////
// Enqueue one kernel per task.
int CM_LAYER::Enqueue_A_Kernel()
{
	// Input data comes in pWeights buffer.
	// Convert input data in FP32 to tiled format in InputBuffer. 
	// Putting the code here is for reuse for frames in video stream without setup input layer again.
	// pWeights is mapped to the input buffer during setup.
	if (Param.IsInputlayer) {
		if (pWeights == NULL) {
			printf("Invalid input buffer.\n");
			return -1;
		}


		// Convert multiple samples in input batch to tiled images 
		LinearToTiledImage(	pWeights, 
							InputBuffer,
							pitch_inputSurf/sizeof(float), 
							Param.SrcDepth * Param.BatchSize, 
							iTileHoriCount,
							iTileWidth, 
							iTileHeight, 
							Param.InputBorderWidth, 
							Param.SrcWidth, 
							Param.SrcHeight,
							Param.SrcDepth,
							0);
							
printf("Before WriteSurfaceStride\n");
printf("InputBuffer = %x, pitch_inputSurf = %d, size_inputSurf = %d\n", InputBuffer, pitch_inputSurf, size_inputSurf);
	
		pInputSurf->WriteSurfaceStride((unsigned char *) InputBuffer, NULL, pitch_inputSurf);	// Copy input data to video memory

printf("After WriteSurfaceStride\n");

#ifdef _DEBUG
		char fn[128];
		sprintf(fn, "./output/INPUT.%dx%d.bmp", iFrameWidth, iFrameHeight);
		SaveBMPFile(fn, (void *)InputBuffer, pitch_inputSurf, iFrameWidth, iFrameHeight, 0);
#endif
	}

	// Create a task (container) to be put in the task queue
	int result = pCm->pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateTask error %d in layer %d %s.\n", result, this->LayerID, this->Param.LayerType);
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		printf("CmDevice AddKernel error %d in layer %d %s.\n", result, this->LayerID, this->Param.LayerType);
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
/*	if (pTGS != NULL) {
		result = pCm->pCmQueue->EnqueueWithGroup(pKernelArray, pEvent, pTGS);
	} else*/ {
		result = pCm->pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	}
	if (result != CM_SUCCESS ) {
		_FUNC_TRACE
		printf("CmDevice enqueue error %d in layer %d %s\n", result, this->LayerID, this->Param.LayerType);
		return -1;
	}

	// Destroy a task (container) after using it
	pCm->pCmDev->DestroyTask(pKernelArray);

	return 0;
}


