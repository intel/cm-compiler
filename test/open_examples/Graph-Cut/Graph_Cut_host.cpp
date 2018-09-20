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

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>

#include "Graph_Cut_host.h"

#ifdef CMRT_EMU

extern "C" void GC_Create_BlockMask_u32(SurfaceIndex pExcessFlowIndex,
                                        SurfaceIndex pHeightIndex,
                                        SurfaceIndex pBlockMaskIndex,
                                        ushort HEIGHT_MAX);
extern "C" void GC_Create_BlockMask_u16(SurfaceIndex pExcessFlowIndex,
                                        SurfaceIndex pHeightIndex,
                                        SurfaceIndex pBlockMaskIndex,
                                        ushort HEIGHT_MAX);

extern "C" void GC_Init_Height_u32(SurfaceIndex pHeightIndex, uint HEIGHT_MAX);
extern "C" void GC_Init_Height_u16(SurfaceIndex pHeightIndex,
                                   ushort HEIGHT_MAX);

extern "C" void
GC_Relabel_u32(SurfaceIndex pBlockMaskIndex, SurfaceIndex pExcessFlowIndex,
               SurfaceIndex pHeightIndex, SurfaceIndex pWestCapIndex,
               SurfaceIndex pNorthCapIndex, SurfaceIndex pEastCapIndex,
               SurfaceIndex pSouthCapIndex, uint HEIGHT_MAX);

extern "C" void
GC_Relabel_u16(SurfaceIndex pBlockMaskIndex, SurfaceIndex pExcessFlowIndex,
               SurfaceIndex pHeightIndex, SurfaceIndex pWestCapIndex,
               SurfaceIndex pNorthCapIndex, SurfaceIndex pEastCapIndex,
               SurfaceIndex pSouthCapIndex, ushort HEIGHT_MAX);

extern "C" void GC_Global_Relabel_NR_u32(SurfaceIndex pHeightIndex,
                                         SurfaceIndex pWestCapIndex,
                                         SurfaceIndex pNorthCapIndex,
                                         SurfaceIndex pEastCapIndex,
                                         SurfaceIndex pSouthCapIndex);
extern "C" void GC_Global_Relabel_NR_u16(SurfaceIndex pBlockMaskIndex,
                                         SurfaceIndex pHeightIndex,
                                         SurfaceIndex pWestCapIndex,
                                         SurfaceIndex pNorthCapIndex,
                                         SurfaceIndex pEastCapIndex,
                                         SurfaceIndex pSouthCapIndex);

extern "C" void
GC_Global_Relabel_u32(SurfaceIndex pBlockMaskIndex, SurfaceIndex pHeightIndex,
                      SurfaceIndex pWestCapIndex, SurfaceIndex pNorthCapIndex,
                      SurfaceIndex pEastCapIndex, SurfaceIndex pSouthCapIndex,
                      SurfaceIndex pStatusIndex);
extern "C" void
GC_Global_Relabel_u16(SurfaceIndex pHeightIndex, SurfaceIndex pWestCapIndex,
                      SurfaceIndex pNorthCapIndex, SurfaceIndex pEastCapIndex,
                      SurfaceIndex pSouthCapIndex, SurfaceIndex pStatusIndex);

extern "C" void
GC_V_Push_NR_VWF_u32(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                     SurfaceIndex pNorthCapIndex, SurfaceIndex pSouthCapIndex,
                     uint HEIGHT_MAX, int PhysicalThreadsWidth, int BankHeight);
extern "C" void GC_V_Push_NR_VWF_u16(SurfaceIndex pExcessFlowIndex,
                                     SurfaceIndex pHeightIndex,
                                     SurfaceIndex pNorthCapIndex,
                                     SurfaceIndex pSouthCapIndex,
                                     ushort HEIGHT_MAX,
                                     int PhysicalThreadsWidth, int BankHeight);

extern "C" void
GC_H_Push_NR_VWF_u32(SurfaceIndex pExcessFlowIndex, SurfaceIndex pHeightIndex,
                     SurfaceIndex pWestCapIndex, SurfaceIndex pEastCapIndex,
                     uint HEIGHT_MAX, int PhysicalThreadsHeight, int BankWidth);

extern "C" void GC_H_Push_NR_VWF_u16(SurfaceIndex pExcessFlowIndex,
                                     SurfaceIndex pHeightIndex,
                                     SurfaceIndex pWestCapIndex,
                                     SurfaceIndex pEastCapIndex,
                                     ushort HEIGHT_MAX,
                                     int PhysicalThreadsHeight, int BankWidth);

extern "C" void GC_V_Push_VWF_u32(SurfaceIndex pExcessFlowIndex,
                                  SurfaceIndex pHeightIndex,
                                  SurfaceIndex pNorthCapIndex,
                                  SurfaceIndex pSouthCapIndex,
                                  SurfaceIndex pStatusIndex, uint HEIGHT_MAX,
                                  int PhysicalThreadsWidth, int BankHeight);

extern "C" void GC_V_Push_VWF_u16(SurfaceIndex pExcessFlowIndex,
                                  SurfaceIndex pHeightIndex,
                                  SurfaceIndex pNorthCapIndex,
                                  SurfaceIndex pSouthCapIndex,
                                  SurfaceIndex pStatusIndex, ushort HEIGHT_MAX,
                                  int PhysicalThreadsWidth, int BankHeight);

extern "C" void GC_H_Push_VWF_u32(SurfaceIndex pExcessFlowIndex,
                                  SurfaceIndex pHeightIndex,
                                  SurfaceIndex pWestCapIndex,
                                  SurfaceIndex pEastCapIndex,
                                  SurfaceIndex pStatusIndex, uint HEIGHT_MAX,
                                  int PhysicalThreadsHeight, int BankWidth);
extern "C" void GC_H_Push_VWF_u16(SurfaceIndex pExcessFlowIndex,
                                  SurfaceIndex pHeightIndex,
                                  SurfaceIndex pWestCapIndex,
                                  SurfaceIndex pEastCapIndex,
                                  SurfaceIndex pStatusIndex, ushort HEIGHT_MAX,
                                  int PhysicalThreadsHeight, int BankWidth);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MDF_GC::~MDF_GC() {
  if (pTS_Init_Height) {
    pCmDev->DestroyThreadSpace(pTS_Init_Height);
    pTS_Init_Height = 0;
  }
  if (pTS_Relabel) {
    pCmDev->DestroyThreadSpace(pTS_Relabel);
    pTS_Relabel = 0;
  }
  if (pTS_Global_Relabel_NR) {
    pCmDev->DestroyThreadSpace(pTS_Global_Relabel_NR);
    pTS_Global_Relabel_NR = 0;
  }
  if (pTS_Global_Relabel) {
    pCmDev->DestroyThreadSpace(pTS_Global_Relabel);
    pTS_Global_Relabel = 0;
  }
  if (pTS_V_Push) {
    pCmDev->DestroyThreadSpace(pTS_V_Push);
    pTS_V_Push = 0;
  }
  if (pTS_H_Push_NR) {
    pCmDev->DestroyThreadSpace(pTS_H_Push_NR);
    pTS_H_Push_NR = 0;
  }
  if (pTS_H_Push) {
    pCmDev->DestroyThreadSpace(pTS_H_Push);
    pTS_H_Push = 0;
  }

  if (pKernel_Init_Height) {
    pCmDev->DestroyKernel(pKernel_Init_Height);
    pKernel_Init_Height = 0;
  }
  if (pKernel_Relabel) {
    pCmDev->DestroyKernel(pKernel_Relabel);
    pKernel_Relabel = 0;
  }
  if (pKernel_Global_Relabel_NR) {
    pCmDev->DestroyKernel(pKernel_Global_Relabel_NR);
    pKernel_Global_Relabel_NR = 0;
  }
  if (pKernel_Global_Relabel) {
    pCmDev->DestroyKernel(pKernel_Global_Relabel);
    pKernel_Global_Relabel = 0;
  }
  if (pKernel_V_Push) {
    pCmDev->DestroyKernel(pKernel_V_Push);
    pKernel_V_Push = 0;
  }
  if (pKernel_H_Push_NR) {
    pCmDev->DestroyKernel(pKernel_H_Push_NR);
    pKernel_H_Push_NR = 0;
  }
  if (pKernel_H_Push) {
    pCmDev->DestroyKernel(pKernel_H_Push);
    pKernel_H_Push = 0;
  }

  if (pKernelArray1) {
    pCmDev->DestroyTask(pKernelArray1);
    pKernelArray1 = 0;
  }
  if (pKernelArray2) {
    pCmDev->DestroyTask(pKernelArray2);
    pKernelArray2 = 0;
  }
  if (pKernelArray3) {
    pCmDev->DestroyTask(pKernelArray3);
    pKernelArray3 = 0;
  }
  if (pKernelArray4) {
    pCmDev->DestroyTask(pKernelArray4);
    pKernelArray4 = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Create_Kernel_Init_Height(SurfaceIndex *pHeightIndex,
                                      int FrameWidth, int FrameHeight,
                                      int TS_Type) {
  int result;
  int BlockWidth = 16;
  int BlockHeight = 16;
  HEIGHT_TYPE HEIGHT_MAX =
      min(FrameWidth * FrameHeight,
          TYPE_MAX - 1); // Must have -1 to prevent overflow, some calculation
                         // needs HEIGHT_MAX+1

  if (sizeof(HEIGHT_TYPE) == 2)
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Init_Height_u16),
                                  pKernel_Init_Height);
  else
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Init_Height_u32),
                                  pKernel_Init_Height);

  if (result != CM_SUCCESS) {
    perror("CM CreateKernel error for Init_Height");
    return -1;
  }

  // Find # of threads based on output frame size in pixel
  int threadswidth = (int)ceil((float)FrameWidth / BlockWidth);
  int threadsheight = (int)ceil((float)FrameHeight / BlockHeight);
  pKernel_Init_Height->SetThreadCount(threadswidth * threadsheight);

  pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS_Init_Height);
  if (TS_Type == 1)
    pKernel_Init_Height->AssociateThreadSpace(pTS_Init_Height);

  // Set curbe data
  int ParaIdx = 0;
  pKernel_Init_Height->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                    pHeightIndex);
  pKernel_Init_Height->SetKernelArg(ParaIdx++, sizeof(HEIGHT_MAX), &HEIGHT_MAX);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Create_Kernel_BlockMask(SurfaceIndex *pExcessFlowIndex,
                                    SurfaceIndex *pHeightIndex,
                                    SurfaceIndex *pBlockMaskIndex,
                                    int FrameWidth, int FrameHeight,
                                    int BlkRows, int BlkCols) {
  int result;
  HEIGHT_TYPE HEIGHT_MAX = min(FrameWidth * FrameHeight, TYPE_MAX - 1);

  if (sizeof(HEIGHT_TYPE) == 2)
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Create_BlockMask_u16),
                                  pKernel_BlockMask);
  else
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Create_BlockMask_u32),
                                  pKernel_BlockMask);

  if (result != CM_SUCCESS) {
    printf("CM CreateKernel error %d for BlockMask", result);
    return -1;
  }

  int threadswidth = BlkCols / 4; // 4 bytes for DW write in kernel
  int threadsheight = BlkRows;

  pKernel_BlockMask->SetThreadCount(threadswidth * threadsheight);
  pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS_BlockMask);

  // Set curbe data
  int ParaIdx = 0;
  pKernel_BlockMask->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pExcessFlowIndex);
  pKernel_BlockMask->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pHeightIndex);
  pKernel_BlockMask->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pBlockMaskIndex);

  pKernel_BlockMask->SetKernelArg(ParaIdx++, sizeof(HEIGHT_MAX), &HEIGHT_MAX);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Create_Kernel_Relabel(SurfaceIndex *pBlockMaskIndex,
                                  SurfaceIndex *pExcessFlowIndex,
                                  SurfaceIndex *pHeightIndex,
                                  SurfaceIndex *pWestCapIndex,
                                  SurfaceIndex *pNorthCapIndex,
                                  SurfaceIndex *pEastCapIndex,
                                  SurfaceIndex *pSouthCapIndex, int FrameWidth,
                                  int FrameHeight, int TS_Type) {
  int result;
  int BlockWidth;
  int BlockHeight;

  if (sizeof(HEIGHT_TYPE) == 2) {
    BlockWidth = 16;  // 8; // short
    BlockHeight = 16; // 8;
  } else {
    BlockWidth = 8;
    BlockHeight = 8;
  }

  HEIGHT_TYPE HEIGHT_MAX = min(FrameWidth * FrameHeight, TYPE_MAX - 1);

  if (sizeof(HEIGHT_TYPE) == 2)
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Relabel_u16),
                                  pKernel_Relabel);
  else
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Relabel_u32),
                                  pKernel_Relabel);

  if (result != CM_SUCCESS) {
    perror("CM CreateKernel error");
    return -1;
  }

  // Find # of threads based on output frame size in pixel
  int threadswidth = (int)ceil((float)FrameWidth / BlockWidth);
  int threadsheight = (int)ceil((float)FrameHeight / BlockHeight);
  pKernel_Relabel->SetThreadCount(threadswidth * threadsheight);

  pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS_Relabel);
  if (TS_Type == 1)
    pKernel_Relabel->AssociateThreadSpace(pTS_Relabel);

  // Set curbe data
  int ParaIdx = 0;
  pKernel_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                pBlockMaskIndex);
  pKernel_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                pExcessFlowIndex);
  pKernel_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pHeightIndex);
  pKernel_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWestCapIndex);
  pKernel_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                pNorthCapIndex);
  pKernel_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pEastCapIndex);
  pKernel_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                pSouthCapIndex);
  pKernel_Relabel->SetKernelArg(ParaIdx++, sizeof(HEIGHT_MAX), &HEIGHT_MAX);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Create_Kernel_Global_Relabel_NR(
    SurfaceIndex *pBlockMaskIndex, SurfaceIndex *pHeightIndex,
    SurfaceIndex *pWestCapIndex, SurfaceIndex *pNorthCapIndex,
    SurfaceIndex *pEastCapIndex, SurfaceIndex *pSouthCapIndex, int FrameWidth,
    int FrameHeight, int TS_Type) {
  int result;
  int BlockWidth = 8;
  int BlockHeight = 8;
  //    HEIGHT_TYPE HEIGHT_MAX = min( FrameWidth * FrameHeight, TYPE_MAX-1);

  if (sizeof(HEIGHT_TYPE) == 2)
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Global_Relabel_NR_u16),
                                  pKernel_Global_Relabel_NR);
  else
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Global_Relabel_NR_u32),
                                  pKernel_Global_Relabel_NR);

  if (result != CM_SUCCESS) {
    perror("CM CreateKernel error for Global_Relabel");
    return -1;
  }

  // Find # of threads based on output frame size in pixel
  int threadswidth = (int)ceil((float)FrameWidth / BlockWidth);
  int threadsheight = (int)ceil((float)FrameHeight / BlockHeight);
  pKernel_Global_Relabel_NR->SetThreadCount(threadswidth * threadsheight);

  pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS_Global_Relabel_NR);
  if (TS_Type == 1)
    pKernel_Global_Relabel_NR->AssociateThreadSpace(pTS_Global_Relabel_NR);

  // Set curbe data
  int ParaIdx = 0;
  pKernel_Global_Relabel_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                          pBlockMaskIndex);
  pKernel_Global_Relabel_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                          pHeightIndex);
  pKernel_Global_Relabel_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                          pWestCapIndex);
  pKernel_Global_Relabel_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                          pNorthCapIndex);
  pKernel_Global_Relabel_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                          pEastCapIndex);
  pKernel_Global_Relabel_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                          pSouthCapIndex);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Create_Kernel_Global_Relabel(
    SurfaceIndex *pBlockMaskIndex, SurfaceIndex *pHeightIndex,
    SurfaceIndex *pWestCapIndex, SurfaceIndex *pNorthCapIndex,
    SurfaceIndex *pEastCapIndex, SurfaceIndex *pSouthCapIndex,
    SurfaceIndex *pStatusIndex, int FrameWidth, int FrameHeight, int TS_Type) {
  int result;
  int BlockWidth = 8;
  int BlockHeight = 8;
  //    HEIGHT_TYPE HEIGHT_MAX = min( FrameWidth * FrameHeight, TYPE_MAX-1);

  if (sizeof(HEIGHT_TYPE) == 2)
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Global_Relabel_u16),
                                  pKernel_Global_Relabel);
  else
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_Global_Relabel_u32),
                                  pKernel_Global_Relabel);

  if (result != CM_SUCCESS) {
    perror("CM CreateKernel error for Global_Relabel");
    return -1;
  }

  // Find # of threads based on output frame size in pixel
  int threadswidth = (int)ceil((float)FrameWidth / BlockWidth);
  int threadsheight = (int)ceil((float)FrameHeight / BlockHeight);
  pKernel_Global_Relabel->SetThreadCount(threadswidth * threadsheight);

  pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS_Global_Relabel);
  if (TS_Type == 1)
    pKernel_Global_Relabel->AssociateThreadSpace(pTS_Global_Relabel);

  // Set curbe data
  int ParaIdx = 0;
  pKernel_Global_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                       pBlockMaskIndex);
  pKernel_Global_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                       pHeightIndex);
  pKernel_Global_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                       pWestCapIndex);
  pKernel_Global_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                       pNorthCapIndex);
  pKernel_Global_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                       pEastCapIndex);
  pKernel_Global_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                       pSouthCapIndex);
  pKernel_Global_Relabel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                       pStatusIndex);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Create_Kernel_V_Push_NR_VWF(SurfaceIndex *pExcessFlowIndex,
                                        SurfaceIndex *pHeightIndex,
                                        SurfaceIndex *pNorthCapIndex,
                                        SurfaceIndex *pSouthCapIndex,
                                        int FrameWidth, int FrameHeight,
                                        int TS_Type, int V_Banks) {
  int result;
  int BlockWidth = 16;
  int BlockHeight = 8;
  HEIGHT_TYPE HEIGHT_MAX = min(FrameWidth * FrameHeight, TYPE_MAX - 1);

  if (sizeof(HEIGHT_TYPE) == 2)
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_V_Push_NR_VWF_u16),
                                  pKernel_V_Push_NR);
  else
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_V_Push_NR_VWF_u32),
                                  pKernel_V_Push_NR);

  if (result != CM_SUCCESS) {
    perror("CM CreateKernel error");
    return -1;
  }

  // Virtual dimension for multiple wave fronts
  int vFrameWidth = FrameWidth * V_Banks;
  int vFrameHeight = FrameHeight / V_Banks;

  // Find # of threads based on output frame size in pixel
  int threadswidth = (int)ceil((float)vFrameWidth / BlockWidth);
  int threadsheight = (int)ceil((float)vFrameHeight / BlockHeight);
  pKernel_V_Push_NR->SetThreadCount(threadswidth * threadsheight);

  pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS_V_Push_NR);
  pTS_V_Push_NR->SelectThreadDependencyPattern(CM_HORIZONTAL_WAVE);

  if (TS_Type == 1)
    pKernel_V_Push_NR->AssociateThreadSpace(pTS_V_Push_NR);

  int ParaIdx = 0;
  pKernel_V_Push_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pExcessFlowIndex); // Set input surface index
  pKernel_V_Push_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pHeightIndex);
  pKernel_V_Push_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pNorthCapIndex);
  pKernel_V_Push_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pSouthCapIndex);
  pKernel_V_Push_NR->SetKernelArg(ParaIdx++, sizeof(HEIGHT_MAX), &HEIGHT_MAX);

  // Physical thread width
  int PhysicalThreadsWidth = (int)ceil((float)FrameWidth / BlockWidth);
  // Bank height
  int BankHeight = (int)ceil((float)FrameHeight / V_Banks / BlockHeight);
  pKernel_V_Push_NR->SetKernelArg(ParaIdx++, sizeof(PhysicalThreadsWidth),
                                  &PhysicalThreadsWidth);
  pKernel_V_Push_NR->SetKernelArg(ParaIdx++, sizeof(BankHeight), &BankHeight);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Create_Kernel_V_Push_VWF(SurfaceIndex *pExcessFlowIndex,
                                     SurfaceIndex *pHeightIndex,
                                     SurfaceIndex *pNorthCapIndex,
                                     SurfaceIndex *pSouthCapIndex,
                                     SurfaceIndex *pStatusIndex, int FrameWidth,
                                     int FrameHeight, int TS_Type,
                                     int V_Banks) {
  int result;
  int BlockWidth = 16;
  int BlockHeight = 8;
  HEIGHT_TYPE HEIGHT_MAX = min(FrameWidth * FrameHeight, TYPE_MAX - 1);

  if (sizeof(HEIGHT_TYPE) == 2)
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_V_Push_VWF_u16),
                                  pKernel_V_Push);
  else
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_V_Push_VWF_u32),
                                  pKernel_V_Push);

  if (result != CM_SUCCESS) {
    perror("CM CreateKernel error");
    return -1;
  }

  // Virtual dimension for multiple wave fronts
  int vFrameWidth = FrameWidth * V_Banks;
  int vFrameHeight = FrameHeight / V_Banks;

  // Find # of threads based on output frame size in pixel
  int threadswidth = (int)ceil((float)vFrameWidth / BlockWidth);
  int threadsheight = (int)ceil((float)vFrameHeight / BlockHeight);
  pKernel_V_Push->SetThreadCount(threadswidth * threadsheight);

  pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS_V_Push);
  pTS_V_Push->SelectThreadDependencyPattern(CM_HORIZONTAL_WAVE);

  if (TS_Type == 1)
    pKernel_V_Push->AssociateThreadSpace(pTS_V_Push);

  int ParaIdx = 0;
  pKernel_V_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                               pExcessFlowIndex); // Set input surface index
  pKernel_V_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pHeightIndex);
  pKernel_V_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pNorthCapIndex);
  pKernel_V_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pSouthCapIndex);
  pKernel_V_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pStatusIndex);
  pKernel_V_Push->SetKernelArg(ParaIdx++, sizeof(HEIGHT_MAX), &HEIGHT_MAX);

  // Physical thread width
  int PhysicalThreadsWidth = (int)ceil((float)FrameWidth / BlockWidth);
  // Bank height
  int BankHeight = (int)ceil((float)FrameHeight / V_Banks / BlockHeight);
  pKernel_V_Push->SetKernelArg(ParaIdx++, sizeof(PhysicalThreadsWidth),
                               &PhysicalThreadsWidth);
  pKernel_V_Push->SetKernelArg(ParaIdx++, sizeof(BankHeight), &BankHeight);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Create_Kernel_H_Push_NR_VWF(SurfaceIndex *pExcessFlowIndex,
                                        SurfaceIndex *pHeightIndex,
                                        SurfaceIndex *pWestCapIndex,
                                        SurfaceIndex *pEastCapIndex,
                                        int FrameWidth, int FrameHeight,
                                        int TS_Type, int H_Banks) {
  int result;
  int BlockWidth = 8;
  int BlockHeight = 8;
  HEIGHT_TYPE HEIGHT_MAX = min(FrameWidth * FrameHeight, TYPE_MAX - 1);

  if (sizeof(HEIGHT_TYPE) == 2)
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_H_Push_NR_VWF_u16),
                                  pKernel_H_Push_NR);
  else
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_H_Push_NR_VWF_u32),
                                  pKernel_H_Push_NR);

  if (result != CM_SUCCESS) {
    perror("CM CreateKernel error");
    return -1;
  }

  // Virtual dimension for multiple wave fronts
  int vFrameWidth = FrameWidth / H_Banks;
  int vFrameHeight = FrameHeight * H_Banks;

  // Find # of threads based on output frame size in pixel
  int threadswidth = (int)ceil((float)vFrameWidth / BlockWidth);
  int threadsheight = (int)ceil((float)vFrameHeight / BlockHeight);
  pKernel_H_Push_NR->SetThreadCount(threadswidth * threadsheight);

  pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS_H_Push_NR);
  pTS_H_Push_NR->SelectThreadDependencyPattern(CM_VERTICAL_WAVE);
  if (TS_Type == 1)
    pKernel_H_Push_NR->AssociateThreadSpace(pTS_H_Push_NR);

  int ParaIdx = 0;
  pKernel_H_Push_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pExcessFlowIndex); // Set input surface index
  pKernel_H_Push_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pHeightIndex);
  pKernel_H_Push_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pWestCapIndex);
  pKernel_H_Push_NR->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                                  pEastCapIndex);
  pKernel_H_Push_NR->SetKernelArg(ParaIdx++, sizeof(HEIGHT_MAX), &HEIGHT_MAX);

  // Physical thread height
  int PhysicalThreadsHeight = (int)ceil((float)FrameHeight / BlockHeight);
  // Bank width
  int BankWidth = (int)ceil((float)FrameWidth / H_Banks / BlockWidth);
  pKernel_H_Push_NR->SetKernelArg(ParaIdx++, sizeof(PhysicalThreadsHeight),
                                  &PhysicalThreadsHeight);
  pKernel_H_Push_NR->SetKernelArg(ParaIdx++, sizeof(BankWidth), &BankWidth);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Create_Kernel_H_Push_VWF(SurfaceIndex *pExcessFlowIndex,
                                     SurfaceIndex *pHeightIndex,
                                     SurfaceIndex *pWestCapIndex,
                                     SurfaceIndex *pEastCapIndex,
                                     SurfaceIndex *pStatusIndex, int FrameWidth,
                                     int FrameHeight, int TS_Type,
                                     int H_Banks) {
  int result;
  int BlockWidth = 8;
  int BlockHeight = 8;
  HEIGHT_TYPE HEIGHT_MAX = min(FrameWidth * FrameHeight, TYPE_MAX - 1);

  if (sizeof(HEIGHT_TYPE) == 2)
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_H_Push_VWF_u16),
                                  pKernel_H_Push);
  else
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GC_H_Push_VWF_u32),
                                  pKernel_H_Push);

  if (result != CM_SUCCESS) {
    perror("CM CreateKernel error");
    return -1;
  }

  // Virtual dimension for multiple wave fronts
  int vFrameWidth = FrameWidth / H_Banks;
  int vFrameHeight = FrameHeight * H_Banks;

  // Find # of threads based on output frame size in pixel
  int threadswidth = (int)ceil((float)vFrameWidth / BlockWidth);
  int threadsheight = (int)ceil((float)vFrameHeight / BlockHeight);
  pKernel_H_Push->SetThreadCount(threadswidth * threadsheight);

  pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS_H_Push);
  pTS_H_Push->SelectThreadDependencyPattern(CM_VERTICAL_WAVE);
  if (TS_Type == 1)
    pKernel_H_Push->AssociateThreadSpace(pTS_H_Push);

  int ParaIdx = 0;
  pKernel_H_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex),
                               pExcessFlowIndex); // Set input surface index
  pKernel_H_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pHeightIndex);
  pKernel_H_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pWestCapIndex);
  pKernel_H_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pEastCapIndex);
  pKernel_H_Push->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pStatusIndex);
  pKernel_H_Push->SetKernelArg(ParaIdx++, sizeof(HEIGHT_MAX), &HEIGHT_MAX);

  // Physical thread height
  int PhysicalThreadsHeight = (int)ceil((float)FrameHeight / BlockHeight);
  // Bank width
  int BankWidth = (int)ceil((float)FrameWidth / H_Banks / BlockWidth);
  pKernel_H_Push->SetKernelArg(ParaIdx++, sizeof(PhysicalThreadsHeight),
                               &PhysicalThreadsHeight);
  pKernel_H_Push->SetKernelArg(ParaIdx++, sizeof(BankWidth), &BankWidth);

  return 0;
}

////////////////////////////////////////////////////////////
// Enqueue kernel chain: Relabel + V_Push + H_Push_NR
int MDF_GC::Enqueue_GC_RL_VPush_HPush_NR(int nRatio) {
  int iter;
  int KernelGroups = 4; // 3x4 kernels per enqueue

  // Create a task (container) to be put in the task queue
  int result = pCmDev->CreateTask(pKernelArray1);

  // Fill kernel array
  for (int i = 0; i < KernelGroups; i++) {
    if (i != 0)
      pKernelArray1->AddSync();
    pKernelArray1->AddKernel(pKernel_Relabel);
    pKernelArray1->AddSync();
    pKernelArray1->AddKernel(pKernel_V_Push_NR);
    pKernelArray1->AddSync();
    pKernelArray1->AddKernel(pKernel_H_Push_NR);
  }

  for (iter = 0; iter < nRatio; iter += KernelGroups) {
    result = pCmQueue->Enqueue(pKernelArray1, pEvent);
    if (result != CM_SUCCESS) {
      printf("CmDevice enqueue error: %d in Enqueue_GC_RL_VPush_HPush_NR\n",
             result);
      return -1;
    }
  }

  // Destroy a task (container) after using it
  pCmDev->DestroyTask(pKernelArray1);

  return nRatio;
}

////////////////////////////////////////////////////////////
// Enqueue kernel chain: Relabel + V_Push + H_Push
int MDF_GC::Enqueue_GC_RL_VPush_HPush() {
  //    int iter;
  int KernelGroups = 5; // 3x5 kernels per enqueue

  int result = pCmDev->CreateTask(pKernelArray2);

  // Fill kernel array
  for (int i = 0; i < KernelGroups; i++) {
    if (i != 0)
      pKernelArray2->AddSync();
    pKernelArray2->AddKernel(pKernel_Relabel);
    pKernelArray2->AddSync();
    pKernelArray2->AddKernel(pKernel_V_Push_NR);
    pKernelArray2->AddSync();
    if (i < KernelGroups - 1)
      pKernelArray2->AddKernel(pKernel_H_Push_NR);
    else
      pKernelArray2->AddKernel(pKernel_H_Push);
  }

  // Put kernel array into task queue to be executed on GPU
  result = pCmQueue->Enqueue(pKernelArray2, pEvent);
  if (result != CM_SUCCESS) {
    printf("CmDevice enqueue error: %d in Enqueue_GC_RL_VPush_HPush\n", result);
    return -1;
  }
  pCmDev->DestroyTask(pKernelArray2);

  return KernelGroups;
}

////////////////////////////////////////////////////////////
// Enqueue kernel chain: Init_Height + Global_Relabel
int MDF_GC::Enqueue_GC_InitH_GlobalRL(int *pStatus) {
  int KernelsPerEnqueue = 12;
  int MaxIter = (int)ceil(32.0 / KernelsPerEnqueue); // global relabel iter < 32

  // Create a task (container) to be put in the task queue
  int result = pCmDev->CreateTask(pKernelArray3);
  result = pCmDev->CreateTask(pKernelArray4);

  // Fill kernel array3
  pKernelArray3->AddKernel(pKernel_Init_Height);
  for (int i = 1; i < KernelsPerEnqueue; i++) {
    pKernelArray3->AddSync();
    pKernelArray3->AddKernel(pKernel_Global_Relabel_NR);
  }
  CmEvent *pNoEvent = CM_NO_EVENT;
  pCmQueue->Enqueue(pKernelArray3, pNoEvent);

  // Fill kernel array4
  for (int i = 1; i < KernelsPerEnqueue; i++) {
    if (i != 0)
      pKernelArray4->AddSync();
    pKernelArray4->AddKernel(pKernel_Global_Relabel_NR);
  }
  pKernelArray4->AddSync();
  pKernelArray4->AddKernel(pKernel_Global_Relabel);

  pStatus[0] = INT_MAX;
  int PrevCount;
  int iter = 0;

  do {
    PrevCount = pStatus[0];

    // Reset status surface
    memset(pStatus, 0, 8 * sizeof(int));

    result = pCmQueue->Enqueue(pKernelArray4, pEvent);
    if (result != CM_SUCCESS) {
      printf("CmDevice enqueue error: %d in Enqueue_GC_InitH_GlobalRL\n",
             result);
      return -1;
    }

    DWORD dwTimeOutMs = -1;
    result = pEvent->WaitForTaskFinished(dwTimeOutMs);
    if (result != CM_SUCCESS) {
      printf("CM WaitForTaskFinished error: %d.\n", result);
      return -1;
    }

#ifdef _DEBUG
    //        printf("\tGR: %d\n", pStatus[0]);
    printf("\tGlobal relabel new blocks: %d\n", pStatus[0]);
#endif

  } while (pStatus[0] > 0 && ++iter < MaxIter);

  pCmDev->DestroyTask(pKernelArray3);
  pCmDev->DestroyTask(pKernelArray4);
  return 0;
}

////////////////////////////////////////////////////////////
// Enqueue kernel chain: V_Push + H_Push
int MDF_GC::Enqueue_GC_VPush_HPush() {
  int result = pCmDev->CreateTask(pKernelArray4);

  // Fill kernel array, 4 kernels per enqueue, 2 push ops.
  pKernelArray4->AddKernel(pKernel_V_Push_NR);
  pKernelArray4->AddSync();
  //    pKernelArray4->AddKernel(pKernel_H_Push);
  pKernelArray4->AddKernel(pKernel_H_Push_NR);
  pKernelArray4->AddSync();
  pKernelArray4->AddKernel(pKernel_V_Push_NR);
  pKernelArray4->AddSync();
  pKernelArray4->AddKernel(pKernel_H_Push);

  // Put kernel array into task queue to be executed on GPU
  result = pCmQueue->Enqueue(pKernelArray4, pEvent);
  if (result != CM_SUCCESS) {
    printf("CmDevice enqueue error: %d in Enqueue_GC_VPush_HPush\n", result);
    return -1;
  }
  pCmDev->DestroyTask(pKernelArray4);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Enqueue_One_Kernel(CmKernel *pKernel, CmThreadSpace *pTS) {
  int result;

  // Create a task (container) to be put in the task queue
  pKernelArray = NULL;
  result = pCmDev->CreateTask(pKernelArray);
  if (result != CM_SUCCESS) {
    perror("CmDevice CreateTask error");
    return -1;
  }

  // Add a kernel to kernel array
  result = pKernelArray->AddKernel(pKernel);
  if (result != CM_SUCCESS) {
    perror("CmDevice AddKernel error");
    return -1;
  }

  // Put kernel array into task queue to be executed on GPU
  result = pCmQueue->Enqueue(pKernelArray, pEvent, pTS);
  if (result != CM_SUCCESS) {
    perror("CmDevice enqueue error");
    return -1;
  }

  // Destroy a task (container) after using it
  pCmDev->DestroyTask(pKernelArray);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_GC::Enqueue_GC_Global_Relabel(int *pStatus) {
  int result;

  // Create a task (container) to be put in the task queue
  CmTask *pKernelArrayNR = NULL;
  result = pCmDev->CreateTask(pKernelArrayNR);
  if (result != CM_SUCCESS) {
    perror("CmDevice CreateTask error");
    return -1;
  }

  pKernelArray = NULL;
  result = pCmDev->CreateTask(pKernelArray);
  if (result != CM_SUCCESS) {
    perror("CmDevice CreateTask error");
    return -1;
  }

  // Add a kernel without return to kernel array
  pKernelArrayNR->AddKernel(pKernel_Global_Relabel_NR);
  for (int i = 1; i < 4; i++) {
    //        pKernelArrayNR->AddSync();
    pKernelArrayNR->AddKernel(pKernel_Global_Relabel_NR);
  }

  // Back to back enqueue with no return
  pCmQueue->Enqueue(pKernelArrayNR, pEvent, pTS_Global_Relabel_NR);

  // Add kernel with return to kernel array
  pKernelArray->AddKernel(pKernel_Global_Relabel);

  pStatus[0] = INT_MAX;
  int PrevCount;
  static int FirstTime = 1;
  static int ActiveThreshold;

  do {
    PrevCount = pStatus[0];

    // Reset status surface
    // memset(pStatus, 0, 8*sizeof(int));
    pStatus[0] = 0;

    result = pCmQueue->Enqueue(pKernelArray, pEvent, pTS_Global_Relabel);

    DWORD dwTimeOutMs = -1;
    result = pEvent->WaitForTaskFinished(dwTimeOutMs);
    if (result != CM_SUCCESS) {
      printf("CM WaitForTaskFinished error: %d.\n", result);
      return -1;
    }
    /*
            if (FirstTime) {
                ActiveThreshold = pStatus[0] / 10;
                FirstTime = 0;
            }
    */

#ifdef _DEBUG
    printf("\tGlobal relabel new blocks: %d\n", pStatus[0]);
#endif
  } while (PrevCount >= pStatus[0] &&
           pStatus[0] >
               0 /*ActiveThreshold*/); // At least one new height is found

  // Destroy a task (container) after using it
  pCmDev->DestroyTask(pKernelArray);

  return 0;
}
