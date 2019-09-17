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

#ifdef WIN32
#include "stdafx.h"
#endif

#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <math.h>
#ifdef WIN32
#include <io.h>
#endif
#include <stdlib.h>

#include "cmDNN.h"

//using namespace CM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CM_LAYER::CModel_Convolution()
{
	if (Param.NumGroups == 1) {
		// For all ouptut images
		for (int DstIdx = 0; DstIdx < Param.DestDepth; DstIdx++)
			// For all input images
			for (int SrcIdx = 0; SrcIdx < Param.SrcDepth; SrcIdx++)
				CModel_Convol(SrcIdx, DstIdx, 0);
	} else if (Param.NumGroups == 2) {
		// 1st half of output
		for (int DstIdx = 0; DstIdx < Param.DestDepth / Param.NumGroups; DstIdx++)
			// 1st half of Input
			for (int SrcIdx = 0; SrcIdx < Param.SrcDepth / Param.NumGroups; SrcIdx++)
				CModel_Convol(SrcIdx, DstIdx, 0);

		// 2nd half of output
		for (int DstIdx = Param.DestDepth / Param.NumGroups; DstIdx < Param.DestDepth; DstIdx++)
			// 2nd half of Input
			for (int SrcIdx = Param.SrcDepth / Param.NumGroups; SrcIdx < Param.SrcDepth; SrcIdx++)
				CModel_Convol(SrcIdx, DstIdx, 1);
	}

	// Add bias to each output
	for (int DstIdx = 0; DstIdx < Param.DestDepth; DstIdx++) {
		int oOffsetX = DstIdx%oTileHoriCount;
		int oOffsetY = DstIdx/oTileHoriCount;
		float * pDest = OutputBuffer + pitch_outputSurf/4 * oTileHeight * oOffsetY + oTileWidth * oOffsetX;

		for (int oRow = Param.OutputBorderWidth; oRow < Param.DestHeight + Param.OutputBorderWidth; oRow++) {
			for (int oCol = Param.OutputBorderWidth; oCol < Param.DestWidth + Param.OutputBorderWidth; oCol++) {

				float * ptr = pDest + pitch_outputSurf/4 * oRow + oCol;
				*ptr += *(pBias + DstIdx);

				// Perform ReLU
				if (Param.EnableReLU && *ptr < 0.0f)
					*ptr = 0.0f;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Convolution in one tile
void CM_LAYER::CModel_Convol(int SrcIdx, int DstIdx, int GroupID)
{
	// Get src tile origin indexd by SrcIdx
	int iOffsetX = SrcIdx%iTileHoriCount;
	int iOffsetY = SrcIdx/iTileHoriCount;
	float * pSrc = InputBuffer + pitch_inputSurf/sizeof(float) * iTileHeight * iOffsetY + iTileWidth * iOffsetX;
	// Get dst tile origin indexd by DstIdx
	int oOffsetX = DstIdx%oTileHoriCount;
	int oOffsetY = DstIdx/oTileHoriCount;
	float * pDest = OutputBuffer + pitch_outputSurf/sizeof(float) * oTileHeight * oOffsetY + oTileWidth * oOffsetX;

	// Copy corresponding weights to a pKBuf;
#ifdef NOT_USED
	int FilterSize = Param.KernelHeight * Param.KernelWidth;
	float * pKBuf = (float *) malloc(FilterSize*sizeof(float)); 
	int GroupAdjustedSrcIdx = (GroupID == 0) ? SrcIdx : (SrcIdx - Param.SrcDepth / Param.NumGroups);
	float * pKTileBase = pKernelWeights + KernelSurfWidth * (KernelTileHeight * DstIdx) + (KernelTileWidth * GroupAdjustedSrcIdx);
	for (int j = 0, k = 0; j < KernelTileHeight; j++)
		for (int i = 0; i < KernelTileWidth; i++) {
			if (k < FilterSize)
				*(pKBuf + k++) = *(pKTileBase + KernelSurfWidth * j + i);
			else 
				break;
		}
#endif
	// Copy corresponding weights to a pKBuf;
	int GroupAdjustedSrcIdx = (GroupID == 0) ? SrcIdx : (SrcIdx - Param.SrcDepth / Param.NumGroups);	// For 2 groups case

	// Get the kernel offset in the kernel buffer given DstIdx and SrcIdx
	float * pBase = pWeightsBias + 
					DstIdx * (WeightSize+1) * (Param.SrcDepth / Param.NumGroups) +
					GroupAdjustedSrcIdx * (WeightSize+1);

	float * pKBuf = (float *) malloc((WeightSize+1) * sizeof(float)); 

	for (int i = 0, k = 0; i < (WeightSize+1); i++, k++) {
		*(pKBuf + k) = *(pBase + i);
	}

	// oRow and oCol are for output grid. iRow and iCol are for input grid.
	for (int oRow = 0, iRow = 0; oRow < Param.DestHeight; oRow++, iRow += Param.KernelStride) {
			for (int oCol = 0, iCol = 0; oCol < Param.DestWidth; oCol++, iCol += Param.KernelStride) {

			float fPix = 0.0f;
			int idx = 0;

			// Convolution for one output pixel
			for (int j = iRow; j < iRow + Param.KernelHeight; j++) {
				for (int i = iCol; i < iCol + Param.KernelWidth; i++) {
					float * pix = pSrc + pitch_inputSurf/4 * j + i;
					fPix += pKBuf[idx] * (*pix);
					idx++;
				}
			}

			*(pDest + pitch_outputSurf/4 * (oRow + Param.OutputBorderWidth) + (oCol + Param.OutputBorderWidth)) += fPix;
		}
	}

	free(pKBuf);
}

