#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <io.h>
#include <direct.h>

#include "GC_API.h"  
#include "Graph_Cut_host.h"
#include "General.h"

#define VERTICAL_PUSH       0
#define HORIZONTAL_PUSH     1

#define NUM_FILES			6
#define ALIGNMENT32			32
#define ALIGNMENT4K			4096

/////////////////////////////////////////////////////////////////////////////////
// CPU API

int _GC::CPU_GraphCut( short * pNodes, short * pHoriWeights, short * pVertWeights, unsigned char * pOutput, int FrameWidth, int FrameHeight )  
{  
	HEIGHT_TYPE HEIGHT_MAX;

	int BlkWidth = 16;
	int BlkHeight = 16;
	int nBits  = 5;			// Lower bits to check

	// Note: This buffer is used in CPU code only with 16x16 block size.
	// 2D buffer, pitch may not be aligned
    int BlkCols = (int) (((float) FrameWidth) / BlkWidth + 0.5f);
    int BlkRows = (int) (((float) FrameHeight) / BlkHeight + 0.5f);
	unsigned char * pBlockMask = (unsigned char *) _aligned_malloc( BlkCols * BlkRows, ALIGNMENT4K );
    if (pBlockMask == NULL) {
        printf("pBlockMask allocation failed.\n");
		exit(-1);
    }
	memset(pBlockMask, 0, BlkCols * BlkRows);

	int size = FrameWidth*FrameHeight / (SIDE_SQUARE*SIDE_SQUARE) + 1;
	RelabelBlock * tableBlock = (RelabelBlock *) _aligned_malloc(sizeof(RelabelBlock) * size, ALIGNMENT32);
	memset(tableBlock, -1, sizeof(RelabelBlock) * size);

	//sprintf(CPU_fn, ".\\Output\\%s_Output_CPU.%dx%d.Y8", prefix, FrameWidth, FrameHeight);

	// TF code
	int FrameWidthTF = FrameWidth;
	int FrameHeightTF = FrameHeight + FrameWidth;

	int wBufferSizeTF = sizeof(short) * FrameHeightTF * FrameWidthTF;
    short * pExcessFlowTF = (short *) _aligned_malloc( wBufferSizeTF, ALIGNMENT32 ); 
    short * pWestCapTF = (short *) _aligned_malloc( wBufferSizeTF, ALIGNMENT32 ); 
    short * pNorthCapTF = (short *) _aligned_malloc( wBufferSizeTF, ALIGNMENT32 ); 
    short * pEastCapTF = (short *) _aligned_malloc( wBufferSizeTF, ALIGNMENT32 ); 
    short * pSouthCapTF = (short *) _aligned_malloc( wBufferSizeTF, ALIGNMENT32 ); 
	memset(pExcessFlowTF, 0, wBufferSizeTF);
	memset(pWestCapTF, 0, wBufferSizeTF);
	memset(pNorthCapTF, 0, wBufferSizeTF);
	memset(pEastCapTF, 0, wBufferSizeTF);
	memset(pSouthCapTF, 0, wBufferSizeTF);

	int iBufferSizeTF = sizeof(HEIGHT_TYPE) * wBufferSizeTF;
	HEIGHT_TYPE * pHeightTF = (HEIGHT_TYPE *) _aligned_malloc( iBufferSizeTF, ALIGNMENT32 ); 
	memset(pHeightTF, 0, iBufferSizeTF);

    int BlkColsTF = BlkCols;
    int BlkRowsTF = BlkRows + BlkCols;

	unsigned char * pBlockMaskTF = (unsigned char *) _aligned_malloc( BlkColsTF * BlkRowsTF, ALIGNMENT4K );
    if (pBlockMaskTF == NULL) {
        printf("pBlockMaskTF allocation failed.\n");
		exit(-1);
    }
	memset(pBlockMaskTF, 0, BlkColsTF * BlkRowsTF);


	BlkColsTF = (FrameWidth >> LOG_SIZE) + ((FrameWidth&(SIDE_SQUARE - 1)) != 0);
	BlkRowsTF = ((FrameHeight + FrameWidth - 1) >> LOG_SIZE) + (((FrameHeight + FrameWidth - 1) & (SIDE_SQUARE - 1)) != 0);
	FrameWidthTF = (BlkColsTF << LOG_SIZE) + IMAGE_PADDING + 1;
	FrameHeightTF = (BlkRowsTF << LOG_SIZE) + 2;
//	FrameWidthTF = (BlkColsTF << LOG_SIZE) + 2 * BORDER;
//	FrameHeightTF = (BlkRowsTF << LOG_SIZE) + 2 * BORDER;

	wBufferSizeTF = sizeof(short) * FrameWidthTF * FrameHeightTF;
	pExcessFlowTF = (short *)_aligned_malloc(wBufferSizeTF, ALIGNMENT32);
	pWestCapTF = (short *)_aligned_malloc(wBufferSizeTF, ALIGNMENT32);
	pNorthCapTF = (short *)_aligned_malloc(wBufferSizeTF, ALIGNMENT32);
	pEastCapTF = (short *)_aligned_malloc(wBufferSizeTF, ALIGNMENT32);
	pSouthCapTF = (short *)_aligned_malloc(wBufferSizeTF, ALIGNMENT32);
	memset(pExcessFlowTF, 0, wBufferSizeTF);
	memset(pWestCapTF, 0, wBufferSizeTF);
	memset(pNorthCapTF, 0, wBufferSizeTF);
	memset(pEastCapTF, 0, wBufferSizeTF);
	memset(pSouthCapTF, 0, wBufferSizeTF);

	iBufferSizeTF = sizeof(HEIGHT_TYPE) * FrameWidthTF*FrameHeightTF;
	pHeightTF = (HEIGHT_TYPE *)_aligned_malloc(iBufferSizeTF, ALIGNMENT32);
	HEIGHT_MAX = min(FrameWidth*FrameHeight, TYPE_MAX - 1);
	for (int i = 0; i < (FrameWidthTF*FrameHeightTF); i++)
		pHeightTF[i] = HEIGHT_MAX;

	size = ((FrameWidthTF - (IMAGE_PADDING + 1)) * (FrameHeightTF - 2)) / (SIDE_SQUARE*SIDE_SQUARE) + 1;
//	size = ((FrameWidthTF - 2 * BORDER) * (FrameHeightTF -  2 * BORDER)) / (SIDE_SQUARE*SIDE_SQUARE) + 1;
	RelabelBlock * pTableBlockTF = (RelabelBlock *)_aligned_malloc(sizeof(RelabelBlock) * size, ALIGNMENT32);
	memset(pTableBlockTF, -1, sizeof(RelabelBlock) * size);

//	printf("--------------------------------------------------------------------------------------------------------------\n\n");
//	printf("TF AVX2 model:\n");
	double time_e = GetTimeMS();

	// Graph cut (push-relabel) C model entry
	AVX2PushRelabel_Init_TF(pNodes, pHoriWeights, pVertWeights, FrameHeight, FrameWidth, FrameWidthTF,
		pExcessFlowTF, pHeightTF, pWestCapTF, pNorthCapTF, pEastCapTF, pSouthCapTF);

	// Debug dump
#ifdef _DEBUG
	sprintf(AVX_TF_fn, ".\\Output\\ExcessFlowTF.AVX2.%dx%d.Y8", 2*FrameWidthTF, FrameHeightTF);
	Dump2File(AVX_TF_fn, (unsigned char *)pExcessFlowTF, wBufferSizeTF);
#endif

	int iter_AVX2_TF = AVX2Model_Push_Relabel_TF(pExcessFlowTF, pHeightTF, pWestCapTF, pNorthCapTF, pEastCapTF, pSouthCapTF, pBlockMaskTF, pTableBlockTF,
		pOutput, FrameHeight, FrameWidth, FrameHeightTF, FrameWidthTF, BlkRowsTF, BlkColsTF, BlkWidth, BlkHeight, nBits);

	double time_f = GetTimeMS();

	printf("AVX2 Graph-Cut loops = %d\n", iter_AVX2_TF);
	printf("AVX2 model time = %g ms\n", time_f - time_e);

//	sprintf(AVX_TF_fn, ".\\Output\\%s_Output_TF_AVX2.%dx%d.Y8", prefix, FrameWidth, FrameHeight);
//	Dump2File(AVX_TF_fn, (unsigned char *)pOutputCPU, OutputFrameSize);

//	sprintf(diff_fn, ".\\Output\\%s_CPU_AVX_DIFF.txt", prefix);
//	Comp2ImageFileByte(CPU_fn, AVX_TF_fn, diff_fn, FrameWidth, FrameWidth, FrameHeight);
//	printf("--------------------------------------------------------------------------------------------------------------\n\n");

	_aligned_free(pExcessFlowTF); 
    _aligned_free(pWestCapTF); 
    _aligned_free(pNorthCapTF); 
    _aligned_free(pEastCapTF); 
    _aligned_free(pSouthCapTF); 
	_aligned_free(pHeightTF); 
	_aligned_free(pBlockMaskTF);

	return 0;
}
