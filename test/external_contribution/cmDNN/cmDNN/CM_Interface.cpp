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
#ifdef WIN32
#include <io.h>
#else
#define _aligned_malloc(x,y) malloc(x)
#endif
//#include "half.h"
#include <cm_rt.h>

//using namespace half_float;

#include "cmDNN.h"

int LayerCount = 0;

extern void LinearToTiledImage(float * pSrc, float * pDst, int DstPitch, int Images, int HoriTiles, 
						int TileWidth, int TileHeight, int BorderWidth, int ImgWidth, int ImgHeight, int ImgDepth, int UseFP16);
extern int SaveBMPFile(char fn[], void * Buffer, int Pitch, int Width, int Height, int UseFP16);

////////////////////////////////////////////////////////////////////
// Input is always processed in FP32.  Output could be in FP32 or FP16.
void CM_LAYER::Config_Input_Layer()
{
	NumInputs = 2;
	iBlockWidth = iBlockHeight = 8;

	CreateInputSurface();	
	CreateOutputSurface();

	InputBuffer2 = (float *) _aligned_malloc(size_inputSurf, 4096);
	if (!InputBuffer2) {
		printf("Failed allocate InputBuffer\n");
		return;
	}
	memset(InputBuffer2, 0, size_inputSurf);

	pBias = (float *) malloc(size_inputSurf);
	if (!pBias) {
		printf("Failed allocate pBias\n");
		return;
	}
	memset(pBias, 0, size_inputSurf);

	// Read cnnAvg data to pBias
	FILE * fh = fopen(Param.BiasFile, "rb");
	if (fh == NULL) {
		_FUNC_TRACE
		printf("Fail to open %s.\n", Param.BiasFile);
		return;
	}
	int OneBatchSize = Param.SrcWidth * Param.SrcHeight * Param.SrcDepth;	// Size for one batch
	float * pCnnAvg = (float *)malloc(OneBatchSize * sizeof(float));
	int rtn = fread((void *)pBias, sizeof(float), OneBatchSize, fh);
	if (rtn != OneBatchSize)
		perror("Error fread");
	fclose(fh);

	// Convert cnnAvg data to tiled images in InputBuffer2.
	// Assume cnnAvg size is one sample size.  Reuse it for multiple samples in a batch.
	LinearToTiledImage(	pBias, 
						InputBuffer2, //DestBufferOneSampleOffset,
						pitch_inputSurf/sizeof(float),		// pitch in elements
						Param.SrcDepth * Param.BatchSize,	// Images
						iTileHoriCount,
						iTileWidth, 
						iTileHeight, 
						Param.InputBorderWidth, 
						Param.SrcWidth, 
						Param.SrcHeight,
						Param.SrcDepth,
						0);

#ifdef _DEBUG
char fn[128];
sprintf(fn, "./output/cnnAvg.%dx%d.bmp", iFrameWidth, iFrameHeight);
SaveBMPFile(fn, (void *) InputBuffer2, pitch_inputSurf, iFrameWidth, iFrameHeight, 0);
#endif

//	pBias = 0;  // Buffers is not allocated in cmDNN.  Reset to 0 to avoid free them in destructor.

	// GPU
	SetupGPUInputSurface();
	SetupGPUOutputSurface();					// 2D
	CreateKernel_InputProc();
}

//////////////////////////////////////////////
void CM_LAYER::Define_Convol_iBlockSize()
{
	if (!UseFP16) {		// FP32
		if (Param.KernelWidth == 1 && Param.KernelStride == 2) {
			iBlockWidth = 16;
			iBlockHeight = 16;
		} else if (Param.KernelWidth == 3) {
			if (Param.SrcWidth <= 8 && Param.SrcHeight <= 8) {
				iBlockWidth = 8;
				iBlockHeight = 8;
			} else if (Param.SrcWidth <= 16 && Param.SrcHeight <= 16) {	
				iBlockWidth = 16;		// IPT for image smaller than 16x16, support 2 groups
				iBlockHeight = 16;
			} else {					// BPT
				iBlockWidth = 8;
				iBlockHeight = 8;
			}
		} else if (Param.KernelWidth == 5) {
			iBlockWidth = 8;
			iBlockHeight = 32;
		} else if (Param.KernelWidth == 11 || Param.KernelWidth == 7) {
			iBlockWidth = 16;
			iBlockHeight = 16;
		} else {
			iBlockWidth = 8;
			iBlockHeight = 8;
		}
	} else {	// FP16
		if (Param.KernelWidth == 1 && Param.KernelStride == 2) {
			iBlockWidth = 16;
			iBlockHeight = 16;
		} else if (Param.KernelWidth == 3) {
			if (Param.SrcWidth <= 8 && Param.SrcHeight <= 8) {
				iBlockWidth = 8;
				iBlockHeight = 8;
		} else if (Param.SrcWidth <= 16 && Param.SrcHeight <= 16) {
				iBlockWidth = 16;		// IPT for small image 16x16 or smaller, support 2 groups
				iBlockHeight = 16;
			} else {					// BPT
				iBlockWidth = 16; //8;
				iBlockHeight = 8;
			}
		} else if (Param.KernelWidth == 5) {
			iBlockWidth = 16;
			iBlockHeight = 32;
		} else if (Param.KernelWidth == 11) {
			iBlockWidth = 16;
			iBlockHeight = 16;
		} else {
			iBlockWidth = 16;
			iBlockHeight = 16;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void CM_LAYER::Config_Convol_Layer()
{
	WeightSize = Param.KernelWidth * Param.KernelHeight;

	TotalWeightSize = WeightSize * Param.SrcDepth * Param.DestDepth / Param.NumGroups;				// Total weight size in float32
	TotalWeightSize = (int)ceil((float)TotalWeightSize / 4) * 4;									// Multiple of OWORD (4 dword)

	TotalBiasSize = Param.DestDepth;																// Total bias size in float32
	TotalBiasSize = (int)ceil((float)TotalBiasSize / 4) * 4;										// Multiple of OWORD (4 dword)

	BiasSize = 1;
	WeightBiasSize = WeightSize + BiasSize;
	TotalWeightBiasSize = WeightBiasSize * Param.SrcDepth * Param.DestDepth / Param.NumGroups;		// Total kernel + bias size  
	TotalWeightBiasSize = (int)ceil((float)TotalWeightBiasSize / 4) * 4;							// Multiple of OWORD (4 dword)

	Define_Convol_iBlockSize();

	CreateInputSurface();				// 2D surface
	CreateOutputSurface();				// 2D surface
	ReadWeightsBias(Param.WeightFile, Param.BiasFile);	
	// GPU
	SetupGPUInputSurface();

//	SetupGPUWeightBiasBuffer();
	SetupGPUWeightBuffer();
	SetupGPUBiasBuffer();

	SetupGPUOutputSurface();

	if (!UseFP16) {
		if (Param.KernelWidth == 3 && iBlockWidth == 16 && iBlockHeight == 16)
		    CreateKernel_Convol_IPT(Param.KernelWidth);
		else
			CreateKernel_Convol_BPT(Param.KernelWidth);
	} else {
		if (Param.KernelWidth == 3 && iBlockWidth == 16 && iBlockHeight == 16)
			CreateKernel_Convol_IPT_HF(Param.KernelWidth);
		else
			CreateKernel_Convol_BPT_HF(Param.KernelWidth);
	}
}


////////////////////////////////////////////////////////////
void CM_LAYER::Config_LRN_Layer()
{
	if (!UseFP16) {		// FP32
		iBlockWidth = 8;
		iBlockHeight = 8;
	} else {
		iBlockWidth = 16;
		iBlockHeight = 16;
	}
	CreateInputSurface();
	CreateOutputSurface();
	ReadImage2D();
	// GPU
	SetupGPUInputSurface();
	SetupGPUOutputSurface();
	CreateKernel_LRN();
}

////////////////////////////////////////////////////////////
void CM_LAYER::Config_MaxPool_Layer()
{
	if (!UseFP16) {		// FP32
		iBlockWidth = 8;
		iBlockHeight = 8;
	} else {			// FP16
		iBlockWidth = 16;
		iBlockHeight = 16;
	}
	CreateInputSurface();
	CreateOutputSurface();
	ReadImage2D();
	// GPU
	SetupGPUInputSurface();
	SetupGPUOutputSurface();
	CreateKernel_MaxPool();
}

////////////////////////////////////////////////////////////
void CM_LAYER::Config_AvgPool_Layer()
{
	if (!UseFP16) {		// FP32
		iBlockWidth = 8;
		iBlockHeight = 8;
	}
	else {			// FP16
		iBlockWidth = 16;
		iBlockHeight = 16;
	}
	CreateInputSurface();
	
	if (Param.DestHeight > 1)
		CreateOutputSurface();	// 	MEMORY_2D
	else
		CreateOutputBuffer();	// MEMORY_1D

	ReadImage2D();
	// GPU
	SetupGPUInputSurface();

	if (Param.DestHeight > 1)
		SetupGPUOutputSurface();
	else
		SetupGPUOutputBuffer();
	CreateKernel_AvgPool();
}


//////////////////////////////////////////////////////////
void CM_LAYER::Config_FC_Layer()
{
	if (!UseFP16) {		// FP32
		iBlockWidth = 8;
		iBlockHeight = 8;
	} else {
		iBlockWidth = 16;
		iBlockHeight = 16;
	}
	CreateInputBuffer();				// 1D
	CreateOutputBuffer();						// 1D
	ReadImage1D();					// 1D
	ReadWeightsBias(Param.WeightFile, Param.BiasFile);	
	// GPU
	SetupGPUInputBuffer();
	SetupGPUWeightSurface();					// 2D weight surface
	SetupGPUBiasBuffer();
	SetupGPUOutputBuffer();
	CreateKernel_FC_NPatch();
}

////////////////////////////////////////////////////////////////
void CM_LAYER::Config_ReLU_Layer()
{
	if (!UseFP16) {		// FP32
		iBlockWidth = 8;
		iBlockHeight = 8;
	} else {
		iBlockWidth = 16;
		iBlockHeight = 16;
	}

	// Create ReLU layer based on prev layer's ouptut type.
	// Surface type is not changed in ReLU layer.
	if (pPrevLayer->OutputMemType == MEMORY_2D) {
		CreateInputSurface();
		CreateOutputSurface();
		ReadImage2D();
		// GPU
		SetupGPUInputSurface();
		SetupGPUOutputSurface();
	} else {	// 1D buffer
		CreateInputBuffer();
		CreateOutputBuffer();
		ReadImage2D();
		// GPU
		SetupGPUInputBuffer();
		SetupGPUOutputBuffer();
	}
	CreateKernel_ReLU();
}

////////////////////////////////////////////////////////////////
void CM_LAYER::Config_SurfConv_Layer()
{
	if (!UseFP16) {		// FP32
		iBlockWidth = 8;
		iBlockHeight = 8;
	} else {
		iBlockWidth = 16;
		iBlockHeight = 16;
	}

	if (Param.SrcHeight > 1)
		CreateInputSurface();	// MEMORY_2D
	else
		CreateInputBuffer();	// MEMORY_1D

	if (Param.DestHeight > 1) 
		CreateOutputSurface();			// MEMORY_2D
	else
		CreateOutputBuffer();			// MEMORY_1D

	ReadImage2D();
	
	// GPU
	if (Param.SrcHeight > 1)
		SetupGPUInputSurface();
	else
		SetupGPUInputBuffer();

	if (Param.DestHeight > 1) 
		SetupGPUOutputSurface();
	else 
		SetupGPUOutputBuffer();

	CreateKernel_SurfConv();
}

///////////////////////////////////////////////////////////
void CM_LAYER::Config_SoftMax_Layer()
{
	if (!UseFP16) {		// FP32
		iBlockWidth = 8;
		iBlockHeight = 8;
	} else {
		iBlockWidth = 16;
		iBlockHeight = 16;
	}

	CreateInputBuffer();
	CreateOutputBuffer();
	ReadImage1D();
	// GPU
	SetupGPUInputBuffer();
	SetupGPUOutputBuffer();
	CreateKernel_SoftMax();
}

