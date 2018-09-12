#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <io.h>
#include <direct.h>

#include "GC_API.h"  
#include "Graph_Cut_host.h"
#include "General.h"

#define VERTICAL_PUSH       0
#define HORIZONTAL_PUSH     1

#define NUM_FILES			6
#define ALIGNMENT32			32
#define ALIGNMENT4K			4096

/////////////////////////////////////////////////////////////////////////////////
// GPU API

// Detect if MDF exists on this system.  MDF_Existence = true is MDF exist.

int _GC::Detect_MDF()
{
	char * buffer;
	DWORD length;

	// Find new MDF driver in registry key
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Intel\\MDF", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		if(RegQueryValueExA(hKey, "DriverStorePath", NULL, NULL, NULL, &length) == ERROR_SUCCESS) {
		    buffer = (char *)LocalAlloc(LPTR, length);
		    // query
			if(RegQueryValueExA(hKey, "DriverStorePath", NULL, NULL, (BYTE *) buffer, &length) == ERROR_SUCCESS) {
			    RegCloseKey(hKey);
				if (strstr(buffer, "igdlh64.inf_")) {
					printf("Detected MDF driver on this system\n");
					return true;
				}
			}
			LocalDiscard(buffer);
		}
	}


	size_t ReturnValue;
	length = 4096;
    buffer = (char *)LocalAlloc(LPTR, length);
	// Find older MDF driver in location
	getenv_s(&ReturnValue, buffer, length, "SystemRoot");
	
	char FullPath[8192];
	sprintf(FullPath, "%s\\System32\\igfxcmrt64.dll", buffer); 
	int status = _access(FullPath, 0);
	if (status == 0) {
		printf("Detected MDF driver on this system.\n");
		return true;
	}
	LocalDiscard(buffer);
	printf("Could not detect MDF driver on this system.\n");
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int _GC::CM_GraphCut( short * pNodes, short * pHoriWeights, short * pVertWeights, unsigned char * pOutput, int FrameWidth, int FrameHeight )  
{  
	MDF_GC cm;    // CM Context

	HEIGHT_TYPE HEIGHT_MAX;

	unsigned int pitch_inputSurf;
	unsigned int size_inputSurf;

	int BlkWidth = 8;
	int BlkHeight = 8;
	int nRatio = 32;

	// Graph cut internal buffers
	int wBufferSize = sizeof(short) * FrameHeight * FrameWidth;
    short * pExcessFlow = (short *) _aligned_malloc( wBufferSize, ALIGNMENT32 ); 
    short * pWestCap = (short *) _aligned_malloc( wBufferSize, ALIGNMENT32 ); 
    short * pNorthCap = (short *) _aligned_malloc( wBufferSize, ALIGNMENT32 ); 
    short * pEastCap = (short *) _aligned_malloc( wBufferSize, ALIGNMENT32 ); 
    short * pSouthCap = (short *) _aligned_malloc( wBufferSize, ALIGNMENT32 ); 

	int iBufferSize = sizeof(HEIGHT_TYPE) * FrameHeight * FrameWidth;
    HEIGHT_TYPE * pHeight = (HEIGHT_TYPE *) _aligned_malloc( iBufferSize, ALIGNMENT32 ); 

	// 1D status buffer
	unsigned int StatusSize = 8 * sizeof(int);
	int * pStatus = (int *) _aligned_malloc(StatusSize, ALIGNMENT4K);
    if (pStatus == NULL) {
        printf("pStatus allocation failed.\n");
		exit(-1);
    }
	memset(pStatus, 0, 8*sizeof(int));

    int BlkCols = (int) (((float) FrameWidth) / BlkWidth + 0.5f);
    int BlkRows = (int) (((float) FrameHeight) / BlkHeight + 0.5f);
	unsigned char * pBlockMask = (unsigned char *) cm.AlignedMalloc( BlkCols * BlkRows, ALIGNMENT4K );
    if (pBlockMask == NULL) {
        printf("pBlockMask allocation failed.\n");
		exit(-1);
    }
	memset(pBlockMask, 0, BlkCols * BlkRows);

	cm.pCmDev->GetSurface2DInfo(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8, pitch_inputSurf, size_inputSurf);
	short * pDebug = (short *) _aligned_malloc(size_inputSurf, ALIGNMENT4K);		
    if (pDebug == NULL) {
        printf("pDebug allocation failed.\n");
		exit(-1);
    }
	memset(pDebug, 0, size_inputSurf);

	int size = FrameWidth*FrameHeight / (SIDE_SQUARE*SIDE_SQUARE) + 1;
	RelabelBlock * tableBlock = (RelabelBlock *) _aligned_malloc(sizeof(RelabelBlock) * size, ALIGNMENT32);
	memset(tableBlock, -1, sizeof(RelabelBlock) * size);

	////////////////////////////////////////////////////////////////////////////////////////////////
    // Allocate input surface
	//CM_SURFACE_FORMAT InputD3DFMT = CM_SURFACE_FORMAT_A8;
    CmSurface2D * pInputSurf = NULL;
    SurfaceIndex * pInputIndex = NULL;
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_A8, &pInputSurf, &pInputIndex);

    // Allocate input surface2
    CmSurface2D * pInputSurf2 = NULL;
    SurfaceIndex * pInputIndex2 = NULL;
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_A8, &pInputSurf2, &pInputIndex2);

    // Allocate input surface3
    CmSurface2D * pInputSurf3 = NULL;
    SurfaceIndex * pInputIndex3 = NULL;
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_A8, &pInputSurf3, &pInputIndex3);


    // Input suirfaces for Push-Relabel

    CmSurface2D * pBlockMaskSurf = NULL;
    SurfaceIndex * pBlockMaskIndex = NULL;
    cm.AllocGPUSurface(BlkCols, BlkRows, CM_SURFACE_FORMAT_A8, &pBlockMaskSurf, &pBlockMaskIndex);
	
    //InputD3DFMT = CM_SURFACE_FORMAT_A8R8G8B8;  // D3DFMT_V8U8; 
    CmSurface2D * pHeightSurf = NULL;
    SurfaceIndex * pHeightIndex = NULL;
	if (sizeof(HEIGHT_TYPE) == 2)
		cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8, &pHeightSurf, &pHeightIndex);
	else
		cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_A8R8G8B8, &pHeightSurf, &pHeightIndex);

    //InputD3DFMT = CM_SURFACE_FORMAT_V8U8;
    CmSurface2D * pExcessFlowSurf = NULL;
    SurfaceIndex * pExcessFlowIndex = NULL;
#ifdef CM_DX9
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_L16, &pExcessFlowSurf, &pExcessFlowIndex);	// L16 for signed short, V8U8 for unsigned short
#endif
#ifdef CM_DX11
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_R16_SINT, &pExcessFlowSurf, &pExcessFlowIndex);	// R16_SINT for signed short, R16_UINT for unsigned short
#endif

    CmSurface2D * pWestCapSurf = NULL;
    SurfaceIndex * pWestCapIndex = NULL;
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8, &pWestCapSurf, &pWestCapIndex);

    CmSurface2D * pNorthCapSurf = NULL;
    SurfaceIndex * pNorthCapIndex = NULL;
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8, &pNorthCapSurf, &pNorthCapIndex);

    CmSurface2D * pEastCapSurf = NULL;
    SurfaceIndex * pEastCapIndex = NULL;
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8, &pEastCapSurf, &pEastCapIndex);

    CmSurface2D * pSouthCapSurf = NULL;
    SurfaceIndex * pSouthCapIndex = NULL;
    cm.AllocGPUSurface(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_V8U8, &pSouthCapSurf, &pSouthCapIndex);

	// Allocate debug surface
	CmSurface2DUP * pDebugSurf = NULL;
    SurfaceIndex * pDebugIndex = NULL;
#ifdef CM_DX9
    cm.AllocGPUSurfaceUP(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_L16, (unsigned char *) pDebug, &pDebugSurf, &pDebugIndex);
#endif
#ifdef CM_DX11
    cm.AllocGPUSurfaceUP(FrameWidth, FrameHeight, CM_SURFACE_FORMAT_R16_SINT, (unsigned char *) pDebug, &pDebugSurf, &pDebugIndex);
#endif

    // Status buffer
    CmBufferUP * pStatusBuf = NULL;
    SurfaceIndex * pStatusIndex = NULL;
    cm.AllocGPUBufferUP(StatusSize, (unsigned char *) pStatus, &pStatusBuf, &pStatusIndex);

	// Init input surface with input images.  Could use GPU copy for higher performance
	int result = pInputSurf->WriteSurface( (unsigned char *) pNodes, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pInputSurf2->WriteSurface( (unsigned char *) pHoriWeights, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pInputSurf3->WriteSurface( (unsigned char *) pVertWeights, NULL ); // mask
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }

    //result = pDebugSurf->WriteSurface( (unsigned char *) pDebug, NULL ); // mask
    //if (result != CM_SUCCESS ) {
    //    perror("CM WriteSurface error");
    //    return -1;
    //}

    // Graph cut inputs

    PushRelabel_Init(pNodes, pHoriWeights, pVertWeights, FrameHeight, FrameWidth, 
					pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap);

	memset(pBlockMask, 0, BlkCols * BlkRows);
	result = pBlockMaskSurf->WriteSurface( (unsigned char *) pBlockMask, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }

    result = pExcessFlowSurf->WriteSurface( (unsigned char *) pExcessFlow, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pHeightSurf->WriteSurface( (unsigned char *) pHeight, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pWestCapSurf->WriteSurface( (unsigned char *) pWestCap, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pNorthCapSurf->WriteSurface( (unsigned char *) pNorthCap, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pEastCapSurf->WriteSurface( (unsigned char *) pEastCap, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pSouthCapSurf->WriteSurface( (unsigned char *) pSouthCap, NULL );
    if (result != CM_SUCCESS ) {
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

#ifdef _DEBUG
	printf("Height type is %s\n", (sizeof(HEIGHT_TYPE) == 2) ? "short" : "int");
#endif

	// Frame width = 320, valid values = 1, 2, 4, 5, 8 or 10.
	// Frame height = 240, valide values = 1, 2, 3, 5, 6 or 10.
	// Need evenly dividable by 8x8 blocks
	int H_Banks = (FrameWidth >= 1280) ? 5 : 4;
	int V_Banks = (FrameHeight >= 720) ? 5 : 3;
#ifdef _DEBUG
	printf("Horizontal Banks = %d\n", H_Banks);
	printf("Vertical Banks = %d\n", V_Banks);
#endif

	cm.Create_Kernel_BlockMask(pExcessFlowIndex, pHeightIndex, pBlockMaskIndex, FrameWidth, FrameHeight, BlkRows, BlkCols);

#define COMBINED_ENQUEUE 1
#ifdef COMBINED_ENQUEUE
//	printf("Multiple kernels per enqueue\n\n");
	TS_Type = 1;
	cm.Create_Kernel_Init_Height(pHeightIndex, FrameWidth, FrameHeight, TS_Type);
	cm.Create_Kernel_Relabel(pBlockMaskIndex, pExcessFlowIndex, pHeightIndex, pWestCapIndex, pNorthCapIndex, pEastCapIndex, pSouthCapIndex, 
							FrameWidth, FrameHeight, TS_Type);
	cm.Create_Kernel_Global_Relabel_NR(pBlockMaskIndex, pHeightIndex, pWestCapIndex, pNorthCapIndex, pEastCapIndex, pSouthCapIndex, FrameWidth, FrameHeight, TS_Type);
	cm.Create_Kernel_Global_Relabel(pBlockMaskIndex, pHeightIndex, pWestCapIndex, pNorthCapIndex, pEastCapIndex, pSouthCapIndex, pStatusIndex, FrameWidth, FrameHeight, TS_Type);
	cm.Create_Kernel_V_Push_NR_VWF(pExcessFlowIndex, pHeightIndex, pNorthCapIndex, pSouthCapIndex, FrameWidth, FrameHeight, TS_Type, V_Banks);
	cm.Create_Kernel_V_Push_VWF(pExcessFlowIndex, pHeightIndex, pNorthCapIndex, pSouthCapIndex, pStatusIndex, FrameWidth, FrameHeight, TS_Type, V_Banks);
	cm.Create_Kernel_H_Push_NR_VWF(pExcessFlowIndex, pHeightIndex, pWestCapIndex, pEastCapIndex, FrameWidth, FrameHeight, TS_Type, H_Banks);
	cm.Create_Kernel_H_Push_VWF(pExcessFlowIndex, pHeightIndex, pWestCapIndex, pEastCapIndex, pStatusIndex, FrameWidth, FrameHeight, TS_Type, H_Banks);

	TotalStartTime = GetTimeMS();

	// Build block mask. pStatus[0] = active pixels, pStatus[1] = active blocks
	cm.Enqueue_One_Kernel(cm.pKernel_BlockMask, cm.pTS_BlockMask);
	pBlockMaskSurf->ReadSurface( (unsigned char *) pBlockMask, cm.pEvent );

#ifdef _DEBUG
    sprintf(GPU_fn, ".\\Output\\_0_GPU_BlockMask.%dx%d.Y8", BlkCols, BlkRows);
    Dump2File(GPU_fn, pBlockMask, BlkCols*BlkRows);
#endif

	ActiveBlocks = GetActiveBlocks(pBlockMask, BlkRows, BlkCols);

	// GPU path
		iter = cm.Enqueue_GC_RL_VPush_HPush_NR(nRatio);
		if (iter == -1) 
			exit(-1);
        
        int NextGlobalReabel = nRatio;
        do {
			pStatus[0] = pStatus[1] = 0;

            if (iter < NextGlobalReabel) {
				result = cm.Enqueue_GC_RL_VPush_HPush();
				if (result > 0) 
					iter += result;
				else 
					exit(-1);
			} else {
				// Replace relabel with global relabel every 10 iterations.
				cm.Enqueue_GC_InitH_GlobalRL(pStatus);
				cm.Enqueue_One_Kernel(cm.pKernel_BlockMask, cm.pTS_BlockMask);	// Build block mask
				cm.Enqueue_GC_VPush_HPush();
				NextGlobalReabel = iter + 10;
				iter++;
			}
			DWORD dwTimeOutMs = -1;
			result = cm.pEvent->WaitForTaskFinished(dwTimeOutMs);
			if (result != CM_SUCCESS ) {
				printf("CM WaitForTaskFinished error: %d.\n", result);
				return -1;
			}

		} while (pStatus[0] > 0 && iter < 512);		// 256

#endif	// COMBINED_ENQUEUE
		DWORD dwTimeOutCopy = -1;
		AVX2StartTime = AVX2EndTime = 0.0;
		cm.pCmQueue->EnqueueCopyGPUToCPU(pHeightSurf, (unsigned char *) pHeight, cm.pEvent);
		result = cm.pEvent->WaitForTaskFinished(dwTimeOutCopy);
		if (result != CM_SUCCESS) {
			printf("CM WaitForTaskFinished error: %d.\n", result);
			return -1;
		} 

	////////////////////////////////////////
	TotalEndTime = GetTimeMS();

	printf("GPU Graph-Cut loops = %d\n", iter);
	printf("Total Time: %f ms\n\n", TotalEndTime - TotalStartTime);

    HEIGHT_MAX = min(FrameWidth * FrameHeight, TYPE_MAX-1);

    // Convert output short to black white byte image
    for (int j = 0; j < FrameWidth * FrameHeight; j++) 
        *(pOutput+j) = *(pHeight+j) < HEIGHT_MAX ? 0 : 255;

//	char GPU_fn[128];
//  sprintf(GPU_fn, ".\\Output\\%s_Output_GPU.%dx%d.Y8", prefix, FrameWidth, FrameHeight);
//    Dump2File(GPU_fn, (unsigned char *) pOutputGPU, OutputFrameSize);


	cm.AlignedFree(pStatus);
	cm.AlignedFree(pExcessFlow);
	cm.AlignedFree(pWestCap);
	cm.AlignedFree(pNorthCap);
	cm.AlignedFree(pEastCap);
	cm.AlignedFree(pSouthCap);
	cm.AlignedFree(pHeight);
	cm.AlignedFree(pBlockMask);
    cm.AlignedFree(pDebug);

	return 0;
}
