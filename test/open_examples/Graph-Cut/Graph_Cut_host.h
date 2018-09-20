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

#ifndef _GRAPH_CUT_HOST_H
#define _GRAPH_CUT_HOST_H

#include "cm_rt.h"

#include "General.h"
#include "MDF_Base.h"
#include "common_C_model.h"

#include <climits>

// typedef unsigned int HEIGHT_TYPE;
// typedef unsigned short HEIGHT_TYPE;

/*
#define HEIGHT_SHORT 1

#if HEIGHT_SHORT
typedef unsigned short HEIGHT_TYPE;
#define SIDE_SQUARE 16
#define LOG_SIZE 4
#else
typedef unsigned int HEIGHT_TYPE;
#define SIDE_SQUARE 8
#define LOG_SIZE 3
#endif
*/
#define IMAGE_PADDING 16
#define TYPE_MAX (sizeof(HEIGHT_TYPE) == 2 ? USHRT_MAX : UINT_MAX)

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// C model
extern void PushRelabel_Init(short *pWeight, short *pHCap, short *pVCap,
                             int Rows, int Cols, short *pExcessFlow,
                             HEIGHT_TYPE *pHeight, short *pWestCap,
                             short *pNorthCap, short *pEastCap,
                             short *pSouthCap);

extern void CreateBlockMask(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                            int ImgRows, int ImgCols, unsigned char *pBlockMask,
                            int BlkRows, int BlkCols, int BlkWidth,
                            int BlkHeight);

extern int GetActiveBlocks(unsigned char *pBlockMask, int BlkRows, int BlkCols);

extern int CModel_Push_Relabel(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                               short *pWestCap, short *pNorthCap,
                               short *pEastCap, short *pSouthCap,
                               unsigned char *pBlockMask,
                               unsigned char *pOutput, int Rows, int Cols,
                               int BlkRows, int BlkCols, int BlkWidth,
                               int BlkHeight, int nRatio);

// TF
extern void PushRelabel_Init_TF(short *pWeight, short *pHCap, short *pVCap,
                                int Rows, int Cols, short *pExcessFlow,
                                HEIGHT_TYPE *pHeight, short *pWestCap,
                                short *pNorthCap, short *pEastCap,
                                short *pSouthCap);

extern int CModel_Push_Relabel_TF(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                                  short *pWestCap, short *pNorthCap,
                                  short *pEastCap, short *pSouthCap,
                                  unsigned char *pBlockMask,
                                  unsigned char *pOutput, int Rows, int Cols,
                                  int BlkRows, int BlkCols, int BlkWidth,
                                  int BlkHeight, int nRatio);

// AVX2 TF
extern void AVX2PushRelabel_Init_TF(short *pWeight, short *pHCap, short *pVCap,
                                    int Rows, int Cols, int ColsTF,
                                    short *pExcessFlow, HEIGHT_TYPE *pHeight,
                                    short *pWestCap, short *pNorthCap,
                                    short *pEastCap, short *pSouthCap);

int AVX2Model_Push_Relabel_TF(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                              short *pWestCap, short *pNorthCap,
                              short *pEastCap, short *pSouthCap,
                              unsigned char *pBlockMask,
                              RelabelBlock *pTableBlock, unsigned char *pOutput,
                              int Rows, int Cols, int RowsTF, int ColsTF,
                              int BlkRows, int BlkCols, int BlkWidth,
                              int BlkHeight, int nRatio);

// AVX2
extern void AVX2_PushRelabel_Init(short *pWeight, short *pHCap, short *pVCap,
                                  int Rows, int Cols, short *pExcessFlow,
                                  HEIGHT_TYPE *pHeight, short *pWestCap,
                                  short *pNorthCap, short *pEastCap,
                                  short *pSouthCap, RelabelBlock *tableBlock);
extern int AVX2_Create_TableBlock(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                                  int Rows, int Cols,
                                  RelabelBlock *pTableBlock);
extern int AVX2_Push_Relabel(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                             short *pWestCap, short *pNorthCap, short *pEastCap,
                             short *pSouthCap, unsigned char *pBlockMask,
                             RelabelBlock *tableBlock, unsigned char *pOutput,
                             int Rows, int Cols, int nRatio);

class MDF_GC : public MDF_Base {
public:
  MDF_GC() : MDF_Base() {
    pKernel_BlockMask = 0;
    pKernel_Init_Height = 0;
    pKernel_Relabel = 0;
    pKernel_Global_Relabel_NR = 0;
    pKernel_Global_Relabel = 0;
    pKernel_V_Push_NR = 0;
    pKernel_V_Push = 0;
    pKernel_H_Push_NR = 0;
    pKernel_H_Push = 0;

    pTS_BlockMask = 0;
    pTS_Init_Height = 0;
    pTS_Relabel = 0;
    pTS_Global_Relabel_NR = 0;
    pTS_Global_Relabel = 0;
    pTS_V_Push_NR = 0;
    pTS_V_Push = 0;
    pTS_H_Push_NR = 0;
    pTS_H_Push = 0;

    pKernelArray1 = 0;
    pKernelArray2 = 0;
    pKernelArray3 = 0;
    pKernelArray4 = 0;
  };

  ~MDF_GC();

  CmKernel *pKernel_BlockMask;
  CmKernel *pKernel_Init_Height;
  CmKernel *pKernel_Relabel;
  CmKernel *pKernel_Global_Relabel_NR;
  CmKernel *pKernel_Global_Relabel;
  CmKernel *pKernel_V_Push_NR;
  CmKernel *pKernel_V_Push;
  CmKernel *pKernel_H_Push_NR;
  CmKernel *pKernel_H_Push;

  CmThreadSpace *pTS_BlockMask;
  CmThreadSpace *pTS_Init_Height;
  CmThreadSpace *pTS_Relabel;
  CmThreadSpace *pTS_Global_Relabel_NR;
  CmThreadSpace *pTS_Global_Relabel;
  CmThreadSpace *pTS_V_Push_NR;
  CmThreadSpace *pTS_V_Push;
  CmThreadSpace *pTS_H_Push_NR;
  CmThreadSpace *pTS_H_Push;

  CmTask *pKernelArray1;
  CmTask *pKernelArray2;
  CmTask *pKernelArray3;
  CmTask *pKernelArray4;

  int Create_Kernel_Init_Height(SurfaceIndex *pHeightIndex, int FrameWidth,
                                int FrameHeight, int TS_Type);
  int Create_Kernel_BlockMask(SurfaceIndex *pExcessFlowIndex,
                              SurfaceIndex *pHeightIndex,
                              SurfaceIndex *pBlockMaskIndex, int FrameWidth,
                              int FrameHeight, int BlkRows, int BlkCols);
  int Create_Kernel_Relabel(SurfaceIndex *pBlockMaskIndex,
                            SurfaceIndex *pExcessFlowIndex,
                            SurfaceIndex *pHeightIndex,
                            SurfaceIndex *pWestCapIndex,
                            SurfaceIndex *pNorthCapIndex,
                            SurfaceIndex *pEastCapIndex,
                            SurfaceIndex *pSouthCapIndex, int FrameWidth,
                            int FrameHeight, int TS_Type);
  int Create_Kernel_Global_Relabel_NR(
      SurfaceIndex *pBlockMaskIndex, SurfaceIndex *pHeightIndex,
      SurfaceIndex *pWestCapIndex, SurfaceIndex *pNorthCapIndex,
      SurfaceIndex *pEastCapIndex, SurfaceIndex *pSouthCapIndex, int FrameWidth,
      int FrameHeight, int TS_Type);
  int Create_Kernel_Global_Relabel(
      SurfaceIndex *pBlockMaskIndex, SurfaceIndex *pHeightIndex,
      SurfaceIndex *pWestCapIndex, SurfaceIndex *pNorthCapIndex,
      SurfaceIndex *pEastCapIndex, SurfaceIndex *pSouthCapIndex,
      SurfaceIndex *pStatusIndex, int FrameWidth, int FrameHeight, int TS_Type);
  // Virutal wavefront kernel
  int Create_Kernel_V_Push_NR_VWF(SurfaceIndex *pExcessFlowIndex,
                                  SurfaceIndex *pHeightIndex,
                                  SurfaceIndex *pNorthCapIndex,
                                  SurfaceIndex *pSouthCapIndex, int FrameWidth,
                                  int FrameHeight, int TS_Type, int V_Banks);
  int Create_Kernel_V_Push_VWF(SurfaceIndex *pExcessFlowIndex,
                               SurfaceIndex *pHeightIndex,
                               SurfaceIndex *pNorthCapIndex,
                               SurfaceIndex *pSouthCapIndex,
                               SurfaceIndex *pStatusIndex, int FrameWidth,
                               int FrameHeight, int TS_Type, int V_Banks);
  int Create_Kernel_H_Push_NR_VWF(SurfaceIndex *pExcessFlowIndex,
                                  SurfaceIndex *pHeightIndex,
                                  SurfaceIndex *pWestCapIndex,
                                  SurfaceIndex *pEastCapIndex, int FrameWidth,
                                  int FrameHeight, int TS_Type, int H_Banks);
  int Create_Kernel_H_Push_VWF(SurfaceIndex *pExcessFlowIndex,
                               SurfaceIndex *pHeightIndex,
                               SurfaceIndex *pWestCapIndex,
                               SurfaceIndex *pEastCapIndex,
                               SurfaceIndex *pStatusIndex, int FrameWidth,
                               int FrameHeight, int TS_Type, int H_Banks);

  int Enqueue_One_Kernel(CmKernel *pKernel, CmThreadSpace *pTS);
  int Enqueue_GC_Global_Relabel(int *pStatus);

  int Enqueue_GC_RL_VPush_HPush_NR(
      int nRatio); // Enqueue kernel chain: Relabel + Vertical_Push +
                   // Horizontal_Push_NR
  int Enqueue_GC_RL_VPush_HPush(); // Enqueue kernel chain: Relabel +
                                   // Vertical_Push + Horizontal_Push
  int Enqueue_GC_InitH_GlobalRL(
      int *pStatus); // Enqueue kernel chain: Init_Height + Global_Relabel
  int Enqueue_GC_VPush_HPush(); // Enqueue kernel chain: Vertical_Push +
                                // Horizontal_Push
};

#endif
