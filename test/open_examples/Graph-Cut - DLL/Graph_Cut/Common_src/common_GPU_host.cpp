#ifdef WIN32
#include "stdafx.h"
#endif
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
//#include <cm/cm.h>

#include "MDF_Base.h"

extern double mygetTickFrequency( void );
__int64 TimeStamp[20];


#ifdef CMRT_EMU
extern "C" void Sobel_3x3_8u16s(SurfaceIndex iIndex, SurfaceIndex oIndex); 
extern "C" void SobelEdge_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex); 
extern "C" void Scaling16x16(SamplerIndex sIndex, SurfaceIndex iIndex, SurfaceIndex oIndex, float DeltaU, float DeltaV);
extern "C" void RGBXToNV12(SurfaceIndex iIndex, SurfaceIndex oIndexNV12, int FrameHeight);
extern "C" void NV12toYUY2(SurfaceIndex iNV12Index, SurfaceIndex oYUY2Index, int FrameHeight);
extern "C" void ColorToGray(SurfaceIndex iIndex, SurfaceIndex oIndex, int InputFormat);
extern "C" void Dilate_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex);
extern "C" void Erode_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex);
extern "C" void BinaryMedian_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex);
extern "C" void BinaryMedian_5x5_8u(SurfaceIndex iIndex, SurfaceIndex oIndex); 
extern "C" void BinaryMedian_9x9_8u(SurfaceIndex iIndex, SurfaceIndex oIndex); 
extern "C" void BinaryMedian_Mask_3x3_8u(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex iMaskIndex); 
extern "C" void BinaryMedian_Mask_5x5_8u(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex iMaskIndex); 
extern "C" void BinaryMedian_Mask_9x9_8u(SurfaceIndex iIndex, SurfaceIndex oIndex, SurfaceIndex iMaskIndex); 
extern "C" void MergeRGBA_8u(SurfaceIndex iIndex, SurfaceIndex iIndex2, SurfaceIndex oIndex);
extern "C" void GPU_memcpy(SurfaceIndex iIndex, SurfaceIndex oIndex);
extern "C" void GPU_memcpy_1D(SurfaceIndex iIndex, SurfaceIndex oIndex);
extern "C" void GPU_memcpy_Scattered(SurfaceIndex iIndex, SurfaceIndex oIndex, int ThreadCount);
extern "C" void NULL_kernel(SurfaceIndex iIndex, SurfaceIndex oIndex);
extern "C" void FlipX_DualSurface(SurfaceIndex iIndex, SurfaceIndex oIndex, int width);
//extern "C" void FlipX_SingleSurface(SurfaceIndex iIndex, int width);
extern "C" void RGBA2BGRA(SurfaceIndex iIndex, SurfaceIndex oIndex);
extern "C" void FlipX_RGBA2BGRA(SurfaceIndex iIndex, SurfaceIndex oIndex, int MaxX);
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Common host code
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MDF_Base::EnqueueSobel(int FrameWidth, int FrameHeight, SurfaceIndex * pInputIndex, SurfaceIndex * pOutputIndex)
{
	int result;
    int BlockWidth = 16;
    int BlockHeight = 16;

    // Create kernel Sobel from Cm Program
    pCmKernel = NULL;
//	result = pCmDev->CreateKernel(pCmProgram, _NAME(SobelEdge_3x3_8u) , pCmKernel);
    result = pCmDev->CreateKernel(pCmProgram, _NAME(SobelEdge_3x3_8u) , pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil((float) FrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);	// Set input surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);	// Set output surface index

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}

////////////////////////////////////////////////////////////////////////
int MDF_Base::EnqueueScale(int FrameWidth, int FrameHeight, 
                SamplerIndex * pSamplerIndex, SurfaceIndex * pInputIndex, SurfaceIndex * pOutputIndex, float fScalingFactor)
{
	int result;
    int BlockWidth = 16;
    int BlockHeight = 16;

    TimeStamp[0] = __rdtsc();

    // Create kernel Scaling from Cm Program
    CmKernel* kernel = NULL;
//	if (BlockHeight == 4 && BlockWidth == 8)
//		result = pCmDev->CreateKernel(pCmProgram, _NAME(Scaling4x8) , pCmKernel);
	if (BlockHeight == 16 && BlockWidth == 16)
		result = pCmDev->CreateKernel(pCmProgram, _NAME(Scaling16x16) , pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }
    	
    TimeStamp[1] = __rdtsc();

    // Normalized step size
	float fScalingStepSizeX = 1.0f/fScalingFactor/FrameWidth;
	float fScalingStepSizeY = 1.0f/fScalingFactor/FrameHeight;

	int OutputFrameWidth = (int) (FrameWidth * fScalingFactor + 0.5f);
	int OutputFrameHeight = (int) (FrameHeight * fScalingFactor + 0.5f);

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil((float)OutputFrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float)OutputFrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

    TimeStamp[2] = __rdtsc();

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SamplerIndex), pSamplerIndex);		// Curbe data 
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);		// Curbe data
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);		// Curbe data
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(fScalingStepSizeX), &fScalingStepSizeX);	// Curbe data
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(fScalingStepSizeY), &fScalingStepSizeY);	// Curbe data

    pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

    TimeStamp[3] = __rdtsc();

	// Create a task (container) to be put in the task queue
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		printf("CmDevice CreateTask error");
		return -1;
	}

    TimeStamp[4] = __rdtsc();

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		printf("CmDevice AddKernel error");
		return -1;
	}

    TimeStamp[5] = __rdtsc();

    // Put kernel array into task queue to be executed on GPU
    result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		printf("CmDevice enqueue error");
		return -1;
	}

    TimeStamp[6] = __rdtsc();

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    TimeStamp[7] = __rdtsc();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::EnqueueRGBToNV12(int FrameWidth, int FrameHeight, SurfaceIndex * pRGBXIndex, SurfaceIndex * pNV12Index)
{
	int result;
    int BlockWidth = 8;
    int BlockHeight = 8;

    pCmKernel = NULL;
	result = pCmDev->CreateKernel(pCmProgram, _NAME(RGBXToNV12) , pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil((float) FrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pRGBXIndex);    // Set RGBX input surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pNV12Index);	// Set NV12 output surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(FrameHeight), &FrameHeight);	// Offset from Y origin to UV origin

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::Enqueue_NV12toYUY2(SurfaceIndex * pInputIndex, SurfaceIndex * pOutputIndex, int FrameWidth, int FrameHeight)
{
	int result;
    int BlockWidth = 16;
    int BlockHeight = 16;

    pCmKernel = NULL;
	result = pCmDev->CreateKernel(pCmProgram, _NAME(NV12toYUY2) , pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil((float) FrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(FrameHeight), &FrameHeight);	    // Offset from Y origin to UV origin

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::Enqueue_ColorToGray(SurfaceIndex * pIndex, SurfaceIndex * pIndex2, int FrameWidth, int FrameHeight, int InputFormat)
{
	int result;
    int BlockWidth = 24;    // 3 bytes color, RGB/BGR
    int BlockHeight = 8;

    pCmKernel = NULL;
	result = pCmDev->CreateKernel(pCmProgram, _NAME(ColorToGray) , pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil(3.0f * FrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pIndex);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pIndex2);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(InputFormat), &InputFormat);   // 0: RGB, 1: BGR

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::EnqueueMorphologicalFilter(SurfaceIndex * pInputIndex, SurfaceIndex * pOutputIndex, int FrameWidth, int FrameHeight, int FilterType)
{
	int result;
    int BlockWidth = 16;
    int BlockHeight = 16;

    // Create kernel from Cm Program
    pCmKernel = NULL;
    if (FilterType == FILTER_DILATE)
        result = pCmDev->CreateKernel(pCmProgram, _NAME(Dilate_3x3_8u), pCmKernel);
    else if (FilterType == FILTER_ERODE)
        result = pCmDev->CreateKernel(pCmProgram, _NAME(Erode_3x3_8u), pCmKernel);
    else {
        perror("Invalid filter type");
        return -1;
    }

	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil((float) FrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);	// Set input surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);	// Set output surface index

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}

////////////////////////////////////////////////////////////////////////
int MDF_Base::EnqueueBinaryMedian(SurfaceIndex * pInputIndex, SurfaceIndex * pOutputIndex, SurfaceIndex * pMaskIndex,
                        int FrameWidth, int FrameHeight, int winSize, bool bMaskOn)
{
	int result;
    int BlockWidth = 16;
    int BlockHeight = 16;
	
    char kname[256];
    pCmKernel = NULL;

    if (winSize == 3 || winSize == 5 || winSize == 9) { 
        if (bMaskOn) {
	        sprintf(kname, "BinaryMedian_Mask_%dx%d_8u", winSize, winSize);
            result = pCmDev->CreateKernel(pCmProgram, kname, pCmKernel);
        } else {
	        sprintf(kname, "BinaryMedian_%dx%d_8u", winSize, winSize);
            result = pCmDev->CreateKernel(pCmProgram, kname, pCmKernel);
        }
    	if (result != CM_SUCCESS ) {
            perror("CM CreateKernel error");
            return -1;
        }
    } else {
        printf("Invalid filter kernel size: %d\n", winSize);
        return -1;
    }

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil((float) FrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);	// Set input surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);	// Set output surface index

    if (bMaskOn)
        pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pMaskIndex);

    pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}

////////////////////////////////////////////////////////////////////////
int MDF_Base::EnqueueMergeRGBA(int FrameWidth, int FrameHeight, SurfaceIndex * pRGBIndex, SurfaceIndex * pAlphaIndex, SurfaceIndex * pOutputIndex)
{
	int result;
    int BlockWidth = 8;
    int BlockHeight = 8;

    pCmKernel = NULL;
	result = pCmDev->CreateKernel(pCmProgram, _NAME(MergeRGBA_8u) , pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil((float) FrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pRGBIndex);	// Set RGB surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pAlphaIndex);	// Set alpha surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);	// Set output surface index

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
int MDF_Base::Enqueue_GPU_memcpy(SurfaceIndex * pInputIndex, SurfaceIndex * pOutputIndex, int FrameWidth, int FrameHeight)
{
	int result;
    int BlockWidth = 32;
    int BlockHeight = 8;

    // Create kernel from Cm Program
    pCmKernel = NULL;
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GPU_memcpy), pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil((float) FrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);	// Set input surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);	// Set output surface index

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	pCmDev->DestroyTask(pKernelArray); pKernelArray = 0;
    pCmDev->DestroyThreadSpace(pThrdSpace); pThrdSpace = 0;
    pCmDev->DestroyKernel(pCmKernel); pCmKernel = 0;
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
int MDF_Base::Enqueue_NULL_kernel(SurfaceIndex * pInputIndex, SurfaceIndex * pOutputIndex, int FrameWidth, int FrameHeight)
{
	int result;
    int BlockWidth = 32;
    int BlockHeight = 8;

    TimeStamp[0] = __rdtsc();

    // Create kernel from Cm Program
    pCmKernel = NULL;
    result = pCmDev->CreateKernel(pCmProgram, _NAME(NULL_kernel), pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }
    TimeStamp[1] = __rdtsc();
/*
    // Find # of threads based on output frame size in pixel 
//    int threadswidth  = (int) ceil((float) FrameWidth / BlockWidth); 
//    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    int threadswidth  = 16; 
    int threadsheight = 16; 

    pCmKernel->SetThreadCount( threadswidth * threadsheight );

    TimeStamp[2] = __rdtsc();

    // Set curbe data
	int ParaIdx = 0;
//    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);	// Set input surface index
//	  pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);	// Set output surface index
*/
    vector<unsigned short, 2> ScanSize;
    ScanSize[0] = 0;
    ScanSize[1] = 0;

    matrix<float, 4, 4> xfVoxelToProjection;   // Last row unused
    xfVoxelToProjection[0][0] = 0;
    xfVoxelToProjection[0][1] = 0;
    xfVoxelToProjection[0][2] = 0;
    xfVoxelToProjection[0][3] = 0;
    xfVoxelToProjection[1][0] = 0;
    xfVoxelToProjection[1][1] = 0;
    xfVoxelToProjection[1][2] = 0;
    xfVoxelToProjection[1][3] = 0;
    xfVoxelToProjection[2][0] = 0;
    xfVoxelToProjection[2][1] = 0;
    xfVoxelToProjection[2][2] = 0;
    xfVoxelToProjection[2][3] = 0;
    xfVoxelToProjection[3][0] = 0;
    xfVoxelToProjection[3][1] = 0;
    xfVoxelToProjection[3][2] = 0;
    xfVoxelToProjection[3][3] = 0;

    vector<unsigned short, 4> VoxelSpace;
    VoxelSpace[0] = 256;
    VoxelSpace[1] = 256;
    VoxelSpace[2] = 256;
    VoxelSpace[3] = 1;  // Use spare space for args.shiftNormalScan_

    // from run(int x, int y, int min, int max)
    bool hasColor = 1;

    vector<unsigned short, 4> VoxelStride;
    VoxelStride[0] = 1024;
    VoxelStride[1] = 1024;
    VoxelStride[2] = 1024;
    VoxelStride[3] = hasColor;              // Use spare space for hasColor       

    vector<short, 6> MinMax;
    MinMax[0] = 0;
    MinMax[1] = 256;
    MinMax[2] = 0;
    MinMax[3] = 256;
    MinMax[4] = 0;     // zMin
    MinMax[5] = 256;     // zMax

    // One thread processes all z values for GROUP_SIZE*x and 1*y
    int threadswidth  = 256;
    int threadsheight = 256;
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

    TimeStamp[2] = __rdtsc();

	float f;

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);
    
    pCmKernel->SetKernelArg(ParaIdx++, ScanSize.get_size_data(), ScanSize.get_addr_data());
    pCmKernel->SetKernelArg(ParaIdx++, xfVoxelToProjection.get_size_data(), xfVoxelToProjection.get_addr_data());
    pCmKernel->SetKernelArg(ParaIdx++, VoxelSpace.get_size_data(), VoxelSpace.get_addr_data());
    pCmKernel->SetKernelArg(ParaIdx++, VoxelStride.get_size_data(), VoxelStride.get_addr_data());
    pCmKernel->SetKernelArg(ParaIdx++, MinMax.get_size_data(), MinMax.get_addr_data());

    pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &f);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &f);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &f);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &f);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &f);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &f);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &f);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &f);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(float), &f);


	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

    TimeStamp[3] = __rdtsc();

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

    TimeStamp[4] = __rdtsc();

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

    TimeStamp[5] = __rdtsc();

    // Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

    TimeStamp[6] = __rdtsc();

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    TimeStamp[7] = __rdtsc();

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
int MDF_Base::Enqueue_GPU_memcpy_1D(SurfaceIndex * pInputIndex, SurfaceIndex * pOutputIndex, unsigned int size)
{
    int result;
 
    // Create kernel from Cm Program
    pCmKernel = NULL;
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GPU_memcpy_1D), pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Each thread process min 16 bytes, max 128 bytes

    // Each thread process 8 OWORD (128 bytes)
    int ThreadCount = (int) ceil((float) size/128);

    // Each thread process 1 OWORD (16 bytes)
//    int ThreadCount = (int) ceil((float) size/16);

    int threadswidth = (ThreadCount < 511) ? ThreadCount : 511;
    int threadsheight = (ThreadCount - 1) / 511 + 1;
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);	// Set input surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);	// Set output surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadCount);

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}


    TimeStamp[5] = __rdtsc();

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

    TimeStamp[6] = __rdtsc();
    printf( "Enqueue time 1 = %g ms\n", (TimeStamp[6] - TimeStamp[5])/((double) mygetTickFrequency()*1000.) );

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
int MDF_Base::Enqueue_GPU_memcpy_Scattered(SurfaceIndex * pInputIndex, SurfaceIndex * pOutputIndex, unsigned int size)
{
    int result;
 
    // Create kernel from Cm Program
    pCmKernel = NULL;
    result = pCmDev->CreateKernel(pCmProgram, _NAME(GPU_memcpy_Scattered), pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Each thread process 16 DWs
//    int ThreadCount = (int) ceil((float) size/sizeof(int)/16);

    // Each thread process 8 DWs
    int ThreadCount = (int) ceil((float) size/sizeof(int)/8);

    int threadswidth = (ThreadCount < 511) ? ThreadCount : 511;
    int threadsheight = (ThreadCount - 1) / 511 + 1;
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInputIndex);	// Set input surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutputIndex);	// Set output surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadCount);

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::Enqueue_FlipX(SurfaceIndex * pInIndex, SurfaceIndex * pOutIndex, int FrameWidth, int FrameHeight)
{
    int result;
	int WidthInByte = FrameWidth * sizeof(int);
 
	vector<uchar, 32> ReverseWedge;
	for (uchar i = 0; i < 32; i++)
		ReverseWedge[31-i] = i;

    // Create kernel from Cm Program
    pCmKernel = NULL;
    result = pCmDev->CreateKernel(pCmProgram, _NAME(FlipX_DualSurface), pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Each thread process 8 OWORD (128 bytes)
    int ThreadCount = (int) ceil((float) (WidthInByte * FrameHeight) /128 );

	int threadswidth = (ThreadCount < 511) ? ThreadCount : 511;
    int threadsheight = (ThreadCount - 1) / 511 + 1;
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInIndex);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pOutIndex);

    pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &WidthInByte);
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &ThreadCount);

    pCmKernel->SetKernelArg(ParaIdx++, ReverseWedge.get_size_data(), ReverseWedge.get_addr_data());

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}

#ifdef NOT_USED
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::Enqueue_FlipX(SurfaceIndex * pInIndex, int FrameWidth, int FrameHeight)
{
    int result;
    int BlockWidth = 8;    // 8x8 pixels, RGBA 4 bytes per pixel
    int BlockHeight = 8;
 
    // Create kernel from Cm Program
    pCmKernel = NULL;
    result = pCmDev->CreateKernel(pCmProgram, _NAME(FlipX_SingleSurface), pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Each thread process 8 OWORD (128 bytes)
    int ThreadCount = (int) ceil((float) (FrameWidth*FrameHeight)/128);

	int threadswidth = (ThreadCount < 511) ? ThreadCount : 511;
    int threadsheight = (ThreadCount - 1) / 511 + 1;
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInIndex);	// Set input/output surface index
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(int), &FrameWidth);

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MDF_Base::Enqueue_RGBA2BGRA(SurfaceIndex * pInIndex, int FrameWidth, int FrameHeight)
{
    int result;
    int BlockWidth = 8;    // 8x8 pixels, RGBA 4 bytes per pixel
    int BlockHeight = 8;
 
    // Create kernel from Cm Program
    pCmKernel = NULL;
    result = pCmDev->CreateKernel(pCmProgram, _NAME(RGBA2BGRA), pCmKernel);
	if (result != CM_SUCCESS ) {
        perror("CM CreateKernel error");
        return -1;
    }

    // Find # of threads based on output frame size in pixel 
    int threadswidth  = (int) ceil((float) FrameWidth / BlockWidth); 
    int threadsheight = (int) ceil((float) FrameHeight / BlockHeight); 
    pCmKernel->SetThreadCount( threadswidth * threadsheight );

	// Set curbe data
	int ParaIdx = 0;
    pCmKernel->SetKernelArg(ParaIdx++, sizeof(SurfaceIndex), pInIndex);	// Set input surface index

	pThrdSpace = NULL;
	pCmDev->CreateThreadSpace(threadswidth, threadsheight, pThrdSpace);

	// Create a task (container) to be put in the task queue
    pKernelArray = NULL;
	result = pCmDev->CreateTask(pKernelArray);
	if (result != CM_SUCCESS ) {
		perror("CmDevice CreateTask error");
		return -1;
	}

	// Add a kernel to kernel array
	result = pKernelArray->AddKernel(pCmKernel);
	if (result != CM_SUCCESS ) {
		perror("CmDevice AddKernel error");
		return -1;
	}

	// Put kernel array into task queue to be executed on GPU
	result = pCmQueue->Enqueue(pKernelArray, pEvent, pThrdSpace);
	if (result != CM_SUCCESS ) {
		perror("CmDevice enqueue error");
		return -1;
	}

	// Destroy a task (container) after using it
	pCmDev->DestroyTask(pKernelArray);

    return 0;
}
