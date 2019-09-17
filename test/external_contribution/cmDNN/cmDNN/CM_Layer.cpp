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
#include "cmtest.h"
#include <cm_rt.h>

//using namespace half_float;

#ifdef _DEBUG
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

#include "cmDNN.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Convert linear input buffer to tiles images.
//
// Images are stored in input buffer sequentially.  There is no border. Input image size is SrcWidth and SrcHeight.
// Output surface is described by DstPitch, Images, Horitiles, TileWidth, TileHeight, and BorderWidth 
//
void LinearToTiledImage(float * pSrc, float * pDst, int DstPitch, int Images, int HoriTiles, 
						int TileWidth, int TileHeight, int BorderWidth, int ImgWidth, int ImgHeight, int ImgDepth, int UseFP16)
{
	int SrcOffset = 0;
	for (int SrcIdx = 0; SrcIdx < Images; SrcIdx++) {

		// Wrap around for input data reuse.
		SrcOffset = SrcOffset % (ImgWidth * ImgHeight * ImgDepth);

		// Get src tile origin indexd by SrcIdx
		int TileOffsetX = SrcIdx % HoriTiles;	// Tile origin x in # of tiles
		int TileOffsetY = SrcIdx / HoriTiles;	// Tile origin y in # of tiles
		
		// Dest tile origin in linear address
		float * pBase = pDst + DstPitch * (TileHeight * TileOffsetY) + (TileWidth * TileOffsetX);
//		half * pBase16 = ((half*) pDst) + DstPitch * (TileHeight * TileOffsetY) + (TileWidth * TileOffsetX);

		for (int j = 0; j < ImgHeight; j++) {
			if (!UseFP16) {
				memcpy((pBase + DstPitch * (j + BorderWidth) + BorderWidth), (pSrc + SrcOffset), ImgWidth * sizeof(float));	// Copy a row
				SrcOffset += ImgWidth;
			} else {
//				for (int i = 0; i < ImgWidth; i++) {
//					*(pBase16 + DstPitch * (j + BorderWidth) + i + BorderWidth) = *(pSrc + SrcOffset++);
//				}
			}
		} 
	}
}

#ifdef _DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////
// Src order: Matrix by matrix
void Matrices2File(char * filename, float * ptr, int MatrixWidth, int MatrixHeight, int NumMatrix, 
				   int BorderWidth, int oTileWidth, int oTileHeight, int fp16)
{

FILE * fh;
int offset = 0;
float Zero = 0.0f;
int DestSize = oTileWidth * oTileHeight;

    fh = fopen(filename, "w");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

	char * pStr = (char *) malloc(16 * DestSize + 1024);
	memset(pStr, 0, 16 * DestSize + 1024);
//	half_float::half *ptr1 = (half_float::half *)ptr;

    for (int k = 0; k < NumMatrix; k++) {
		// Header
		fprintf(fh, "%d:  ", k);
		for (int i = 0; i < oTileWidth; i++) 
			fprintf(fh, "%-9d", i);
		fprintf(fh, "\n");

		for (int j = 0; j < oTileHeight; j++) {
			for (int i = 0; i < oTileWidth; i++) {

				char Buf[1024];
				if (j < BorderWidth || j >= MatrixHeight + BorderWidth || i < BorderWidth || i >= MatrixWidth + BorderWidth) {
					sprintf(Buf, "%8.3f ", Zero);
				} else {
					if (!fp16)
						sprintf(Buf, "%8.3f ", *(ptr + offset++));
//					else 
//						sprintf(Buf, "%8.3f ", (float) *(ptr1 + offset++));

					if (offset > MatrixWidth * MatrixHeight * NumMatrix) {
						printf("Invalid source size in Matrices2File().\n");
						return;
					}
				}
				strcat(pStr, Buf);
			}
			strcat(pStr, "\n");
		}
		fprintf(fh, "%s", pStr);
		*pStr = 0;
    }

	free(pStr);
    fclose(fh);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Src order: First line of all matrices in the 1st row, then the 2nd line, ...
void TiledMatrices2File(char * filename, float * ptr, int Pitch, int TileWidth, int TileHeight, int TileHoriCount, int NumMatrix, int fp16)
{
FILE * fh;
int offset = 0;

    fh = fopen(filename, "w");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    for (int k = 0; k < NumMatrix; k++) {
		// Header
		fprintf(fh, "%d:  ", k);
		for (int i = 0; i < TileWidth; i++) 
			fprintf(fh, "%-9d", i);
		fprintf(fh, "\n");

		// Get src tile origin indexd by k
		int TileOffsetX = k%TileHoriCount;
		int TileOffsetY = k/TileHoriCount;
		float * pBase = ptr + Pitch * TileHeight * TileOffsetY + TileWidth * TileOffsetX;
//		half * pBase16 = ((half *) ptr) + Pitch * TileHeight * TileOffsetY + TileWidth * TileOffsetX;
		float fVal;

		// Write a tile
		for (int j = 0; j < TileHeight; j++) {
			for (int i = 0; i < TileWidth; i++) {
				if (!fp16)
				    fprintf(fh, "%8.3f ", *(pBase + Pitch * j + i));
				else {
//					fVal = (float)*(pBase16 + Pitch * j + i);
//					fprintf(fh, "%8.3f ", (float)*(pBase16 + Pitch * j + i));
				}
			}
			fprintf(fh, "\n");
		}
    }

    fclose(fh);
}

/////////////////////////////////////////////////////////////////////////////////////////
int SaveBMPFile(char fn[], void * Buffer, int Pitch, int Width, int Height, int UseFP16)
{
	float * pFP32 = NULL;
//	half * pFP16 = NULL;
	int FP32Pitch;
	int FP32Size;

	if (!UseFP16) {		// FP32
		pFP32 = (float *) Buffer;
		FP32Pitch = Pitch;
	} else {			// FP16
/*	
	pFP16 = (half *) Buffer;
		FP32Pitch = Pitch * sizeof(half);	// In bytes
		FP32Size = FP32Pitch * Height;	// In bytes

		// Convert FP16 to FP32
		pFP32 = (float *) malloc(FP32Size);
		memset(pFP32, 0, FP32Size);
		for (int i = 0; i < Pitch/sizeof(half) * Height; i++)
			pFP32[i] = pFP16[i];
*/
	}

	CvSize  cvSize;
	if (Height > 1) {  // for 2D surface
		cvSize.width = Width;
		cvSize.height = Height;
	} else {
		//printf("1D data is not saved in bmp file.\n");
		return -1;
	}

	// cast to a uchar pointer
	unsigned char * ptr = (unsigned char *) pFP32;
	IplImage * bgr_GPU_Frame = cvCreateImage(cvSize, IPL_DEPTH_32F, 1);

    for (int j = 0; j < Height; j++) {
	    for (int i = 0; i < Width * sizeof(float); i++) {
			*(bgr_GPU_Frame->imageData + bgr_GPU_Frame->widthStep*j + i) = *(ptr + FP32Pitch * j + i);	// Copy byte by byte
	    }
    }

	cvSaveImage(fn, bgr_GPU_Frame);
	cvReleaseImage(&bgr_GPU_Frame);

	if (UseFP16 && pFP32 != 0)
		free(pFP32);
	return 0;
}

////////////////////////////////////////////////////////////////////////////
int SaveColorBMPFile(char fn[], uchar * Buffer, int Pitch, int Width, int Height)
{
	CvSize  cvSize;

	if (Height > 1) {  // for 2D surface
		cvSize.width = Width;
		cvSize.height = Height;
	} else {
		//printf("1D data is not saved in bmp file.\n");
		return -1;
	}

	IplImage * bgr_GPU_Frame = cvCreateImage(cvSize, IPL_DEPTH_8U, 3);

	int k = 0;
    for (int j = 0; j < Width*Height*3; j++) {
	    *(bgr_GPU_Frame->imageData + k++) = *(Buffer + j);
    }

	cvSaveImage(fn, bgr_GPU_Frame);
	cvReleaseImage(&bgr_GPU_Frame);
	return 0;
}

#endif

///////////////////////////////////////////////////////
CM_LAYER::CM_LAYER()
{
	Layer_Param * pParam = &Param;
	pParam = 0;
	LayerID = 0;

	NumInputs = 1;
	pPrevLayer = NULL;
	pPrevLayer2 = NULL;

	InputBuffer = 0;
	InputBuffer2 = 0;
	InputBufferIsMalloced = 0;

	OutputBuffer = 0;
	OutputBufferIsMalloced = 0;

	pWeights = 0;
	pBias = 0;
	pWeightsBias = 0;

	ThreadsWidth = 1; 
	ThreadsHeight = 1; 
	ThreadsWidthPerTile = 1;
	ThreadsHeightPerTile = 1;

	InputMemType = MEMORY_2D;
	pInputSurf = NULL;
	pInputBuf = NULL;
	pInputIndex = NULL;

	pInputSurf2 = NULL;
	pInputBuf2 = NULL;
	pInputIndex2 = NULL;

	OutputMemType = MEMORY_2D;
	pOutputSurf = NULL;
	pOutputBuf = NULL;
	pOutputIndex = NULL;

	pInputSurfUP = NULL;
	pInputSurfUP = NULL;
	pOutputSurfUP = NULL;
	pOutputBufUP = NULL;

	pCmKernel = 0;
	pThrdSpace = 0;
	pTGS = 0;
	pKernelArray = 0;
	pEvent = 0;

	fKernelTimeMS = 0.0f;

	pitch_inputSurf = 0;
	size_inputSurf = 0;
	pitch_outputSurf = 0;
	size_outputSurf = 0;

	UseFP16 = 0;				// Default to FP32.
	qFCWt = 0;					// Default to non-quantized weight data in FP32.
};

///////////////////////////////////////////////////////
CM_LAYER::~CM_LAYER()
{
	if (pTGS) {
		pCm->pCmDev->DestroyThreadGroupSpace(pTGS);
		pTGS = 0;
	}
	if (pThrdSpace) {
		pCm->pCmDev->DestroyThreadSpace(pThrdSpace);
		pThrdSpace = 0;
	}
	if (pCmKernel) {
		pCm->pCmDev->DestroyKernel(pCmKernel);
		pCmKernel = 0;
	}
    if (pKernelArray) {
		pCm->pCmDev->DestroyTask(pKernelArray);
		pKernelArray = 0;
	}
    if (InputBuffer && InputBufferIsMalloced) {
		AlignedFree(InputBuffer);
		InputBuffer = 0;
		InputBufferIsMalloced = 0;
	}
    if (InputBuffer2) {
		AlignedFree(InputBuffer2);
		InputBuffer2 = 0;
	}
    if (OutputBuffer && OutputBufferIsMalloced) {
		AlignedFree(OutputBuffer);
		OutputBuffer = 0;
		OutputBufferIsMalloced = 0;
	}
    if (pBias) {
		free(pBias);
		pBias = 0;
	}
    if (pWeightsBias) {
		free(pWeightsBias);
		pWeightsBias = 0;
	}
	if (pWeights && strcmp(Param.LayerType, "Input")) {
		free(pWeights);
		pWeights = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Find image grid dimension for ImageCnt.  
// Both dimensions are whole numbers with vertical dimension possibly round up.  
void CM_LAYER::FindImageTile(int ImageCnt, int BatchSize, uint & HoriCount, uint & VertCount)
{
#ifdef NOT_USED
	if (ThreadsWidthPerTile * ImageCnt < 2048) {
//	if (ImageCnt <= Param.SrcDepth) {
		// Store samll image set in 1 row.
		HoriCount = ImageCnt;
		VertCount = 1;
	} else {
		// The max surface width is 16384 pixels.  Wide enough to store ImageCnt in 2 rows. 
		HoriCount = ImageCnt / TILE_ROWS;
		VertCount = TILE_ROWS;
	}
	VertCount *= BatchSize;
	return;
#endif

	int TotalImages = ImageCnt * BatchSize;

	int Seed = (int) (sqrt(TotalImages) + 0.5);
	int S1, S2;
	int Cx[2] = { 0, 0 };

	// Search tile width and height from sqrt of src images.
	// S1 for search down, S2 for search up.
	for (S1 = S2 = Seed; S1 >= Seed/2; S1--, S2++) {

		// Find exact fit with S2 as tile width.
		if (TotalImages % S2 == 0) {
			HoriCount = S2;
			VertCount = TotalImages / S2;
			if (HoriCount < VertCount) {	// Swap if tall layout
				HoriCount = VertCount;
				VertCount = S2;
			}
			return;
		}

		// Find exact fit with S1 as tile width.
		if (TotalImages % S1 == 0) {
			HoriCount = S1;
			VertCount = TotalImages / S1;
			if (HoriCount < VertCount) {	// Swap
				HoriCount = VertCount;
				VertCount = S1;
			}
 			return;
		}

		// Find a width who has the most tiles in the last row.  Save width in Cx[0], and its remainder in Cx[1].
		// Check S1 and the corresonding remainder
		if (TotalImages % S1 > Cx[1]) {
			Cx[0] = S1;
			Cx[1] = TotalImages % S1;
		}
		// Save S2 and the corresonding remainder
		if (TotalImages % S2 > Cx[1]) {
			Cx[0] = S2;
			Cx[1] = TotalImages % S2;
		}

	}

	HoriCount = Cx[0];
	VertCount = (uint) ceil( (float)TotalImages / Cx[0]);

	// Make sure tile layout has more tiles horizontally if not squared.
	if (HoriCount < VertCount) {	// Swap
		HoriCount = VertCount;
		VertCount = Cx[0];
	}

}

///////////////////////////////////////////
int inline CM_LAYER::IsActiveLayer()
{
	return(Param.IsActive);
}

////////////////////////////////////////////
float * CM_LAYER::GetInputBuffer()
{
	return(InputBuffer);
}

////////////////////////////////////////////
void CM_LAYER::PrintLayerInfo(int CurLayer)
{
	printf("Surface info: Frame Size    Tiles\tTile Size\tBlock Size\tThreads/Tile\tThread Space\n");

	if (IsActiveLayer()) {
		printf("Layer %d In:  ", CurLayer); 
		printf("%4dx%-4d\t  ", iFrameWidth, iFrameHeight);
		printf("%4dx%-4d\t", iTileHoriCount, iTileVertCount);
		printf("%4dx%-4d\t", iTileWidth, iTileHeight);
		printf("%4dx%-4d\n", iBlockWidth, iBlockHeight);

		printf("        Out:  "); 
		printf("%4dx%-4d\t  ", oFrameWidth, oFrameHeight);
		printf("%4dx%-4d\t", oTileHoriCount, oTileVertCount);
		printf("%4dx%-4d\t", oTileWidth, oTileHeight);
		printf("%4dx%-4d\t", oBlockWidth, oBlockHeight);
		printf("%4dx%-4d\t", ThreadsWidthPerTile, ThreadsHeightPerTile);
		printf("%4dx%-4d\n", ThreadsWidth, ThreadsHeight);
	}

	printf("\n");
}

////////////////////////////////////////////////////////
int CM_LAYER::CreateOutputSurface()
{
	CM_SURFACE_FORMAT fmt = UseFP16 ? CM_SURFACE_FORMAT_V8U8 : CM_SURFACE_FORMAT_R32F;
	FindImageTile(Param.DestDepth, Param.BatchSize, oTileHoriCount, oTileVertCount);

	oBlockWidth = iBlockWidth / Param.KernelStride;
	oBlockHeight = iBlockHeight / Param.KernelStride;

	// DestWidth is precalculated with SrcWidth, KernelWidth InputBorder width and KernelStride.

	// Round up tilesize to multiple of oblock size, for thread size per tile and thread space.
	if (Param.DestWidth < 8) {
		oTileWidth = (int)ceil((float)(Param.DestWidth + Param.OutputBorderWidth) / oBlockWidth) * oBlockWidth;
		oTileHeight = (int)ceil((float)(Param.DestHeight + Param.OutputBorderWidth) / oBlockHeight) * oBlockHeight;
	} else {
		oTileWidth = (int)ceil((float)(Param.DestWidth + 2 * Param.OutputBorderWidth) / oBlockWidth) * oBlockWidth;
		oTileHeight = (int)ceil((float)(Param.DestHeight + 2 * Param.OutputBorderWidth) / oBlockHeight) * oBlockHeight;
	}

	ThreadsWidthPerTile = oTileWidth / oBlockWidth;
	ThreadsHeightPerTile = oTileHeight / oBlockHeight;

	ThreadsWidth = ThreadsWidthPerTile * oTileHoriCount;
	ThreadsHeight = ThreadsHeightPerTile * oTileVertCount;

	// Round up tile size and frame size to multiple of 8
	if (!strcmp(Param.LayerType, "Convol") && Param.KernelWidth == 11) {
		oTileWidth = (int)ceil((float)oTileWidth / 8) * 8;
		oTileHeight = (int)ceil((float)oTileHeight / 8) * 8;
	}

	oFrameWidth = oTileWidth * oTileHoriCount;
	oFrameHeight = oTileHeight * oTileVertCount;

	pCm->pCmDev->GetSurface2DInfo(oFrameWidth, oFrameHeight, fmt, pitch_outputSurf, size_outputSurf);
		
	OutputBuffer = (float *) _aligned_malloc(size_outputSurf, 4096);	
	if (!OutputBuffer) {
		printf("Failed allocate OutputBuffer[0]\n");
		exit(1);
	}
	memset(OutputBuffer, 0, size_outputSurf);
	OutputBufferIsMalloced = 1;
	return 0;	
}

////////////////////////////////////////////////////////
int CM_LAYER::CreateInputSurface()
{
    CM_SURFACE_FORMAT fmt = UseFP16 ? CM_SURFACE_FORMAT_V8U8 : CM_SURFACE_FORMAT_R32F;
	// Input surface is always in FP32
	if (!strcmp(Param.LayerType, "Input"))
		fmt = CM_SURFACE_FORMAT_R32F;

	FindImageTile(Param.SrcDepth, Param.BatchSize, iTileHoriCount, iTileVertCount);

	if (pPrevLayer == NULL || !pPrevLayer->IsActiveLayer()) {
		// Round up image size to multiple of iBlock.
		iTileWidth = (int) ceil((float)(Param.SrcWidth + 2*Param.InputBorderWidth) / iBlockWidth) * iBlockWidth;
		iTileHeight = (int)ceil((float)(Param.SrcHeight + 2*Param.InputBorderWidth) / iBlockHeight) * iBlockHeight;

		iFrameWidth = iTileWidth * iTileHoriCount;
		iFrameHeight = iTileHeight * iTileVertCount;

		pCm->pCmDev->GetSurface2DInfo(iFrameWidth, iFrameHeight, fmt, pitch_inputSurf, size_inputSurf);
		InputBuffer = (float *) _aligned_malloc(size_inputSurf, 4096);
		if (!InputBuffer) {
			printf("Failed allocate InputBuffer\n");
			return -1;
		}
		memset(InputBuffer, 0, size_inputSurf);
		InputBufferIsMalloced = 1;
	} else {
		// Use prev layer output buffer parameters
		iTileWidth = pPrevLayer->oTileWidth;
		iTileHeight = pPrevLayer->oTileHeight;

		iFrameWidth = pPrevLayer->oFrameWidth;
		iFrameHeight = pPrevLayer->oFrameHeight;

		pitch_inputSurf = pPrevLayer->pitch_outputSurf;
		size_inputSurf = pPrevLayer->size_outputSurf;

		InputBuffer = pPrevLayer->OutputBuffer;
		InputBufferIsMalloced = 0;
	}

	return 0;
}

///////////////////////////////////////////////////////////
// Tile in buffer has no border and no round up dimension.  
// Tile width = DestWidth * DestHeight.
// Tile height = 1.
int CM_LAYER::CreateOutputBuffer()
{
	oDepth = (int) ceil((float) Param.DestDepth / 32) * 32;
	oTileHoriCount = oDepth * Param.BatchSize;
	oTileVertCount = 1;

	oBlockWidth = Param.DestWidth * Param.DestHeight;
	oBlockHeight = 1;

	oTileWidth = oBlockWidth;
	oTileHeight = oBlockHeight;

	oFrameWidth = oTileWidth * oTileHoriCount;
	oFrameHeight = oTileHeight * oTileVertCount;

	ThreadsWidthPerTile = oTileWidth / oBlockWidth;		// = 1
	ThreadsHeightPerTile = oTileHeight / oBlockHeight;	// = 1

	ThreadsWidth = ThreadsWidthPerTile * oTileHoriCount;
	ThreadsHeight = ThreadsHeightPerTile * oTileVertCount;

    if (!UseFP16) {
	    pitch_outputSurf = size_outputSurf = oBlockWidth * oTileHoriCount * sizeof(float);
    } else {
//	    pitch_outputSurf = size_outputSurf = oBlockWidth * oTileHoriCount * sizeof(half);
    }
	OutputBuffer = (float *) _aligned_malloc(size_outputSurf, 4096);	
	if (!OutputBuffer) {
		printf("Failed allocate OutputBuffer[0]\n");
		exit(1);
	}
	memset(OutputBuffer, 0, size_outputSurf);
	OutputBufferIsMalloced = 1;
	
	return 0;
}

/////////////////////////////////////////////////////
int CM_LAYER::CreateInputBuffer()
{
	iDepth = Param.SrcDepth;
	iTileHoriCount = iDepth * Param.BatchSize;
	iTileVertCount = 1;

	if (pPrevLayer == NULL || !pPrevLayer->IsActiveLayer()) {

		iFrameWidth = (Param.SrcWidth + Param.InputBorderWidth) * iTileHoriCount + Param.InputBorderWidth;
		iFrameHeight = (Param.SrcHeight + Param.InputBorderWidth) * iTileVertCount + Param.InputBorderWidth;

		iTileWidth = iFrameWidth / iTileHoriCount;
		iTileHeight = iFrameHeight / iTileVertCount;

		if (!UseFP16) {
		    pitch_inputSurf = size_inputSurf = iFrameWidth * sizeof(float);
		} else {
//		    pitch_inputSurf = size_inputSurf = iFrameWidth * sizeof(half);
		}
		InputBuffer = (float *) _aligned_malloc(size_inputSurf, 4096);
		if (!InputBuffer) {
			printf("Failed allocate InputBuffer\n");
			exit(1);
		}
		memset(InputBuffer, 0, size_inputSurf);
		InputBufferIsMalloced = 1;
	} else {
		// Use prev layer output buffer parameters
		iTileWidth = pPrevLayer->oTileWidth;
		iTileHeight = pPrevLayer->oTileHeight;

		iFrameWidth = pPrevLayer->oFrameWidth;
		iFrameHeight = pPrevLayer->oFrameHeight;

		pitch_inputSurf = pPrevLayer->pitch_outputSurf;
		size_inputSurf = pPrevLayer->size_outputSurf;

		InputBuffer = pPrevLayer->OutputBuffer;
		InputBufferIsMalloced = 0;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////
// This is for individual layer testing.
int CM_LAYER::ReadImage2D()
{
	// If prev layer is active, InputBuffer has the prev layer's OutputBuffer content. 
	// No need to read image from file.
	if (pPrevLayer != NULL && pPrevLayer->IsActiveLayer())
		return 0;

	// Read input layer input if the previous layer does not exist.
	FILE * fh = fopen(Param.InputFile, "rb");
	if (fh == NULL) {
		_FUNC_TRACE
		printf("Fail to open %s.\n", Param.InputFile);
		return -1;
	}

	int OneBatchSize = Param.SrcWidth * Param.SrcHeight * Param.SrcDepth;	// Size for one batch 
	int AllBatchSize = OneBatchSize * Param.BatchSize;						// Size for all batches

	float * pSrc;

	// In 50-batch test case,  the input data is replicated 50 times. 
	// The intermediate image has one batch data only.  
	// It needs to be replicated 50 times to fill for multi-batch buffer.
	{
		pSrc = (float *) malloc(AllBatchSize * sizeof(float));
		int rtn = fread((void *) pSrc, sizeof(float), OneBatchSize, fh);
		if (rtn != OneBatchSize)
			perror ("Error fread");
		fclose(fh);

		// Replicate
		int idx = 1;
		while (idx < Param.BatchSize) {
			memcpy(&pSrc[OneBatchSize*idx], pSrc, OneBatchSize * sizeof(float));
			idx++;
		}
	}

	ConvertInputToTiledImage(pSrc, InputBuffer);
	free(pSrc);

	return 0;
}

/////////////////////////////////////////////////
int CM_LAYER::ReadImage1D()
{
	// If prev layer is active, InputBuffer has the prev layer's OutputBuffer content. 
	// No need to read image from file.
	if (pPrevLayer != NULL && pPrevLayer->IsActiveLayer())
		return 0;

	FILE * fh = fopen(Param.InputFile, "rb");
	if (fh == NULL) {
		_FUNC_TRACE
		printf("Fail to open %s.\n", Param.InputFile);
		return -1;
	}

	int InputSize = Param.SrcWidth * Param.SrcHeight * Param.SrcDepth;	// Size for one batch
	int OneBatchSize = Param.SrcWidth * Param.SrcHeight * iDepth;

	float * pSrc = (float *) malloc(InputSize * sizeof(float));
	int rtn = fread((void *) pSrc, sizeof(float), InputSize, fh);
	if (rtn != InputSize)
	    perror ("Error fread");
	fclose(fh);

	// Replicate
	int idx = 1;
	while (idx < Param.BatchSize) {
		memcpy(&pSrc[OneBatchSize*idx], pSrc, OneBatchSize * sizeof(float));
		idx++;
	}

	ConvertInputToLinearImage(pSrc, InputBuffer);
	free(pSrc);

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Convert from linear data to tiled format add borders.
// This is store multiple images in tiled format for 2D images.
void CM_LAYER::ConvertInputToTiledImage(float * pSrc, float * pInputBuffer)
{
	int SrcOffset = 0;
	for (int SrcIdx = 0; SrcIdx < Param.SrcDepth * Param.BatchSize; SrcIdx++) {
		// Get src tile origin indexd by SrcIdx
		int TileOffsetX = SrcIdx%iTileHoriCount;
		int TileOffsetY = SrcIdx/iTileHoriCount;
		float * pBase = pInputBuffer + pitch_inputSurf/sizeof(float) * iTileHeight * TileOffsetY + iTileWidth * TileOffsetX;
//		half * pBase16 = ((half*) pInputBuffer) + pitch_inputSurf/sizeof(half) * iTileHeight * TileOffsetY + iTileWidth * TileOffsetX;

		int jj, ii;
		for (int j = 0; j < Param.SrcHeight; j++) {
			if (!UseFP16) {
				jj = j + Param.InputBorderWidth;
				ii = 0 + Param.InputBorderWidth;
				memcpy((pBase + pitch_inputSurf/sizeof(float)*jj + ii), (pSrc + SrcOffset), Param.SrcWidth * sizeof(float));	// Copy a row
				SrcOffset += Param.SrcWidth;
			} else {
//				jj = j + Param.InputBorderWidth;
//				for (int i = 0; i < Param.SrcWidth; i++) {
//					ii = i + 2*Param.InputBorderWidth;
//					*(pBase16 + pitch_inputSurf/sizeof(half)*jj + ii) = *(pSrc + SrcOffset++);
//				}
			}
		} 
	}
}

////////////////////////////////////////////////////////////////////
// Convert from linear data to linera format add borders.
// This is store multiple images in linear format for 1D buffer.
void CM_LAYER::ConvertInputToLinearImage(float * pSrc, float * pDst)
{
	// Store multiple images in linear format for 2D images
	int SrcOffset = 0;
	for (int SrcIdx = 0; SrcIdx < Param.SrcDepth * Param.BatchSize; SrcIdx++) {

		float * pBase = pDst + iTileWidth * SrcIdx;
//		half * pBase16 = ((half*) pDst) + iTileWidth * SrcIdx;

		for (int j = 0; j < Param.SrcHeight; j++) {
			int jj = j + Param.OutputBorderWidth;
			if (!UseFP16) {
				int ii = 0 + Param.OutputBorderWidth;
				memcpy((pBase + pitch_outputSurf/sizeof(float)*jj + ii), (pSrc + SrcOffset), Param.SrcWidth * sizeof(float));	// Copy a row
				SrcOffset += Param.SrcWidth;
			} else {
//				for (int i = 0; i < Param.SrcWidth; i++) {
//					int ii = i + Param.OutputBorderWidth;
//					*(pBase16 + pitch_outputSurf/sizeof(half)*jj + ii) = *(pSrc + SrcOffset++);
//				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Copying weights and bias to a 1D buffer. 
// Each set space = kernel size + bias size.
int CM_LAYER::ReadWeightsBias(const char * wfile, const char* bfile)
{
	if (Param.KernelWidth == 0)
		return 0;

//	char fn[256];
	int rtn;

	// Allocate weight buffer
	WeightSize = Param.KernelWidth * Param.KernelHeight;
	TotalWeightSize = WeightSize * Param.SrcDepth * Param.DestDepth / Param.NumGroups;			// Total kernel size in float32
	TotalWeightSize = (int) ceil((float) TotalWeightSize / 4) * 4;	// Multiple of OWORD (4 dword)
	if (TotalWeightSize > 0) {
		pWeights = (float *) malloc(TotalWeightSize * sizeof(float));
		if (!pWeights) {
			printf("Failed allocate pWeights\n");
			exit(1);
		}
		memset(pWeights, 0, TotalWeightSize * sizeof(float));
	}

	// Allocate bias buffer
	BiasSize = 1;
	TotalBiasSize = Param.DestDepth;	// in float
	TotalBiasSize = (int) ceil((float) TotalBiasSize / 4) * 4;				// Multiple of OWORD (4 dword)
	if (TotalBiasSize > 0) {
		pBias = (float *) malloc(TotalBiasSize * sizeof(float));
		if (!pBias) {
			printf("Failed allocate pBias\n");
			exit(1);
		}
		memset(pBias, 0, TotalBiasSize * sizeof(float));
	}

	// Read weights
	FILE * fh = fopen(wfile, "rb");
	if (fh == NULL) {
		_FUNC_TRACE
		printf("Fail to open %s.\n", wfile);
		return -1;
	}
	rtn = fread((void *) pWeights, sizeof(float), TotalWeightSize, fh);
	if (rtn != TotalWeightSize) {
		printf("Error reading %s.\n", wfile);
		return -1;
	}
	fclose(fh);

	// Quantize FC weight data if required
	if (1 == qFCWt) {
		char bMin, bMax;
		float a, b;
		char *pbWeight = (char *) pWeights;		// Use the same storage for quantized FC weight data

		// Find min/max of FC weight data
		fWMin = fWMax = *pWeights;

		for (int i = 1; i < TotalWeightSize; i++) {
			fWMin = min(fWMin, pWeights[i]);
			fWMax = max(fWMax, pWeights[i]);
		}
//		printf("fMin = %f, fMax = %f\n", *fMin, *fMax);

		bMin = -128;
		bMax = 127;		// Range of INT8 data type
		a = (bMax - bMin) / (fWMax - fWMin);
		b = bMin - a * fWMin;
//		printf ("Compression fA = %f, fB = %f\n", a, b);
		for (int i = 0; i < TotalWeightSize; i++) {
			pbWeight[i] = (char) (a * pWeights[i] + b);		// y = ax+b;
		}
	}

	// Read bias
	fh = fopen(bfile, "rb");
	if (fh == NULL) {
		_FUNC_TRACE
		printf("Fail to open %s.\n", bfile);
		return -1;
	}
	rtn = fread((void *) pBias, sizeof(float), TotalBiasSize, fh);
	if (rtn != TotalBiasSize){
		printf("Error reading %s.\n", bfile);
		return -1;
	}
	fclose(fh);

	// Transpose weights due to MathWorks data order in 2D weight files
	if (Param.KernelHeight > 1) {
		if (0 == qFCWt) {		// Replace following code with template function later
			float * pTemp = (float * ) malloc(sizeof(float) * WeightSize);
			for (int k = 0; k < TotalWeightSize / Param.KernelHeight / Param.KernelWidth; k++) {  // Loop through all kernel weight matrix.
				// Copy the filter weight to pTemp for transpose.
				for (int i = 0; i < WeightSize; i++) 
					pTemp[i] = pWeights[k * WeightSize + i];
				// Write back the transposed one.
				for (int j = 0; j < Param.KernelHeight; j++)
					for (int i = 0; i < Param.KernelWidth; i++) 
						pWeights[k * WeightSize + j * Param.KernelWidth + i] = pTemp[j + i * Param.KernelHeight];
			}
			free(pTemp);
		}
		else {
			char * pTemp = (char * ) malloc(sizeof(char) * WeightSize);
			char * pbWeights = (char *) pWeights;
			for (int k = 0; k < TotalWeightSize / Param.KernelHeight / Param.KernelWidth; k++) {  // Loop through all kernel weight matrix.
				// Copy the filter weight to pTemp for transpose.
				for (int i = 0; i < WeightSize; i++) 
					pTemp[i] = pbWeights[k * WeightSize + i];
				// Write back the transposed one.
				for (int j = 0; j < Param.KernelHeight; j++)
					for (int i = 0; i < Param.KernelWidth; i++) 
						pbWeights[k * WeightSize + j * Param.KernelWidth + i] = pTemp[j + i * Param.KernelHeight];
			}
			free(pTemp);
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////
void CM_LAYER::SetupGPUInputSurface()
{
	if (pPrevLayer == NULL || !pPrevLayer->IsActiveLayer() || !pPrevLayer->pOutputSurf || !pPrevLayer->pOutputIndex) {
		// Allocate input surface from video memory
		if (!UseFP16 || !strcmp(Param.LayerType, "Input")) {
			// 1st input
		    AllocGPUSurface(iFrameWidth, iFrameHeight, CM_SURFACE_FORMAT_R32F, &pInputSurf, &pInputIndex);
		    pInputSurf->WriteSurfaceStride((unsigned char *) InputBuffer, NULL, pitch_inputSurf);
			// 2nd input if present
			if (NumInputs == 2) {
				AllocGPUSurface(iFrameWidth, iFrameHeight, CM_SURFACE_FORMAT_R32F, &pInputSurf2, &pInputIndex2);
				pInputSurf2->WriteSurfaceStride((unsigned char *)InputBuffer2, NULL, pitch_inputSurf);
			}
		} else {	// FP16
			// 1st input
		    AllocGPUSurface(iFrameWidth, iFrameHeight, CM_SURFACE_FORMAT_V8U8, &pInputSurf, &pInputIndex);
		    pInputSurf->WriteSurfaceStride((unsigned char *) InputBuffer, NULL, pitch_inputSurf);
			// 2nd input if present
			if (NumInputs == 2) {
				AllocGPUSurface(iFrameWidth, iFrameHeight, CM_SURFACE_FORMAT_V8U8, &pInputSurf2, &pInputIndex2);
				pInputSurf2->WriteSurfaceStride((unsigned char *)InputBuffer2, NULL, pitch_inputSurf);
			}
		}
	} else {	// Copy the output surface of prev layer.
		// 1st input
		pInputSurf = pPrevLayer->pOutputSurf;
		pInputIndex = pPrevLayer->pOutputIndex;
		// 2nd input if present
		if (NumInputs == 2) {
			pInputSurf2 = pPrevLayer2->pOutputSurf;
			pInputIndex2 = pPrevLayer2->pOutputIndex;
		}
	}
	InputMemType = MEMORY_2D;
}

//////////////////////////////////////
void CM_LAYER::SetupGPUOutputSurface()
{
    if (!UseFP16)
	    AllocGPUSurface(oFrameWidth, oFrameHeight, CM_SURFACE_FORMAT_R32F, &pOutputSurf, &pOutputIndex);
//	else	
//	    AllocGPUSurface(oFrameWidth, oFrameHeight, CM_SURFACE_FORMAT_V8U8, &pOutputSurf, &pOutputIndex);
	OutputMemType = MEMORY_2D;
}

//////////////////////////////////////
void CM_LAYER::SetupGPUOutputBuffer()
{
    if (!UseFP16)
	    AllocGPUBuffer(oFrameWidth * oFrameHeight * sizeof(float), &pOutputBuf, &pOutputIndex);
//	else
//	    AllocGPUBuffer(oFrameWidth * oFrameHeight * sizeof(half), &pOutputBuf, &pOutputIndex);
	OutputMemType = MEMORY_1D;
}

/////////////////////////////////////
void CM_LAYER::SetupGPUInputBuffer()
{
	if (pPrevLayer == NULL || !pPrevLayer->IsActiveLayer() || !pPrevLayer->pOutputBuf || !pPrevLayer->pOutputIndex) {
		// Not tested for createing 1D buffer, as it always has prev layer.
		if (!UseFP16) {
			// 1st input
			AllocGPUBuffer(iFrameWidth * iFrameHeight * sizeof(float), &pInputBuf, &pInputIndex);
			if (NumInputs == 2)
				AllocGPUBuffer(iFrameWidth * iFrameHeight * sizeof(float), &pInputBuf2, &pInputIndex2);
		}
		else {
//			AllocGPUBuffer(iFrameWidth * iFrameHeight * sizeof(half), &pInputBuf, &pInputIndex);
//			if (NumInputs == 2)
//				AllocGPUBuffer(iFrameWidth * iFrameHeight * sizeof(half), &pInputBuf2, &pInputIndex2);
		}
	} else {	// Copy the output buffer of prev layer.
		// 1st input
		pInputBuf = pPrevLayer->pOutputBuf;
		pInputIndex = pPrevLayer->pOutputIndex;
		// 2nd input if present
		if (NumInputs == 2) {
			pInputBuf2 = pPrevLayer2->pOutputBuf;
			pInputIndex2 = pPrevLayer2->pOutputIndex;
		}
	}
	InputMemType = MEMORY_1D;
}

///////////////////////////////////////////////////////////////////////////////
// This is used by Convol layer configuration. Weights and bias are combined.
void CM_LAYER::SetupGPUWeightBiasBuffer()
{

	if (Param.KernelWidth != 0) {
		if (!UseFP16) {
			AllocGPUBuffer(TotalWeightBiasSize * sizeof(float), &pWeightBiasBuf, &pWeightBiasIndex);
			pWeightBiasBuf->WriteSurface((unsigned char *)pWeightsBias, NULL);
		} else {
			// pWeightsBias has FP16 data when combined weights and bias.
//			AllocGPUBuffer(TotalWeightBiasSize * sizeof(half), &pWeightBiasBuf, &pWeightBiasIndex);
//			pWeightBiasBuf->WriteSurface((unsigned char *)pWeightsBias, NULL);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CM_LAYER::Convol1x1_Rearrange_Weight(void * pWeights, int ChunkSize, int *nWeightSize, void ** pOutWeight)
{
	int WeightSize = Param.DestDepth * Param.SrcDepth;
	int DestOffset = 0;

	if (!UseFP16) {
		float * pSrc = (float *) pWeights;
		float * pDest = (float *) malloc(WeightSize * sizeof(float));
		*pOutWeight = pDest;

		for (int j = 0; j < Param.DestDepth; j += ChunkSize)
			for (int i = 0; i < Param.SrcDepth; i++)
				for (int k = 0; k < ChunkSize; k++)
					pDest[DestOffset++] = pSrc[(j + k) * Param.SrcDepth + i];
	} else {
/*		half * pSrc = (half *) pWeights;
		half * pDest = (half *) malloc(WeightSize * sizeof(half));
		*pOutWeight = pDest;

		for (int j = 0; j < Param.DestDepth; j += ChunkSize)
			for (int i = 0; i < Param.SrcDepth; i++)
				for (int k = 0; k < ChunkSize; k++)
					pDest[DestOffset++] = pSrc[(j + k) * Param.SrcDepth + i];
*/				
	}
}

///////////////////////////////////////////////////////////////////////////////
// This is used by Convol layer configuration.  Weights and bias are separate.
void CM_LAYER::SetupGPUWeightBuffer()
{
	int nWeightSize;	// Remapped weight size

	if (Param.KernelWidth != 0) {
		if (!UseFP16) {
			AllocGPUBuffer(TotalWeightSize * sizeof(float), &pWeightBuf, &pWeightIndex);

			if (Param.KernelWidth == 1) {	// For Convol 1x1
				float * pTemp;
				Convol1x1_Rearrange_Weight((void *) pWeights, MULTIBLOCKS, &nWeightSize, (void **) &pTemp);
				pWeightBuf->WriteSurface((unsigned char *)pTemp, NULL);
				free(pTemp);
			} else {
				pWeightBuf->WriteSurface((unsigned char *)pWeights, NULL);
			}

		} else {
/*			
			half * pFP16 = (half *)malloc(TotalWeightSize * sizeof(half));
			for (int i = 0; i < TotalWeightSize; i++)
				pFP16[i] = pWeights[i];
			
			AllocGPUBuffer(TotalWeightSize * sizeof(half), &pWeightBuf, &pWeightIndex);
			pWeightBuf->WriteSurface((unsigned char *)pFP16, NULL);
			free(pFP16);
*/			
		}
	}
}

////////////////////////////////////
void CM_LAYER::SetupGPUBiasBuffer()
{
	if (Param.KernelWidth != 0) {
		if (!UseFP16) {
			AllocGPUBuffer(TotalBiasSize * sizeof(float), &pBiasBuf, &pBiasIndex);
			pBiasBuf->WriteSurface((unsigned char *) pBias, NULL);
		} else {
/*			
			half * pFP16 = (half *) malloc(TotalBiasSize * sizeof(half));
			for (int i = 0; i < TotalBiasSize; i++)
				pFP16[i] = pBias[i];

		    AllocGPUBuffer(TotalBiasSize * sizeof(half), &pBiasBuf, &pBiasIndex);
			pBiasBuf->WriteSurface((unsigned char *)pFP16, NULL);
			free(pFP16);
*/			
		}
	}
}

//////////////////////////////////////////
// This is used by FC layer configuration.
void CM_LAYER::SetupGPUWeightSurface()
{
	int nWeightSize;	// Remapped FC weight size

	if (Param.KernelWidth != 0) {
		int mWidth = Param.DestDepth;
		int mHeight = Param.SrcWidth * Param.SrcHeight * Param.SrcDepth;

		if (qFCWt)	{	// Quantized weight data
			char *pTemp;
			if (!UseFP16)		// If feature data don't use HF16
				FC_Rearrange_Weight <char> ((char*)pWeights, 8, 16, &nWeightSize, (char**)&pTemp);
			else		// If feature data use HF16
				FC_Rearrange_Weight <char> ((char*)pWeights, 16, 16, &nWeightSize, (char**)&pTemp);
			AllocGPUBuffer(nWeightSize, &pWeightSurf_1D, &pWeightIndex);
			pWeightSurf_1D->WriteSurface((unsigned char *) pTemp, NULL);
			free(pTemp);
		}
		else {			// Original unquantized weight data
			if (!UseFP16) {
#ifdef FCWT_2D				// Use Surface2D for FC weight data
				float * pTemp = (float *) malloc(mWidth * mHeight * sizeof(float));
				AllocGPUSurface( mWidth, mHeight, CM_SURFACE_FORMAT_R32F, &pWeightSurf, &pWeightIndex);
				// Transpose the weight matrix.
				// For layer 11, the input is 6x6x256=9216.  Weight is stored in a matrix of hxw = 4096x9216.
				// Here we wave 1st set of weights (1st row) in the 1st column of the matrix of hxw = 9216x4096.
				// Then GPU does dot-product from top row to bottom row.
#if 0
				for (int j = 0; j < mHeight; j++)
					for (int i = 0; i < mWidth; i++) {
						pTemp[mWidth * j + i] = pWeights[mHeight * i + j];
					}
#else
				FC_Transpose_Weight <float> (pWeights, mWidth, mHeight, pTemp);
#endif

				pWeightSurf->WriteSurface((unsigned char *) pTemp, NULL);
				free(pTemp);
#else						// Use Buffer for FC weight data
				// First convert the weight data to vertical tile based buffer for better OWord read by kernels
				// The width of the tile should be the same as output size from the FC kernels
				// And the tile is padded to the multiple of the block height the FC kernels process within the loop
				// Current each FC kernel loops through 8x16 (wxh) data blocks

				float * pTemp;
				FC_Rearrange_Weight <float> ((float*)pWeights, 8, 16, &nWeightSize, (float**)&pTemp);

				AllocGPUBuffer(nWeightSize, &pWeightSurf_1D, &pWeightIndex);
				pWeightSurf_1D->WriteSurface((unsigned char *) pTemp, NULL);
				free(pTemp);
#endif
			} else {				// HF16 data
/*			
#ifdef FCWT_2D				// Use Surface2D for FC weight data
				half *pTemp16 = (half *)malloc(mWidth * mHeight * sizeof(half));
				AllocGPUSurface( mWidth, mHeight, CM_SURFACE_FORMAT_V8U8, &pWeightSurf, &pWeightIndex);
				// Transpose the weight matrix.
				// For layer 11, the input is 6x6x256=9216.  Weight is stored in a matrix of hxw = 4096x9216.
				// Here we wave 1st set of weights (1st row) in the 1st column of the matrix of hxw = 9216x4096.
				// Then GPU does dot-product from top row to bottom row.
#if 0
				for (int j = 0; j < mHeight; j++)
					for (int i = 0; i < mWidth; i++) {
						pTemp16[mWidth * j + i] = pWeights[mHeight * i + j];
					}
#else
				FC_Transpose_Weight <half> (pWeights, mWidth, mHeight, pTemp16);
#endif
				pWeightSurf->WriteSurface((unsigned char *) pTemp16, NULL);
				free(pTemp16);
#else
				// First convert FC weights from float to half
				half * pTemp = (half*)pWeights;
				for(int i = 0; i < WeightSize * Param.SrcDepth * Param.DestDepth; i++)
					*pTemp++ = pWeights[i];

				FC_Rearrange_Weight <half> ((half*)pWeights, 16, 16, &nWeightSize, (half**)&pTemp);

				AllocGPUBuffer(nWeightSize, &pWeightSurf_1D, &pWeightIndex);
				pWeightSurf_1D->WriteSurface((unsigned char *) pTemp, NULL);
				free(pTemp);
#endif
*/
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
// Rearrange FC weight data for 1D buffer access
template <typename WT>
int CM_LAYER::FC_Rearrange_Weight(
	WT* pWeight,
	int nBlockWidth,
	int nBlockHeight,
	int *nWeightSize,
	WT** pOutWeight
	)
{
	WT *m_pWeight;
	int nNumBand;
	int SrcBandSize;		// Original weight size per band
	int DstBandSize;		// Converted weight size per band
	WT *pWeightTmp;
	int nWeight = 0;
	int nBandPadding;
	int m_nWeightCount = Param.DestDepth;
	int m_nWeightLength = WeightSize * Param.SrcDepth;
	int m_nWeightCount_pad;			// Aligned to next 8
	int m_nWeightLength_pad;		// Aligned to next 16
	uint nWidthMask, nHeightMask;

	nWidthMask = ~(nBlockWidth - 1);
	nHeightMask = ~(nBlockHeight -1);
	m_nWeightCount_pad = (m_nWeightCount + nBlockWidth - 1) & nWidthMask;		// Aligned to next nBlockWidth
	m_nWeightLength_pad = (m_nWeightLength + nBlockHeight -1) & nHeightMask;	// Aligned to next nBlockHeight
	*nWeightSize = m_nWeightCount_pad * m_nWeightLength_pad * sizeof(WT);		// New buffer size
//	m_pWeight = (WT *)_aligned_malloc(*nWeightSize, 4096);
	m_pWeight = (WT *) malloc(*nWeightSize);

	*pOutWeight = m_pWeight;

	nNumBand = m_nWeightCount_pad / nBlockWidth;
	SrcBandSize = m_nWeightLength * nBlockWidth;		// Original weight size per band
	DstBandSize = m_nWeightLength_pad * nBlockWidth;	// Converted weight size per band

	for (int i = 0; i<nNumBand-1; i++) {
		pWeightTmp = &pWeight[SrcBandSize * i];
		for (int j = 0; j < m_nWeightLength; j++) {
			for (int k = 0; k < nBlockWidth; k++) {
				m_pWeight[nWeight++] = pWeightTmp[k * m_nWeightLength];
			}
			pWeightTmp++;	// Move to next weight
		}
		// Possible padding data
		if (m_nWeightLength_pad > m_nWeightLength) {
			nBandPadding = (m_nWeightLength_pad - m_nWeightLength) * nBlockWidth;
			for (int i = 0; i < nBandPadding; i++)
				m_pWeight[nWeight++] = 0;
		}
	}
	// Special handling of the last band since it might be a partial band
	pWeightTmp = &pWeight[SrcBandSize * (nNumBand-1)];
	if (m_nWeightCount_pad == m_nWeightCount) {		// If the last band is a full band
		for (int j = 0; j < m_nWeightLength; j++) {
			for (int k = 0; k < nBlockWidth; k++) {
				m_pWeight[nWeight++] = pWeightTmp[k * m_nWeightLength];
			}
			pWeightTmp++;	// Move to next weight
		}
	}
	else {											// If the last band is a partial band
		int nLastBandPadding = m_nWeightCount_pad - m_nWeightCount;
		int nLastBandWidth = nBlockWidth - nLastBandPadding;
		for (int j = 0; j < m_nWeightLength; j++) {
			for (int k = 0; k < nLastBandWidth; k++) {
				m_pWeight[nWeight++] = pWeightTmp[k * m_nWeightLength];
			}
			for (int k = 0; k < nLastBandPadding; k++) {	// Padding remaining of row with "0"
				m_pWeight[nWeight++] = 0;
			}
			pWeightTmp++;	// Move to next weight
		}
	}
	// Possible padding data on the bottom of last band
	if (m_nWeightLength_pad > m_nWeightLength) {
		nBandPadding = (m_nWeightLength_pad - m_nWeightLength) * nBlockWidth;
		for (int i = 0; i < nBandPadding; i++)
			m_pWeight[nWeight++] = 0;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
// Transpose the weight matrix.
template <typename WT>
int CM_LAYER::FC_Transpose_Weight(
	float* pInWeights,		// Input is always float type
	int mWidth,
	int mHeight,
	WT* pOutWeights
	)
{
	// Transpose the weight matrix.
	// For example for layer 11, the input is 6x6x256=9216.  Weight is stored in a matrix of hxw = 4096x9216.
	// Here we wave 1st set of weights (1st row) in the 1st column of the matrix of hxw = 9216x4096.
	// Then GPU does dot-product from top row to bottom row.
	for (int j = 0; j < mHeight; j++) {
		for (int i = 0; i < mWidth; i++) {
			pOutWeights[mWidth * j + i] = pInWeights[mHeight * i + j];
		}
	}
	return 0;
}

///////////////////////////////////
int CM_LAYER::CopyFromGPUSurface()
{
	if (oFrameHeight > 1)
		pOutputSurf->ReadSurfaceStride( (unsigned char *) OutputBuffer, NULL, pitch_outputSurf);
	else
		pOutputBuf->ReadSurface( (unsigned char *) OutputBuffer, NULL);
	return 0;
}

//////////////////////////////////
int CM_LAYER::CopyFromGPUBuffer()
{
	pOutputBuf->ReadSurface( (unsigned char *) OutputBuffer, NULL);
	return 0;
}

#ifdef _DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Save floating point data in raw format
int CM_LAYER::SaveImage(char RefFile[])
{
	char fn[64];

	sprintf(fn, "Output/Layer%d.%dx%d.bmp", LayerID, oFrameWidth, oFrameHeight);
	SaveBMPFile(fn, (void *) OutputBuffer, pitch_outputSurf, oFrameWidth, oFrameHeight, UseFP16);  // Only 2D image is saved.

//*
	// Increase output intensity for better viewing bmp files.
	float * oBuf = (float *) _aligned_malloc(size_outputSurf, 4096);
	memcpy(oBuf, OutputBuffer, size_outputSurf);
//	for (int i = 0; i < size_outputSurf / sizeof(float); i++)
//		oBuf[i] *= 16.0f;
	SaveBMPFile(fn, (void *) oBuf, pitch_outputSurf, oFrameWidth, oFrameHeight, UseFP16);  // Only 2D image is saved.
	_aligned_free(oBuf);
//*/
	// Read the reference file
	int RefSize = Param.DestWidth * Param.DestHeight * Param.DestDepth;	// in float, signle batch
	float * pRef = (float *) malloc(RefSize * sizeof(float));
	if (ReadRefFile(RefFile, pRef, RefSize) == -1)
		return -1;

	// Save Ref (in FP32) in bmp
	if (OutputMemType == MEMORY_2D) {
		sprintf(fn, "Output/Layer%d.REF.%dx%dx%d.bmp", LayerID, oTileWidth, oTileHeight, Param.DestDepth);
		float * pRef2 = (float *) malloc(oFrameWidth * oFrameHeight * sizeof(float));
		memset(pRef2, 0, oFrameWidth * oFrameHeight * sizeof(float));
		LinearToTiledImage(	pRef,								// src
							pRef2,								// dest
							oFrameWidth,					 	// pitch
							Param.DestDepth * Param.BatchSize,	// images
							oTileHoriCount,						// HoriTiles
							oTileWidth,						 
							oTileHeight, 
							Param.OutputBorderWidth, 
							Param.DestWidth, 
							Param.DestHeight, 
							Param.DestDepth,
							0);
		// Debug
//		for (int i = 0; i < oFrameWidth * oFrameHeight; i++)
//			pRef2[i] *= 16.0f;

		SaveBMPFile(fn, (void *) pRef2, oFrameWidth * sizeof(float), oFrameWidth, oFrameHeight, 0);
		free(pRef2);
	}

	unsigned int TotalMismatches = CompareRefernece(OutputBuffer, pRef);
	if (TotalMismatches > 0 || OutputMemType == MEMORY_1D) {
		// Svae Ref in txt if there is a mismatch.
		sprintf(fn, "Output/Layer%d.REF.%dx%dx%d.txt", LayerID, Param.DestWidth, Param.DestHeight, Param.DestDepth);
		Matrices2File(fn, pRef, Param.DestWidth, Param.DestHeight, Param.DestDepth, Param.OutputBorderWidth, oTileWidth, oTileHeight, 0);

		// Output pixels in numerical values in a txt file image by image
		if (oFrameHeight > 1) {  // For 2D surface
			sprintf(fn, "Output/Layer%d.%dx%dx%d.txt", LayerID, oTileWidth, oTileHeight, Param.DestDepth * Param.BatchSize);
//			int ElementSize = UseFP16 ? sizeof(half) : sizeof(float);
			int ElementSize = sizeof(float);
			TiledMatrices2File(fn, OutputBuffer, pitch_outputSurf/ElementSize, oTileWidth, oTileHeight, oTileHoriCount, Param.DestDepth, UseFP16);
		} else {				// For 1D buffer
			sprintf(fn, "Output/Layer%d.%dx%dx%d.txt", LayerID, Param.DestWidth, Param.DestHeight, Param.DestDepth * Param.BatchSize);
			Matrices2File(fn, OutputBuffer, Param.DestWidth, Param.DestHeight, Param.DestDepth * Param.BatchSize, Param.OutputBorderWidth, oTileWidth, oTileHeight, UseFP16);
		}
	}

	free(pRef);
	return 0;
}

////////////////////////////////////////////////////////////////////
int CM_LAYER::ReadRefFile(char RefFile[], float * pRef, int RefSize)
{
	FILE * fh;

	if (strstr(RefFile, "txt") == NULL) {
		// Read binany file
		fh = fopen(RefFile, "rb");
		if (fh == NULL) {
			_FUNC_TRACE
			printf("Ref file %s does not exist.\n", RefFile);
			return -1;
		}
		int rtn = fread((void *) pRef, sizeof(float), RefSize, fh);
		if (rtn != RefSize) {
			printf("Cannot read specific size %d.\n", RefSize);
			return -1;
		}
	} else {
		// Read txt file
		fh = fopen(RefFile, "r");
		if (fh == NULL) {
			_FUNC_TRACE
			printf("Ref file %s does not exist.\n", RefFile);
			return -1;
		}
		for (int i = 0; i < RefSize; i++) 
			fscanf(fh, "%f\n", &pRef[i]);
	}

	fclose(fh);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Compare with reference data
unsigned int CM_LAYER::CompareRefernece(float * OutputBuffer, float * pRef)
{
	unsigned int TotalMisMatches = 0;
	unsigned int BatchMisMatch[64] = { 0 };
	float Epsilon = (!UseFP16) ? 0.05f : 0.5f;

	float * pOneImage = (float *) malloc(Param.DestWidth * Param.DestHeight * sizeof(float));

	if (OutputMemType == MEMORY_2D) {  // For 2D surface
		for (int DstIdx = 0; DstIdx < Param.DestDepth * Param.BatchSize; DstIdx++) {
			int b = DstIdx / Param.DestDepth;
			ReadOneTiledImage(OutputBuffer, DstIdx, pOneImage, Param.OutputBorderWidth);
			// The refernece image has data for one batch only.  It needs to be reused.
			float * pRef2 = pRef + DstIdx % Param.DestDepth * Param.DestWidth * Param.DestHeight;

			for (int j = 0; j < Param.DestHeight; j++) {
				for (int i = 0; i < Param.DestWidth; i++) {
					float u = *(pOneImage + Param.DestWidth * j + i);
					float v = *(pRef2 + Param.DestWidth * j + i);
					float Diff = u - v;
					if (Diff > Epsilon || Diff < -Epsilon) {
						TotalMisMatches++;
						BatchMisMatch[b]++;
					}
				}
			}
		}
	}
	else {		// For 1D buffer.  Use oDepth for allocated space.
		int OneBatchSize = Param.DestWidth * Param.DestHeight * oDepth;
//		if (UseFP16)
//			OneBatchSize /= (sizeof(float)/sizeof(half));

		for (int b = 0; b < Param.BatchSize; b++) {
			for (int DstIdx = 0; DstIdx < Param.DestDepth; DstIdx++) {
				ReadOneLinearImage(OutputBuffer + (OneBatchSize * b), DstIdx, pOneImage, 0);
				// The refernece image has data for one batch only.  It needs to be reused.
				float * pRef2 = pRef + DstIdx % Param.DestDepth * Param.DestWidth * Param.DestHeight;

				for (int j = 0; j < Param.DestHeight; j++) {
					for (int i = 0; i < Param.DestWidth; i++) {
						float u = *(pOneImage + Param.DestWidth * j + i);
						float v = *(pRef2 + Param.DestWidth * j + i);
						float Diff = u - v;
						if (Diff > Epsilon || Diff < -Epsilon) {
							TotalMisMatches++;
							BatchMisMatch[b]++;
						}
					}
				}
			}
		}
	}
	
	printf("Layer%4d %-12s total mismatches: %d \t(Sample mismatches:", LayerID, Param.LayerType, TotalMisMatches);
	for (int b = 0; b < Param.BatchSize; b++)
		printf(" %d", BatchMisMatch[b]);
	printf(")\n");

	free(pOneImage);
	return TotalMisMatches;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Read an image from the given tiled image surface withput border.
void CM_LAYER::ReadOneTiledImage(float * OutputBuffer, int ImgIdx, float * pOneImage, int BorderWidth)
{
	int oOffsetX = ImgIdx%oTileHoriCount;
	int oOffsetY = ImgIdx/oTileHoriCount;
	float * pBase = OutputBuffer + pitch_outputSurf/sizeof(float) * (oTileHeight * oOffsetY) + (oTileWidth * oOffsetX);
//	half * pBase16 = (half*)(OutputBuffer) + pitch_outputSurf/sizeof(half) * (oTileHeight * oOffsetY) + (oTileWidth * oOffsetX);

	for (int j = 0; j < Param.DestHeight; j++) {
		for (int i = 0; i < Param.DestWidth; i++) {
			if (!UseFP16) {
			    *(pOneImage + Param.DestWidth * j + i) = *(pBase + pitch_outputSurf/sizeof(float) * (j+BorderWidth) + (i+BorderWidth));
			} else {
//				*(pOneImage + Param.DestWidth * j + i) = *(pBase16 + pitch_outputSurf/sizeof(half) * (j+BorderWidth) + (i+BorderWidth));
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Read an image from the given linear image buffer withput border.
void CM_LAYER::ReadOneLinearImage(float * OutputBuffer, int ImgIdx, float * pOneImage, int BorderWidth)
{
	int oOffsetX = oTileWidth * ImgIdx;
	float * pBase = OutputBuffer + oOffsetX;
//	half * pBase16 = (half*)(OutputBuffer) + oOffsetX;

	for (int j = 0; j < Param.DestHeight; j++) {
		for (int i = 0; i < Param.DestWidth; i++) {
			if (!UseFP16) {
			    *(pOneImage + Param.DestWidth * j + i) = *(pBase + Param.DestWidth * (j+BorderWidth) + (i+BorderWidth));
			} else {
//				*(pOneImage + Param.DestWidth * j + i) = *(pBase16 + Param.DestWidth * (j+BorderWidth) + (i+BorderWidth));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
int CM_LAYER::GPUSync()
{
	int dwTimeOutMs = -1;
	int result = pEvent->WaitForTaskFinished(dwTimeOutMs);
	if (result != CM_SUCCESS ) {
		printf("CM WaitForTaskFinished error: %d.\n", result);
		return -1;
	}
	return 0;
}

///////////////////////////////////////////////
void CM_LAYER::GetLayerOutput(float * pMWOutput)
{
	// Copy to CM OutputBuffer
	if (oFrameHeight > 1)
		CopyFromGPUSurface();	
	else
		CopyFromGPUBuffer();

	// Copy OutputBuffer to pMWOutput.  pMWOutput is consecutive buffer, no pitch.
	int OneImageSize = Param.DestWidth * Param.DestHeight;
	for (int DstIdx = 0; DstIdx < Param.DestDepth * Param.BatchSize; DstIdx++) {
		if (oFrameHeight > 1)   // for 2D surface
			ReadOneTiledImage(OutputBuffer, DstIdx, &pMWOutput[OneImageSize * DstIdx], Param.OutputBorderWidth);
		else					// for 1D buffer
			ReadOneLinearImage(OutputBuffer, DstIdx, &pMWOutput[OneImageSize * DstIdx], Param.OutputBorderWidth);
	}
}

////////////////////////////////////////////////////////////////	
// This function should be called after a CPU and GPU sync point.
float CM_LAYER::GetLayerPerformance()
{
	UINT64 CurTime;
	pEvent->GetExecutionTime(CurTime);
	fKernelTimeMS = CurTime / 1000000.0;		// In milliseconds
	return fKernelTimeMS;
}

///////////////////////////////////////////////////////////////////////
void CM_LAYER::CompareOutput(int netIdx)
{
#ifdef _DEBUG
	// Save the layer's output
	if (!strcmp(Param.LayerType, "Input") || 
		!strcmp(Param.LayerType, "Convol") || 
		!strcmp(Param.LayerType, "ReLU") && OutputMemType == MEMORY_2D || 
		!strcmp(Param.LayerType, "LRN") || 
		!strcmp(Param.LayerType, "MaxPool") ||
		!strcmp(Param.LayerType, "AvgPool"))
		CopyFromGPUSurface();
	else if (!strcmp(Param.LayerType, "SurfConv") && OutputMemType == MEMORY_1D || 
		!strcmp(Param.LayerType, "FC") || 
		!strcmp(Param.LayerType, "ReLU") && OutputMemType == MEMORY_1D || 
		!strcmp(Param.LayerType, "SoftMax") || 
		!strcmp(Param.LayerType, "Output"))
		CopyFromGPUBuffer();
	else {
		printf("Unknown layer type: %s\n", Param.LayerType);
	}

	SaveImage(Param.RefFile);
#endif
}

