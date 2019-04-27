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

#define X 0
#define Y 1

#define BLOCK_HEIGHT    16

#define ROW0	0
#define ROW1	1
#define ROW2	2
#define ROW3	3
#define ROW4	4
#define ROW5	5
#define ROW6	6

#define COL0	0
#define COL1	1
#define COL2	2
#define COL3	3
#define COL4	4
#define COL5	5
#define COL6	6

#define SUM_B	0
#define SUM_C	1
#define SUM_D	2
#define SUM_E	3
#define SUM_F	4
#define SUM_G	5
#define SUM_H	6
#define SUM_I	7
#define SUM_J	8

inline _GENX_ void BgSupress_GENX(
      SurfaceIndex LUTSI,
      matrix_ref<uchar, 16, 16>  inputL,
      matrix_ref<uchar, 16, 16>  inputA,
      matrix_ref<uchar, 16, 16>  inputB,
      matrix_ref<uchar, 16, 16>  outL,
      matrix_ref<uchar, 16, 16>  outA,
      matrix_ref<uchar, 16, 16>  outB
      )
{
   // Initialize the required parameters
   int L_thresh      = 230;

   int a_low         = 116;
   int a_high        = 140;

   int b_low         = 116;
   int b_high        = 140;

   int whitepointL   = 255;
   int whitepointa   = 128;
   int whitepointb   = 128;

   int L_low_thresh  = 70;
   int neutral       = 128;
   int plusminus     = 10;

   int neutral_plus_plusminus  = neutral + plusminus;
   int neutral_minus_plusminus = neutral - plusminus;


   // L channel
   matrix<uchar, 16, 16> tmpL;

   tmpL = cm_max<uchar>(inputL, L_thresh);
   matrix<uchar, 16, 16> block0 = (inputL >= tmpL);  // inputL[] >= L_thresh

   tmpL = cm_min<uchar>(inputL, L_low_thresh);
   matrix<uchar, 16, 16> block1 = (inputL <= tmpL);  // inputL[] <= L_low_thresh 

   // A channel
   matrix<uchar, 16, 16> tmpA;
   tmpA = cm_min<uchar>(inputA, a_high);
   block0 = block0 & (inputA <= tmpA);     // inputA[] <= a_high

   tmpA = cm_max<uchar>(inputA, a_low);
   block0 = block0 & (inputA >= tmpA);     // inputA[] >= a_low

   tmpA = cm_min<uchar>(inputA, neutral_plus_plusminus);
   block1 = block1 & (inputA <= tmpA);     // inputA[] <= (neutral + plusminus)

   tmpA = cm_max<uchar>(inputA, neutral_minus_plusminus);
   block1 = block1 & (inputA >= tmpA);     // inputA[] >= (neutral - plusminus)

   // B channel
   matrix<uchar, 16, 16> tmpB;
   tmpB = cm_min<uchar>(inputB, b_high);
   block0 = block0 & (inputB <= tmpB);     // inputB <= b_high

   tmpB = cm_max<uchar>(inputB, b_low);
   block0 = block0 & (inputB >= tmpB);     // inputB[] >= b_low

   tmpB = cm_min<uchar>(inputB, neutral_plus_plusminus);
   block1 = block1 & (inputB <= tmpB);     // inputB[] <= (neutral + plusminus)

   tmpB = cm_max<uchar>(inputB, neutral_minus_plusminus);
   block1 = block1 & (inputB >= tmpB);     // inputB[] >= (neutral - plusminus)

   outL.merge(whitepointL, inputL, block0);

   vector<uint, 16*16> inputIndex = vector<uint, 16*16>(outL);
   vector<uchar, 16*16> lightsourceout;

   //lightness contrast
   inputIndex = matrix<uint, 16, 16>(outL);
#pragma unroll
   for (int i=0; i<16; i++)
      read(LUTSI, 0 ,inputIndex.select<16,1>(i*16), lightsourceout.select<16,1>(i*16));

   outL = lightsourceout;

   outA = (((block0 << 7) & whitepointa) | ((block1 << 7) & neutral));
   outB = (((block0 << 7) & whitepointb) | ((block1 << 7) & neutral));

   matrix<uchar, 16, 16> block0_or_block1 = ((block0 | block1) * 0xff);
   matrix<uchar, 16, 16> tt;

   tt = (~(block0_or_block1) & inputA);

   outA = (outA | tt );

   tt = (~(block0_or_block1) & inputB);

   outB = (outB | tt );

}

//
//  Symmetric 7x7 filter for picture in U8 pixel format.
//
//////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
SyBgLc_GENX(
       SurfaceIndex SrcLSI,                // Source picture surface index
       SurfaceIndex SrcASI,                // Source picture surface index
       SurfaceIndex SrcBSI,                // Source picture surface index
       SurfaceIndex DstLSI,                // Destination picture surface index
       SurfaceIndex DstASI,                // Destination picture surface index
       SurfaceIndex DstBSI,                // Destination picture surface index
       SurfaceIndex LUTSI,
       vector<short, 10> Filter_Coeffs,   // 10 unique filter coefficients
       int nRightShift                    //
     )
{
   vector<short, 2>        SrcXY;      // Pixel block origin
   matrix<uchar, 22, 32>   SrcData;    // Actually use 22x22 for output of 16x16 block
   matrix<short, 22, 16>   Subsum0;    // Temp buffer for column0_6 subsum results
   matrix<short, 22, 16>   Subsum1;    // Temp buffer for column1_5 subsum results
   matrix<short, 22, 16>   Subsum2;    // Temp buffer for column2_4 subsum results
   matrix<short,  9, 16>   finalsum;   // Temp buffer for final sum results of each output row (for coefficients B-J)
   vector<int, 16>         Temp_Filter;// Temp buffer for sum of coefficients multiplication
   matrix<uchar, 16, 16> DstData;      // Output data buffer
   short sRightShift;

   sRightShift = nRightShift;

   // Load source pixel block 22x32
   SrcXY(X) = cm_local_id(X) + cm_group_id(X)*cm_local_size(X);
   SrcXY(Y) = cm_local_id(Y) + cm_group_id(Y)*cm_local_size(Y);  // Location of the processing pixel block 

   SrcXY = (SrcXY << 4);     // Pixel block size of 16x16
   read(SrcLSI, SrcXY(X)-3, SrcXY(Y)-3, SrcData.select<8, 1, 32, 1>(0,0));
   read(SrcLSI, SrcXY(X)-3, SrcXY(Y)+5, SrcData.select<8, 1, 32, 1>(8,0));
   read(SrcLSI, SrcXY(X)-3, SrcXY(Y)+13, SrcData.select<6, 1, 32, 1>(16,0));

   // Generate partial sum of symmetric columns
   Subsum0 = SrcData.select<22, 1, 16, 1>(ROW0, COL0) + SrcData.select<22, 1, 16, 1>(ROW0, COL6);	// Column 0 + 6
   Subsum1 = SrcData.select<22, 1, 16, 1>(ROW0, COL1) + SrcData.select<22, 1, 16, 1>(ROW0, COL5);	// Column 1 + 5
   Subsum2 = SrcData.select<22, 1, 16, 1>(ROW0, COL2) + SrcData.select<22, 1, 16, 1>(ROW0, COL4);	// Column 2 + 4

   vector_ref<short, 9> Filter_Coeffs_B(Filter_Coeffs.select<9, 1>(1));	// Map to coefficient B

   // Loop through each row for final sum and filtering
#pragma unroll
   for (int nRow = 0; nRow < BLOCK_HEIGHT; nRow++)
   {
      // Compute final sum of pixels for each coefficient

      // First compute sub-sum of current pixel column for (B, D, G)
      finalsum.row(SUM_B) = SrcData.select<1, 1, 16, 1>(nRow + ROW2, COL3) + SrcData.select<1, 1, 16, 1>(nRow + ROW4, COL3);
      finalsum.row(SUM_D) = SrcData.select<1, 1, 16, 1>(nRow + ROW1, COL3) + SrcData.select<1, 1, 16, 1>(nRow + ROW5, COL3);
      finalsum.row(SUM_G) = SrcData.select<1, 1, 16, 1>(nRow + ROW0, COL3) + SrcData.select<1, 1, 16, 1>(nRow + ROW6, COL3);

      // Total sums
      finalsum.row(SUM_B) = finalsum.row(SUM_B) + Subsum2.row(nRow + 3);
      finalsum.row(SUM_C) = Subsum2.row(nRow + 2) + Subsum2.row(nRow + 4);
      finalsum.row(SUM_D) = finalsum.row(SUM_D) + Subsum1.row(nRow + 3);
#if 1
      finalsum.row(SUM_E) = Subsum1.row(nRow + 2) + Subsum1.row(nRow + 4) + Subsum2.row(nRow + 1) + Subsum2.row(nRow + 5);
#else
      finalsum.row(SUM_E) = Subsum1.row(nRow + 2) + Subsum1.row(nRow + 4);
      finalsum.row(SUM_E) += Subsum2.row(nRow + 1);
      finalsum.row(SUM_E) += Subsum2.row(nRow + 5);
#endif
      finalsum.row(SUM_F) = Subsum1.row(nRow + 1) + Subsum1.row(nRow + 5);
      finalsum.row(SUM_G) = finalsum.row(SUM_G) + Subsum0.row(nRow + 3);
#if 1
      finalsum.row(SUM_H) = Subsum0.row(nRow + 2) + Subsum0.row(nRow + 4) + Subsum2.row(nRow + 0) + Subsum2.row(nRow + 6);
      finalsum.row(SUM_I) = Subsum0.row(nRow + 1) + Subsum0.row(nRow + 5) + Subsum1.row(nRow + 0) + Subsum1.row(nRow + 6);
#else
      finalsum.row(SUM_H) = Subsum0.row(nRow + 2) + Subsum0.row(nRow + 4);
      finalsum.row(SUM_H) += Subsum2.row(nRow + 0);
      finalsum.row(SUM_H) += Subsum2.row(nRow + 6);
      finalsum.row(SUM_I) = Subsum0.row(nRow + 1) + Subsum0.row(nRow + 5);
      finalsum.row(SUM_I) += Subsum1.row(nRow + 0);
      finalsum.row(SUM_I) += Subsum1.row(nRow + 6);
#endif
      finalsum.row(SUM_J) = Subsum0.row(nRow + 0) + Subsum0.row(nRow + 6);

      // Multiplication with filter coefficients
      Temp_Filter = Filter_Coeffs(0) * SrcData.select<1, 1, 16, 1>(nRow + ROW3, COL3);	// * A
#pragma unroll
      for (int nCoeff = 0; nCoeff < 9; nCoeff++)
         Temp_Filter += Filter_Coeffs_B(nCoeff) * finalsum.row(nCoeff);
      DstData.row(nRow) = cm_shr<uchar>(Temp_Filter, sRightShift, SAT);
   }

   matrix<uchar, 16, 16> inA;
   matrix<uchar, 16, 16> inB;
   matrix<uchar, 16, 16> outL;
   matrix<uchar, 16, 16> outA;
   matrix<uchar, 16, 16> outB;

   read(SrcASI, SrcXY(X), SrcXY(Y), inA);
   read(SrcBSI, SrcXY(X), SrcXY(Y), inB);

   BgSupress_GENX(LUTSI, DstData, inA, inB, outL, outA, outB);

   // Output final filtered pixel block
   write(DstLSI, SrcXY(X), SrcXY(Y), outL);
   write(DstASI, SrcXY(X), SrcXY(Y), outA);
   write(DstBSI, SrcXY(X), SrcXY(Y), outB);
}
