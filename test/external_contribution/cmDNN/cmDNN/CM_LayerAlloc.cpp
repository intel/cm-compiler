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

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#ifdef WIN32
#include <io.h>
#else
#define _aligned_malloc(x,y) malloc(x)
#endif
//#include "half.h"
#include <cm_rt.h>

//using namespace half_float;

#include "cmDNN.h"

/////////////////////////////////////////////////////////////////////////////////////////////
int CM_LAYER::AllocGPUSurface(int Width, int Height, CM_SURFACE_FORMAT format, CmSurface2D ** pSurf, SurfaceIndex ** pIndex)
{
	int result;
    CmSurface2D * pSurf2 = NULL;
    SurfaceIndex * pIndex2 = NULL;

	// Create a 2D surface
    result = pCm->pCmDev->CreateSurface2D( Width, Height, format, pSurf2 );
    if (result != CM_SUCCESS ) {
        printf("CM CreateSurface2D error: %d\n", result);
        return -1;
    }
	pSurf2->GetIndex(pIndex2);

    *pSurf = pSurf2;
    *pIndex = pIndex2;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
int CM_LAYER::AllocGPUSurfaceUP(int iFrameWidth, int iFrameHeight, CM_SURFACE_FORMAT format, unsigned char * pBuffer, CmSurface2DUP ** pSurf, SurfaceIndex ** pIndex)
{
	int result;
    CmSurface2DUP * pSurf2 = NULL;
    SurfaceIndex * pIndex2 = NULL;

	// Create a 2D surface
    result = pCm->pCmDev->CreateSurface2DUP( iFrameWidth, iFrameHeight, format, pBuffer, pSurf2 );

    if (result != CM_SUCCESS ) {
        printf("CM CreateSurface2DUP error: %d\n", result);
        return -1;
    }
	pSurf2->GetIndex(pIndex2);

    *pSurf = pSurf2;
    *pIndex = pIndex2;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
int CM_LAYER::AllocGPUBuffer(unsigned int size, CmBuffer ** pBuffer, SurfaceIndex ** pIndex)
{
	int result;
    CmBuffer * pBuffer2 = NULL;
    SurfaceIndex * pIndex2 = NULL;

	// Create a 1D buffer
    result = pCm->pCmDev->CreateBuffer( size, pBuffer2 );
    if (result != CM_SUCCESS ) {
        printf("CM CreateBuffer error: %d\n", result);
        return -1;
    }
	pBuffer2->GetIndex(pIndex2);

    *pBuffer = pBuffer2;
    *pIndex = pIndex2;
    return 0;
}

////////////////////////////////////////////////////////////////////////
int CM_LAYER::AllocGPUBufferUP(unsigned int size, unsigned char * pSysMem, CmBufferUP ** pBuffer, SurfaceIndex ** pIndex)
{
	int result;
    CmBufferUP * pBuffer2 = NULL;
    SurfaceIndex * pIndex2 = NULL;

	// Create 1D UP buffer
    result = pCm->pCmDev->CreateBufferUP( size, pSysMem, pBuffer2 );

    if (result != CM_SUCCESS ) {
        printf("CM CreateBufferUP error: %d\n", result);
        return -1;
    }
	pBuffer2->GetIndex(pIndex2);

    *pBuffer = pBuffer2;
    *pIndex = pIndex2;
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
void * CM_LAYER::AlignedMalloc(unsigned int size, int alignment)
{
    void * pBuffer;

#ifdef WIN32
    pBuffer = (void *) _aligned_malloc(size, alignment);
    if (pBuffer == NULL) {
        printf("Failed _aligned_malloc.\n");
        pBuffer = 0;
    }
#elif ANDROID
    pBuffer = (void *) memalign(size, alignment);
    if (pBuffer == NULL) {
        printf("ANDROID memalign error.\n");
        pBuffer = 0;
    }
#else
	printf("ANDROID memalign error.\n");
    pBuffer = 0;
#endif

    return pBuffer;
}

////////////////////////////////////////////////////////////
void CM_LAYER::AlignedFree(void * pBuffer)
{
#ifdef WIN32
    _aligned_free(pBuffer);
#elif ANDROID
    free(pBuffer);
#endif
}

#ifdef WIN32

#ifdef CM_DX9
/////////////////////////////////////////////////////////////////////////////////////////////
int CM_LAYER::ImportDX9Surface(IDirect3DSurface9 * pD3DSurf, CmSurface2D ** pSurf, SurfaceIndex ** pIndex)
{
	int result;
    CmSurface2D * pSurf2 = NULL;
    SurfaceIndex * pIndex2 = NULL;

	// Bind to a D3D surface
    result = pCm->pCmDev->CreateSurface2D(pD3DSurf, pSurf2);

    if (result != CM_SUCCESS ) {
        printf("CM CreateSurface2D error: %d.\n", result);
        return -1;
    }
	pSurf2->GetIndex(pIndex2);

    *pSurf = pSurf2;
    *pIndex = pIndex2;
    return 0;
}
#endif

#ifdef CM_DX11
////////////////////////////////////////////////////////////////////////
int CM_LAYER::ImportDX11Surface(ID3D11Texture2D * pD3DSurf, CmSurface2D ** pSurf, SurfaceIndex ** pIndex)
{
	int result;
    CmSurface2D * pSurf2 = NULL;
    SurfaceIndex * pIndex2 = NULL;

	// Bind to a D3D surface
    result = pCm->pCmDev->CreateSurface2D (pD3DSurf, pSurf2);

    if (result != CM_SUCCESS ) {
        printf("CM CreateSurface2D error: %d.\n", result);
        return -1;
    }
	pSurf2->GetIndex(pIndex2);

    *pSurf = pSurf2;
    *pIndex = pIndex2;
    return 0;
}
#endif

#endif

//////////////////////////////////////////////////////////////////////////////////////////
int CM_LAYER::AllocatGPUSampler(CmSurface2D * pInputSurf, SurfaceIndex ** pSurfIndex, SamplerIndex ** pSamplerIndex )
{
    int result;
    SurfaceIndex * pSurfIndex2 = NULL;
    SamplerIndex * pSamplerIndex2 = NULL;

    // Define sampler state
	CM_SAMPLER_STATE SamplerState;
	SamplerState.minFilterType = SamplerState.magFilterType = CM_TEXTURE_FILTER_TYPE_POINT; // CM_TEXTURE_FILTER_TYPE_LINEAR;
	SamplerState.addressU = SamplerState.addressV = SamplerState.addressW = CM_TEXTURE_ADDRESS_CLAMP;

    // Create sampler state  
	CmSampler * pSampler = NULL;
	result = pCm->pCmDev->CreateSampler(SamplerState, pSampler);
    if (result != CM_SUCCESS ) {
        printf("CM CreateSampler error.");
        return -1;
    }

    // Bind the actual 2D surface with a virtual sampler surface index
	// Get surface index for input surface used by sampler
	pCm->pCmDev->CreateSamplerSurface2D(pInputSurf, pSurfIndex2);	// Since CM2.4

	// Get sampler index for input surface
	pSampler->GetIndex( pSamplerIndex2 );

    *pSurfIndex = pSurfIndex2;
    *pSamplerIndex = pSamplerIndex2;
    return 0;
}


