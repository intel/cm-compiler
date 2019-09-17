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

#ifdef CMRT_EMU
#include <cm/half.h>
using namespace half_float;
#endif

#include <cm/cm.h>
#include "cm/cmtl.h"

#define KWIDTH3		3
#define KWIDTH5		5
#define KWIDTH11	11

static const int InitHorPos_HF[] = {0, 1, 2 ,3, 4, 5, 6, 7};
static const int Init0123[] = {0, 1, 2 ,3};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Process a 16x16 tile.
extern "C" _GENX_MAIN_ void InputProc_HF(SurfaceIndex iIndex, SurfaceIndex iIndex2, SurfaceIndex oIndex, uint Border)
{
	// h_pos and v_pos maps to output tile origin.
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<float, 8, 8> fInputData;		// 8 GRFs
	matrix<float, 8, 8> fCnnAvg;		// 8 GRFs
	matrix<half, 8, 8> fDst;			// 8 GRFs

	int BlkWidth = 8;
	int BlkHeight = 8;

	int nX = BlkWidth * h_pos;		// Based on frame origin
	int nY = BlkHeight * v_pos;

	read(iIndex, nX * sizeof(float), nY, fInputData);
	read(iIndex2, nX * sizeof(float), nY, fCnnAvg);

	fDst = fInputData - fCnnAvg;
	fCnnAvg = fDst;

	write(oIndex, (nX + Border) * sizeof(half), nY + Border, fDst);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Input image up to 16x16
// Output image < 15x15
extern "C" _GENX_MAIN_ void Convol3x3_IPT_2D_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, 
										SurfaceIndex wIndex, SurfaceIndex bIndex,
										uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
										uint iTileHoriCount, uint iTileWidth, uint iTileHeight,
										uint oTileHoriCount, uint oTileWidth, uint oTileHeight,
										uint iBorder, uint oBorder, uint NumGroups, uint EnableReLU ) 
{
	// h_pos and v_pos maps to output tile origin.
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<half, 16, 32> fSrc(0.0f);	// GRF 32
	matrix<half, 16, 16> fDst(0.0f);	// GRF 26
	
	vector<half, 16> vWeight;
	vector<half, 8> vBias;

	int WeightSize = 9;		// 3*3

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

	for (int SrcIdx = SrcIdxStart, KernalIdx = 0; SrcIdx < SrcIdxEnd; SrcIdx++, KernalIdx++) {
		int iTileX = SrcIdx%iTileHoriCount;
		int iTileY = SrcIdx/iTileHoriCount;
		int iBaseX = iTileWidth * iTileX;
		int iBaseY = iTileHeight * iTileY;

		// Read 16x32 half input block
		// In FP16, border is doubled.  Need to subtract 1 to read at correct offset.
		read(iIndex, (iBaseX + iBorder - 1) * sizeof(half), iBaseY + iBorder - 1, fSrc.select<8,1,16,1>(0,0));
		read(iIndex, (iBaseX + iBorder - 1) * sizeof(half), iBaseY + iBorder - 1 + 8, fSrc.select<8,1,16,1>(8,0));

		// Read a set of filter weights and bias
		int GroupAdjustedSrcIdx = (GroupID == 0) ? SrcIdx : (SrcIdx - SrcDepthPerGroup);	// For 2nd group handling
		int WeightOffset = (DstIdx%DestDepth * SrcDepthPerGroup + GroupAdjustedSrcIdx) * WeightSize;		// Offset into kernel matrix of DstIdx and SrcIdx dimension.
		read(DWALIGNED(wIndex), WeightOffset * sizeof(half), vWeight);

		// Convol left 13x16 pixels in SIMD16
		#pragma unroll
		for (int i = 0; i < KWIDTH3; i++) {
			#pragma unroll
			for (int j = 0; j < KWIDTH3; j++) {
				fDst.select<14,1,16,1>(0,0) += vWeight[KWIDTH3*j+i] * fSrc.select<14,1,16,1>(j,i);
			}
		}
	}

	//int BiasOffset = FeatureOffsetInBatch;
	read(DWALIGNED(bIndex), FeatureOffsetInBatch * sizeof(half), vBias);
	fDst += vBias[0];	// Add bias

    if (EnableReLU)
	    fDst.merge((half)0.0f, fDst < (half)0.0f);		// ReLU

	// Zero out area outside of image for partial write
	if (DestWidth < iBlkWidth16) {		// Last block horizontally
		for (int c = DestWidth; c < iBlkWidth16; c++)
			fDst.column(c) = (half)0.0f;
	}
	if (DestHeight < iBlkHeight16) {		// last block vertically
		for (int r = DestHeight; r < iBlkHeight16; r++)
			fDst.row(r) = (half)0.0f;
	}

	int oBaseX = h_pos * oTileWidth;
	int oBaseY = v_pos * oTileHeight;
	
	write(oIndex, (oBaseX + oBorder) * sizeof(half), oBaseY + oBorder,		fDst.select<8,1,16,1>(0,0));
	write(oIndex, (oBaseX + oBorder) * sizeof(half), oBaseY + oBorder + 8,	fDst.select<8,1,16,1>(8,0));
}
 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This kernel performs 3x3 convolution on 8x8 multiple blocks  
extern "C" _GENX_MAIN_ void Convol3x3_BPT_8x8_MB_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, 
													SurfaceIndex wIndex, SurfaceIndex bIndex,
													uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
													uint iTileHoriCount, uint iTileVertCount, uint iTileWidth, uint iTileHeight, 
													uint oTileHoriCount, uint oTileVertCount, uint oTileWidth, uint oTileHeight, 
													uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
													uint iBorder, uint oBorder, uint EnableReLU ) 
{
	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();

	// Output blocks' coordinates.
	vector<int, MULTIBLOCKS> OutX, OutY;

	matrix<half, 8+2, 16+2> fSrc;
	matrix<half, 8*MULTIBLOCKS, 16> fDst((half)0.0f);
	vector<half, 16> vWeight;
	vector<half, MULTIBLOCKS> vBias;

//	int BlkWidth = 16;										// Block width
//	int BlkHeight = 8;										// Block height

	// Output horizontal and vertical blocks of entire surface
	int oHoriBlocks = oTileHoriCount * ThreadsWidthPerTile;
	int oVertBlocks = oTileVertCount * ThreadsHeightPerTile;
	// Input horizontal and vertical blocks of the surface
	int iHoriBlocks = iTileHoriCount * ThreadsWidthPerTile;
	int iVertBlocks = iTileVertCount * ThreadsHeightPerTile;

	int h_pos2 = (h_pos / ThreadsWidthPerTile) * ThreadsWidthPerTile * MULTIBLOCKS + (h_pos % ThreadsWidthPerTile); 
	int oBlockID = v_pos * oHoriBlocks + h_pos2;

	// Distance to next collocated block: add ThreadsWidthPerTile.
	for (int i = 0, j = oBlockID; i < MULTIBLOCKS; i++, j += ThreadsWidthPerTile) {
		OutX[i] = j % oHoriBlocks;							// X coordinate from frame origin in block
		OutY[i] = j / oHoriBlocks;							// Y coordinate from frame origin in block
	}

	int nX = h_pos * iBlkWidth16;								// X coordinate from frame origin
	int nY = v_pos * iBlkHeight8;								// Y coordinate from frame origin
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
			read(iIndex, (iBase[0] + TileX + iBorder - 1) * sizeof(half), iBase[1] + TileY + iBorder - 1, fSrc.select<8, 1, 8, 1>(0, 0));
		} else {
			// Read a 10x18 block
			read(iIndex, (iBase[0] + TileX + iBorder - 1) * sizeof(half), iBase[1] + TileY + iBorder - 1, fSrc.select<8, 1, 16, 1>(0, 0));
			read(iIndex, (iBase[0] + TileX + iBorder - 1 + 16) * sizeof(half), iBase[1] + TileY + iBorder - 1, fSrc.select<8, 1, 2, 1>(0, 16));
			read(iIndex, (iBase[0] + TileX + iBorder - 1) * sizeof(half), iBase[1] + TileY + iBorder - 1 + 8, fSrc.select<2, 1, 16, 1>(8, 0));
			read(iIndex, (iBase[0] + TileX + iBorder - 1 + 16) * sizeof(half), iBase[1] + TileY + iBorder - 1 + 8, fSrc.select<2, 1, 2, 1>(8, 16));
		}

		#pragma unroll
		for (int i = 0; i < MULTIBLOCKS; i++) {
			int TileIDx = OutX[i] / ThreadsWidthPerTile;					// Tile id X. Thread (v_pos, h_pos) falls in this tile.
			int TileIDy = OutY[i] / ThreadsHeightPerTile;					// Tile id Y, Thread (v_pos, h_pos) falls in this tile.
			int DestTileID = TileIDy * oTileHoriCount + TileIDx;			// Dest tile id	
			int WeightSet = DestTileID % DestDepth * SrcDepth;				// % DestDepth for multi-batch. Weightset is total weight size for all inputs and one output.
			int KernelOffset = (WeightSet + SrcIdx) * WeightSize;		// Offset into weight matrix of DstTid and SrcIdx dimension.

			read(DWALIGNED(wIndex), KernelOffset * sizeof(half), vWeight);

			#pragma unroll
			for (int c = 0; c < KWIDTH3; c++) {
				#pragma unroll
				for (int r = 0; r < KWIDTH3; r++) {
					fDst.select<8,1,16,1>(8*i,0) += vWeight[KWIDTH3*r+c] * fSrc.select<8,1,16,1>(r,c);
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
		fDst.select<8, 1, 16, 1>(8 * i, 0) += vBias[i];
	}

	// Apply ReLU
	if (EnableReLU)
		fDst.select<8*MULTIBLOCKS,1,16,1>(0,0).merge((half)0.0f, fDst.select<8*MULTIBLOCKS,1,16,1>(0,0) < (half)0.0f);

	// Out of boundary handling
	vector<ushort, 16> horPos = InitHorPos16;
	vector<ushort, 8> verPos = InitHorPos8;
	horPos += (nX % oTileWidth);
	verPos += (nY % oTileHeight);

	#pragma unroll
	for(int j = 0; j < 8; j++) {
		#pragma unroll
		for(int i = 0; i < MULTIBLOCKS; i++)
			fDst.row(8*i+j).merge((half)0.0f, horPos >= DestWidth);	// Mute right side
		if (verPos[j] >= DestHeight) {							// Mute bottom 
			#pragma unroll
			for(int i = 0; i < MULTIBLOCKS; i++)
				fDst.row(8*i+j) = (half)0.0f;	
		}
	}

	#pragma unroll
	for (int i = 0; i < MULTIBLOCKS; i++)
		write(oIndex, (OutX[i] * oBlkWidth16 + oBorder) * sizeof(half), (OutY[i] * oBlkHeight8 + oBorder), fDst.select<8,1,16,1>(8*i,0));
}

#ifdef NOT_USED
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This kernel performs 3x3 convolution on 8x8 block.  
// Each thread processes colocated 8x8 block on 8 destination surface.  
// The 8 destination blocks share the same set of input image blocks.  This approach reduces memory read.
//
extern "C" _GENX_MAIN_ void Convol3x3_BPT8x8_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex kIndex, 
												uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
												uint iBorder, uint oBorder, uint NumGroups, uint EnableReLU,
												uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
												uint oTileHoriCount, uint oTileWidth, uint oTileHeight, 
												uint ThreadsWidthPerTile, uint ThreadsHeightPerTile) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	int BlkWidth = 16;										// Block width
	int BlkHeight = 16;
	int nX = h_pos * BlkWidth;								// X coordinate based on frame origin
	int nY = v_pos * BlkHeight;
	int TileX = nX % oTileWidth;							// X coordinate based on current tile origin
	int TileY = nY % oTileHeight;

	int BlkHoriOffset = h_pos % ThreadsWidthPerTile;		// Block X offset within the tile
	int BlkVertOffset = v_pos % ThreadsHeightPerTile;
	
	// Input block size = 10x32.  Output block size = 8x16.
	matrix<half, 16+2, 32> fSrc;
	matrix<half, 16, 16> fDst(0.0f);
	vector<half, 16> vKW;

	// v_pos and h_pos are grid from output surface.  DstIdx can be derived from them.  16 threads work on one ouptut image and they share one DstIdx.
	int KernelBiasSize = 10;		// 3*3+1
	int SrcIdxStart, SrcIdxEnd, GroupID;
	int SrcDepthPerGroup = SrcDepth / NumGroups;

	int TileIDx = h_pos / ThreadsWidthPerTile;		// Tile X coodinate that (v_pos, h_pos) falls in
	int TileIDy = v_pos / ThreadsHeightPerTile;
	int DestTileID = TileIDy * oTileHoriCount + TileIDx;	

	int FeatureOffsetInBatch = DestTileID % DestDepth;	// Feature offset wihtin the batch

	if (FeatureOffsetInBatch < DestDepth/NumGroups) {	// in the 1st half of DestDepth -> group 0
		SrcIdxStart = 0;
		SrcIdxEnd = SrcDepthPerGroup;
		GroupID = 0;
	} else {
		SrcIdxStart = SrcDepthPerGroup;
		SrcIdxEnd = SrcDepth;
		GroupID = 1;
	}

	for (int SrcIdx = SrcIdxStart; SrcIdx < SrcIdxEnd; SrcIdx++) {

		int iTileX = SrcIdx%iTileHoriCount;
		int iTileY = SrcIdx/iTileHoriCount;
		int iBaseX = iTileWidth * iTileX;		// Tile base x in float
		int iBaseY = iTileHeight * iTileY;		// Tile base y 

		// Read 10x20 floats input block
		read(iIndex, (iBaseX + TileX + iBorder - 1) * sizeof(half),			iBaseY + TileY + iBorder - 1,		fSrc.select<8,1,16,1>(0,0));
		read(iIndex, (iBaseX + TileX + iBorder - 1 + 16) * sizeof(half),	iBaseY + TileY + iBorder - 1,		fSrc.select<8,1,16,1>(0,16));
		read(iIndex, (iBaseX + TileX + iBorder - 1) * sizeof(half),			iBaseY + TileY + iBorder - 1 + 8,	fSrc.select<8,1,16,1>(8,0));
		read(iIndex, (iBaseX + TileX + iBorder - 1 + 16) * sizeof(half),	iBaseY + TileY + iBorder - 1 + 8,	fSrc.select<8,1,16,1>(8,16));
		read(iIndex, (iBaseX + TileX + iBorder - 1) * sizeof(half),			iBaseY + TileY + iBorder - 1 + 16,	fSrc.select<2,1,16,1>(16,0));
		read(iIndex, (iBaseX + TileX + iBorder - 1 + 16) * sizeof(half),	iBaseY + TileY + iBorder - 1 + 16,	fSrc.select<2,1,16,1>(16,16));

		int GroupAdjustedSrcIdx = (GroupID == 0) ? SrcIdx : (SrcIdx - SrcDepthPerGroup);	// For 2nd group handling

		// Read a set of filter weights and bias
		int KernelOffset = (DestTileID % DestDepth * SrcDepthPerGroup + GroupAdjustedSrcIdx) * KernelBiasSize;		// Offset into kernel matrix of DstIdx and SrcIdx dimension.
		read(DWALIGNED(kIndex), KernelOffset * sizeof(half), vKW);

		// Convol 8*ONE8x8 x 8 pixels in SIMD16
		#pragma unroll
		for (int i = 0; i < KWIDTH3; i++) {
			#pragma unroll
			for (int j = 0; j < KWIDTH3; j++) {
//				fDst.select<16,1,16,1>(0,0) += vKW[KWIDTH3*j+i] * fSrc.select<16,1,16,1>(j,i);
				fDst += vKW[KWIDTH3*j+i] * fSrc.select<16,1,16,1>(j,i);
			}
		}
	}

	// Add bias
	fDst += vKW[KernelBiasSize-1];
	
	// Apply ReLU
	if (EnableReLU) {
		fDst.merge((half)0.0f, fDst < (half)0.0f);
	}

	// From pos_h and pos_v, find the offsets of the blocks.
	int ThreadWidth = oTileHoriCount * ThreadsWidthPerTile;				// Hori thread count without consider SET8
	int TreadID = ThreadWidth * v_pos + h_pos;							// Thread id = block id

	// Prepare for output
	int A = (TreadID/ThreadsWidthPerTile) * ThreadsWidthPerTile;		// Tile offset for SET8
	int B = A + TreadID % ThreadsWidthPerTile;								// + tile offset within a tile
	int oBaseX = B % ThreadWidth;											// Hori thread offset in thread for colocated blocks
	int oBaseY = B / ThreadWidth;											// Vert threas offset in thread

	// Out of boundary handling
	vector<ushort, 16> horPos = InitHorPos16;
	vector<ushort, 16> verPos = horPos;

	horPos += (nX % oTileWidth);
	verPos += (nY % oTileHeight);
	#pragma unroll
	for(int i = 0; i < 16; i ++) {
		fDst.row(i).merge((half)0.0f, horPos >= DestWidth);	// Mute right side
		if (verPos[i] >= DestHeight)					// Mute bottom 
			fDst.row(i) = (half)0.0f;
	}

	write(oIndex, (oBaseX * BlkWidth + oBorder) * sizeof(half), (oBaseY * BlkHeight + oBorder), fDst.select<8,1,16,1>(0, 0));
	write(oIndex, (oBaseX * BlkWidth + oBorder) * sizeof(half), (oBaseY * BlkHeight + oBorder + 8), fDst.select<8,1,16,1>(8, 0));
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void Convol5x5_BPT27x16_HF(	SurfaceIndex iIndex, SurfaceIndex oIndex, 
													SurfaceIndex wIndex, SurfaceIndex bIndex,
													uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
	   												uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
													uint oTileHoriCount, uint oTileWidth, uint oTileHeight, 
													uint ThreadsWidthPerTile, uint ThreadsHeightPerTile, 
													uint iBorder, uint oBorder, uint NumGroups, uint EnableReLU)

{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	int EndBlkWidth = DestWidth % iBlkWidth16;					// End block width
	int EndBlkHeight = DestHeight % iBlkHeight32;
	int nX = iBlkWidth16 * h_pos;			// Based on frame origin
	int nY = iBlkHeight32 * v_pos;
	int nTX = nX % iTileWidth;			// Base on Tile origin
	int nTY = nY % iTileHeight;
	int TileHoriOffset = h_pos / ThreadsWidthPerTile;		// Tile X coodinate that (v_pos, h_pos) falls in
	int TileVertOffset = v_pos / ThreadsHeightPerTile;
	int BlkHoriOffset = h_pos % ThreadsWidthPerTile;		// Block X offset within the tile
	int BlkVertOffset = v_pos % ThreadsHeightPerTile;

	// Build block rows to loop through
	int BlkCols = (BlkHoriOffset+1 == ThreadsWidthPerTile) ? EndBlkWidth : iBlkWidth16;
	int BlkRows = (BlkVertOffset+1 == ThreadsHeightPerTile) ? EndBlkHeight : iBlkHeight32;

	matrix<half, 31, 20> fSrc;			// GRF 62
//	matrix<half, 27, 16> fDst(0.0f);	// GRF 27
	matrix<half, 32, 16> fDst(0.0f);	// GRF 27

	vector<float, 32> vWeight;			// 4 OWORD
	vector<float, 8> vBias;				// 1 OWORD

	int WeightSize = 25;				// 5*5 FP32

	// v_pos and h_pos are grid from output surface.  DstIdx can be derived from them.  16 threads work on one ouptut image and they share one DstIdx.
	
//	int KernelBiasSize = 26;		// 5*5+1
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
		// Read a set of filter weights and bias
		int GroupAdjustedSrcIdx = (GroupID == 0) ? SrcIdx : (SrcIdx - SrcDepthPerGroup);	// For 2nd group handling
		int WeightOffset = (DstIdx%DestDepth * SrcDepthPerGroup + GroupAdjustedSrcIdx) * WeightSize;		// Offset into kernel matrix of DstIdx and SrcIdx dimension.
		read(DWALIGNED(wIndex), WeightOffset * sizeof(half), vWeight);

		int iTileX = SrcIdx%iTileHoriCount;
		int iTileY = SrcIdx/iTileHoriCount;
		int iBaseX = iTileWidth * iTileX;		// Tile base x in float
		int iBaseY = iTileHeight * iTileY;		// Tile base y 
		
		// Read 31x20 half input block on each input image
		read(iIndex, (iBaseX + nTX) * sizeof(half),		iBaseY + nTY,	 fSrc.select<8,1,16,1>(0,0));
		read(iIndex, (iBaseX + nTX) * sizeof(half),		iBaseY + nTY + 8, fSrc.select<8,1,16,1>(8,0));
		read(iIndex, (iBaseX + nTX) * sizeof(half),		iBaseY + nTY + 16, fSrc.select<8,1,16,1>(16,0));
		read(iIndex, (iBaseX + nTX) * sizeof(half),		iBaseY + nTY + 24, fSrc.select<7,1,16,1>(24,0));

		read(iIndex, (iBaseX + nTX + 16) * sizeof(half),	iBaseY + nTY,	 fSrc.select<8,1,4,1>(0,16));
		read(iIndex, (iBaseX + nTX + 16) * sizeof(half),	iBaseY + nTY + 8, fSrc.select<8,1,4,1>(8,16));
		read(iIndex, (iBaseX + nTX + 16) * sizeof(half),	iBaseY + nTY + 16, fSrc.select<8,1,4,1>(16,16));
		read(iIndex, (iBaseX + nTX + 16) * sizeof(half),	iBaseY + nTY + 24, fSrc.select<7,1,4,1>(24,16));


		// Convol27x16 pixels in SIMD16
		#pragma unroll
		for (int i = 0; i < KWIDTH5; i++) {
			#pragma unroll
			for (int j = 0; j < KWIDTH5; j++) {
				fDst.select<27,1,16,1>(0,0) += vWeight[KWIDTH5*j+i] * fSrc.select<27,1,16,1>(j,i);
			}
		}
	}

	//int BiasOffset = FeatureOffsetInBatch;
	read(DWALIGNED(bIndex), FeatureOffsetInBatch * sizeof(half), vBias);
	fDst += vBias[0];	// Add bias

	if (EnableReLU)
	    fDst.merge((half)0.0f, fDst < (half)0.0f);		// ReLU

	// Find last block for partial block write.  if 0, no partial block.
	int LastBlkWidth = DestWidth % iBlkWidth16;		// True last block width 
	int LastBlkHeight = DestHeight % iBlkHeight32;	// True last block height

	// Zero out area outside of image for partial write
	if (LastBlkWidth > 0)
		if ((BlkHoriOffset+1) * iBlkWidth16 > DestWidth) {
			for (int c = LastBlkWidth; c < iBlkWidth16; c++)
				fDst.column(c) = (half) 0.0f;
		}
	if (LastBlkHeight > 0)
		if ((BlkVertOffset+1) * iBlkHeight32 > DestHeight) {
			for (int r = LastBlkHeight; r < iBlkHeight32; r++)
				fDst.row(r) = (half) 0.0f;
		}

	write(oIndex, (nX + oBorder) * sizeof(half),		(nY + oBorder),			fDst.select<8,1,16,1>(0,0));
	write(oIndex, (nX + oBorder) * sizeof(half),		(nY + oBorder + 8),		fDst.select<8,1,16,1>(8,0));
	write(oIndex, (nX + oBorder) * sizeof(half),		(nY + oBorder + 16),	fDst.select<8,1,16,1>(16,0));
//	write(oIndex, (nX + oBorder) * sizeof(half),		(nY + oBorder + 24),	fDst.select<3,1,16,1>(24,0));
	write(oIndex, (nX + oBorder) * sizeof(half),		(nY + oBorder + 24),	fDst.select<8,1,16,1>(24,0));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void Convol11x11_BPT_HF(	SurfaceIndex iIndex, SurfaceIndex oIndex, 
												SurfaceIndex wIndex, SurfaceIndex bIndex,
												uint SrcDepth, uint DestDepth, uint DestWidth, uint DestHeight,
												uint iTileHoriCount, uint iTileWidth, uint iTileHeight, 
												uint oTileHoriCount, uint oTileWidth, uint oTileHeight, 
												uint ThreadsWidthPerTile, uint ThreadsHeightPerTile,
												uint iBorder, uint oBorder, uint NumGroups, uint EnableReLU)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<half, 26, 32> fSrc;			// 104 GRFs
	matrix<half, 4, 4> fDst(0.0f);		// 2 GRFs

	vector<half, 132> vWeight;			// 16 DWORD
	matrix_ref<half, 12, 11> mWeight = vWeight.format<half, 12, 11>();
	vector<half, 8> vBias;				// 1 OWORD

	int WeightSize = 121;		// 11*11 FP16

	int BlkHoriOffset = h_pos % ThreadsWidthPerTile;		// Block X offset within the tile
	int BlkVertOffset = v_pos % ThreadsHeightPerTile;		// Block Y offset within the tile

	// Skip the block if the output block lends completely outside of actual output data region within a tile
	if (BlkHoriOffset * oBlkWidth4 >= DestWidth)
		return;
	if (BlkVertOffset * oBlkHeight4 >= DestHeight)
		return;

	// 16 pixels step size from one block/thread to the next, in horizontally and vertically direction.
	int OffsetX = BlkHoriOffset * iBlkWidth16;					// Offset from Tile origin in pixel
	int OffsetY = BlkVertOffset * iBlkHeight16;

//	int KernelBiasSize = 122;		// 11*11+1
//	int SrcIdxStart, SrcIdxEnd, GroupID;
//	int SrcDepthPerGroup = SrcDepth / NumGroups;

	// Calcualte DstIdx from v_pos and h_pos. Threads working on one ouptut image share a common DstIdx.
	int DstIdx = oTileHoriCount * (v_pos / ThreadsHeightPerTile) + (h_pos / ThreadsWidthPerTile);
	// DstId is shared among all batches.
	int FeatureOffsetInBatch = DstIdx % DestDepth;	// Feature offset wihtin a batch. Wrap around for multiple batches.

	for (int SrcIdx = 0; SrcIdx < SrcDepth; SrcIdx++) {

		// Offset into weight the matrix of DstIdx X SrcIdx, in which the features are in laid out in NCHW sequence.
		int WeightOffset = (FeatureOffsetInBatch * SrcDepth + SrcIdx) * WeightSize;
		read(DWALIGNED(wIndex), WeightOffset * sizeof(half), vWeight.select<64,1>(0));
		read(DWALIGNED(wIndex), (WeightOffset + 64) * sizeof(half), vWeight.select<64,1>(64));

		// Read a input block, 26x32 half
		int iTileX = SrcIdx%iTileHoriCount;
		int iTileY = SrcIdx/iTileHoriCount;
		int iBaseX = iTileWidth * iTileX;		// Tile base x in float
		int iBaseY = iTileHeight * iTileY;		// Tile base y 

		read(iIndex, (iBaseX + OffsetX) * sizeof(half),			iBaseY + OffsetY,	 fSrc.select<8,1,16,1>(0,0));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(half),	iBaseY + OffsetY,	 fSrc.select<8,1,16,1>(0,16));

		read(iIndex, (iBaseX + OffsetX) * sizeof(half),			iBaseY + OffsetY + 8, fSrc.select<8,1,16,1>(8,0));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(half),	iBaseY + OffsetY + 8, fSrc.select<8,1,16,1>(8,16));

		read(iIndex, (iBaseX + OffsetX) * sizeof(half),			iBaseY + OffsetY + 16, fSrc.select<8,1,16,1>(16,0));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(half),	iBaseY + OffsetY + 16, fSrc.select<8,1,16,1>(16,16));

		read(iIndex, (iBaseX + OffsetX) * sizeof(half),			iBaseY + OffsetY + 24, fSrc.select<2,1,16,1>(24,0));
		read(iIndex, (iBaseX + OffsetX + 16) * sizeof(half),	iBaseY + OffsetY + 24, fSrc.select<2,1,16,1>(24,16));

		// Align 8-11 weights mapped to 1-4 for simd4 ops.  7 is copied to 0 but not used.
		matrix<half, 11, 16> mWeight_11_16(0);
		mWeight_11_16.select<11,1,8,1>(0,0) = mWeight.select<11,1,8,1>(0,0);
		mWeight_11_16.select<11,1,4,1>(0,7) = mWeight.select<11,1,4,1>(0,7);
		// 11x11 convol on 4x4 pixels
		matrix<half, 11, 16> vTemp;
		matrix<half, 4, 32> mSum;

		// Calculate 4x4 output pixels
		#pragma unroll
		for (int r = 0; r < oBlkHeight4; r++) {
			int rr = Stride4 * r;

			#pragma unroll
			for (int c = 0; c < oBlkWidth4; c++) {
				int cc = Stride4 * c;
				
				vTemp.select<11,1,16,1>(0,0) = mWeight_11_16.select<11,1,16,1>(0,0) * fSrc.select<11,1,16,1>(0+rr,cc);
				vTemp.select<5,2,16,1>(2,0) += vTemp.select<5,2,16,1>(1,0);//1->2 3->4 5->6 7->8 9->10
				vTemp.select<3,2,16,1>(0,0) += vTemp.select<3,2,16,1>(6,0);//6->0 8->2 10->4
				vTemp.select<1,1,16,1>(0,0) += vTemp.select<1,1,16,1>(2,0);//2->0
    			vTemp.select<1,1,16,1>(0,0) += vTemp.select<1,1,16,1>(4,0);//4->0
				mSum.select<1,1,8,1>(r, 8*c) = vTemp.select<1,1,8,1>(0,0) + vTemp.select<1,1,8,1>(0,8);
			}
		}

		// Final sum 
		mSum.select<4,1,16,1>(0,0) = mSum.select<4,1,16,2>(0,0) + mSum.select<4,1,16,2>(0,1);
		mSum.select<4,1,8,1>(0,0) = mSum.select<4,1,8,2>(0,0) + mSum.select<4,1,8,2>(0,1);
		fDst += mSum.select<4,1,4,2>(0,0) + mSum.select<4,1,4,2>(0,1);
	} // for

	//int BiasOffset = FeatureOffsetInBatch;
	read(DWALIGNED(bIndex), FeatureOffsetInBatch * sizeof(half), vBias);  // Read 1 OWORD, only 1st half is used.
	fDst += vBias[0];	// Add bias

	if (EnableReLU)
	    fDst.merge((half)0.0f, fDst < (half)0.0f);		// ReLU

	// Find last block for partial block write.  if 0, no partial block.
	int LastBlkWidth = DestWidth % oBlkWidth4;		// True last block width 
	int LastBlkHeight = DestHeight % oBlkHeight4;	// True last block height

	// Zero out area outside of image for partial write
	if (LastBlkWidth > 0)
		if ((BlkHoriOffset+1) * oBlkWidth4 > DestWidth) {
			for (int c = LastBlkWidth; c < oBlkWidth4; c++)
				fDst.column(c) = (half)0.0f;
		}
	if (LastBlkHeight > 0)
		if ((BlkVertOffset+1) * oBlkHeight4 > DestHeight) {
			for (int r = LastBlkHeight; r < oBlkHeight4; r++)
				fDst.row(r) = (half)0.0f;
		}

	int oTileX = DstIdx%oTileHoriCount;		// Output image location based on image id 
	int oTileY = DstIdx/oTileHoriCount;
	int oBaseX = oTileWidth * oTileX;		// Image origin in float
	int oBaseY = oTileHeight * oTileY;
	OffsetX = (h_pos%ThreadsWidthPerTile) * oBlkWidth4;		// Offset from Tile origin
	OffsetY = (v_pos%ThreadsHeightPerTile) * oBlkHeight4;

	write(oIndex, (oBaseX + OffsetX + oBorder) * sizeof(half), (oBaseY + OffsetY + oBorder), fDst);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void ReLU_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, int InputMemType, int OutputMemType, int ImagesPerRow)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<half, 16, 16> fBlk;
	vector_ref<half, 256> fVec = fBlk.format<half>();

	int BlkWidth = 16;										// Src block width
	int BlkHeight = 16;										// Src block height
	int nX = BlkWidth * h_pos;								// X coordinate based on frame origin
	int nY = BlkHeight * v_pos;

	int ImageOffset = ImagesPerRow * v_pos + h_pos;
	int PixOffset = ImageOffset * BlkWidth * BlkHeight;;

	if (InputMemType == MEMORY_2D) {
		// Read 16x16 HF
		read(iIndex, nX * sizeof(half), nY, fBlk.select<8,1,16,1>(0,0));
		read(iIndex, nX * sizeof(half), nY+8, fBlk.select<8,1,16,1>(8,0));
	} else {
		// Read 256 HF
		read(iIndex, PixOffset * sizeof(half), fVec.select<64,1>(0));
		read(iIndex, (PixOffset + 64) * sizeof(half), fVec.select<64,1>(64));
		read(iIndex, (PixOffset + 128) * sizeof(half), fVec.select<64,1>(128));
		read(iIndex, (PixOffset + 192) * sizeof(half), fVec.select<64,1>(192));
	}

	fBlk.merge((half)0.0f, fBlk < (half)0.0f);		// ReLU

	if (OutputMemType == MEMORY_2D) {
		write(oIndex, nX * sizeof(half), nY, fBlk.select<8,1,16,1>(0,0));
		write(oIndex, nX * sizeof(half), nY+8, fBlk.select<8,1,16,1>(8,0));
	} else {
		write(oIndex, PixOffset * sizeof(half), fVec.select<64,1>(0));
		write(oIndex, (PixOffset + 64) * sizeof(half), fVec.select<64,1>(64));
		write(oIndex, (PixOffset + 128) * sizeof(half), fVec.select<64,1>(128));
		write(oIndex, (PixOffset + 192) * sizeof(half), fVec.select<64,1>(192));
	} 
}


//////////////////////////////////////////////////////////////////////////
// Convert 2D surface to 1D buffer.  Tile padding is removed in 1D output.
// For convert surface, it supports image size <= 8x8.  E.g. 6x6 and 7x7.
// To do: have one thread processing one tile and read input 2D data linearly within the tile.  Works on larger image.
extern "C" _GENX_MAIN_ void SurfConv_HF(SurfaceIndex iIndex, SurfaceIndex oIndex, int ImagesPerRow, int DstWidth)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<half, 8, 8> fBlk;
	vector_ref<half, 64> fVec = fBlk.format<half>();
	vector<half, 49> DstBuffer;								// No tile padding is written to the ouptut buffer.
	vector<half, 8> dst;
	vector<uint, 8> vOffset;

	int BlkWidth = 8;										// Src block width
	int BlkHeight = 8;										// Src block height
	int nX = BlkWidth * h_pos;								// X coordinate based on frame origin
	int nY = BlkHeight * v_pos;

	// Read 8x8
	read(iIndex, nX * sizeof(half), nY, fBlk);

	int ImageOffset = ImagesPerRow * v_pos + h_pos;
	int PixOffset = ImageOffset * DstWidth;

	// The code below can be optimized after scattered write for FP16 is fixed.

	// Currently suports 6x6 and 7x7 floats output
	if (DstWidth == 36) {
		#pragma unroll
		for (int j = 0; j < 6; j++) {
			int k = 6 * j;
			#pragma unroll
			for (int i = 0; i < 6; i++) {
				int offset = k + i;
				dst = fBlk[j][i];											
				vOffset = PixOffset + offset;
				write(oIndex, 0, vOffset, dst);
			}
		}

	} else if (DstWidth == 49) {
		#pragma unroll
		for (int j = 0; j < 7; j++) {
			int k = 7 * j;
			#pragma unroll
			for (int i = 0; i < 7; i++) {
				int offset = k + i;
				dst = fBlk[j][i];											
				vOffset = PixOffset + offset;
				write(oIndex, 0, vOffset, dst);
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef LOW_PRESION
extern "C" _GENX_MAIN_ void
LRN_HF(SurfaceIndex INBUF,						//input, source image
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
		matrix<half, 16, 16> prev2, prev1, cur, next1, next2, dst;
		matrix<float, 16, 16>temp;

		srcXY(0) = get_thread_origin_x();
		srcXY(1) = get_thread_origin_y();

		/* from 2nd image */
		int imageID = (srcXY(1)/Height)*(Blocks/Width)+srcXY(0)/Width;

		srcXY(0) = srcXY(1)/Height*Blocks+srcXY(0);
		srcXY(1) = srcXY(1)%Height;

		int cur_y = srcXY(0)/Blocks*Height + srcXY(1);
		int cur_x = srcXY(0)%Blocks;

		read(INBUF, cur_x*16*sizeof(half), cur_y*16, cur.select<8,1,16,1>(0,0));
		read(INBUF, cur_x*16*sizeof(half), cur_y*16 + 8, cur.select<8,1,16,1>(8,0));

		dst = cur*cur;

		int start = (imageID/Images)*Images;
		int end = (imageID/Images+1)*Images;

		/* todo, SIMD4 for neighbor coordinate calculation */
		if(imageID-2 >= start)
		{
			int prev2_y = (srcXY(0)-2*Width)/Blocks*Height+srcXY(1);
			int prev2_x = (srcXY(0)-2*Width)%Blocks;

			read(INBUF, prev2_x*16*sizeof(half), prev2_y*16, prev2.select<8,1,16,1>(0,0));
			read(INBUF, prev2_x*16*sizeof(half), prev2_y*16 + 8, prev2.select<8,1,16,1>(8,0));

			dst += prev2*prev2;
		}

		if(imageID-1 >= start)
		{
			int prev1_y = (srcXY(0)-Width)/Blocks*Height+srcXY(1);
			int prev1_x = (srcXY(0)-Width)%Blocks;
			
			read(INBUF, prev1_x*16*sizeof(half), prev1_y*16, prev1.select<8,1,16,1>(0,0));
			read(INBUF, prev1_x*16*sizeof(half), prev1_y*16 + 8, prev1.select<8,1,16,1>(8,0));

			dst += prev1*prev1;
		}

		if(imageID+1 < end)
		{
			int next1_y = (srcXY(0)+Width)/Blocks*Height+srcXY(1);
			int next1_x = (srcXY(0)+Width)%Blocks;

			read(INBUF, next1_x*16*sizeof(half), next1_y*16, next1.select<8,1,16,1>(0,0));
			read(INBUF, next1_x*16*sizeof(half), next1_y*16 + 8, next1.select<8,1,16,1>(8,0));

			dst += next1*next1;
		}

		if(imageID+2 < end)
		{
			int next2_y = (srcXY(0)+2*Width)/Blocks*Height+srcXY(1);
			int next2_x = (srcXY(0)+2*Width)%Blocks;

			read(INBUF, next2_x*16*sizeof(half), next2_y*16, next2.select<8,1,16,1>(0,0));
			read(INBUF, next2_x*16*sizeof(half), next2_y*16 + 8, next2.select<8,1,16,1>(8,0));

			dst += next2*next2;
		}

		temp = K + Alpha * dst;
		temp = cm_pow(temp, -Beta);
		dst = cur*temp;

		write(OUTBUF, cur_x*16*sizeof(half), cur_y*16, dst.select<8,1,16,1>(0,0));
		write(OUTBUF, cur_x*16*sizeof(half), cur_y*16 + 8, dst.select<8,1,16,1>(8,0));
	}
	else
	{
		vector<int, 2> srcXY;
		matrix<float, 16, 16>temp;

		srcXY(0) = get_thread_origin_x();
		srcXY(1) = get_thread_origin_y();

		srcXY <<= 4;

		//kernel input, 16*12 temp block, 32bpp format, template size of horizontal/vertical is 5
		matrix<half, 20, 32> srcPic = 0.0f;

		//kernel output, each thread handle 8*8 temp block, 32bpp format
		matrix<half, 16, 16> dstPic = 0.0f;

		read(INBUF, (srcXY(0) - 2)  * sizeof(half), srcXY(1) - 2,  srcPic.select<8, 1, 16, 1>(0, 0));
		read(INBUF, (srcXY(0) + 14)  * sizeof(half), srcXY(1) - 2,  srcPic.select<8, 1, 16, 1>(0, 16));

		read(INBUF, (srcXY(0) - 2)  * sizeof(half), srcXY(1) + 6,  srcPic.select<8, 1, 16, 1>(8, 0));
		read(INBUF, (srcXY(0) + 14)  * sizeof(half), srcXY(1) + 6,  srcPic.select<8, 1, 16, 1>(8, 16));

		read(INBUF, (srcXY(0) - 2)  * sizeof(half), srcXY(1) + 14,  srcPic.select<4, 1, 16, 1>(16, 0));
		read(INBUF, (srcXY(0) + 14)  * sizeof(half), srcXY(1) + 14,  srcPic.select<4, 1, 16, 1>(16, 16));

		matrix<half, 20, 16> sumPic = 0.0f;

	#pragma unroll
		for(int i = 0; i < 20; i ++)
	#pragma unroll
			for(int j = 0; j < 4; j ++)
				sumPic.row(i) += srcPic.row(i).select<16, 1>(j) * srcPic.row(i).select<16, 1>(j);

	#pragma unroll
		for(int i = 0; i < 5;  i ++)
			dstPic.row(0) += sumPic.row(i);

	#pragma unroll
		for(int i = 1; i < 16;  i ++)
			dstPic.row(i) = dstPic.row(i-1) + sumPic.row(i+4) - sumPic.row(i-1);

		temp = K + Alpha * dstPic;
		temp = cm_pow(temp, -Beta);
		dstPic = srcPic.select<16, 1, 16, 1>(2, 2)*temp;

		write(OUTBUF, srcXY(0)*sizeof(half), srcXY(1), dstPic.select<8, 1, 16, 1>(0,0));
		write(OUTBUF, srcXY(0)*sizeof(half), srcXY(1) + 8, dstPic.select<8, 1, 16, 1>(8,0));
	}
}

#else
extern "C" _GENX_MAIN_ void
LRN_HF(SurfaceIndex INBUF,						//input, source image
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
		matrix<half, 16, 16> prev2, prev1, cur, next1, next2, dst;
		matrix<float, 16, 16>temp;
		matrix<float, 16, 16>dst_float;

		srcXY(0) = get_thread_origin_x();
		srcXY(1) = get_thread_origin_y();

		/* from 2nd image */
		int imageID = (srcXY(1)/Height)*(Blocks/Width)+srcXY(0)/Width;

		srcXY(0) = srcXY(1)/Height*Blocks+srcXY(0);
		srcXY(1) = srcXY(1)%Height;

		int cur_y = srcXY(0)/Blocks*Height + srcXY(1);
		int cur_x = srcXY(0)%Blocks;

		read(INBUF, cur_x*16*sizeof(half), cur_y*16, cur.select<8,1,16,1>(0,0));
		read(INBUF, cur_x*16*sizeof(half), cur_y*16 + 8, cur.select<8,1,16,1>(8,0));

		temp = cur;
		dst_float = temp*temp;

		int start = (imageID/Images)*Images;
		int end = (imageID/Images+1)*Images;

		/* todo, SIMD4 for neighbor coordinate calculation */
		if(imageID-2 >= start)
		{
			int prev2_y = (srcXY(0)-2*Width)/Blocks*Height+srcXY(1);
			int prev2_x = (srcXY(0)-2*Width)%Blocks;

			read(INBUF, prev2_x*16*sizeof(half), prev2_y*16, prev2.select<8,1,16,1>(0,0));
			read(INBUF, prev2_x*16*sizeof(half), prev2_y*16 + 8, prev2.select<8,1,16,1>(8,0));

			temp = prev2;
			dst_float += temp*temp;
		}

		if(imageID-1 >= start)
		{
			int prev1_y = (srcXY(0)-Width)/Blocks*Height+srcXY(1);
			int prev1_x = (srcXY(0)-Width)%Blocks;
			
			read(INBUF, prev1_x*16*sizeof(half), prev1_y*16, prev1.select<8,1,16,1>(0,0));
			read(INBUF, prev1_x*16*sizeof(half), prev1_y*16 + 8, prev1.select<8,1,16,1>(8,0));
			
			temp = prev1;
			dst_float += temp*temp;
		}

		if(imageID+1 < end)
		{
			int next1_y = (srcXY(0)+Width)/Blocks*Height+srcXY(1);
			int next1_x = (srcXY(0)+Width)%Blocks;

			read(INBUF, next1_x*16*sizeof(half), next1_y*16, next1.select<8,1,16,1>(0,0));
			read(INBUF, next1_x*16*sizeof(half), next1_y*16 + 8, next1.select<8,1,16,1>(8,0));
			
			temp = next1;
			dst_float += temp*temp;
		}

		if(imageID+2 < end)
		{
			int next2_y = (srcXY(0)+2*Width)/Blocks*Height+srcXY(1);
			int next2_x = (srcXY(0)+2*Width)%Blocks;

			read(INBUF, next2_x*16*sizeof(half), next2_y*16, next2.select<8,1,16,1>(0,0));
			read(INBUF, next2_x*16*sizeof(half), next2_y*16 + 8, next2.select<8,1,16,1>(8,0));

			temp = next2;
			dst_float += temp*temp;
		}

		temp = K + Alpha * dst_float;
		temp = cm_pow(temp, -Beta);
		dst = cur*temp;

		write(OUTBUF, cur_x*16*sizeof(half), cur_y*16, dst.select<8,1,16,1>(0,0));
		write(OUTBUF, cur_x*16*sizeof(half), cur_y*16 + 8, dst.select<8,1,16,1>(8,0));
	}
	else
	{
		vector<int, 2> srcXY;
		matrix<float, 16, 16>temp;
        vector<float, 16> temp1;
		srcXY(0) = get_thread_origin_x();
		srcXY(1) = get_thread_origin_y();

		srcXY <<= 4;

		//kernel input, 16*12 temp block, 32bpp format, template size of horizontal/vertical is 5
		matrix<half, 20, 32> srcPic = 0.0f;

		//kernel output, each thread handle 8*8 temp block, 32bpp format
		matrix<half, 16, 16> dstPic = 0.0f;

		read(INBUF, (srcXY(0) - 2)  * sizeof(half), srcXY(1) - 2,  srcPic.select<8, 1, 16, 1>(0, 0));
		read(INBUF, (srcXY(0) + 14)  * sizeof(half), srcXY(1) - 2,  srcPic.select<8, 1, 16, 1>(0, 16));

		read(INBUF, (srcXY(0) - 2)  * sizeof(half), srcXY(1) + 6,  srcPic.select<8, 1, 16, 1>(8, 0));
		read(INBUF, (srcXY(0) + 14)  * sizeof(half), srcXY(1) + 6,  srcPic.select<8, 1, 16, 1>(8, 16));

		read(INBUF, (srcXY(0) - 2)  * sizeof(half), srcXY(1) + 14,  srcPic.select<4, 1, 16, 1>(16, 0));
		read(INBUF, (srcXY(0) + 14)  * sizeof(half), srcXY(1) + 14,  srcPic.select<4, 1, 16, 1>(16, 16));

		matrix<float, 20, 16> sumPic = 0.0f;

	#pragma unroll
		for(int i = 0; i < 20; i ++)
	#pragma unroll
			for(int j = 0; j < 4; j ++) {
				temp1 = srcPic.row(i).select<16, 1>(j);
				sumPic.row(i) += temp1 * temp1;
			}

	#pragma unroll
		for(int i = 0; i < 5;  i ++)
			dstPic.row(0) += sumPic.row(i);

	#pragma unroll
		for(int i = 1; i < 16;  i ++)
			dstPic.row(i) = dstPic.row(i-1) + sumPic.row(i+4) - sumPic.row(i-1);

		temp = K + Alpha * dstPic;
		temp = cm_pow(temp, -Beta);
		dstPic = srcPic.select<16, 1, 16, 1>(2, 2)*temp;

		write(OUTBUF, srcXY(0)*sizeof(half), srcXY(1), dstPic.select<8, 1, 16, 1>(0,0));
		write(OUTBUF, srcXY(0)*sizeof(half), srcXY(1) + 8, dstPic.select<8, 1, 16, 1>(8,0));
	}
}

#endif

/*
////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
MaxPooling2D_HF(SurfaceIndex INBUF,
					SurfaceIndex OUTBUF,
					vector<int, 2> ThreadDim,			//input, thread width/height for one image
					vector<int, 2> ImageDim8,			//input, width/height for one image 8 alignment
					vector<int, 2> ImageDim,			//input, width/height for one image
					vector<int, 2> ImageBorder,			//input, left/top border for one image
					int PoolSize)						//input, pool size
{
	vector<int, 2> srcXY;

	srcXY(0) = get_thread_origin_x();
	srcXY(1) = get_thread_origin_y();

	srcXY <<= 4;

	srcXY += ImageBorder;

	//kernel output, each thread handle 4*4 block, 32bpp format
	matrix<half, 8, 8> dstPic = 0.0f;
//	matrix<float, 8, 8> dstPic_fp32;

	if(PoolSize == POOLSIZE_3x3)
	{

		//kernel input, 16*9 temp block, 32bpp format, template size of horizontal/vertical is 3, start from (1, 1)
		matrix<half, 18, 32> srcPic = 0.0f;

		read(INBUF, srcXY(0)*sizeof(half), srcXY(1),  srcPic.select<8, 1,16, 1>(0, 0));
		read(INBUF, (srcXY(0)+16)*sizeof(half), srcXY(1),  srcPic.select<8, 1, 16, 1>(0, 16));
		read(INBUF, srcXY(0)*sizeof(half), srcXY(1)+8,  srcPic.select<8, 1,16, 1>(8, 0));
		read(INBUF, (srcXY(0)+16)*sizeof(half), srcXY(1)+ 8,  srcPic.select<8, 1, 16, 1>(8, 16));
		read(INBUF, srcXY(0)*sizeof(half), srcXY(1)+16,  srcPic.select<2, 1, 16, 1>(16, 0));
		read(INBUF, (srcXY(0)+16)*sizeof(half), srcXY(1)+16,  srcPic.select<2, 1, 16, 1>(16, 16));

	#pragma unroll
		for(int i = 0; i < 3; i ++)
	#pragma unroll
			for(int j = 0; j < 3; j ++)
				dstPic = cm_max<half>(srcPic.select<8, 2, 8, 2>(i, j), dstPic);
	}
	else if(PoolSize == POOLSIZE_2x2)
	{
		//kernel input, 16*8 temp block, 32bpp format, template size of horizontal/vertical is 3, start from (1, 1)
		matrix<half, 16, 16> srcPic = 0.0f;
//		matrix<float, 16, 16> srcPic_fp32;

		read(INBUF, srcXY(0)*sizeof(half), srcXY(1),    srcPic.select<8, 1,16, 1>(0, 0));
		read(INBUF, srcXY(0)*sizeof(half), srcXY(1)+8,  srcPic.select<8, 1,16, 1>(8, 0));

//		srcPic_fp32 = srcPic;

		#pragma unroll
		for(int i = 0; i < 2; i ++)
			#pragma unroll
			for(int j = 0; j < 2; j ++)
				dstPic = cm_max<half>(srcPic.select<8, 2, 8, 2>(i, j), dstPic);

//		dstPic_fp32 = dstPic;
	}

	srcXY(0) = get_thread_origin_x();
	srcXY(1) = get_thread_origin_y();

	srcXY <<= 3;

	srcXY += ImageBorder;

	// out of boundary handling 
	matrix<int, 8, 8> horPos, verPos;
	vector<int, 8> InitPos = InitHorPos_HF;

	#pragma unroll
	for(int i = 0; i < 8; i ++) {
		verPos.row(i) = i;
		horPos.row(i) = InitPos;
	}

	horPos += srcXY(0);
	verPos += srcXY(1);

	vector<int, 2> dstXY;
	vector<int, 2> tmp;
	vector<int, 2> tmp2;

	dstXY(0) = get_thread_origin_x();
	dstXY(1) = get_thread_origin_y();

	// boundry for each image 
	tmp = (dstXY/ThreadDim)*ImageDim8;
	tmp2 = (dstXY%ThreadDim) * 8 + tmp ;
	dstXY = tmp + ImageDim;
	tmp = tmp2 + ImageBorder ;

	dstPic.merge((half)0.0f, horPos>dstXY(0)-1 | verPos>dstXY(1)-1);

//	dstPic_fp32 = dstPic;

	write(OUTBUF, tmp(0)*sizeof(half), tmp(1), dstPic);
}
*/

////////////////////////////////////////////////////////////////////////////////////////////
// Input block size = 16x16, output block size = 8x8
extern "C" _GENX_MAIN_ void
MaxPooling2D_HF(SurfaceIndex INBUF,
					SurfaceIndex OUTBUF,
					vector<int, 2> ThreadsPerTile,		//input, thread width/height per tile
					vector<int, 2> oTileSize,			//input, otile width/height
					vector<int, 2> oDestSize,			//input, dest image width/height
					vector<int, 2> ImageBorder,			//input, left/top border for one image
					int PoolSize)						//input, pool size
{
	vector<int, 2> srcXY;

	srcXY(0) = get_thread_origin_x();
	srcXY(1) = get_thread_origin_y();
	srcXY <<= 4;
//	srcXY[0] += 2*ImageBorder[0];
	srcXY[0] += ImageBorder[0];
	srcXY[1] += ImageBorder[1];

	//kernel output, each thread handle 4*4 block, 32bpp format
	matrix<half, 8, 8> dstPic = (half)0.0f;

	if(PoolSize == POOLSIZE_3x3) {
		//kernel input, 16*9 temp block, 32bpp format, template size of horizontal/vertical is 3, start from (1, 1)
		matrix<half, 18, 32> srcPic = (half)0.0f;

		read(INBUF, srcXY(0)*sizeof(half), srcXY(1),  srcPic.select<8, 1,16, 1>(0, 0));
		read(INBUF, (srcXY(0)+16)*sizeof(half), srcXY(1),  srcPic.select<8, 1, 16, 1>(0, 16));
		read(INBUF, srcXY(0)*sizeof(half), srcXY(1)+8,  srcPic.select<8, 1,16, 1>(8, 0));
		read(INBUF, (srcXY(0)+16)*sizeof(half), srcXY(1)+ 8,  srcPic.select<8, 1, 16, 1>(8, 16));
		read(INBUF, srcXY(0)*sizeof(half), srcXY(1)+16,  srcPic.select<2, 1, 16, 1>(16, 0));
		read(INBUF, (srcXY(0)+16)*sizeof(half), srcXY(1)+16,  srcPic.select<2, 1, 16, 1>(16, 16));

		#pragma unroll
		for(int i = 0; i < 3; i ++)
			#pragma unroll
			for(int j = 0; j < 3; j ++)
				dstPic = cm_max<half>(srcPic.select<8, 2, 8, 2>(i, j), dstPic);

	} else if (PoolSize == POOLSIZE_2x2) {

		//kernel input, 16*8 temp block, 32bpp format, template size of horizontal/vertical is 3, start from (1, 1)
		matrix<half, 16, 16> srcPic = (half)0.0f;

		read(INBUF, srcXY(0)*sizeof(half), srcXY(1),    srcPic.select<8, 1,16, 1>(0, 0));
		read(INBUF, srcXY(0)*sizeof(half), srcXY(1)+8,  srcPic.select<8, 1,16, 1>(8, 0));

		#pragma unroll
		for(int i = 0; i < 2; i ++)
			#pragma unroll
			for(int j = 0; j < 2; j ++)
				dstPic = cm_max<half>(srcPic.select<8, 2, 8, 2>(i, j), dstPic);
	}

	vector<ushort, 2> dstXY;
	dstXY[0] = get_thread_origin_x();
	dstXY[1] = get_thread_origin_y();

	vector<ushort, 2> oBase;
	oBase = (dstXY / ThreadsPerTile) * oTileSize;	// Current tile origin in pix
	oBase += (dstXY % ThreadsPerTile) * 8;			// + oBlock offset in pix

	// Out of boundary handling
	vector<ushort, 8> horPos = InitHorPos8;
	vector<ushort, 8> verPos = horPos;
	horPos += ((8*dstXY[0]) % oTileSize[0]);
	verPos += ((8*dstXY[1]) % oTileSize[1]);
	#pragma unroll
	for(int i = 0; i < 8; i ++) {
		dstPic.row(i).merge((half)0.0f, horPos >= oDestSize[0]);	// Mute right side
		if (verPos[i] >= oDestSize[1])						// Mute bottom 
			dstPic.row(i) = (half)0.0f;
	}

	write(OUTBUF, (oBase[0] + ImageBorder[0]) * sizeof(half), oBase[1] + ImageBorder[1], dstPic.select<8,1,8,1>(0,0));
}

////////////////////////////////////////////////////////////////////////////////////////////
// Input block size = 16x16, output block size = 8x8
extern "C" _GENX_MAIN_ void
AvgPooling2D_HF(SurfaceIndex INBUF,
				SurfaceIndex OUTBUF,
				int PoolSize,
				int iTileHoriCount)						//input, pool size
{

}

////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
MaxPooling1D_HF(SurfaceIndex INBUF,
		 SurfaceIndex OUTBUF,
		 vector<int, 2> ImageBorder,		//input, left/top border for one image
		 int Images,						//input, image number per row
		 int Input2D,						//input, 2D or 1D input
		 int PoolSize)						//input, pool size
{
	int off = Images*get_thread_origin_y()+get_thread_origin_x()*2;
	
	if(PoolSize == POOLSIZE_3x3)
	{
		//kernel input, 16*13 temp block, 32bpp format, template size of horizontal/vertical is 3, start from (1, 1)
		matrix<half, 13, 32> srcPic = 0.0f;

		//kernel output, each thread handle 6*6 block, 32bpp format
		vector<half, 72> dstPic = 0.0f;

		matrix_ref<half, 12, 6> mDst = dstPic.format<half, 12, 6>();

		if( Input2D )
		{
			vector<int, 2> srcXY;

			srcXY(0) = get_thread_origin_x();
			srcXY(1) = get_thread_origin_y();

			srcXY(0) <<= 5;
			srcXY(1) <<= 4;

			srcXY += ImageBorder;

			read(INBUF, srcXY(0)*sizeof(half), srcXY(1),  srcPic.select<8, 1, 16, 1>(0, 0));
			read(INBUF, (srcXY(0) + 16)*sizeof(half), srcXY(1),  srcPic.select<8, 1, 16, 1>(0, 16));

			read(INBUF, srcXY(0)*sizeof(half), srcXY(1)+8,  srcPic.select<5, 1, 16, 1>(8, 0));	
			read(INBUF, (srcXY(0) + 16)*sizeof(half), srcXY(1)+8,  srcPic.select<5, 1, 16, 1>(8, 16));
		}
		else
		{
			//1D input, each image is 16*16
			int xOff = off*16*16;

		#pragma unroll
			for(int i = 0; i < 6; i ++)
				read(DWALIGNED(INBUF), (xOff+i*2*32)*sizeof(half), srcPic.select<2, 1, 32, 1>(i*2, 0).format<half>());

			read(DWALIGNED(INBUF), (xOff+6*2*32)*sizeof(half), srcPic.select<1, 1, 32, 1>(12, 0).format<half>());
		}

	#pragma unroll
		for(int i = 0; i < 3; i ++)
	#pragma unroll
			for(int j = 0; j < 3; j ++) {
				mDst.select<6, 1, 6, 1>(0, 0) = cm_max<half>(srcPic.select<6, 2, 6, 2>(i, j), mDst.select<6, 1, 6, 1>(0, 0));
				mDst.select<6, 1, 6, 1>(6, 0) = cm_max<half>(srcPic.select<6, 2, 6, 2>(i, j+16), mDst.select<6, 1, 6, 1>(6, 0));
			}

		/* 6*6 features, 1D buffer */
		write(OUTBUF, 36*off*sizeof(half), dstPic.select<64, 1>(0));
		write(OUTBUF, 36*off*sizeof(half)+64*sizeof(half), dstPic.select<8, 1>(64));
	}
	else if(PoolSize == POOLSIZE_2x2)
	{
		//kernel input, 16*14 temp block, 32bpp format, template size of horizontal/vertical is 3, start from (1, 1)
		matrix<half, 14, 32> srcPic = 0.0f;

		//kernel output, each thread handle 7*7 block, 32bpp format
		vector<half, 98> dstPic = 0.0f;

		matrix_ref<half, 14, 7> mDst = dstPic.format<half, 14, 7>();

		if( Input2D )
		{
			vector<int, 2> srcXY;

			srcXY(0) = get_thread_origin_x();
			srcXY(1) = get_thread_origin_y();

			srcXY(0) <<= 5;
			srcXY(1) <<= 4;

			srcXY += ImageBorder;

			read(INBUF, srcXY(0)*sizeof(half), srcXY(1),  srcPic.select<8, 1, 16, 1>(0, 0));
			read(INBUF, (srcXY(0) + 16)*sizeof(half), srcXY(1),  srcPic.select<8, 1, 16, 1>(0, 16));

			read(INBUF, srcXY(0)*sizeof(half), srcXY(1)+8,  srcPic.select<6, 1, 16, 1>(8, 0));	
			read(INBUF, (srcXY(0) + 16)*sizeof(half), srcXY(1)+8,  srcPic.select<6, 1, 16, 1>(8, 16));
		}
		else
		{
			//1D input, each image is 16*16
			int xOff = off*16*16;

		#pragma unroll
			for(int i = 0; i < 7; i ++)
				read(DWALIGNED(INBUF), (xOff+i*2*32)*sizeof(half), srcPic.select<2, 1, 32, 1>(i*2, 0).format<half>());
		}

	#pragma unroll
		for(int i = 0; i < 2; i ++)
	#pragma unroll
			for(int j = 0; j < 2; j ++) {
				mDst.select<7, 1, 7, 1>(0, 0) = cm_max<half>(srcPic.select<7, 2, 7, 2>(i, j), mDst.select<7, 1, 7, 1>(0, 0));
				mDst.select<7, 1, 7, 1>(7, 0) = cm_max<half>(srcPic.select<7, 2, 7, 2>(i, j+16), mDst.select<7, 1, 7, 1>(7, 0));
			}

		/* 7*7 features, 1D buffer */
		write(OUTBUF, 49*off*sizeof(half), dstPic.select<64, 1>(0));
		write(OUTBUF, 49*off*sizeof(half)+64*sizeof(half), dstPic.select<32, 1>(64));

		vector<half, 16> dst = dstPic.select<2, 1>(96).replicate<8>();

		vector<uint, 8> offset = 49*off*sizeof(half)+96*sizeof(half);
		write(OUTBUF, 0, offset, dst.format<float>());
	}
	else
		return;
}


/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FCWT_2D				// Use Surface2D for FC weight data
extern "C" _GENX_MAIN_ void
FullyConnection_NPatch_HF(SurfaceIndex INBUF,	//input, 
				SurfaceIndex OUTBUF,	//output,
				SurfaceIndex COEFF,		//input, 
				SurfaceIndex BIAS,		//input, last layer image size/64
				int OneSrcBatchSize,	// Src size per batch
				int OneDestBatchSize,	// Dest size per batch
				uint HoriThreads,		// Thread space width
				int Batches,			// # of batches
				int ReLU)				//input, ReLU control flag
{

	vector<half, 16> feature;				// 2 grf
	matrix<half, 16, 16> coeff;			// 16 grf
	vector<half, 16> bias;					// 2 grf
	vector<half, 16> dst;					// 2 grf, 32 output features
	matrix<half, 100, 16> sum = 0.0f;	// 64 grf

	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();
	h_pos = v_pos * HoriThreads + h_pos;
	//v_pos = 1;

	int iBaseX = h_pos * 16;						// Offset in the cureent Batch for weight and bias
	int oBaseX = h_pos * 16;						// Offset of the cureent Batch for output

	read(BIAS, iBaseX * sizeof(half), bias.select<16,1>(0));

	int iter = OneSrcBatchSize/16;		// each loop process 16 inputs for the same batch.

	// through all inputs
	for(int j = 0; j < iter; j++) {
		// Read 16x32 float
		read(COEFF, iBaseX * sizeof(half), 16 * j, coeff.select<8,1,16,1>(0,0));	// 64 float 
		read(COEFF, iBaseX * sizeof(half), 16 * j + 8, coeff.select<8,1,16,1>(8,0));	// 64 float 

		// through batches
		for (int b = 0; b < Batches; b++) {
			read(INBUF, (OneSrcBatchSize*b + _ROWS*j) * sizeof(half), feature);			// 8 float

			#pragma unroll
			for (int i = 0; i < 16; i++) 
				sum.row(b) += feature[i] * coeff.row(i);
		}
	}

	for (int b = 0; b < Batches; b++) {
		dst = sum.row(b) + bias;
		if (ReLU)
			dst.merge((half)0.0f, dst < (half)0.0f);
		write(OUTBUF, (oBaseX + OneDestBatchSize*b) * sizeof(half), dst);
	}
}
#else			// Use Buffer for FC weight data
/////////////////////////////////////////////////////////////////////////////
// weight in HF16 format in 1D Surface
extern "C" _GENX_MAIN_ void
FullyConnection_NPatch_HF(SurfaceIndex INBUF,	//input, 
				SurfaceIndex OUTBUF,	//output,
				SurfaceIndex COEFF,		//input, 
				SurfaceIndex BIAS,		//input, last layer image size/64
				int OneSrcBatchSize,	// Src size per batch
				int OneDestBatchSize,	// Dest size per batch
				uint HoriThreads,		// Thread space width
				int Batches,			// # of batches
				int ReLU)				//input, ReLU control flag
{

	vector<half, 16> feature;				// 2 grf
	matrix<half, 16, 16> coeff;			// 16 grf
	vector<half, 16> bias;					// 2 grf
	vector<half, 16> dst;					// 2 grf, 32 output features
	matrix<half, 100, 16> sum = 0.0f;	// 64 grf
	int nWeightOffset;							// Weight data offset

	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();
	h_pos = v_pos * HoriThreads + h_pos;
	//v_pos = 1;

	int iBaseX = h_pos * 16;						// Offset in the cureent Batch for weight and bias
	int oBaseX = h_pos * 16;						// Offset of the cureent Batch for output

	read(BIAS, iBaseX * sizeof(half), bias.select<16,1>(0));

	nWeightOffset = OneSrcBatchSize * iBaseX * sizeof(half);
	int iter = OneSrcBatchSize/16;		// each loop process 16 inputs for the same batch.

	// through all inputs
	for(int j = 0; j < iter; j++) {
		// Read 16x32 float
		read(COEFF, nWeightOffset, coeff.format<half>().select<64,1>(0));
		read(COEFF, nWeightOffset+128, coeff.format<half>().select<64,1>(64));
		read(COEFF, nWeightOffset+256, coeff.format<half>().select<64,1>(128));
		read(COEFF, nWeightOffset+384, coeff.format<half>().select<64,1>(192));
//		read(COEFF, iBaseX * sizeof(half), 16 * j, coeff.select<8,1,16,1>(0,0));	// 64 float 
//		read(COEFF, iBaseX * sizeof(half), 16 * j + 8, coeff.select<8,1,16,1>(8,0));	// 64 float 

		// through batches
		for (int b = 0; b < Batches; b++) {
			read(INBUF, (OneSrcBatchSize*b + iBlkHeight16*j) * sizeof(half), feature);			// 8 float

			#pragma unroll
			for (int i = 0; i < 16; i++) 
				sum.row(b) += feature[i] * coeff.row(i);
		}
		nWeightOffset += 16 * 16 * sizeof(half);
	}

	for (int b = 0; b < Batches; b++) {
		dst = sum.row(b) + bias;
		if (ReLU)
			dst.merge((half)0.0f, dst < (half)0.0f);
		write(OUTBUF, (oBaseX + OneDestBatchSize*b) * sizeof(half), dst);
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Weight in quantized Int8 format in 1D Surface, Half-floating data
extern "C" _GENX_MAIN_ void
FullyConnection_NPatch_HF_QWT(SurfaceIndex INBUF,	//input, 
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

	vector<half, 16> feature;				// 2 grf
	matrix<half, 16, 16> coeff;			// 16 grf
	vector<half, 16> bias;					// 2 grf
	vector<half, 16> dst;					// 2 grf, 32 output features
	matrix<half, 100, 16> sum = 0.0f;	// 64 grf
    vector<char, 16*16> WeightData;		// Weight data
	int nWeightOffset;							// Weight data offset

	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();
	h_pos = v_pos * HoriThreads + h_pos;
	//v_pos = 1;

	int iBaseX = h_pos * 16;						// Offset in the cureent Batch for weight and bias
	int oBaseX = h_pos * 16;						// Offset of the cureent Batch for output

	read(BIAS, iBaseX * sizeof(half), bias.select<16,1>(0));

	nWeightOffset = OneSrcBatchSize * iBaseX;

	int iter = OneSrcBatchSize/16;		// each loop process 16 inputs for the same batch.

	// through all inputs
	for(int j = 0; j < iter; j++) {
		// Read 256 char
		read(COEFF, nWeightOffset, WeightData.select<128,1>(0));
		read(COEFF, nWeightOffset+128, WeightData.select<128,1>(128));
		coeff.format<half>() = fA * WeightData + fB;
//		read(COEFF, iBaseX * sizeof(half), 16 * j, coeff.select<8,1,16,1>(0,0));	// 64 float 
//		read(COEFF, iBaseX * sizeof(half), 16 * j + 8, coeff.select<8,1,16,1>(8,0));	// 64 float 

		// through batches
		for (int b = 0; b < Batches; b++) {
			read(INBUF, (OneSrcBatchSize*b + iBlkHeight16*j) * sizeof(half), feature);			// 8 float

			#pragma unroll
			for (int i = 0; i < 16; i++) 
				sum.row(b) += feature[i] * coeff.row(i);
		}
		nWeightOffset += 16 * 16;
	}

	for (int b = 0; b < Batches; b++) {
		dst = sum.row(b) + bias;
		if (ReLU)
			dst.merge((half)0.0f, dst < (half)0.0f);
		write(OUTBUF, (oBaseX + OneDestBatchSize*b) * sizeof(half), dst);
	}
}


///////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
Softmax_HF(SurfaceIndex INBUF,		//input, 
		SurfaceIndex OUTBUF,		//output
		int oDepth,
		int FeaturesPerThread)
{
	/* each thread handle 8 neurons*/
	vector<half, 32> feature = 0.0f;
	vector<float, 32> tmp;
	vector<float, 32> sum = 0.0f;
	vector<half, 8> bias;
	vector<float, 8> tmp8;

	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();

	int BatchOffset = v_pos * oDepth;

	// Go through all the features of the last layer images, 0 - 992 
#pragma unroll
	for(int i = 0; i < 31; i ++) {
		read(INBUF, (BatchOffset + 32*i) * sizeof(half), feature);
		tmp = feature;
		sum += cm_pow(2.718f, tmp);
	}

	// and last 8 features 
	vector<half, 8> last8;
	read(INBUF, (BatchOffset + 32*31) * sizeof(half), last8);
	tmp8 = last8;
	
	tmp8 = cm_pow(2.718f, tmp8);

	/* calculate the sum of 32 features */
	vector<float, 16> mid16 = sum.select<16, 1>(0) + sum.select<16, 1>(16);
	vector<float, 8> mid8 = mid16.select<8, 1>(0) + mid16.select<8, 1>(8) + tmp8;
	vector<float, 4> mid4 = mid8.select<4, 1>(0) + mid8.select<4, 1>(4);
	vector<float, 2> mid2 = mid4.select<2, 1>(0) + mid4.select<2, 1>(2);
	float st = mid2(0) + mid2(1);

	int xOff = BatchOffset + FeaturesPerThread * h_pos;
	read(INBUF, xOff * sizeof(half), bias);

    tmp8 = bias;
    tmp8 = cm_pow(2.718f, tmp8);
	bias = tmp8/st;

	write(OUTBUF, xOff * sizeof(half), bias);
}

