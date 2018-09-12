#include <cm/cm.h>


/////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void Sobel_3x3_8u16s(SurfaceIndex iIndex, SurfaceIndex oIndex) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<uchar, 18, 32> bWriteback;	
	matrix<short, 18, 16> sTemp;
	matrix<short, 16, 16> sGx;
	matrix<short, 16, 16> sGy;

	vector<float, 16> fG;
	matrix<float, 16, 16> fGm;

	uint nX = 16*h_pos-1;   // Read from (-1,-1)
	uint nY = 16*v_pos-1;

	read(iIndex, nX, nY, bWriteback.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bWriteback.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bWriteback.select<2,1,32,1>(16,0));

	// Get [-1 0 1] for all rows
	sTemp = bWriteback.select<18,1,16,1>(0,0) - bWriteback.select<18,1,16,1>(0,2);

	// Add 3 rows with weight
	sGx = sTemp.select<16,1,16,1>(0,0) + sTemp.select<16,1,16,1>(2,0);
	sGx += 2 * sTemp.select<16,1,16,1>(1,0);

	// Get [1 2 1] for all rows
	sTemp = bWriteback.select<18,1,16,1>(0,2) + bWriteback.select<18,1,16,1>(0,0);
	sTemp +=  2 * bWriteback.select<18,1,16,1>(0,1);

	sGy = sTemp.select<16,1,16,1>(2,0) - sTemp.select<16,1,16,1>(0,0);

	fGm = sGx*sGx + sGy*sGy;
	sGy = cm_sqrt(fGm);

	// Output 16x16 short
	write(oIndex, 32*h_pos, 16*v_pos, sGy.select<8,1,16,1>(0,0));	
	write(oIndex, 32*h_pos, 16*v_pos+8, sGy.select<8,1,16,1>(8,0));
}

/////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void SobelEdge_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<uchar, 18, 32> bWriteback;	
	matrix<short, 18, 16> sTemp;
	matrix<short, 16, 16> sGx;
	matrix<short, 16, 16> sGy;

	matrix<uchar, 16, 16> bDest;	

	vector<float, 16> fG;
	matrix<float, 16, 16> fGm;

	uint nX = 16*h_pos-1;   // Read from (-1,-1)
	uint nY = 16*v_pos-1;

	read(iIndex, nX, nY, bWriteback.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bWriteback.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bWriteback.select<2,1,32,1>(16,0));

	// Get [-1 0 1] for all rows
	sTemp = bWriteback.select<18,1,16,1>(0,0) - bWriteback.select<18,1,16,1>(0,2);

	// Add 3 rows with weight
	sGx = sTemp.select<16,1,16,1>(0,0) + sTemp.select<16,1,16,1>(2,0);
	sGx += 2 * sTemp.select<16,1,16,1>(1,0);

	// Get [1 2 1] for all rows
	sTemp = bWriteback.select<18,1,16,1>(0,2) + bWriteback.select<18,1,16,1>(0,0);
	sTemp +=  2 * bWriteback.select<18,1,16,1>(0,1);

	sGy = sTemp.select<16,1,16,1>(2,0) - sTemp.select<16,1,16,1>(0,0);

	fGm = sGx*sGx + sGy*sGy;
	sGy = cm_sqrt(fGm);

    bDest.merge(0, 255, cm_abs<short>(sGy) < 128);

	// Output 16x16 u8
	write(oIndex, 16*h_pos, 16*v_pos, bDest);	
}

/////////////////////////////////////////////////////////////////////////////////////////
// IVB sampler output format
extern "C" _GENX_MAIN_ void Scaling16x16(SamplerIndex sIndex, SurfaceIndex iIndex, SurfaceIndex oIndex, float DeltaU, float DeltaV)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<ushort, 1, 32> wData0;		// 1 channel of 4 rows x 8	columns in 8.8 fixed point format 
	matrix_ref<uchar, 1, 64> bIVBData0 = wData0.format<uchar, 1, 64>();		// Byte version of wWriteback
	
	matrix<ushort, 1, 32> wData1;
	matrix_ref<uchar, 1, 64> bIVBData1 = wData1.format<uchar, 1, 64>();

	matrix<ushort, 1, 32> wData2;
	matrix_ref<uchar, 1, 64> bIVBData2 = wData2.format<uchar, 1, 64>();

	matrix<ushort, 1, 32> wData3;
	matrix_ref<uchar, 1, 64> bIVBData3 = wData3.format<uchar, 1, 64>();

	matrix<ushort, 1, 32> wData4;
	matrix_ref<uchar, 1, 64> bIVBData4 = wData4.format<uchar, 1, 64>();

	matrix<ushort, 1, 32> wData5;
	matrix_ref<uchar, 1, 64> bIVBData5 = wData5.format<uchar, 1, 64>();

	matrix<ushort, 1, 32> wData6;
	matrix_ref<uchar, 1, 64> bIVBData6 = wData6.format<uchar, 1, 64>();

	matrix<ushort, 1, 32> wData7;
	matrix_ref<uchar, 1, 64> bIVBData7 = wData7.format<uchar, 1, 64>();

	matrix<uchar, 16, 16> Y;

	float nX0 = (float) (16 * DeltaU * (h_pos + 1/32.0f));
	float nY = (float) (16 * DeltaV * (v_pos + 1/32.0f));
	
	float nX8 = nX0+(8 * DeltaU);

	// Call sampler32() to use sample unorm for scaling 
	sample32(wData0, CM_A_ENABLE, iIndex, sIndex, nX0, nY, DeltaU, DeltaV); 
	Y.select<4,1,8,1>(0,0) = bIVBData0.select<1,1,32,2>(0,1);	// Take 4x8 high bytes

	sample32(wData1, CM_A_ENABLE, iIndex, sIndex, nX8, nY, DeltaU, DeltaV); 
	Y.select<4,1,8,1>(0,8) = bIVBData1.select<1,1,32,2>(0,1);

	nY += (4 * DeltaV);
	sample32(wData2, CM_A_ENABLE, iIndex, sIndex, nX0, nY, DeltaU, DeltaV); 
	Y.select<4,1,8,1>(4,0) = bIVBData2.select<1,1,32,2>(0,1);
	
	sample32(wData3, CM_A_ENABLE, iIndex, sIndex, nX8, nY, DeltaU, DeltaV); 
	Y.select<4,1,8,1>(4,8) = bIVBData3.select<1,1,32,2>(0,1);

	nY += (4 * DeltaV);
	sample32(wData4, CM_A_ENABLE, iIndex, sIndex, nX0, nY, DeltaU, DeltaV);
	Y.select<4,1,8,1>(8,0) = bIVBData4.select<1,1,32,2>(0,1);
	
	sample32(wData5, CM_A_ENABLE, iIndex, sIndex, nX8, nY, DeltaU, DeltaV); 
	Y.select<4,1,8,1>(8,8) = bIVBData5.select<1,1,32,2>(0,1);

	nY += (4 * DeltaV);
	sample32(wData6, CM_A_ENABLE, iIndex, sIndex, nX0, nY, DeltaU, DeltaV); 
	Y.select<4,1,8,1>(12,0) = bIVBData6.select<1,1,32,2>(0,1);
	
	sample32(wData7, CM_A_ENABLE, iIndex, sIndex, nX8, nY, DeltaU, DeltaV); 
	Y.select<4,1,8,1>(12,8) = bIVBData7.select<1,1,32,2>(0,1);

	write(oIndex, 16*h_pos, 16*v_pos, Y);
}

/////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void RGBXToNV12(SurfaceIndex iIndex, SurfaceIndex oIndexNV12, int FrameHeight)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    matrix<uchar, 8, 32> bRGBX;	

	matrix<uchar, 8, 8> bR;	
	matrix<uchar, 8, 8> bG;	
	matrix<uchar, 8, 8> bB;	

	matrix<uchar, 8, 8> bY;	
	matrix<uchar, 8, 8> bU;	
	matrix<uchar, 8, 8> bV;	
	matrix<uchar, 4, 8> bUV;	

    matrix<short, 8, 8> sTemp;
    matrix<short, 4, 4> sTemp2;

	uint nX = 32*h_pos;
	uint nY = 8*v_pos;
    read(iIndex, nX, nY, bRGBX.select<8,1,32,1>(0,0));   // Read RGBX, 8x32

    bR = bRGBX.select<8,1,8,4>(0,0);
    bG = bRGBX.select<8,1,8,4>(0,1);
    bB = bRGBX.select<8,1,8,4>(0,2);

    // CSC
    // y = ( (  65 * r + 128 * g +  24 * b + 128) >> 8) +  16;
    // u = ( ( -37 * r -  74 * g + 112 * b + 128) >> 8) + 128;
    // v = ( ( 112 * r -  93 * g -  18 * b + 128) >> 8) + 128;
    // CSC weights from msdn
    // y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
    // u = ( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
    // v = ( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128;
    
    //bY = (( 65 * bR + 128 * bG + 24 * bB + 128) >> 8) + 16;
    sTemp = 128;
    sTemp += 66 * bR;
    sTemp += 129 * bG;
    sTemp += 25 * bB;
    sTemp >>= 8;
    sTemp += 16;
    bY = sTemp;

    //bU = (( -37 * bR - 74 * bG + 112 * bB + 128) >> 8) + 128;
    sTemp = 128;
    sTemp -= 38 * bR;
    sTemp -= 74 * bG;
    sTemp += 112 * bB;
    sTemp >>= 8;
    sTemp += 128;
    bU = sTemp;

    //bV = (( 112 * bR - 93 * bG - 18 * bB + 128) >> 8) + 128;
    sTemp = 128;
    sTemp += 112 * bR;
    sTemp -= 94 * bG;
    sTemp -= 18 * bB;
    sTemp >>= 8;
    sTemp += 128;
    bV = sTemp;


    sTemp2 = bU.select<4,2,4,2>(0,0) + bU.select<4,2,4,2>(0,1);
    sTemp2 += bU.select<4,2,4,2>(1,0);
    sTemp2 += bU.select<4,2,4,2>(1,1);
    sTemp2 >>= 2;
    bUV.select<4,1,4,2>(0,0) = sTemp2;

// Debug
/*
if ( nX == 0 && nY == 0 ) {
	for (int j = 0; j < 4; j++) {
	    for (int i = 0; i < 4; i++)
		    printf("%d, ", sTemp[i][j]);
	    printf("\n");
    }
    printf("\n");
}
*/

    //bV2 = (bV.select<4,1,4,1>(0,0) + bV.select<4,1,4,1>(0,1) + bV.select<4,1,4,1>(1,0) + bV.select<4,1,4,1>(1,1)) >> 2;
    sTemp2 = bV.select<4,2,4,2>(0,0) + bV.select<4,2,4,2>(0,1);
    sTemp2 += bV.select<4,2,4,2>(1,0);
    sTemp2 += bV.select<4,2,4,2>(1,1);
    sTemp2 >>= 2;
    bUV.select<4,1,4,2>(0,1) = sTemp2;

    write_plane(oIndexNV12, GENX_SURFACE_Y_PLANE, 8*h_pos, 8*v_pos, bY);
    write_plane(oIndexNV12, GENX_SURFACE_UV_PLANE, 8*h_pos, 4*v_pos, bUV);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void NV12toYUY2(SurfaceIndex iNV12Index, SurfaceIndex oYUY2Index, int FrameHeight)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    matrix<uchar, 16, 16> bY;	
	matrix<uchar, 9, 16> bUV;	
    matrix<uchar, 16, 32> bYUY2;	

	uint h16 = 16*h_pos;
	uint h32 = 32*h_pos;
	uint v16 = 16*v_pos;
	uint v8 = 8*v_pos;

    // Read 16*16 Y
    read_plane(iNV12Index, GENX_SURFACE_Y_PLANE, h16, v16, bY);
    // Read 9*16 UV
    read_plane(iNV12Index, GENX_SURFACE_UV_PLANE, h16, v8, bUV);

    // Y
    bYUY2.select<16,1,16,2>(0,0) = bY;
    // Interpolated UV
    bYUY2.select<8,2,16,2>(0,1) = bUV.select<8,1,16,1>(0,0);
    bYUY2.select<8,2,16,2>(1,1) = (bUV.select<8,1,16,1>(0,0) + bUV.select<8,1,16,1>(1,0)) >> 1;

    // Output 16*32 YUY2
    write(oYUY2Index, h32, v16, bYUY2.select<8,1,32,1>(0,0));
    write(oYUY2Index, h32, v16+8, bYUY2.select<8,1,32,1>(8,0));
}

/////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void ColorToGray(SurfaceIndex iIndex, SurfaceIndex oIndex, int InputFormat)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    matrix<uchar, 8, 32> bColor3;	// RGB and BGR 3 bytes
    matrix<uchar, 8, 32> bColor4;	// RGB and BGR 3 bytes

	matrix<uchar, 8, 8> bR;	
	matrix<uchar, 8, 8> bG;	
	matrix<uchar, 8, 8> bB;	

	matrix<uchar, 8, 8> bY;	
    matrix<short, 8, 8> sTemp;

	uint nX = 24*h_pos;
	uint nY = 8*v_pos;
    read(iIndex, nX, nY, bColor3.select<8,1,24,1>(0,0));

    // 3 bytes to 4 bytes block expansion
    bColor4.select<8,1,4,1>(0,0) = bColor3.select<8,1,4,1>(0,0);    // 4
    bColor4.select<8,1,4,1>(0,4) = bColor3.select<8,1,4,1>(0,3);    // 4
    bColor4.select<8,1,4,1>(0,8) = bColor3.select<8,1,4,1>(0,6);    // 4
    bColor4.select<8,1,4,1>(0,12) = bColor3.select<8,1,4,1>(0,9);    // 4
    bColor4.select<8,1,4,1>(0,16) = bColor3.select<8,1,4,1>(0,12);    // 4
    bColor4.select<8,1,4,1>(0,20) = bColor3.select<8,1,4,1>(0,15);    // 4
    bColor4.select<8,1,4,1>(0,24) = bColor3.select<8,1,4,1>(0,18);    // 4
    bColor4.select<8,1,4,1>(0,28) = bColor3.select<8,1,4,1>(0,21);    // 4

    if (InputFormat == 0) {     // LSB is R
        bR = bColor4.select<8,1,8,4>(0,0);
        bG = bColor4.select<8,1,8,4>(0,1);
        bB = bColor4.select<8,1,8,4>(0,2);
    } else {                    // LSB is B
        bB = bColor4.select<8,1,8,4>(0,0);
        bG = bColor4.select<8,1,8,4>(0,1);
        bR = bColor4.select<8,1,8,4>(0,2);
    }

    // CSC weights from msdn
    // y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
    // u = ( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
    // v = ( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128;
    
    // Y only
    sTemp = 128;
    sTemp += 66 * bR;
    sTemp += 129 * bG;
    sTemp += 25 * bB;
    sTemp >>= 8;
    sTemp += 16;
    bY = sTemp;

    write(oIndex, 8*h_pos, 8*v_pos, bY);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void Dilate_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex)
{
	matrix<uchar, 18, 32> bWriteback;	
	matrix<uchar, 16, 16> bOut(0);

	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();
    int BaseX = 1;
    int BaseY = 1;

	uint nX = 16*h_pos - BaseX;   // Read from (-1,-1)
	uint nY = 16*v_pos - BaseY;

	read(iIndex, nX, nY, bWriteback.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bWriteback.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bWriteback.select<2,1,32,1>(16,0));

    if (bWriteback.select<18,1,18,1>(0,0).any()) {     // Bypass if none of pixels is on

        // Set true if any of neightbors is true 
        bOut.merge(255, bWriteback.select<16,1,16,1>(BaseY,BaseX), 
                        bWriteback.select<16,1,16,1>(BaseY-1,BaseX-1) > 0 | 
                        bWriteback.select<16,1,16,1>(BaseY-1,BaseX) > 0 | 
                        bWriteback.select<16,1,16,1>(BaseY-1,BaseX+1) > 0 |
                        bWriteback.select<16,1,16,1>(BaseY,BaseX-1) > 0 | 
                        bWriteback.select<16,1,16,1>(BaseY,BaseX+1) > 0 |
                        bWriteback.select<16,1,16,1>(BaseY+1,BaseX-1) > 0 | 
                        bWriteback.select<16,1,16,1>(BaseY+1,BaseX) > 0 | 
                        bWriteback.select<16,1,16,1>(BaseY+1,BaseX+1) > 0 );
	    write(oIndex, 16*h_pos, 16*v_pos, bOut);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void Erode_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex)
{
	matrix<uchar, 18, 32> bWriteback;	
	matrix<uchar, 16, 16> bOut(0);

	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();
    int BaseX = 1;
    int BaseY = 1;

	uint nX = 16*h_pos - BaseX;   // Read from (-1,-1)
	uint nY = 16*v_pos - BaseY;

	read(iIndex, nX, nY, bWriteback.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bWriteback.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bWriteback.select<2,1,32,1>(16,0));

    if (bWriteback.select<18,1,18,1>(0,0).any()) {     // Bypass if none of pixels is on

        // Set to false if any of neightbors are false 
        bOut.merge(0, bWriteback.select<16,1,16,1>(BaseY,BaseX), 
                        bWriteback.select<16,1,16,1>(BaseY-1,BaseX-1) == 0 | 
                        bWriteback.select<16,1,16,1>(BaseY-1,BaseX) == 0 | 
                        bWriteback.select<16,1,16,1>(BaseY-1,BaseX+1) == 0 |
                        bWriteback.select<16,1,16,1>(BaseY,BaseX-1) == 0 | 
                        bWriteback.select<16,1,16,1>(BaseY,BaseX+1) == 0 |
                        bWriteback.select<16,1,16,1>(BaseY+1,BaseX-1) == 0 | 
                        bWriteback.select<16,1,16,1>(BaseY+1,BaseX) == 0 | 
                        bWriteback.select<16,1,16,1>(BaseY+1,BaseX+1) == 0 );
	    write(oIndex, 16*h_pos, 16*v_pos, bOut);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////
// Assume the src is U8 mask and contains binary values, 0 and 1
// Algorithm: sum up 3x3 pixels, if sum >= 5, its median = 1, otherwise = 0.
extern "C" _GENX_MAIN_ void BinaryMedian_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<uchar, 18, 32> bSrc;	
	matrix<uchar, 16, 16> bDest;

    matrix<short, 18, 16> sTemp;
	matrix<short, 16, 16> sSum;

	uint nX = 16*h_pos-1;   // Read from (-1,-1)
	uint nY = 16*v_pos-1;

    // Read 18x32
	read(iIndex, nX, nY, bSrc.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bSrc.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bSrc.select<2,1,32,1>(16,0));

	// Sum 3 columns for all rows
	sTemp = bSrc.select<18,1,16,1>(0,0) + bSrc.select<18,1,16,1>(0,1);
    sTemp += bSrc.select<18,1,16,1>(0,2);

	// Sum 3 rows
	sSum = sTemp.select<16,1,16,1>(0,0) + sTemp.select<16,1,16,1>(1,0);
    sSum += sTemp.select<16,1,16,1>(2,0);
	
    sSum.merge(0, 255, sSum <= 1020);  // 4*255 = 1020
    bDest = sSum;

    write(oIndex, 16*h_pos, 16*v_pos, bDest);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Assume the src is U8 mask and contains binary values, 0 and 1
// Algorithm: sum up 5x5 pixels, if sum >= 13, its median = 1, otherwise = 0.
extern "C" _GENX_MAIN_ void BinaryMedian_5x5_8u(SurfaceIndex iIndex, SurfaceIndex oIndex) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<uchar, 20, 32> bSrc;	
	matrix<uchar, 16, 16> bDest;

    matrix<short, 20, 16> sTemp;
	matrix<short, 16, 16> sSum;

	uint nX = 16*h_pos-2;   // Read from (-2,-2)
	uint nY = 16*v_pos-2;

    // Read 20x32
	read(iIndex, nX, nY, bSrc.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bSrc.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bSrc.select<4,1,32,1>(16,0));

	// Sum 5 columns for all rows
	sTemp = bSrc.select<20,1,16,1>(0,0) + bSrc.select<20,1,16,1>(0,1);
    sTemp += bSrc.select<20,1,16,1>(0,2);
    sTemp += bSrc.select<20,1,16,1>(0,3);
    sTemp += bSrc.select<20,1,16,1>(0,4);

	// Sum 5 rows
	sSum = sTemp.select<16,1,16,1>(0,0) + sTemp.select<16,1,16,1>(1,0);
    sSum += sTemp.select<16,1,16,1>(2,0);
    sSum += sTemp.select<16,1,16,1>(3,0);
    sSum += sTemp.select<16,1,16,1>(4,0);
	
    sSum.merge(0, 255, sSum <= 3060);  // 12*255 = 3060
    bDest = sSum;

// Debug
/*
if ( nX == 0 && nY == 0 ) {
	for (int j = 0; j < 4; j++) {
	    for (int i = 0; i < 4; i++)
		    printf("%d, ", bDest[j][i]);
	    printf("\n");
    }
    printf("\n");
}
*/
    write(oIndex, 16*h_pos, 16*v_pos, bDest);
}

/////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void BinaryMedian_9x9_8u(SurfaceIndex iIndex, SurfaceIndex oIndex) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<uchar, 24, 32> bSrc;	
	matrix<uchar, 16, 16> bDest;

    matrix<short, 24, 16> sTemp;
	matrix<short, 16, 16> sSum;

	uint nX = 16*h_pos-4;   // Read from (-4,-4)
	uint nY = 16*v_pos-4;

    // Read 24x32
	read(iIndex, nX, nY, bSrc.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bSrc.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bSrc.select<8,1,32,1>(16,0));

	// Sum 9 columns for all rows
	sTemp = bSrc.select<24,1,16,1>(0,0) + bSrc.select<24,1,16,1>(0,1);
    sTemp += bSrc.select<24,1,16,1>(0,2);
    sTemp += bSrc.select<24,1,16,1>(0,3);
    sTemp += bSrc.select<24,1,16,1>(0,4);
    sTemp += bSrc.select<24,1,16,1>(0,5);
    sTemp += bSrc.select<24,1,16,1>(0,6);
    sTemp += bSrc.select<24,1,16,1>(0,7);
    sTemp += bSrc.select<24,1,16,1>(0,8);

	// Sum 9 rows
	sSum = sTemp.select<16,1,16,1>(0,0) + sTemp.select<16,1,16,1>(1,0);
    sSum += sTemp.select<16,1,16,1>(2,0);
    sSum += sTemp.select<16,1,16,1>(3,0);
    sSum += sTemp.select<16,1,16,1>(4,0);
    sSum += sTemp.select<16,1,16,1>(5,0);
    sSum += sTemp.select<16,1,16,1>(6,0);
    sSum += sTemp.select<16,1,16,1>(7,0);
    sSum += sTemp.select<16,1,16,1>(8,0);
	
    sSum.merge(0, 255, sSum <= 10200);  // 40*255 = 10200
    bDest = sSum;

// Debug
/*
if ( nX == 0 && nY == 0 ) {
	for (int j = 0; j < 4; j++) {
	    for (int i = 0; i < 4; i++)
		    printf("%d, ", bDest[j][i]);
	    printf("\n");
    }
    printf("\n");
}
*/
    write(oIndex, 16*h_pos, 16*v_pos, bDest);
}

/////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void BinaryMedian_Mask_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex iMaskIndex) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    matrix<uchar, 16, 16> bMask;
	matrix<uchar, 18, 32> bSrc;	
	matrix<uchar, 16, 16> bDest;

    vector<ushort, 16> wMask;
    matrix<short, 18, 16> sTemp;
	matrix<short, 16, 16> sSum;

	uint nX = 16*h_pos-1;   // Read from (-1,-1)
	uint nY = 16*v_pos-1;

    // Read 16x16 mask
	read(iMaskIndex, 16*h_pos, 16*v_pos, bMask);

    // Sum up the mask
    wMask = bMask.row(0) + bMask.row(1);
    wMask += bMask.row(2);
    wMask += bMask.row(3);
    wMask += bMask.row(4);
    wMask += bMask.row(5);
    wMask += bMask.row(6);
    wMask += bMask.row(7);
    wMask.select<4,1>(0) += wMask.select<4,1>(4);
    wMask.select<2,1>(0) += wMask.select<2,1>(2);
    wMask[0] += wMask[1];

    // Skip if mask block is all 0s.  Pass input through
    if (wMask[0] == 0) 
    {
        read(iIndex, 16*h_pos, 16*v_pos, bDest);
        write(oIndex, 16*h_pos, 16*v_pos, bDest);
        return;     
    }

    // Read 18x32
	read(iIndex, nX, nY, bSrc.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bSrc.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bSrc.select<2,1,32,1>(16,0));

	// Sum 3 columns for all rows
	sTemp = bSrc.select<18,1,16,1>(0,0) + bSrc.select<18,1,16,1>(0,1);
    sTemp += bSrc.select<18,1,16,1>(0,2);

	// Sum 5 rows
	sSum = sTemp.select<16,1,16,1>(0,0) + sTemp.select<16,1,16,1>(1,0);
    sSum += sTemp.select<16,1,16,1>(2,0);
	
    sSum.merge(0, 255, sSum <= 1020);  // 4*255 = 1020
    bDest = sSum;

    write(oIndex, 16*h_pos, 16*v_pos, bDest);
}

/////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void BinaryMedian_Mask_5x5_8u(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex iMaskIndex) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    matrix<uchar, 16, 16> bMask;
	matrix<uchar, 20, 32> bSrc;	
	matrix<uchar, 16, 16> bDest;

    vector<ushort, 16> wMask;
    matrix<short, 20, 16> sTemp;
	matrix<short, 16, 16> sSum;

	uint nX = 16*h_pos-2;   // Read from (-2,-2)
	uint nY = 16*v_pos-2;

    // Read 16x16 mask
	read(iMaskIndex, 16*h_pos, 16*v_pos, bMask);

    // Sum up the mask
    wMask = bMask.row(0) + bMask.row(1);
    wMask += bMask.row(2);
    wMask += bMask.row(3);
    wMask += bMask.row(4);
    wMask += bMask.row(5);
    wMask += bMask.row(6);
    wMask += bMask.row(7);
    wMask.select<4,1>(0) += wMask.select<4,1>(4);
    wMask.select<2,1>(0) += wMask.select<2,1>(2);
    wMask[0] += wMask[1];

    // Skip if mask block is all 0s.  Pass input through
    if (wMask[0] == 0) 
    {
        read(iIndex, 16*h_pos, 16*v_pos, bDest);
        write(oIndex, 16*h_pos, 16*v_pos, bDest);
        return;     
    }

    // Read 20x32
	read(iIndex, nX, nY, bSrc.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bSrc.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bSrc.select<4,1,32,1>(16,0));

	// Sum 5 columns for all rows
	sTemp = bSrc.select<20,1,16,1>(0,0) + bSrc.select<20,1,16,1>(0,1);
    sTemp += bSrc.select<20,1,16,1>(0,2);
    sTemp += bSrc.select<20,1,16,1>(0,3);
    sTemp += bSrc.select<20,1,16,1>(0,4);

	// Sum 5 rows
	sSum = sTemp.select<16,1,16,1>(0,0) + sTemp.select<16,1,16,1>(1,0);
    sSum += sTemp.select<16,1,16,1>(2,0);
    sSum += sTemp.select<16,1,16,1>(3,0);
    sSum += sTemp.select<16,1,16,1>(4,0);
	
    sSum.merge(0, 255, sSum <= 3060);  // 12*255 = 3060
    bDest = sSum;

    write(oIndex, 16*h_pos, 16*v_pos, bDest);
}

/////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void BinaryMedian_Mask_9x9_8u(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex iMaskIndex) 
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    matrix<uchar, 16, 16> bMask;
	matrix<uchar, 24, 32> bSrc;	
	matrix<uchar, 16, 16> bDest;

    vector<ushort, 16> wMask;
    matrix<short, 24, 16> sTemp;
	matrix<short, 16, 16> sSum;

	uint nX = 16*h_pos-4;   // Read from (-4,-4)
	uint nY = 16*v_pos-4;

    // Read 16x16 mask
	read(iMaskIndex, 16*h_pos, 16*v_pos, bMask);

    // Sum up the mask
    wMask = bMask.row(0) + bMask.row(1);
    wMask += bMask.row(2);
    wMask += bMask.row(3);
    wMask += bMask.row(4);
    wMask += bMask.row(5);
    wMask += bMask.row(6);
    wMask += bMask.row(7);
    wMask.select<4,1>(0) += wMask.select<4,1>(4);
    wMask.select<2,1>(0) += wMask.select<2,1>(2);
    wMask[0] += wMask[1];

    // Skip if mask block is all 0s.  Pass input through
    if (wMask[0] == 0) 
    {
        read(iIndex, 16*h_pos, 16*v_pos, bDest);
        write(oIndex, 16*h_pos, 16*v_pos, bDest);
        return;     
    }

    // Read 24x32 image
	read(iIndex, nX, nY, bSrc.select<8,1,32,1>(0,0));
	read(iIndex, nX, nY+8, bSrc.select<8,1,32,1>(8,0));
	read(iIndex, nX, nY+16, bSrc.select<8,1,32,1>(16,0));

	// Sum 9 columns for all rows
	sTemp = bSrc.select<24,1,16,1>(0,0) + bSrc.select<24,1,16,1>(0,1);
    sTemp += bSrc.select<24,1,16,1>(0,2);
    sTemp += bSrc.select<24,1,16,1>(0,3);
    sTemp += bSrc.select<24,1,16,1>(0,4);
    sTemp += bSrc.select<24,1,16,1>(0,5);
    sTemp += bSrc.select<24,1,16,1>(0,6);
    sTemp += bSrc.select<24,1,16,1>(0,7);
    sTemp += bSrc.select<24,1,16,1>(0,8);

	// Sum 9 rows
	sSum = sTemp.select<16,1,16,1>(0,0) + sTemp.select<16,1,16,1>(1,0);
    sSum += sTemp.select<16,1,16,1>(2,0);
    sSum += sTemp.select<16,1,16,1>(3,0);
    sSum += sTemp.select<16,1,16,1>(4,0);
    sSum += sTemp.select<16,1,16,1>(5,0);
    sSum += sTemp.select<16,1,16,1>(6,0);
    sSum += sTemp.select<16,1,16,1>(7,0);
    sSum += sTemp.select<16,1,16,1>(8,0);
	
    sSum.merge(0, 255, sSum <= 10200);  // 40*255 = 10200
    bDest = sSum;

    write(oIndex, 16*h_pos, 16*v_pos, bDest);
}


/////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void MergeRGBA_8u(SurfaceIndex iIndex, SurfaceIndex iIndex2, SurfaceIndex oIndex)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    matrix<uchar, 8, 32> bRGBX;	
	matrix<uchar, 8, 8> bA;	

	uint nX = 32*h_pos;
	uint nY = 8*v_pos;
    read(iIndex, nX, nY, bRGBX.select<8,1,32,1>(0,0));   // Read RGBX, 8x32
    
	nX = 8*h_pos;
	nY = 8*v_pos;
	read(iIndex2, nX, nY, bA.select<8,1,8,1>(0,0));    // Read alpha, 8x8

    bRGBX.select<8,1,8,4>(0,3) = bA.select<8,1,8,1>(0,0);   // Merge alpha channel

    write(oIndex, 32*h_pos, 8*v_pos, bRGBX);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void GPU_memcpy(SurfaceIndex iIndex, SurfaceIndex oIndex)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<uchar, 8, 32> bWriteback;	

	uint nX = 32*h_pos;
	uint nY = 8*v_pos;

	read(iIndex, nX, nY, bWriteback);
	write(oIndex, nX, nY, bWriteback);	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void NULL_kernel(SurfaceIndex iIndex, SurfaceIndex oIndex)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void GPU_memcpy_1D(SurfaceIndex iIndex, SurfaceIndex oIndex, int ThreadCount)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    // Each thread process min 16 bytes, max 128 bytes
	vector<uchar, 128> bWriteback;	            // 16 OWORD
    uint offset = (511 * v_pos + h_pos)*128;      // in bytes

//    vector<uchar, 16> bWriteback;	                // 1 OWORD
//    uint offset = (511 * v_pos + h_pos)*16;      // in bytes

    if ( offset >= ThreadCount)
        return;

    read(iIndex, offset, bWriteback);   // Linear buffer read
	write(oIndex, offset, bWriteback);	
}

const short init_0_15_[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//const short init_0_7_[8] = {0,1,2,3,4,5,6,7};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void GPU_memcpy_Scattered(SurfaceIndex iIndex, SurfaceIndex oIndex, int ThreadCount)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    // Each thread process 16 int
//	vector<uint, 16> bWriteback;	
//    vector<uint, 16> EleOffset(init_0_15_);
//    uint GlobalOffset = (511 * v_pos + h_pos)*16;      // in DWord

    // Each thread process 8 int
	vector<uint, 8> bWriteback;	
    vector<uint, 8> EleOffset(init_0_7_);
    uint GlobalOffset = (511 * v_pos + h_pos)*8;      // in DWord

    if ( GlobalOffset >= ThreadCount)
        return;

    read(iIndex, GlobalOffset, EleOffset, bWriteback);   // Linear buffer read
	write(oIndex, GlobalOffset, EleOffset, bWriteback);	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void Float_Op(SurfaceIndex iIndex, SurfaceIndex oIndex, int ThreadCount)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    // Each thread process min 16 bytes, max 128 bytes
	vector<uchar, 128> bWriteback;	            // 16 OWORD
	vector_ref <float, 32> fData = bWriteback.format<float>();
	vector_ref <int, 32> dData = bWriteback.format<int>();
	vector_ref <short, 64> sData = bWriteback.format<short>();

    uint offset = (511 * v_pos + h_pos)*128;      // in bytes

    if ( offset >= ThreadCount)
        return;

//    read(iIndex, offset, bWriteback);   // Linear buffer read

	for (int i = 0; i < 10000; i++) {
/*
		dData.select<8,1>(0) += dData.select<8,1>(0);
		dData.select<8,1>(8) += dData.select<8,1>(8);
		dData.select<8,1>(16) += dData.select<8,1>(16);
		dData.select<8,1>(24) += dData.select<8,1>(24);
*/
		//fData += fData;
		dData += dData;
		//sData += sData;

	}

//	write(oIndex, offset, bWriteback);	
}

/////////////////////////////////////////////////////////////////////////////////////////
// This function does flipX to output surface.  This function supports all image widths.
extern "C" _GENX_MAIN_ void FlipX_DualSurface(SurfaceIndex iIndex, SurfaceIndex oIndex, 
											int WidthInByte, int ThreadCount, vector<uchar,32> ReverseIdx)
{
	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();

	int ThreadID = 511 * v_pos + h_pos;
	if (ThreadID >= ThreadCount)
		return;							// Skip all threads beyond thread count

	vector<int, 32> InBuffer;
	vector<int, 32> OutBuffer;

	// The offset for read of 128 bytes
    uint offset = ThreadID * 128;		
    read(iIndex, offset, InBuffer);

	OutBuffer = InBuffer.iselect(ReverseIdx);

	// Current origin of 128 bytes at (x,y)
	int x = offset % WidthInByte;
	int y = offset / WidthInByte;
	// New origin of 128 bytes at (x',y)
	x = (WidthInByte-128) - x;
	// New offset for write
	offset = y * WidthInByte + x;

    write(oIndex, offset, OutBuffer);
}

#ifdef NOT_USED
//////////////////////////////////////////////////////////////////////////////////////////
// This function does in-place flipX. This function assumes the surface width is even # of blocks (multiples of 16)
extern "C" _GENX_MAIN_ void FlipX_SingleSurface(SurfaceIndex iIndex, int width)
{
	int h_pos = get_thread_origin_x();
	int v_pos = get_thread_origin_y();

    // Abort if h_pos is on the right half of image
    if (h_pos*8 >= width/2)
        return;

	matrix<int, 8, 8> nLeft;
	matrix<int, 8, 8> nRight;
	matrix<int, 8, 8> nOutput;
    matrix<int, 8, 8> nTemp;

	int nX = 8*h_pos;  // 8x8 pixels
	int nY = 8*v_pos;

    int readX = width - (h_pos + 1)*8;
    int diff = readX - nX;

    // Sawp block pair
	read(iIndex, nX*4, nY, nLeft);
	read(iIndex, readX*4, nY, nRight);

    // left to right
    nTemp.select<8,1,4,1>(0,4) = nLeft.select<8,1,4,1>(0,0);
    nTemp.select<8,1,4,1>(0,0) = nLeft.select<8,1,4,1>(0,4);

    nOutput.select<8,1,2,4>(0,3) = nTemp.select<8,1,2,4>(0,0);
    nOutput.select<8,1,2,4>(0,0) = nTemp.select<8,1,2,4>(0,3);

    nOutput.select<8,1,2,4>(0,2) = nTemp.select<8,1,2,4>(0,1);
    nOutput.select<8,1,2,4>(0,1) = nTemp.select<8,1,2,4>(0,2);

    write(iIndex, readX*4, nY, nOutput);	

    // Right to left
    nTemp.select<8,1,4,1>(0,4) = nRight.select<8,1,4,1>(0,0);
    nTemp.select<8,1,4,1>(0,0) = nRight.select<8,1,4,1>(0,4);

    nOutput.select<8,1,2,4>(0,3) = nTemp.select<8,1,2,4>(0,0);
    nOutput.select<8,1,2,4>(0,0) = nTemp.select<8,1,2,4>(0,3);

    nOutput.select<8,1,2,4>(0,2) = nTemp.select<8,1,2,4>(0,1);
    nOutput.select<8,1,2,4>(0,1) = nTemp.select<8,1,2,4>(0,2);

	write(iIndex, nX*4, nY, nOutput);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void RGBA2BGRA(SurfaceIndex iIndex)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<uint, 8, 8> nColor;
	matrix_ref<uchar, 8, 32> bColor = nColor.format<uchar,8,32>();
    matrix<uchar, 8, 8> bTemp;

	uint nX = 8*h_pos * 4;  // 8x8 pixels, RGBA 4 bytes per pixel
	uint nY = 8*v_pos;

	read(iIndex, nX, nY, nColor);

    bTemp = bColor.select<8,1,8,4>(0,0);
    bColor.select<8,1,8,4>(0,0) = bColor.select<8,1,8,4>(0,2);
    bColor.select<8,1,8,4>(0,2) = bTemp;

    write(iIndex, nX, nY, nColor);	
}

//////////////////////////////////////////////////////////////////////////////////////////
/*
extern "C" _GENX_MAIN_ void FlipX_RGBA2BGRA(SurfaceIndex iIndex, SurfaceIndex oIndex, int MaxX)
{
	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

	matrix<uint, 8, 8> nInput;
	matrix_ref<uchar, 8, 32> bInput = nInput.format<uchar,8,32>();
    matrix<uint, 8, 8> nOutput;
    matrix<uint, 8, 8> nTemp;
    matrix<uchar, 8, 8> bTemp;

	uint nX = 8*h_pos * 4;  // 8x8 pixels, RGBA 4 bytes per pixel
	uint nY = 8*v_pos;

	read(iIndex, nX, nY, nInput);

    // RGBA to BGRA
    bTemp = bInput.select<8,1,8,4>(0,0);
    bInput.select<8,1,8,4>(0,0) = bInput.select<8,1,8,4>(0,2);
    bInput.select<8,1,8,4>(0,2) = bTemp;

    // Flip x
    nTemp.select<8,1,4,1>(0,4) = nInput.select<8,1,4,1>(0,0);
    nTemp.select<8,1,4,1>(0,0) = nInput.select<8,1,4,1>(0,4);

    nOutput.select<8,1,2,4>(0,3) = nTemp.select<8,1,2,4>(0,0);
    nOutput.select<8,1,2,4>(0,0) = nTemp.select<8,1,2,4>(0,3);

    nOutput.select<8,1,2,4>(0,2) = nTemp.select<8,1,2,4>(0,1);
    nOutput.select<8,1,2,4>(0,1) = nTemp.select<8,1,2,4>(0,2);

    write(oIndex, 8*(MaxX-1)*4 - nX, nY, nOutput);	
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void StaticBufferTest(SurfaceIndex iIndex, 
											SurfaceIndex oIndex,
											SurfaceIndex oIndex2,
											int ThreadCount)
{

	uint h_pos = get_thread_origin_x();
	uint v_pos = get_thread_origin_y();

    // Each thread process 512 bytes
	vector<uchar, 128> bWriteback;

    uint offset = (511 * v_pos + h_pos);      // Thread offset

    if ( offset >= ThreadCount)
        return;
	
	offset *= 512;							// in bytes, process 512 bytes per thread

	for (int i = 0; i < 512; i += 128) {
//*
		read(iIndex, offset, bWriteback);   // Linear buffer read
		write(oIndex, offset, bWriteback);	
		write(oIndex2, offset, bWriteback);	
//*/
/*
		read(CM_STATIC_BUFFER_0, offset+i, bWriteback);   // Linear buffer read
		write(CM_STATIC_BUFFER_1, offset+i, bWriteback);	
		write(CM_STATIC_BUFFER_2, offset+i, bWriteback);	
//*/
	}
}

