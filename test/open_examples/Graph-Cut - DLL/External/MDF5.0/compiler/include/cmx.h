/********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013 Intel Corporation. All Rights Reserved.
File: cmx.h
Description: CM extension library
*********************************************************************************/

//remove false postivie warnings
#pragma once
/*
#ifdef CMRT_EMU
#pragma warning(push)
#pragma warning( disable : 4018 ) 
#include <cm/cm.h>
#pragma warning(pop)
#pragma warning( disable :4068)
#else
#include <cm/cm.h>
#endif
*/
#include <cm/cm.h>

/****************** API defintion ********************************************************/

//plain old good assert, optimized away if not in emulation mode
void cmx_Assert(bool x);

/* Read a pixel block of size HIGHT x WIDTH , from surface, all input are in pixel sizes. HIGHT and WIDTH can be any integer
h_pix_pos - horizontal offset in pixel size
v_pix_pos - vertical offset in pixel size
*/

template<typename T, int HIGHT, int WIDTH>
void cmx_read_block(SurfaceIndex obuf, int h_pix_pos, int v_pix_pos, matrix_ref<T, HIGHT, WIDTH> block);

/* Write a pixel block of size HIGHT x WIDTH from surface, all inputs are in pixel sizes. IGHT and WIDTH can be any integer
h_pix_pos - horizontal offset in pixel size
v_pix_pos - vertical offset in pixel size
*/
template<typename T, int HIGHT, int WIDTH>
void cmx_write_block(SurfaceIndex obuf, int h_pix_pos, int v_pix_pos, matrix_ref<T, HIGHT, WIDTH> block); 

/* Creates a nighborhood matrix of a 2D Kernel, whereby for each pixel in src, nighbor (i,j) is located at row(_2D2_1D<KRNL_SZ>(i,j)) "above it"
useful to ensure register alignment irrespective of pixel offset (since we create copies for every offset),  
src - matrix representing pixel line and rows above / below it that fit into the kernel
dst - output matrix including vectorized kernel nighborhood
*/
template<typename Tsrc, typename Tdst, int KRNL_SZ, int SIMD_SZ>
void cmx_vectorize_2Dkrnl(matrix_ref<Tsrc, KRNL_SZ, SIMD_SZ + KRNL_SZ -1> src, matrix_ref<Tdst, KRNL_SZ * KRNL_SZ, SIMD_SZ> dst);


/* updatea a single "row" into a vectorized kernel segment of KRNL_SZ rows
src - input vector for update
dstRow the row segment to update in dst
*/
template<typename Tsrc, typename Tdst, int KRNL_SZ, int SIMD_SZ>
void cmx_vectorize_2Dkrnlrow(vector_ref<Tsrc, SIMD_SZ + KRNL_SZ -1> src, matrix_ref<Tdst, KRNL_SZ * KRNL_SZ, SIMD_SZ> dst, int dstRow);


/* translates a matrix representation into vector represnetaiton, use when accessing elements in a vectorized kernel 
*/
template <int KRNL_SZ> int _2Dto1D(int x, int y);

/* number of pixel in a single register */
#define PIXEL_REG(T)  (32 / sizeof(T))

/* ************************* Implementation ************************************************/


template<typename T, int HIGHT, int WIDTH, int SZ>
_GENX_ void 
inline Read8Block(SurfaceIndex obuf, int h_pos, int v_pos, matrix_ref<T, HIGHT, WIDTH> block) 
{

		int vblock, hblock;
		int h_pos_abs = sizeof(T) * h_pos;

#pragma unroll
		for(vblock = 0; vblock < HIGHT - (8 -1); vblock += 8){
#pragma unroll
			for(hblock = 0; hblock < WIDTH - ( SZ -1); hblock +=SZ)
			{
				read((obuf), sizeof(T) * hblock + h_pos_abs , v_pos + vblock, block.select<8,1,SZ,1>(vblock, hblock));
			}
			if(WIDTH % SZ)
			{
				read((obuf), sizeof(T) * hblock + h_pos_abs , v_pos + vblock, block.select<8,1,((WIDTH % SZ) ?(WIDTH % SZ) : 1),1>(vblock, hblock));
			}
		}
		if( HIGHT % 8)
		{
#pragma unroll
			for(hblock = 0; hblock < WIDTH - ( SZ - 1); hblock +=SZ){
				read((obuf), sizeof(T) * hblock + h_pos_abs , v_pos + vblock, block.select<((HIGHT % 8) ?(HIGHT % 8) : 1),1,SZ,1>(vblock, hblock));
			}
			if(WIDTH % SZ)
			{
				read((obuf), sizeof(T) * hblock + h_pos_abs, v_pos + vblock, 
													block.select<((HIGHT % 8) ?(HIGHT % 8) : 1),1,((WIDTH % SZ) ?(WIDTH % SZ) : 1),1>(vblock, hblock));
			}

	}
}


#ifdef OLD
template<typename T, int HIGHT, int WIDTH, int SZ>
_GENX_ void 
inline Write8Block(SurfaceIndex obuf, int h_pos, int v_pos, matrix_ref<T, HIGHT, WIDTH> block) 
{
	int vblock, hblock;
	int h_pos_abs = sizeof(T) * h_pos;
#pragma unroll
	for(vblock = 0; vblock < HIGHT; vblock +=8){
#pragma unroll
		for(hblock = 0; hblock < WIDTH - ( SZ -1); hblock +=SZ)
		{
			write(obuf,  sizeof(T) * hblock + h_pos_abs, v_pos  + vblock, block.select<8,1, SZ ,1>(vblock, hblock));
		}
		if(WIDTH % SZ)
		{
			write(obuf,  sizeof(T) * hblock + h_pos_abs, v_pos  + vblock, block.select<8,1, ((WIDTH % SZ) ?(WIDTH % SZ) : 1) ,1>(vblock, hblock));
		}
	}	

}
#else
template<typename T, int HIGHT, int WIDTH, int SZ>
_GENX_ void 
inline Write8Block(SurfaceIndex obuf, int h_pos, int v_pos, matrix_ref<T, HIGHT, WIDTH> block) 
{

		int vblock, hblock;
		int h_pos_abs = sizeof(T) * h_pos;

#pragma unroll
		for(vblock = 0; vblock < HIGHT - (8 -1); vblock += 8){
#pragma unroll
			for(hblock = 0; hblock < WIDTH - ( SZ -1); hblock +=SZ)
			{
				write((obuf), sizeof(T) * hblock + h_pos_abs , v_pos + vblock, block.select<8,1,SZ,1>(vblock, hblock));
			}
			if(WIDTH % SZ)
			{
				write((obuf), sizeof(T) * hblock + h_pos_abs , v_pos + vblock, block.select<8,1,((WIDTH % SZ) ?(WIDTH % SZ) : 1),1>(vblock, hblock));
			}
		}
		if( HIGHT % 8)
		{
#pragma unroll
			for(hblock = 0; hblock < WIDTH - ( SZ - 1); hblock +=SZ){
				write((obuf), sizeof(T) * hblock + h_pos_abs , v_pos + vblock, block.select<((HIGHT % 8) ?(HIGHT % 8) : 1),1,SZ,1>(vblock, hblock));
			}
			if(WIDTH % SZ)
			{
				write((obuf), sizeof(T) * hblock + h_pos_abs, v_pos + vblock, 
													block.select<((HIGHT % 8) ?(HIGHT % 8) : 1),1,((WIDTH % SZ) ?(WIDTH % SZ) : 1),1>(vblock, hblock));
			}

	}
}


#endif


template<typename T, int HIGHT, int WIDTH> _GENX_ void 
inline cmx_read_block(SurfaceIndex obuf, int h_pos, int v_pos, matrix_ref<T, HIGHT, WIDTH> block)
{

	Read8Block<T, HIGHT, WIDTH, 32 / sizeof(T)>(obuf, h_pos, v_pos,  block);
}


template<typename T, int HIGHT, int STRIDE, int WSEGMENTS> _GENX_ void 
inline ReadBlock(SurfaceIndex obuf, int h_pos, int v_pos, matrix_ref<T, HIGHT * WSEGMENTS, STRIDE> block)
{
#pragma unroll
	for(int i = 0 ; i <  WSEGMENTS; i++)
	{
		ReadBlock<T, HIGHT, STRIDE>(obuf, h_pos + i*STRIDE, v_pos,  block.select<HIGHT, 1,STRIDE,1>(HIGHT*i,0));
	}
}




template<typename T, int HIGHT, int WIDTH> _GENX_ void 
 inline cmx_write_block(SurfaceIndex obuf, int h_pos, int v_pos, matrix_ref<T, HIGHT, WIDTH> block)
{
	Write8Block<T, HIGHT, WIDTH, 32 / sizeof(T)>(obuf, h_pos, v_pos, block);
}

template <int KRNL_SZ>
_GENX_ int
inline _2Dto1D(int x, int y)
{
	return x*KRNL_SZ + y;
}




template<typename Tsrc, typename Tdst, int KRNL_SZ, int SIMD_SZ>
_GENX_ void
inline cmx_vectorize_2Dkrnlrow(vector_ref<Tsrc, SIMD_SZ + KRNL_SZ -1> src, matrix_ref<Tdst, KRNL_SZ * KRNL_SZ, SIMD_SZ> dst, int dstRow)
{
	assert(KRNL_SZ % 2 == 1);
	cm_assert(dstRow < KRNL_SZ * KRNL_SZ);
	int radius = KRNL_SZ /2 ;
	int anchor = KRNL_SZ /2;
#pragma unroll
		for(int  clmn = -radius; clmn < radius + 1; clmn++){
			dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)) = src.select<SIMD_SZ,1>(clmn + anchor);
		}
}

template <typename Tsrc, typename Tdst, int KRNL_SZ, int SIMD_SZ, int OFFSET>
_GENX_ void
inline glue_vector(vector_ref<Tsrc, SIMD_SZ> src_1, vector_ref<Tsrc, KRNL_SZ-1>src_2, vector_ref<Tdst, SIMD_SZ> dst)
{
	dst.select<SIMD_SZ - OFFSET,1>(0) = src_1.select<SIMD_SZ - OFFSET,1>(OFFSET);
	dst.select<OFFSET,1>(SIMD_SZ-OFFSET) = src_2.select<OFFSET,1>(0);
}

#define ROWS_P_LINE(T, LENGTH) (( (LENGTH) + PIXEL_REG(T) -1) / PIXEL_REG(T))

template<typename Tsrc, typename Tdst, int KRNL_SZ, int SIMD_SZ>
_GENX_ void
inline Vectorize2DKRNLRow(/*vector_ref<Tsrc, SIMD_SZ> src, vector_ref<Tsrc, KRNL_SZ -1> src_2,*/
							matrix_ref<Tsrc, ROWS_P_LINE(Tsrc, SIMD_SZ + KRNL_SZ-1), PIXEL_REG(Tsrc)> src,
							matrix_ref<Tdst, KRNL_SZ * KRNL_SZ, SIMD_SZ> dst, int dstRow)
{
	cm_assert(KRNL_SZ % 2 == 1);
	cm_assert(dstRow < KRNL_SZ * KRNL_SZ);
	cm_assert(SIMD_SZ % 8 == 0);
	int radius = KRNL_SZ /2 ;
	int anchor = KRNL_SZ /2;
#pragma unroll
		for(int  clmn = -radius; clmn < radius + 1; clmn++)
		{
			//vector<Tsrc, SIMD_SZ + KRNL_SZ -1> glue;
			
			vector<Tsrc, ROWS_P_LINE(Tsrc, SIMD_SZ + KRNL_SZ-1) * PIXEL_REG(Tsrc)> glue = src;

			dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)) = glue.select<SIMD_SZ,1>(clmn + anchor);
			
			/* not working yet, limited to 3x3
			int clmn = -1; //glue_vector<Tsrc, Tdst, KRNL_SZ, SIMD_SZ,0>(src, src_2, 
				dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)) = src;
			
			clmn = 0; glue_vector<Tsrc, Tdst, KRNL_SZ, SIMD_SZ,1>(src, src_2, 
				dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)));

			clmn = 1; glue_vector<Tsrc, Tdst, KRNL_SZ, SIMD_SZ,2>(src, src_2, 
				dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)));
			*/
			
		}
}



template<typename Tsrc, typename Tdst, int KRNL_SZ, int SIMD_SZ>
_GENX_ void
inline cmx_vectorize_2Dkrnl(matrix_ref<Tsrc, KRNL_SZ, SIMD_SZ + KRNL_SZ -1> src, matrix_ref<Tdst, KRNL_SZ * KRNL_SZ, SIMD_SZ> dst)
{
	assert(KRNL_SZ % 2 == 1);
	
	int radius = KRNL_SZ /2 ;
	int anchor = KRNL_SZ /2;
#pragma unroll
	for(int row = -radius; row < radius + 1; row++){
#pragma unroll
		for(int  clmn = -radius; clmn < radius + 1; clmn++){
			dst.row(_2Dto1D<KRNL_SZ>(row + anchor, clmn + anchor)) = src.select<1,1, SIMD_SZ,1>(row + anchor, clmn + anchor);
		}
	}

}


template<typename Tsrc, typename Tdst, int  ROWS, int KRNL_SZ, int SIMD_SZ>
_GENX_ void
inline Vectorize2DKRNLRow(vector_ref<Tsrc, SIMD_SZ + KRNL_SZ -1> src, matrix_ref<Tdst, ROWS * KRNL_SZ, SIMD_SZ> dst, int dstRow)
{
	assert(KRNL_SZ % 2 == 1);
	cm_assert(dstRow < ROWS * KRNL_SZ);
	int radius = KRNL_SZ /2 ;
	int anchor = KRNL_SZ /2;
#pragma unroll
		for(int  clmn = -radius; clmn < radius + 1; clmn++){
			dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)) = src.select<SIMD_SZ,1>(clmn + anchor);
		}
}

template<typename T, int  ROWS, int KRNL_SZ, int SIMD_SZ>
_GENX_ void
inline Vectorize2DKRNLRow(vector_ref<T, SIMD_SZ + KRNL_SZ -1> src, matrix_ref<T, ROWS * KRNL_SZ, SIMD_SZ> dst, int dstRow)
{
	Vectorize2DKRNLRow<T,T, ROWS, KRNL_SZ, SIMD_SZ>(src, dst, dstRow);
}

_GENX_ void
inline cm_assert(bool x)
{
#ifdef CMRT_EMU
	assert(x);
#endif

} 

