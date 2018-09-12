#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <math.h>

#include "common_C_model.h"
#include "Graph_Cut_host.h"
#include "General.h"

// Slice width in HEIGHT_TYPE elements
#define SLICE_W ((4*64)/sizeof(HEIGHT_TYPE))
#define SLICE_H 128

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Graph Cut (Push and Relabel) Transformed MWYI
void AVX2PushRelabel_Init_TF(short * pWeight,
	short * pHCap,
	short * pVCap,
	int Rows,
	int Cols,
	int ColsTF,
	short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap)
{
	short w;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);	// Must have -1 to prevent overflow, some calculation needs HEIGHT_MAX+1

	for (int y = 0; y < Rows; y++) {
		// Index for destination
		int index = y;

		for (int x = 0; x < Cols; x++) {
			// Set excess flow
			w = GetPix(pWeight, Cols, x, y);
			PutPix(pExcessFlow, ColsTF, x + IMAGE_PADDING, index + 1, w);    // w>0 if a cap from source exists, w<0 if cap from sink exists. ExcessFlow = w.
			/*
			HEIGHT_TYPE h = (w > 0) ? w : 0;
			if (w == FG_THRESHOLD)
			h = HEIGHT_MAX;
			PutPix(pHeight, Cols, x, y, h);
			*/
			//            PutPix((int *) pHeight, Cols, x, y, 0);
			PutPix(pHeight, ColsTF, x + IMAGE_PADDING, index + 1, (HEIGHT_TYPE)0);

			// Set west and east caps
			w = GetPix(pHCap, Cols, x, y);
			if (x == 0) {
				PutPix(pWestCap, ColsTF, x + IMAGE_PADDING, index + 1, (short)0);
			}
			else if (x < Cols - 1) {
				PutPix(pWestCap, ColsTF, x + IMAGE_PADDING, index + 1, w);
//				PutPix(pEastCap, ColsTF, (x + IMAGE_PADDING) - 1, index + 1, w);
				PutPix(pEastCap, ColsTF, (x + IMAGE_PADDING) - 1, index, w);
			}
			else {    // x = Cols-1
				PutPix(pWestCap, ColsTF, x + IMAGE_PADDING, index + 1, w);
//				PutPix(pEastCap, ColsTF, (x + IMAGE_PADDING) - 1, index + 1, w);
				PutPix(pEastCap, ColsTF, (x + IMAGE_PADDING) - 1, index, w);
				PutPix(pEastCap, ColsTF, x + IMAGE_PADDING, index + 1, (short)0);
			}

			// Set north and south caps
			w = GetPix(pVCap, Cols, x, y);
			if (y == 0) {
				PutPix(pNorthCap, ColsTF, x + IMAGE_PADDING, index + 1, (short)0);
			}
			else if (y < Rows - 1) {
				PutPix(pNorthCap, ColsTF, x + IMAGE_PADDING, index + 1, w);
				PutPix(pSouthCap, ColsTF, x + IMAGE_PADDING, (index + 1) - 1, w);
			}
			else {
				PutPix(pNorthCap, ColsTF, x + IMAGE_PADDING, index + 1, w);
				PutPix(pSouthCap, ColsTF, x + IMAGE_PADDING, (index + 1) - 1, w);
				PutPix(pSouthCap, ColsTF, x + IMAGE_PADDING, index + 1, (short)0);
			}

			index++;
		}	
	}
}


void AVX2PushRelabel_Init_TF_opt(short * pWeight,
	short * pHCap,
	short * pVCap,
	int Rows,
	int Cols,
	int ColsTF,
	short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap)
{
	short w;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);	// Must have -1 to prevent overflow, some calculation needs HEIGHT_MAX+1

	// Set pHeight buffer to zero. Should be just a single memset!!!
	int FrameWidthTF = Cols;
	int FrameHeightTF = Rows + Cols;
	memset(pHeight, 0, sizeof(HEIGHT_TYPE) * FrameWidthTF*FrameHeightTF);

	int s_y = 0;
	for (; s_y < (Rows & ~SLICE_H); s_y += SLICE_H) {

		// Do slices horizontaly
		int s_x=0;
		for (; s_x < (Cols & ~SLICE_W); s_x += SLICE_W)

			for (int y = s_y; y < s_y+SLICE_H; y++) {
				int index = y;
				for (int x=s_x; x < s_x+SLICE_W; ++x, ++index)
				{
					w = GetPix(pWeight, Cols, x, y);
					PutPix(pExcessFlow, ColsTF, x + IMAGE_PADDING, index + 1, w);    // w>0 if a cap from source exists, w<0 if cap from sink exists. ExcessFlow = w.
				}
			}

		// Finish last slice on the right if needed
		for (int y = s_y; y < s_y+SLICE_H; y++) {
			int index = y;
			for (int x=s_x; x < Cols; ++x, ++index)
			{
				w = GetPix(pWeight, Cols, x, y);
				PutPix(pExcessFlow, ColsTF, x + IMAGE_PADDING, index + 1, w);    // w>0 if a cap from source exists, w<0 if cap from sink exists. ExcessFlow = w.
			}
		}

	}

	/// Finish bottom part of the image
	for (int y = s_y; y < Rows; y++) {
		// Index for destination
		int index = y;
		for (int x = 0; x < Cols; x++, ++index) {
			// Set excess flow
			w = GetPix(pWeight, Cols, x, y);
			PutPix(pExcessFlow, ColsTF, x + IMAGE_PADDING, index + 1, w);    // w>0 if a cap from source exists, w<0 if cap from sink exists. ExcessFlow = w.
		}
	}


	// Baseline code
    /*for (int y = 0; y < Rows; y++) {
        // Index for destination
        int index = y;

        for (int x = 0; x < Cols; x++) {
            // Set excess flow
            w = GetPix(pWeight, Cols, x, y);
            PutPix(pExcessFlow, ColsTF, x + IMAGE_PADDING, index + 1, w);    // w>0 if a cap from source exists, w<0 if cap from sink exists. ExcessFlow = w.
		}}*/

	for (int y = 0; y < Rows; y++) {
		// Index for destination
		int index = y;

		/*for (int x = 0; x < Cols; x++) {
			// Set excess flow
			w = GetPix(pWeight, Cols, x, y);
			PutPix(pExcessFlow, ColsTF, x + IMAGE_PADDING, index + 1, w);    // w>0 if a cap from source exists, w<0 if cap from sink exists. ExcessFlow = w.

			++index;
		}*/

		index = y;
		for (int x = 0; x < Cols; x++) {
			// Set west and east caps
			w = GetPix(pHCap, Cols, x, y);
			if (x == 0) {
				PutPix(pWestCap, ColsTF, x + IMAGE_PADDING, index + 1, (short)0);
			}
			else if (x < Cols - 1) {
				PutPix(pWestCap, ColsTF, x + IMAGE_PADDING, index + 1, w);
				PutPix(pEastCap, ColsTF, (x + IMAGE_PADDING) - 1, index + 1, w);
			}
			else {    // x = Cols-1
				PutPix(pWestCap, ColsTF, x + IMAGE_PADDING, index + 1, w);
				PutPix(pEastCap, ColsTF, (x + IMAGE_PADDING) - 1, index + 1, w);
				PutPix(pEastCap, ColsTF, x + IMAGE_PADDING, index + 1, (short)0);
			}
			++index;
		}

		index = y;
		for (int x = 0; x < Cols; x++) {
			// Set north and south caps
			w = GetPix(pVCap, Cols, x, y);
			if (y == 0) {
				PutPix(pNorthCap, ColsTF, x + IMAGE_PADDING, index + 1, (short)0);
			}
			else if (y < Rows - 1) {
				PutPix(pNorthCap, ColsTF, x + IMAGE_PADDING, index + 1, w);
				PutPix(pSouthCap, ColsTF, x + IMAGE_PADDING, (index + 1) - 1, w);
			}
			else {
				PutPix(pNorthCap, ColsTF, x + IMAGE_PADDING, index + 1, w);
				PutPix(pSouthCap, ColsTF, x + IMAGE_PADDING, (index + 1) - 1, w);
				PutPix(pSouthCap, ColsTF, x + IMAGE_PADDING, index + 1, (short)0);
			}

			index++;
		}
	}
}

/* x, and y should be divided by 16
   Rows and Cols are the original dimenstion of the image 
   checks if the 16x16 block is located in the out of bound image*/
inline bool IsBLOCKOOB(int y, int x, int Rows, int Cols)
{
	assert(x >= 0 && y >= 0);
	assert(y % 16 == 0);
	assert(x % 16 == 0);

	int col_id = x / 16;
	int row_id = y / 16;
	if ((row_id > col_id) && (row_id <= (col_id + Rows / 16)) )
		return false;
	return true;

	//check bottom traingle 
	/*int start_lower_triangle = (((Rows + 15) >> LOG_SIZE) + (((Rows + 15) & (SIDE_SQUARE - 1)) != 0)) << LOG_SIZE;
	int valid_block = y - start_lower_triangle + 16;
	if (valid_block > x)
		return true;	
	//check top triangle
	int last_valid_block = y + 16;
	if (last_valid_block <= x)
			return true;
	return false;*/
}


///// Rebuild the list of active blocks
// Should go through original rows configuration for efficient SMT tiling
void AVX2ActiveBlockTF16x16(short * pExcessFlow, HEIGHT_TYPE * pHeight, int Rows, int Cols, int RowsTF, int ColsTF, RelabelBlock* pTableBlock, 	active_list &a_list)
{

	char ActiveNodeCount;
	int blockInRow = Cols >> LOG_SIZE;
	__m256i currentHeightReg, excessFlowReg, cmpCurrHeight, cmpExcessFlow;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i threshReg = _mm256_set1_epi16(-100);
	__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	int lastActiveBlock = 0;
	pTableBlock[lastActiveBlock].nextActiveBlock = -1;
	int mask;
	int currentBlock = 0;
	int activeBlocks = 0;

	for (int y = 1; y < (RowsTF-1); y += SIDE_SQUARE) {
		for (int x = IMAGE_PADDING; x < (ColsTF - 1); x += SIDE_SQUARE) {
			//passing in unpadded transformed cordinants and the original Rows/Cols of the image 
			if (IsBLOCKOOB(y - 1, x - IMAGE_PADDING, Rows, Cols) == true)		// Check if (u,v) is an OOB pixel
				continue;
			ActiveNodeCount = 0;
			for (int i = y; (i < y + SIDE_SQUARE) && (ActiveNodeCount == 0); i++) {
				__m256i active_reg;
				currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + i*ColsTF + x));
				excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + i*ColsTF + x));
				cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
				cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, threshReg);
				cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
				active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);
				mask = _mm256_movemask_epi8(active_reg);
				if (mask != 0) {
					ActiveNodeCount = 1;
				}
			}
			pTableBlock[currentBlock + 1].row = y;
			pTableBlock[currentBlock + 1].col = x;
			if (ActiveNodeCount > 0) {
				a_list.push_back( active_block_t(y,x) );
				pTableBlock[lastActiveBlock].nextActiveBlock = (currentBlock + 1) - lastActiveBlock;
				lastActiveBlock = currentBlock + 1;
			}
			pTableBlock[currentBlock + 1].nextActiveBlock = -1;
			currentBlock++;
		}
	}

}


void AVX2Model_GraphCut_RelabelTF16x16(short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	RelabelBlock* pTableBlock,
	int Rows,
	int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i zeroReg = _mm256_setzero_si256();
	__m256i	heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	__m256i	oneReg = _mm256_set1_epi16(1);

	int prevIndex = 0;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
	while (indexActiveBlock != -1) {
		int currentBlock = prevIndex + indexActiveBlock;
		int row = pTableBlock[currentBlock].row;
		int col = pTableBlock[currentBlock].col;
		prevIndex = currentBlock;
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
		for (int y = row; y < (row + SIDE_SQUARE); y++) {
			__m256i cmpExcessFlow, currentHeightReg, cmpCurrHeight, active_reg;
			/* Check for active node */
			cmpExcessFlow = _mm256_loadu_si256((__m256i const*)(pExcessFlow + Cols*y + col));
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col));
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(cmpExcessFlow, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);
			int mask = _mm256_movemask_epi8(active_reg);
			if (mask != 0) {
				__m256i capReg, cmpCap;
				__m256i newHeight = heightMax;

				/*North Neighbor*/
				__m256i northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col));
				capReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + Cols*y + col));
				northHeightReg = _mm256_add_epi16(northHeightReg, oneReg);
				cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg);
				northHeightReg = _mm256_blendv_epi8(heightMax, northHeightReg, cmpCap);
				newHeight = _mm256_min_epu16(heightMax, northHeightReg);

				/*South Neighbor*/
				__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col));
				capReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + Cols*y + col));
				southHeightReg = _mm256_add_epi16(southHeightReg, oneReg);
				cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg);
				southHeightReg = _mm256_blendv_epi8(heightMax, southHeightReg, cmpCap);
				newHeight = _mm256_min_epu16(newHeight, southHeightReg);

				/*East Neighbor*/
				__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col + 1));
				capReg = _mm256_loadu_si256((__m256i const*)(pEastCap + Cols*y + col));
				eastHeightReg = _mm256_add_epi16(eastHeightReg, oneReg);
				cmpCap = _mm256_cmpgt_epi16((capReg), zeroReg);
				eastHeightReg = _mm256_blendv_epi8(heightMax, eastHeightReg, cmpCap);
				newHeight = _mm256_min_epu16(newHeight, eastHeightReg);
				
				/*West Neighbor*/
				__m256i westHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col - 1));
				capReg = _mm256_loadu_si256((__m256i const*)(pWestCap + Cols*y + col));
				westHeightReg = _mm256_add_epi16(westHeightReg, oneReg);
				cmpCap = _mm256_cmpgt_epi16((capReg), zeroReg);
				westHeightReg = _mm256_blendv_epi8(heightMax, westHeightReg, cmpCap);
				newHeight = _mm256_min_epu16(newHeight, westHeightReg);

				newHeight = _mm256_blendv_epi8(currentHeightReg,newHeight, active_reg);

				// save the height
				_mm256_storeu_si256((__m256i *)(pHeight + Cols*y + col), newHeight);
			}
		}
	}
}

void AVX2Model_GraphCut_RelabelTF16x16_single(short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	active_block_t &pBlock,
	int Rows,
	int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i zeroReg = _mm256_setzero_si256();
	__m256i	heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	__m256i	oneReg = _mm256_set1_epi16(1);

	//int currentBlock = prevIndex + indexActiveBlock;
	int row = pBlock.row; // pTableBlock[currentBlock].row;
	int col = pBlock.col; // pTableBlock[currentBlock].col;
	for (int y = row; y < (row + SIDE_SQUARE); y++) {
		__m256i cmpExcessFlow, currentHeightReg, cmpCurrHeight, active_reg;
		/* Check for active node */
		cmpExcessFlow = _mm256_loadu_si256((__m256i const*)(pExcessFlow + Cols*y + col));
		currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col));
		cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
		cmpExcessFlow = _mm256_cmpgt_epi16(cmpExcessFlow, zeroReg);
		cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
		active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);
		int mask = _mm256_movemask_epi8(active_reg);
		if (mask != 0) {
			__m256i capReg, cmpCap;
			__m256i newHeight = heightMax;

			/*North Neighbor*/
			__m256i northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col));
			capReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + Cols*y + col));
			northHeightReg = _mm256_add_epi16(northHeightReg, oneReg);
			cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg);
			northHeightReg = _mm256_blendv_epi8(heightMax, northHeightReg, cmpCap);
			newHeight = _mm256_min_epu16(heightMax, northHeightReg);

			/*South Neighbor*/
			__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col));
			capReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + Cols*y + col));
			southHeightReg = _mm256_add_epi16(southHeightReg, oneReg);
			cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg);
			southHeightReg = _mm256_blendv_epi8(heightMax, southHeightReg, cmpCap);
			newHeight = _mm256_min_epu16(newHeight, southHeightReg);

			/*East Neighbor*/
			__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col + 1));
			capReg = _mm256_loadu_si256((__m256i const*)(pEastCap + Cols*y + col));
			eastHeightReg = _mm256_add_epi16(eastHeightReg, oneReg);
			cmpCap = _mm256_cmpgt_epi16((capReg), zeroReg);
			eastHeightReg = _mm256_blendv_epi8(heightMax, eastHeightReg, cmpCap);
			newHeight = _mm256_min_epu16(newHeight, eastHeightReg);
				
			/*West Neighbor*/
			__m256i westHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col - 1));
			capReg = _mm256_loadu_si256((__m256i const*)(pWestCap + Cols*y + col));
			westHeightReg = _mm256_add_epi16(westHeightReg, oneReg);
			cmpCap = _mm256_cmpgt_epi16((capReg), zeroReg);
			westHeightReg = _mm256_blendv_epi8(heightMax, westHeightReg, cmpCap);
			newHeight = _mm256_min_epu16(newHeight, westHeightReg);

			newHeight = _mm256_blendv_epi8(currentHeightReg,newHeight, active_reg);

			// save the height
			_mm256_storeu_si256((__m256i *)(pHeight + Cols*y + col), newHeight);
		}
	}
}

void AVX2Model_Global_RelabelTF16x16(//short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	RelabelBlock* pTableBlock,
	int Rows,
	int Cols,
	active_list &a_list,
	short * pExcessFlow, int RowsTF, int ColsTF)
{

	{

	int y;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	HEIGHT_TYPE NewH = 0;
	unsigned int AnyValuesProcessed = 0;
	__m256i currentHeightReg;
	__m256i zeroReg = _mm256_setzero_si256();
	__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	__m256i oneReg = _mm256_set1_epi16(1);

	/* Init Height */
	// TODO: validate the code
	#pragma omp for
	for(int i=0; i<a_list.size(); ++i)
	{
		int row = a_list[i].row;
		int col = a_list[i].col;
		for (y = row; y < (row + SIDE_SQUARE); y++) {
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col));
			__m256i cmpHeightreg = _mm256_cmpeq_epi16(currentHeightReg, zeroReg);
			currentHeightReg = _mm256_andnot_si256(cmpHeightreg, heightMax);
			_mm256_storeu_si256((__m256i *)(pHeight + Cols*y + col), currentHeightReg);
		}
	}

	/* Global relabel */
	int numIter = 0;
	do {
		#pragma omp for
		for(int i=0; i<a_list.size(); ++i)
		{
			int row = a_list[i].row;
			int col = a_list[i].col;

			__m256i mask = _mm256_setzero_si256();

			// 16x16 block processing
			for (y = row; y < (row + SIDE_SQUARE); y++) {
				__m256i capReg, newHeight, cmpCap;
				currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col));
				newHeight = currentHeightReg;
				/*North Neighbor*/
				__m256i northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col));
				capReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + Cols*y + col));
				northHeightReg = _mm256_add_epi16(northHeightReg, oneReg);
				cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg);
				northHeightReg = _mm256_blendv_epi8(currentHeightReg, northHeightReg, cmpCap);
				newHeight = _mm256_min_epu16(currentHeightReg, northHeightReg);
				/*South Neighbor*/
				__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col));
				capReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + Cols*y + col));
				southHeightReg = _mm256_add_epi16(southHeightReg, oneReg);
				cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg);
				southHeightReg = _mm256_blendv_epi8(newHeight, southHeightReg, cmpCap);
				newHeight = _mm256_min_epu16(newHeight, southHeightReg);
				/*East Neighbor*/
				__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y+1) + col + 1));
				capReg = _mm256_loadu_si256((__m256i const*)(pEastCap + Cols*y + col));
				eastHeightReg = _mm256_add_epi16(eastHeightReg, oneReg);
				cmpCap = _mm256_cmpgt_epi16((capReg), zeroReg);
				eastHeightReg = _mm256_blendv_epi8(newHeight, eastHeightReg, cmpCap);
				newHeight = _mm256_min_epu16(newHeight, eastHeightReg);
				
				/*West Neighbor*/
				__m256i westHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col - 1));
				capReg = _mm256_loadu_si256((__m256i const*)(pWestCap + Cols*y + col));
				westHeightReg = _mm256_add_epi16(westHeightReg, oneReg);
				cmpCap = _mm256_cmpgt_epi16((capReg), zeroReg);
				westHeightReg = _mm256_blendv_epi8(newHeight, westHeightReg, cmpCap);
				newHeight = _mm256_min_epu16(newHeight, westHeightReg);
				__m256i maskHeightCount = _mm256_cmpeq_epi16(newHeight, currentHeightReg);
				mask = _mm256_or_si256(mask, maskHeightCount);

				_mm256_storeu_si256((__m256i *)(pHeight + Cols*y + col), newHeight);
			}
			// End 16x16

			AnyValuesProcessed |= _mm256_movemask_epi8(mask);

		} // Can be parallel

	} while (++numIter < 32 && AnyValuesProcessed != 0);

	} ////////////// END GLOBAL RELABLE

	#pragma omp single
	{ ////////////// BUILD LIST OF ACTIVE BLOCKS

	// Do Active nodes and rebuild the list of active nodes
	char ActiveNodeCount;
	int blockInRow = Cols >> LOG_SIZE;
	__m256i currentHeightReg, excessFlowReg, cmpCurrHeight, cmpExcessFlow;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i threshReg = _mm256_set1_epi16(-100);
	__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	int lastActiveBlock = 0;
	pTableBlock[lastActiveBlock].nextActiveBlock = -1;
	int mask;
	int currentBlock = 0;
	int activeBlocks = 0;

	a_list.clear();

	for (int y = 1; y < (RowsTF-1); y += SIDE_SQUARE) {
		for (int x = IMAGE_PADDING; x < (ColsTF - 1); x += SIDE_SQUARE) {
			//passing in unpadded transformed cordinants and the original Rows/Cols of the image 
			if (IsBLOCKOOB(y - 1, x - IMAGE_PADDING, Rows, Cols) == true)		// Check if (u,v) is an OOB pixel
				continue;
			ActiveNodeCount = 0;
			for (int i = y; (i < y + SIDE_SQUARE) && (ActiveNodeCount == 0); i++) {
				__m256i active_reg;
				currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + i*ColsTF + x));
				excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + i*ColsTF + x));
				cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
				cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, threshReg);
				cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
				active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);
				mask = _mm256_movemask_epi8(active_reg);
				if (mask != 0) {
					ActiveNodeCount = 1;
				}
			}
			pTableBlock[currentBlock + 1].row = y;
			pTableBlock[currentBlock + 1].col = x;
			if (ActiveNodeCount > 0) {
				a_list.push_back( active_block_t(y,x) );
				pTableBlock[lastActiveBlock].nextActiveBlock = (currentBlock + 1) - lastActiveBlock;
				lastActiveBlock = currentBlock + 1;
			}
			pTableBlock[currentBlock + 1].nextActiveBlock = -1;
			currentBlock++;
		}
	}

	} //////////////// END BUILDING LIST OF ACTIVE BLOCKS
}

void AVX2Model_GraphCut_Vertical_PushTF16x16(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pNorthCap, short * pSouthCap, 
											 ActiveBlocks &blocks, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i zeroReg = _mm256_setzero_si256();
	__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	__m256i oneReg = _mm256_set1_epi16(1);
	__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight;
	__m256i northCapReg, southCapReg, northExcessFlowReg, southExcessFlowReg, northHeightReg, cmpHeight, newExcessFlowReg, newNorthExcessFlowReg, flowReg;

	for(auto block: blocks)
	{
		int row = block.row;
		int col = block.col;

		int y = Cols*row;
		excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + col));
		currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + col));
		northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y - Cols + col));
		northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + y + col));
		northExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y - Cols + col));
		southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + y - Cols + col));
		int lastRow = y + (Cols<<LOG_SIZE);
		for (; y < lastRow; y += Cols) {
			/* Check for active node */
			__m256i active_reg;
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);

			/*North Neighbor*/
			northHeightReg = _mm256_add_epi16(northHeightReg, oneReg);
			cmpHeight = _mm256_cmpeq_epi16(northHeightReg, currentHeightReg);
			flowReg = _mm256_min_epi16(northCapReg, excessFlowReg);
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			newNorthExcessFlowReg = _mm256_add_epi16(northExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			northExcessFlowReg = _mm256_blendv_epi8(northExcessFlowReg, newNorthExcessFlowReg, cmpHeight);
			__m256i newNorthCapReg = _mm256_sub_epi16(northCapReg, flowReg);
			__m256i newSouthCapReg = _mm256_add_epi16(southCapReg, flowReg);
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col), northExcessFlowReg);
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pNorthCap + y + col), northCapReg);
			_mm256_storeu_si256((__m256i*)(pSouthCap + y - Cols + col), southCapReg);
		
			/*South Neighbor*/
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + Cols + col));
			southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + y + col));
			cmpHeight = _mm256_cmpeq_epi16(_mm256_add_epi16(southHeightReg, oneReg), currentHeightReg);
			flowReg = _mm256_min_epi16(southCapReg, excessFlowReg);
			cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight);
			southExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + Cols + col));
			northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + y + Cols + col));
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			__m256i newSouthExcessFlowReg = _mm256_add_epi16(southExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			newSouthCapReg = _mm256_sub_epi16(southCapReg, flowReg);
			southExcessFlowReg = _mm256_blendv_epi8(southExcessFlowReg, newSouthExcessFlowReg, cmpHeight);
			newNorthCapReg = _mm256_add_epi16(northCapReg, flowReg);
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight);
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight);
			northHeightReg = currentHeightReg;
			northExcessFlowReg = excessFlowReg;
			excessFlowReg = southExcessFlowReg;
			currentHeightReg = southHeightReg;
		}
		if (y  < (Rows*Cols)) {
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col), northExcessFlowReg);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y + col), excessFlowReg);
			_mm256_storeu_si256((__m256i*)(pSouthCap + y - Cols + col), southCapReg);
			_mm256_storeu_si256((__m256i*)(pNorthCap + y + col), northCapReg);
		}
		else {
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col), excessFlowReg);
		}
	}

}

void AVX2Model_GraphCut_Horizontal_PushFT16x16(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pWestCap, short * pEastCap, 
											   ActiveBlocks &blocks, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i zeroReg = _mm256_setzero_si256();
	__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	__m256i oneReg = _mm256_set1_epi16(1);
	__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight;
	__m256i westCapReg, eastCapReg, westExcessFlowReg, eastExcessFlowReg, westHeightReg, cmpHeight, newExcessFlowReg, newWestExcessFlowReg, flowReg;

	/*TODO: Fix the Store Forward Load*/
	for(auto block: blocks)
	{
		int row = block.row;
		int col = block.col;

		int y = Cols*row;
		int lastRow = y + (Cols << LOG_SIZE);
		for (; y < lastRow; y += Cols) {
			/* Check for active node */
			__m256i active_reg;
			/*current pixel*/
			excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + col));	// Load-to-store penalty because of the store with 2 byte offset below
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + col));
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);

			/*West Neighbor*/
			westHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y - Cols + col - 1));
			westCapReg = _mm256_loadu_si256((__m256i const*)(pWestCap + y + col));			// Load-to-store penalty because of the store with 2 byte offset below
			westHeightReg = _mm256_add_epi16(westHeightReg, oneReg);
			cmpHeight = _mm256_cmpeq_epi16(westHeightReg, currentHeightReg);
			flowReg = _mm256_min_epi16(westCapReg, excessFlowReg);
			westExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y - Cols + col - 1));
			eastCapReg = _mm256_loadu_si256((__m256i const*)(pEastCap + y - Cols + col - 1));
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			newWestExcessFlowReg = _mm256_add_epi16(westExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			westExcessFlowReg = _mm256_blendv_epi8(westExcessFlowReg, newWestExcessFlowReg, cmpHeight);
			__m256i newWestCapReg = _mm256_sub_epi16(westCapReg, flowReg);
			__m256i newEastCapReg = _mm256_add_epi16(eastCapReg, flowReg);
			westCapReg = _mm256_blendv_epi8(westCapReg, newWestCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col - 1), westExcessFlowReg);
			eastCapReg = _mm256_blendv_epi8(eastCapReg, newEastCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pWestCap + y + col), westCapReg);
			_mm256_storeu_si256((__m256i*)(pEastCap + y - Cols + col - 1), eastCapReg);

			/*East Neighbor*/
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + Cols + col + 1));
			eastCapReg = _mm256_loadu_si256((__m256i const*)(pEastCap + y + col));
			cmpHeight = _mm256_cmpeq_epi16(_mm256_add_epi16(eastHeightReg, oneReg), currentHeightReg);
			flowReg = _mm256_min_epi16(eastCapReg, excessFlowReg);
			cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight);
			eastExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + Cols + col + 1));
			westCapReg = _mm256_loadu_si256((__m256i const*)(pWestCap + y + Cols + col + 1));
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			__m256i newEastExcessFlowReg = _mm256_add_epi16(eastExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			newEastCapReg = _mm256_sub_epi16(eastCapReg, flowReg);
			eastExcessFlowReg = _mm256_blendv_epi8(eastExcessFlowReg, newEastExcessFlowReg, cmpHeight);
			newWestCapReg = _mm256_add_epi16(westCapReg, flowReg);
			eastCapReg = _mm256_blendv_epi8(eastCapReg, newEastCapReg, cmpHeight);
			westCapReg = _mm256_blendv_epi8(westCapReg, newWestCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y + col), excessFlowReg);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y + Cols + col + 1), eastExcessFlowReg);	// Store one short offset from the next load
			_mm256_storeu_si256((__m256i*)(pEastCap + y + col), eastCapReg);
			_mm256_storeu_si256((__m256i*)(pWestCap + y + Cols + col + 1), westCapReg);				// Store one short offset from the next load
		}
	}

}

void AVX2Model_GraphCut_PushTF16x16(short * pExcessFlow, HEIGHT_TYPE * pHeight, 
											 short * pWestCap, short * pEastCap, short * pNorthCap, short * pSouthCap,
											 ActiveBlocks &blocks, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

	// Go through all active blocks
	for(auto block: blocks)
	{
		int row = block.row;
		int col = block.col;

		// Do vertical push
		{
		__m256i zeroReg = _mm256_setzero_si256();
		__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
		__m256i oneReg = _mm256_set1_epi16(1);
		__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight;
		__m256i northCapReg, southCapReg, northExcessFlowReg, southExcessFlowReg, northHeightReg, cmpHeight, newExcessFlowReg, newNorthExcessFlowReg, flowReg;

		int y = Cols*row;
		excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + col));
		currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + col));
		northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y - Cols + col));
		northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + y + col));
		northExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y - Cols + col));
		southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + y - Cols + col));
		int lastRow = y + (Cols<<LOG_SIZE);
		for (; y < lastRow; y += Cols) {
			/* Check for active node */
			__m256i active_reg;
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);

			/*North Neighbor*/
			northHeightReg = _mm256_add_epi16(northHeightReg, oneReg);
			cmpHeight = _mm256_cmpeq_epi16(northHeightReg, currentHeightReg);
			flowReg = _mm256_min_epi16(northCapReg, excessFlowReg);
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			newNorthExcessFlowReg = _mm256_add_epi16(northExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			northExcessFlowReg = _mm256_blendv_epi8(northExcessFlowReg, newNorthExcessFlowReg, cmpHeight);
			__m256i newNorthCapReg = _mm256_sub_epi16(northCapReg, flowReg);
			__m256i newSouthCapReg = _mm256_add_epi16(southCapReg, flowReg);
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col), northExcessFlowReg);
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pNorthCap + y + col), northCapReg);
			_mm256_storeu_si256((__m256i*)(pSouthCap + y - Cols + col), southCapReg);
		
			/*South Neighbor*/
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + Cols + col));
			southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + y + col));
			cmpHeight = _mm256_cmpeq_epi16(_mm256_add_epi16(southHeightReg, oneReg), currentHeightReg);
			flowReg = _mm256_min_epi16(southCapReg, excessFlowReg);
			cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight);
			southExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + Cols + col));
			northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + y + Cols + col));
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			__m256i newSouthExcessFlowReg = _mm256_add_epi16(southExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			newSouthCapReg = _mm256_sub_epi16(southCapReg, flowReg);
			southExcessFlowReg = _mm256_blendv_epi8(southExcessFlowReg, newSouthExcessFlowReg, cmpHeight);
			newNorthCapReg = _mm256_add_epi16(northCapReg, flowReg);
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight);
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight);
			northHeightReg = currentHeightReg;
			northExcessFlowReg = excessFlowReg;
			excessFlowReg = southExcessFlowReg;
			currentHeightReg = southHeightReg;
		}
		if (y  < (Rows*Cols)) {
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col), northExcessFlowReg);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y + col), excessFlowReg);
			_mm256_storeu_si256((__m256i*)(pSouthCap + y - Cols + col), southCapReg);
			_mm256_storeu_si256((__m256i*)(pNorthCap + y + col), northCapReg);
		}
		else {
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col), excessFlowReg);
		}

		} ///// End vertical push block

		// Do horizontal push
		{
		__m256i zeroReg = _mm256_setzero_si256();
		__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
		__m256i oneReg = _mm256_set1_epi16(1);
		__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight;
		__m256i westCapReg, eastCapReg, westExcessFlowReg, eastExcessFlowReg, westHeightReg, cmpHeight, newExcessFlowReg, newWestExcessFlowReg, flowReg;

		int y = Cols*row;
		int lastRow = y + (Cols << LOG_SIZE);
		for (; y < lastRow; y += Cols) {
			/* Check for active node */
			__m256i active_reg;
			/*current pixel*/
			excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + col));	// Load-to-store penalty because of the store with 2 byte offset below
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + col));
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);

			/*West Neighbor*/
			westHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y - Cols + col - 1));
			westCapReg = _mm256_loadu_si256((__m256i const*)(pWestCap + y + col));			// Load-to-store penalty because of the store with 2 byte offset below
			westHeightReg = _mm256_add_epi16(westHeightReg, oneReg);
			cmpHeight = _mm256_cmpeq_epi16(westHeightReg, currentHeightReg);
			flowReg = _mm256_min_epi16(westCapReg, excessFlowReg);
			westExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y - Cols + col - 1));
			eastCapReg = _mm256_loadu_si256((__m256i const*)(pEastCap + y - Cols + col - 1));
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			newWestExcessFlowReg = _mm256_add_epi16(westExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			westExcessFlowReg = _mm256_blendv_epi8(westExcessFlowReg, newWestExcessFlowReg, cmpHeight);
			__m256i newWestCapReg = _mm256_sub_epi16(westCapReg, flowReg);
			__m256i newEastCapReg = _mm256_add_epi16(eastCapReg, flowReg);
			westCapReg = _mm256_blendv_epi8(westCapReg, newWestCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col - 1), westExcessFlowReg);
			eastCapReg = _mm256_blendv_epi8(eastCapReg, newEastCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pWestCap + y + col), westCapReg);
			_mm256_storeu_si256((__m256i*)(pEastCap + y - Cols + col - 1), eastCapReg);

			/*East Neighbor*/
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + Cols + col + 1));
			eastCapReg = _mm256_loadu_si256((__m256i const*)(pEastCap + y + col));
			cmpHeight = _mm256_cmpeq_epi16(_mm256_add_epi16(eastHeightReg, oneReg), currentHeightReg);
			flowReg = _mm256_min_epi16(eastCapReg, excessFlowReg);
			cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight);
			eastExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + Cols + col + 1));
			westCapReg = _mm256_loadu_si256((__m256i const*)(pWestCap + y + Cols + col + 1));
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			__m256i newEastExcessFlowReg = _mm256_add_epi16(eastExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			newEastCapReg = _mm256_sub_epi16(eastCapReg, flowReg);
			eastExcessFlowReg = _mm256_blendv_epi8(eastExcessFlowReg, newEastExcessFlowReg, cmpHeight);
			newWestCapReg = _mm256_add_epi16(westCapReg, flowReg);
			eastCapReg = _mm256_blendv_epi8(eastCapReg, newEastCapReg, cmpHeight);
			westCapReg = _mm256_blendv_epi8(westCapReg, newWestCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y + col), excessFlowReg);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y + Cols + col + 1), eastExcessFlowReg);	// Store one short offset from the next load
			_mm256_storeu_si256((__m256i*)(pEastCap + y + col), eastCapReg);
			_mm256_storeu_si256((__m256i*)(pWestCap + y + Cols + col + 1), westCapReg);				// Store one short offset from the next load
		}
		} ///// End horizontal push

	}

}


template<typename T> __m256i _mm256_insert_left(__m256i left, T right) {
	size_t VS = sizeof right; // How many bytes to shift
	if(VS == 2) // support 16bit inserts
	{
		// Move 7th element through the border
		__m256i nr = _mm256_permute4x64_epi64(left, 0x55);
		// nr = _mm256_slli_epi64(nr, 64-16);
		nr = _mm256_shufflelo_epi16(nr, 0xFF);
		//nr = _mm256_srli_si256(nr, 6);
		nr = _mm256_insert_epi16(nr, static_cast<__int16>(right), 0);

		// Shift full "reg" by VS bytes to left
		left = _mm256_slli_si256(left, 2);
		left = _mm256_blend_epi16(left, nr, 0x101);


		return left;
	}
	else
		return _mm256_setzero_si256();
}


__forceinline void AVX2Model_GraphCut_PushTF16x16_single(short * pExcessFlow, HEIGHT_TYPE * pHeight, 
											 short * pWestCap, short * pEastCap, short * pNorthCap, short * pSouthCap,
											 active_block_t &block, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

	// Go through all active blocks
	// for(auto block: blocks)
	{
		int row = block.row;
		int col = block.col;

		// Do vertical push
		{
		__m256i zeroReg = _mm256_setzero_si256();
		__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
		__m256i oneReg = _mm256_set1_epi16(1);
		__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight;
		__m256i northCapReg, southCapReg, northExcessFlowReg, southExcessFlowReg, northHeightReg, cmpHeight, newExcessFlowReg, newNorthExcessFlowReg, flowReg;

		int y = Cols*row;
		excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + col));
		currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + col));
		northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y - Cols + col));
		northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + y + col));
		northExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y - Cols + col));
		southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + y - Cols + col));
		int lastRow = y + (Cols<<LOG_SIZE);
		for (; y < lastRow; y += Cols) {
			/* Check for active node */
			__m256i active_reg;
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);

			/*North Neighbor*/
			northHeightReg = _mm256_add_epi16(northHeightReg, oneReg);
			cmpHeight = _mm256_cmpeq_epi16(northHeightReg, currentHeightReg);
			flowReg = _mm256_min_epi16(northCapReg, excessFlowReg);
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			newNorthExcessFlowReg = _mm256_add_epi16(northExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			northExcessFlowReg = _mm256_blendv_epi8(northExcessFlowReg, newNorthExcessFlowReg, cmpHeight);
			__m256i newNorthCapReg = _mm256_sub_epi16(northCapReg, flowReg);
			__m256i newSouthCapReg = _mm256_add_epi16(southCapReg, flowReg);
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col), northExcessFlowReg);
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pNorthCap + y + col), northCapReg);
			_mm256_storeu_si256((__m256i*)(pSouthCap + y - Cols + col), southCapReg);
		
			/*South Neighbor*/
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + Cols + col));
			southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + y + col));
			cmpHeight = _mm256_cmpeq_epi16(_mm256_add_epi16(southHeightReg, oneReg), currentHeightReg);
			flowReg = _mm256_min_epi16(southCapReg, excessFlowReg);
			cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight);
			southExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + Cols + col));
			northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + y + Cols + col));
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			__m256i newSouthExcessFlowReg = _mm256_add_epi16(southExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			newSouthCapReg = _mm256_sub_epi16(southCapReg, flowReg);
			southExcessFlowReg = _mm256_blendv_epi8(southExcessFlowReg, newSouthExcessFlowReg, cmpHeight);
			newNorthCapReg = _mm256_add_epi16(northCapReg, flowReg);
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight);
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight);
			northHeightReg = currentHeightReg;
			northExcessFlowReg = excessFlowReg;
			excessFlowReg = southExcessFlowReg;
			currentHeightReg = southHeightReg;
		}
		if (y  < (Rows*Cols)) {
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col), northExcessFlowReg);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y + col), excessFlowReg);
			_mm256_storeu_si256((__m256i*)(pSouthCap + y - Cols + col), southCapReg);
			_mm256_storeu_si256((__m256i*)(pNorthCap + y + col), northCapReg);
		}
		else {
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col), excessFlowReg);
		}

		} ///// End vertical push block

		// Do horizontal push
		{
		__m256i zeroReg = _mm256_setzero_si256();
		__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
		__m256i oneReg = _mm256_set1_epi16(1);
		__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight;
		__m256i westCapReg, eastCapReg, westExcessFlowReg, eastExcessFlowReg, westHeightReg, cmpHeight, newExcessFlowReg, newWestExcessFlowReg, flowReg;

		int y = Cols*row;
		int lastRow = y + (Cols << LOG_SIZE);
		eastExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + col + 1));
		westCapReg        = _mm256_loadu_si256((__m256i const*)(pWestCap    + y + col + 1));
		eastCapReg        = _mm256_loadu_si256((__m256i const*)(pEastCap    + y + col - 1));
		__m256i excessFlowReg__ = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y - Cols + col));

		for (; y < lastRow; y += Cols) {
			/* Check for active node */
			__m256i active_reg;
			/*current pixel*/
			excessFlowReg = _mm256_insert_left(eastExcessFlowReg, * (pExcessFlow + y + col) ); // _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + col));	// Load-to-store penalty because of the store with 2 byte offset below
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + col));
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);

			/*West Neighbor*/
			westHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y - Cols + col - 1));
			westCapReg = _mm256_insert_left(westCapReg, * (pWestCap + y + col) ); // _mm256_loadu_si256((__m256i const*)(pWestCap + y + col));			// Load-to-store penalty because of the store with 2 byte offset below
			westHeightReg = _mm256_add_epi16(westHeightReg, oneReg);
			cmpHeight = _mm256_cmpeq_epi16(westHeightReg, currentHeightReg);
			flowReg = _mm256_min_epi16(westCapReg, excessFlowReg);
			westExcessFlowReg = _mm256_insert_left(excessFlowReg__, *(pExcessFlow + y - Cols + col - 1)); //_mm256_loadu_si256((__m256i const*)(pExcessFlow + y - Cols + col - 1));
			eastCapReg = _mm256_insert_left(eastCapReg, *(pEastCap + y - Cols + col - 1 ) ); // _mm256_loadu_si256((__m256i const*)(pEastCap + y - Cols + col - 1));
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			newWestExcessFlowReg = _mm256_add_epi16(westExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			westExcessFlowReg = _mm256_blendv_epi8(westExcessFlowReg, newWestExcessFlowReg, cmpHeight);
			__m256i newWestCapReg = _mm256_sub_epi16(westCapReg, flowReg);
			__m256i newEastCapReg = _mm256_add_epi16(eastCapReg, flowReg);
			westCapReg = _mm256_blendv_epi8(westCapReg, newWestCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y - Cols + col - 1), westExcessFlowReg);
			eastCapReg = _mm256_blendv_epi8(eastCapReg, newEastCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pWestCap + y + col), westCapReg);
			_mm256_storeu_si256((__m256i*)(pEastCap + y - Cols + col - 1), eastCapReg);

			/*East Neighbor*/
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y + Cols + col + 1));
			eastCapReg = _mm256_loadu_si256((__m256i const*)(pEastCap + y + col));
			cmpHeight = _mm256_cmpeq_epi16(_mm256_add_epi16(eastHeightReg, oneReg), currentHeightReg);
			flowReg = _mm256_min_epi16(eastCapReg, excessFlowReg);
			cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight);
			eastExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y + Cols + col + 1));
			westCapReg = _mm256_loadu_si256((__m256i const*)(pWestCap + y + Cols + col + 1));
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight);
			newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
			__m256i newEastExcessFlowReg = _mm256_add_epi16(eastExcessFlowReg, flowReg);
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight);
			newEastCapReg = _mm256_sub_epi16(eastCapReg, flowReg);
			eastExcessFlowReg = _mm256_blendv_epi8(eastExcessFlowReg, newEastExcessFlowReg, cmpHeight);
			newWestCapReg = _mm256_add_epi16(westCapReg, flowReg);
			eastCapReg = _mm256_blendv_epi8(eastCapReg, newEastCapReg, cmpHeight);
			westCapReg = _mm256_blendv_epi8(westCapReg, newWestCapReg, cmpHeight);
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y + col), excessFlowReg);
			excessFlowReg__ = excessFlowReg;
			_mm256_storeu_si256((__m256i*)(pExcessFlow + y + Cols + col + 1), eastExcessFlowReg);	// Store one short offset from the next load
			_mm256_storeu_si256((__m256i*)(pEastCap + y + col), eastCapReg);
			_mm256_storeu_si256((__m256i*)(pWestCap + y + Cols + col + 1), westCapReg);				// Store one short offset from the next load
		}
		} ///// End horizontal push

	}

}

__forceinline int AVX2ActivePixelsTF16x16_single(short * pExcessFlow, HEIGHT_TYPE * pHeight, active_block_t &block, int Rows, int Cols)
{
	__m256i heightMax;
	__m256i currentHeightReg, excessFlowReg, cmpCurrHeight, cmpExcessFlow;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i zeroReg = _mm256_setzero_si256();
	heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	int mask;
	char ActiveNodeCount = 0;
	//for(auto block: blocks) 
	{
		int row = block.row;
		int col = block.col;
		for (int y = row; (y < (row + SIDE_SQUARE)) && (y <(Rows-1)) && (ActiveNodeCount == 0); y++) {
			__m256i active_reg;
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y*Cols + col));
			excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y*Cols + col));
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);
			mask = _mm256_movemask_epi8(active_reg);
			ActiveNodeCount |= (mask != 0);
		}
	}
	return ActiveNodeCount;
}

int AVX2ActivePixelsTF16x16(short * pExcessFlow, HEIGHT_TYPE * pHeight, active_list &blocks, int Rows, int Cols)
{
	__m256i heightMax;
	__m256i currentHeightReg, excessFlowReg, cmpCurrHeight, cmpExcessFlow;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i zeroReg = _mm256_setzero_si256();
	// heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	heightMax = _mm256_set1_epi16(HEIGHT_MAX); // _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	// int mask;
	int ActiveNodeCount = 0;

	// for(auto block: blocks) 
	
	#pragma omp parallel for reduction(+: ActiveNodeCount)
	for(int i=0; i<blocks.size(); ++i)
	{
		int row = blocks[i].row;
		int col = blocks[i].col;
		for (int y = row; (y < (row + SIDE_SQUARE)) && (y <(Rows-1)) && (ActiveNodeCount == 0); y++) {
			__m256i active_reg;
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y*Cols + col));
			excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y*Cols + col));
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);
			int mask = _mm256_movemask_epi8(active_reg);
			// ActiveNodeCount |= (mask != 0);
//			ActiveNodeCount = __popcnt(mask) / 8;
			ActiveNodeCount += __popcnt((unsigned int)mask) / 2;
		}
	}
	return ActiveNodeCount;
}


/////////////////// Graph Cut entry function MWYI//////////////////////////////////////////////////////////////////
int AVX2Model_Push_Relabel_TF(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pWestCap, short * pNorthCap,
	short * pEastCap, short * pSouthCap, unsigned char * pBlockMask, RelabelBlock * pTableBlock, unsigned char * pOutput,
	int Rows, int Cols, int RowsTF, int ColsTF, int BlkRows, int BlkCols, int BlkWidth, int BlkHeight, int nBits)
{
	int iter = 1;
	int ActivePixels;
	active_list a_list;
	AVX2ActiveBlockTF16x16(pExcessFlow, pHeight, Rows, Cols, RowsTF, ColsTF, pTableBlock, a_list);

#ifdef _DEBUG
	std::cout << "Number of active blocks " << a_list.size() << std::endl;
#endif
	/// New parallel model
	for(int i=0; i<a_list.size(); ++i)
		AVX2Model_GraphCut_RelabelTF16x16_single(pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, a_list[i], RowsTF, ColsTF);

#ifdef _DEBUG
	printf("a_list size %d\n", a_list.size());
#endif
	if(a_list.size()>1000) // parallel processing only for big workloads
		#pragma omp parallel shared(pExcessFlow, pHeight, pWestCap, pEastCap, pNorthCap, pSouthCap, a_list, ActivePixels) reduction(|:iter)
		{
			iter = 1;
			do {
				// Relabel all active nodes
				if(iter != 1) // skip first iteration
				{
					if( iter & ((1<<nBits)-1) ) // Check nRatio lower bits and do Global relable only when all of them are 0
					{
						// Doesn't update a_list and can be done in parallel and change it to a_list processing
						#pragma omp for
						for(int i=0; i<a_list.size(); ++i)
							AVX2Model_GraphCut_RelabelTF16x16_single(pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, a_list[i], RowsTF, ColsTF);
					}
					else {    // Global relabel 
						//#pragma omp single
						//printf("global relable %d\n", a_list.size());

						// Single thread should take care of doing global relable
						//// Merge with AVX2ActiveBlockTF16x16 to build new active list
						// #pragma omp single
						AVX2Model_Global_RelabelTF16x16(pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, pTableBlock, RowsTF, ColsTF, a_list, pExcessFlow, RowsTF, ColsTF);

						// This one can be done in parallel
						//#pragma omp single
						// AVX2ActiveBlockTF16x16(pExcessFlow, pHeight, Rows, Cols, RowsTF, ColsTF, pTableBlock, a_list);
					}
				}

				/// No need for barrier due to sync from "omp for" & "omp single"
				// #pragma omp barrier

				// Split in 2 pieces with barrier in the middle to avoid neigbours modification
				#pragma omp for
				for(int i=0; i<a_list.size(); ++i)
					AVX2Model_GraphCut_PushTF16x16_single(pExcessFlow, pHeight, pWestCap, pEastCap, pNorthCap, pSouthCap, a_list[i], RowsTF, ColsTF);

				// Should be done in parallel as well
				// #pragma omp single
				ActivePixels = AVX2ActivePixelsTF16x16(pExcessFlow, pHeight, a_list, RowsTF, ColsTF);

#ifdef _DEBUG
				printf("%d, ListSize = %d, ActivePixels = %d\n", iter, a_list.size(), ActivePixels);
#endif

			} while (++iter < 256 && ActivePixels > 0);
		}



	// if(ActivePixels > 0 && iter < 256)
	do {
#ifdef _DEBUG
		printf("%3d, ", iter);
#endif
		// Relabel all active nodes
		if( iter & ((1<<nBits)-1) ) // Check nRatio lower bits and do Global relable only when all of them are 0
		{
			// Doesn't update a_list and can be done in parallel and change it to a_list processing
			AVX2Model_GraphCut_RelabelTF16x16(pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, pTableBlock, RowsTF, ColsTF);
		}
		else {    // Global relabel 
			// I guess single thread should take care of this one
			//AVX2Model_Global_RelabelTF16x16(pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, pTableBlock, RowsTF, ColsTF);
			//AVX2ActiveBlockTF16x16(pExcessFlow, pHeight, Rows, Cols, RowsTF, ColsTF, pTableBlock, a_list);
			AVX2Model_Global_RelabelTF16x16(pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, pTableBlock, RowsTF, ColsTF, a_list, pExcessFlow, RowsTF, ColsTF);
		}
		//		CreateBlockMask(pExcessFlow, pHeight, Rows, Cols, pBlockMask, BlkRows, BlkCols, BlkWidth, BlkHeight);

#ifdef _DEBUG
		char fn[128];
		sprintf(fn, ".\\Output\\_%d_AVX_TF_Height.%dx%d.Y8", iter, sizeof(HEIGHT_TYPE)*Cols, Rows);
		Dump2File(fn, (unsigned char *)pHeight, Rows*Cols*sizeof(HEIGHT_TYPE));

		//sprintf(fn, ".\\Output\\_%d_CPU_BlockMask.%dx%d.Y8", iter, BlkCols, BlkRows);
		//Dump2File(fn, (unsigned char *)pBlockMask, BlkRows*BlkCols);
#endif

		ActiveBlocks blocks(a_list);
		
		if(a_list.size() > 2000) {

			// Push all active nodes in all directions
			ActivePixels = 0;
			#pragma omp parallel for shared(pExcessFlow, pHeight, pWestCap, pEastCap, pNorthCap, pSouthCap)
			for(int i=0; i<a_list.size(); ++i)
				AVX2Model_GraphCut_PushTF16x16_single(pExcessFlow, pHeight, pWestCap, pEastCap, pNorthCap, pSouthCap, a_list[i], RowsTF, ColsTF);

				{

				//#pragma omp for // schedule(static) 
				//#pragma omp for schedule(static) 
				//for(int i=0; i<a_list.size(); ++i)
				//	ActivePixels |= AVX2ActivePixelsTF16x16_single(pExcessFlow, pHeight, a_list[i], RowsTF, ColsTF);
			}
		}
		else
		{
			AVX2Model_GraphCut_PushTF16x16(pExcessFlow, pHeight, pWestCap, pEastCap, pNorthCap, pSouthCap, blocks, RowsTF, ColsTF);
		}

		ActivePixels = AVX2ActivePixelsTF16x16(pExcessFlow, pHeight, a_list, RowsTF, ColsTF);

#ifdef _DEBUG
		printf("Active pixels = %6d\n", ActivePixels);
#endif
	} while (ActivePixels > 0 && ++iter < 256);
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

	// Write pOutput Transformed back
	for (int y = 0; y < Rows; y++) {
		int index = y;
		for (int x = 0; x < Cols; x++) {
			unsigned char val = GetPix(pHeight, ColsTF, x + IMAGE_PADDING, index + 1) < HEIGHT_MAX ? 0 : 255;
			PutPix(pOutput, Cols, x, y, val);
			index++;
		}
	}
	return iter;
}