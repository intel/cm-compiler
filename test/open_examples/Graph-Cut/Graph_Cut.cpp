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

#define NUM_FILES 6
#define ALIGNMENT32 32
#define ALIGNMENT4K 4096

////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  // Detect if MDF exists on this system.  MDF_Existence = true is MDF exist.
  bool MDF_Existence = Detect_MDF();

  char IMAGE_FOLDER[64] = "";
  char LineBuffer[256];
  int result;

  char iFilename[NUM_FILES][64] = {0};
  FILE *iFileHandle[NUM_FILES] = {0};
  float *iOrigBuffer;
  short *iBuffer[NUM_FILES] = {0};

  char CPU_fn[128];
  char AVX_fn[128];
  char CPU_TF_fn[128];
  char AVX_TF_fn[128];
  char GPU_fn[128];
  char diff_fn[128];

  unsigned char *pOutputCPU;
  unsigned char *pOutputGPU;

  int FrameWidth, FrameHeight;
  int BlkWidth = 16;
  int BlkHeight = 16;
  // int BlkCols;            // Block count horizontally
  // int BlkRows;            // Block count vertically

  int nBits = 5; // Lower bits to check
  int nRatio = 1 << nBits;
  int ActivePixThreshold = 0;
  int iter_AVX2;
  float fDataScaler = 1.0;

  float MaxNode = -1000.0;
  float MinNode = 1000.0;
  float MaxVertEdge = -1000.0;
  float MinVertEdge = 1000.0;
  float MaxHoriEdge = -1000.0;
  float MinHoriEdge = 1000.0;

  unsigned int pitch_inputSurf;
  unsigned int size_inputSurf;

  for (int i = 1; i < argc; i++) {
    strcpy(LineBuffer, argv[i]);

    if (LineBuffer[0] != '-') {
      printf("%s is an invalid parameter\n", LineBuffer);
      exit(0);
    } else {
      if (LineBuffer[1] == 'i' && LineBuffer[2] == ':') // node_weight file
        strcpy(iFilename[0], &LineBuffer[3]);
      else if (LineBuffer[1] == 'j' &&
               LineBuffer[2] == ':') // horizontal_weight file
        strcpy(iFilename[1], &LineBuffer[3]);
      else if (LineBuffer[1] == 'k' &&
               LineBuffer[2] == ':') // vertical_weight file
        strcpy(iFilename[2], &LineBuffer[3]);

      else if (LineBuffer[1] == 'w' && LineBuffer[2] == ':') // Frame width
        sscanf(&LineBuffer[3], "%d", &FrameWidth);
      else if (LineBuffer[1] == 'h' && LineBuffer[2] == ':') // Frame height
        sscanf(&LineBuffer[3], "%d", &FrameHeight);
      else if (LineBuffer[1] == 'r' &&
               LineBuffer[2] == ':') // Ratio of relable and global relabel
        sscanf(&LineBuffer[3], "%d", &nRatio);
      else {
        printf("%s is an invalid parameter\n", LineBuffer);
        exit(0);
      }
    }
  }

  printf("\nInput parameters:\n");
  printf("Node weight file: %s\n", iFilename[0]);
  printf("Horizontal weight file: %s\n", iFilename[1]);
  printf("Vertical weight file: %s\n", iFilename[2]);
  printf("Frame width: %d\n", FrameWidth);
  printf("Frame height: %d\n", FrameHeight);
  printf("Local to global relabel ratio: %d\n", nRatio);

  // Get test name prefix
  char prefix[64];
  char fn_copy[128];
  strcpy(fn_copy, iFilename[0]);
  char *token_ptr = strtok(fn_copy, "_.\n");
  strcpy(prefix, token_ptr);

  // Add image folder name to the iFilename
  for (int i = 0; i < NUM_FILES; i++) {
    strcpy(LineBuffer, IMAGE_FOLDER);
    strcat(LineBuffer, iFilename[i]);
    strcpy(iFilename[i], LineBuffer);
  }

  // Set frame facter to get complete frame size based on the video format given
  // in filename
  float InputFrameFactor = 1.0;
  float OutputFrameFactor = 1.0;

  if (strstr(iFilename[0], ".Y8")) {
    InputFrameFactor = 1.0;
  } else if (strstr(iFilename[0], ".16bin")) {
    InputFrameFactor = 2.0;
  } else if (strstr(iFilename[0], ".F32")) {
    InputFrameFactor = 4.0;
  } else {
    printf("Unknown input image format, %s\n", iFilename[0]);
    exit(0);
  }

  // Open input file 0
  iFileHandle[0] = fopen(iFilename[0], "rb");
  if (iFileHandle[0] == NULL) {
    printf("Error opening %s\n", iFilename[0]);
    exit(1);
  }

  // Allocate original input buffer
  int OrigInputFrameSize = (int)(FrameWidth * FrameHeight * InputFrameFactor);
  iOrigBuffer = (float *)CM_ALIGNED_MALLOC(OrigInputFrameSize, ALIGNMENT32);
  if (!iOrigBuffer) {
    printf("Failed allocate iOrigBuffer\n");
    exit(1);
  }

  // Allocate input buffers
  int InputFrameSize = (int)(FrameWidth * FrameHeight * 2); // short
  for (int i = 0; i < 3; i++) {
    iBuffer[i] = (short *)CM_ALIGNED_MALLOC(InputFrameSize, ALIGNMENT32);
    if (!iBuffer[i]) {
      printf("Failed allocate InputBuffer\n");
      exit(1);
    }
  }

  // Read node_weight
  if (strstr(iFilename[0], ".Y8") || strstr(iFilename[0], ".16bin")) {
    if (fread(iBuffer[0], sizeof(char), InputFrameSize, iFileHandle[0]) !=
        InputFrameSize) {
      printf("Unable to read a frame of data.");
      return -1;
    }
  } else if (strstr(iFilename[0], ".F32")) {
    if (result = fread(iOrigBuffer, sizeof(char), OrigInputFrameSize,
                       iFileHandle[0]) != OrigInputFrameSize) {
      printf("Unable to read a frame of data.");
      return -1;
    }

    for (int i = 0; i < FrameWidth * FrameHeight; i++) {
      //            *(iBuffer[0] + i) = (short) (*(iOrigBuffer + i) + 0.5f); //
      //            float to short
      *(iBuffer[0] + i) =
          (short)(*(iOrigBuffer + i) * fDataScaler); // float to short

      MaxNode = (*(iOrigBuffer + i) > MaxNode) ? *(iOrigBuffer + i) : MaxNode;
      MinNode = (*(iOrigBuffer + i) < MinNode) ? *(iOrigBuffer + i) : MinNode;
    }
  }

  // Read horizontal weight
  if (strstr(iFilename[1], ".Y8") || strstr(iFilename[0], ".16bin")) {
    iFileHandle[1] = fopen(iFilename[1], "rb");
    if (iFileHandle[1] == NULL) {
      printf("Error opening %s\n", iFilename[1]);
      exit(1);
    }
    if (fread(iBuffer[1], sizeof(char), InputFrameSize, iFileHandle[1]) !=
        InputFrameSize) {
      printf("Unable to read a frame of data.");
      return -1;
    }
  } else if (strstr(iFilename[1], ".F32")) {

    iFileHandle[1] = fopen(iFilename[1], "rb");
    if (iFileHandle[1] == NULL) {
      printf("Error opening %s\n", iFilename[1]);
      exit(1);
    }
    if (fread(iOrigBuffer, sizeof(char), OrigInputFrameSize, iFileHandle[1]) !=
        OrigInputFrameSize) {
      printf("Unable to read a frame of data.");
      return -1;
    }

    for (int i = 0; i < FrameWidth * FrameHeight; i++) {
      //            *(iBuffer[1] + i) = (short) (*(iOrigBuffer + i) + 0.5f); //
      //            float to short
      *(iBuffer[1] + i) =
          (short)(*(iOrigBuffer + i) * fDataScaler); // float to short

      MaxHoriEdge =
          (*(iOrigBuffer + i) > MaxHoriEdge) ? *(iOrigBuffer + i) : MaxHoriEdge;
      MinHoriEdge =
          (*(iOrigBuffer + i) < MinHoriEdge) ? *(iOrigBuffer + i) : MinHoriEdge;
    }
  }

  // Read vertical weight
  if (strstr(iFilename[2], ".Y8") || strstr(iFilename[0], ".16bin")) {
    iFileHandle[2] = fopen(iFilename[2], "rb");
    if (iFileHandle[2] == NULL) {
      printf("Error opening %s\n", iFilename[2]);
      exit(1);
    }
    if (fread(iBuffer[2], sizeof(char), InputFrameSize, iFileHandle[2]) !=
        InputFrameSize) {
      printf("Unable to read a frame of data.");
      return -1;
    }

  } else if (strstr(iFilename[2], ".F32")) {

    iFileHandle[2] = fopen(iFilename[2], "rb");
    if (iFileHandle[2] == NULL) {
      printf("Error opening %s\n", iFilename[2]);
      exit(1);
    }
    if (fread(iOrigBuffer, sizeof(char), OrigInputFrameSize, iFileHandle[2]) !=
        OrigInputFrameSize) {
      printf("Unable to read a frame of data.");
      return -1;
    }

    for (int i = 0; i < FrameWidth * FrameHeight; i++) {
      //            *(iBuffer[2] + i) = (short) (*(iOrigBuffer + i) + 0.5f); //
      //            float to short
      *(iBuffer[2] + i) =
          (short)(*(iOrigBuffer + i) * fDataScaler); // float to short

      MaxVertEdge =
          (*(iOrigBuffer + i) > MaxVertEdge) ? *(iOrigBuffer + i) : MaxVertEdge;
      MinVertEdge =
          (*(iOrigBuffer + i) < MinVertEdge) ? *(iOrigBuffer + i) : MinVertEdge;
    }
  }

  // Close input files
  for (int i = 0; i < NUM_FILES; i++) {
    if (iFileHandle[i] != 0) {
      fclose(iFileHandle[i]);
      iFileHandle[i] = 0;
    }
  }

  // Allocate output buffers
  int OutputFrameSize = (int)(FrameWidth * FrameHeight * OutputFrameFactor);
  pOutputCPU = (unsigned char *)CM_ALIGNED_MALLOC(
      OutputFrameSize, ALIGNMENT32); // CPU output buffer
  if (!pOutputCPU) {
    printf("Failed allocate OutputBuffer\n");
    exit(1);
  }
  pOutputGPU = (unsigned char *)CM_ALIGNED_MALLOC(
      OutputFrameSize, ALIGNMENT32); // GPU output buffer
  if (!pOutputGPU) {
    printf("Failed allocate OutputBuffer3\n");
    exit(1);
  }

  ////////////////////////////////////////////////////////////////////
  // Graph Cut C model
  int wBufferSize = sizeof(short) * FrameHeight * FrameWidth;
  short *pExcessFlow = (short *)CM_ALIGNED_MALLOC(wBufferSize, ALIGNMENT32);
  short *pWestCap = (short *)CM_ALIGNED_MALLOC(wBufferSize, ALIGNMENT32);
  short *pNorthCap = (short *)CM_ALIGNED_MALLOC(wBufferSize, ALIGNMENT32);
  short *pEastCap = (short *)CM_ALIGNED_MALLOC(wBufferSize, ALIGNMENT32);
  short *pSouthCap = (short *)CM_ALIGNED_MALLOC(wBufferSize, ALIGNMENT32);

  int iBufferSize = sizeof(HEIGHT_TYPE) * FrameHeight * FrameWidth;
  HEIGHT_TYPE *pHeight =
      (HEIGHT_TYPE *)CM_ALIGNED_MALLOC(iBufferSize, ALIGNMENT32);

  // 1D status buffer
  unsigned int StatusSize = 8 * sizeof(int);
  int *pStatus = (int *)CM_ALIGNED_MALLOC(StatusSize, ALIGNMENT4K);
  if (pStatus == NULL) {
    printf("pStatus allocation failed.\n");
    exit(-1);
  }
  memset(pStatus, 0, 8 * sizeof(int));

  // Note: This buffer is used in CPU code only with 16x16 block size.
  // 2D buffer, pitch may not be aligned
  int BlkCols = (int)(((float)FrameWidth) / BlkWidth + 0.5f);
  int BlkRows = (int)(((float)FrameHeight) / BlkHeight + 0.5f);
  unsigned char *pBlockMask =
      (unsigned char *)CM_ALIGNED_MALLOC(BlkCols * BlkRows, ALIGNMENT4K);
  if (pBlockMask == NULL) {
    printf("pBlockMask allocation failed.\n");
    exit(-1);
  }
  memset(pBlockMask, 0, BlkCols * BlkRows);

  int size = FrameWidth * FrameHeight / (SIDE_SQUARE * SIDE_SQUARE) + 1;
  RelabelBlock *tableBlock =
      (RelabelBlock *)CM_ALIGNED_MALLOC(sizeof(RelabelBlock) * size, ALIGNMENT32);
  memset(tableBlock, -1, sizeof(RelabelBlock) * size);

  sprintf(CPU_fn, "%s_Output_CPU.%dx%d.Y8", prefix, FrameWidth,
          FrameHeight);
  printf("---------------------------------------------------------------------"
         "--\n\n");

#define _C_MODEL_ 1
#ifdef _C_MODEL_
  printf("C model:\n");
  double time_a = GetTimeMS();

  // Graph cut (push-relabel) C model entry
  PushRelabel_Init(iBuffer[0], iBuffer[1], iBuffer[2], FrameHeight, FrameWidth,
                   pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap,
                   pSouthCap);

  int CPU_iter = CModel_Push_Relabel(
      pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap,
      pBlockMask, pOutputCPU, FrameHeight, FrameWidth, BlkRows, BlkCols,
      BlkWidth, BlkHeight, nRatio);

  double time_b = GetTimeMS();

  printf("CPU Graph-Cut loops = %d\n", CPU_iter);
  printf("C model time = %g ms\n", time_b - time_a);

  Dump2File(CPU_fn, (unsigned char *)pOutputCPU, OutputFrameSize);
  printf("---------------------------------------------------------------------"
         "--\n\n");
#endif

  CM_ALIGNED_FREE(pBlockMask); // Free buffer for CPU code only

  HEIGHT_TYPE HEIGHT_MAX;

  ///////////////////////////////////////////////////////////////////////
  // Exit the sample app if no MDF is detected.
  if (!MDF_Existence)
    return 0;

  ////////////////////////////// GPU code //////////////////////////////
  printf("GPU execution:\n");

  MDF_GC cm; // CM Context

  // Node: this pBlockMask buffer is for GPU only with block sise of 8x8
  BlkWidth = 8;
  BlkHeight = 8;
  BlkCols = (int)(((float)FrameWidth) / BlkWidth + 0.5f);
  BlkRows = (int)(((float)FrameHeight) / BlkHeight + 0.5f);
  pBlockMask =
      (unsigned char *)CM_ALIGNED_MALLOC(BlkCols * BlkRows, ALIGNMENT4K);
  memset(pBlockMask, 0, BlkCols * BlkRows);

  cm.pCmDev->GetSurface2DInfo(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8,
                              pitch_inputSurf, size_inputSurf);
  short *pDebug = (short *)CM_ALIGNED_MALLOC(size_inputSurf, ALIGNMENT4K);
  if (pDebug == NULL) {
    printf("pDebug allocation failed.\n");
    exit(-1);
  }
  memset(pDebug, 0, size_inputSurf);

  // Allocate input surface
  // CM_SURFACE_FORMAT InputD3DFMT = CM_SURFACE_FORMAT_A8;
  CmSurface2D *pInputSurf = NULL;
  SurfaceIndex *pInputIndex = NULL;
  cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_A8, &pInputSurf,
                     &pInputIndex);

  // Allocate input surface2
  CmSurface2D *pInputSurf2 = NULL;
  SurfaceIndex *pInputIndex2 = NULL;
  cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_A8,
                     &pInputSurf2, &pInputIndex2);

  // Allocate input surface3
  CmSurface2D *pInputSurf3 = NULL;
  SurfaceIndex *pInputIndex3 = NULL;
  cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_A8,
                     &pInputSurf3, &pInputIndex3);

  // Input suirfaces for Push-Relabel

  CmSurface2D *pBlockMaskSurf = NULL;
  SurfaceIndex *pBlockMaskIndex = NULL;
  cm.AllocGPUSurface(BlkCols, BlkRows, CM_SURFACE_FORMAT_A8, &pBlockMaskSurf,
                     &pBlockMaskIndex);

  // InputD3DFMT = CM_SURFACE_FORMAT_A8R8G8B8;  // D3DFMT_V8U8;
  CmSurface2D *pHeightSurf = NULL;
  SurfaceIndex *pHeightIndex = NULL;
  if (sizeof(HEIGHT_TYPE) == 2)
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8,
                       &pHeightSurf, &pHeightIndex);
  else
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_A8R8G8B8,
                       &pHeightSurf, &pHeightIndex);

  // InputD3DFMT = CM_SURFACE_FORMAT_V8U8;
  CmSurface2D *pExcessFlowSurf = NULL;
  SurfaceIndex *pExcessFlowIndex = NULL;

#ifdef CM_DX11
  cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_R16_SINT,
                     &pExcessFlowSurf,
                     &pExcessFlowIndex); // R16_SINT for signed short, R16_UINT
#else
  cm.AllocGPUSurface(
      FrameWidth, FrameHeight, CM_SURFACE_FORMAT_L16, &pExcessFlowSurf,
      &pExcessFlowIndex); // L16 for signed short, V8U8 for unsigned short
#endif

  CmSurface2D *pWestCapSurf = NULL;
  SurfaceIndex *pWestCapIndex = NULL;
  cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8,
                     &pWestCapSurf, &pWestCapIndex);

  CmSurface2D *pNorthCapSurf = NULL;
  SurfaceIndex *pNorthCapIndex = NULL;
  cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8,
                     &pNorthCapSurf, &pNorthCapIndex);

  CmSurface2D *pEastCapSurf = NULL;
  SurfaceIndex *pEastCapIndex = NULL;
  cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8,
                     &pEastCapSurf, &pEastCapIndex);

  CmSurface2D *pSouthCapSurf = NULL;
  SurfaceIndex *pSouthCapIndex = NULL;
  cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8,
                     &pSouthCapSurf, &pSouthCapIndex);

  // Status buffer
  CmBufferUP *pStatusBuf = NULL;
  SurfaceIndex *pStatusIndex = NULL;
  cm.AllocGPUBufferUP(StatusSize, (unsigned char *)pStatus, &pStatusBuf,
                      &pStatusIndex);

  // Init input surface with input images.  Could use GPU copy for higher
  // performance
  result = pInputSurf->WriteSurface((unsigned char *)iBuffer[0], NULL);
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }
  result = pInputSurf2->WriteSurface((unsigned char *)iBuffer[1], NULL);
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }
  result = pInputSurf3->WriteSurface((unsigned char *)iBuffer[2], NULL); // mask
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }

  // Graph cut inputs
  PushRelabel_Init(iBuffer[0], iBuffer[1], iBuffer[2], FrameHeight, FrameWidth,
                   pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap,
                   pSouthCap);

  memset(pBlockMask, 0, BlkCols * BlkRows);
  result = pBlockMaskSurf->WriteSurface((unsigned char *)pBlockMask, NULL);
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }

  result = pExcessFlowSurf->WriteSurface((unsigned char *)pExcessFlow, NULL);
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }
  result = pHeightSurf->WriteSurface((unsigned char *)pHeight, NULL);
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }
  result = pWestCapSurf->WriteSurface((unsigned char *)pWestCap, NULL);
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }
  result = pNorthCapSurf->WriteSurface((unsigned char *)pNorthCap, NULL);
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }
  result = pEastCapSurf->WriteSurface((unsigned char *)pEastCap, NULL);
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }
  result = pSouthCapSurf->WriteSurface((unsigned char *)pSouthCap, NULL);
  if (result != CM_SUCCESS) {
    perror("CM WriteSurface error");
    return -1;
  }

  ////////////////////////////////////////////////////////////////////////////
  double TotalStartTime, TotalEndTime;
  double AVX2StartTime, AVX2EndTime;
  double CopyStartTime, CopyEndTime;
  double T1, T2;
  int iter = 1;
  int TS_Type;
  int ActiveBlocks;

  printf("Height type is %s\n", (sizeof(HEIGHT_TYPE) == 2) ? "short" : "int");

  // Frame width = 320, valid values = 1, 2, 4, 5, 8 or 10.
  // Frame height = 240, valide values = 1, 2, 3, 5, 6 or 10.
  // Need evenly dividable by 8x8 blocks
  int H_Banks = (FrameWidth >= 1280) ? 5 : 4;
  int V_Banks = (FrameHeight >= 720) ? 5 : 3;
  printf("Horizontal Banks = %d\n", H_Banks);
  printf("Vertical Banks = %d\n", V_Banks);

  cm.Create_Kernel_BlockMask(pExcessFlowIndex, pHeightIndex, pBlockMaskIndex,
                             FrameWidth, FrameHeight, BlkRows, BlkCols);

#define COMBINED_ENQUEUE 1
#ifdef COMBINED_ENQUEUE
  printf("Multiple kernels per enqueue\n\n");
  TS_Type = 1;
  cm.Create_Kernel_Init_Height(pHeightIndex, FrameWidth, FrameHeight, TS_Type);
  cm.Create_Kernel_Relabel(pBlockMaskIndex, pExcessFlowIndex, pHeightIndex,
                           pWestCapIndex, pNorthCapIndex, pEastCapIndex,
                           pSouthCapIndex, FrameWidth, FrameHeight, TS_Type);
  cm.Create_Kernel_Global_Relabel_NR(
      pBlockMaskIndex, pHeightIndex, pWestCapIndex, pNorthCapIndex,
      pEastCapIndex, pSouthCapIndex, FrameWidth, FrameHeight, TS_Type);
  cm.Create_Kernel_Global_Relabel(pBlockMaskIndex, pHeightIndex, pWestCapIndex,
                                  pNorthCapIndex, pEastCapIndex, pSouthCapIndex,
                                  pStatusIndex, FrameWidth, FrameHeight,
                                  TS_Type);
  cm.Create_Kernel_V_Push_NR_VWF(pExcessFlowIndex, pHeightIndex, pNorthCapIndex,
                                 pSouthCapIndex, FrameWidth, FrameHeight,
                                 TS_Type, V_Banks);
  cm.Create_Kernel_V_Push_VWF(pExcessFlowIndex, pHeightIndex, pNorthCapIndex,
                              pSouthCapIndex, pStatusIndex, FrameWidth,
                              FrameHeight, TS_Type, V_Banks);
  cm.Create_Kernel_H_Push_NR_VWF(pExcessFlowIndex, pHeightIndex, pWestCapIndex,
                                 pEastCapIndex, FrameWidth, FrameHeight,
                                 TS_Type, H_Banks);
  cm.Create_Kernel_H_Push_VWF(pExcessFlowIndex, pHeightIndex, pWestCapIndex,
                              pEastCapIndex, pStatusIndex, FrameWidth,
                              FrameHeight, TS_Type, H_Banks);

  TotalStartTime = GetTimeMS();

  // Build block mask. pStatus[0] = active pixels, pStatus[1] = active blocks
  cm.Enqueue_One_Kernel(cm.pKernel_BlockMask, cm.pTS_BlockMask);
  pBlockMaskSurf->ReadSurface((unsigned char *)pBlockMask, cm.pEvent);

#ifdef _DEBUG
  sprintf(GPU_fn, "_0_GPU_BlockMask.%dx%d.Y8", BlkCols, BlkRows);
  Dump2File(GPU_fn, pBlockMask, BlkCols * BlkRows);
#endif

  iter = cm.Enqueue_GC_RL_VPush_HPush_NR(nRatio);
  if (iter == -1)
    exit(-1);

#ifdef _DEBUG
  DWORD dwTimeOutMs01 = -1;
  result = cm.pEvent->WaitForTaskFinished(dwTimeOutMs01);
  if (result != CM_SUCCESS) {
    printf("CM WaitForTaskFinished error: %d.\n", result);
    return -1;
  }
  result = pHeightSurf->ReadSurface((unsigned char *)pHeight, cm.pEvent);
  sprintf(GPU_fn, "_%d_GPU_Height.%dx%d.Y8", iter,
          sizeof(HEIGHT_TYPE) * FrameWidth, FrameHeight);
  Dump2File(GPU_fn, (unsigned char *)pHeight,
            FrameWidth * FrameHeight * sizeof(HEIGHT_TYPE));
  printf("%d ", iter);

  result = pBlockMaskSurf->ReadSurface((unsigned char *)pBlockMask, cm.pEvent);
  sprintf(GPU_fn, "_%d_GPU_BlockMask.%dx%d.Y8", iter, BlkCols,
          BlkRows);
  Dump2File(GPU_fn, pBlockMask, BlkCols * BlkRows);
#endif

#ifdef _DEBUG
  printf("\n");
#endif

  int NextGlobalReabel = nRatio;

  do {
    pStatus[0] = pStatus[1] = 0;

#ifdef _DEBUG
    T1 = GetTimeMS();
#endif
    if (iter < NextGlobalReabel) {
      result = cm.Enqueue_GC_RL_VPush_HPush();
      if (result > 0)
        iter += result;
      else
        exit(-1);
    } else {
      // Replace relabel with global relabel every 10 iterations.
      cm.Enqueue_GC_InitH_GlobalRL(pStatus);
      cm.Enqueue_One_Kernel(cm.pKernel_BlockMask,
                            cm.pTS_BlockMask); // Build block mask
      cm.Enqueue_GC_VPush_HPush();
      NextGlobalReabel = iter + 10;
      iter++;
    }
    DWORD dwTimeOutMs = -1;
    result = cm.pEvent->WaitForTaskFinished(dwTimeOutMs);
    if (result != CM_SUCCESS) {
      printf("CM WaitForTaskFinished error: %d.\n", result);
      return -1;
    }

#ifdef _DEBUG
    T2 = GetTimeMS();

    // Status[0] = active nodes, Status[1] = active blocks
    printf("%d: %d, %d, %8.4f ms\n", iter, pStatus[0], pStatus[1], T2 - T1);
#endif
  } while (pStatus[0] > ActivePixThreshold && iter < 256);

#endif // COMBINED_ENQUEUE

//#define INDIVIDUAL_ENQUEUE    1
#ifdef INDIVIDUAL_ENQUEUE
  printf("One kernel per enqueue\n\n");
  TS_Type = 0;
  cm.Create_Kernel_Init_Height(pHeightIndex, FrameWidth, FrameHeight, TS_Type);
  cm.Create_Kernel_Relabel(pBlockMaskIndex, pExcessFlowIndex, pHeightIndex,
                           pWestCapIndex, pNorthCapIndex, pEastCapIndex,
                           pSouthCapIndex, FrameWidth, FrameHeight, TS_Type);
  cm.Create_Kernel_Global_Relabel_NR(
      pBlockMaskIndex, pHeightIndex, pWestCapIndex, pNorthCapIndex,
      pEastCapIndex, pSouthCapIndex, FrameWidth, FrameHeight, TS_Type);
  cm.Create_Kernel_Global_Relabel(pBlockMaskIndex, pHeightIndex, pWestCapIndex,
                                  pNorthCapIndex, pEastCapIndex, pSouthCapIndex,
                                  pStatusIndex, FrameWidth, FrameHeight,
                                  TS_Type);

  cm.Create_Kernel_V_Push_NR_VWF(pExcessFlowIndex, pHeightIndex, pNorthCapIndex,
                                 pSouthCapIndex, FrameWidth, FrameHeight,
                                 TS_Type, V_Banks);
  cm.Create_Kernel_V_Push_VWF(pExcessFlowIndex, pHeightIndex, pNorthCapIndex,
                              pSouthCapIndex, pStatusIndex, FrameWidth,
                              FrameHeight, TS_Type, V_Banks);
  cm.Create_Kernel_H_Push_NR_VWF(pExcessFlowIndex, pHeightIndex, pWestCapIndex,
                                 pEastCapIndex, FrameWidth, FrameHeight,
                                 TS_Type, H_Banks);
  cm.Create_Kernel_H_Push_VWF(pExcessFlowIndex, pHeightIndex, pWestCapIndex,
                              pEastCapIndex, pStatusIndex, FrameWidth,
                              FrameHeight, TS_Type, H_Banks);

  TotalStartTime = GetTimeMS();

  // Build block mask
  cm.Enqueue_One_Kernel(cm.pKernel_BlockMask, cm.pTS_BlockMask);
  pBlockMaskSurf->ReadSurface((unsigned char *)pBlockMask, cm.pEvent);

  HEIGHT_TYPE HEIGHT_MAX = min(FrameWidth * FrameHeight, TYPE_MAX - 1);

  for (iter = 1; iter < nRatio; iter++) {
    cm.Enqueue_One_Kernel(cm.pKernel_Relabel, cm.pTS_Relabel); // Relabel

#ifdef _DEBUG
    DWORD dwTimeOutMs01 = -1;
    result = cm.pEvent->WaitForTaskFinished(dwTimeOutMs01);
    if (result != CM_SUCCESS) {
      printf("CM WaitForTaskFinished error: %d.\n", result);
      return -1;
    }
    pHeightSurf->ReadSurface((unsigned char *)pHeight, cm.pEvent);
    sprintf(GPU_fn, "_%d_GPU_Height.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * FrameWidth, FrameHeight);
    Dump2File(GPU_fn, (unsigned char *)pHeight,
              FrameWidth * FrameHeight * sizeof(HEIGHT_TYPE));

    pBlockMaskSurf->ReadSurface((unsigned char *)pBlockMask, cm.pEvent);
    sprintf(GPU_fn, "_%d_GPU_BlockMask.%dx%d.Y8", iter, BlkCols,
            BlkRows);
    Dump2File(GPU_fn, pBlockMask, BlkCols * BlkRows);
#endif
    cm.Enqueue_One_Kernel(cm.pKernel_V_Push_NR,
                          cm.pTS_V_Push_NR); // Vertical push
    cm.Enqueue_One_Kernel(cm.pKernel_H_Push_NR,
                          cm.pTS_H_Push_NR); // Horizontal push

#ifdef _DEBUG
    printf("%d ", iter);
#endif
  } // for

#ifdef _DEBUG
  printf("\n");
#endif

  int NextGlobalReabel = nRatio;

  do {
    memset(pStatus, 0, 8 * sizeof(int));

    if (iter != NextGlobalReabel) {
      cm.Enqueue_One_Kernel(cm.pKernel_Relabel, cm.pTS_Relabel); // Relabel
    } else {
      cm.Enqueue_One_Kernel(cm.pKernel_Init_Height,
                            cm.pTS_Init_Height); // Init Height
      cm.Enqueue_GC_Global_Relabel(pStatus);     // Global relabel
      cm.Enqueue_One_Kernel(cm.pKernel_BlockMask,
                            cm.pTS_BlockMask); // Build block mask
      NextGlobalReabel = iter + nRatio;
    }

#ifdef _DEBUG
    DWORD dwTimeOutMs01 = -1;
    result = cm.pEvent->WaitForTaskFinished(dwTimeOutMs01);
    if (result != CM_SUCCESS) {
      printf("CM WaitForTaskFinished error: %d.\n", result);
      return -1;
    }
    result = pHeightSurf->ReadSurface((unsigned char *)pHeight, cm.pEvent);
    sprintf(GPU_fn, "_%d_GPU_Height.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * FrameWidth, FrameHeight);
    Dump2File(GPU_fn, (unsigned char *)pHeight,
              FrameWidth * FrameHeight * sizeof(HEIGHT_TYPE));

    result =
        pBlockMaskSurf->ReadSurface((unsigned char *)pBlockMask, cm.pEvent);
    sprintf(GPU_fn, "_%d_GPU_BlockMask.%dx%d.Y8", iter, BlkCols,
            BlkRows);
    Dump2File(GPU_fn, pBlockMask, BlkCols * BlkRows);
#endif
    cm.Enqueue_One_Kernel(cm.pKernel_V_Push, cm.pTS_V_Push); // Vertical push
    cm.Enqueue_One_Kernel(cm.pKernel_H_Push, cm.pTS_H_Push); // Horizontal push

    DWORD dwTimeOutMs = -1;
    result = cm.pEvent->WaitForTaskFinished(dwTimeOutMs);
    if (result != CM_SUCCESS) {
      printf("CM WaitForTaskFinished error: %d.\n", result);
      return -1;
    }

#ifdef _DEBUG
    // Status[0] = active nodes, Status[1] = active blocks
    printf("%d: active pixels = %d, active blocks = %d\n", iter, pStatus[0],
           pStatus[1]);
#endif
  } while (pStatus[0] > ActivePixThreshold && ++iter < 256);

#endif // Individual queue

  // Common code
  // Process remaining iterations if any or copy back to system memory
  DWORD dwTimeOutCopy = -1;
  CmEvent *pNoEvent = CM_NO_EVENT;

  CopyStartTime = GetTimeMS();

  cm.pCmQueue->EnqueueCopyGPUToCPU(pHeightSurf, (unsigned char *)pHeight,
                                   cm.pEvent);
  result = cm.pEvent->WaitForTaskFinished(dwTimeOutCopy);
  if (result != CM_SUCCESS) {
    printf("CM WaitForTaskFinished error: %d.\n", result);
    return -1;
  }
  CopyEndTime = GetTimeMS();

  ////////////////////////////////////////
  TotalEndTime = GetTimeMS();

  printf("GPU Graph-Cut loops = %d\n", iter);
  printf("Copy time = %f ms\n", fabs(CopyEndTime - CopyStartTime));
  printf("Total Time: %f ms\n\n", TotalEndTime - TotalStartTime);

  HEIGHT_MAX = min(FrameWidth * FrameHeight, TYPE_MAX - 1);

  // Convert output short to black white byte image
  for (int j = 0; j < OutputFrameSize; j++)
    *(pOutputGPU + j) = *(pHeight + j) < HEIGHT_MAX ? 0 : 255;

  sprintf(GPU_fn, "%s_Output_GPU.%dx%d.Y8", prefix, FrameWidth,
          FrameHeight);
  Dump2File(GPU_fn, (unsigned char *)pOutputGPU, OutputFrameSize);

  sprintf(diff_fn, "%s_CPU_GPU_DIFF.txt", prefix);
  Comp2ImageFileByte(CPU_fn, GPU_fn, diff_fn, FrameWidth, FrameWidth,
                     FrameHeight);

  for (int i = 0; i < NUM_FILES; i++)
    cm.AlignedFree(iBuffer[i]);

  cm.AlignedFree(pStatus);
  cm.AlignedFree(pOutputCPU);
  cm.AlignedFree(pOutputGPU);
  cm.AlignedFree(pExcessFlow);
  cm.AlignedFree(pWestCap);
  cm.AlignedFree(pNorthCap);
  cm.AlignedFree(pEastCap);
  cm.AlignedFree(pSouthCap);
  cm.AlignedFree(pHeight);
  cm.AlignedFree(pBlockMask);
  cm.AlignedFree(pDebug);
  cm.AlignedFree(iOrigBuffer);

  return 0;
}
