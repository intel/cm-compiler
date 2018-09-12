#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <math.h>

#include "common_C_model.h"
#include "Graph_Cut_host.h"

#define GRAPHCUT_RELABLE(size, type) \
__m256i zeroReg = _mm256_setzero_si256(); \
__m256i heightMax =  _mm256_broadcast##type##_epi##size##(_mm_cvtsi32_si128(HEIGHT_MAX)); \
__m256i oneReg = _mm256_set1_epi##size(1); \
__declspec(align(32)) HEIGHT_TYPE newHeights[SIDE_SQUARE]; \
	\
int prevIndex = 0; \
int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; \
	while (indexActiveBlock != -1) \
	{ \
	int currentBlock = prevIndex + indexActiveBlock; \
	int row = pTableBlock[currentBlock].row; \
	int col = pTableBlock[currentBlock].col; \
	prevIndex = currentBlock; \
	indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; \
	for (int y = row; y < (row + SIDE_SQUARE); y++) { \
		__m256i cmpExcessFlow, currentHeightReg, cmpCurrHeight, active_reg; \
		/* Check for active node */ \
		if (size == 32) { \
		  __m128i excessFlowReg = _mm_loadu_si128((__m128i const*)(pExcessFlow + Cols*y + col)); \
		  currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col)); \
		  cmpExcessFlow = _mm256_cvtepi16_epi32(excessFlowReg); \
		  cmpExcessFlow = _mm256_cmpgt_epi##size(cmpExcessFlow, zeroReg); \
		  cmpCurrHeight = _mm256_cmpgt_epi##size(heightMax, currentHeightReg); \
		  active_reg = _mm256_and_si256(cmpExcessFlow, cmpCurrHeight); \
						} else { \
		  cmpExcessFlow = _mm256_loadu_si256((__m256i const*)(pExcessFlow + Cols*y + col)); \
		  currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col)); \
		  cmpCurrHeight = _mm256_min_epu##size(heightMax, currentHeightReg); \
		  cmpExcessFlow = _mm256_cmpgt_epi##size(cmpExcessFlow, zeroReg); \
          cmpCurrHeight = _mm256_cmpeq_epi##size(cmpCurrHeight, heightMax); \
		  active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow); \
						} \
		int mask = _mm256_movemask_epi8(active_reg); \
		\
		if (mask != 0) { \
		    \
		    __m256i capReg, cmpCap; \
			__m256i newHeight = heightMax; \
			/*North Neighbor*/ \
			if ((y - 1)>=0) { \
			__m256i northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col)); \
			if (size == 32) { \
			   capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pNorthCap + Cols*y + col))); \
									} else { \
			   capReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + Cols*y + col)); \
									} \
			northHeightReg = _mm256_add_epi##size(northHeightReg, oneReg); \
			cmpCap = _mm256_cmpgt_epi##size(capReg, zeroReg); \
			northHeightReg = _mm256_blendv_epi8(heightMax, northHeightReg, cmpCap); \
			newHeight = _mm256_min_epu##size(heightMax, northHeightReg); \
						} \
			\
			/*South Neighbor*/ \
			if ((y + 1) < Rows) { \
			__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col)); \
			if (size == 32) { \
			  capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pSouthCap + Cols*y + col))); \
									} else { \
			  capReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + Cols*y + col)); \
									} \
			southHeightReg = _mm256_add_epi##size(southHeightReg, oneReg); \
			cmpCap = _mm256_cmpgt_epi##size(capReg, zeroReg); \
			southHeightReg = _mm256_blendv_epi8(heightMax, southHeightReg, cmpCap); \
			newHeight = _mm256_min_epu##size(newHeight, southHeightReg); \
            \
						} \
			/*East Neighbor*/ \
			if (((col+1)+SIDE_SQUARE) < Cols) { \
			__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col + 1)); \
			if (size == 32) { \
			  capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pEastCap + Cols*y + col))); \
									} else { \
			   capReg = _mm256_loadu_si256((__m256i const*)(pEastCap + Cols*y + col)); \
									} \
			eastHeightReg = _mm256_add_epi##size(eastHeightReg, oneReg); \
			cmpCap = _mm256_cmpgt_epi##size((capReg), zeroReg); \
			eastHeightReg = _mm256_blendv_epi8(heightMax, eastHeightReg, cmpCap); \
			newHeight = _mm256_min_epu##size(newHeight, eastHeightReg); \
			\
			newHeight = _mm256_blendv_epi8(currentHeightReg, newHeight, active_reg); \
			_mm256_store_si256((__m256i *)newHeights, newHeight); \
						} else { \
			    _mm256_store_si256((__m256i *)newHeights, newHeight); \
				int copyMask = mask; \
				for (int i = col; i < Cols; i++) { \
					if (copyMask & 1) \
										{ \
						temp = pHeight[Cols*y +i + 1] + 1; \
						HEIGHT_TYPE currHeight = newHeights[i-col]; \
						HEIGHT_TYPE new_height = pEastCap[Cols*y + i] > 0 ? temp : HEIGHT_MAX; \
						temp = new_height < currHeight ? new_height : currHeight; \
						newHeights[i-col] = temp; \
										} \
					copyMask >>= 32/SIDE_SQUARE; \
								} \
						} \
	        \
	        /*West Neighbor*/ \
			for (int i = 0; i < SIDE_SQUARE; i++) \
									{ \
				if (mask & 1) { \
					temp = pHeight[Cols*y + (((col+i - 1)<0)?0:(col+i-1))] + 1; \
					HEIGHT_TYPE currHeight = newHeights[i]; \
					HEIGHT_TYPE new_height = pWestCap[Cols*y+col+i] > 0 ? temp : HEIGHT_MAX; \
					temp = new_height < currHeight ? new_height : currHeight; \
					pHeight[Cols*y+ col+i] = (HEIGHT_TYPE)temp; \
								} \
				mask >>= 32/SIDE_SQUARE; \
									} \
						} \
			} \
		} \

#define GLOBAL_RELABLE(size, type) \
__m256i currentHeightReg; \
__m256i zeroReg = _mm256_setzero_si256(); \
__m256i heightMax = _mm256_broadcast##type##_epi##size##(_mm_cvtsi32_si128(HEIGHT_MAX)); \
__m256i oneReg = _mm256_set1_epi##size(1); \
__declspec(align(32)) HEIGHT_TYPE newHeights[SIDE_SQUARE]; \
	\
/* Init Height */ \
int prevIndex = 0; \
int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; \
while (indexActiveBlock != -1) { \
	int currentBlock = prevIndex + indexActiveBlock; \
	int row = pTableBlock[currentBlock].row; \
	int col = pTableBlock[currentBlock].col; \
	prevIndex = currentBlock; \
	indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; \
	for (y = row; y < (row + SIDE_SQUARE); y++) { \
		currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col)); \
		__m256i cmpHeightreg = _mm256_cmpeq_epi##size(currentHeightReg, zeroReg); \
		currentHeightReg = _mm256_andnot_si256(cmpHeightreg, heightMax); \
		_mm256_storeu_si256((__m256i *)(pHeight + Cols*y + col), currentHeightReg); \
	} \
} \
/* Global relabel */ \
int numIter = 0; \
do { \
	numIter++; \
	NewHeightCount = 0; \
	\
    prevIndex = 0; \
	indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; \
		while (indexActiveBlock != -1) { \
		int currentBlock = prevIndex + indexActiveBlock; \
		int row = pTableBlock[currentBlock].row; \
		int col = pTableBlock[currentBlock].col; \
		prevIndex = currentBlock; \
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; \
		for (y = row; y < (row + SIDE_SQUARE); y++) { \
			__m256i capReg, newHeight, cmpCap; \
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col)); \
			newHeight = currentHeightReg; \
			/*North Neighbor*/ \
			if ((y-1) >= 0) { \
			__m256i northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col)); \
			if (size == 32) { \
				capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pNorthCap + Cols*y + col))); \
																	} else { \
				capReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + Cols*y + col)); \
																	} \
			northHeightReg = _mm256_add_epi##size(northHeightReg, oneReg); \
			cmpCap = _mm256_cmpgt_epi##size(capReg, zeroReg); \
			northHeightReg = _mm256_blendv_epi8(currentHeightReg, northHeightReg, cmpCap); \
			newHeight = _mm256_min_epu##size(currentHeightReg, northHeightReg); \
						} \
			\
			/*South Neighbor*/ \
			if ((y+1) < Rows) { \
			__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col)); \
			if (size == 32) { \
				capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pSouthCap + Cols*y + col))); \
														} else { \
				capReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + Cols*y + col)); \
														} \
			southHeightReg = _mm256_add_epi##size(southHeightReg, oneReg); \
			cmpCap = _mm256_cmpgt_epi##size(capReg, zeroReg); \
			southHeightReg = _mm256_blendv_epi8(newHeight, southHeightReg, cmpCap); \
			newHeight = _mm256_min_epu##size(newHeight, southHeightReg); \
						} \
			/*East Neighbor*/ \
			if ((col + 1 + SIDE_SQUARE) < Cols) { \
			__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col + 1)); \
			if (size == 32) { \
				capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pEastCap + Cols*y + col))); \
																	} else { \
				capReg = _mm256_loadu_si256((__m256i const*)(pEastCap + Cols*y + col)); \
																	} \
			eastHeightReg = _mm256_add_epi##size(eastHeightReg, oneReg); \
			cmpCap = _mm256_cmpgt_epi##size((capReg), zeroReg); \
			eastHeightReg = _mm256_blendv_epi8(newHeight, eastHeightReg, cmpCap); \
			newHeight = _mm256_min_epu##size(newHeight, eastHeightReg); \
			\
			_mm256_store_si256((__m256i *)newHeights, newHeight); \
						} else { \
				_mm256_store_si256((__m256i *)newHeights, newHeight); \
				for (int i = col; i < Cols; i++) { \
					temp = pHeight[Cols*y + i + 1] + 1; \
					HEIGHT_TYPE new_height = newHeights[i - col]; \
					temp = pEastCap[Cols*y + i] > 0 ? temp : new_height; \
					newHeights[i - col] = new_height < temp ? new_height : temp; \
								} \
						} \
			/*West Neighbor*/ \
			temp = pHeight[Cols*y + (((col - 1)<0)?0:(col-1))] + 1; \
			for (int i = 0; i < SIDE_SQUARE; i++) { \
					HEIGHT_TYPE new_height = newHeights[i]; \
					temp = pWestCap[Cols*y+ col + i] > 0 ? temp : new_height; \
					new_height = new_height < temp ? new_height : temp; \
					HEIGHT_TYPE current = pHeight[Cols*y + col + i]; \
					if (new_height != current) { \
					   pHeight[Cols*y + col + i] = (HEIGHT_TYPE)new_height; \
					   NewHeightCount++; \
										} \
					temp = new_height+1; \
						} \
				} \
		   } \
  } while(numIter < 16  && NewHeightCount > 0);

#define SAVE_EVEN_ODD_ROWS16(table, dstTable, j, i, rowD, colD, Cols, tempReg) \
tempReg = table[j]; \
_mm256_storeu_si256((__m256i*)(dstTable + (rowD + i) * Cols + colD), _mm256_permute2x128_si256(tempReg, table[j + 8], 0x20)); \
_mm256_storeu_si256((__m256i*)(dstTable + (rowD + i + 8) * Cols + colD), _mm256_permute2x128_si256(tempReg, table[j + 8], 0x31)); \
tempReg = table[j + 4]; \
_mm256_storeu_si256((__m256i*)(dstTable + (rowD + i + 1) * Cols + colD), _mm256_permute2x128_si256(tempReg, table[j + 12], 0x20)); \
_mm256_storeu_si256((__m256i*)(dstTable + (rowD + i + 9) * Cols + colD), _mm256_permute2x128_si256(tempReg, table[j + 12], 0x31)); \

#define SAVE_BLOCK16(table, dstTable, i, rowD, colD, Cols, tempReg) \
	SAVE_EVEN_ODD_ROWS16(table, dstTable, 0, i+0, rowD, colD, Cols, tempReg) \
	SAVE_EVEN_ODD_ROWS16(table, dstTable, 1, i+4, rowD, colD, Cols, tempReg) \
	SAVE_EVEN_ODD_ROWS16(table, dstTable, 2, i+2, rowD, colD, Cols, tempReg) \
	SAVE_EVEN_ODD_ROWS16(table, dstTable, 3, i+6, rowD, colD, Cols, tempReg) \


#define SAVE_EVEN_ODD_ROWS32(table, dstTable, j, i, rowD, colD, Cols) \
table[j] = _mm256_permute4x64_epi64(table[j], 0xD8); \
_mm_storeu_si128((__m128i*)(dstTable + (rowD + i)*Cols + colD), _mm256_castsi256_si128(table[j])); \
_mm_storeu_si128((__m128i*)(dstTable + (rowD + i + 1)*Cols + colD), _mm256_extracti128_si256(table[j], 1)); \
table[j + 2] = _mm256_permute4x64_epi64(table[j + 2], 0xD8); \
_mm_storeu_si128((__m128i*)(dstTable + (rowD + i + 2)*Cols + colD), _mm256_castsi256_si128(table[j + 2])); \
_mm_storeu_si128((__m128i*)(dstTable + (rowD + i + 3)*Cols + colD), _mm256_extracti128_si256(table[j + 2], 1)); \

#define SAVE_BLOCK32(table, dstTable, i, rowD, colD, Cols) \
	SAVE_EVEN_ODD_ROWS32(table, dstTable, 0, i+0, rowD, colD, Cols) \
	SAVE_EVEN_ODD_ROWS32(table, dstTable, 1, i+4, rowD, colD, Cols) \

#define UNPACK16BIT16(back, i, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	if (back == 0) { \
			heightReg = _mm256_loadu_si256((__m256i*)(pHeightS + (rowS + i)*numColsS + colS)); \
			nextHeightReg = _mm256_loadu_si256((__m256i*)(pHeightS + (rowS + i + 1)*numColsS + colS)); \
			heightT[i] = _mm256_unpacklo_epi16(heightReg, nextHeightReg); \
			heightT[i + 1] = _mm256_unpackhi_epi16(heightReg, nextHeightReg); \
		} \
	excessFlowReg = _mm256_loadu_si256((__m256i*)(pExcessFlowS + (rowS + i)*numColsS + colS)); \
	nextExcessFlowReg = _mm256_loadu_si256((__m256i*)(pExcessFlowS + (rowS + i + 1)*numColsS + colS)); \
	excessFlowT[i] = _mm256_unpacklo_epi16(excessFlowReg, nextExcessFlowReg); \
	excessFlowT[i + 1] = _mm256_unpackhi_epi16(excessFlowReg, nextExcessFlowReg); \
	westCapReg = _mm256_loadu_si256((__m256i*)(pWestCapS + (rowS + i)*numColsS + colS)); \
	nextWestCapReg = _mm256_loadu_si256((__m256i*)(pWestCapS + (rowS + i + 1)*numColsS + colS)); \
	westCapT[i] = _mm256_unpacklo_epi16(westCapReg, nextWestCapReg); \
	westCapT[i + 1] = _mm256_unpackhi_epi16(westCapReg, nextWestCapReg); \
	eastCapReg = _mm256_loadu_si256((__m256i*)(pEastCapS + (rowS + i)*numColsS + colS)); \
	nextEastCapReg = _mm256_loadu_si256((__m256i*)(pEastCapS + (rowS + i + 1)*numColsS + colS)); \
	eastCapT[i] = _mm256_unpacklo_epi16(eastCapReg, nextEastCapReg); \
	eastCapT[i + 1] = _mm256_unpackhi_epi16(eastCapReg, nextEastCapReg); \

#define UNPACK16BIT16X8(back, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	UNPACK16BIT16(back, 0, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	UNPACK16BIT16(back, 2, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	UNPACK16BIT16(back, 4, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	UNPACK16BIT16(back, 6, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	UNPACK16BIT16(back, 8, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	UNPACK16BIT16(back, 10, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	UNPACK16BIT16(back, 12, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	UNPACK16BIT16(back, 14, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \

#define UNPACK16BIT32(back, i, j) \
	if (back == 0) { \
			heightReg = heightT[i + j]; \
			heightT[i + j] = _mm256_unpacklo_epi32(heightReg, heightT[i + j + 2]); \
			heightT[i + j + 2] = _mm256_unpackhi_epi32(heightReg, heightT[i + j + 2]); \
		} \
	excessFlowReg = excessFlowT[i + j]; \
	excessFlowT[i + j] = _mm256_unpacklo_epi32(excessFlowReg, excessFlowT[i + j + 2]); \
	excessFlowT[i + j + 2] = _mm256_unpackhi_epi32(excessFlowReg, excessFlowT[i + j + 2]); \
	westCapReg = westCapT[i + j]; \
	westCapT[i + j] = _mm256_unpacklo_epi32(westCapReg, westCapT[i + j + 2]); \
	westCapT[i + j + 2] = _mm256_unpackhi_epi32(westCapReg, westCapT[i + j + 2]); \
	eastCapReg = eastCapT[i + j]; \
	eastCapT[i + j] = _mm256_unpacklo_epi32(eastCapReg, eastCapT[i + j + 2]); \
	eastCapT[i + j + 2] = _mm256_unpackhi_epi32(eastCapReg, eastCapT[i + j + 2]); \

#define UNPACK16BIT32X2(back, i) \
	UNPACK16BIT32(back, i, 0) \
	UNPACK16BIT32(back, i, 1) \

#define UNPACK16BIT32X2X4(back) \
	UNPACK16BIT32X2(back, 0) \
	UNPACK16BIT32X2(back, 4) \
	UNPACK16BIT32X2(back, 8) \
	UNPACK16BIT32X2(back, 12) \

#define UPACK16BIT64(back, i, j) \
	if (back == 0) { \
		heightReg = heightT[i + j]; \
		heightT[i + j] = _mm256_unpacklo_epi64(heightReg, heightT[i + j + 4]); \
		heightT[i + j + 4] = _mm256_unpackhi_epi64(heightReg, heightT[i + j + 4]); \
		} \
	excessFlowReg = excessFlowT[i + j]; \
	excessFlowT[i + j] = _mm256_unpacklo_epi64(excessFlowReg, excessFlowT[i + j + 4]); \
	excessFlowT[i + j + 4] = _mm256_unpackhi_epi64(excessFlowReg, excessFlowT[i + j + 4]); \
	westCapReg = westCapT[i + j]; \
	westCapT[i + j] = _mm256_unpacklo_epi64(westCapReg, westCapT[i + j + 4]); \
	westCapT[i + j + 4] = _mm256_unpackhi_epi64(westCapReg, westCapT[i + j + 4]); \
	eastCapReg = eastCapT[i + j]; \
	eastCapT[i + j] = _mm256_unpacklo_epi64(eastCapReg, eastCapT[i + j + 4]); \
	eastCapT[i + j + 4] = _mm256_unpackhi_epi64(eastCapReg, eastCapT[i + j + 4]); \

#define UPACK16BIT64X4(back, i) \
	UPACK16BIT64(back, i, 0) \
	UPACK16BIT64(back, i, 1) \
	UPACK16BIT64(back, i, 2) \
	UPACK16BIT64(back, i, 3) \

#define UPACK16BIT64X4X2(back) \
	UPACK16BIT64X4(back, 0) \
	UPACK16BIT64X4(back, 8) \

#define TRANSPOSE(back,size, rowS, colS, numColsS, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowD, colD, numColsD, pHeightD, pExcessFlowD,pWestCapD, pEastCapD) \
if (size == 32) { \
	for (i = 0; i < 4; i++){ \
		__m256i excessFlow = _mm256_castsi128_si256(_mm_loadu_si128((__m128i*)(pExcessFlowS + (rowS + i)*numColsS + colS))); \
		excessFlowT[i] = _mm256_inserti128_si256(excessFlow, _mm_loadu_si128((__m128i*)(pExcessFlowS + (rowS + i + 4)*numColsS + colS)), 1); \
		\
		__m256i westCap = _mm256_castsi128_si256(_mm_loadu_si128((__m128i*)(pWestCapS + (rowS + i)*numColsS + colS))); \
		westCapT[i] = _mm256_inserti128_si256(westCap, _mm_loadu_si128((__m128i*)(pWestCapS + (rowS + i + 4)*numColsS + colS)), 1); \
		\
		__m256i eastCap = _mm256_castsi128_si256(_mm_loadu_si128((__m128i*)(pEastCapS + (rowS + i)*numColsS + colS))); \
		eastCapT[i] = _mm256_inserti128_si256(eastCap, _mm_loadu_si128((__m128i*)(pEastCapS + (rowS + i + 4)*numColsS + colS)), 1); \
		} \
	for (i = 0; i < 4; i += 2){ \
		__m256i excessFlowReg = excessFlowT[i]; \
		excessFlowT[i] = _mm256_unpacklo_epi16(excessFlowReg, excessFlowT[i + 1]); \
		excessFlowT[i + 1] = _mm256_unpackhi_epi16(excessFlowReg, excessFlowT[i + 1]); \
		\
		__m256i westCapReg = westCapT[i]; \
		westCapT[i] = _mm256_unpacklo_epi16(westCapReg, westCapT[i + 1]); \
		westCapT[i + 1] = _mm256_unpackhi_epi16(westCapReg, westCapT[i + 1]); \
		\
		__m256i eastCapReg = eastCapT[i]; \
		eastCapT[i] = _mm256_unpacklo_epi16(eastCapReg, eastCapT[i + 1]); \
		eastCapT[i + 1] = _mm256_unpackhi_epi16(eastCapReg, eastCapT[i + 1]); \
						} \
	if (back == 0) { \
		for (i = 0; i < 8; i += 2){ \
			heightReg = _mm256_loadu_si256((__m256i const *)(pHeightS + (rowS + i)*numColsS + colS)); \
			nextHeightReg = _mm256_loadu_si256((__m256i const *)(pHeightS + (rowS + i + 1)*numColsS + colS)); \
			heightT[i] = _mm256_unpacklo_epi32(heightReg, nextHeightReg); \
			heightT[i + 1] = _mm256_unpackhi_epi32(heightReg, nextHeightReg); \
				} \
		} \
	for (j = 0; j < 2; j++) { \
		excessFlowReg = excessFlowT[j]; \
		excessFlowT[j] = _mm256_unpacklo_epi32(excessFlowReg, excessFlowT[j + 2]); \
		excessFlowT[j + 2] = _mm256_unpackhi_epi32(excessFlowReg, excessFlowT[j + 2]); \
		westCapReg = westCapT[j]; \
		westCapT[j] = _mm256_unpacklo_epi32(westCapReg, westCapT[j + 2]); \
		westCapT[j + 2] = _mm256_unpackhi_epi32(westCapReg, westCapT[j + 2]); \
		eastCapReg = eastCapT[j]; \
		eastCapT[j] = _mm256_unpacklo_epi32(eastCapReg, eastCapT[j + 2]); \
		eastCapT[j + 2] = _mm256_unpackhi_epi32(eastCapReg, eastCapT[j + 2]); \
		} \
	if (back == 0) { \
		for (i = 0; i < 8; i += 4) \
			for (j = 0; j < 2; j++) { \
				heightReg = heightT[i + j]; \
				heightT[i + j] = _mm256_unpacklo_epi64(heightReg, heightT[i + j + 2]); \
				heightT[i + j + 2] = _mm256_unpackhi_epi64(heightReg, heightT[i + j + 2]); \
						} \
		} \
	if (back == 0) { \
		for (i = 1, j =0 ; i < 4; i+=2, j++) { \
			_mm256_storeu_si256((__m256i*)(heightH + (0 + i)*SIDE_SQUARE + 0), _mm256_permute2x128_si256(heightT[j], heightT[j + 4], 0x20)); \
			_mm256_storeu_si256((__m256i*)(heightH + (0 + i + 4)*SIDE_SQUARE + 0), _mm256_permute2x128_si256(heightT[j], heightT[j + 4], 0x31)); \
			_mm256_storeu_si256((__m256i*)(heightH + (0 + i + 1)*SIDE_SQUARE + 0), _mm256_permute2x128_si256(heightT[j + 2], heightT[j + 6], 0x20)); \
			_mm256_storeu_si256((__m256i*)(heightH + (0 + i + 5)*SIDE_SQUARE + 0), _mm256_permute2x128_si256(heightT[j + 2], heightT[j + 6], 0x31)); \
				} \
		i = 1; \
		} else { \
		i = 0; \
		} \
	SAVE_BLOCK32(excessFlowT, pExcessFlowD, i, rowD, colD, numColsD) \
	SAVE_BLOCK32(westCapT, pWestCapD, i, rowD, colD, numColsD) \
	SAVE_BLOCK32(eastCapT, pEastCapD, i, rowD, colD, numColsD) \
} else { \
	UNPACK16BIT16X8(back, pHeightS, pExcessFlowS, pWestCapS, pEastCapS, rowS, colS, numColsS) \
	UNPACK16BIT32X2X4(back) \
	UPACK16BIT64X4X2(back) \
	i= (back == 0)?1:0; \
	if (back == 0) { \
		SAVE_BLOCK16(heightT, pHeightD, i, rowD, colD, numColsD, heightReg) \
		} \
	SAVE_BLOCK16(excessFlowT, pExcessFlowD, i, rowD, colD, numColsD, excessFlowReg) \
	SAVE_BLOCK16(westCapT, pWestCapD, i, rowD, colD, numColsD, westCapReg) \
	SAVE_BLOCK16(eastCapT, pEastCapD, i, rowD, colD, numColsD, eastCapReg) \
} \
if (back == 0) { \
	for (i = 0; i < 256/size; i++) { \
		pHeightD[(rowD)*numColsD + colD + i] = pHeightS [(rowS + i)*numColsS + colS - 1]; \
		pHeightD[(rowD + 256 / size + 1)*numColsD + colD + i] = pHeightS [(rowS + i)*numColsS + colS + 256 / size]; \
		pExcessFlowD[(rowD)*numColsD + colD + i] = pExcessFlowS[(rowS + i)*numColsS + colS - 1]; \
		pExcessFlowD[(rowD + 256 / size + 1)*numColsD + colD + i] = pExcessFlowS[(rowS + i)*numColsS + colS + 256 / size]; \
		pWestCapD[(rowD)*numColsD + colD + i] = pWestCapS[(rowS + i)*numColsS + colS - 1]; \
		pWestCapD[(rowD + 256 / size + 1)*numColsD + colD + i] = pWestCapS[(rowS + i)*numColsS + colS + 256 / size]; \
		pEastCapD[(rowD)*numColsD + colD + i] = pEastCapS[(rowS + i)*numColsS + colS - 1]; \
		pEastCapD[(rowD + 256 / size + 1)*numColsD + colD + i] = pEastCapS[(rowS + i)*numColsS + colS + 256 / size]; \
		} \
} else { \
	for (i = 0; i < 256/size; i++) { \
		pExcessFlowD[(rowD + i)*numColsD + colD - 1] = pExcessFlowS[(rowS-1)*numColsS + colS + i]; \
		pExcessFlowD[(rowD + i)*numColsD + colD + 256 / size] = pExcessFlowS[(rowS + 256 / size)*numColsS + colS + i]; \
		pWestCapD[(rowD + i)*numColsD + colD - 1] = pWestCapS[(rowS-1)*numColsS + colS + i]; \
		pWestCapD[(rowD + i)*numColsD + colD + 256 / size] = pWestCapS[(rowS + 256 / size)*numColsS + colS + i]; \
		pEastCapD[(rowD + i)*numColsD + colD - 1] = pEastCapS[(rowS-1)*numColsS + colS + i]; \
		pEastCapD[(rowD + i)*numColsD + colD + 256 / size] = pEastCapS[(rowS + 256 / size)*numColsS + colS + i]; \
		} \
} \

#define GRAPHCUT_PUSH_BLOCK(size, row, col, numCols, start, end, pHeight, pExcessFlow, pNorthCap, pSouthCap) \
for (y = 0; y < SIDE_SQUARE; y+=1) { \
	/* Check for active node */ \
	__m256i active_reg; \
	if (size == 32) { \
	   if (y == 0) { \
			__m128i excessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + numCols*(y+row) + col)); \
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + numCols*(y+row) + col)); \
			excessFlowReg = _mm256_cvtepi16_epi32(excessFlowReg16); \
	   	   } \
	   cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, zeroReg); \
	   cmpCurrHeight = _mm256_cmpgt_epi##size(heightMax, currentHeightReg); \
	   active_reg = _mm256_and_si256(cmpExcessFlow, cmpCurrHeight); \
		} else { \
		 if (y == 0) { \
			excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + numCols*(y+row) + col)); \
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + numCols*(y+row) + col)); \
		 		 } \
		cmpCurrHeight = _mm256_min_epu##size(heightMax, currentHeightReg); \
		cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, zeroReg); \
		cmpCurrHeight = _mm256_cmpeq_epi##size(cmpCurrHeight, heightMax); \
		active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow); \
		} \
	if ((y + start) > 0) { \
		/*North Neighbor*/ \
		if (y == 0) { \
			northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + numCols*(y + row - 1) + col)); \
			if (size == 32) { \
				northCapReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pNorthCap + numCols*(y + row) + col))); \
						} else { \
				northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + numCols*(y + row) + col)); \
						} \
				} \
		northHeightReg = _mm256_add_epi##size(northHeightReg, oneReg); \
		cmpHeight = _mm256_cmpeq_epi##size(northHeightReg, currentHeightReg); \
		flowReg = _mm256_min_epi##size(northCapReg, excessFlowReg); \
		if (y == 0) { \
			if (size == 32) { \
				__m128i northExcessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + numCols*(y + row -1) + col)); \
				__m128i southCapReg16 = _mm_loadu_si128((__m128i const*)(pSouthCap + numCols*(y + row -1) + col)); \
				northExcessFlowReg = _mm256_cvtepi16_epi32(northExcessFlowReg16); \
				southCapReg = _mm256_cvtepi16_epi32(southCapReg16); \
						} else { \
				northExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + numCols*(y + row -1) + col)); \
				southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + numCols*(y + row -1) + col)); \
						} \
				} \
		cmpHeight = _mm256_and_si256(active_reg, cmpHeight); \
		newExcessFlowReg = _mm256_sub_epi##size(excessFlowReg, flowReg); \
		newNorthExcessFlowReg = _mm256_add_epi##size(northExcessFlowReg, flowReg); \
		excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight); \
		northExcessFlowReg = _mm256_blendv_epi8(northExcessFlowReg, newNorthExcessFlowReg, cmpHeight); \
		__m256i newNorthCapReg = _mm256_sub_epi##size(northCapReg, flowReg); \
		__m256i newSouthCapReg = _mm256_add_epi##size(southCapReg, flowReg); \
		northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight); \
		if (size == 32) { \
			 northExcessFlowReg = _mm256_packs_epi32(northExcessFlowReg, northExcessFlowReg); \
			_mm_storeu_si128((__m128i*)(pExcessFlow + numCols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(northExcessFlowReg, 0x8))); \
				} else { \
			_mm256_storeu_si256((__m256i*)(pExcessFlow + numCols*(y + row -1) + col), northExcessFlowReg); \
				} \
		southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight); \
		\
		if (size == 32) { \
		     northCapReg = _mm256_packs_epi32(northCapReg, northCapReg); \
			_mm_storeu_si128((__m128i*) (pNorthCap + numCols*(y + row) + col),_mm256_castsi256_si128(_mm256_permute4x64_epi64(northCapReg, 0x8))); \
			southCapReg = _mm256_packs_epi32(southCapReg, southCapReg); \
			_mm_storeu_si128((__m128i*) (pSouthCap + numCols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(southCapReg, 0x8))); \
				} else { \
			_mm256_storeu_si256((__m256i*)(pNorthCap + numCols*(y + row) + col), northCapReg); \
			_mm256_storeu_si256((__m256i*)(pSouthCap + numCols*(y + row -1) + col), southCapReg); \
				} \
		} \
	if ((start + y) < (end - 1)) { \
		/*South Neighbor*/ \
		cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, zeroReg); \
		__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + numCols*(y + row + 1) + col)); \
		if (size == 32) { \
			southCapReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pSouthCap + numCols*(y + row) + col))); \
				} else { \
			southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + numCols*(y + row) + col)); \
				} \
		cmpHeight = _mm256_cmpeq_epi##size(_mm256_add_epi##size(southHeightReg, oneReg), currentHeightReg); \
		flowReg = _mm256_min_epi##size(southCapReg, excessFlowReg); \
		cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight); \
		if (size == 32) { \
			__m128i southExcessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + numCols*(y + row + 1) + col)); \
			__m128i northCapReg16 = _mm_loadu_si128((__m128i const*)(pNorthCap + numCols*(y + row + 1) + col)); \
			southExcessFlowReg = _mm256_cvtepi16_epi32(southExcessFlowReg16); \
			northCapReg = _mm256_cvtepi16_epi32(northCapReg16); \
				} else { \
			southExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + numCols*(y + row + 1) + col)); \
			northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + numCols*(y + row + 1) + col)); \
				} \
		cmpHeight = _mm256_and_si256(active_reg, cmpHeight); \
		newExcessFlowReg = _mm256_sub_epi##size(excessFlowReg, flowReg); \
		__m256i newSouthExcessFlowReg = _mm256_add_epi##size(southExcessFlowReg, flowReg); \
		excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight); \
		__m256i newSouthCapReg = _mm256_sub_epi##size(southCapReg, flowReg); \
		southExcessFlowReg = _mm256_blendv_epi8(southExcessFlowReg, newSouthExcessFlowReg, cmpHeight); \
		__m256i newNorthCapReg = _mm256_add_epi##size(northCapReg, flowReg); \
		southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight); \
		northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight); \
		northHeightReg = currentHeightReg; \
		northExcessFlowReg = excessFlowReg; \
		excessFlowReg = southExcessFlowReg; \
		currentHeightReg = southHeightReg; \
		} \
  } \
  if ((y + start) < end) { \
		if (size == 32) { \
				northExcessFlowReg = _mm256_packs_epi32(northExcessFlowReg, northExcessFlowReg); \
				_mm_storeu_si128((__m128i*)(pExcessFlow + numCols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(northExcessFlowReg, 0x8))); \
				excessFlowReg = _mm256_packs_epi32(excessFlowReg, excessFlowReg); \
				_mm_storeu_si128((__m128i*)(pExcessFlow + numCols*(y + row) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(excessFlowReg, 0x8))); \
				southCapReg = _mm256_packs_epi32(southCapReg, southCapReg); \
				_mm_storeu_si128((__m128i*) (pSouthCap + numCols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(southCapReg, 0x8))); \
				northCapReg = _mm256_packs_epi32(northCapReg, northCapReg); \
				_mm_storeu_si128((__m128i*) (pNorthCap + numCols*(y + row) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(northCapReg, 0x8))); \
				}else { \
				_mm256_storeu_si256((__m256i*)(pExcessFlow + numCols*(y + row -1) + col), northExcessFlowReg); \
				_mm256_storeu_si256((__m256i*)(pExcessFlow + numCols*(y + row) + col), excessFlowReg); \
				_mm256_storeu_si256((__m256i*)(pSouthCap + numCols*(y + row -1) + col), southCapReg); \
				_mm256_storeu_si256((__m256i*)(pNorthCap + numCols*(y + row) + col), northCapReg); \
				} \
     } else { \
		if (size == 32) { \
			excessFlowReg = _mm256_packs_epi32(excessFlowReg, excessFlowReg); \
			_mm_storeu_si128((__m128i*)(pExcessFlow + numCols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(excessFlowReg, 0x8))); \
			  	} else { \
			_mm256_storeu_si256((__m256i*)(pExcessFlow + numCols*(y + row -1) + col), excessFlowReg); \
			  	} \
     } \




#define GRAPHCUT_PUSH_BLOCK_HORIZ(size, row, col, numCols, start, end, pHeight, pExcessFlow, pNorthCap, pSouthCap) \
int mask; \
for (y = 0; y < SIDE_SQUARE; y+=1) { \
	/* Check for active node */ \
	if (size == 32) { \
		if (y == 0) { \
			__m128i excessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + numCols*(y + row) + col)); \
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + numCols*(y + row) + col)); \
			excessFlowReg = _mm256_cvtepi16_epi32(excessFlowReg16); \
				} \
	   cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, zeroReg); \
	   cmpCurrHeight = _mm256_cmpgt_epi##size(heightMax, currentHeightReg); \
			} else { \
		if (y == 0) { \
			excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + numCols*(y+row) + col)); \
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + numCols*(y + row) + col)); \
				} \
		cmpCurrHeight = _mm256_min_epu##size(heightMax, currentHeightReg); \
		cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, zeroReg); \
		cmpCurrHeight = _mm256_cmpeq_epi##size(cmpCurrHeight, currentHeightReg); \
			} \
	__m256i active_reg = _mm256_and_si256(cmpExcessFlow, cmpCurrHeight); \
	if ((y + start) > 0) { \
		/*North Neighbor*/ \
		if (y == 0) { \
			northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + numCols*(y + row - 1) + col)); \
				} \
		northHeightReg = _mm256_add_epi##size(northHeightReg, oneReg); \
		cmpHeight = _mm256_cmpeq_epi##size(northHeightReg, currentHeightReg); \
		cmpHeight = _mm256_and_si256(active_reg, cmpHeight); \
		mask = _mm256_movemask_epi8(cmpHeight); \
		\
		if (mask != 0) { \
			if (size == 32) { \
					northCapReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pNorthCap + numCols*(y + row) + col))); \
						} else { \
					northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + numCols*(y + row) + col)); \
						} \
			flowReg = _mm256_min_epi##size(northCapReg, excessFlowReg); \
			if (size == 32) { \
				__m128i northExcessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + numCols*(y + row -1) + col)); \
				__m128i southCapReg16 = _mm_loadu_si128((__m128i const*)(pSouthCap + numCols*(y + row -1) + col)); \
				northExcessFlowReg = _mm256_cvtepi16_epi32(northExcessFlowReg16); \
				southCapReg = _mm256_cvtepi16_epi32(southCapReg16); \
						} else { \
				northExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + numCols*(y + row -1) + col)); \
				southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + numCols*(y + row -1) + col)); \
						} \
			newExcessFlowReg = _mm256_sub_epi##size(excessFlowReg, flowReg); \
			newNorthExcessFlowReg = _mm256_add_epi##size(northExcessFlowReg, flowReg); \
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight); \
			northExcessFlowReg = _mm256_blendv_epi8(northExcessFlowReg, newNorthExcessFlowReg, cmpHeight); \
			__m256i newNorthCapReg = _mm256_sub_epi##size(northCapReg, flowReg); \
			__m256i newSouthCapReg = _mm256_add_epi##size(southCapReg, flowReg); \
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight); \
			if (size == 32) { \
				 northExcessFlowReg = _mm256_packs_epi32(northExcessFlowReg, northExcessFlowReg); \
				_mm_storeu_si128((__m128i*)(pExcessFlow + numCols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(northExcessFlowReg, 0x8))); \
								} else { \
				_mm256_storeu_si256((__m256i*)(pExcessFlow + numCols*(y + row -1) + col), northExcessFlowReg); \
								} \
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight); \
			\
			if (size == 32) { \
				 northCapReg = _mm256_packs_epi32(northCapReg, northCapReg); \
				_mm_storeu_si128((__m128i*) (pNorthCap + numCols*(y + row) + col),_mm256_castsi256_si128(_mm256_permute4x64_epi64(northCapReg, 0x8))); \
				southCapReg = _mm256_packs_epi32(southCapReg, southCapReg); \
				_mm_storeu_si128((__m128i*) (pSouthCap + numCols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(southCapReg, 0x8))); \
								} else { \
				_mm256_storeu_si256((__m256i*)(pNorthCap + numCols*(y + row) + col), northCapReg); \
				_mm256_storeu_si256((__m256i*)(pSouthCap + numCols*(y + row -1) + col), southCapReg); \
								} \
				} \
		} \
	if ((start + y) < (end - 1)) { \
		/*South Neighbor*/ \
		cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, zeroReg); \
		__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + numCols*(y + row + 1) + col)); \
		cmpHeight = _mm256_cmpeq_epi##size(_mm256_add_epi##size(southHeightReg, oneReg), currentHeightReg); \
		cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight); \
		cmpHeight = _mm256_and_si256(active_reg, cmpHeight); \
		mask = _mm256_movemask_epi8(cmpHeight); \
		if (size == 32) { \
			__m128i southExcessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + numCols*(y + row + 1) + col)); \
			southExcessFlowReg = _mm256_cvtepi16_epi32(southExcessFlowReg16); \
				} else { \
			southExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + numCols*(y + row + 1) + col)); \
				} \
		\
		if (mask != 0) { \
			if (size == 32) { \
				southCapReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pSouthCap + numCols*(y + row) + col))); \
								} else { \
				southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + numCols*(y + row) + col)); \
								} \
			flowReg = _mm256_min_epi##size(southCapReg, excessFlowReg); \
			if (size == 32) { \
				__m128i northCapReg16 = _mm_loadu_si128((__m128i const*)(pNorthCap + numCols*(y + row + 1) + col)); \
				northCapReg = _mm256_cvtepi16_epi32(northCapReg16); \
											} else { \
				northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + numCols*(y + row + 1) + col)); \
											} \
			newExcessFlowReg = _mm256_sub_epi##size(excessFlowReg, flowReg); \
			__m256i newSouthExcessFlowReg = _mm256_add_epi##size(southExcessFlowReg, flowReg); \
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight); \
			__m256i newSouthCapReg = _mm256_sub_epi##size(southCapReg, flowReg); \
			southExcessFlowReg = _mm256_blendv_epi8(southExcessFlowReg, newSouthExcessFlowReg, cmpHeight); \
			__m256i newNorthCapReg = _mm256_add_epi##size(northCapReg, flowReg); \
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight); \
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight); \
			if (size == 32) { \
				southCapReg = _mm256_packs_epi32(southCapReg, southCapReg); \
				_mm_storeu_si128((__m128i*) (pSouthCap + numCols*(y + row) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(southCapReg, 0x8))); \
				northCapReg = _mm256_packs_epi32(northCapReg, northCapReg); \
				_mm_storeu_si128((__m128i*) (pNorthCap + numCols*(y + row+1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(northCapReg, 0x8))); \
						}else { \
				_mm256_storeu_si256((__m256i*)(pSouthCap + numCols*(y + row) + col), southCapReg); \
				_mm256_storeu_si256((__m256i*)(pNorthCap + numCols*(y + row+1) + col), northCapReg); \
						} \
				} \
		northHeightReg = currentHeightReg; \
		northExcessFlowReg = excessFlowReg; \
		excessFlowReg = southExcessFlowReg; \
		currentHeightReg = southHeightReg; \
		} \
  } \
  if (((y + start) < end)&&(mask != 0)) { \
		if (size == 32) { \
				northExcessFlowReg = _mm256_packs_epi32(northExcessFlowReg, northExcessFlowReg); \
				_mm_storeu_si128((__m128i*)(pExcessFlow + numCols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(northExcessFlowReg, 0x8))); \
				excessFlowReg = _mm256_packs_epi32(excessFlowReg, excessFlowReg); \
				_mm_storeu_si128((__m128i*)(pExcessFlow + numCols*(y + row) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(excessFlowReg, 0x8))); \
						}else { \
				_mm256_storeu_si256((__m256i*)(pExcessFlow + numCols*(y + row -1) + col), northExcessFlowReg); \
				_mm256_storeu_si256((__m256i*)(pExcessFlow + numCols*(y + row) + col), excessFlowReg); \
						} \
       } else { \
		if (size == 32) { \
			excessFlowReg = _mm256_packs_epi32(excessFlowReg, excessFlowReg); \
			_mm_storeu_si128((__m128i*)(pExcessFlow + numCols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(excessFlowReg, 0x8))); \
					  	} else { \
			_mm256_storeu_si256((__m256i*)(pExcessFlow + numCols*(y + row -1) + col), excessFlowReg); \
					  	} \
       } \

#define GRAPHCUT_PUSH(size, type) \
__m256i zeroReg = _mm256_setzero_si256(); \
__m256i heightMax = _mm256_broadcast##type##_epi##size##(_mm_cvtsi32_si128(HEIGHT_MAX)); \
__m256i oneReg = _mm256_set1_epi##size##(1); \
__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight; \
__m256i northCapReg, southCapReg, northExcessFlowReg, southExcessFlowReg, northHeightReg, cmpHeight, newExcessFlowReg, newNorthExcessFlowReg,flowReg; \
__m256i heightReg, nextHeightReg, nextExcessFlowReg, westCapReg, nextWestCapReg, eastCapReg, nextEastCapReg; \
HEIGHT_TYPE heightH[SIDE_SQUARE * (SIDE_SQUARE+2)]; \
short excessFlowH[SIDE_SQUARE * (SIDE_SQUARE + 2)]; \
short westCapH[SIDE_SQUARE * (SIDE_SQUARE + 2)]; \
short eastCapH[SIDE_SQUARE * (SIDE_SQUARE + 2)]; \
int prevIndex = 0; \
int i,j;\
int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; \
	while (indexActiveBlock != -1) \
	{ \
	int y; \
	int currentBlock = prevIndex + indexActiveBlock; \
	int row = pTableBlock[currentBlock].row; \
	int col = pTableBlock[currentBlock].col; \
	prevIndex = currentBlock; \
	indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; \
	GRAPHCUT_PUSH_BLOCK(size, row, col, Cols, row, Rows, pHeight, pExcessFlow, pNorthCap, pSouthCap) \
	} \
prevIndex = 0; \
indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; \
	while (indexActiveBlock != -1) \
	{ \
	int y; \
	int currentBlock = prevIndex + indexActiveBlock; \
	int row = pTableBlock[currentBlock].row; \
	int col = pTableBlock[currentBlock].col; \
	prevIndex = currentBlock; \
	indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; \
	TRANSPOSE(0, size, row, col, Cols, pHeight, pExcessFlow, pWestCap, pEastCap, 0, 0, SIDE_SQUARE, heightH, excessFlowH, westCapH, eastCapH) \
	HEIGHT_TYPE *pHeightH = heightH + SIDE_SQUARE; \
	short* pExcessFlowH = excessFlowH + SIDE_SQUARE; \
	short* pWestCapH = westCapH + SIDE_SQUARE; \
	short* pEastCapH = eastCapH + SIDE_SQUARE; \
	GRAPHCUT_PUSH_BLOCK(size, 0, 0, SIDE_SQUARE, col, Cols, pHeightH, pExcessFlowH, pWestCapH, pEastCapH) \
	TRANSPOSE(1, size, 0, 0, SIDE_SQUARE, pHeightH, pExcessFlowH, pWestCapH, pEastCapH, row, col, Cols, pHeight, pExcessFlow, pWestCap, pEastCap) \
	} \



#define ACTIVE_BLOCK(size, type) \
char ActiveNodeCount; \
int blockInRow = Cols >> LOG_SIZE; \
__m256i currentHeightReg, excessFlowReg, cmpCurrHeight, cmpExcessFlow; \
HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1); \
__m256i threshReg = _mm256_set1_epi##size(-100); \
__m256i heightMax = _mm256_broadcast##type##_epi##size##(_mm_cvtsi32_si128(HEIGHT_MAX)); \
int lastActiveBlock = 0; \
pTableBlock[lastActiveBlock].nextActiveBlock = -1; \
int mask; \
int currentBlock = 0; \
int activeBlocks = 0; \
for (int y = 0; y < Rows; y += SIDE_SQUARE) { \
	for (int x = 0; x < Cols; x += SIDE_SQUARE) { \
		ActiveNodeCount = 0; \
		for (int i = y; (i < y + SIDE_SQUARE) && (ActiveNodeCount == 0); i++) { \
		    __m256i active_reg; \
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + i*Cols + x)); \
			if (size == 32) { \
				__m128i excessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + i*Cols + x)); \
				excessFlowReg = _mm256_cvtepi16_epi32(excessFlowReg16); \
				cmpCurrHeight = _mm256_cmpgt_epi##size(heightMax, currentHeightReg); \
				cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, threshReg); \
				active_reg = _mm256_and_si256(cmpExcessFlow, cmpCurrHeight); \
									} else { \
				excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + i*Cols + x)); \
				cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg); \
				cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, threshReg); \
				cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax); \
				active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow); \
									} \
			mask = _mm256_movemask_epi8(active_reg); \
			if (mask != 0) { \
					ActiveNodeCount = 1; \
						} \
				} \
		pTableBlock[currentBlock + 1].row = y; \
		pTableBlock[currentBlock + 1].col = x; \
		if (ActiveNodeCount > 0) { \
			pTableBlock[lastActiveBlock].nextActiveBlock = (currentBlock + 1) - lastActiveBlock; \
			lastActiveBlock = currentBlock + 1; \
						} \
		pTableBlock[currentBlock + 1].nextActiveBlock = -1; \
		currentBlock++; \
			} \
} \


#define IS_ACTIVE_PIXEL(size, type) \
int blockInRow = Cols >> LOG_SIZE; \
__m256i currentHeightReg, excessFlowReg, cmpCurrHeight, cmpExcessFlow; \
HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1); \
__m256i zeroReg = _mm256_setzero_si256(); \
__m256i heightMax = _mm256_broadcast##type##_epi##size##(_mm_cvtsi32_si128(HEIGHT_MAX)); \
int mask; \
int currentBlock = 0; \
char ActiveNodeCount = 0; \
int prevIndex = 0; \
int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; \
while (indexActiveBlock != -1) { \
	int currentBlock = prevIndex + indexActiveBlock; \
	int row = pTableBlock[currentBlock].row; \
	int col = pTableBlock[currentBlock].col; \
	prevIndex = currentBlock; \
	indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; \
	for (int y = row; (y<(row+SIDE_SQUARE))&&(y < Rows)&&(ActiveNodeCount == 0); y++) { \
		    __m256i active_reg; \
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y*Cols + col)); \
			if (size == 32) { \
				__m128i excessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + y*Cols + col)); \
				excessFlowReg = _mm256_cvtepi16_epi32(excessFlowReg16); \
				cmpCurrHeight = _mm256_cmpgt_epi##size(heightMax, currentHeightReg); \
				cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, zeroReg); \
				active_reg = _mm256_and_si256(cmpExcessFlow, cmpCurrHeight); \
			} else { \
				excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y*Cols + col)); \
				cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg); \
				cmpExcessFlow = _mm256_cmpgt_epi##size(excessFlowReg, zeroReg); \
				cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax); \
				active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow); \
			} \
			mask = _mm256_movemask_epi8(active_reg); \
			if (mask != 0) { \
					ActiveNodeCount = 1; \
			} \
	} \
} \
return ActiveNodeCount; \

#define GraphCut_Push_HORIZ_SCALAR(size, type) \
HEIGHT_TYPE curHeight; \
short curExcessFlow; \
for (y = row ; y < row+16; y+=1) { \
	for (int x = col; x < (col + 16); x++) { \
	    if (x == col) { \
			curHeight = GetPix(pHeight, Cols, x, y); \
			curExcessFlow = GetPix(pExcessFlow, Cols, x, y); \
				} \
        if ((curHeight < HEIGHT_MAX) && (curExcessFlow > 0)) { \
			/* West neighbour (x-1, y) */ \
			if (x > 0) { \
				short northHeight = GetPix(pHeight, Cols, x - 1, y); \
				if (northHeight == curHeight - 1) { \
					flow = min(GetPix(pWestCap, Cols, x, y), curExcessFlow); \
					curExcessFlow-=flow; \
					PutPix(pExcessFlow, Cols, x, y, curExcessFlow); \
					AddPix(pExcessFlow, Cols, x - 1, y, flow); \
					AddPix(pWestCap, Cols, x, y, (short)-flow); \
					AddPix(pEastCap, Cols, x - 1, y, flow); \
								} \
						} \
			/* East neighbour (x+1, y) */ \
			if (x < Cols - 1) { \
			    short southHeight = GetPix(pHeight, Cols, x + 1, y); \
				short southExcessFlow = GetPix(pExcessFlow, Cols, x+1, y); \
				if (curExcessFlow > 0 &&  southHeight == curHeight - 1) { \
					flow = min(GetPix(pEastCap, Cols, x, y), curExcessFlow); \
					southExcessFlow+=flow; \
					AddPix(pExcessFlow, Cols, x, y, (short)-flow); \
					PutPix(pExcessFlow, Cols, x + 1, y, southExcessFlow); \
					AddPix(pEastCap, Cols, x, y, (short)-flow); \
					AddPix(pWestCap, Cols, x + 1, y, flow); \
								} \
				curExcessFlow = southExcessFlow; \
				curHeight = southHeight; \
						} \
				} \
		} \
} \

void AVX2Model_GraphCut_Relabel8x8(short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	RelabelBlock* pTableBlock,
	int Rows,
	int Cols)
{
	HEIGHT_TYPE temp;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i zeroReg = _mm256_setzero_si256();
	__m256i heightMax = _mm256_broadcastd_epi32(_mm_cvtsi32_si128(HEIGHT_MAX));
	__m256i oneReg = _mm256_set1_epi32(1);
	__declspec(align(32)) HEIGHT_TYPE newHeights[SIDE_SQUARE];

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
			__m128i excessFlowReg = _mm_loadu_si128((__m128i const*)(pExcessFlow + Cols*y + col));
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col));
			cmpExcessFlow = _mm256_cvtepi16_epi32(excessFlowReg);
			cmpExcessFlow = _mm256_cmpgt_epi32(cmpExcessFlow, zeroReg);
			cmpCurrHeight = _mm256_cmpgt_epi32(heightMax, currentHeightReg);
			active_reg = _mm256_and_si256(cmpExcessFlow, cmpCurrHeight);
			int mask = _mm256_movemask_epi8(active_reg);
			if (mask != 0) {
				__m256i capReg, cmpCap;
				__m256i newHeight = heightMax;
				/*North Neighbor*/
				if ((y - 1) >= 0) {
					__m256i northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col));
					capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pNorthCap + Cols*y + col)));
					northHeightReg = _mm256_add_epi32(northHeightReg, oneReg);
					cmpCap = _mm256_cmpgt_epi32(capReg, zeroReg);
					northHeightReg = _mm256_blendv_epi8(heightMax, northHeightReg, cmpCap);
					newHeight = _mm256_min_epu32(heightMax, northHeightReg);
				}
				/*South Neighbor*/
				if ((y + 1) < Rows) {
					__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col));
					capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pSouthCap + Cols*y + col)));
					southHeightReg = _mm256_add_epi32(southHeightReg, oneReg);
					cmpCap = _mm256_cmpgt_epi32(capReg, zeroReg);
					southHeightReg = _mm256_blendv_epi8(heightMax, southHeightReg, cmpCap);
					newHeight = _mm256_min_epu32(newHeight, southHeightReg);
				}
				/*East Neighbor*/
				if (((col + 1) + SIDE_SQUARE) < Cols) {
					__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col + 1));
					capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pEastCap + Cols*y + col)));
					eastHeightReg = _mm256_add_epi32(eastHeightReg, oneReg);
					cmpCap = _mm256_cmpgt_epi32((capReg), zeroReg);
					eastHeightReg = _mm256_blendv_epi8(heightMax, eastHeightReg, cmpCap);
					newHeight = _mm256_min_epu32(newHeight, eastHeightReg);
					newHeight = _mm256_blendv_epi8(currentHeightReg, newHeight, active_reg);
					_mm256_store_si256((__m256i *)newHeights, newHeight);
				}
				else {
					_mm256_store_si256((__m256i *)newHeights, newHeight);
					int copyMask = mask;
					for (int i = col; i < Cols; i++) {
						if (copyMask & 1)
						{
							temp = pHeight[Cols*y + i + 1] + 1;
							HEIGHT_TYPE currHeight = newHeights[i - col];
							HEIGHT_TYPE new_height = pEastCap[Cols*y + i] > 0 ? temp : HEIGHT_MAX;
							temp = new_height < currHeight ? new_height : currHeight;
							newHeights[i - col] = temp;
						}
						copyMask >>= 32 / SIDE_SQUARE;
					}
				}
				/*West Neighbor*/
				for (int i = 0; i < SIDE_SQUARE; i++)
				{
					if (mask & 1) {
						temp = pHeight[Cols*y + (((col + i - 1) < 0) ? 0 : (col + i - 1))] + 1;
						HEIGHT_TYPE currHeight = newHeights[i];
						HEIGHT_TYPE new_height = pWestCap[Cols*y + col + i] > 0 ? temp : HEIGHT_MAX;
						temp = new_height < currHeight ? new_height : currHeight;
						pHeight[Cols*y + col + i] = (HEIGHT_TYPE)temp;
					}
					mask >>= 32 / SIDE_SQUARE;
				}
			}
		}
	}
}

void AVX2Model_GraphCut_Relabel16x16(short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	RelabelBlock* pTableBlock,
	int Rows,
	int Cols)
{
	HEIGHT_TYPE temp;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i zeroReg = _mm256_setzero_si256();
	__m256i	heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	__m256i	oneReg = _mm256_set1_epi16(1);
	__declspec(align(32)) HEIGHT_TYPE newHeights[SIDE_SQUARE];

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
				if ((y - 1) >= 0) {
					__m256i northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col));
					capReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + Cols*y + col));
					northHeightReg = _mm256_add_epi16(northHeightReg, oneReg);
					cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg);
					northHeightReg = _mm256_blendv_epi8(heightMax, northHeightReg, cmpCap);
					newHeight = _mm256_min_epu16(heightMax, northHeightReg);
				}
				/*South Neighbor*/
				if ((y + 1) < Rows) {
					__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col));
					capReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + Cols*y + col));
					southHeightReg = _mm256_add_epi16(southHeightReg, oneReg);
					cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg);
					southHeightReg = _mm256_blendv_epi8(heightMax, southHeightReg, cmpCap);
					newHeight = _mm256_min_epu16(newHeight, southHeightReg);
				}
				/*East Neighbor*/
				if (((col + 1) + SIDE_SQUARE) < Cols) {
					__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col + 1));
					capReg = _mm256_loadu_si256((__m256i const*)(pEastCap + Cols*y + col));
					eastHeightReg = _mm256_add_epi16(eastHeightReg, oneReg);
					cmpCap = _mm256_cmpgt_epi16((capReg), zeroReg);
					eastHeightReg = _mm256_blendv_epi8(heightMax, eastHeightReg, cmpCap);
					newHeight = _mm256_min_epu16(newHeight, eastHeightReg);
					newHeight = _mm256_blendv_epi8(currentHeightReg, newHeight, active_reg);
					_mm256_store_si256((__m256i *)newHeights, newHeight);
				}
				else {
					_mm256_store_si256((__m256i *)newHeights, newHeight);
					int copyMask = mask;
					for (int i = col; i < Cols; i++) {
						if (copyMask & 1)
						{
							temp = pHeight[Cols*y + i + 1] + 1;
							HEIGHT_TYPE currHeight = newHeights[i - col];
							HEIGHT_TYPE new_height = pEastCap[Cols*y + i] > 0 ? temp : HEIGHT_MAX;
							temp = new_height < currHeight ? new_height : currHeight;
							newHeights[i - col] = temp;
						}
						copyMask >>= 32 / SIDE_SQUARE;
					}
				}
				/*West Neighbor*/
				for (int i = 0; i < SIDE_SQUARE; i++)
				{
					if (mask & 1) {
						temp = pHeight[Cols*y + (((col + i - 1) < 0) ? 0 : (col + i - 1))] + 1;
						HEIGHT_TYPE currHeight = newHeights[i];
						HEIGHT_TYPE new_height = pWestCap[Cols*y + col + i] > 0 ? temp : HEIGHT_MAX;
						temp = new_height < currHeight ? new_height : currHeight;
						pHeight[Cols*y + col + i] = (HEIGHT_TYPE)temp;
					}
					mask >>= 32 / SIDE_SQUARE;
				}
			}
		}
	}
}

void AVX2Model_Global_Relabel8x8(//short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	RelabelBlock* pTableBlock,
	int Rows,
	int Cols)
{
	int y;
	HEIGHT_TYPE temp;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	HEIGHT_TYPE NewH = 0;
	HEIGHT_TYPE NewHeightCount = HEIGHT_MAX;
	int PrevCount;
	__m256i currentHeightReg; 
	__m256i zeroReg = _mm256_setzero_si256(); 
	__m256i heightMax = _mm256_broadcastd_epi32(_mm_cvtsi32_si128(HEIGHT_MAX)); 
	__m256i oneReg = _mm256_set1_epi32(1); 
	__declspec(align(32)) HEIGHT_TYPE newHeights[SIDE_SQUARE]; 
	/* Init Height */ 
	int prevIndex = 0; 
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; 
	while (indexActiveBlock != -1) { 
		int currentBlock = prevIndex + indexActiveBlock; 
		int row = pTableBlock[currentBlock].row; 
		int col = pTableBlock[currentBlock].col; 
		prevIndex = currentBlock; 
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; 
		for (y = row; y < (row + SIDE_SQUARE); y++) { 
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col)); 
			__m256i cmpHeightreg = _mm256_cmpeq_epi32(currentHeightReg, zeroReg); 
			currentHeightReg = _mm256_andnot_si256(cmpHeightreg, heightMax); 
			_mm256_storeu_si256((__m256i *)(pHeight + Cols*y + col), currentHeightReg); 
		} 
	} 
	/* Global relabel */ 
	int numIter = 0; 
	do { 
		NewHeightCount = 0; 
		prevIndex = 0; 
		indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; 
				while (indexActiveBlock != -1) { 
			int currentBlock = prevIndex + indexActiveBlock; 
			int row = pTableBlock[currentBlock].row; 
			int col = pTableBlock[currentBlock].col; 
			prevIndex = currentBlock; 
			indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; 
			for (y = row; y < (row + SIDE_SQUARE); y++) { 
				__m256i capReg, newHeight, cmpCap; 
				currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col)); 
				newHeight = currentHeightReg; 
				/*North Neighbor*/ 
				if ((y-1) >= 0) { 
				__m256i northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col)); 
				capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pNorthCap + Cols*y + col))); 
				northHeightReg = _mm256_add_epi32(northHeightReg, oneReg); 
				cmpCap = _mm256_cmpgt_epi32(capReg, zeroReg); 
				northHeightReg = _mm256_blendv_epi8(currentHeightReg, northHeightReg, cmpCap); 
				newHeight = _mm256_min_epu32(currentHeightReg, northHeightReg); 
				} 
				/*South Neighbor*/ 
				if ((y+1) < Rows) { 
				__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col)); 
				capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pSouthCap + Cols*y + col))); 
				southHeightReg = _mm256_add_epi32(southHeightReg, oneReg); 
				cmpCap = _mm256_cmpgt_epi32(capReg, zeroReg); 
				southHeightReg = _mm256_blendv_epi8(newHeight, southHeightReg, cmpCap); 
				newHeight = _mm256_min_epu32(newHeight, southHeightReg); 
				} 
				/*East Neighbor*/ 
				if ((col + 1 + SIDE_SQUARE) < Cols) { 
				__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col + 1)); 
				capReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pEastCap + Cols*y + col))); 
				eastHeightReg = _mm256_add_epi32(eastHeightReg, oneReg);
				cmpCap = _mm256_cmpgt_epi32((capReg), zeroReg);
				eastHeightReg = _mm256_blendv_epi8(newHeight, eastHeightReg, cmpCap);
				newHeight = _mm256_min_epu32(newHeight, eastHeightReg); 
				_mm256_store_si256((__m256i *)newHeights, newHeight); 
				} else { 
					_mm256_store_si256((__m256i *)newHeights, newHeight); 
					for (int i = col; i < Cols; i++) { 
						temp = pHeight[Cols*y + i + 1] + 1; 
						HEIGHT_TYPE new_height = newHeights[i - col]; 
						temp = pEastCap[Cols*y + i] > 0 ? temp : new_height; 
						newHeights[i - col] = new_height < temp ? new_height : temp; 
					} 
				} 
				/*West Neighbor*/ 
				temp = pHeight[Cols*y + (((col - 1)<0)?0:(col-1))] + 1; 
				for (int i = 0; i < SIDE_SQUARE; i++) { 
						HEIGHT_TYPE new_height = newHeights[i]; 
						temp = pWestCap[Cols*y+ col + i] > 0 ? temp : new_height; 
						new_height = new_height < temp ? new_height : temp; 
						HEIGHT_TYPE current = pHeight[Cols*y + col + i]; 
						if (new_height != current) { 
						   pHeight[Cols*y + col + i] = (HEIGHT_TYPE)new_height; 
						   NewHeightCount++; 
						} 
						temp = new_height+1; 
				 } 
			} 
		} 
	} while (NewHeightCount > 0 && ++numIter < 32);

}


void AVX2Model_Global_Relabel16x16(//short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	RelabelBlock* pTableBlock,
	int Rows,
	int Cols)
{
	int y;
	HEIGHT_TYPE temp;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	HEIGHT_TYPE NewH = 0;
	HEIGHT_TYPE NewHeightCount = HEIGHT_MAX;
	int PrevCount;
	__m256i currentHeightReg; 
	__m256i zeroReg = _mm256_setzero_si256(); 
	__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX)); 
	__m256i oneReg = _mm256_set1_epi16(1); 
	__declspec(align(32)) HEIGHT_TYPE newHeights[SIDE_SQUARE]; 
	/* Init Height */ 
	int prevIndex = 0; 
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; 
	while (indexActiveBlock != -1) { 
		int currentBlock = prevIndex + indexActiveBlock; 
		int row = pTableBlock[currentBlock].row; 
		int col = pTableBlock[currentBlock].col; 
		prevIndex = currentBlock; 
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; 
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
		NewHeightCount = 0; 
		prevIndex = 0; 
		indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; 
		while (indexActiveBlock != -1) { 
			int currentBlock = prevIndex + indexActiveBlock; 
			int row = pTableBlock[currentBlock].row; 
			int col = pTableBlock[currentBlock].col; 
			prevIndex = currentBlock; 
			indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; 
			for (y = row; y < (row + SIDE_SQUARE); y++) { 
				__m256i capReg, newHeight, cmpCap; 
				currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col)); 
				newHeight = currentHeightReg; 
				/*North Neighbor*/ 
				if ((y-1) >= 0) { 
				__m256i northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y - 1) + col)); 
				capReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + Cols*y + col)); 
				northHeightReg = _mm256_add_epi16(northHeightReg, oneReg); 
				cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg); 
				northHeightReg = _mm256_blendv_epi8(currentHeightReg, northHeightReg, cmpCap); 
				newHeight = _mm256_min_epu16(currentHeightReg, northHeightReg); 
				} 
				/*South Neighbor*/ 
				if ((y+1) < Rows) { 
				__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + 1) + col)); 
				capReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + Cols*y + col)); 
				southHeightReg = _mm256_add_epi16(southHeightReg, oneReg); 
				cmpCap = _mm256_cmpgt_epi16(capReg, zeroReg); 
				southHeightReg = _mm256_blendv_epi8(newHeight, southHeightReg, cmpCap); 
				newHeight = _mm256_min_epu16(newHeight, southHeightReg); 
				} 
				/*East Neighbor*/ 
				if ((col + 1 + SIDE_SQUARE) < Cols) { 
				__m256i eastHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*y + col + 1)); 
				capReg = _mm256_loadu_si256((__m256i const*)(pEastCap + Cols*y + col)); 
				eastHeightReg = _mm256_add_epi16(eastHeightReg, oneReg); 
				cmpCap = _mm256_cmpgt_epi16((capReg), zeroReg); 
				eastHeightReg = _mm256_blendv_epi8(newHeight, eastHeightReg, cmpCap); 
				newHeight = _mm256_min_epu16(newHeight, eastHeightReg); 
				_mm256_store_si256((__m256i *)newHeights, newHeight); 
				} else { 
					_mm256_store_si256((__m256i *)newHeights, newHeight); 
					for (int i = col; i < Cols; i++) { 
						temp = pHeight[Cols*y + i + 1] + 1; 
						HEIGHT_TYPE new_height = newHeights[i - col]; 
						temp = pEastCap[Cols*y + i] > 0 ? temp : new_height; 
						newHeights[i - col] = new_height < temp ? new_height : temp; 
					} 
				} 
				/*West Neighbor*/ 
				temp = pHeight[Cols*y + (((col - 1)<0)?0:(col-1))] + 1; 
				for (int i = 0; i < SIDE_SQUARE; i++) { 
						HEIGHT_TYPE new_height = newHeights[i]; 
						temp = pWestCap[Cols*y+ col + i] > 0 ? temp : new_height; 
						new_height = new_height < temp ? new_height : temp; 
						HEIGHT_TYPE current = pHeight[Cols*y + col + i]; 
						if (new_height != current) { 
						   pHeight[Cols*y + col + i] = (HEIGHT_TYPE)new_height; 
						   NewHeightCount++; 
						} 
						temp = new_height+1; 
				} 
			} 
		} 
	} while (NewHeightCount > 0 && ++numIter < 32);
}

void AVX2Model_GraphCut_Vertical_Push8x8(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pNorthCap, short * pSouthCap, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	int blockInRow = Cols >> LOG_SIZE;
	__m256i zeroReg = _mm256_setzero_si256(); 
	__m256i heightMax = _mm256_broadcastd_epi32(_mm_cvtsi32_si128(HEIGHT_MAX)); 
	__m256i oneReg = _mm256_set1_epi32(1); 
	__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight; 
	__m256i northCapReg, southCapReg, northExcessFlowReg, southExcessFlowReg, northHeightReg, cmpHeight, newExcessFlowReg, newNorthExcessFlowReg,flowReg; 
	__m256i heightReg, nextHeightReg, nextExcessFlowReg, westCapReg, nextWestCapReg, eastCapReg, nextEastCapReg; 
	int prevIndex = 0; 
	int i,j;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; 
	while (indexActiveBlock != -1) 
	{ 
		int y; 
		int currentBlock = prevIndex + indexActiveBlock; 
		int row = pTableBlock[currentBlock].row; 
		int col = pTableBlock[currentBlock].col; 
		
		prevIndex = currentBlock; 
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; 
		for (y = 0; y < SIDE_SQUARE; y += 1) {
			/* Check for active node */ 
			__m256i active_reg; 
			
			if (y == 0) {
				__m128i excessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + Cols*(y + row) + col)); 
				currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + row) + col)); 
				excessFlowReg = _mm256_cvtepi16_epi32(excessFlowReg16); 
			}
			cmpExcessFlow = _mm256_cmpgt_epi32(excessFlowReg, zeroReg); 
			cmpCurrHeight = _mm256_cmpgt_epi32(heightMax, currentHeightReg); 
			active_reg = _mm256_and_si256(cmpExcessFlow, cmpCurrHeight); 				
			if ((y + row) > 0) { 
			/*North Neighbor*/ 
			if (y == 0) { 
				northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + row - 1) + col)); 
				northCapReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pNorthCap + Cols*(y + row) + col))); 
			} 
			northHeightReg = _mm256_add_epi32(northHeightReg, oneReg); 
			cmpHeight = _mm256_cmpeq_epi32(northHeightReg, currentHeightReg); 
			flowReg = _mm256_min_epi32(northCapReg, excessFlowReg); 
			if (y == 0) { 
				__m128i northExcessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + Cols*(y + row -1) + col)); 
				__m128i southCapReg16 = _mm_loadu_si128((__m128i const*)(pSouthCap + Cols*(y + row -1) + col)); 
				northExcessFlowReg = _mm256_cvtepi16_epi32(northExcessFlowReg16); 
				southCapReg = _mm256_cvtepi16_epi32(southCapReg16); 
			} 
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight); 
			newExcessFlowReg = _mm256_sub_epi32(excessFlowReg, flowReg); 
			newNorthExcessFlowReg = _mm256_add_epi32(northExcessFlowReg, flowReg); 
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight); 
			northExcessFlowReg = _mm256_blendv_epi8(northExcessFlowReg, newNorthExcessFlowReg, cmpHeight); 
			__m256i newNorthCapReg = _mm256_sub_epi32(northCapReg, flowReg); 
			__m256i newSouthCapReg = _mm256_add_epi32(southCapReg, flowReg); 
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight); 
			northExcessFlowReg = _mm256_packs_epi32(northExcessFlowReg, northExcessFlowReg); 
			_mm_storeu_si128((__m128i*)(pExcessFlow + Cols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(northExcessFlowReg, 0x8))); 
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight); 
			northCapReg = _mm256_packs_epi32(northCapReg, northCapReg); 
			_mm_storeu_si128((__m128i*) (pNorthCap + Cols*(y + row) + col),_mm256_castsi256_si128(_mm256_permute4x64_epi64(northCapReg, 0x8))); 
			southCapReg = _mm256_packs_epi32(southCapReg, southCapReg); 
			_mm_storeu_si128((__m128i*) (pSouthCap + Cols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(southCapReg, 0x8))); 
		} 
		if ((row + y) < (Rows - 1)) { 
			/*South Neighbor*/ 
			cmpExcessFlow = _mm256_cmpgt_epi32(excessFlowReg, zeroReg); 
			__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + row + 1) + col)); 
			southCapReg = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i const*)(pSouthCap + Cols*(y + row) + col))); 
			cmpHeight = _mm256_cmpeq_epi32(_mm256_add_epi32(southHeightReg, oneReg), currentHeightReg); 
			flowReg = _mm256_min_epi32(southCapReg, excessFlowReg); 
			cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight); 
			__m128i southExcessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + Cols*(y + row + 1) + col)); 
			__m128i northCapReg16 = _mm_loadu_si128((__m128i const*)(pNorthCap + Cols*(y + row + 1) + col)); 
			southExcessFlowReg = _mm256_cvtepi16_epi32(southExcessFlowReg16); 
			northCapReg = _mm256_cvtepi16_epi32(northCapReg16); 
			cmpHeight = _mm256_and_si256(active_reg, cmpHeight); 
			newExcessFlowReg = _mm256_sub_epi32(excessFlowReg, flowReg); 
			__m256i newSouthExcessFlowReg = _mm256_add_epi32(southExcessFlowReg, flowReg); 
			excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight); 
			__m256i newSouthCapReg = _mm256_sub_epi32(southCapReg, flowReg); 
			southExcessFlowReg = _mm256_blendv_epi8(southExcessFlowReg, newSouthExcessFlowReg, cmpHeight); 
			__m256i newNorthCapReg = _mm256_add_epi32(northCapReg, flowReg); 
			southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight); 
			northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight); 
			northHeightReg = currentHeightReg; 
			northExcessFlowReg = excessFlowReg; 
			excessFlowReg = southExcessFlowReg; 
			currentHeightReg = southHeightReg; 
		} 
	  } 
	  if ((y + row) < Rows) { 
			northExcessFlowReg = _mm256_packs_epi32(northExcessFlowReg, northExcessFlowReg); 
			_mm_storeu_si128((__m128i*)(pExcessFlow + Cols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(northExcessFlowReg, 0x8))); 
			excessFlowReg = _mm256_packs_epi32(excessFlowReg, excessFlowReg); 
			_mm_storeu_si128((__m128i*)(pExcessFlow + Cols*(y + row) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(excessFlowReg, 0x8))); 
			southCapReg = _mm256_packs_epi32(southCapReg, southCapReg); 
			_mm_storeu_si128((__m128i*) (pSouthCap + Cols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(southCapReg, 0x8))); 
			northCapReg = _mm256_packs_epi32(northCapReg, northCapReg); 
			_mm_storeu_si128((__m128i*) (pNorthCap + Cols*(y + row) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(northCapReg, 0x8))); 
	   } else { 
			excessFlowReg = _mm256_packs_epi32(excessFlowReg, excessFlowReg); 
			_mm_storeu_si128((__m128i*)(pExcessFlow + Cols*(y + row -1) + col), _mm256_castsi256_si128(_mm256_permute4x64_epi64(excessFlowReg, 0x8))); 
	   } 
   } 

}

void AVX2Model_GraphCut_Vertical_Push16x16(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pNorthCap, short * pSouthCap, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	int blockInRow = Cols >> LOG_SIZE;
	__m256i zeroReg = _mm256_setzero_si256();
	__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	__m256i oneReg = _mm256_set1_epi16(1);
	__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight;
	__m256i northCapReg, southCapReg, northExcessFlowReg, southExcessFlowReg, northHeightReg, cmpHeight, newExcessFlowReg, newNorthExcessFlowReg, flowReg;
	__m256i heightReg, nextHeightReg, nextExcessFlowReg, westCapReg, nextWestCapReg, eastCapReg, nextEastCapReg;
	int prevIndex = 0;
	int i, j;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
	while (indexActiveBlock != -1)
	{
		int y;
		int currentBlock = prevIndex + indexActiveBlock;
		int row = pTableBlock[currentBlock].row;
		int col = pTableBlock[currentBlock].col;
		prevIndex = currentBlock;
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
		for (y = 0; y < SIDE_SQUARE; y += 1) {
			/* Check for active node */ 
			__m256i active_reg; 
			if (y == 0) { 
				excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + Cols*(y+row) + col)); 
				currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y+row) + col)); 
			} 
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg); 
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg); 
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax); 
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow); 
			if ((y + row) > 0) { 
				/*North Neighbor*/ 
				if (y == 0) { 
					northHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + row - 1) + col)); 
					northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + Cols*(y + row) + col)); 
				} 
				northHeightReg = _mm256_add_epi16(northHeightReg, oneReg); 
				cmpHeight = _mm256_cmpeq_epi16(northHeightReg, currentHeightReg); 
				flowReg = _mm256_min_epi16(northCapReg, excessFlowReg); 
				if (y == 0) { 
					northExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + Cols*(y + row -1) + col)); 
					southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + Cols*(y + row -1) + col)); 
				} 
				cmpHeight = _mm256_and_si256(active_reg, cmpHeight); 
				newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg);
				newNorthExcessFlowReg = _mm256_add_epi16(northExcessFlowReg, flowReg); 
				excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight); 
				northExcessFlowReg = _mm256_blendv_epi8(northExcessFlowReg, newNorthExcessFlowReg, cmpHeight); 
				__m256i newNorthCapReg = _mm256_sub_epi16(northCapReg, flowReg); 
				__m256i newSouthCapReg = _mm256_add_epi16(southCapReg, flowReg); 
				northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight); 
				_mm256_storeu_si256((__m256i*)(pExcessFlow + Cols*(y + row -1) + col), northExcessFlowReg); 
				southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight); 
				_mm256_storeu_si256((__m256i*)(pNorthCap + Cols*(y + row) + col), northCapReg); 
				_mm256_storeu_si256((__m256i*)(pSouthCap + Cols*(y + row -1) + col), southCapReg); 
			} 
			if ((row + y) < (Rows - 1)) { 
				/*South Neighbor*/ 
				cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg); 
				__m256i southHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + Cols*(y + row + 1) + col)); 
				southCapReg = _mm256_loadu_si256((__m256i const*)(pSouthCap + Cols*(y + row) + col)); 
				cmpHeight = _mm256_cmpeq_epi16(_mm256_add_epi16(southHeightReg, oneReg), currentHeightReg); 
				flowReg = _mm256_min_epi16(southCapReg, excessFlowReg); 
				cmpHeight = _mm256_and_si256(cmpExcessFlow, cmpHeight); 
				southExcessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + Cols*(y + row + 1) + col)); 
				northCapReg = _mm256_loadu_si256((__m256i const*)(pNorthCap + Cols*(y + row + 1) + col)); 
				cmpHeight = _mm256_and_si256(active_reg, cmpHeight); 
				newExcessFlowReg = _mm256_sub_epi16(excessFlowReg, flowReg); 
				__m256i newSouthExcessFlowReg = _mm256_add_epi16(southExcessFlowReg, flowReg); 
				excessFlowReg = _mm256_blendv_epi8(excessFlowReg, newExcessFlowReg, cmpHeight); 
				__m256i newSouthCapReg = _mm256_sub_epi16(southCapReg, flowReg); 
				southExcessFlowReg = _mm256_blendv_epi8(southExcessFlowReg, newSouthExcessFlowReg, cmpHeight); 
				__m256i newNorthCapReg = _mm256_add_epi16(northCapReg, flowReg); 
				southCapReg = _mm256_blendv_epi8(southCapReg, newSouthCapReg, cmpHeight); 
				northCapReg = _mm256_blendv_epi8(northCapReg, newNorthCapReg, cmpHeight); 
				northHeightReg = currentHeightReg; 
				northExcessFlowReg = excessFlowReg; 
				excessFlowReg = southExcessFlowReg; 
				currentHeightReg = southHeightReg; 
			} 
		} 
		if ((y + row) < Rows) { 
			_mm256_storeu_si256((__m256i*)(pExcessFlow + Cols*(y + row -1) + col), northExcessFlowReg); 
			_mm256_storeu_si256((__m256i*)(pExcessFlow + Cols*(y + row) + col), excessFlowReg); 
			_mm256_storeu_si256((__m256i*)(pSouthCap + Cols*(y + row -1) + col), southCapReg); 
			_mm256_storeu_si256((__m256i*)(pNorthCap + Cols*(y + row) + col), northCapReg); 
		} else { 
			_mm256_storeu_si256((__m256i*)(pExcessFlow + Cols*(y + row -1) + col), excessFlowReg); 
		} 
	}

}

void AVX2Model_GraphCut_Horizontal_Push8x8(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pWestCap, short * pEastCap, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	int blockInRow = Cols >> LOG_SIZE;
	__m256i heightT[16];
	__m256i excessFlowT[16];
	__m256i westCapT[16];
	__m256i eastCapT[16];
	__m256i zeroReg = _mm256_setzero_si256(); 
	__m256i heightMax = _mm256_broadcastd_epi32(_mm_cvtsi32_si128(HEIGHT_MAX)); 
	__m256i oneReg = _mm256_set1_epi32(1); 
	__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight; 
	__m256i northCapReg, southCapReg, northExcessFlowReg, southExcessFlowReg, northHeightReg, cmpHeight, newExcessFlowReg, newNorthExcessFlowReg,flowReg; 
	__m256i heightReg, nextHeightReg, nextExcessFlowReg, westCapReg, nextWestCapReg, eastCapReg, nextEastCapReg; 
	HEIGHT_TYPE heightH[SIDE_SQUARE * (SIDE_SQUARE+2)]; 
	short excessFlowH[SIDE_SQUARE * (SIDE_SQUARE + 2)]; 
	short westCapH[SIDE_SQUARE * (SIDE_SQUARE + 2)]; 
	short eastCapH[SIDE_SQUARE * (SIDE_SQUARE + 2)]; 
	int prevIndex = 0; 
	int i,j;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; 
	prevIndex = 0; 
	indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; 
	while (indexActiveBlock != -1) 
	{ 
		int y; 
		int currentBlock = prevIndex + indexActiveBlock; 
		int row = pTableBlock[currentBlock].row; 
		int col = pTableBlock[currentBlock].col; 
		prevIndex = currentBlock; 
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; 
		TRANSPOSE(0, 32, row, col, Cols, pHeight, pExcessFlow, pWestCap, pEastCap, 0, 0, SIDE_SQUARE, heightH, excessFlowH, westCapH, eastCapH) 
		HEIGHT_TYPE *pHeightH = heightH + SIDE_SQUARE; 
		short* pExcessFlowH = excessFlowH + SIDE_SQUARE; 
		short* pWestCapH = westCapH + SIDE_SQUARE; 
		short* pEastCapH = eastCapH + SIDE_SQUARE; 
		GRAPHCUT_PUSH_BLOCK(32, 0, 0, SIDE_SQUARE, col, Cols, pHeightH, pExcessFlowH, pWestCapH, pEastCapH) 
		TRANSPOSE(1, 32, 0, 0, SIDE_SQUARE, pHeightH, pExcessFlowH, pWestCapH, pEastCapH, row, col, Cols, pHeight, pExcessFlow, pWestCap, pEastCap) 
	} 
}

void AVX2Model_GraphCut_Horizontal_Push16x16(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pWestCap, short * pEastCap, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	int blockInRow = Cols >> LOG_SIZE;
	__m256i heightT[16];
	__m256i excessFlowT[16];
	__m256i westCapT[16];
	__m256i eastCapT[16];
	__m256i zeroReg = _mm256_setzero_si256(); 
	__m256i heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	__m256i oneReg = _mm256_set1_epi16(1);
	__m256i excessFlowReg, cmpExcessFlow, currentHeightReg, cmpCurrHeight; 
	__m256i northCapReg, southCapReg, northExcessFlowReg, southExcessFlowReg, northHeightReg, cmpHeight, newExcessFlowReg, newNorthExcessFlowReg,flowReg; 
	__m256i heightReg, nextHeightReg, nextExcessFlowReg, westCapReg, nextWestCapReg, eastCapReg, nextEastCapReg; 
	HEIGHT_TYPE heightH[SIDE_SQUARE * (SIDE_SQUARE+2)]; 
	short excessFlowH[SIDE_SQUARE * (SIDE_SQUARE + 2)]; 
	short westCapH[SIDE_SQUARE * (SIDE_SQUARE + 2)]; 
	short eastCapH[SIDE_SQUARE * (SIDE_SQUARE + 2)]; 
	int i,j;
	int prevIndex = 0; 
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; 
	while (indexActiveBlock != -1) 
	{ 
		int y; 
		int currentBlock = prevIndex + indexActiveBlock; 
		int row = pTableBlock[currentBlock].row; 
		int col = pTableBlock[currentBlock].col; 
		prevIndex = currentBlock; 
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; 
		TRANSPOSE(0, 16, row, col, Cols, pHeight, pExcessFlow, pWestCap, pEastCap, 0, 0, SIDE_SQUARE, heightH, excessFlowH, westCapH, eastCapH) 
		HEIGHT_TYPE *pHeightH = heightH + SIDE_SQUARE; 
		short* pExcessFlowH = excessFlowH + SIDE_SQUARE; 
		short* pWestCapH = westCapH + SIDE_SQUARE; 
		short* pEastCapH = eastCapH + SIDE_SQUARE; 
		GRAPHCUT_PUSH_BLOCK(16, 0, 0, SIDE_SQUARE, col, Cols, pHeightH, pExcessFlowH, pWestCapH, pEastCapH) 
		TRANSPOSE(1, 16, 0, 0, SIDE_SQUARE, pHeightH, pExcessFlowH, pWestCapH, pEastCapH, row, col, Cols, pHeight, pExcessFlow, pWestCap, pEastCap) 
	} 
}

void AVX2ActiveBlocks8x8(short * pExcessFlow, HEIGHT_TYPE * pHeight, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	char ActiveNodeCount; 
	int blockInRow = Cols >> LOG_SIZE; 
	__m256i currentHeightReg, excessFlowReg, cmpCurrHeight, cmpExcessFlow; 
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1); 
	__m256i threshReg = _mm256_set1_epi32(-100); 
	__m256i heightMax = _mm256_broadcastd_epi32(_mm_cvtsi32_si128(HEIGHT_MAX)); 
	int lastActiveBlock = 0; 
	pTableBlock[lastActiveBlock].nextActiveBlock = -1; 
	int mask; 
	int currentBlock = 0; 
	int activeBlocks = 0; 
	for (int y = 0; y < Rows; y += SIDE_SQUARE) { 
		for (int x = 0; x < Cols; x += SIDE_SQUARE) { 
			ActiveNodeCount = 0; 
			for (int i = y; (i < y + SIDE_SQUARE) && (ActiveNodeCount == 0); i++) { 
				__m256i active_reg; 
				currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + i*Cols + x)); 
				__m128i excessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + i*Cols + x)); 
				excessFlowReg = _mm256_cvtepi16_epi32(excessFlowReg16); 
				cmpCurrHeight = _mm256_cmpgt_epi32(heightMax, currentHeightReg); 
				cmpExcessFlow = _mm256_cmpgt_epi32(excessFlowReg, threshReg); 
				active_reg = _mm256_and_si256(cmpExcessFlow, cmpCurrHeight); 
				mask = _mm256_movemask_epi8(active_reg); 
				if (mask != 0) { 
						ActiveNodeCount = 1; 
				} 
			} 
			pTableBlock[currentBlock + 1].row = y; 
			pTableBlock[currentBlock + 1].col = x; 
			if (ActiveNodeCount > 0) { 
				pTableBlock[lastActiveBlock].nextActiveBlock = (currentBlock + 1) - lastActiveBlock; 
				lastActiveBlock = currentBlock + 1; 
			} 
			pTableBlock[currentBlock + 1].nextActiveBlock = -1; 
			currentBlock++; 
		} 
	} 
}

void AVX2ActiveBlocks16x16(short * pExcessFlow, HEIGHT_TYPE * pHeight, RelabelBlock* pTableBlock, int Rows, int Cols)
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
	for (int y = 0; y < Rows; y += SIDE_SQUARE) { 
		for (int x = 0; x < Cols; x += SIDE_SQUARE) { 
			ActiveNodeCount = 0; 
			for (int i = y; (i < y + SIDE_SQUARE) && (ActiveNodeCount == 0); i++) { 
				__m256i active_reg; 
				currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + i*Cols + x)); 
				excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + i*Cols + x)); 
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
				pTableBlock[lastActiveBlock].nextActiveBlock = (currentBlock + 1) - lastActiveBlock; 
				lastActiveBlock = currentBlock + 1; 
			} 
			pTableBlock[currentBlock + 1].nextActiveBlock = -1; 
			currentBlock++; 
		} 
	} 

}

int AVX2ActivePixels8x8(short * pExcessFlow, HEIGHT_TYPE * pHeight, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	__m256i heightMax;
int blockInRow = Cols >> LOG_SIZE; 
__m256i currentHeightReg, excessFlowReg, cmpCurrHeight, cmpExcessFlow; 
HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1); 
__m256i zeroReg = _mm256_setzero_si256(); 
heightMax = _mm256_broadcastd_epi32(_mm_cvtsi32_si128(HEIGHT_MAX));
int mask; 
int currentBlock = 0; 
char ActiveNodeCount = 0; 
int prevIndex = 0; 
int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock; 
	while (indexActiveBlock != -1) { 
	int currentBlock = prevIndex + indexActiveBlock; 
	int row = pTableBlock[currentBlock].row; 
	int col = pTableBlock[currentBlock].col; 
	prevIndex = currentBlock; 
	indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock; 
	for (int y = row; (y < (row + SIDE_SQUARE)) && (y < Rows) && (ActiveNodeCount == 0); y++) {
		    __m256i active_reg; 
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y*Cols + col));
			__m128i excessFlowReg16 = _mm_loadu_si128((__m128i const*)(pExcessFlow + y*Cols + col)); 
			excessFlowReg = _mm256_cvtepi16_epi32(excessFlowReg16); 
			cmpCurrHeight = _mm256_cmpgt_epi32(heightMax, currentHeightReg); 
			cmpExcessFlow = _mm256_cmpgt_epi32(excessFlowReg, zeroReg); 
			active_reg = _mm256_and_si256(cmpExcessFlow, cmpCurrHeight); 
			mask = _mm256_movemask_epi8(active_reg); 
			if (mask != 0) { 
					ActiveNodeCount = 1; 
			} 
		} 
	} 
return ActiveNodeCount; 
}

int AVX2ActivePixels16x16(short * pExcessFlow, HEIGHT_TYPE * pHeight, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	__m256i heightMax;
	int blockInRow = Cols >> LOG_SIZE;
	__m256i currentHeightReg, excessFlowReg, cmpCurrHeight, cmpExcessFlow;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	__m256i zeroReg = _mm256_setzero_si256();
	heightMax = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(HEIGHT_MAX));
	int mask;
	int currentBlock = 0;
	char ActiveNodeCount = 0;
	int prevIndex = 0;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
	while (indexActiveBlock != -1) {
		int currentBlock = prevIndex + indexActiveBlock;
		int row = pTableBlock[currentBlock].row;
		int col = pTableBlock[currentBlock].col;
		prevIndex = currentBlock;
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
		for (int y = row; (y < (row + SIDE_SQUARE)) && (y < Rows) && (ActiveNodeCount == 0); y++) {
			__m256i active_reg;
			currentHeightReg = _mm256_loadu_si256((__m256i const*)(pHeight + y*Cols + col));
			excessFlowReg = _mm256_loadu_si256((__m256i const*)(pExcessFlow + y*Cols + col));
			cmpCurrHeight = _mm256_min_epu16(heightMax, currentHeightReg);
			cmpExcessFlow = _mm256_cmpgt_epi16(excessFlowReg, zeroReg);
			cmpCurrHeight = _mm256_cmpeq_epi16(cmpCurrHeight, heightMax);
			active_reg = _mm256_andnot_si256(cmpCurrHeight, cmpExcessFlow);	
			mask = _mm256_movemask_epi8(active_reg);
			if (mask != 0) {
				ActiveNodeCount = 1;
			}
		}
	}
	return ActiveNodeCount;
}

void CTamarModel_GraphCut_Relabel(short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	RelabelBlock* pTableBlock,
	int Rows,
	int Cols)
{
	// Scan line based Relabel
	HEIGHT_TYPE NewHeight, temp;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	int blockInRow = Cols >> LOG_SIZE;
	bool rtn = false;
	int prevIndex = 0;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
	while (indexActiveBlock != -1)
	{
		int currentBlock = prevIndex + indexActiveBlock;
		int row = pTableBlock[currentBlock].row;
		int col = pTableBlock[currentBlock].col;
		prevIndex = currentBlock;
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
		for (int y = row; y < row + SIDE_SQUARE; y++)
		{
			/*HEIGHT_TYPE NewHeight[16];
			unsigned short mask[16];*/
			for (int x = col; x < col + SIDE_SQUARE; x++)
			{
				/*NewHeight[x-col] = HEIGHT_MAX;
				mask[x-col] = (GetPix(pExcessFlow, Cols, x, y) > 0 && GetPix(pHeight, Cols, x, y) < HEIGHT_MAX) ? 1 : 0;
			}
			for (int x = col; x < col + SIDE_SQUARE; x++)
			{
				if (mask[x-col]) {
				// North neighbour
				temp = GetPix(pHeight, Cols, x, (y - 1 < 0) ? 0 : y - 1) + 1;
				if (GetPix(pNorthCap, Cols, x, y) > 0 && temp < NewHeight[x-col])
					NewHeight[x-col] = temp;

				// South neighbour
				temp = GetPix(pHeight, Cols, x, (y + 1 >= Rows) ? Rows - 1 : y + 1) + 1;
				if (GetPix(pSouthCap, Cols, x, y) > 0 && temp < NewHeight[x-col])
					NewHeight[x-col] = temp;
				}
			}
			for (int x = col; x < col + SIDE_SQUARE; x++)
			{
				if (mask[x-col]) {
				// West neighbour
				temp = GetPix(pHeight, Cols, (x - 1 < 0) ? 0 : x - 1, y) + 1;
				if (GetPix(pWestCap, Cols, x, y) > 0 && temp < NewHeight[x-col]) {
					NewHeight[x-col] = temp;
				}

				// East neighbour
				temp = GetPix(pHeight, Cols, (x + 1 >= Cols) ? Cols - 1 : x + 1, y) + 1;
				if (GetPix(pEastCap, Cols, x, y) > 0 && temp < NewHeight[x-col])
					NewHeight[x-col] = temp;

				// Update Height so the next pixel to the right will use this new value
				PutPix(pHeight, Cols, x, y, NewHeight[x-col]);
			    }
			}
		}*/
				// Check for active node
				short CurExcessFlow = GetPix(pExcessFlow, Cols, x, y);
				HEIGHT_TYPE CurHeight = GetPix(pHeight, Cols, x, y);
				if (CurExcessFlow > 0 && CurHeight < HEIGHT_MAX) {

					NewHeight = HEIGHT_MAX;

					// North neighbour
					temp = GetPix(pHeight, Cols, x, ((y - 1) < 0) ? 0 : (y - 1)) + 1;
					if (GetPix(pNorthCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// South neighbour
					temp = GetPix(pHeight, Cols, x, ((y + 1) >= Rows) ? (Rows - 1) : (y + 1)) + 1;
					if (GetPix(pSouthCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// West neighbour
					temp = GetPix(pHeight, Cols, ((x - 1) < 0) ? 0 : (x - 1), y) + 1;
					if (GetPix(pWestCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// East neighbour
					temp = GetPix(pHeight, Cols, ((x + 1) >= Cols) ? (Cols - 1) : (x + 1), y) + 1;
					if (GetPix(pEastCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					PutPix(pHeight, Cols, x, y, NewHeight);
				}
			}
		}
	}
}

void CTamarModel_Global_Relabel( //float * pExcessFlow, 
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	RelabelBlock* pTableBlock,
	int Rows,
	int Cols)
{
	HEIGHT_TYPE temp;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

	HEIGHT_TYPE NewH = 0;
	HEIGHT_TYPE NewHeightCount = HEIGHT_MAX;
	int PrevCount;
	int blockInRow = Cols >> LOG_SIZE;
	// Init Height
	int prevIndex = 0;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
	while (indexActiveBlock != -1)
	{
		int currentBlock = prevIndex + indexActiveBlock;
		int row = pTableBlock[currentBlock].row;
		int col = pTableBlock[currentBlock].col;
		prevIndex = currentBlock;
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
		for (int y = row; y < (row + SIDE_SQUARE); y++) {
			for (int x = col; x < (col + SIDE_SQUARE); x++) {
				if (GetPix(pHeight, Cols, x, y) != 0)
					PutPix(pHeight, Cols, x, y, HEIGHT_MAX);
			}
		}
	}

	// Global relabel
	int numIter = 0;
	do {
		numIter++;
		PrevCount = NewHeightCount;
		NewHeightCount = 0;
		// Block based global relabel
		prevIndex = 0;
		indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
		while (indexActiveBlock != -1)
		{
			int currentBlock = prevIndex + indexActiveBlock;
			int row = pTableBlock[currentBlock].row;
			int col = pTableBlock[currentBlock].col;
			prevIndex = currentBlock;
			indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
			for (int y = row; y < row + SIDE_SQUARE; y++)
			{
				for (int x = col; x < col + SIDE_SQUARE; x++)
				{
					HEIGHT_TYPE CurHeight = GetPix(pHeight, Cols, x, y);
					HEIGHT_TYPE NewHeight = CurHeight;
					// West neighbour
					temp = GetPix(pHeight, Cols, ((x - 1) < 0) ? 0 : (x - 1), y) + 1;
					if (GetPix(pWestCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// East neighbour
					temp = GetPix(pHeight, Cols, ((x + 1) >= Cols) ? (Cols - 1) : (x + 1), y) + 1;
					if (GetPix(pEastCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// North neighbour
					temp = GetPix(pHeight, Cols, x, ((y - 1)<0) ? 0 : (y - 1)) + 1;
					if (GetPix(pNorthCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// South neighbour
					temp = GetPix(pHeight, Cols, x, ((y + 1) >= Rows) ? (Rows - 1) : (y + 1)) + 1;
					if (GetPix(pSouthCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// Update the current Height to new Height
					if (CurHeight != NewHeight) {
						PutPix(pHeight, Cols, x, y, NewHeight);
						NewHeightCount++;
					}
				}
			}
		}
#ifdef _DEBUG
		printf("\tGlobal relabel: %d\n", NewHeightCount);
#endif

	} while (numIter < 16 && NewHeightCount > 0);
}

void CTamarActiveBlocks(short * pExcessFlow, HEIGHT_TYPE * pHeight, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	char ActiveNodeCount;

	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

	short CurExcessFlow;
	HEIGHT_TYPE CurHeight;
	int blockInRow = Cols >> LOG_SIZE;
	int lastActiveBlock = 0;
	pTableBlock[lastActiveBlock].nextActiveBlock = -1;
	int currentBlock = 0;
	for (int y = 0; y < Rows; y += SIDE_SQUARE)
	{
		for (int x = 0; x < Cols; x += SIDE_SQUARE) {
			ActiveNodeCount = 0;
			for (int i = y; (i < y + SIDE_SQUARE) && (ActiveNodeCount == 0); i++)
			{
				for (int j = x; (j < x + SIDE_SQUARE) && (ActiveNodeCount == 0); j++)
				{
					CurHeight = *(pHeight + i*Cols + j);
					CurExcessFlow = *(pExcessFlow + i*Cols + j);
					if ((CurHeight < HEIGHT_MAX) && (CurExcessFlow > -100))
					{
						ActiveNodeCount = 1;
					}
				}
			}
			pTableBlock[currentBlock + 1].row = y;
			pTableBlock[currentBlock + 1].col = x;
			if (ActiveNodeCount == 1)
			{
				pTableBlock[lastActiveBlock].nextActiveBlock = (currentBlock + 1) - lastActiveBlock;
				lastActiveBlock = currentBlock + 1;
			}
			pTableBlock[currentBlock + 1].nextActiveBlock = -1;
			currentBlock++;
		}
	}
}

int IsActivePixel(short * pExcessFlow, HEIGHT_TYPE * pHeight, int Rows, int Cols)
{
	char ActiveNodeCount;

	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

	short CurExcessFlow;
	HEIGHT_TYPE CurHeight;
	int blockInRow = Cols >> LOG_SIZE;
	ActiveNodeCount = 0;
	for (int y = 0; y < Rows; y++)
	{
		for (int x = 0; x < Cols; x++) {
			CurHeight = *(pHeight + y*Cols + x);
			CurExcessFlow = *(pExcessFlow + y*Cols + x);
			if ((CurHeight < HEIGHT_MAX) && (CurExcessFlow > 0))
			{
						
					ActiveNodeCount = 1;
			}		
		}
	}
	return ActiveNodeCount;
}

void CTamarModel_GraphCut_Vertical_Push(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pNorthCap, short * pSouthCap, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	short flow;
	int nPush = 0;
	int sPush = 0;
	int blockInRow = Cols >> LOG_SIZE;
	int prevIndex = 0;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
	while (indexActiveBlock != -1)
	{
		int currentBlock = prevIndex + indexActiveBlock;
		int row = pTableBlock[currentBlock].row;
		int col = pTableBlock[currentBlock].col;
		prevIndex = currentBlock;
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
		for (int y = row; y < (row + SIDE_SQUARE); y++)
			for (int x = col; x < (col + SIDE_SQUARE); x++)
			{
				if (GetPix(pHeight, Cols, x, y) < HEIGHT_MAX) {

					// North neighbour (x, y-1)
					if (y > 0) {
						if (GetPix(pExcessFlow, Cols, x, y) > 0 && GetPix(pHeight, Cols, x, y - 1) == GetPix(pHeight, Cols, x, y) - 1) {
							flow = min(GetPix(pNorthCap, Cols, x, y), GetPix(pExcessFlow, Cols, x, y));
							if (flow != 0) {
								AddPix(pExcessFlow, Cols, x, y, (short)-flow);
								AddPix(pExcessFlow, Cols, x, y - 1, flow);
								AddPix(pNorthCap, Cols, x, y, (short)-flow);
								AddPix(pSouthCap, Cols, x, y - 1, flow);
								nPush++;
							}
						}
					}

					// South neighbour (x, y+1)
					if (y < Rows - 1) {
						if (GetPix(pExcessFlow, Cols, x, y) > 0 && GetPix(pHeight, Cols, x, y + 1) == GetPix(pHeight, Cols, x, y) - 1) {
							flow = min(GetPix(pSouthCap, Cols, x, y), GetPix(pExcessFlow, Cols, x, y));
							if (flow != 0) {
								AddPix(pExcessFlow, Cols, x, y, (short)-flow);
								AddPix(pExcessFlow, Cols, x, y + 1, flow);
								AddPix(pSouthCap, Cols, x, y, (short)-flow);
								AddPix(pNorthCap, Cols, x, y + 1, flow);
								sPush++;
							}
						}
					}
				}
			}
	}
#ifdef _DEBUG
	printf("nPush = %6d, sPush = %6d, ", nPush, sPush);
#endif
}

void CTamarModel_GraphCut_Horizontal_Push(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pWestCap, short * pEastCap, RelabelBlock* pTableBlock, int Rows, int Cols)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
	short flow;
	int rtn = 0;
	int wPush = 0;
	int ePush = 0;
	int blockInRow = Cols >> LOG_SIZE;
	int prevIndex = 0;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
	while (indexActiveBlock != -1)
	{
		int currentBlock = prevIndex + indexActiveBlock;
		int row = pTableBlock[currentBlock].row;
		int col = pTableBlock[currentBlock].col;
		prevIndex = currentBlock;
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
		for (int y = row; y < (row + SIDE_SQUARE); y++)
		{
			for (int x = col; x < (col + SIDE_SQUARE); x++)
			{
				if (GetPix(pHeight, Cols, x, y) < HEIGHT_MAX) {
					// West neighbour (x-1, y)
					if (x > 0) {
						if ((GetPix(pExcessFlow, Cols, x, y) > 0) && GetPix(pHeight, Cols, x - 1, y) == GetPix(pHeight, Cols, x, y) - 1) {
							flow = min(GetPix(pWestCap, Cols, x, y), GetPix(pExcessFlow, Cols, x, y));
							if (flow != 0) {
								AddPix(pExcessFlow, Cols, x, y, (short)-flow);
								AddPix(pExcessFlow, Cols, x - 1, y, flow);
								AddPix(pWestCap, Cols, x, y, (short)-flow);
								AddPix(pEastCap, Cols, x - 1, y, flow);
								wPush++;
							}
						}
					}
					// East neighbour (x+1, y)
					if (x < Cols - 1) {
						if (GetPix(pExcessFlow, Cols, x, y) > 0 && GetPix(pHeight, Cols, x + 1, y) == GetPix(pHeight, Cols, x, y) - 1) {
							flow = min(GetPix(pEastCap, Cols, x, y), GetPix(pExcessFlow, Cols, x, y));
							if (flow != 0) {
								AddPix(pExcessFlow, Cols, x, y, (short)-flow);
								AddPix(pExcessFlow, Cols, x + 1, y, flow);
								AddPix(pEastCap, Cols, x, y, (short)-flow);
								AddPix(pWestCap, Cols, x + 1, y, flow);
								ePush++;
							}
						}
					}
				}
			}
		}
	}


#ifdef _DEBUG
	printf("wPush = %6d, ePush = %6d, ", wPush, ePush);
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AVX2_PushRelabel_Init(short * pWeight,
						short * pHCap, 
						short * pVCap, 
						int Rows, 
						int Cols, 
						short * pExcessFlow, 
						HEIGHT_TYPE * pHeight, 
						short * pWestCap, 
						short * pNorthCap, 
						short * pEastCap, 
						short * pSouthCap,
						RelabelBlock * pTableBlock)
{
	short w, ch, cv;
	int x, y;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);	// Must have -1 to prevent overflow, some calculation needs HEIGHT_MAX+1
	int numActivePixels;
	int blockInRow = Cols >> LOG_SIZE;
	int lastActiveBlock = 0;
	pTableBlock[lastActiveBlock].row = -1;
	pTableBlock[lastActiveBlock].col = -1;
	pTableBlock[lastActiveBlock].nextActiveBlock = -1;

	for (y = 0; y < Rows; y += SIDE_SQUARE)
	{
		for (x = 0; x < Cols; x += SIDE_SQUARE) {

			numActivePixels = 0;
			for (int i = y; i < y + SIDE_SQUARE; i++)
			{
				for (int j = x; j < x + SIDE_SQUARE; j++)
				{
					// Set excess flow
					w = GetPix(pWeight, Cols, j, i);
					if (w > -100)
						numActivePixels++;
					PutPix(pExcessFlow, Cols, j, i, w);    // w>0 if a cap from source exists, w<0 if cap from sink exists. ExcessFlow = w.
					PutPix(pHeight, Cols, j, i, (HEIGHT_TYPE)0);

					// Set west and east caps
					ch = GetPix(pHCap, Cols, j, i);
					if (j == 0) {
						PutPix(pWestCap, Cols, j, i, (short)0);
					}
					else if (j < Cols - 1) {
						PutPix(pWestCap, Cols, j, i, ch);
						PutPix(pEastCap, Cols, j - 1, i, ch);
					}
					else {    // x = Cols-1
						PutPix(pWestCap, Cols, j, i, ch);
						PutPix(pEastCap, Cols, j - 1, i, ch);
						PutPix(pEastCap, Cols, j, i, (short)0);
					}

					// Set north and south caps
					cv = GetPix(pVCap, Cols, j, i);
					if (i == 0) {
						PutPix(pNorthCap, Cols, j, i, (short)0);
					}
					else if (i < Rows - 1) {
						PutPix(pNorthCap, Cols, j, i, cv);
						PutPix(pSouthCap, Cols, j, i - 1, cv);
					}
					else {
						PutPix(pNorthCap, Cols, j, i, cv);
						PutPix(pSouthCap, Cols, j, i - 1, cv);
						PutPix(pSouthCap, Cols, j, i, (short)0);
					}
				}
			}
			int currentBlock = (y >> LOG_SIZE)*blockInRow + (x >> LOG_SIZE);
			pTableBlock[currentBlock + 1].row = y;
			pTableBlock[currentBlock + 1].col = x;
			if (numActivePixels > 0)
			{
				pTableBlock[lastActiveBlock].nextActiveBlock = (currentBlock + 1) - lastActiveBlock;
				lastActiveBlock = currentBlock + 1;
			}
			pTableBlock[currentBlock + 1].nextActiveBlock = -1;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
int AVX2_Create_TableBlock(short * pExcessFlow, HEIGHT_TYPE * pHeight, int Rows, int Cols, RelabelBlock * pTableBlock)
{
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);	// Must have -1 to prevent overflow, some calculation needs HEIGHT_MAX+1
	int numActivePixels;
	int blockInRow = Cols >> LOG_SIZE;
	int lastActiveBlock = 0;
	pTableBlock[lastActiveBlock].row = -1;
	pTableBlock[lastActiveBlock].col = -1;
	pTableBlock[lastActiveBlock].nextActiveBlock = -1;

	for (int y = 0; y < Rows; y += SIDE_SQUARE) {
		for (int x = 0; x < Cols; x += SIDE_SQUARE) {
			numActivePixels = 0;
			for (int i = y; i < y + SIDE_SQUARE; i++) {
				for (int j = x; j < x + SIDE_SQUARE; j++) {
					if (GetPix(pExcessFlow, Cols, j, i) > -100 && GetPix(pHeight, Cols, j, i) < HEIGHT_MAX)
						numActivePixels++;	// Why not exit break out the loop when find the 1st active pixel?
				}
			}
			int currentBlock = (y >> LOG_SIZE)*blockInRow + (x >> LOG_SIZE);
			pTableBlock[currentBlock + 1].row = y;
			pTableBlock[currentBlock + 1].col = x;
			if (numActivePixels > 0) {
				pTableBlock[lastActiveBlock].nextActiveBlock = (currentBlock + 1) - lastActiveBlock;
				lastActiveBlock = currentBlock + 1;
			}
			pTableBlock[currentBlock + 1].nextActiveBlock = -1;
		}
	}

	return lastActiveBlock;
}
*/

#ifdef NO_USED
/////////////////////////////////////////////////////////////////////////////////////////
void Gap_Relable(HEIGHT_TYPE * pHeight, int Rows, int Cols)
{	
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
	unsigned int hist[1024];
	HEIGHT_TYPE h;
	int gap;

	memset(hist, 0, 1024*sizeof(int));

	// Build histogram
    for (int y = 1; y < Rows-1; y++) {  // within the frame
        for (int x = 1; x < Cols-1; x++) {
			h = GetPix(pHeight, Cols, x, y);
			if (h < 1024)
				hist[h]++;
		}
	}

	for (gap = 0; gap < 1024; gap++) {
		if (!hist[gap])
			break;
	}

	// gap is the gap value
    for (int y = 1; y < Rows-1; y++) {  // within the frame
        for (int x = 1; x < Cols-1; x++) {
			h = GetPix(pHeight, Cols, x, y);
			if (h > gap && h < HEIGHT_MAX)
				PutPix(pHeight, Cols, x, y, HEIGHT_MAX);
		}
	}
}
#endif


//////////////////////////////
void BlockCModel_Global_Relabel( //float * pExcessFlow, 
                            HEIGHT_TYPE * pHeight,
                            short * pWestCap,
                            short * pNorthCap,
                            short * pEastCap,
                            short * pSouthCap,
							RelabelBlock* pTableBlock,
							int Rows,
							int Cols)
{
	HEIGHT_TYPE temp;
	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

	HEIGHT_TYPE NewH = 0;
	HEIGHT_TYPE NewHeightCount = HEIGHT_MAX;
	int PrevCount;
	int blockInRow = Cols >> LOG_SIZE;

	// Init Height
	int prevIndex = 0;
	int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
	while (indexActiveBlock != -1)
	{
		int currentBlock = prevIndex + indexActiveBlock;
		int row = pTableBlock[currentBlock].row;
		int col = pTableBlock[currentBlock].col;
		prevIndex = currentBlock;
		indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
		for (int y = row; y < row + SIDE_SQUARE; y++)
		{
			for (int x = col; x < col + SIDE_SQUARE; x++)
			{
				if (GetPix(pHeight, Cols, x, y) != 0)
					PutPix(pHeight, Cols, x, y, HEIGHT_MAX);
			}
		}
	}
	int numIter = 0;
	// Global relabel
	do {
		numIter++;
		NewHeightCount = 0;
		// Block based global relabel
		int prevIndex = 0;
		int indexActiveBlock = pTableBlock[prevIndex].nextActiveBlock;
		while (indexActiveBlock != -1)
		{
			int currentBlock = prevIndex + indexActiveBlock;
			int row = pTableBlock[currentBlock].row;
			int col = pTableBlock[currentBlock].col;
			prevIndex = currentBlock;
			indexActiveBlock = pTableBlock[currentBlock].nextActiveBlock;
			for (int y = row; y < row + SIDE_SQUARE; y++)
			{
				for (int x = col; x < col + SIDE_SQUARE; x++)
				{

					HEIGHT_TYPE CurHeight = GetPix(pHeight, Cols, x, y);
					HEIGHT_TYPE NewHeight = CurHeight;

					// West neighbour
					temp = GetPix(pHeight, Cols, ((x - 1) < 0) ? 0 : (x - 1), y) + 1;
					if (GetPix(pWestCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// East neighbour
					temp = GetPix(pHeight, Cols, ((x + 1) >= Cols) ? (Cols - 1) : (x + 1), y) + 1;
					if (GetPix(pEastCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// North neighbour
					temp = GetPix(pHeight, Cols, x, ((y - 1)<0) ? 0 : (y - 1)) + 1;
					if (GetPix(pNorthCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// South neighbour
					temp = GetPix(pHeight, Cols, x, ((y + 1) >= Rows) ? (Rows - 1) : (y + 1)) + 1;
					if (GetPix(pSouthCap, Cols, x, y) > 0 && temp < NewHeight)
						NewHeight = temp;

					// Update the current Height to new Height
					if (CurHeight != NewHeight) {
						PutPix(pHeight, Cols, x, y, NewHeight);
						NewHeightCount++;
					}
				}
			}
		}
#ifdef _DEBUG
		printf("\tGlobal relabel: %d\n", NewHeightCount);
#endif

//	} while (/*PrevCount > NewHeightCount &&*/ NewHeightCount > 0);
	} while (numIter < 16 && NewHeightCount > 0);
}
void CModel_GraphCut_Relabel(short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	unsigned char * pBlockMask,
	int Rows,
	int Cols,
	int BlkRows,
	int BlkCols,
	int BlkWidth,
	int BlkHeight);

void CModel_Global_Relabel( //float * pExcessFlow, 
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pNorthCap,
	short * pEastCap,
	short * pSouthCap,
	unsigned char * pBlockMask,
	int Rows,
	int Cols,
	int BlkRows,
	int BlkCols,
	int BlkWidth,
	int BlkHeight);

void CModel_GraphCut_Vertical_Push(short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pNorthCap,
	short * pSouthCap,
	unsigned char * pBlockMask,
	int Rows,
	int Cols,
	int BlkRows,
	int BlkCols,
	int BlkWidth,
	int BlkHeight);

int CModel_GraphCut_Horizontal_Push(short * pExcessFlow,
	HEIGHT_TYPE * pHeight,
	short * pWestCap,
	short * pEastCap,
	unsigned char * pBlockMask,
	int Rows,
	int Cols,
	int BlkRows,
	int BlkCols,
	int BlkWidth,
	int BlkHeight);

/////////////////// Graph Cut entry function //////////////////////////////////////////////////////////////////
int AVX2_Push_Relabel(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pWestCap, short * pNorthCap,
	short * pEastCap, short * pSouthCap, unsigned char * pBlockMask, RelabelBlock * pTableBlock, 
						unsigned char * pOutput, int Rows, int Cols, int nRatio)
{
	int iter = 1;
	double T1, T2;
	int ActivePixels;

	int lastBlock = 0;
	int i;
	AVX2ActiveBlocks16x16(pExcessFlow, pHeight, pTableBlock, Rows, Cols);


	do {

//#ifdef _DEBUG
		T1 = GetTimeMS();
//#endif
		// Relabel all active nodes
		if (iter % nRatio) {
			AVX2Model_GraphCut_Relabel16x16(pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, pTableBlock, Rows, Cols); // pBlockMask, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);
			//CModel_GraphCut_Relabel(pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, pBlockMask, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);
		}
		else {    // Global relabel 
			
			AVX2Model_Global_Relabel16x16(pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, pTableBlock, Rows, Cols);//pBlockMask, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);
			AVX2ActiveBlocks16x16(pExcessFlow, pHeight, pTableBlock, Rows, Cols);// pBlockMask, BlkRows, BlkCols, BlkWidth, BlkHeight);

		}
		T2 = GetTimeMS();

#ifdef _DEBUG
		T2 = GetTimeMS();

		char fn[128];
		sprintf(fn, ".\\Output\\_%d_AVX_Height.%dx%d.Y8", iter, sizeof(HEIGHT_TYPE)*Cols, Rows);
		Dump2File(fn, (unsigned char *)pHeight, Rows*Cols*sizeof(HEIGHT_TYPE));

		//sprintf(fn, ".\\Output\\_%d_CPU_BlockMask.%dx%d.Y8", iter, BlkCols, BlkRows);
		//Dump2File(fn, (unsigned char *) pBlockMask, BlkRows*BlkCols);
#endif
		// Push all active nodes in all directions
		AVX2Model_GraphCut_Vertical_Push16x16(pExcessFlow, pHeight, pNorthCap, pSouthCap, pTableBlock, Rows, Cols);

		

		AVX2Model_GraphCut_Horizontal_Push16x16(pExcessFlow, pHeight, pWestCap, pEastCap, pTableBlock, Rows, Cols);
		ActivePixels = AVX2ActivePixels16x16(pExcessFlow, pHeight, pTableBlock, Rows, Cols);

		
#ifdef _DEBUG
		//printf("%3d, %d, %8.4f ms\n", iter, activeBlocks, T2 - T1);
#endif
	} while (ActivePixels > 0 && ++iter < 100000);

	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

	// Write pOutput
	for (int y = 0; y < Rows; y++)
		for (int x = 0; x < Cols; x++) {
			unsigned char val = GetPix(pHeight, Cols, x, y) < HEIGHT_MAX ? 0 : 255;
			PutPix(pOutput, Cols, x, y, val);
		}

	// Return iterations
	return iter;	
}


