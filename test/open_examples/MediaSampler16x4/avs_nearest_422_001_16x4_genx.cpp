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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	void PA_444AVS_PA(SamplerIndex sIndex, SurfaceIndex iIndex, SurfaceIndex oIndex,
//							uint GroupID, uint VertBlockNum,
//							float Pix0U, float Pix0V,
//							float DeltaU, float DeltaV,
//							float u2d, float v2d,
//							uint DestX, uint DestY)
//
//	Description:
//		This funciton calls sample8x8 to carry out 1x AVS scaling on one 16x4 block on a YUY2 surface.
//
//	Parameters:
//		sIndex - Sampler index of the input surface
//		iIndex - Binding table index of the input surface.
//		oIndex - Binding table index of the output surface.
//		GroupID - Group ID.  In this test, 1 16x4 sampling block per group
//		VertBlockNum -  Vertical block # within the group
//		Pix0U, Pix0V -  pixel 0 addr relative to group origin
//		DeltaU, DeltaV - Scaling step size
//		u2d, v2d - For NLAS
//		DestX, DestY - Destination block addr
//
//	Return:
//		Write scaled block to dest surface.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
PA_444AVS_PA(SamplerIndex sIndex,
			 SurfaceIndex iIndex,
			 SurfaceIndex oIndex,
			 int BlockWidth,
			 int BlockHeight,
			 int FrameWidth,
			 int FrameHeight
			 )
{
    int j = get_thread_origin_x();
    int i = get_thread_origin_y();
    int GroupID = j;
    int VertBlockNum = 0;
    float Pix0U = BlockWidth * j / ((float)FrameWidth);		// Normlized block origin
    float Pix0V = BlockHeight * i / ((float)FrameHeight);
    float DeltaU = 1 / ((float)FrameWidth);					// Normlized step size
    float DeltaV = 1 / ((float)FrameHeight);
    float u2d = 0;												// No NLAS
    float v2d = 0;
    int DestX = 2 * BlockWidth * j;								// yuy2 in byte addr
    int DestY = BlockHeight * i;

#if 1
    matrix<ushort, 4, 64> Writeback;
	matrix_ref<uchar, 4, 128> bWriteback = Writeback.format<uchar, 4, 128>();
	matrix<uchar, 4, 32> YUY2_422;  //output container for output_format = .yuy2
#else
	matrix<ushort, 6, 64> Writeback;
	matrix_ref<uchar, 6, 128> bWriteback = Writeback.format<uchar, 6, 128>();
	matrix<uchar, 8, 32> YUY2_422;  //output container for output_format = .yuy2
#endif

	//YUY2_422.select<4,1,8,4>(4,3)  = bWriteback.select<1,1,32,4>(3,1);				// V
	//YUY2_422.select<4,1,16,2>(4,0) = bWriteback.select<1,1,64,2>(4,1);				// Y
	//YUY2_422.select<4,1,8,4>(4,1)  = bWriteback.select<1,1,32,4>(5,1);				// U

	matrix<ushort, 4, 64> Y16U16V16A16;
	// Old
	// 1x non NLAS AVS scaling, 444 output, block size 16x4, IEFbypass is true
	// sample8x8AVS(Writeback, CM_BGR_ENABLE, iIndex, sIndex, Pix0U, Pix0V, DeltaU, DeltaV, u2d, CM_16_FULL, v2d, CM_AVS_16x4, true);

	// New
	// sample8x8AVS(matrix<RT, N, 64> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex,
    //				const float &u, const float &v, const float &deltaU, const float &deltaV, const float &u2d,
    //				int groupID = -1, short verticalBlockNumber = -1,
    //				OutputFormatControl outControl=CM_16_FULL, float v2d=0, AVSExecMode execMode=CM_AVS_16x4, bool IEFBypass=false )


	// Return 16bits YUV444, 3*64 ushort in 12 GRFs, alpha channel is ignored.

	// 444 -> 422.  Both solutions work.
	// Shift right to low byte
	// YUY2_422.select<4,1,8,4>(0,3)  = Writeback.select<1,1,32,2>(0,0) >> 8;		// V
	// YUY2_422.select<4,1,16,2>(0,0) = Writeback.select<1,1,64,1>(1,0) >> 8;		// Y
	// YUY2_422.select<4,1,8,4>(0,1)  = Writeback.select<1,1,32,2>(2,0) >> 8;		// U

	/*if(output16bit){
		//Arrange writeback data to be of .y416 format
		Y16U16V16A16.select<4,1,16,4>(0,2) = Writeback.select<1,1,64,1>(0,0);						// V
		Y16U16V16A16.select<4,1,16,4>(0,0) = Writeback.select<1,1,64,1>(1,0);						// Y
		Y16U16V16A16.select<4,1,16,4>(0,1) = Writeback.select<1,1,64,1>(2,0);						// U
		Y16U16V16A16.select<4,1,16,4>(0,3) = Writeback.select<1,1,64,1>(3,0);						// A

		// Output AVSed block 16x4 pixels = 64x4 bytes.  If output_format = 16bits per pixel.
	    write(oIndex, DestX,      DestY, Y16U16V16A16.select<4,1,16,1>(0,0));
		write(oIndex, DestX + 32, DestY, Y16U16V16A16.select<4,1,16,1>(0,16));
		write(oIndex, DestX + 64, DestY, Y16U16V16A16.select<4,1,16,1>(0,32));
		write(oIndex, DestX + 96, DestY, Y16U16V16A16.select<4,1,16,1>(0,48));
	}
	else{*/
		// Take high byte and arrage writeback data to be of .yuy2 format

#if 1
		cm_avs_sampler(Writeback, CM_ABGR_ENABLE, iIndex, sIndex, Pix0U, Pix0V, DeltaU, DeltaV, u2d, GroupID, VertBlockNum, CM_16_FULL, v2d, CM_AVS_16x4, true);
		YUY2_422.select<4,1,8,4>(0,3)  = bWriteback.select<1,1,32,4>(0,1);				// V
		YUY2_422.select<4,1,16,2>(0,0) = bWriteback.select<1,1,64,2>(1,1);				// Y
		YUY2_422.select<4,1,8,4>(0,1)  = bWriteback.select<1,1,32,4>(2,1);				// U

		// Output AVSed block 16x4 pixels = 32x4 bytes.  I foutput_format = 8bits per pixel.
	    write(oIndex, DestX, DestY, YUY2_422);
#else
	//}
		cm_avs_sampler(Writeback, CM_BGR_ENABLE, iIndex, sIndex, Pix0U, Pix0V, DeltaU, DeltaV, u2d, GroupID, VertBlockNum, CM_16_FULL, v2d, CM_AVS_16x8, true);
		YUY2_422.select<4,1,8,4>(0,3)  = bWriteback.select<1,1,32,4>(0,1);				// V
		YUY2_422.select<4,1,16,2>(0,0) = bWriteback.select<1,1,64,2>(1,1);				// Y
		YUY2_422.select<4,1,8,4>(0,1)  = bWriteback.select<1,1,32,4>(2,1);				// U

		YUY2_422.select<4,1,8,4>(4,3)  = bWriteback.select<1,1,32,4>(3,1);				// V
		YUY2_422.select<4,1,16,2>(4,0) = bWriteback.select<1,1,64,2>(4,1);				// Y
		YUY2_422.select<4,1,8,4>(4,1)  = bWriteback.select<1,1,32,4>(5,1);				// U
		// Output AVSed block 16x4 pixels = 32x4 bytes.  I foutput_format = 8bits per pixel.
	    write(oIndex, DestX, DestY, YUY2_422);
#endif

}
