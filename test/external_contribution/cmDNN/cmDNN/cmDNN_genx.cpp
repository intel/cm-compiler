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

#include <cm/cm.h>
//#include "Constants.h"
#ifdef CMRT_EMU
//#include <cm/half.h>
//using namespace half_float;
#endif

#define KWIDTH3		3
#define KWIDTH5		5
#define KWIDTH7		7
#define KWIDTH11	11

#define MEMORY_2D	0
#define MEMORY_1D	1

#define BLK8x8CNT	4
#define MULTIBLOCKS	4

typedef enum _POOLSIZE
{
	POOLSIZE_2x2 = 2,
	POOLSIZE_3x3 = 3,
} POOLSIZE;

static const int InitHorPos4[] = {0, 1, 2 ,3};
static const int InitHorPos8[] = {0, 1, 2, 3, 4, 5, 6, 7};
static const int InitHorPos16[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

//static const int InitHorPos16_S2[] = { 0, 2, 4, 6, 8, 10, 12, 14, 0, 2, 4, 6, 8, 10, 12, 14 };
//static const int InitVertPos16_S2[] = { 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2 };

const int iBlkWidth8 = 8;
const int iBlkHeight8 = 8;
const int iBlkWidth16 = 16;
const int iBlkHeight16 = 16;
const int iBlkHeight32 = 32;

const int oBlkWidth4 = 4;
const int oBlkHeight4 = 4;
const int oBlkWidth8 = 8;
const int oBlkHeight8 = 8;
const int oBlkWidth16 = 16;
const int oBlkHeight16 = 16;

const int Stride4 = 4;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Process a 8x8 tile.
extern "C" _GENX_MAIN_ void InputProc(SurfaceIndex iIndex, SurfaceIndex iIndex2, SurfaceIndex oIndex, uint Border)
{
	// h_pos and v_pos maps to output tile origin.
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();
//	uint h_pos = cm_local_size(0) * cm_group_id(0) + cm_local_id(0);
//	uint v_pos = cm_local_size(1) * cm_group_id(1) + cm_local_id(1);
/*
	int local_id_x = cm_local_id(0);
	int local_id_y = cm_local_id(1);
	int local_size_x = cm_local_size(0);
	int local_size_y = cm_local_size(1);
	int group_id_x = cm_group_id(0);
	int group_id_y = cm_group_id(1);
	int group_size_x = cm_group_count(0);
	int group_size_y = cm_group_count(1);
*/
	matrix<float, 8, 8> fInputData;		// 8 GRFs
	matrix<float, 8, 8> fCnnAvg;		// 8 GRFs
	matrix<float, 8, 8> fDst;			// 8 GRFs

	int nX = iBlkWidth8 * h_pos;		// Based on frame origin
	int nY = iBlkHeight8 * v_pos;

	read(iIndex, (nX + Border) * sizeof(float), nY + Border, fInputData);
	read(iIndex2, (nX + Border) * sizeof(float), nY + Border, fCnnAvg);

	fDst = fInputData - fCnnAvg;

	write(oIndex, (nX + Border) * sizeof(float), nY + Border, fDst);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IPT: Image per thread
// Input image up to 16x16
// Output image < 15x15
extern "C" _GENX_MAIN_ void Convol3x3_IPT16_2D(SurfaceIndex iIndex, SurfaceIndex oIndex,
											SurfaceIndex wIndex, SurfaceIndex bIndex,
											uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
											uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
											uint oTileHoriCount, uint Border, uint NumGroups, uint EnableReLU) 
{
	// h_pos and v_pos maps to output tile origin.
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();
//	uint h_pos = cm_local_size(0) * cm_group_id(0) + cm_local_id(0);
//	uint v_pos = cm_local_size(1) * cm_group_id(1) + cm_local_id(1);

#ifdef CMRT_EMU 
	matrix<float, 16, 20> fSrc;			// for EMU mode
#else
	matrix<float, 16, 16> fSrc;			// GRF 32
#endif
	matrix<float, 16, 16> fDst(0.0f);	// GRF 32
//	vector_ref<float, 256> vfDst = fDst.format<float>();
	vector<float, 12> vWeight;
	vector<float, 4> vBias;

	int WeightSize = 9;		// 3*3
	//int BiasSize = 1;

	int SrcIdxStart, SrcIdxEnd, GroupID;
	int SrcDepthPerGroup = SrcDepth / NumGroups;

	int DstIdx = oTileHoriCount * v_pos + h_pos;	// v_pos and h_pos are grid on output surface.  Each thread processes one ouptut image. 
	int FeatureOffsetInBatch = DstIdx%DestDepth;	// Feature offset wihtin the batch

	if (FeatureOffsetInBatch < DestDepth/NumGroups) {	// in the 1st half of DestDepth -> group 0
		SrcIdxStart = 0;
		SrcIdxEnd = SrcDepthPerGroup;   //source feature set
		GroupID = 0;
	} else {											// in the 2nd half of DestDepth -> group 1
		SrcIdxStart = SrcDepthPerGroup;
		SrcIdxEnd = SrcDepth;
		GroupID = 1;
	}

	for (int SrcIdx = SrcIdxStart; SrcIdx < SrcIdxEnd; SrcIdx++) {
		int iTileX = SrcIdx%iTileHoriCount;
		int iTileY = SrcIdx/iTileHoriCount;
		int iBaseX = iTileWidth * iTileX;
		int iBaseY = iTileHeight * iTileY;
		// Read 16x16 floats input block
		read(iIndex, iBaseX * sizeof(float),		iBaseY,		fSrc.select<8,1,8,1>(0,0));
		read(iIndex, (iBaseX + 8) * sizeof(float),	iBaseY,		fSrc.select<8,1,8,1>(0,8));
		read(iIndex, iBaseX * sizeof(float),		iBaseY + 8,	fSrc.select<8,1,8,1>(8,0));
		read(iIndex, (iBaseX + 8) * sizeof(float),	iBaseY + 8, fSrc.select<8,1,8,1>(8,8));

		// Read a set of filter weights and bias
		int GroupAdjustedSrcIdx = (GroupID == 0) ? SrcIdx : (SrcIdx - SrcDepthPerGroup);	// Handle 2 groups
		int WeightOffset = (FeatureOffsetInBatch * SrcDepthPerGroup + GroupAdjustedSrcIdx) * WeightSize;	// Offset into weight matrix of DstIdx and SrcIdx dimension.
		read(DWALIGNED(wIndex), WeightOffset * sizeof(float), vWeight);

//		matrix<float, 14, 16> fTemp(0);
		#pragma unroll
		for (int i = 0; i < KWIDTH3; i++) {
			#pragma unroll
			for (int j = 0; j < KWIDTH3; j++) {
//				fTemp.select<14, 1, 16, 1>(0, 0) = fSrc.select<14, 1, 16, 1>(j, i);
//				fTemp.select<14,1,8,1>(0,0) = fSrc.select<14,1,8,1>(j,i);
//				fTemp.select<14,1,8,1>(0,8) = fSrc.select<14,1,8,1>(j,8+i);
//				fDst.select<14, 1, 16, 1>(0, 0) += vKW[KWIDTH3*j + i] * fTemp;
				fDst.select<14,1,16,1>(0,0) += vWeight[KWIDTH3*j+i] * fSrc.select<14, 1, 16, 1>(j, i); //fTemp;
			}
		}
/*
		// Convol left 13x8 pixels in SIMD8
		#pragma unroll
		for (int i = 0; i < KWIDTH3; i++) {
			#pragma unroll
			for (int j = 0; j < KWIDTH3; j++) {
//				fDst.select<13,1,8,1>(0,0) += vKW[KWIDTH3*j+i] * fSrc.select<13,1,8,1>(j,i);
				fDst.select<14,1,8,1>(0,0) += vKW[KWIDTH3*j+i] * fSrc.select<14,1,8,1>(j,i);
			}
		}
		// Convol right 13x5 pixels in SIMD8
		#pragma unroll
		for (int i = 0; i < KWIDTH3; i++) {
			#pragma unroll
			for (int j = 0; j < KWIDTH3; j++) {
//				fDst.select<13,1,8,1>(0,8) += vKW[KWIDTH3*j+i] * fSrc.select<13,1,8,1>(j,8+i);
				fDst.select<14,1,8,1>(0,8) += vKW[KWIDTH3*j+i] * fSrc.select<14,1,8,1>(j,8+i);
			}
		}
//*/
	}

	//int BiasOffset = FeatureOffsetInBatch;
	read(DWALIGNED(bIndex), FeatureOffsetInBatch * sizeof(float), vBias);
	fDst += vBias[0];	// Add bias

	if (EnableReLU)
		fDst.merge(0.0f, fDst < 0.0f);		// ReLU

	// Zero out area outside of image for partial write
	if (DestWidth < oBlkWidth16) {		// Last block horizontally
		for (int c = DestWidth; c < oBlkWidth16; c++)
			fDst.column(c) = 0.0f;
	}
	if (DestHeight < oBlkHeight16) {		// last block vertically
		for (int r = DestHeight; r < oBlkHeight16; r++)
			fDst.row(r) = 0.0f;
	}

	int oBaseX = h_pos * oBlkWidth16;
	int oBaseY = v_pos * oBlkHeight16;

/*  // Mute OOB pixels
	matrix<uchar, 16, 16> horPos(InitHorPos16);
	#pragma unroll
	for (int j = 0; j < BlkHeight; j++)
		horPos.row(j) = horPos.row(0);
	fDst.merge(0.0f, horPos >= DestWidth);	// Mute right side
	#pragma unroll
	for (int j = DestHeight; j < BlkHeight; j++)
		fDst.row(j) = 0.0f;
*/
	// Output 16x16 float
	write(oIndex, (oBaseX + Border) * sizeof(float),		oBaseY + Border,		fDst.select<8,1,8,1>(0,0));
	write(oIndex, (oBaseX + Border + 8) * sizeof(float),	oBaseY + Border,		fDst.select<8,1,8,1>(0,8));
	write(oIndex, (oBaseX + Border) * sizeof(float),		oBaseY + Border + 8,	fDst.select<8,1,8,1>(8,0));
	write(oIndex, (oBaseX + Border + 8) * sizeof(float),	oBaseY + Border + 8,	fDst.select<8,1,8,1>(8,8));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This kernel performs 3x3 convolution on 8x8 multiple blocks  
extern "C" _GENX_MAIN_ void Convol3x3_BPT_8x8_MB(SurfaceIndex iIndex, SurfaceIndex oIndex, 
												SurfaceIndex wIndex, SurfaceIndex bIndex,
												uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
												uint iTileHoriCount, uint iTileVertCount, uint iTileWidth, uint iTileHeight, 
												uint oTileHoriCount, uint oTileVertCount, uint oTileWidth, uint oTileHeight, 
												uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
												uint Border, uint EnableReLU) 
{
	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();
//	int h_pos = cm_local_size(0) * cm_group_id(0) + cm_local_id(0);
//	int v_pos = cm_local_size(1) * cm_group_id(1) + cm_local_id(1);

	// Output blocks' coordinates.
	vector<int, MULTIBLOCKS> OutX, OutY;

	matrix<float, 8+2, 8+2> fSrc;
	matrix<float, 8*MULTIBLOCKS, 8> fDst(0.0f);
	vector<float, 12> vWeight;
	vector<float, MULTIBLOCKS> vBias;

	// Output horizontal and vertical blocks of entire surface
	int oHoriBlocks = oTileHoriCount * ThreadsWidthPerTile;
	int oVertBlocks = oTileVertCount * ThreadsHeightPerTile;

	int h_pos2 = (h_pos / ThreadsWidthPerTile) * ThreadsWidthPerTile * MULTIBLOCKS + (h_pos % ThreadsWidthPerTile); 
	int oBlockID = v_pos * oHoriBlocks + h_pos2;

	// Distance to next collocated block: add ThreadsWidthPerTile.
	for (int i = 0, j = oBlockID; i < MULTIBLOCKS; i++, j += ThreadsWidthPerTile) {
		OutX[i] = j % oHoriBlocks;							// X coordinate from frame origin in block
		OutY[i] = j / oHoriBlocks;							// Y coordinate from frame origin in block
	}

	int nX = h_pos * iBlkWidth8;							// X coordinate from frame origin
	int nY = v_pos * iBlkHeight8;							// Y coordinate from frame origin
	int TileX = nX % oTileWidth;							// X coordinate from tile origin
	int TileY = nY % oTileHeight;							// Y coordinate from tile origin

	int WeightSize = 9;		// 3*3
	
	if (DestWidth <= 8)
		fSrc = 0;

	for (int SrcIdx = 0; SrcIdx < SrcDepth; SrcIdx++) {

		vector<int, 2> SrcTileXY;
		vector<int, 2> iBase;

		// Get src tile origin indexd by SrcIdx
		SrcTileXY[0] = SrcIdx % iTileHoriCount;		// Tile origin X in tiles
		SrcTileXY[1] = SrcIdx / iTileHoriCount;		// Tile origin Y in tiles
		iBase[0] = iTileWidth * SrcTileXY[0];		// Tile origin X in pixels
		iBase[1] = iTileHeight * SrcTileXY[1];		// Tile origin Y in pixels

		if (DestWidth <= 8) {
			// Read a 8x8 block
			read(iIndex, (iBase[0] + TileX) * sizeof(float), iBase[1] + TileY, fSrc.select<8, 1, 8, 1>(0, 0));
		} else {
			// Read a 10x10 block
			read(iIndex, (iBase[0] + TileX) * sizeof(float), iBase[1] + TileY, fSrc.select<8, 1, 8, 1>(0, 0));
			read(iIndex, (iBase[0] + TileX + 8) * sizeof(float), iBase[1] + TileY, fSrc.select<8, 1, 2, 1>(0, 8));
			read(iIndex, (iBase[0] + TileX) * sizeof(float), iBase[1] + TileY + 8, fSrc.select<2, 1, 8, 1>(8, 0));
			read(iIndex, (iBase[0] + TileX + 8) * sizeof(float), iBase[1] + TileY + 8, fSrc.select<2, 1, 2, 1>(8, 8));
		}

		#pragma unroll
		for (int i = 0; i < MULTIBLOCKS; i++) {
			int TileIDx = OutX[i] / ThreadsWidthPerTile;					// Tile id X. Thread (v_pos, h_pos) falls in this tile.
			int TileIDy = OutY[i] / ThreadsHeightPerTile;					// Tile id Y, Thread (v_pos, h_pos) falls in this tile.
			int DestTileID = TileIDy * oTileHoriCount + TileIDx;			// Dest tile id	
			int WeightSet = DestTileID % DestDepth * SrcDepth;				// % DestDepth for multi-batch. Weightset is total weight size of all inputs for one output.
			int WeightOffset = (WeightSet + SrcIdx) * WeightSize;			// Offset into weight matrix of DstTid and SrcIdx dimension.

			read(DWALIGNED(wIndex), WeightOffset * sizeof(float), vWeight);
			//Bias[i] =  vKW[WeightSize - 1];

			#pragma unroll
			for (int r = 0; r < KWIDTH3; r++) {
				#pragma unroll
				for (int c = 0; c < KWIDTH3; c++) {
					fDst.select<8,1,8,1>(8*i,0) += vWeight[KWIDTH3*r + c] * fSrc.select<8,1,8,1>(r,c);
				} 
			}
		}
	}

	// Get bias for all outputs.
	int TileIDx = OutX[0] / ThreadsWidthPerTile;					// Tile id X. Thread (v_pos, h_pos) falls in this tile.
	int TileIDy = OutY[0] / ThreadsHeightPerTile;					// Tile id Y, Thread (v_pos, h_pos) falls in this tile.
	int DestTileID = TileIDy * oTileHoriCount + TileIDx;			// Dest tile id	
	int FeatureOffsetInBatch = DestTileID % DestDepth;				// % DestDepth for multi-batch. Weightset is total weight size of all inputs for one output.

	// Read bias for all outputs of this kernel.
	read(DWALIGNED(bIndex), FeatureOffsetInBatch * sizeof(float), vBias);

	// Add bias to each output block
	#pragma unroll
	for (int i = 0; i < MULTIBLOCKS; i++) {
		fDst.select<8,1,8,1>(8*i,0) += vBias[i];
	}

	// Apply ReLU
	if (EnableReLU)
		fDst.select<8*MULTIBLOCKS,1,8,1>(0,0).merge(0.0f, fDst.select<8*MULTIBLOCKS,1,8,1>(0,0) < 0.0f);

	// Out of boundary handling
	vector<ushort, 8> horPos = InitHorPos8;
	vector<ushort, 8> verPos = horPos;
	horPos += (nX % oTileWidth);
	verPos += (nY % oTileHeight);

	#pragma unroll
	for(int j = 0; j < 8; j++) {
		#pragma unroll
		for(int i = 0; i < MULTIBLOCKS; i++)
			fDst.row(8*i+j).merge(0.0f, horPos >= DestWidth);	// Mute right side
		if (verPos[j] >= DestHeight) {							// Mute bottom 
			#pragma unroll
			for(int i = 0; i < MULTIBLOCKS; i++)
				fDst.row(8*i+j) = 0.0f;	
		}
	}

	#pragma unroll
	for (int i = 0; i < MULTIBLOCKS; i++)
		write(oIndex, (OutX[i] * oBlkWidth8 + Border) * sizeof(float), (OutY[i] * oBlkHeight8 + Border), fDst.select<8,1,8,1>(8*i,0));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This kernel performs 1x1 convolution on 8x8 multiple blocks  
extern "C" _GENX_MAIN_ void Convol1x1_BPT_8x8_MB(SurfaceIndex iIndex, SurfaceIndex iIndex2, int NumInputs,
												SurfaceIndex oIndex, SurfaceIndex wIndex, SurfaceIndex bIndex,
												uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
												uint iTileHoriCount, uint iTileVertCount, uint iTileWidth, uint iTileHeight,
												uint oTileHoriCount, uint oTileVertCount, uint oTileWidth, uint oTileHeight,
												uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
												uint Border, uint Input2Border, uint EnableReLU)
{
	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();

	// Output blocks' coordinates.
	vector<int, MULTIBLOCKS> OutX, OutY;

	matrix<float, 8, 8> fSrc;
	matrix<float, 8 * MULTIBLOCKS, 8> fDst(0.0f);
	vector<float, MULTIBLOCKS> vWeight;
	vector<float, MULTIBLOCKS> vBias;

	// Output horizontal and vertical blocks of entire surface
	int oHoriBlocks = oTileHoriCount * ThreadsWidthPerTile;
	int oVertBlocks = oTileVertCount * ThreadsHeightPerTile;

	int h_pos2 = (h_pos / ThreadsWidthPerTile) * ThreadsWidthPerTile * MULTIBLOCKS + (h_pos % ThreadsWidthPerTile);
	int oBlockID = v_pos * oHoriBlocks + h_pos2;

	// Find 8 outputs co-located blocks.
	// For distance to next collocated block, add ThreadsWidthPerTile.
	for (int i = 0, j = oBlockID; i < MULTIBLOCKS; i++, j += ThreadsWidthPerTile) {
		OutX[i] = j % oHoriBlocks;							// X coordinate from frame origin in block
		OutY[i] = j / oHoriBlocks;							// Y coordinate from frame origin in block
	}

	int nX = h_pos * iBlkWidth8;							// X coordinate from frame origin
	int nY = v_pos * iBlkHeight8;							// Y coordinate from frame origin
	int TileX = nX % oTileWidth;							// X coordinate from tile origin
	int TileY = nY % oTileHeight;							// Y coordinate from tile origin

	//int WeightSize = 1;			// 1*1

	// Calculate dest tile id for reading weights and bias.
	int TileIDx = OutX[0] / ThreadsWidthPerTile;					// Tile id X. Thread (v_pos, h_pos) falls in this tile.
	int TileIDy = OutY[0] / ThreadsHeightPerTile;					// Tile id Y, Thread (v_pos, h_pos) falls in this tile.
	int DestTileID = TileIDy * oTileHoriCount + TileIDx;			// Dest tile id	for the 1st one in set specified by OutX, OutY.
	DestTileID = DestTileID % DestDepth;

	// Loop through SrcDepth
	for (int SrcIdx = 0; SrcIdx < SrcDepth; SrcIdx++) {

		vector<int, 2> SrcTileXY;
		vector<int, 2> iBase;

		// Get src tile origin indexd by SrcIdx
		SrcTileXY[0] = SrcIdx % iTileHoriCount;		// Tile origin X in tiles
		SrcTileXY[1] = SrcIdx / iTileHoriCount;		// Tile origin Y in tiles
		iBase[0] = iTileWidth * SrcTileXY[0];		// Tile origin X in pixels
		iBase[1] = iTileHeight * SrcTileXY[1];		// Tile origin Y in pixels

		// Read a 8x8 block form SrcIdx.
		read(iIndex, (iBase[0] + TileX + Border) * sizeof(float), (iBase[1] + TileY + Border), fSrc.select<8,1,8,1>(0,0));

		// Read MULTIBLOCKS co-located weights for MULTIBLOCKS outputs
		int WeightOffset = ((DestTileID / MULTIBLOCKS) * MULTIBLOCKS * SrcDepth + MULTIBLOCKS * SrcIdx);
		read(DWALIGNED(wIndex), WeightOffset * sizeof(float), vWeight);  // Read MULTIBLOCKS FP32

		#pragma unroll
		for (int i = 0; i < MULTIBLOCKS; i++)
			fDst.select<8,1,8,1>(8*i,0) += vWeight[i] * fSrc.select<8,1,8,1>(0,0);
	}

	// Read bias for MULTIBLOCKS outputs
	read(DWALIGNED(bIndex), DestTileID * sizeof(float), vBias);

	// Add bias to each output block
	#pragma unroll
	for (int i = 0; i < MULTIBLOCKS; i++)
		fDst.select<8,1,8,1>(8*i,0) += vBias[i];
	
	// Add branch 2 result to each output block
	if (NumInputs == 2) {
		for (int i = 0; i < MULTIBLOCKS; i++) {
			read(iIndex2, (OutX[i] * iBlkWidth8 + Input2Border) * sizeof(float), (OutY[i] * iBlkHeight8 + Input2Border), fSrc);
			fDst.select<8,1,8,1>(8*i,0) += fSrc;
		}
	}

	// Apply ReLU
	if (EnableReLU)
		fDst.select<8*MULTIBLOCKS,1,8,1>(0,0).merge(0.0f, fDst.select<8*MULTIBLOCKS,1,8,1>(0,0) < 0.0f);

	// Out of boundary handling
	vector<ushort, 8> horPos = InitHorPos8;
	vector<ushort, 8> verPos = horPos;
	horPos += (nX % oTileWidth);
	verPos += (nY % oTileHeight);

	#pragma unroll
	for (int j = 0; j < 8; j++) {
		#pragma unroll
		for (int i = 0; i < MULTIBLOCKS; i++)
			fDst.row(8 * i + j).merge(0.0f, horPos >= DestWidth);	// Mute right side
		if (verPos[j] >= DestHeight) {							// Mute bottom 
			#pragma unroll
			for (int i = 0; i < MULTIBLOCKS; i++)
				fDst.row(8 * i + j) = 0.0f;
		}
	}

	#pragma unroll
	for (int i = 0; i < MULTIBLOCKS; i++)
		write(oIndex, (OutX[i] * oBlkWidth8 + Border) * sizeof(float), (OutY[i] * oBlkHeight8 + Border), fDst.select<8,1,8,1>(8*i,0));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This kernel performs 1x1 convolution on multiple 16x16 blocks.  Stride = 2. Ouptut size is 8x8.
extern "C" _GENX_MAIN_ void Convol1x1_BPT_8x8_MB_S2(SurfaceIndex iIndex, SurfaceIndex iIndex2, int NumInputs,
													SurfaceIndex oIndex, SurfaceIndex wIndex, SurfaceIndex bIndex,
													uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
													uint iTileHoriCount, uint iTileVertCount, uint iTileWidth, uint iTileHeight,
													uint oTileHoriCount, uint oTileVertCount, uint oTileWidth, uint oTileHeight,
													uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
													uint Border, uint Input2Border, uint EnableReLU)
{
	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();

	// Output blocks' coordinates.
	vector<int, MULTIBLOCKS> OutX, OutY;

//	matrix<float, 16, 16> fSrc;
	matrix<float, 8, 8> fSrc;
	matrix_ref<float, 4, 16> fSrc4x16 = fSrc.format<float, 4, 16>();

	matrix_ref<float, 1, 16> L0 = fSrc4x16.row(0).format<float, 1, 16>();
	matrix_ref<float, 1, 16> L1 = fSrc4x16.row(1).format<float, 1, 16>();
	matrix_ref<float, 1, 16> L2 = fSrc4x16.row(2).format<float, 1, 16>();
	matrix_ref<float, 1, 16> L3 = fSrc4x16.row(3).format<float, 1, 16>();
	vector<uint, 8> vedge8(InitHorPos8);

	matrix<float, 8 * MULTIBLOCKS, 8> fDst(0.0f);
	vector<float, MULTIBLOCKS> vWeight;
	vector<float, MULTIBLOCKS> vBias;

	// Output horizontal and vertical blocks of entire surface
	int oHoriBlocks = oTileHoriCount * ThreadsWidthPerTile;
	int oVertBlocks = oTileVertCount * ThreadsHeightPerTile;

	int h_pos2 = (h_pos / ThreadsWidthPerTile) * ThreadsWidthPerTile * MULTIBLOCKS + (h_pos % ThreadsWidthPerTile);
	int oBlockID = v_pos * oHoriBlocks + h_pos2;

	// For distance to next collocated block, add ThreadsWidthPerTile.
	for (int i = 0, j = oBlockID; i < MULTIBLOCKS; i++, j += ThreadsWidthPerTile) {
		OutX[i] = j % oHoriBlocks;							// X coordinate from frame origin in block
		OutY[i] = j / oHoriBlocks;							// Y coordinate from frame origin in block
	}

	int nX = h_pos * iBlkWidth16;							// X coordinate from frame origin
	int nY = v_pos * iBlkHeight16;							// Y coordinate from frame origin
	int TileX = nX % iTileWidth;							// X coordinate from tile origin
	int TileY = nY % iTileHeight;							// Y coordinate from tile origin
	
	//int WeightSize = 1;			// 1*1

	// Calculate dest tile id for reading weights and bias.
	int TileIDx = OutX[0] / ThreadsWidthPerTile;					// Tile id X. Thread (v_pos, h_pos) falls in this tile.
	int TileIDy = OutY[0] / ThreadsHeightPerTile;					// Tile id Y, Thread (v_pos, h_pos) falls in this tile.
	int DestTileID = TileIDy * oTileHoriCount + TileIDx;			// Dest tile id	for the 1st one in set specified by OutX, OutY.
	DestTileID = DestTileID % DestDepth;

	// Loop through SrcDepth
	for (int SrcIdx = 0; SrcIdx < SrcDepth; SrcIdx++) {

		vector<int, 2> SrcTileXY;
		vector<int, 2> iBase;

		// Get src tile origin indexd by SrcIdx
		SrcTileXY[0] = SrcIdx % iTileHoriCount;		// Tile origin X in tiles
		SrcTileXY[1] = SrcIdx / iTileHoriCount;		// Tile origin Y in tiles
		iBase[0] = iTileWidth * SrcTileXY[0];		// Tile origin X in pixels
		iBase[1] = iTileHeight * SrcTileXY[1];		// Tile origin Y in pixels

		// Read a 16x16 block
/*		read(iIndex, (iBase[0] + TileX + Border) * sizeof(float),		(iBase[1] + TileY + Border),		fSrc.select<8, 1, 8, 1>(0, 0));
		read(iIndex, (iBase[0] + TileX + 8 + Border) * sizeof(float),	(iBase[1] + TileY + Border),		fSrc.select<8, 1, 8, 1>(0, 8));
		read(iIndex, (iBase[0] + TileX + Border) * sizeof(float),		(iBase[1] + TileY + 8 + Border),	fSrc.select<8, 1, 8, 1>(8, 0));
		read(iIndex, (iBase[0] + TileX + 8 + Border) * sizeof(float),	(iBase[1] + TileY + 8 + Border),	fSrc.select<8, 1, 8, 1>(8, 8));
*/
		vector<uint, 16> u = iBase[0] + TileX + Border;
		vector<uint, 16> v = iBase[1] + TileY + Border;

		// 16 addresses cover 2x8 every other pixels
		u.select<8, 1>(0) += 2 * vedge8;		// stride 2
		u.select<8, 1>(8) += 2 * vedge8;		// stride 2
		v.select<8, 1>(0) += 0;
		v.select<8, 1>(8) += 2;

		// Read 8x8 input data, 
		read_typed(iIndex, CM_R_ENABLE, L0, u, v); 
		read_typed(iIndex, CM_R_ENABLE, L1, u, v+4);
		read_typed(iIndex, CM_R_ENABLE, L2, u, v+8);
		read_typed(iIndex, CM_R_ENABLE, L3, u, v+12);

		// Read MULTIBLOCKS co-located weights for MULTIBLOCKS outputs
		int WeightOffset = ((DestTileID / MULTIBLOCKS) * MULTIBLOCKS * SrcDepth + MULTIBLOCKS * SrcIdx);
		read(DWALIGNED(wIndex), WeightOffset * sizeof(float), vWeight);  // Read MULTIBLOCKS FP32

		#pragma unroll
		for (int i = 0; i < MULTIBLOCKS; i++)
			//fDst.select<8,1,8,1>(8*i,0) += vWeight[i] * fSrc.select<8,2,8,2>(0,0);	// Stride 2
			fDst.select<8, 1, 8, 1>(8 * i, 0) += vWeight[i] * fSrc;
	}

	// Read bias for all outputs of this kernel.
	read(DWALIGNED(bIndex), DestTileID * sizeof(float), vBias);

	// Add bias to each output block
	#pragma unroll
	for (int i = 0; i < MULTIBLOCKS; i++)
		fDst.select<8,1,8,1>(8*i,0) += vBias[i];

	// Add branch 2 result to each output block
	if (NumInputs == 2) {

		#pragma unroll
		for (int i = 0; i < MULTIBLOCKS; i++) {
			vector<uint, 16> u = OutX[i] * iBlkWidth8 + Input2Border;
			vector<uint, 16> v = OutY[i] * iBlkHeight8 + Input2Border;

			// 16 addresses cover 2x8 every other pixels
			u.select<8, 1>(0) += 2 * vedge8;		// stride 2
			u.select<8, 1>(8) += 2 * vedge8;		// stride 2
			v.select<8, 1>(0) += 0;
			v.select<8, 1>(8) += 2;

			// Read 8x8 input data, 
			read_typed(iIndex2, CM_R_ENABLE, L0, u, v);
			read_typed(iIndex2, CM_R_ENABLE, L1, u, v + 4);
			read_typed(iIndex2, CM_R_ENABLE, L2, u, v + 8);
			read_typed(iIndex2, CM_R_ENABLE, L3, u, v + 12);

			fDst.select<8,1,8,1>(8*i,0) += fSrc;

//			read(iIndex2, (OutX[i] * iBlkWidth8 + Input2Border) * sizeof(float),		(OutY[i] * iBlkHeight8 + Input2Border),		fSrc.select<8,1,8,1>(0,0));
//			read(iIndex2, (OutX[i] * iBlkWidth8 + 8 + Input2Border) * sizeof(float),	(OutY[i] * iBlkHeight8 + Input2Border),		fSrc.select<8,1,8,1>(0,8));
//			read(iIndex2, (OutX[i] * iBlkWidth8 + Input2Border) * sizeof(float),		(OutY[i] * iBlkHeight8 + 8 + Input2Border), fSrc.select<8,1,8,1>(8,0));
//			read(iIndex2, (OutX[i] * iBlkWidth8 + 8 + Input2Border) * sizeof(float),	(OutY[i] * iBlkHeight8 + 8 + Input2Border), fSrc.select<8,1,8,1>(8,8));
//			fDst.select<8,1,8,1>(8*i,0) += fSrc.select<8,2,8,2>(0,0);
		}
	}

	// Apply ReLU
	if (EnableReLU)
		fDst.select<8*MULTIBLOCKS,1,8,1>(0,0).merge(0.0f, fDst.select<8*MULTIBLOCKS,1,8,1>(0,0) < 0.0f);

	// Out of boundary handling
	nX = h_pos * oBlkWidth8;								// X coordinate from frame origin
	nY = v_pos * oBlkHeight8;								// Y coordinate from frame origin
	vector<ushort, 8> horPos = InitHorPos8;
	vector<ushort, 8> verPos = horPos;
	horPos += (nX % oTileWidth);
	verPos += (nY % oTileHeight);

	#pragma unroll
	for (int j = 0; j < 8; j++) {
		#pragma unroll
		for (int i = 0; i < MULTIBLOCKS; i++)
			fDst.row(8 * i + j).merge(0.0f, horPos >= DestWidth);	// Mute right side
		if (verPos[j] >= DestHeight) {							// Mute bottom 
			#pragma unroll
			for (int i = 0; i < MULTIBLOCKS; i++)
				fDst.row(8 * i + j) = 0.0f;
		}
	}

	#pragma unroll
	for (int i = 0; i < MULTIBLOCKS; i++)
		write(oIndex, (OutX[i] * oBlkWidth8 + Border) * sizeof(float), (OutY[i] * oBlkHeight8 + Border), fDst.select<8,1,8,1>(8*i,0));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input block size = 32x8. Output block size = 27x8
extern "C" _GENX_MAIN_ void Convol5x5_BPT27x8(	SurfaceIndex iIndex, SurfaceIndex oIndex, /*SurfaceIndex kIndex,*/
												SurfaceIndex wIndex, SurfaceIndex bIndex,
												uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
												uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
												uint oTileHoriCount, uint oTileWidth, uint oTileHeight, 
												uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
												uint Border, uint NumGroups, uint EnableReLU) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();
//	uint h_pos = cm_local_size(0) * cm_group_id(0) + cm_local_id(0);
//	uint v_pos = cm_local_size(1) * cm_group_id(1) + cm_local_id(1);

	int EndBlkWidth = DestWidth % iBlkWidth8;					// End block width
	int EndBlkHeight = DestHeight % iBlkHeight32;
	int nX = iBlkWidth8 * h_pos;								// X coordinate based on frame origin
	int nY = iBlkHeight32 * v_pos;
	int nTX = nX % iTileWidth;								// X coordinate based on current tile origin
	int nTY = nY % iTileHeight;
	int TileHoriOffset = h_pos / ThreadsWidthPerTile;		// Tile X coodinate that (v_pos, h_pos) falls in
	int TileVertOffset = v_pos / ThreadsHeightPerTile;
	int BlkHoriOffset = h_pos % ThreadsWidthPerTile;		// Block X offset within the tile
	int BlkVertOffset = v_pos % ThreadsHeightPerTile;

	// Build block rows to loop through
	int BlkCols = (BlkHoriOffset+1 == ThreadsWidthPerTile) ? EndBlkWidth : iBlkWidth8; 
	int BlkRows = (BlkVertOffset+1 == ThreadsHeightPerTile) ? EndBlkHeight : iBlkHeight32; 

	matrix<float, 31, 12> fSrc;			// GRF 62
	matrix<float, 27, 8> fDst(0.0f);	// GRF 27
	vector<float, 28> vWeight;			// 7 OWORD
	vector<float, 4> vBias;				// 1 OWORD

	int WeightSize = 25;				// 5*5 FP32
	//int BiasSize = 1;

	// v_pos and h_pos are grid from output surface.  DstIdx can be derived from them.  16 threads work on one ouptut image and they share one DstIdx.

	int SrcIdxStart, SrcIdxEnd, GroupID;
	int SrcDepthPerGroup = SrcDepth / NumGroups;

	int DstIdx = oTileHoriCount * TileVertOffset + TileHoriOffset;	
	int FeatureOffsetInBatch = DstIdx%DestDepth;	// Feature offset wihtin the batch

	if (FeatureOffsetInBatch < DestDepth/NumGroups) {	// in the 1st half of DestDepth -> group 0
		SrcIdxStart = 0;
		SrcIdxEnd = SrcDepthPerGroup;
		GroupID = 0;
	} else {
		SrcIdxStart = SrcDepthPerGroup;
		SrcIdxEnd = SrcDepth;
		GroupID = 1;
	}

	for (int SrcIdx = SrcIdxStart, KernalIdx = 0; SrcIdx < SrcIdxEnd; SrcIdx++, KernalIdx++) {

		int GroupAdjustedSrcIdx = (GroupID == 0) ? SrcIdx : (SrcIdx - SrcDepthPerGroup);	// Handle 2 groups		
		int WeightOffset = (DstIdx%DestDepth * SrcDepthPerGroup + GroupAdjustedSrcIdx) * WeightSize;		// Offset into kernel matrix of DstIdx and SrcIdx dimension.
		read(DWALIGNED(wIndex), WeightOffset * sizeof(float), vWeight);
		
		int iTileX = SrcIdx%iTileHoriCount;
		int iTileY = SrcIdx/iTileHoriCount;
		int iBaseX = iTileWidth * iTileX;		// Tile base x in float
		int iBaseY = iTileHeight * iTileY;		// Tile base y 

		// Read 31x16 floats input block on each input image
		read(iIndex, (iBaseX + nTX) * sizeof(float),		iBaseY + nTY,	 fSrc.select<8,1,8,1>(0,0));
		read(iIndex, (iBaseX + nTX) * sizeof(float),		iBaseY + nTY + 8, fSrc.select<8,1,8,1>(8,0));
		read(iIndex, (iBaseX + nTX) * sizeof(float),		iBaseY + nTY + 16, fSrc.select<8,1,8,1>(16,0));
		read(iIndex, (iBaseX + nTX) * sizeof(float),		iBaseY + nTY + 24, fSrc.select<7,1,8,1>(24,0));

		read(iIndex, (iBaseX + nTX + 8) * sizeof(float),	iBaseY + nTY,	 fSrc.select<8,1,4,1>(0,8));
		read(iIndex, (iBaseX + nTX + 8) * sizeof(float),	iBaseY + nTY + 8, fSrc.select<8,1,4,1>(8,8));
		read(iIndex, (iBaseX + nTX + 8) * sizeof(float),	iBaseY + nTY + 16, fSrc.select<8,1,4,1>(16,8));
		read(iIndex, (iBaseX + nTX + 8) * sizeof(float),	iBaseY + nTY + 24, fSrc.select<7,1,4,1>(24,8));

		// Convol 27x8 pixels in SIMD8
		#pragma unroll
		for (int i = 0; i < KWIDTH5; i++) {
			#pragma unroll
			for (int j = 0; j < KWIDTH5; j++) {
				fDst.select<27,1,8,1>(0,0) += vWeight[KWIDTH5*j+i] * fSrc.select<27,1,8,1>(j,i);
			}
		}
	}

	//int BiasOffset = FeatureOffsetInBatch;
	read(DWALIGNED(bIndex), FeatureOffsetInBatch * sizeof(float), vBias);
	fDst += vBias[0];	// Add bias

	if (EnableReLU)
		fDst.merge(0.0f, fDst < 0.0f);		// ReLU

	// Out of boundary handling
	vector<ushort, 8> horPos = InitHorPos8;
	vector<ushort, 32> verPos;
	#pragma unroll
	for(int i = 0; i < 4; i ++) 
		verPos.select<8,1>(8*i) = 8*i + horPos;
	horPos += (nX % oTileWidth);
	verPos += (nY % oTileHeight);
	#pragma unroll
	for(int i = 0; i < 27; i ++) {
		fDst.row(i).merge(0.0f, horPos >= DestWidth);	// Mute right side
		if (verPos[i] >= DestHeight)					// Mute bottom 
			fDst.row(i) = 0.0f;
	}

	// Output 27x8
	write(oIndex, (nX + Border) * sizeof(float), (nY + Border),			fDst.select<8,1,8,1>(0,0));
	write(oIndex, (nX + Border) * sizeof(float), (nY + Border + 8),		fDst.select<8,1,8,1>(8,0));
	write(oIndex, (nX + Border) * sizeof(float), (nY + Border + 16),	fDst.select<8,1,8,1>(16,0));
	write(oIndex, (nX + Border) * sizeof(float), (nY + Border + 24),	fDst.select<3,1,8,1>(24,0));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void Convol7x7_BPT(	SurfaceIndex iIndex, SurfaceIndex oIndex, 
											SurfaceIndex wIndex, SurfaceIndex bIndex,
											uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
											uint iTileHoriCount, uint iTileWidth, uint iTileHeight,
											uint oTileHoriCount, uint oTileWidth, uint oTileHeight,
											uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
											uint Border, uint NumGroups, uint EnableReLU)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<float, 22, 24> fSrc;								// Input block size = 22x22. 66 GRFs
	matrix<float, 8, 8> fDst(0.0f);							// Output block size = 8x8. 8 GRFs

	vector<float, 52> vWeight;								// 13 OWORD
	vector<float, 4> vBias;									// 1 DWORD

	int WeightSize = 49;									// 7*7 FP32
	//int BiasSize = 1;

	int BlkHoriOffset = h_pos % ThreadsWidthPerTile;		// Block X offset within the tile
	int BlkVertOffset = v_pos % ThreadsHeightPerTile;		// Block Y offset within the tile

	// Skip the block if the output block lands completely outside of actual output data region within a tile
	if (BlkHoriOffset * oBlkWidth8 >= DestWidth)
		return;
	if (BlkVertOffset * oBlkHeight8 >= DestHeight)
		return;

	// 2 pixels step size from one block (or thread) to the next, in horizontally and vertically directions.
	int OffsetX = BlkHoriOffset * iBlkWidth16;					// Offset from Tile origin in pixel
	int OffsetY = BlkVertOffset * iBlkHeight16;

	// Calcualte DstIdx from v_pos and h_pos. Threads working on one ouptut image share a common DstIdx.
	int DstIdx = oTileHoriCount * (v_pos / ThreadsHeightPerTile) + (h_pos / ThreadsWidthPerTile);
	int FeatureOffsetInBatch = DstIdx % DestDepth;	// Feature offset wihtin the batch

	for (int SrcIdx = 0; SrcIdx < SrcDepth; SrcIdx++) {

		// Offset into weight the matrix of DstIdx X SrcIdx, in which the features are in laid out in NCHW sequence.
		int WeightOffset = (FeatureOffsetInBatch * SrcDepth + SrcIdx) * WeightSize;
		// Read 52 FP32, 13 OWORD
		read(DWALIGNED(wIndex), WeightOffset * sizeof(float), vWeight.select<32, 1>(0));
		read(DWALIGNED(wIndex), (WeightOffset + 32) * sizeof(float), vWeight.select<20, 1>(32));

		// Read a input block, 22x32 floats
		int iTileX = SrcIdx % iTileHoriCount;
		int iTileY = SrcIdx / iTileHoriCount;
		int iBaseX = iTileWidth * iTileX;		// Tile base x in float
		int iBaseY = iTileHeight * iTileY;		// Tile base y

		// Read row 0-7
		read(iIndex, (iBaseX + OffsetX) * sizeof(float),		iBaseY + OffsetY, fSrc.select<8, 1, 8, 1>(0, 0));
		read(iIndex, (iBaseX + OffsetX + 8) * sizeof(float),	iBaseY + OffsetY, fSrc.select<8, 1, 8, 1>(0, 8));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(float),	iBaseY + OffsetY, fSrc.select<8, 1, 8, 1>(0, 16));
		// Read row 8-15
		read(iIndex, (iBaseX + OffsetX) * sizeof(float),		iBaseY + OffsetY + 8, fSrc.select<8, 1, 8, 1>(8, 0));
		read(iIndex, (iBaseX + OffsetX + 8) * sizeof(float),	iBaseY + OffsetY + 8, fSrc.select<8, 1, 8, 1>(8, 8));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(float),	iBaseY + OffsetY + 8, fSrc.select<8, 1, 8, 1>(8, 16));
		// Read row 16-21
		read(iIndex, (iBaseX + OffsetX) * sizeof(float),		iBaseY + OffsetY + 16, fSrc.select<6, 1, 8, 1>(16, 0));
		read(iIndex, (iBaseX + OffsetX + 8) * sizeof(float),	iBaseY + OffsetY + 16, fSrc.select<6, 1, 8, 1>(16, 8));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(float),	iBaseY + OffsetY + 16, fSrc.select<6, 1, 8, 1>(16, 16));

		// 7x7 convol on 8x8 pixels
		#pragma unroll
		for (int i = 0; i < KWIDTH7; i++) {
			#pragma unroll
			for (int j = 0; j < KWIDTH7; j++) {
				fDst.select<8, 1, 8, 1>(0, 0) += vWeight[KWIDTH7*j + i] * fSrc.select<8, 2, 8, 2>(j, i);
			}
		}
	}
	
	//int BiasOffset = FeatureOffsetInBatch;
	read(DWALIGNED(bIndex), FeatureOffsetInBatch * sizeof(float), vBias);  // Read 1 OWORD, only 1st float is used.
	fDst += vBias[0];						// Add bias

	if (EnableReLU)
		fDst.merge(0.0f, fDst < 0.0f);		// ReLU

	// Find last block for partial block write.  if 0, no partial block.
	int LastBlkWidth = DestWidth % oBlkWidth8;		// True last block width 
	int LastBlkHeight = DestHeight % oBlkHeight8;	// True last block height

	// Zero out area outside of image for partial write
	if (LastBlkWidth > 0)
		if ((BlkHoriOffset + 1) * oBlkWidth8 > DestWidth) {
			for (int c = LastBlkWidth; c < oBlkWidth8; c++)
				fDst.column(c) = 0.0f;
		}
	if (LastBlkHeight > 0)
		if ((BlkVertOffset + 1) * oBlkHeight8 > DestHeight) {
			for (int r = LastBlkHeight; r < oBlkHeight8; r++)
				fDst.row(r) = 0.0f;
		}

	int oTileX = DstIdx % oTileHoriCount;		// Output image location based on image id 
	int oTileY = DstIdx / oTileHoriCount;
	int oBaseX = oTileWidth * oTileX;		// Image origin in float
	int oBaseY = oTileHeight * oTileY;
	OffsetX = (h_pos%ThreadsWidthPerTile) * oBlkWidth8;		// Offset from Tile origin
	OffsetY = (v_pos%ThreadsHeightPerTile) * oBlkHeight8;

	write(oIndex, (oBaseX + OffsetX + Border) * sizeof(float), (oBaseY + OffsetY + Border), fDst);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 11x11 convolution with stride = 4.  Output 4x4.
extern "C" _GENX_MAIN_ void Convol11x11_BPT(SurfaceIndex iIndex, SurfaceIndex oIndex,
											SurfaceIndex wIndex, SurfaceIndex bIndex,
											uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
											uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
											uint oTileHoriCount, uint oTileWidth, uint oTileHeight, 
											uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
											uint Border, uint NumGroups, uint EnableReLU) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<float, 26, 32> fSrc;			// Input block size = 26x32. 104 GRFs
	matrix<float, 4, 4> fDst(0.0f);		// Output block size = 4x4. 2 GRFs
	
	vector<float, 132> vWeight;			// 33 OWORD
	matrix_ref<float, 12, 11> mWeight = vWeight.format<float, 12, 11>(); 
	vector<float, 4> vBias;				// 1 OWORD

	int WeightSize = 121;		// 11*11 FP32
	//int BiasSize = 1;

	int BlkHoriOffset = h_pos % ThreadsWidthPerTile;		// Block X offset within the tile
	int BlkVertOffset = v_pos % ThreadsHeightPerTile;		// Block Y offset within the tile

	// Skip the block if the output block lands completely outside of actual output data region within a tile
	if (BlkHoriOffset * oBlkWidth4 >= DestWidth)
		return;
	if (BlkVertOffset * oBlkHeight4 >= DestHeight)
		return;

	// 16 pixels step size from one block/thread to the next, in horizontally and vertically direction.
	int OffsetX = BlkHoriOffset * iBlkWidth16;					// Offset from Tile origin in pixel
	int OffsetY = BlkVertOffset * iBlkHeight16;

	// Calcualte DstIdx from v_pos and h_pos. Threads working on one ouptut image share a common DstIdx.
	int DstIdx = oTileHoriCount * (v_pos/ThreadsHeightPerTile) + (h_pos/ThreadsWidthPerTile);
	// DstId is shared among all batches.
	int FeatureOffsetInBatch = DstIdx%DestDepth;	// Feature offset wihtin a batch. Wrap around for multiple batches.

	for (int SrcIdx = 0; SrcIdx < SrcDepth; SrcIdx++) {

		// Offset into weight the matrix of DstIdx X SrcIdx, in which the features are in laid out in NCHW sequence.
		int WeightOffset = (FeatureOffsetInBatch * SrcDepth + SrcIdx) * WeightSize;		
		// Read 124 float, 31 OWORD
		read(DWALIGNED(wIndex), WeightOffset * sizeof(float), vWeight.select<32,1>(0));
		read(DWALIGNED(wIndex), (WeightOffset + 32) * sizeof(float), vWeight.select<32,1>(32));
		read(DWALIGNED(wIndex), (WeightOffset + 64) * sizeof(float), vWeight.select<32,1>(64));
		read(DWALIGNED(wIndex), (WeightOffset + 96) * sizeof(float), vWeight.select<28,1>(96));

		// Read a input block, 26x32 floats
		int iTileX = SrcIdx%iTileHoriCount;
		int iTileY = SrcIdx/iTileHoriCount;
		int iBaseX = iTileWidth * iTileX;		// Tile base x in float
		int iBaseY = iTileHeight * iTileY;		// Tile base y 

		read(iIndex, (iBaseX + OffsetX) * sizeof(float),		iBaseY + OffsetY,	 fSrc.select<8,1,8,1>(0,0));
		read(iIndex, (iBaseX + OffsetX + 8) * sizeof(float),	iBaseY + OffsetY,	 fSrc.select<8,1,8,1>(0,8));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(float),	iBaseY + OffsetY,	 fSrc.select<8,1,8,1>(0,16));
		read(iIndex, (iBaseX + OffsetX + 24) * sizeof(float),	iBaseY + OffsetY,	 fSrc.select<8,1,8,1>(0,24));

		read(iIndex, (iBaseX + OffsetX) * sizeof(float),		iBaseY + OffsetY + 8, fSrc.select<8,1,8,1>(8,0));
		read(iIndex, (iBaseX + OffsetX + 8) * sizeof(float),	iBaseY + OffsetY + 8, fSrc.select<8,1,8,1>(8,8));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(float),	iBaseY + OffsetY + 8, fSrc.select<8,1,8,1>(8,16));
		read(iIndex, (iBaseX + OffsetX + 24) * sizeof(float),	iBaseY + OffsetY + 8, fSrc.select<8,1,8,1>(8,24));

		read(iIndex, (iBaseX + OffsetX) * sizeof(float),		iBaseY + OffsetY + 16, fSrc.select<8,1,8,1>(16,0));
		read(iIndex, (iBaseX + OffsetX + 8) * sizeof(float),	iBaseY + OffsetY + 16, fSrc.select<8,1,8,1>(16,8));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(float),	iBaseY + OffsetY + 16, fSrc.select<8,1,8,1>(16,16));
		read(iIndex, (iBaseX + OffsetX + 24) * sizeof(float),	iBaseY + OffsetY + 16, fSrc.select<8,1,8,1>(16,24));

		read(iIndex, (iBaseX + OffsetX) * sizeof(float),		iBaseY + OffsetY + 24, fSrc.select<2,1,8,1>(24,0));
		read(iIndex, (iBaseX + OffsetX + 8) * sizeof(float),	iBaseY + OffsetY + 24, fSrc.select<2,1,8,1>(24,8));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(float),	iBaseY + OffsetY + 24, fSrc.select<2,1,8,1>(24,16));
		read(iIndex, (iBaseX + OffsetX + 24) * sizeof(float),	iBaseY + OffsetY + 24, fSrc.select<2,1,8,1>(24,24));

		// Align 8-11 weights mapped to element 1-4 for simd4 ops.  Element 7 is copied to element 0 but not used.
		matrix<float, 11, 5> mWeight_8_12(0); 
		mWeight_8_12.select<11,1,4,1>(0,0) = mWeight.select<11,1,4,1>(0,7);

		// 11x11 convol on 4x4 pixels
		vector<float, 16> vTemp(0.0f);
		matrix<float, 4, 32> mSum;

		// Calculate 4x4 output pixels
		#pragma unroll
		for (int r = 0; r < oBlkHeight4; r++) {
			int rr = Stride4 * r;

			#pragma unroll
			for (int c = 0; c < oBlkWidth4; c++) {
				int cc = Stride4 * c;

				vTemp.select<8,1>(0) = mWeight.select<1,1,8,1>(0,0) * fSrc.select<1,1,8,1>(0+rr,cc);
				#pragma unroll
				for (int i = 1; i < KWIDTH11; i++) {
					vTemp.select<8,1>(0) += mWeight.select<1,1,8,1>(i,0) * fSrc.select<1,1,8,1>(i+rr,cc);
				}
				vTemp.select<4,1>(8) = mWeight_8_12.select<1,1,4,1>(0,1) * fSrc.select<1,1,4,1>(0+rr,8+cc);
				#pragma unroll
				for (int i = 1; i < KWIDTH11; i++) {
					vTemp.select<4,1>(8) += mWeight_8_12.select<1,1,4,1>(i,1) * fSrc.select<1,1,4,1>(i+rr,8+cc);
				}
				//vTemp.select<8,1>(0) += vTemp.select<8,1>(8);
				//vSum.select<4,1>(OUTPUT_ROWS * rr + cc) = vTemp.select<4,1>(0) + vTemp.select<4,1>(4);
				mSum.select<1,1,8,1>(r, 8*c) = vTemp.select<8,1>(0) + vTemp.select<8,1>(8);
			}
		}

		// Final sum 
		mSum.select<4,1,16,1>(0,0) = mSum.select<4,1,16,2>(0,0) + mSum.select<4,1,16,2>(0,1);
		mSum.select<4,1,8,1>(0,0) = mSum.select<4,1,8,2>(0,0) + mSum.select<4,1,8,2>(0,1);
		fDst += mSum.select<4,1,4,2>(0,0) + mSum.select<4,1,4,2>(0,1);
	} // for

	//int BiasOffset = FeatureOffsetInBatch;
	read(DWALIGNED(bIndex), FeatureOffsetInBatch * sizeof(float), vBias);  // Read 1 OWORD, only 1st float is used.
	fDst += vBias[0];	// Add bias

	if (EnableReLU)
		fDst.merge(0.0f, fDst < 0.0f);		// ReLU

	// Find last block for partial block write.  if 0, no partial block.
	int LastBlkWidth = DestWidth % oBlkWidth4;		// True last block width 
	int LastBlkHeight = DestHeight % oBlkHeight4;	// True last block height

	// Zero out area outside of image for partial write
	if (LastBlkWidth > 0)
		if ((BlkHoriOffset+1) * oBlkWidth4 > DestWidth) {
			for (int c = LastBlkWidth; c < oBlkWidth4; c++)
				fDst.column(c) = 0.0f;
		}
	if (LastBlkHeight > 0)
		if ((BlkVertOffset+1) * oBlkHeight4 > DestHeight) {
			for (int r = LastBlkHeight; r < oBlkHeight4; r++)
				fDst.row(r) = 0.0f;
		}

	int oTileX = DstIdx%oTileHoriCount;		// Output image location based on image id 
	int oTileY = DstIdx/oTileHoriCount;
	int oBaseX = oTileWidth * oTileX;		// Image origin in float
	int oBaseY = oTileHeight * oTileY;
	OffsetX = (h_pos%ThreadsWidthPerTile) * oBlkWidth4;		// Offset from Tile origin
	OffsetY = (v_pos%ThreadsHeightPerTile) * oBlkHeight4;

	write(oIndex, (oBaseX + OffsetX + Border) * sizeof(float), (oBaseY + OffsetY + Border), fDst);
}

/////////////////////////////////////////////////////////////////////////
// New
extern "C" _GENX_MAIN_ void
LRN(SurfaceIndex INBUF,						//input, source image
  SurfaceIndex OUTBUF,						//output, destination image
  float Alpha,								//input, alpha /= 25 spatial; alpha /= 5, cross channel
  float Beta,								//input,
  float K,									//input,
  int Width,                                //input, image width of block
  int Height,								//input, image height of block
  int Blocks,								//input, block number per row
  int Images,								//input, total image per batch
  int Channel)								//input, accross channel
{
	if( Channel )
	{
		/* hor thread=width/8*image, -2 -1 0 1 2 image */
		vector<int, 2> srcXY;

		/* each thread handle 8*8 */
		matrix<float, 8, 8> prev2, prev1, cur, next1, next2, dst;

		srcXY(0) = get_thread_origin_x();
		srcXY(1) = get_thread_origin_y();

		/* from 2nd image */
		int imageID = (srcXY(1)/Height)*(Blocks/Width)+srcXY(0)/Width;

		srcXY(0) = srcXY(1)/Height*Blocks+srcXY(0);
		srcXY(1) = srcXY(1)%Height;

		int cur_y = srcXY(0)/Blocks*Height+srcXY(1);
		int cur_x = srcXY(0)%Blocks;

		read(INBUF, cur_x*8*sizeof(float), cur_y*8, cur);

		dst = cur*cur;

		int start = (imageID/Images)*Images;
		int end = (imageID/Images+1)*Images;

		/* todo, SIMD4 for neighbor coordinate calculation */
		if(imageID-2 >= start)
		{
			int prev2_y = (srcXY(0)-2*Width)/Blocks*Height+srcXY(1);
			int prev2_x = (srcXY(0)-2*Width)%Blocks;

			read(INBUF, prev2_x*8*sizeof(float), prev2_y*8, prev2);

			dst += prev2*prev2;
		}

		if(imageID-1 >= start)
		{
			int prev1_y = (srcXY(0)-Width)/Blocks*Height+srcXY(1);
			int prev1_x = (srcXY(0)-Width)%Blocks;

			read(INBUF, prev1_x*8*sizeof(float), prev1_y*8, prev1);

			dst += prev1*prev1;
		}

		if(imageID+1 < end)
		{
			int next1_y = (srcXY(0)+Width)/Blocks*Height+srcXY(1);
			int next1_x = (srcXY(0)+Width)%Blocks;

			read(INBUF, next1_x*8*sizeof(float), next1_y*8, next1);

			dst += next1*next1;
		}

		if(imageID+2 < end)
		{
			int next2_y = (srcXY(0)+2*Width)/Blocks*Height+srcXY(1);
			int next2_x = (srcXY(0)+2*Width)%Blocks;

			read(INBUF, next2_x*8*sizeof(float), next2_y*8, next2);

			dst += next2*next2;
		}

		dst = K + Alpha * dst;
		dst = cur / cm_pow(dst, Beta);

		write(OUTBUF, cur_x*8*sizeof(float), cur_y*8, dst);
	}
	else
	{
		vector<int, 2> srcXY;

		srcXY(0) = get_thread_origin_x();
		srcXY(1) = get_thread_origin_y();
		srcXY <<= 3;

		//kernel input, 16*12 temp block, 32bpp format, template size of horizontal/vertical is 5
		matrix<float, 12, 16> srcPic = 0.0f;

		//kernel output, each thread handle 8*8 temp block, 32bpp format
		matrix<float, 8, 8> dstPic = 0.0f;

		read(INBUF, (srcXY(0) - 2)  * sizeof(float), srcXY(1) - 2,  srcPic.select<8, 1, 8, 1>(0, 0));
		read(INBUF, (srcXY(0) + 6)  * sizeof(float), srcXY(1) - 2,  srcPic.select<8, 1, 8, 1>(0, 8));

		read(INBUF, (srcXY(0) - 2)  * sizeof(float), srcXY(1) + 6,  srcPic.select<4, 1, 8, 1>(8, 0));
		read(INBUF, (srcXY(0) + 6)  * sizeof(float), srcXY(1) + 6,  srcPic.select<4, 1, 8, 1>(8, 8));

		matrix<float, 12, 8> sumPic = 0.0f;

	#pragma unroll
		for(int i = 0; i < 12; i ++)
	#pragma unroll
			for(int j = 0; j < 4; j ++)
				sumPic.row(i) += srcPic.row(i).select<8, 1>(j) * srcPic.row(i).select<8, 1>(j);

	#pragma unroll
		for(int i = 0; i < 5;  i ++)
			dstPic.row(0) += sumPic.row(i);

	#pragma unroll
		for(int i = 1; i < 8;  i ++)
			dstPic.row(i) = dstPic.row(i-1) + sumPic.row(i+4) - sumPic.row(i-1);

		dstPic = K + Alpha * dstPic;
		dstPic = srcPic.select<8, 1, 8, 1>(2, 2) / cm_pow(dstPic, Beta);

		write(OUTBUF, srcXY(0)*sizeof(float), srcXY(1), dstPic);
	}
}

//////////////////////////////////////////////////////////////////////////////////
// Input block 8x8, output block 4x4
extern "C" _GENX_MAIN_ void
MaxPooling2D(SurfaceIndex INBUF,				//input, 2D
			SurfaceIndex OUTBUF,				//output, 2D
			vector<int, 2> ThreadsPerTile,		//input, thread width/height per tile
			vector<int, 2> oTileSize,			//input, otile width/height
			vector<int, 2> oDestSize,			//input, dest image width/height
			vector<int, 2> ImageBorder,			//input, left/top border for one image
			int PoolSize)						//input, pool size
{
	vector<int, 2> srcXY;
	srcXY(0) = get_thread_origin_x();
	srcXY(1) = get_thread_origin_y();

	int BlkSize = 8;										// Block width = Block height
	srcXY *= BlkSize;										// X,Y coordinate from frame origin in pixel
	srcXY += ImageBorder;									// Block origin for read

	// kernel output, each thread handle 4*4 block, 32bpp format
	matrix<float, 4, 4> dstPic = 0.0f;

	if (PoolSize == POOLSIZE_3x3) {

		matrix<float, 10, 16> srcPic = 0.0f;

		// Read 10x16 floats
		read(INBUF, srcXY[0] * sizeof(float),		srcXY[1],		srcPic.select<8, 1, 8, 1>(0, 0));
		read(INBUF, (srcXY[0] + 8) * sizeof(float),	srcXY[1],		srcPic.select<8, 1, 8, 1>(0, 8));
		read(INBUF, srcXY[0] * sizeof(float),		srcXY[1] + 8,	srcPic.select<2, 1, 8, 1>(8, 0));
		read(INBUF, (srcXY[0] + 8) * sizeof(float),	srcXY[1] + 8,	srcPic.select<2, 1, 8, 1>(8, 8));

		#pragma unroll
		for(int i = 0; i < 3; i ++)
			#pragma unroll
			for(int j = 0; j < 3; j ++)
				dstPic = cm_max<float>(srcPic.select<4, 2, 4, 2>(i, j), dstPic);

	} else if (PoolSize == POOLSIZE_2x2) {

		matrix<float, 8, 8> srcPic = 0.0f;

		read(INBUF, srcXY(0)*sizeof(float), srcXY(1),  srcPic);

		#pragma unroll
		for(int i = 0; i < 2; i ++)
			#pragma unroll
			for(int j = 0; j < 2; j ++)
				dstPic = cm_max<float>(srcPic.select<4, 2, 4, 2>(i, j), dstPic);
	}


	vector<ushort, 2> dstXY;
	dstXY[0] = get_thread_origin_x();
	dstXY[1] = get_thread_origin_y();

	BlkSize = 4;

	vector<int, 2> TileXY = dstXY % oTileSize;		// Thread coord from tile origin

	vector<ushort, 2> oBase;
	oBase = (dstXY / ThreadsPerTile) * oTileSize;	// Tile origin in pix
	oBase += (dstXY % ThreadsPerTile) * BlkSize;	// + Offset within tile in pix

	// Out of boundary handling
	vector<ushort, 4> horPos = InitHorPos4;
	vector<ushort, 4> verPos = horPos;

	horPos += ((4*dstXY[0]) % oTileSize[0]);
	verPos += ((4*dstXY[1]) % oTileSize[1]);
	#pragma unroll
	for(int i = 0; i < 4; i ++) {
		dstPic.row(i).merge(0.0f, horPos >= oDestSize[0]);	// Mute right side
		if (verPos[i] >= oDestSize[1])						// Mute bottom 
			dstPic.row(i) = 0.0f;
	}

	write(OUTBUF, (oBase[0] + ImageBorder[0]) * sizeof(float), oBase[1] + ImageBorder[1], dstPic);
}

////////////////////////////////////////////////////////////////////////////////////
// Assume tile size is 8x8, feature map size is 7x7, right and lower borders are 0s. 7x7 block averages down to 1x1.
// Process multiple input blocks.
extern "C" _GENX_MAIN_ void
AvgPooling2D(	SurfaceIndex INBUF,					// input, 2D
				SurfaceIndex OUTBUF,				// output, 1D
				int PoolSize,						// input, pool size
				int iTileHoriCount)
{
	vector<int, 2> srcXY;
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	uint nX = h_pos * iBlkWidth8;
	uint nY = v_pos * iBlkHeight8;

	matrix<float, 8, 8> srcPic;
	vector<float, 8> dst;

	read(INBUF, nX * sizeof(float), nY, srcPic);

	vector<uint, 8> vOffset = iTileHoriCount * v_pos + h_pos;	// Output offset
	dst = cm_sum<float>(srcPic) / (PoolSize * PoolSize);	// Get average of 7x7 elements

	write(OUTBUF, 0, vOffset, dst);							// Scattered write 1 float, 8 times
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is function supports
//		- 2D to 2D ReLU, block size = 8x8.
//		- 1D to 1D ReLU, block size = 64x1.
extern "C" _GENX_MAIN_ void ReLU(SurfaceIndex iIndex, SurfaceIndex oIndex, 
								 int InputMemType, int OutputMemType, int ImagesPerRow)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();
//	uint h_pos = cm_local_size(0) * cm_group_id(0) + cm_local_id(0);
//	uint v_pos = cm_local_size(1) * cm_group_id(1) + cm_local_id(1);

	matrix<float, 8, 8> fBlk;
	vector_ref<float, 64> fVec = fBlk.format<float>();

//	int BlkWidth = 8;										// Src block width
//	int BlkHeight = 8;										// Src block height
	int nX = iBlkWidth8 * h_pos;								// X coordinate based on frame origin
	int nY = iBlkHeight8 * v_pos;

	int ImageOffset = ImagesPerRow * v_pos + h_pos;
	int PixOffset = ImageOffset * iBlkWidth8 * iBlkHeight8;

	if (InputMemType == MEMORY_2D) {
		// Read 8x8 floats
		read(iIndex, nX * sizeof(float), nY, fBlk);
	} else {
		// Read 64 floats
		read(iIndex, PixOffset * sizeof(float), fVec.select<32,1>(0));
		read(iIndex, (PixOffset + 32) * sizeof(float), fVec.select<32,1>(32));
	}

	fBlk.merge(0.0f, fBlk < 0.0f);	// ReLU

	if (OutputMemType == MEMORY_2D) {
		// Write 8x8 float 
		write(oIndex, nX * sizeof(float), nY, fBlk);
	} else {
		// Write 64 float
		write(oIndex, PixOffset * sizeof(float), fVec.select<32,1>(0));
		write(oIndex, (PixOffset + 32) * sizeof(float), fVec.select<32,1>(32));
	} 
}

/////////////////////////////////////////////////////////////////////////////
// Convert 2D surface to 1D buffer.  Tile padding is removed in 1D output.
// For convert surface, it supports image size <= 8x8.  E.g. 6x6 and 7x7.
// To do: have one thread processing one tile and read input 2D data linearly within the tile.  Works on larger image.
extern "C" _GENX_MAIN_ void SurfConv(SurfaceIndex iIndex, SurfaceIndex oIndex, int ImagesPerRow, int DstWidth)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<float, 8, 8> fBlk;
	vector_ref<float, 64> fVec = fBlk.format<float>();
	vector<float, 49> DstBuffer;							// No tile padding is written to the ouptut buffer.
	vector<float, 8> dst;
	vector<uint, 8> vOffset;

	int nX = iBlkWidth8 * h_pos;								// X coordinate based on frame origin
	int nY = iBlkHeight8 * v_pos;

	int ImageOffset = ImagesPerRow * v_pos + h_pos;
	int PixOffset; // = ImageOffset * BlkWidth * BlkHeight;

	read(iIndex, nX * sizeof(float), nY, fBlk);

	// Currently suports 6x6 and 7x7 floats output
	if (DstWidth == 36) {
		DstBuffer.select<36, 1>(0) = fBlk.select<6, 1, 6, 1>(0, 0);
		PixOffset = ImageOffset * 36;

		write(oIndex, PixOffset * sizeof(float), DstBuffer.select<32,1>(0));				// OWORD write 32 floats
		write(oIndex, (PixOffset + 32) * sizeof(float), DstBuffer.select<4,1>(32));			// OWORD write 4 floats
	} else if (DstWidth == 49) {
		DstBuffer.select<49, 1>(0) = fBlk.select<7, 1, 7, 1>(0, 0);	
		PixOffset = ImageOffset * 49;

		// Check for OWORD alignment. OWORD aligned = 4 FP32 algined.
		if (PixOffset % 4 == 0) {			// 32+16+1=49
			write(oIndex, PixOffset * sizeof(float), DstBuffer.select<32,1>(0));			// OWORD write 32 floats
			write(oIndex, (PixOffset + 32) * sizeof(float), DstBuffer.select<16,1>(32));	// OWORD write 16 floats
			dst = DstBuffer(48);											
			vOffset = (PixOffset + 48);
			write(oIndex, 0, vOffset, dst);													// Scattered write 1 float, 8 times
		} else if (PixOffset % 4 == 1) {	// 3+32+8+4+2=49
			#pragma unroll
			for (int i = 0; i < 3; i++) {
				dst = DstBuffer(i);											
				vOffset = (PixOffset + i);
				write(oIndex, 0, vOffset, dst);													// Scattered write 3 float, 8 times
			}
			write(oIndex, (PixOffset + 3) * sizeof(float), DstBuffer.select<32,1>(3));			// OWORD write 32 floats
			write(oIndex, (PixOffset + 3+32) * sizeof(float), DstBuffer.select<8,1>(3+32));		// OWORD write 8 floats
			write(oIndex, (PixOffset + 3+32+8) * sizeof(float), DstBuffer.select<4,1>(3+32+8));	// OWORD write 4 floats
			#pragma unroll
			for (int i = 0; i < 2; i++) {
				dst = DstBuffer(3+32+8+4+i);											
				vOffset = (PixOffset + 3+32+8+4+i);
				write(oIndex, 0, vOffset, dst);													// Scattered write 2 float, 8 times
			}
		} else if (PixOffset % 4 == 2) {	// 2+32+8+4+3=49
			#pragma unroll
			for (int i = 0; i < 2; i++) {
				dst = DstBuffer(i);											
				vOffset = (PixOffset + i);
				write(oIndex, 0, vOffset, dst);													// Scattered write 2 float, 8 times
			}
			write(oIndex, (PixOffset + 2) * sizeof(float), DstBuffer.select<32,1>(2));			// OWORD write 32 floats
			write(oIndex, (PixOffset + 2+32) * sizeof(float), DstBuffer.select<8,1>(2+32));		// OWORD write 8 floats
			write(oIndex, (PixOffset + 2+32+8) * sizeof(float), DstBuffer.select<4,1>(2+32+8));	// OWORD write 4 floats
			#pragma unroll
			for (int i = 0; i < 3; i++) {
				dst = DstBuffer(2+32+8+4+i);											
				vOffset = (PixOffset + 2+32+8+4+i);
				write(oIndex, 0, vOffset, dst);													// Scattered write 3 float, 8 times
			}
		} else if (PixOffset % 4 == 3) {	// 1+32+16=49
			dst = DstBuffer(0);											
			vOffset = (PixOffset);
			write(oIndex, 0, vOffset, dst);														// Scattered write 1 float, 8 times
			write(oIndex, (PixOffset + 1) * sizeof(float), DstBuffer.select<32,1>(1));			// OWORD write 32 floats
			write(oIndex, (PixOffset + 1+32) * sizeof(float), DstBuffer.select<16,1>(1+32));	// OWORD write 16 floats
		}
	}
}

#ifdef FCWT_2D				// Use Surface2D for FC weight data
/////////////////////////////////////////////////////////////////////////////
// weight in 2D Surface
extern "C" _GENX_MAIN_ void
FullyConnection_NPatch(SurfaceIndex INBUF,	//input, 
				SurfaceIndex OUTBUF,	//output,
				SurfaceIndex COEFF,		//input, 
				SurfaceIndex BIAS,		//input, last layer image size/64
				int OneSrcBatchSize,	// Src size per batch
				int OneDestBatchSize,	// Dest size per batch
				uint HoriThreads,		// Thread space width
				int Batches,			// # of batches
				int ReLU)				//input, ReLU control flag
{
#define _ROWS 16
#define _COLS 8
#define _BATCH 100						// Suport up to 64 batches

	vector<float, _ROWS> feature;				// 2 grf
	matrix<float, _ROWS, _COLS> coeff;			// 16 grf
	vector<float, _COLS> bias;					// 2 grf
	vector<float, _COLS> dst;					// 2 grf, 32 output features
	matrix<float, _BATCH, _COLS> sum = 0.0f;	// 64 grf

	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();
	h_pos = v_pos * HoriThreads + h_pos;
	//v_pos = 1;

	int iBaseX = h_pos * _COLS;						// Offset in the cureent Batch for weight and bias
	int oBaseX = h_pos * _COLS;						// Offset of the cureent Batch for output

	read(BIAS, iBaseX * sizeof(float), bias.select<_COLS,1>(0));

	int iter = OneSrcBatchSize/_ROWS;		// each loop process 16 inputs for the same batch.

	// through all inputs
	for(int j = 0; j < iter; j++) {
		// Read 16x32 float
		read(COEFF, iBaseX * sizeof(float), _ROWS * j, coeff.select<8,1,8,1>(0,0));	// 64 float 
		read(COEFF, iBaseX * sizeof(float), ROWS * j + 8, coeff.select<8,1,8,1>(8,0));	// 64 float 

		// through batches
		for (int b = 0; b < Batches; b++) {
			read(INBUF, (OneSrcBatchSize*b + _ROWS*j) * sizeof(float), feature);			// 8 float

			#pragma unroll
			for (int i = 0; i < _ROWS; i++) 
				sum.row(b) += feature[i] * coeff.row(i);
		}
	}

	for (int b = 0; b < Batches; b++) {
		dst = sum.row(b) + bias;
		dst.merge(0.0f, (dst < 0.0f) & ReLU);
		write(OUTBUF, (oBaseX + OneDestBatchSize*b) * sizeof(float), dst);
	}
}

#else	// Not define FCWT_2D
////////////////////////////////////////////////////////////////////////////////////////////
// weight in FP32 format in 1D Surface
// This kernel processes 8 outputs in parallel.  The inputs are broken down multiple chunks.
// The chunk size is 16 x 8, where 8 corresonding to 8 outputs.

extern "C" _GENX_MAIN_ void
FullyConnection_NPatch(SurfaceIndex INBUF,	//input, 
				SurfaceIndex OUTBUF,	//output,
				SurfaceIndex COEFF,		//input, 
				SurfaceIndex BIAS,		//input, last layer image size/64
				int OneSrcBatchSize,	// Src size per batch
				int OneDestBatchSize,	// Dest size per batch
				uint HoriThreads,		// Thread space width
				int Batches,			// # of batches
				int ReLU)				//input, ReLU control flag
{
#define _BATCH 64						// Suport up to 64 batches

	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();
//	int h_pos = cm_local_size(0) * cm_group_id(0) + cm_local_id(0);
//	int v_pos = cm_local_size(1) * cm_group_id(1) + cm_local_id(1);

	vector<float, iBlkHeight16> feature;				// 2 grf
	matrix<float, iBlkHeight16, iBlkWidth8> coeff;			// 16 grf
	vector<float, iBlkWidth8> bias;					// 2 grf
	vector<float, iBlkWidth8> dst;					// 2 grf, 32 output features
	matrix<float, _BATCH, iBlkWidth8> sum = 0.0f;	// 64 grf
	int nWeightOffset;							// Weight data offset

	h_pos = v_pos * HoriThreads + h_pos;
	//v_pos = 1;

	int iBaseX = h_pos * iBlkWidth8;			// Offset in the cureent Batch for weight and bias
	int oBaseX = h_pos * iBlkWidth8;			// Offset of the cureent Batch for output

	read(BIAS, iBaseX * sizeof(float), bias.select<iBlkWidth8,1>(0));

	nWeightOffset = OneSrcBatchSize * iBaseX * sizeof(float);
	int iter = OneSrcBatchSize/ iBlkHeight16;		// each loop process 16 inputs for the same batch.

	// through all inputs
	for(int j = 0; j < iter; j++) {
		// Read 128 float
		read(COEFF, nWeightOffset, coeff.format<float>().select<32,1>(0));
		read(COEFF, nWeightOffset+128, coeff.format<float>().select<32,1>(32));
		read(COEFF, nWeightOffset+256, coeff.format<float>().select<32,1>(64));
		read(COEFF, nWeightOffset+384, coeff.format<float>().select<32,1>(96));

		// through batches
		for (int b = 0; b < Batches; b++) {
			read(INBUF, (OneSrcBatchSize * b + iBlkHeight16 * j) * sizeof(float), feature);			// 8 float

			#pragma unroll
			for (int i = 0; i < iBlkHeight16; i++)
				sum.row(b) += feature[i] * coeff.row(i);
		}
		nWeightOffset += iBlkHeight16 * iBlkWidth8 * sizeof(float);
	}

	for (int b = 0; b < Batches; b++) {
		dst = sum.row(b) + bias;
		dst.merge(0.0f, (dst < 0.0f) & ReLU);
		write(OUTBUF, (oBaseX + OneDestBatchSize*b) * sizeof(float), dst);
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Weight in quantized Int8 format in 1D Surface
extern "C" _GENX_MAIN_ void
FullyConnection_NPatch_QWT(SurfaceIndex INBUF,	//input, 
				SurfaceIndex OUTBUF,	//output,
				SurfaceIndex COEFF,		//input, 
				SurfaceIndex BIAS,		//input, last layer image size/64
				int OneSrcBatchSize,	// Src size per batch
				int OneDestBatchSize,	// Dest size per batch
				uint HoriThreads,		// Thread space width
				int Batches,			// # of batches
				int ReLU,				//input, ReLU control flag
				float fA,				// Inverse quantization parameters
				float fB)
{
//#define _ROWS 16
//#define _COLS 8
#define _BATCH 64						// Suport up to 64 batches

	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();
//	int h_pos = cm_local_size(0) * cm_group_id(0) + cm_local_id(0);
//	int v_pos = cm_local_size(1) * cm_group_id(1) + cm_local_id(1);

	vector<float, iBlkHeight16> feature;				// 2 grf
	matrix<float, iBlkHeight16, iBlkWidth8> coeff;			// 16 grf
	vector<float, iBlkWidth8> bias;					// 2 grf
	vector<float, iBlkWidth8> dst;					// 2 grf, 32 output features
	matrix<float, _BATCH, iBlkWidth8> sum = 0.0f;	// 64 grf
    vector<char, iBlkHeight16*iBlkWidth8> WeightData;		// Weight data
	int nWeightOffset;							// Weight data offset

	h_pos = v_pos * HoriThreads + h_pos;
	//v_pos = 1;

	int iBaseX = h_pos * iBlkWidth8;						// Offset in the cureent Batch for weight and bias
	int oBaseX = h_pos * iBlkWidth8;						// Offset of the cureent Batch for output

	read(BIAS, iBaseX * sizeof(float), bias.select<iBlkWidth8,1>(0));

	nWeightOffset = OneSrcBatchSize * iBaseX;

	int iter = OneSrcBatchSize/iBlkHeight16;		// each loop process 16 inputs for the same batch.

	// through all inputs
	for(int j = 0; j < iter; j++) {
		// Read 128 bytes
		read(COEFF, nWeightOffset, WeightData);
		coeff.format<float>() = fA * WeightData + fB;

		// through batches
		for (int b = 0; b < Batches; b++) {
			read(INBUF, (OneSrcBatchSize*b + iBlkHeight16*j) * sizeof(float), feature);			// 8 float

			#pragma unroll
			for (int i = 0; i < iBlkHeight16; i++) 
				sum.row(b) += feature[i] * coeff.row(i);
		}
		nWeightOffset += iBlkHeight16 * iBlkWidth8;
	}

	for (int b = 0; b < Batches; b++) {
		dst = sum.row(b) + bias;
		dst.merge(0.0f, (dst < 0.0f) & ReLU);
		write(OUTBUF, (oBaseX + OneDestBatchSize*b) * sizeof(float), dst);
	}
}


////////////////////////////////////////////
// Softmax process 1000 features
extern "C" _GENX_MAIN_ void
Softmax(SurfaceIndex INBUF,		//input, 
		SurfaceIndex OUTBUF,		//output
		int oDepth,
		int FeaturesPerThread)
{
	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();

	/* each thread handle 8 neurons*/
	vector<float, 32> feature = 0.0f;
	vector<float, 32> sum = 0.0f;
	vector<float, 8> bias;

	int BatchOffset = v_pos * oDepth;

	// Go through all the features of the last layer images, 0 - 992 
	#pragma unroll
	for(int i = 0; i < 31; i ++) {
		read(INBUF, (BatchOffset + 32*i) * sizeof(float), feature);
		sum += cm_pow(2.718f, feature);
	}

	// and last 8 features 
	vector<float, 8> last8;
	read(INBUF, (BatchOffset + 32*31) * sizeof(float), last8);
	last8 = cm_pow(2.718f, last8);

	/* calculate the sum of 32 features */
	vector<float, 16> mid16 = sum.select<16, 1>(0) + sum.select<16, 1>(16);
	vector<float, 8> mid8 = mid16.select<8, 1>(0) + mid16.select<8, 1>(8) + last8;
	vector<float, 4> mid4 = mid8.select<4, 1>(0) + mid8.select<4, 1>(4);
	vector<float, 2> mid2 = mid4.select<2, 1>(0) + mid4.select<2, 1>(2);
	float st = mid2(0) + mid2(1);

	int xOff = BatchOffset + FeaturesPerThread * h_pos;
	read(INBUF, xOff * sizeof(float), bias);

	bias = cm_pow(2.718f, bias);
	bias /= st;

	write(OUTBUF, xOff * sizeof(float), bias);
}


//#ifndef CMRT_EMU
//#include "cmDNN_FP16_genx.hpp"
//#endif

