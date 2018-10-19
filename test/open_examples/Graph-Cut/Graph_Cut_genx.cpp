/*
 * Copyright (c) 2018, Intel Corporation
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

const short init_0_7_[8] = {0, 1, 2, 3, 4, 5, 6, 7};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Traspose Y*X = 8x16 to 16x8 short
_GENX_ inline void Transpose_8x16_To_16x8_Short(matrix_ref<short, 8, 16> Src,
                                                matrix_ref<short, 16, 8> Dest) {
  matrix<short, 8, 16> sTemp;

  sTemp.select<2, 1, 16, 1>(0, 0) = Src.select<8, 1, 4, 4>(0, 0);
  sTemp.select<2, 1, 16, 1>(2, 0) = Src.select<8, 1, 4, 4>(0, 1);
  sTemp.select<2, 1, 16, 1>(4, 0) = Src.select<8, 1, 4, 4>(0, 2);
  sTemp.select<2, 1, 16, 1>(6, 0) = Src.select<8, 1, 4, 4>(0, 3);

  Dest.select<4, 1, 8, 1>(0, 0) = sTemp.select<8, 1, 4, 4>(0, 0);
  Dest.select<4, 1, 8, 1>(4, 0) = sTemp.select<8, 1, 4, 4>(0, 1);
  Dest.select<4, 1, 8, 1>(8, 0) = sTemp.select<8, 1, 4, 4>(0, 2);
  Dest.select<4, 1, 8, 1>(12, 0) = sTemp.select<8, 1, 4, 4>(0, 3);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Traspose Y*X = 16x8 to 8x16 short
_GENX_ inline void Transpose_16x8_To_8x16_Short(matrix_ref<short, 16, 8> Src,
                                                matrix_ref<short, 8, 16> Dest) {
  matrix<short, 8, 16> sTemp;

  sTemp.select<2, 1, 16, 1>(0, 0) = Src.select<16, 1, 2, 4>(0, 0);
  sTemp.select<2, 1, 16, 1>(2, 0) = Src.select<16, 1, 2, 4>(0, 1);
  sTemp.select<2, 1, 16, 1>(4, 0) = Src.select<16, 1, 2, 4>(0, 2);
  sTemp.select<2, 1, 16, 1>(6, 0) = Src.select<16, 1, 2, 4>(0, 3);

  Dest.select<4, 1, 16, 1>(0, 0) = sTemp.select<8, 1, 8, 2>(0, 0);
  Dest.select<4, 1, 16, 1>(4, 0) = sTemp.select<8, 1, 8, 2>(0, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Traspose Y*X = 8x16 to 16x8 short
_GENX_ inline void
Transpose_8x16_To_16x8_UShort(matrix_ref<ushort, 8, 16> Src,
                              matrix_ref<ushort, 16, 8> Dest) {
  matrix<ushort, 8, 16> sTemp;

  sTemp.select<2, 1, 16, 1>(0, 0) = Src.select<8, 1, 4, 4>(0, 0);
  sTemp.select<2, 1, 16, 1>(2, 0) = Src.select<8, 1, 4, 4>(0, 1);
  sTemp.select<2, 1, 16, 1>(4, 0) = Src.select<8, 1, 4, 4>(0, 2);
  sTemp.select<2, 1, 16, 1>(6, 0) = Src.select<8, 1, 4, 4>(0, 3);

  Dest.select<4, 1, 8, 1>(0, 0) = sTemp.select<8, 1, 4, 4>(0, 0);
  Dest.select<4, 1, 8, 1>(4, 0) = sTemp.select<8, 1, 4, 4>(0, 1);
  Dest.select<4, 1, 8, 1>(8, 0) = sTemp.select<8, 1, 4, 4>(0, 2);
  Dest.select<4, 1, 8, 1>(12, 0) = sTemp.select<8, 1, 4, 4>(0, 3);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Traspose Y*X = 8x16 to 16x8 uint
_GENX_ inline void Transpose_8x16_To_16x8_Uint(matrix_ref<uint, 8, 16> Src,
                                               matrix_ref<uint, 16, 8> Dest) {
  matrix<uint, 8, 16> uTemp;

  uTemp.select<2, 1, 16, 1>(0, 0) = Src.select<8, 1, 4, 4>(0, 0);
  uTemp.select<2, 1, 16, 1>(2, 0) = Src.select<8, 1, 4, 4>(0, 1);
  uTemp.select<2, 1, 16, 1>(4, 0) = Src.select<8, 1, 4, 4>(0, 2);
  uTemp.select<2, 1, 16, 1>(6, 0) = Src.select<8, 1, 4, 4>(0, 3);

  Dest.select<4, 1, 8, 1>(0, 0) = uTemp.select<8, 1, 4, 4>(0, 0);
  Dest.select<4, 1, 8, 1>(4, 0) = uTemp.select<8, 1, 4, 4>(0, 1);
  Dest.select<4, 1, 8, 1>(8, 0) = uTemp.select<8, 1, 4, 4>(0, 2);
  Dest.select<4, 1, 8, 1>(12, 0) = uTemp.select<8, 1, 4, 4>(0, 3);
}

#define BLOCK_INT 1
#define BLOCK_SHORT 1

#ifdef BLOCK_INT
// Block 16x16
////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void GC_Init_Height_u32(SurfaceIndex pHeightIndex,
                                               uint HEIGHT_MAX) {
  matrix<uint, 16, 16> Height;

  uint h_pos = get_thread_origin_x();
  uint v_pos = get_thread_origin_y();

  int dX = 16 * h_pos * sizeof(uint); // in bytes
  int dY = 16 * v_pos;                // in rows

  read(pHeightIndex, dX, dY, Height.select<8, 1, 8, 1>(0, 0));
  read(pHeightIndex, dX + 32, dY, Height.select<8, 1, 8, 1>(0, 8));
  read(pHeightIndex, dX, dY + 8, Height.select<8, 1, 8, 1>(8, 0));
  read(pHeightIndex, dX + 32, dY + 8, Height.select<8, 1, 8, 1>(8, 8));

  Height.merge(HEIGHT_MAX, Height > 0);

  write(pHeightIndex, dX, dY, Height.select<8, 1, 8, 1>(0, 0));
  write(pHeightIndex, dX + 32, dY, Height.select<8, 1, 8, 1>(0, 8));
  write(pHeightIndex, dX, dY + 8, Height.select<8, 1, 8, 1>(8, 0));
  write(pHeightIndex, dX + 32, dY + 8, Height.select<8, 1, 8, 1>(8, 8));
}
#endif

#ifdef BLOCK_SHORT
// Block 16x16
////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void GC_Init_Height_u16(SurfaceIndex pHeightIndex,
                                               ushort HEIGHT_MAX) {
  matrix<ushort, 16, 16> Height;

  uint h_pos = get_thread_origin_x();
  uint v_pos = get_thread_origin_y();

  int dX = 16 * h_pos * sizeof(ushort); // in bytes
  int dY = 16 * v_pos;                  // in rows

  read(pHeightIndex, dX, dY, Height.select<8, 1, 16, 1>(0, 0));
  read(pHeightIndex, dX, dY + 8, Height.select<8, 1, 16, 1>(8, 0));

  Height.merge(HEIGHT_MAX, Height > 0);

  write(pHeightIndex, dX, dY, Height.select<8, 1, 16, 1>(0, 0));
  write(pHeightIndex, dX, dY + 8, Height.select<8, 1, 16, 1>(8, 0));
}
#endif

#ifdef BLOCK_INT
// Block 8x8
///////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_Create_BlockMask_u32(SurfaceIndex pExcessFlowIndex,
                        SurfaceIndex pHeightIndex, SurfaceIndex pBlockMaskIndex,
                        uint HEIGHT_MAX) {
  matrix<short, 8, 8> ExcessFlow;
  matrix<uint, 8, 8> Height;
  matrix<uchar, 1, 4> BlockMask;
  matrix<short, 8, 8> mask;

  uint h_pos = get_thread_origin_x();
  uint v_pos = get_thread_origin_y();

  int iX = 4 * 8 * h_pos * sizeof(uint); // 4 blocks width in bytes
  int iY = 8 * v_pos;                    // in rows

  int sX = 4 * 8 * h_pos * sizeof(ushort); // 4 blocks width in bytes
  int sY = 8 * v_pos;                      // in rows

  int bX = 4 * h_pos; // 4 bytes per write
  int bY = v_pos;

#pragma unroll
  for (int i = 0; i < 4; i++) { // process 4 8x8 blocks
    read(pHeightIndex, iX, iY, Height.select<8, 1, 8, 1>(0, 0));
    read(pExcessFlowIndex, sX, sY, ExcessFlow.select<8, 1, 8, 1>(0, 0));

    mask = 0;
    //        mask.merge(1, 0, (Height < HEIGHT_MAX) & (ExcessFlow > -100) );
    mask.merge(1, ExcessFlow > -100);
    mask.merge(0, Height == HEIGHT_MAX);
    BlockMask[0][i] = mask.any() ? 255 : 0;

    iX += 8 * sizeof(uint);   // To next block to the right
    sX += 8 * sizeof(ushort); // To next block to the right
  }

  write(pBlockMaskIndex, bX, bY, BlockMask);
}
#endif

#ifdef BLOCK_SHORT
// Block 8x8
///////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_Create_BlockMask_u16(SurfaceIndex pExcessFlowIndex,
                        SurfaceIndex pHeightIndex, SurfaceIndex pBlockMaskIndex,
                        ushort HEIGHT_MAX) {
  matrix<short, 8, 8> ExcessFlow;
  matrix<ushort, 8, 8> Height;
  matrix<uchar, 1, 4> BlockMask;
  matrix<short, 8, 8> mask(0);

  uint h_pos = get_thread_origin_x();
  uint v_pos = get_thread_origin_y();

  int sX = 4 * 8 * h_pos * sizeof(ushort); // 4 blocks width in bytes
  int sY = 8 * v_pos;                      // in rows
  int bX = 4 * h_pos;                      // 4 bytes per write
  int bY = v_pos;

#pragma unroll
  for (int i = 0; i < 4; i++) { // process 4 8x8 blocks
    read(pExcessFlowIndex, sX, sY, ExcessFlow.select<8, 1, 8, 1>(0, 0));
    read(pHeightIndex, sX, sY, Height.select<8, 1, 8, 1>(0, 0));

    mask = 0;
    //        mask.merge(1, 0, (Height < HEIGHT_MAX) & (ExcessFlow > -100) );
    mask.merge(1, ExcessFlow > -100);
    mask.merge(0, Height == HEIGHT_MAX);
    BlockMask[0][i] = mask.any() ? 255 : 0;

    sX += 8 * sizeof(ushort); // To next block to the right
  }

  write(pBlockMaskIndex, bX, bY, BlockMask);
}
#endif

#ifdef BLOCK_INT
// Block 8x8
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_Relabel_u32(SurfaceIndex pBlockMaskIndex, SurfaceIndex pExcessFlowIndex,
               SurfaceIndex pHeightIndex, SurfaceIndex pWestCapIndex,
               SurfaceIndex pNorthCapIndex, SurfaceIndex pEastCapIndex,
               SurfaceIndex pSouthCapIndex, uint HEIGHT_MAX) {
  matrix<uchar, 1, 1> BlockMask;
  matrix<uint, 10, 8> Height10x8; // 10 GRFs
  matrix<uint, 8, 1> LBorder;
  matrix<uint, 8, 1> RBorder;

  matrix<short, 8, 8> ExcessFlow; // 4 GRFs
  matrix<short, 8, 8> WestCap;    // 4 GRFs
  matrix<short, 8, 8> NorthCap;   // 4 GRFs
  matrix<short, 8, 8> EastCap;    // 4 GRFs
  matrix<short, 8, 8> SouthCap;   // 4 GRFs

  matrix<short, 8, 8> mask;  // 4 GRFs
  matrix<uint, 8, 8> mask2;  // 8 GRFs
  vector<uint, 8> NewHeight; // 1 GRF
  vector<uint, 8> Neighbor;  // 1 GRF
  vector<uint, 2> temps;

  vector<uint, 8> Test;
  vector<uint, 8> Test2;

  uint h_pos = get_thread_origin_x();
  uint v_pos = get_thread_origin_y();

  // Skip inactive block
  read(pBlockMaskIndex, h_pos, v_pos, BlockMask);
  if (BlockMask[0][0] == 0)
    return;

  short baseX = 1; // block base
  short baseY = 1; // Offset for extra row

  // Height block origin without border
  int dX0 = 8 * h_pos * sizeof(uint); // in bytes
  int dY0 = 8 * v_pos;                // in rows

  // Height block origin with border
  int dX = (8 * h_pos - baseX) * sizeof(uint); // in bytes
  int dY = 8 * v_pos - baseY;                  // in rows

  // ExcessFlow, WestCap, NorthCap, EastCap, SouthCap origin without border
  int sX = 8 * h_pos * 2; // in bytes
  int sY = 8 * v_pos;     // in rows

  read(pExcessFlowIndex, sX, sY, ExcessFlow);
  mask.merge(1, 0, ExcessFlow > 0);
  //    if (!mask.any())
  //        return;

  // Read Height
  read(pHeightIndex, dX0, dY, Height10x8.select<8, 1, 8, 1>(0, 0));
  read(pHeightIndex, dX0, dY + 8, Height10x8.select<2, 1, 8, 1>(8, 0));
  // Read left and right Height borders
  read(pHeightIndex, dX, dY0, LBorder);
  read(pHeightIndex, dX + 9 * sizeof(uint), dY0, RBorder);

  mask.merge(1, 0, ExcessFlow > 0);

  //    mask2.merge(1, 0, Height10x8.select<8,1,8,1>(baseY,0) == HEIGHT_MAX);
  //    if (mask2.all())
  //        return;

  read(pWestCapIndex, sX, sY, WestCap);
  read(pNorthCapIndex, sX, sY, NorthCap);
  read(pEastCapIndex, sX, sY, EastCap);
  read(pSouthCapIndex, sX, sY, SouthCap);

#pragma unroll
  for (int j = 0; j < 8; j++) {
    if (mask.row(j).any()) {
      NewHeight = HEIGHT_MAX;

      // North neighbour: x, y-1
      Neighbor = Height10x8.row(baseY - 1 + j) + 1;
      NewHeight.merge(Neighbor, mask.row(j) & (NorthCap.row(j) > 0) &
                                    (Neighbor < NewHeight));

      // South neighbour: x, y+1
      Neighbor = Height10x8.row(baseY + 1 + j) + 1;
      NewHeight.merge(Neighbor, mask.row(j) & (SouthCap.row(j) > 0) &
                                    (Neighbor < NewHeight));

      // East neighbour: x+1, y
      Neighbor.select<4, 1>(0) = Height10x8.select<1, 1, 4, 1>(baseY + j, 1);
      Neighbor.select<4, 1>(3) = Height10x8.select<1, 1, 4, 1>(baseY + j, 4);
      Neighbor[7] = RBorder[j][0];
      Neighbor += 1;
      NewHeight.merge(Neighbor, mask.row(j) & (EastCap.row(j) > 0) &
                                    (Neighbor < NewHeight));

      // West pix[0]
      if (mask[j][0]) {
        // West neighbour
        temps[0] = LBorder[j][0] + 1;
        if ((WestCap[j][0] > 0) && (temps[0] < NewHeight[0]))
          NewHeight[0] = temps[0];
        // Update Height so the next pixel to the right will use this new value
        Height10x8[baseY + j][0] = NewHeight[0];
      }

      // West pix[1:7]
#pragma unroll
      for (int i = 1; i < 8; i++) {
        if (mask[j][i]) {
          //                    temps = Height10x8.select<1,1,2,2>(baseY+j,
          //                    -1+i) + 1;
          temps[0] = Height10x8[baseY + j][-1 + i] + 1;
          // West neighbour
          if ((WestCap[j][i] > 0) && (temps[0] < NewHeight[i]))
            NewHeight[i] = temps[0];
          Height10x8[baseY + j][i] = NewHeight[i];
        }
      }
    }
  }

  // Output the updated height block 8x8
  write(pHeightIndex, 8 * h_pos * sizeof(uint), 8 * v_pos,
        Height10x8.select<8, 1, 8, 1>(1, 0));
  //    cm_fence();
}
#endif

#ifdef BLOCK_SHORT
// Block 16x16
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_Relabel_u16(SurfaceIndex pBlockMaskIndex, SurfaceIndex pExcessFlowIndex,
               SurfaceIndex pHeightIndex, SurfaceIndex pWestCapIndex,
               SurfaceIndex pNorthCapIndex, SurfaceIndex pEastCapIndex,
               SurfaceIndex pSouthCapIndex, ushort HEIGHT_MAX) {
  matrix<uchar, 2, 2> BlockMask;
  matrix<ushort, 10, 16> Height10x16; // 10 GRFs
  matrix<ushort, 8, 1> LBorder;       // 0.5 GRFs
  matrix<ushort, 8, 1> RBorder;       // 0.5 GRFs

  matrix<short, 8, 16> ExcessFlow; // 8 GRFs
  matrix<short, 8, 16> WestCap;    // 8 GRFs
  matrix<short, 8, 16> NorthCap;   // 8 GRFs
  matrix<short, 8, 16> EastCap;    // 8 GRFs
  matrix<short, 8, 16> SouthCap;   // 8 GRFs

  matrix<short, 8, 16> mask;    // 8 GRFs
  matrix<ushort, 8, 16> mask2;  // 8 GRFs
  vector<ushort, 16> NewHeight; // 1 GRF
  vector<ushort, 16> Neighbor;  // 1 GRF
  vector<ushort, 2> temps;

  matrix<ushort, 1, 2> sOutput;
  matrix<ushort, 1, 2> uOutput;
  vector<short, 16> sMaxFlow;
  vector<ushort, 16> uMinHeight;
  matrix<short, 16, 16> ExcessFlow16x16;

  uint h_pos = get_thread_origin_x();
  uint v_pos = get_thread_origin_y();

  short baseX = 1; // block base
  short baseY = 1; // Offset for extra row

  read(pBlockMaskIndex, 2 * h_pos, 2 * v_pos, BlockMask);

  for (int hb = 0; hb < 16;
       hb +=
       8) { // Process top half and bottom half block due to GRF size limit

    // Skip inactive block
    if (BlockMask[hb >> 3][0] == 0 & BlockMask[hb >> 3][1] == 0)
      continue;

    // Height block origin without border
    int dX0 = 16 * h_pos * sizeof(ushort); // in bytes
    int dY0 = 16 * v_pos + hb;             // in rows

    // Height block origin with border
    int dX = (16 * h_pos - baseX) * sizeof(ushort); // in bytes
    int dY = (16 * v_pos - baseY) + hb;             // in rows

    // ExcessFlow, WestCap, NorthCap, EastCap, SouthCap origin without border
    int sX = 16 * h_pos * sizeof(short); // in bytes
    int sY = 16 * v_pos + hb;            // in rows

    read(pExcessFlowIndex, sX, sY, ExcessFlow);

    // Read Height
    read(pHeightIndex, dX0, dY, Height10x16.select<8, 1, 16, 1>(0, 0));
    read(pHeightIndex, dX0, dY + 8, Height10x16.select<2, 1, 16, 1>(8, 0));

    // Read left and right Height borders
    read(pHeightIndex, dX, dY0, LBorder);
    read(pHeightIndex, dX + 17 * sizeof(ushort), dY0, RBorder);

    mask.merge(1, 0, ExcessFlow > 0);

    read(pWestCapIndex, sX, sY, WestCap);
    read(pNorthCapIndex, sX, sY, NorthCap);
    read(pEastCapIndex, sX, sY, EastCap);
    read(pSouthCapIndex, sX, sY, SouthCap);

#pragma unroll
    for (int j = 0; j < 8; j++) {
      if (mask.row(j).any()) {
        NewHeight = HEIGHT_MAX;

        // North neighbour: x, y-1
        Neighbor = Height10x16.row(baseY - 1 + j) + 1;
        NewHeight.merge(Neighbor, mask.row(j) & (NorthCap.row(j) > 0) &
                                      (Neighbor < NewHeight));

        // South neighbour: x, y+1
        Neighbor = Height10x16.row(baseY + 1 + j) + 1;
        NewHeight.merge(Neighbor, mask.row(j) & (SouthCap.row(j) > 0) &
                                      (Neighbor < NewHeight));

        // East neighbour: x+1, y
        Neighbor.select<8, 1>(0) = Height10x16.select<1, 1, 8, 1>(baseY + j, 1);
        Neighbor.select<8, 1>(7) = Height10x16.select<1, 1, 8, 1>(baseY + j, 8);
        Neighbor[15] = RBorder[j][0];
        Neighbor += 1;
        NewHeight.merge(Neighbor, mask.row(j) & (EastCap.row(j) > 0) &
                                      (Neighbor < NewHeight));

        // West for pix[0]
        if (mask[j][0]) {
          temps[0] = LBorder[j][0] + 1;
          if ((WestCap[j][0] > 0) && (temps[0] < NewHeight[0]))
            NewHeight[0] = temps[0];
          Height10x16[baseY + j][0] =
              NewHeight[0]; // Update Height so the next pixel to the right will
                            // use this new value
        }

        // West for pix[1:14]
#pragma unroll
        for (int i = 1; i < 16; i++) {
          if (mask[j][i]) {
            temps[0] = Height10x16[baseY + j][-1 + i] + 1;
            if ((WestCap[j][i] > 0) && (temps[0] < NewHeight[i]))
              NewHeight[i] = temps[0];
            Height10x16[baseY + j][i] = NewHeight[i];
          }
        }
      }
    } // j

    // Output the updated height block 8x16
    write(pHeightIndex, dX0, dY0, Height10x16.select<8, 1, 16, 1>(1, 0));

  } // hb
}
#endif

#ifdef BLOCK_INT
// BLOCK 8x8
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void GC_Global_Relabel_NR_u32(
    SurfaceIndex pBlockMaskIndex, SurfaceIndex pHeightIndex,
    SurfaceIndex pWestCapIndex, SurfaceIndex pNorthCapIndex,
    SurfaceIndex pEastCapIndex, SurfaceIndex pSouthCapIndex) {
  matrix<uchar, 1, 1> BlockMask;
  matrix<uint, 10, 8> Height10x8; // 10 GRFs
  matrix<uint, 8, 1> LBorder;
  matrix<uint, 8, 1> RBorder;
  matrix<short, 8, 8> WestCap;
  matrix<short, 8, 8> NorthCap;
  matrix<short, 8, 8> EastCap;
  matrix<short, 8, 8> SouthCap;
  vector<uint, 8> Neighbor;
  vector<uint, 2> temps;

  uint h_pos = get_thread_origin_x();
  uint v_pos = get_thread_origin_y();

  // Skip inactive block
  read(pBlockMaskIndex, h_pos, v_pos, BlockMask);
  if (BlockMask[0][0] == 0)
    return;

  short baseX = 1; // block base
  short baseY = 1; // Offset for extra row to include neighbors

  // Height block origin without border
  int dX0 = 8 * h_pos * sizeof(uint); // in bytes
  int dY0 = 8 * v_pos;                // in rows

  int dX = (8 * h_pos - baseX) * sizeof(uint); // in bytes
  int dY = 8 * v_pos - baseY;                  // in rows
  int sX = 8 * h_pos * 2;
  int sY = 8 * v_pos;

  // Read Height
  read(pHeightIndex, dX0, dY, Height10x8.select<8, 1, 8, 1>(0, 0));
  read(pHeightIndex, dX0, dY + 8, Height10x8.select<2, 1, 8, 1>(8, 0));
  // Read left and right Height borders
  read(pHeightIndex, dX, dY0, LBorder);
  read(pHeightIndex, dX + 9 * sizeof(uint), dY0, RBorder);

  read(pWestCapIndex, sX, sY, WestCap);
  read(pNorthCapIndex, sX, sY, NorthCap);
  read(pEastCapIndex, sX, sY, EastCap);
  read(pSouthCapIndex, sX, sY, SouthCap);

#pragma unroll
  for (int j = 0; j < 8; j++) {
    // North neighbour
    Neighbor = Height10x8.row(baseY - 1 + j) + 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (NorthCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // South neighbour
    Neighbor = Height10x8.row(baseY + 1 + j) + 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (SouthCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // East neighbour
    Neighbor.select<4, 1>(0) = Height10x8.select<1, 1, 4, 1>(baseY + j, 1);
    Neighbor.select<4, 1>(3) = Height10x8.select<1, 1, 4, 1>(baseY + j, 4);
    Neighbor[7] = RBorder[j][0];
    Neighbor += 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (EastCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // Pix[0]
    // West neighbour
    temps[0] = LBorder[j][0] + 1;
    if ((WestCap[j][0] > 0) && (temps[0] < Height10x8[baseY + j][0]))
      Height10x8[baseY + j][0] = temps[0];

      // Pix[1:7]
#pragma unroll
    for (int i = 1; i < 8; i++) {
      //            temps = Height10x8.select<1,1,2,2>(baseY+j, -1+i) + 1;
      temps[0] = Height10x8[baseY + j][-1 + i] + 1;
      // West neighbour
      if ((WestCap[j][i] > 0) && (temps[0] < Height10x8[baseY + j][i]))
        Height10x8[baseY + j][i] = temps[0];
    }
  }

  // Output the updated height block 8x8
  write(pHeightIndex, 8 * h_pos * sizeof(uint), 8 * v_pos,
        Height10x8.select<8, 1, 8, 1>(baseY, 0));
}
#endif

#ifdef BLOCK_SHORT
// Block 8x8
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void GC_Global_Relabel_NR_u16(
    SurfaceIndex pBlockMaskIndex, SurfaceIndex pHeightIndex,
    SurfaceIndex pWestCapIndex, SurfaceIndex pNorthCapIndex,
    SurfaceIndex pEastCapIndex, SurfaceIndex pSouthCapIndex) {
  matrix<uchar, 1, 1> BlockMask;
  matrix<ushort, 10, 8> Height10x8;
  matrix<ushort, 8, 1> LBorder;
  matrix<ushort, 8, 1> RBorder;
  vector<ushort, 8> Neighbor;
  vector<ushort, 2> temps;

  matrix<short, 8, 8> WestCap;
  matrix<short, 8, 8> NorthCap;
  matrix<short, 8, 8> EastCap;
  matrix<short, 8, 8> SouthCap;

  int h_pos = get_thread_origin_x();
  int v_pos = get_thread_origin_y();

  // Skip inactive block
  read(pBlockMaskIndex, h_pos, v_pos, BlockMask);
  if (BlockMask[0][0] == 0)
    return;

  short baseX = 1; // block base
  short baseY = 1; // Offset for extra row to include neighbors

  // Height block origin without border
  int dX0 = 8 * h_pos * sizeof(ushort); // in bytes
  int dY0 = 8 * v_pos;                  // in rows

  int dX = (8 * h_pos - baseX) * sizeof(ushort); // in bytes
  int dY = 8 * v_pos - baseY;                    // in rows
  int sX = 8 * h_pos * 2;
  int sY = 8 * v_pos;

  // Read Height
  read(pHeightIndex, dX0, dY, Height10x8.select<10, 1, 8, 1>(0, 0));

  // Read left and right Height borders
  read(pHeightIndex, dX, dY0, LBorder);
  read(pHeightIndex, dX + 9 * sizeof(ushort), dY0, RBorder);

  read(pWestCapIndex, sX, sY, WestCap);
  read(pNorthCapIndex, sX, sY, NorthCap);
  read(pEastCapIndex, sX, sY, EastCap);
  read(pSouthCapIndex, sX, sY, SouthCap);

#pragma unroll
  for (int j = 0; j < 8; j++) {
    // North neighbour
    Neighbor = Height10x8.row(baseY - 1 + j) + 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (NorthCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // South neighbour
    Neighbor = Height10x8.row(baseY + 1 + j) + 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (SouthCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // East neighbour
    Neighbor.select<4, 1>(0) = Height10x8.select<1, 1, 4, 1>(baseY + j, 1);
    Neighbor.select<4, 1>(3) = Height10x8.select<1, 1, 4, 1>(baseY + j, 4);
    Neighbor[7] = RBorder[j][0];
    Neighbor += 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (EastCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // West neighbour
    // Pix[0]
    temps[0] = LBorder[j][0] + 1;
    if ((WestCap[j][0] > 0) && (temps[0] < Height10x8[baseY + j][0]))
      Height10x8[baseY + j][0] = temps[0];

      // Pix[1:7]
#pragma unroll
    for (int i = 1; i < 8; i++) {
      //            temps = Height10x8.select<1,1,2,2>(baseY+j, -1+i) + 1;
      temps[0] = Height10x8[baseY + j][-1 + i] + 1;
      // West neighbour
      if ((WestCap[j][i] > 0) && (temps[0] < Height10x8[baseY + j][i]))
        Height10x8[baseY + j][i] = temps[0];
    }
  }

  // Output the updated height block 8x8
  write(pHeightIndex, 8 * h_pos * sizeof(ushort), 8 * v_pos,
        Height10x8.select<8, 1, 8, 1>(baseY, 0));
}
#endif

#ifdef BLOCK_INT
// Block 8x8
////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_Global_Relabel_u32(SurfaceIndex pBlockMaskIndex, SurfaceIndex pHeightIndex,
                      SurfaceIndex pWestCapIndex, SurfaceIndex pNorthCapIndex,
                      SurfaceIndex pEastCapIndex, SurfaceIndex pSouthCapIndex,
                      SurfaceIndex pStatusIndex) {
  matrix<uchar, 1, 1> BlockMask;
  matrix<uint, 10, 8> Height10x8; // 10 GRFs
  matrix<uint, 8, 1> LBorder;
  matrix<uint, 8, 1> RBorder;

  //    matrix<uint, 10, 10> Height;
  matrix<short, 8, 8> WestCap;
  matrix<short, 8, 8> NorthCap;
  matrix<short, 8, 8> EastCap;
  matrix<short, 8, 8> SouthCap;

  matrix<uint, 8, 8> OrigHeight;
  matrix<uint, 8, 8> NewHeight;
  vector<uint, 8> Neighbor;
  vector<uint, 2> temps;

  vector<uint, 8> element_offset(0);
  vector<uint, 8> count(0);
  vector<uint, 8> tmp(0);

  uint h_pos = get_thread_origin_x();
  uint v_pos = get_thread_origin_y();

  // Skip inactive block
  read(pBlockMaskIndex, h_pos, v_pos, BlockMask);
  if (BlockMask[0][0] == 0)
    return;

  short baseX = 1; // block base
  short baseY = 1; // Offset for extra row to include neighbors

  // Height block origin without border
  int dX0 = 8 * h_pos * sizeof(uint); // in bytes
  int dY0 = 8 * v_pos;                // in rows

  int dX = (8 * h_pos - baseX) * sizeof(uint); // in bytes
  int dY = 8 * v_pos - baseY;                  // in rows

  //    cm_wait();

  // Read Height
  read(pHeightIndex, dX0, dY, Height10x8.select<8, 1, 8, 1>(0, 0));
  read(pHeightIndex, dX0, dY + 8, Height10x8.select<2, 1, 8, 1>(8, 0));
  // Read left and right Height borders
  read(pHeightIndex, dX, dY0, LBorder);
  read(pHeightIndex, dX + 9 * sizeof(uint), dY0, RBorder);

  int sX = 8 * h_pos * 2;
  int sY = 8 * v_pos;
  read(pWestCapIndex, sX, sY, WestCap);
  read(pNorthCapIndex, sX, sY, NorthCap);
  read(pEastCapIndex, sX, sY, EastCap);
  read(pSouthCapIndex, sX, sY, SouthCap);

  //    OrigHeight = Height.select<8,1,8,1>(baseY,baseX);
  OrigHeight = Height10x8.select<8, 1, 8, 1>(baseY, 0);
  uint temp;

#pragma unroll
  for (int j = 0; j < 8; j++) {
    // North neighbour
    Neighbor = Height10x8.row(baseY - 1 + j) + 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (NorthCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // South neighbour
    Neighbor = Height10x8.row(baseY + 1 + j) + 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (SouthCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // East neighbour
    Neighbor.select<4, 1>(0) = Height10x8.select<1, 1, 4, 1>(baseY + j, 1);
    Neighbor.select<4, 1>(3) = Height10x8.select<1, 1, 4, 1>(baseY + j, 4);
    Neighbor[7] = RBorder[j][0];
    Neighbor += 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (EastCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // West neighbour
    // Pix[0]
    temps[0] = LBorder[j][0] + 1;
    if ((WestCap[j][0] > 0) && (temps[0] < Height10x8[baseY + j][0]))
      Height10x8[baseY + j][0] = temps[0];

      // Pix[1:7]
#pragma unroll
    for (int i = 1; i < 8; i++) {
      //            temps = Height10x8.select<1,1,2,2>(baseY+j, -1+i) + 1;
      temps[0] = Height10x8[baseY + j][-1 + i] + 1;
      if ((WestCap[j][i] > 0) && (temps[0] < Height10x8[baseY + j][i]))
        Height10x8[baseY + j][i] = temps[0];
    }
  }

  // Update Height if one or more new height is found
  NewHeight.merge(1, 0, Height10x8.select<8, 1, 8, 1>(baseY, 0) != OrigHeight);
  if (NewHeight.any()) {
    // Output the updated height block 8x8
    write(pHeightIndex, 8 * h_pos * sizeof(uint), 8 * v_pos,
          Height10x8.select<8, 1, 8, 1>(baseY, 0));
    // Add new block count by 1.
    write(pStatusIndex, ATOMIC_INC, 0, element_offset, tmp, count);
  }
  //    cm_fence();
}
#endif

#ifdef BLOCK_SHORT
// block 8x8
////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_Global_Relabel_u16(SurfaceIndex pBlockMaskIndex, SurfaceIndex pHeightIndex,
                      SurfaceIndex pWestCapIndex, SurfaceIndex pNorthCapIndex,
                      SurfaceIndex pEastCapIndex, SurfaceIndex pSouthCapIndex,
                      SurfaceIndex pStatusIndex) {

  matrix<uchar, 1, 1> BlockMask;
  matrix<ushort, 10, 8> Height10x8; // 10 GRFs
  matrix<ushort, 8, 1> LBorder;
  matrix<ushort, 8, 1> RBorder;

  matrix<short, 8, 8> WestCap;
  matrix<short, 8, 8> NorthCap;
  matrix<short, 8, 8> EastCap;
  matrix<short, 8, 8> SouthCap;

  matrix<ushort, 8, 8> OrigHeight;
  matrix<ushort, 8, 8> NewHeight;
  vector<ushort, 8> Neighbor;
  vector<ushort, 2> temps;

  vector<uint, 8> element_offset(0);
  vector<uint, 8> count(0);
  vector<uint, 8> tmp(0);

  int h_pos = get_thread_origin_x();
  int v_pos = get_thread_origin_y();

  // Skip inactive block
  read(pBlockMaskIndex, h_pos, v_pos, BlockMask);
  if (BlockMask[0][0] == 0)
    return;

  short baseX = 1; // block base
  short baseY = 1; // Offset for extra row to include neighbors

  // Height block origin without border
  int dX0 = 8 * h_pos * sizeof(ushort); // in bytes
  int dY0 = 8 * v_pos;                  // in rows

  int dX = (8 * h_pos - baseX) * sizeof(ushort); // in bytes
  int dY = 8 * v_pos - baseY;                    // in rows

  //    cm_wait();

  // Read Height
  read(pHeightIndex, dX0, dY, Height10x8.select<10, 1, 8, 1>(0, 0));
  //    read(pHeightIndex, dX0, dY+8, Height10x8.select<2,1,8,1>(8,0));
  // Read left and right Height borders
  read(pHeightIndex, dX, dY0, LBorder);
  read(pHeightIndex, dX + 9 * sizeof(ushort), dY0, RBorder);

  int sX = 8 * h_pos * 2;
  int sY = 8 * v_pos;
  read(pWestCapIndex, sX, sY, WestCap);
  read(pNorthCapIndex, sX, sY, NorthCap);
  read(pEastCapIndex, sX, sY, EastCap);
  read(pSouthCapIndex, sX, sY, SouthCap);

  //    OrigHeight = Height.select<8,1,8,1>(baseY,baseX);
  OrigHeight = Height10x8.select<8, 1, 8, 1>(baseY, 0);
  uint temp;

#pragma unroll
  for (int j = 0; j < 8; j++) {
    // North neighbour
    Neighbor = Height10x8.row(baseY - 1 + j) + 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (NorthCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // South neighbour
    Neighbor = Height10x8.row(baseY + 1 + j) + 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (SouthCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // East neighbour
    Neighbor.select<4, 1>(0) = Height10x8.select<1, 1, 4, 1>(baseY + j, 1);
    Neighbor.select<4, 1>(3) = Height10x8.select<1, 1, 4, 1>(baseY + j, 4);
    Neighbor[7] = RBorder[j][0];
    Neighbor += 1;
    Height10x8.row(baseY + j).merge(Neighbor,
                                    (EastCap.row(j) > 0) &
                                        (Neighbor < Height10x8.row(baseY + j)));

    // West neighbour
    // Pix[0]
    temps[0] = LBorder[j][0] + 1;
    if ((WestCap[j][0] > 0) && (temps[0] < Height10x8[baseY + j][0]))
      Height10x8[baseY + j][0] = temps[0];

      // Pix[1:7]
#pragma unroll
    for (int i = 1; i < 8; i++) {
      //            temps = Height10x8.select<1,1,2,2>(baseY+j, -1+i) + 1;
      temps[0] = Height10x8[baseY + j][-1 + i] + 1;
      // West neighbour
      if ((WestCap[j][i] > 0) && (temps[0] < Height10x8[baseY + j][i]))
        Height10x8[baseY + j][i] = temps[0];
    }
  }

  // Update Height if one or more new height is found
  NewHeight.merge(1, 0, Height10x8.select<8, 1, 8, 1>(baseY, 0) != OrigHeight);
  if (NewHeight.any()) {
    // Output the updated height block 8x8
    write(pHeightIndex, 8 * h_pos * sizeof(ushort), 8 * v_pos,
          Height10x8.select<8, 1, 8, 1>(baseY, 0));
    // Add new block count by 1.
    //        write(pStatusIndex, ATOMIC_INC, 0, element_offset, tmp, count);
    write(pStatusIndex, ATOMIC_INC, 0, element_offset, tmp, NULL);
  }
  //    cm_fence();
}
#endif

///////////////////////////////////////////////////////////////////////////////
// GraphCut_Push
//  Has neighbor dependency, one thread space on 16x16 block.
//  Each processes a 8x8 block at quatrand given by
/*
if active(x) do
    foreach y=neighbor(x)
        if height(y) == height(x) do // check height
            flow = min( capacity(x,y), excess_flow(x)); // pushed flow
            excess_flow(x) -= flow; // update excess flow
            excess_flow(y) += flow;
            capacity(x,y) -= flow;  // update edge cap
            capacity(y,x) += flow;
        done
    end
done
*/

#ifdef BLOCK_INT
// Block 8x16
////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_V_Push_NR_VWF_u32(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                     SurfaceIndex pNorthCapIndex, SurfaceIndex pSouthCapIndex,
                     uint HEIGHT_MAX, int PhysicalThreadsWidth,
                     int BankHeight) {
  // Output block size = 8x16
  matrix<uint, 10, 16>
      Height; // Input. Block size = 8x16, plus extra 2 rows 10x16
  matrix<short, 10, 16> ExcessFlow; // Input and output. 9x16
  matrix<short, 10, 16> NorthCap;   // Input and output. 8x16
  matrix<short, 10, 16> SouthCap;   // Input and output. 8x16

  vector<short, 16> mask;    // 1 GRF
  vector<short, 16> mask2;   // 1 GRF
  vector<short, 16> flow;    // 1 GRF
  matrix<short, 8, 16> temp; // 8 GRFs
  matrix<short, 8, 16> mask8x16;

  vector<uint, 8> element_offset(0); // 1 GRF
  vector<uint, 8> count(0);          // 1 GRF
  vector<uint, 8> tmp(0);            // 1 GRF

  // Virtual coordinate
  uint h_pos0 = get_thread_origin_x();
  uint v_pos0 = get_thread_origin_y();

  // Actual coordinate
  // x = x' % width
  // y = x' / width * height_b + y'
  // x and y is coordinate in physical thread space.  x' and y' is coordinate in
  // logical thread space. width is physical thread space. width_b is the bank
  // height.
  uint h_pos = h_pos0 % PhysicalThreadsWidth;
  uint v_pos = h_pos0 / PhysicalThreadsWidth * BankHeight +
               v_pos0; // BankHeight is the bank height in blocks

  short baseX = 0; // block base
  short baseY = 1; // Offset for extra row

  uint dX = (16 * h_pos - baseX) *
            sizeof(uint);      // in bytes, -2 pixels for DWORD aligned write
  uint dY = 8 * v_pos - baseY; // in rows

  uint nX = (16 * h_pos - baseX) *
            sizeof(short);     // in bytes, -2 pixels for DWORD aligned write
  uint nY = 8 * v_pos - baseY; // in rows

  short update = 0;

  cm_wait();

  // Read 10 rows x 16 columns
  read(pHeightIndex, dX, dY, Height.select<8, 1, 8, 1>(0, 0));
  read(pHeightIndex, dX + 8 * sizeof(uint), dY,
       Height.select<8, 1, 8, 1>(0, 8));
  read(pHeightIndex, dX, dY + 8, Height.select<2, 1, 8, 1>(8, 0));
  read(pHeightIndex, dX + 8 * sizeof(uint), dY + 8,
       Height.select<2, 1, 8, 1>(8, 8));

  mask8x16.merge(1, 0, Height.select<8, 1, 16, 1>(1, 0) == HEIGHT_MAX);
  if (mask8x16.all())
    return;

  read(pExcessFlowIndex, nX, nY, ExcessFlow.select<8, 1, 16, 1>(0, 0));
  read(pExcessFlowIndex, nX, nY + 8, ExcessFlow.select<2, 1, 16, 1>(8, 0));

  // mask8x16 for active nodes
  mask8x16.merge(1, 0, ExcessFlow.select<8, 1, 16, 1>(1, 0) <= 0);
  if (mask8x16.all())
    return;

  read(pNorthCapIndex, nX, nY, NorthCap.select<8, 1, 16, 1>(0, 0));
  read(pNorthCapIndex, nX, nY + 8, NorthCap.select<2, 1, 16, 1>(8, 0));

  read(pSouthCapIndex, nX, nY, SouthCap.select<8, 1, 16, 1>(0, 0));
  read(pSouthCapIndex, nX, nY + 8, SouthCap.select<2, 1, 16, 1>(8, 0));

#pragma unroll
  for (int j = 0; j < 8; j++) {
    // mask for checking Height < HEIGHT_MAX
    mask.merge(1, 0,
               ExcessFlow.row(baseY + j) > 0 &
                   Height.row(baseY + j) < HEIGHT_MAX);

    SIMD_IF_BEGIN(mask) {
      // North neighbour (x, y-1)
      mask2.merge(1, 0,
                  (Height.row(baseY + j - 1) == Height.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow =
            cm_min<short>(NorthCap.row(baseY + j), ExcessFlow.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow.row(baseY + j) -= flow;
          ExcessFlow.row(baseY + j - 1) += flow;
          NorthCap.row(baseY + j) -= flow;
          SouthCap.row(baseY + j - 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;

      // South neighbour (x, y+1)
      mask2.merge(1, 0,
                  (Height.row(baseY + j + 1) == Height.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow =
            cm_min<short>(SouthCap.row(baseY + j), ExcessFlow.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow.row(baseY + j) -= flow;
          ExcessFlow.row(baseY + j + 1) += flow;
          SouthCap.row(baseY + j) -= flow;
          NorthCap.row(baseY + j + 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;
    }
    SIMD_IF_END;
  }

  if (update) {
    // Output ExcessFlow and capacity blocks [9 rows x 16 cols]
    write(pExcessFlowIndex, nX, nY, ExcessFlow.select<8, 1, 16, 1>(0, 0));
    write(pExcessFlowIndex, nX, nY + 8, ExcessFlow.select<2, 1, 16, 1>(8, 0));

    write(pNorthCapIndex, nX, nY, NorthCap.select<8, 1, 16, 1>(0, 0));
    write(pNorthCapIndex, nX, nY + 8, NorthCap.select<2, 1, 16, 1>(8, 0));

    write(pSouthCapIndex, nX, nY, SouthCap.select<8, 1, 16, 1>(0, 0));
    write(pSouthCapIndex, nX, nY + 8, SouthCap.select<2, 1, 16, 1>(8, 0));
  }

  cm_fence();
}
#endif

#ifdef BLOCK_SHORT
// Block 8x16
////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_V_Push_NR_VWF_u16(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                     SurfaceIndex pNorthCapIndex, SurfaceIndex pSouthCapIndex,
                     ushort HEIGHT_MAX, int PhysicalThreadsWidth,
                     int BankHeight) {
  // Output block size = 8x16
  matrix<ushort, 10, 16>
      Height; // Input. Block size = 8x16, plus extra 2 rows 10x16
  matrix<short, 10, 16> ExcessFlow; // Input and output. 9x16
  matrix<short, 10, 16> NorthCap;   // Input and output. 8x16
  matrix<short, 10, 16> SouthCap;   // Input and output. 8x16

  vector<short, 16> mask;    // 1 GRF
  vector<short, 16> mask2;   // 1 GRF
  vector<short, 16> flow;    // 1 GRF
  matrix<short, 8, 16> temp; // 8 GRFs
  matrix<short, 8, 16> mask8x16;

  vector<uint, 8> element_offset(0); // 1 GRF
  vector<uint, 8> count(0);          // 1 GRF
  vector<uint, 8> tmp(0);            // 1 GRF

  // Virtual coordinate
  uint h_pos0 = get_thread_origin_x();
  uint v_pos0 = get_thread_origin_y();

  // Actual coordinate
  // x = x' % width
  // y = x' / width * height_b + y'
  // x and y is coordinate in physical thread space.  x' and y' is coordinate in
  // logical thread space. width is physical thread space. width_b is the bank
  // height.
  uint h_pos = h_pos0 % PhysicalThreadsWidth;
  uint v_pos = h_pos0 / PhysicalThreadsWidth * BankHeight +
               v_pos0; // BankHeight is the bank height in blocks

  short baseX = 0; // block base
  short baseY = 1; // Offset for extra row

  uint dX = (16 * h_pos - baseX) *
            sizeof(ushort);    // in bytes, -2 pixels for DWORD aligned write
  uint dY = 8 * v_pos - baseY; // in rows

  uint nX = (16 * h_pos - baseX) *
            sizeof(short);     // in bytes, -2 pixels for DWORD aligned write
  uint nY = 8 * v_pos - baseY; // in rows

  short update = 0;

  cm_wait();

  // Read 10 rows x 16 columns
  read(pHeightIndex, dX, dY, Height.select<8, 1, 16, 1>(0, 0));
  read(pHeightIndex, dX, dY + 8, Height.select<2, 1, 16, 1>(8, 0));

  mask8x16.merge(1, 0, Height.select<8, 1, 16, 1>(1, 0) == HEIGHT_MAX);
  if (mask8x16.all())
    return;

  read(pExcessFlowIndex, nX, nY, ExcessFlow.select<8, 1, 16, 1>(0, 0));
  read(pExcessFlowIndex, nX, nY + 8, ExcessFlow.select<2, 1, 16, 1>(8, 0));

  // mask8x16 for active nodes
  mask8x16.merge(1, 0, ExcessFlow.select<8, 1, 16, 1>(1, 0) <= 0);
  if (mask8x16.all())
    return;

  read(pNorthCapIndex, nX, nY, NorthCap.select<8, 1, 16, 1>(0, 0));
  read(pNorthCapIndex, nX, nY + 8, NorthCap.select<2, 1, 16, 1>(8, 0));

  read(pSouthCapIndex, nX, nY, SouthCap.select<8, 1, 16, 1>(0, 0));
  read(pSouthCapIndex, nX, nY + 8, SouthCap.select<2, 1, 16, 1>(8, 0));

#pragma unroll
  for (int j = 0; j < 8; j++) {
    // mask for checking Height < HEIGHT_MAX
    mask.merge(1, 0,
               ExcessFlow.row(baseY + j) > 0 &
                   Height.row(baseY + j) < HEIGHT_MAX);

    SIMD_IF_BEGIN(mask) {
      // North neighbour (x, y-1)
      mask2.merge(1, 0,
                  (Height.row(baseY + j - 1) == Height.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow =
            cm_min<short>(NorthCap.row(baseY + j), ExcessFlow.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow.row(baseY + j) -= flow;
          ExcessFlow.row(baseY + j - 1) += flow;
          NorthCap.row(baseY + j) -= flow;
          SouthCap.row(baseY + j - 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;

      // South neighbour (x, y+1)
      mask2.merge(1, 0,
                  (Height.row(baseY + j + 1) == Height.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow =
            cm_min<short>(SouthCap.row(baseY + j), ExcessFlow.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow.row(baseY + j) -= flow;
          ExcessFlow.row(baseY + j + 1) += flow;
          SouthCap.row(baseY + j) -= flow;
          NorthCap.row(baseY + j + 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;
    }
    SIMD_IF_END;
  }

  if (update) {
    // Output ExcessFlow and capacity blocks [9 rows x 16 cols]
    write(pExcessFlowIndex, nX, nY, ExcessFlow.select<8, 1, 16, 1>(0, 0));
    write(pExcessFlowIndex, nX, nY + 8, ExcessFlow.select<2, 1, 16, 1>(8, 0));

    write(pNorthCapIndex, nX, nY, NorthCap.select<8, 1, 16, 1>(0, 0));
    write(pNorthCapIndex, nX, nY + 8, NorthCap.select<2, 1, 16, 1>(8, 0));

    write(pSouthCapIndex, nX, nY, SouthCap.select<8, 1, 16, 1>(0, 0));
    write(pSouthCapIndex, nX, nY + 8, SouthCap.select<2, 1, 16, 1>(8, 0));
  }

  cm_fence();
}
#endif

#ifdef BLOCK_INT
// Block 8x16
////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_V_Push_VWF_u32(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                  SurfaceIndex pNorthCapIndex, SurfaceIndex pSouthCapIndex,
                  SurfaceIndex pStatusIndex, uint HEIGHT_MAX,
                  int PhysicalThreadsWidth, int BankHeight) {
  // Output block size = 8x16
  matrix<uint, 10, 16>
      Height; // Input. Block size = 8x16, plus extra 2 rows 10x16
  matrix<short, 10, 16> ExcessFlow; // Input and output. 9x16
  matrix<short, 10, 16> NorthCap;   // Input and output. 8x16
  matrix<short, 10, 16> SouthCap;   // Input and output. 8x16

  vector<short, 16> mask;    // 1 GRF
  vector<short, 16> mask2;   // 1 GRF
  vector<short, 16> flow;    // 1 GRF
  matrix<short, 8, 16> temp; // 8 GRFs
  matrix<short, 8, 16> mask8x16;

  vector<uint, 8> element_offset(0); // 1 GRF
  vector<uint, 8> count(0);          // 1 GRF
  vector<uint, 8> tmp(0);            // 1 GRF

  // Virtual coordinate
  uint h_pos0 = get_thread_origin_x();
  uint v_pos0 = get_thread_origin_y();

  // Actual coordinate
  // x = x' % width
  // y = x' / width * height_b + y'
  // x and y is coordinate in physical thread space.  x' and y' is coordinate in
  // logical thread space. width is physical thread space. width_b is the bank
  // height.
  uint h_pos = h_pos0 % PhysicalThreadsWidth;
  uint v_pos = h_pos0 / PhysicalThreadsWidth * BankHeight +
               v_pos0; // BankHeight is the bank height in blocks

  short baseX = 0; // block base
  short baseY = 1; // Offset for extra row

  uint dX = (16 * h_pos - baseX) *
            sizeof(uint);      // in bytes, -2 pixels for DWORD aligned write
  uint dY = 8 * v_pos - baseY; // in rows

  uint nX = (16 * h_pos - baseX) *
            sizeof(short);     // in bytes, -2 pixels for DWORD aligned write
  uint nY = 8 * v_pos - baseY; // in rows

  short update = 0;

  cm_wait();

  // Read 10 rows x 16 columns
  read(pHeightIndex, dX, dY, Height.select<8, 1, 8, 1>(0, 0));
  read(pHeightIndex, dX + 8 * sizeof(uint), dY,
       Height.select<8, 1, 8, 1>(0, 8));
  read(pHeightIndex, dX, dY + 8, Height.select<2, 1, 8, 1>(8, 0));
  read(pHeightIndex, dX + 8 * sizeof(uint), dY + 8,
       Height.select<2, 1, 8, 1>(8, 8));

  mask8x16.merge(1, 0, Height.select<8, 1, 16, 1>(1, 0) == HEIGHT_MAX);
  if (mask8x16.all())
    return;

  read(pExcessFlowIndex, nX, nY, ExcessFlow.select<8, 1, 16, 1>(0, 0));
  read(pExcessFlowIndex, nX, nY + 8, ExcessFlow.select<2, 1, 16, 1>(8, 0));

  // mask8x16 for active nodes
  mask8x16.merge(1, 0, ExcessFlow.select<8, 1, 16, 1>(1, 0) <= 0);
  if (mask8x16.all())
    return;

  read(pNorthCapIndex, nX, nY, NorthCap.select<8, 1, 16, 1>(0, 0));
  read(pNorthCapIndex, nX, nY + 8, NorthCap.select<2, 1, 16, 1>(8, 0));

  read(pSouthCapIndex, nX, nY, SouthCap.select<8, 1, 16, 1>(0, 0));
  read(pSouthCapIndex, nX, nY + 8, SouthCap.select<2, 1, 16, 1>(8, 0));

#pragma unroll
  for (int j = 0; j < 8; j++) {
    // mask for checking Height < HEIGHT_MAX
    mask.merge(1, 0,
               ExcessFlow.row(baseY + j) > 0 &
                   Height.row(baseY + j) < HEIGHT_MAX);

    SIMD_IF_BEGIN(mask) {
      // North neighbour (x, y-1)
      mask2.merge(1, 0,
                  (Height.row(baseY + j - 1) == Height.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow =
            cm_min<short>(NorthCap.row(baseY + j), ExcessFlow.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow.row(baseY + j) -= flow;
          ExcessFlow.row(baseY + j - 1) += flow;
          NorthCap.row(baseY + j) -= flow;
          SouthCap.row(baseY + j - 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;

      // South neighbour (x, y+1)
      mask2.merge(1, 0,
                  (Height.row(baseY + j + 1) == Height.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow =
            cm_min<short>(SouthCap.row(baseY + j), ExcessFlow.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow.row(baseY + j) -= flow;
          ExcessFlow.row(baseY + j + 1) += flow;
          SouthCap.row(baseY + j) -= flow;
          NorthCap.row(baseY + j + 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;
    }
    SIMD_IF_END;
  }

  if (update) {
    // Output ExcessFlow and capacity blocks [9 rows x 16 cols]
    write(pExcessFlowIndex, nX, nY, ExcessFlow.select<8, 1, 16, 1>(0, 0));
    write(pExcessFlowIndex, nX, nY + 8, ExcessFlow.select<2, 1, 16, 1>(8, 0));

    write(pNorthCapIndex, nX, nY, NorthCap.select<8, 1, 16, 1>(0, 0));
    write(pNorthCapIndex, nX, nY + 8, NorthCap.select<2, 1, 16, 1>(8, 0));

    write(pSouthCapIndex, nX, nY, SouthCap.select<8, 1, 16, 1>(0, 0));
    write(pSouthCapIndex, nX, nY + 8, SouthCap.select<2, 1, 16, 1>(8, 0));
  }

  // Return true if any active node
  temp.merge(1, 0,
             (ExcessFlow.select<8, 1, 16, 1>(baseY, baseX) > 0) &
                 (Height.select<8, 1, 16, 1>(baseY, baseX) < HEIGHT_MAX));

  tmp[0] = cm_sum<short>(temp);

  // Update status to indicate at least one new height is found in the block
  //    if (temp.any()) {
  if (tmp[0]) {
    // write(pStatusIndex, ATOMIC_INC, 0, element_offset, tmp, count);
    tmp[1] = 1;
    write(pStatusIndex, ATOMIC_ADD, 0, element_offset, tmp, count);
  }

  cm_fence();
}
#endif

#ifdef BLOCK_SHORT
// Block 8x16
////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_V_Push_VWF_u16(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                  SurfaceIndex pNorthCapIndex, SurfaceIndex pSouthCapIndex,
                  SurfaceIndex pStatusIndex, ushort HEIGHT_MAX,
                  int PhysicalThreadsWidth, int BankHeight) {
  // Output block size = 8x16
  matrix<ushort, 10, 16>
      Height; // Input. Block size = 8x16, plus extra 2 rows 10x16
  matrix<short, 10, 16> ExcessFlow; // Input and output. 9x16
  matrix<short, 10, 16> NorthCap;   // Input and output. 8x16
  matrix<short, 10, 16> SouthCap;   // Input and output. 8x16

  vector<short, 16> mask;    // 1 GRF
  vector<short, 16> mask2;   // 1 GRF
  vector<short, 16> flow;    // 1 GRF
  matrix<short, 8, 16> temp; // 8 GRFs
  matrix<short, 8, 16> mask8x16;

  vector<uint, 8> element_offset(0); // 1 GRF
  vector<uint, 8> count(0);          // 1 GRF
  vector<uint, 8> tmp(0);            // 1 GRF

  // Virtual coordinate
  uint h_pos0 = get_thread_origin_x();
  uint v_pos0 = get_thread_origin_y();

  // Actual coordinate
  // x = x' % width
  // y = x' / width * height_b + y'
  // x and y is coordinate in physical thread space.  x' and y' is coordinate in
  // logical thread space. width is physical thread space. width_b is the bank
  // height.
  uint h_pos = h_pos0 % PhysicalThreadsWidth;
  uint v_pos = h_pos0 / PhysicalThreadsWidth * BankHeight +
               v_pos0; // BankHeight is the bank height in blocks

  short baseX = 0; // block base
  short baseY = 1; // Offset for extra row

  uint dX = (16 * h_pos - baseX) *
            sizeof(ushort);    // in bytes, -2 pixels for DWORD aligned write
  uint dY = 8 * v_pos - baseY; // in rows

  uint nX = (16 * h_pos - baseX) *
            sizeof(short);     // in bytes, -2 pixels for DWORD aligned write
  uint nY = 8 * v_pos - baseY; // in rows

  short update = 0;

  cm_wait();

  // Read 10 rows x 16 columns
  read(pHeightIndex, dX, dY, Height.select<8, 1, 16, 1>(0, 0));
  read(pHeightIndex, dX, dY + 8, Height.select<2, 1, 16, 1>(8, 0));

  mask8x16.merge(1, 0, Height.select<8, 1, 16, 1>(1, 0) == HEIGHT_MAX);
  if (mask8x16.all())
    return;

  read(pExcessFlowIndex, nX, nY, ExcessFlow.select<8, 1, 16, 1>(0, 0));
  read(pExcessFlowIndex, nX, nY + 8, ExcessFlow.select<2, 1, 16, 1>(8, 0));

  // mask8x16 for active nodes
  mask8x16.merge(1, 0, ExcessFlow.select<8, 1, 16, 1>(1, 0) <= 0);
  if (mask8x16.all())
    return;

  read(pNorthCapIndex, nX, nY, NorthCap.select<8, 1, 16, 1>(0, 0));
  read(pNorthCapIndex, nX, nY + 8, NorthCap.select<2, 1, 16, 1>(8, 0));

  read(pSouthCapIndex, nX, nY, SouthCap.select<8, 1, 16, 1>(0, 0));
  read(pSouthCapIndex, nX, nY + 8, SouthCap.select<2, 1, 16, 1>(8, 0));

#pragma unroll
  for (int j = 0; j < 8; j++) {
    // mask for checking Height < HEIGHT_MAX
    mask.merge(1, 0,
               ExcessFlow.row(baseY + j) > 0 &
                   Height.row(baseY + j) < HEIGHT_MAX);

    SIMD_IF_BEGIN(mask) {
      // North neighbour (x, y-1)
      mask2.merge(1, 0,
                  (Height.row(baseY + j - 1) == Height.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow =
            cm_min<short>(NorthCap.row(baseY + j), ExcessFlow.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow.row(baseY + j) -= flow;
          ExcessFlow.row(baseY + j - 1) += flow;
          NorthCap.row(baseY + j) -= flow;
          SouthCap.row(baseY + j - 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;

      // South neighbour (x, y+1)
      mask2.merge(1, 0,
                  (Height.row(baseY + j + 1) == Height.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow =
            cm_min<short>(SouthCap.row(baseY + j), ExcessFlow.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow.row(baseY + j) -= flow;
          ExcessFlow.row(baseY + j + 1) += flow;
          SouthCap.row(baseY + j) -= flow;
          NorthCap.row(baseY + j + 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;
    }
    SIMD_IF_END;
  }

  if (update) {
    // Output ExcessFlow and capacity blocks [9 rows x 16 cols]
    write(pExcessFlowIndex, nX, nY, ExcessFlow.select<8, 1, 16, 1>(0, 0));
    write(pExcessFlowIndex, nX, nY + 8, ExcessFlow.select<2, 1, 16, 1>(8, 0));

    write(pNorthCapIndex, nX, nY, NorthCap.select<8, 1, 16, 1>(0, 0));
    write(pNorthCapIndex, nX, nY + 8, NorthCap.select<2, 1, 16, 1>(8, 0));

    write(pSouthCapIndex, nX, nY, SouthCap.select<8, 1, 16, 1>(0, 0));
    write(pSouthCapIndex, nX, nY + 8, SouthCap.select<2, 1, 16, 1>(8, 0));
  }

  // Return true if any active node
  temp.merge(1, 0,
             (ExcessFlow.select<8, 1, 16, 1>(baseY, baseX) > 0) &
                 (Height.select<8, 1, 16, 1>(baseY, baseX) < HEIGHT_MAX));

  tmp[0] = cm_sum<short>(temp);

  // Update status to indicate at least one new height is found in the block
  //    if (temp.any()) {
  if (tmp[0]) {
    // write(pStatusIndex, ATOMIC_INC, 0, element_offset, tmp, count);
    tmp[1] = 1;
    write(pStatusIndex, ATOMIC_ADD, 0, element_offset, tmp, count);
  }

  cm_fence();
}
#endif

#ifdef BLOCK_INT
// Block 8x8
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_H_Push_NR_VWF_u32(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                     SurfaceIndex pWestCapIndex, SurfaceIndex pEastCapIndex,
                     uint HEIGHT_MAX, int PhysicalThreadsHeight,
                     int BankWidth) {
// Block size is 8x8
#define ROWS 8

  matrix<uint, ROWS, 16>
      Height; // Input. Block size = 8x8, plus extra column 8x12. 8GRFs
  matrix<short, ROWS, 16> ExcessFlow; // Input and output 8x12. 8GRFs
  matrix<short, ROWS, 16> WestCap;    // Input and output 8x12. 8GRFs
  matrix<short, ROWS, 16> EastCap;    // Input and output 8x12. 8GRFs

  // Transposed matrix
  matrix<short, 16, ROWS> ExcessFlow_t; // 8 GRFs
  matrix<short, 16, ROWS> WestCap_t;    // 8 GRFs
  matrix<short, 16, ROWS> EastCap_t;    // 8 GRFs
  matrix<uint, 16, ROWS> Height_t;      // 16 GRFs

  matrix<short, ROWS, 8> mask8x8;
  vector<short, 8> mask;
  vector<short, 8> mask2;
  vector<short, 8> flow;

  // Virtual coordinate
  uint h_pos0 = get_thread_origin_x();
  uint v_pos0 = get_thread_origin_y();

  // Actual coordinate
  // y = y' % Height
  // x = y' / Height * Width_b + x'
  // x and y is coordinate in physical thread space.  x' and y' is coordinate in
  // logical thread space. Height is physical thread height. Width_b is the bank
  // width.
  uint v_pos = v_pos0 % PhysicalThreadsHeight;
  uint h_pos = v_pos0 / PhysicalThreadsHeight * BankWidth +
               h_pos0; // BankWidth is the bank width in blocks

  short baseX = 2; // block base
  short baseY = 0;

  uint dX = (8 * h_pos - baseX) *
            sizeof(uint); // in bytes, 2 extra pixels for DWORD aligned write
  uint dY = ROWS * v_pos - baseY; // in rows

  uint nX = (8 * h_pos - baseX) *
            sizeof(short); // in bytes, 2 extra pixels for DWORD aligned write
  uint nY = ROWS * v_pos - baseY; // in rows

  short update = 0;

  cm_wait();

  read(pExcessFlowIndex, nX, nY, ExcessFlow);

  // mask for active nodes
  mask8x8.merge(1, 0, ExcessFlow.select<8, 1, 8, 1>(baseY, baseX) > 0);
  if (!mask8x8.any())
    return;

  Transpose_8x16_To_16x8_Short(ExcessFlow, ExcessFlow_t);

  // Read 8 rows x 8 columns, increase to 8x12 for DWORD aligned write
  read(pHeightIndex, dX, dY, Height.select<ROWS, 1, 8, 1>(0, 0));
  read(pHeightIndex, dX + 8 * sizeof(uint), dY,
       Height.select<ROWS, 1, 4, 1>(0, 8));

  // mask for active nodes
  mask8x8.merge(1, 0, Height.select<ROWS, 1, 8, 1>(baseY, baseX) < HEIGHT_MAX);
  if (!mask8x8.any())
    return;

  Transpose_8x16_To_16x8_Uint(Height, Height_t);

  read(pWestCapIndex, nX, nY, WestCap);
  read(pEastCapIndex, nX, nY, EastCap);

  Transpose_8x16_To_16x8_Short(WestCap, WestCap_t);
  Transpose_8x16_To_16x8_Short(EastCap, EastCap_t);

  baseX = 0; // Transposed block base
  baseY = 2;

#pragma unroll
  for (int j = 0; j < 8; j++) {

    // mask for checking Height < HEIGHT_MAX
    mask.merge(1, 0,
               ExcessFlow_t.row(baseY + j) > 0 &
                   Height_t.row(baseY + j) < HEIGHT_MAX);

    SIMD_IF_BEGIN(mask) {
      // West neighbour (x, y-1)
      mask2.merge(1, 0,
                  (Height_t.row(baseY + j - 1) == Height_t.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow = cm_min<short>(WestCap_t.row(baseY + j),
                             ExcessFlow_t.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow_t.row(baseY + j) -= flow;
          ExcessFlow_t.row(baseY + j - 1) += flow;
          WestCap_t.row(baseY + j) -= flow;
          EastCap_t.row(baseY + j - 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;

      // East neighbour (x, y+1)
      mask2.merge(1, 0,
                  (Height_t.row(baseY + j + 1) == Height_t.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow = cm_min<short>(EastCap_t.row(baseY + j),
                             ExcessFlow_t.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow_t.row(baseY + j) -= flow;
          ExcessFlow_t.row(baseY + j + 1) += flow;
          EastCap_t.row(baseY + j) -= flow;
          WestCap_t.row(baseY + j + 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;
    }
    SIMD_IF_END;
  }

  // Reverse transpose
  Transpose_16x8_To_8x16_Short(ExcessFlow_t, ExcessFlow);
  Transpose_16x8_To_8x16_Short(WestCap_t, WestCap);
  Transpose_16x8_To_8x16_Short(EastCap_t, EastCap);

  //    baseX = 2;   // Restore block base
  //    baseY = 0;

  if (update) {
    // Output ExcessFlow and capacity blocks [8 rows x 10 cols], 12 to be DWORD
    // aligned
    write(pExcessFlowIndex, nX, nY, ExcessFlow);
    write(pWestCapIndex, nX, nY, WestCap);
    write(pEastCapIndex, nX, nY, EastCap);
  }

  cm_fence();
}
#endif

#ifdef BLOCK_SHORT
// Block 8x8
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_H_Push_NR_VWF_u16(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                     SurfaceIndex pWestCapIndex, SurfaceIndex pEastCapIndex,
                     ushort HEIGHT_MAX, int PhysicalThreadsHeight,
                     int BankWidth) {
// Block size is 8x8
#define ROWS 8

  matrix<ushort, ROWS, 16>
      Height; // Input. Block size = 8x8, plus extra column 8x12. 8GRFs
  matrix<short, ROWS, 16> ExcessFlow; // Input and output 8x12. 8GRFs
  matrix<short, ROWS, 16> WestCap;    // Input and output 8x12. 8GRFs
  matrix<short, ROWS, 16> EastCap;    // Input and output 8x12. 8GRFs

  // Transposed matrix
  matrix<short, 16, ROWS> ExcessFlow_t; // 8 GRFs
  matrix<short, 16, ROWS> WestCap_t;    // 8 GRFs
  matrix<short, 16, ROWS> EastCap_t;    // 8 GRFs
  matrix<ushort, 16, ROWS> Height_t;    // 8 GRFs

  matrix<short, ROWS, 8> mask8x8;
  vector<short, 8> mask;
  vector<short, 8> mask2;
  vector<short, 8> flow;

  // Virtual coordinate
  uint h_pos0 = get_thread_origin_x();
  uint v_pos0 = get_thread_origin_y();

  // Actual coordinate
  // y = y' % Height
  // x = y' / Height * Width_b + x'
  // x and y is coordinate in physical thread space.  x' and y' is coordinate in
  // logical thread space. Height is physical thread height. Width_b is the bank
  // width.
  uint v_pos = v_pos0 % PhysicalThreadsHeight;
  uint h_pos = v_pos0 / PhysicalThreadsHeight * BankWidth +
               h_pos0; // BankWidth is the bank width in blocks

  short baseX = 2; // block base
  short baseY = 0;

  uint dX = (8 * h_pos - baseX) *
            sizeof(ushort); // in bytes, 2 extra pixels for DWORD aligned write
  uint dY = ROWS * v_pos - baseY; // in rows

  uint nX = (8 * h_pos - baseX) *
            sizeof(short); // in bytes, 2 extra pixels for DWORD aligned write
  uint nY = ROWS * v_pos - baseY; // in rows

  short update = 0;

  cm_wait();

  read(pExcessFlowIndex, nX, nY, ExcessFlow);

  // mask for active nodes
  mask8x8.merge(1, 0, ExcessFlow.select<8, 1, 8, 1>(baseY, baseX) > 0);
  if (!mask8x8.any())
    return;

  Transpose_8x16_To_16x8_Short(ExcessFlow, ExcessFlow_t);

  // Read 8 rows x 8 columns, increase to 8x12 for DWORD aligned write
  //    read(pHeightIndex, dX, dY, Height.select<ROWS,1,8,1>(0,0));
  //    read(pHeightIndex, dX+8*sizeof(uint), dY,
  //    Height.select<ROWS,1,4,1>(0,8));
  read(pHeightIndex, dX, dY, Height.select<ROWS, 1, 12, 1>(0, 0));

  // mask for active nodes
  mask8x8.merge(1, 0, Height.select<ROWS, 1, 8, 1>(baseY, baseX) < HEIGHT_MAX);
  if (!mask8x8.any())
    return;

  //    Transpose_8x16_To_16x8_Uint(Height, Height_t);
  Transpose_8x16_To_16x8_UShort(Height, Height_t);

  read(pWestCapIndex, nX, nY, WestCap);
  read(pEastCapIndex, nX, nY, EastCap);

  Transpose_8x16_To_16x8_Short(WestCap, WestCap_t);
  Transpose_8x16_To_16x8_Short(EastCap, EastCap_t);

  baseX = 0; // Transposed block base
  baseY = 2;

#pragma unroll
  for (int j = 0; j < 8; j++) {

    // mask for checking Height < HEIGHT_MAX
    mask.merge(1, 0,
               ExcessFlow_t.row(baseY + j) > 0 &
                   Height_t.row(baseY + j) < HEIGHT_MAX);

    SIMD_IF_BEGIN(mask) {
      // West neighbour (x, y-1)
      mask2.merge(1, 0,
                  (Height_t.row(baseY + j - 1) == Height_t.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow = cm_min<short>(WestCap_t.row(baseY + j),
                             ExcessFlow_t.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow_t.row(baseY + j) -= flow;
          ExcessFlow_t.row(baseY + j - 1) += flow;
          WestCap_t.row(baseY + j) -= flow;
          EastCap_t.row(baseY + j - 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;

      // East neighbour (x, y+1)
      mask2.merge(1, 0,
                  (Height_t.row(baseY + j + 1) == Height_t.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow = cm_min<short>(EastCap_t.row(baseY + j),
                             ExcessFlow_t.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow_t.row(baseY + j) -= flow;
          ExcessFlow_t.row(baseY + j + 1) += flow;
          EastCap_t.row(baseY + j) -= flow;
          WestCap_t.row(baseY + j + 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;
    }
    SIMD_IF_END;
  }

  // Reverse transpose
  Transpose_16x8_To_8x16_Short(ExcessFlow_t, ExcessFlow);
  Transpose_16x8_To_8x16_Short(WestCap_t, WestCap);
  Transpose_16x8_To_8x16_Short(EastCap_t, EastCap);

  //    baseX = 2;   // Restore block base
  //    baseY = 0;

  if (update) {
    // Output ExcessFlow and capacity blocks [8 rows x 10 cols], 12 to be DWORD
    // aligned
    write(pExcessFlowIndex, nX, nY, ExcessFlow);
    write(pWestCapIndex, nX, nY, WestCap);
    write(pEastCapIndex, nX, nY, EastCap);
  }

  cm_fence();
}
#endif

#ifdef BLOCK_INT
// Block 8x8
////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_H_Push_VWF_u32(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                  SurfaceIndex pWestCapIndex, SurfaceIndex pEastCapIndex,
                  SurfaceIndex pStatusIndex, uint HEIGHT_MAX,
                  int PhysicalThreadsHeight, int BankWidth) {
#define ROWS 8

  matrix<uint, ROWS, 16>
      Height; // Input. Block size = 8x8, plus extra column 8x12. 8GRFs
  matrix<short, ROWS, 16> ExcessFlow; // Input and output 8x12. 8GRFs
  matrix<short, ROWS, 16> WestCap;    // Input and output 8x12. 8GRFs
  matrix<short, ROWS, 16> EastCap;    // Input and output 8x12. 8GRFs

  // Transposed matrix
  matrix<short, 16, ROWS> ExcessFlow_t; // 8 GRFs
  matrix<short, 16, ROWS> WestCap_t;    // 8 GRFs
  matrix<short, 16, ROWS> EastCap_t;    // 8 GRFs
  matrix<uint, 16, ROWS> Height_t;      // 16 GRFs

  matrix<short, ROWS, 8> mask8x8;
  vector<short, 8> mask;
  vector<short, 8> mask2;
  vector<short, 8> flow;

  vector<uint, 8> element_offset(init_0_7_); // 1 GRF
  vector<uint, 8> count(0);                  // 1 GRF
  vector<uint, 8> tmp(0);                    // 1 GRF

  //    short flow;
  matrix<short, ROWS, 8> temp;

  // Virtual coordinate
  uint h_pos0 = get_thread_origin_x();
  uint v_pos0 = get_thread_origin_y();

  // Actual coordinate
  // y = y' % Height
  // x = y' / Height * Width_b + x'
  // x and y is coordinate in physical thread space.  x' and y' is coordinate in
  // logical thread space. Height is physical thread height. Width_b is the bank
  // width.
  uint v_pos = v_pos0 % PhysicalThreadsHeight;
  uint h_pos = v_pos0 / PhysicalThreadsHeight * BankWidth +
               h_pos0; // BankWidth is the bank width in blocks

  short baseX = 2; // block base
  short baseY = 0;

  uint dX = (8 * h_pos - baseX) *
            sizeof(uint); // in bytes, 2 extra pixels for DWORD aligned write
  uint dY = ROWS * v_pos - baseY; // in rows

  uint nX = (8 * h_pos - baseX) *
            sizeof(short); // in bytes, 2 extra pixels for DWORD aligned write
  uint nY = ROWS * v_pos - baseY; // in rows

  short update = 0;

  cm_wait();

  read(pExcessFlowIndex, nX, nY, ExcessFlow);

  // mask for active nodes
  mask8x8.merge(1, 0, ExcessFlow.select<8, 1, 8, 1>(baseY, baseX) > 0);
  if (!mask8x8.any())
    return;

  Transpose_8x16_To_16x8_Short(ExcessFlow, ExcessFlow_t);

  // Read 8 rows x 8 columns, increase to 8x12 for DWORD aligned write
  read(pHeightIndex, dX, dY, Height.select<ROWS, 1, 8, 1>(0, 0));
  read(pHeightIndex, dX + 8 * sizeof(uint), dY,
       Height.select<ROWS, 1, 4, 1>(0, 8));

  // mask for active nodes
  mask8x8.merge(1, 0, Height.select<ROWS, 1, 8, 1>(baseY, baseX) < HEIGHT_MAX);
  if (!mask8x8.any())
    return;

  Transpose_8x16_To_16x8_Uint(Height, Height_t);

  read(pWestCapIndex, nX, nY, WestCap);
  read(pEastCapIndex, nX, nY, EastCap);

  Transpose_8x16_To_16x8_Short(WestCap, WestCap_t);
  Transpose_8x16_To_16x8_Short(EastCap, EastCap_t);

  baseX = 0; // Transposed block base
  baseY = 2;

#pragma unroll
  for (int j = 0; j < 8; j++) {

    // mask for checking Height < HEIGHT_MAX
    mask.merge(1, 0,
               ExcessFlow_t.row(baseY + j) > 0 &
                   Height_t.row(baseY + j) < HEIGHT_MAX);

    SIMD_IF_BEGIN(mask) {
      // West neighbour (x, y-1)
      mask2.merge(1, 0,
                  (Height_t.row(baseY + j - 1) == Height_t.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow = cm_min<short>(WestCap_t.row(baseY + j),
                             ExcessFlow_t.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow_t.row(baseY + j) -= flow;
          ExcessFlow_t.row(baseY + j - 1) += flow;
          WestCap_t.row(baseY + j) -= flow;
          EastCap_t.row(baseY + j - 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;

      // East neighbour (x, y+1)
      mask2.merge(1, 0,
                  (Height_t.row(baseY + j + 1) == Height_t.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow = cm_min<short>(EastCap_t.row(baseY + j),
                             ExcessFlow_t.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow_t.row(baseY + j) -= flow;
          ExcessFlow_t.row(baseY + j + 1) += flow;
          EastCap_t.row(baseY + j) -= flow;
          WestCap_t.row(baseY + j + 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;
    }
    SIMD_IF_END;
  }

  // Reverse transpose
  Transpose_16x8_To_8x16_Short(ExcessFlow_t, ExcessFlow);
  Transpose_16x8_To_8x16_Short(WestCap_t, WestCap);
  Transpose_16x8_To_8x16_Short(EastCap_t, EastCap);

  baseX = 2; // Restore block base
  baseY = 0;

  if (update) {
    // Output ExcessFlow and capacity blocks [8 rows x 10 cols], 12 to be DWORD
    // aligned
    write(pExcessFlowIndex, nX, nY, ExcessFlow);
    write(pWestCapIndex, nX, nY, WestCap);
    write(pEastCapIndex, nX, nY, EastCap);
  }

  // Return true if any active node
  temp.merge(1, 0,
             (ExcessFlow.select<ROWS, 1, 8, 1>(baseY, baseX) > 0) &
                 (Height.select<ROWS, 1, 8, 1>(baseY, baseX) < HEIGHT_MAX));

  tmp[0] = cm_sum<short>(temp);

  // Update status to indicate at least one new height is found in the block
  //    if (temp.any()) {
  if (tmp[0]) {
    // write(pStatusIndex, ATOMIC_INC, 0, element_offset, tmp, count);
    tmp[1] = 1;
    write(pStatusIndex, ATOMIC_ADD, 0, element_offset, tmp, count);
  }

  cm_fence();
}
#endif

#ifdef BLOCK_SHORT
// Block 8x8
////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" _GENX_MAIN_ void
GC_H_Push_VWF_u16(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                  SurfaceIndex pWestCapIndex, SurfaceIndex pEastCapIndex,
                  SurfaceIndex pStatusIndex, ushort HEIGHT_MAX,
                  int PhysicalThreadsHeight, int BankWidth) {
#define ROWS 8

  matrix<ushort, ROWS, 16>
      Height; // Input. Block size = 8x8, plus extra column 8x12. 8GRFs
  matrix<short, ROWS, 16> ExcessFlow; // Input and output 8x12. 8GRFs
  matrix<short, ROWS, 16> WestCap;    // Input and output 8x12. 8GRFs
  matrix<short, ROWS, 16> EastCap;    // Input and output 8x12. 8GRFs

  // Transposed matrix
  matrix<short, 16, ROWS> ExcessFlow_t; // 8 GRFs
  matrix<short, 16, ROWS> WestCap_t;    // 8 GRFs
  matrix<short, 16, ROWS> EastCap_t;    // 8 GRFs
  matrix<ushort, 16, ROWS> Height_t;    // 8 GRFs

  matrix<short, ROWS, 8> mask8x8;
  vector<short, 8> mask;
  vector<short, 8> mask2;
  vector<short, 8> flow;

  vector<uint, 8> element_offset(init_0_7_); // 1 GRF
  vector<uint, 8> count(0);                  // 1 GRF
  vector<uint, 8> tmp(0);                    // 1 GRF

  //    short flow;
  matrix<short, ROWS, 8> temp;

  // Virtual coordinate
  uint h_pos0 = get_thread_origin_x();
  uint v_pos0 = get_thread_origin_y();

  // Actual coordinate
  // y = y' % Height
  // x = y' / Height * Width_b + x'
  // x and y is coordinate in physical thread space.  x' and y' is coordinate in
  // logical thread space. Height is physical thread height. Width_b is the bank
  // width.
  uint v_pos = v_pos0 % PhysicalThreadsHeight;
  uint h_pos = v_pos0 / PhysicalThreadsHeight * BankWidth +
               h_pos0; // BankWidth is the bank width in blocks

  short baseX = 2; // block base
  short baseY = 0;

  uint dX = (8 * h_pos - baseX) *
            sizeof(ushort); // in bytes, 2 extra pixels for DWORD aligned write
  uint dY = ROWS * v_pos - baseY; // in rows

  uint nX = (8 * h_pos - baseX) *
            sizeof(short); // in bytes, 2 extra pixels for DWORD aligned write
  uint nY = ROWS * v_pos - baseY; // in rows

  short update = 0;

  cm_wait();

  read(pExcessFlowIndex, nX, nY, ExcessFlow);

  // mask for active nodes
  mask8x8.merge(1, 0, ExcessFlow.select<8, 1, 8, 1>(baseY, baseX) > 0);
  if (!mask8x8.any())
    return;

  Transpose_8x16_To_16x8_Short(ExcessFlow, ExcessFlow_t);

  // Read 8 rows x 8 columns, increase to 8x12 for DWORD aligned write
  //    read(pHeightIndex, dX, dY, Height.select<ROWS,1,8,1>(0,0));
  //    read(pHeightIndex, dX+8*sizeof(uint), dY,
  //    Height.select<ROWS,1,4,1>(0,8));
  read(pHeightIndex, dX, dY, Height.select<ROWS, 1, 12, 1>(0, 0));

  // mask for active nodes
  mask8x8.merge(1, 0, Height.select<ROWS, 1, 8, 1>(baseY, baseX) < HEIGHT_MAX);
  if (!mask8x8.any())
    return;

  Transpose_8x16_To_16x8_UShort(Height, Height_t);

  read(pWestCapIndex, nX, nY, WestCap);
  read(pEastCapIndex, nX, nY, EastCap);

  Transpose_8x16_To_16x8_Short(WestCap, WestCap_t);
  Transpose_8x16_To_16x8_Short(EastCap, EastCap_t);

  baseX = 0; // Transposed block base
  baseY = 2;

#pragma unroll
  for (int j = 0; j < 8; j++) {

    // mask for checking Height < HEIGHT_MAX
    mask.merge(1, 0,
               ExcessFlow_t.row(baseY + j) > 0 &
                   Height_t.row(baseY + j) < HEIGHT_MAX);

    SIMD_IF_BEGIN(mask) {
      // West neighbour (x, y-1)
      mask2.merge(1, 0,
                  (Height_t.row(baseY + j - 1) == Height_t.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow = cm_min<short>(WestCap_t.row(baseY + j),
                             ExcessFlow_t.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow_t.row(baseY + j) -= flow;
          ExcessFlow_t.row(baseY + j - 1) += flow;
          WestCap_t.row(baseY + j) -= flow;
          EastCap_t.row(baseY + j - 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;

      // East neighbour (x, y+1)
      mask2.merge(1, 0,
                  (Height_t.row(baseY + j + 1) == Height_t.row(baseY + j) - 1));
      SIMD_IF_BEGIN(mask2) {
        flow = cm_min<short>(EastCap_t.row(baseY + j),
                             ExcessFlow_t.row(baseY + j));
        SIMD_IF_BEGIN(flow != 0) {
          ExcessFlow_t.row(baseY + j) -= flow;
          ExcessFlow_t.row(baseY + j + 1) += flow;
          EastCap_t.row(baseY + j) -= flow;
          WestCap_t.row(baseY + j + 1) += flow;
          update = 1;
        }
        SIMD_IF_END;
      }
      SIMD_IF_END;
    }
    SIMD_IF_END;
  }

  // Reverse transpose
  Transpose_16x8_To_8x16_Short(ExcessFlow_t, ExcessFlow);
  Transpose_16x8_To_8x16_Short(WestCap_t, WestCap);
  Transpose_16x8_To_8x16_Short(EastCap_t, EastCap);

  baseX = 2; // Restore block base
  baseY = 0;

  if (update) {
    // Output ExcessFlow and capacity blocks [8 rows x 10 cols], 12 to be DWORD
    // aligned
    write(pExcessFlowIndex, nX, nY, ExcessFlow);
    write(pWestCapIndex, nX, nY, WestCap);
    write(pEastCapIndex, nX, nY, EastCap);
  }

  // Return true if any active node
  temp.merge(1, 0,
             (ExcessFlow.select<ROWS, 1, 8, 1>(baseY, baseX) > 0) &
                 (Height.select<ROWS, 1, 8, 1>(baseY, baseX) < HEIGHT_MAX));

  tmp[0] = cm_sum<short>(temp);

  // Update status to indicate at least one new height is found in the block
  //    if (temp.any()) {
  if (tmp[0]) {
    // write(pStatusIndex, ATOMIC_INC, 0, element_offset, tmp, count);
    tmp[1] = 1;
    write(pStatusIndex, ATOMIC_ADD, 0, element_offset, tmp, count);
  }

  cm_fence();
}
#endif
