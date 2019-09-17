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

#ifdef WIN32
#include "stdafx.h"
#endif

#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>

#ifdef WIN32 
#include <io.h>
#include <direct.h>
#include <dxgi.h>
#endif

#include "cmDNN.h"

#define INTEL_VENDOR_ID         0x8086

template <class T> void D3DSafeRelease(T **ppT)
{
    if (*ppT) {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
CM_Base::CM_Base()
{
	pCommonISACode = NULL;
    pCmProgram = NULL;
	pCmDev = NULL;
    pCmProgram = NULL;
    pCmQueue = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CM_Base::~CM_Base()
{
#ifdef _DEBUG
    pCmDev->FlushPrintBuffer();
#endif

	if (pCommonISACode ) {
		free(pCommonISACode);
		pCommonISACode = 0;
	}

    if (pCmProgram) {
        pCmDev->DestroyProgram(pCmProgram);
        pCmProgram = 0;
    }

	if (pCmDev)	{
		::DestroyCmDevice(pCmDev);
        pCmDev = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
int CM_Base::InitGPU(char * isa_file)
{
#ifdef NOT_USED
	if (DetectIntelGPU() == CM_INTEL_GFX_NOTFOUND) {
		printf("Intel graphics hardware is not found.  Enable it in system BIOS on dual graphics system.\n");
		return -1;
	}

	// Detect CM
	if (!DetectCM()) {
		printf("Intel graphics driver is not found.  Download and install Intel graphics driver.\n");
		return -1;
	}
#endif

	// Create a CM Device
//	unsigned int Option = 1;	// Disable scratch space, no need for that, no spill happens in kernels. 
	unsigned int Option = 0;
	int result;

    pCmProgram = 0;
    pCmQueue = 0;

#if 1//def CMRT_EMU
    result = ::CreateCmDevice( pCmDev, Version );
#else
	result = ::CreateCmDeviceEx( pCmDev, Version, NULL, Option );
#endif

    if (result != CM_SUCCESS ) {
        printf("CmDevice creation error, error code %d.\n", result);
        return -1;
    }

    size_t CapValueSize = 4;
    pCmDev->GetCaps(CAP_GPU_PLATFORM, CapValueSize, &GpuPlatform);
    pCmDev->GetCaps(CAP_GT_PLATFORM, CapValueSize, &GtPlatform);
	pCmDev->GetCaps(CAP_MIN_FREQUENCY, CapValueSize, &GpuMinFreq);
	pCmDev->GetCaps(CAP_MAX_FREQUENCY, CapValueSize, &GpuMaxFreq);
	pCmDev->GetCaps(CAP_HW_THREAD_COUNT, CapValueSize, &HWThreadCount);
	pCmDev->GetCaps(CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP, CapValueSize, &ThreadsPerGroup );

    printf("Platform: ");
    if (GpuPlatform == PLATFORM_INTEL_SNB)
        printf("SNB ");
    else if (GpuPlatform == PLATFORM_INTEL_IVB)
        printf("IVB ");
    else if (GpuPlatform == PLATFORM_INTEL_HSW)
        printf("HSW ");
    else if (GpuPlatform == PLATFORM_INTEL_BDW)
        printf("BDW ");
    else if (GpuPlatform == PLATFORM_INTEL_VLV)
        printf("VLV ");
    else if (GpuPlatform == PLATFORM_INTEL_CHV)
        printf("CHV ");
#ifdef _CM5
    else if (GpuPlatform == PLATFORM_INTEL_SKL)
        printf("SKL ");
    else if (GpuPlatform == PLATFORM_INTEL_BXT)
        printf("BXT ");
#endif
#ifdef _CM6
    else if (GpuPlatform == PLATFORM_INTEL_SKL)
        printf("SKL ");
    else if (GpuPlatform == PLATFORM_INTEL_BXT)
        printf("BXT ");
    else if (GpuPlatform == PLATFORM_INTEL_KBL)
        printf("KBL ");
	else if (GpuPlatform == PLATFORM_INTEL_ICL)
		printf("ICL ");
	else if (GpuPlatform == PLATFORM_INTEL_ICLLP)
		printf("ICLLP ");
	else if (GpuPlatform == PLATFORM_INTEL_TGL)
		printf("TGL ");
	else if (GpuPlatform == PLATFORM_INTEL_TGLLP)
		printf("TGLLP ");
#endif

    if (GtPlatform == PLATFORM_INTEL_GT1)
        printf("GT1\n");
    else if (GtPlatform == PLATFORM_INTEL_GT2)
        printf("GT2\n");
    else if (GtPlatform == PLATFORM_INTEL_GT3)
        printf("GT3\n");
    else if (GtPlatform == PLATFORM_INTEL_GT4)
        printf("GT4\n");

    printf("HW thread count: %d\n", HWThreadCount); 
	printf("GPU min frequency: %d MHz\n", GpuMinFreq);
    printf("GPU max frequency: %d MHz\n", GpuMaxFreq);
    printf("Threads per group: %d\n\n", ThreadsPerGroup); 

#ifdef _DEBUG
    pCmDev->InitPrintBuffer();
#endif

//	int result;

    // Open isa file
    pISA = fopen(isa_file, "rb");
	if (pISA == NULL) {
        printf("Error openning isa file: %s\n", isa_file);
        return -1;
    }
	
	// Find the kernel code size
	fseek (pISA, 0, SEEK_END);
    int codeSize = ftell (pISA);
	rewind(pISA);

	if(codeSize == 0) {
        printf("Kernel code size is 0.\n");
		return -1;
	}

	// Allocate memory to hold isa kernel from kernel file
    pCommonISACode = (BYTE*) malloc(codeSize);
	if( !pCommonISACode ) {
        printf("Failed allocate memeory for kernel.\n");
		return -1;
	}

	// Read kernel file
	if (fread(pCommonISACode, 1, codeSize, pISA) != codeSize) {
        printf("Error reading isa file");
        return -1;
    }
	fclose(pISA);

    // Load kernel into CmProgram obj
    pCmProgram = NULL;
    result = pCmDev->LoadProgram(pCommonISACode, codeSize, pCmProgram); 
    if (result != CM_SUCCESS ) {
        printf("CM LoadProgram error: %d\n", result);
        return -1;
    }

    // Create a task queue
    pCmQueue = NULL;
    result = pCmDev->CreateQueue( pCmQueue );
    if (result != CM_SUCCESS ) {
        printf("CM CreateQueue error: %d\n", result);
        return -1;
    }

    return 0;
}

#ifdef NOT_USED
////////////////////////////////////////////////////////////////
#ifdef WIN32
double mygetTickFrequency( void )
{
    LARGE_INTEGER freq;
    if( QueryPerformanceFrequency( &freq ) )
        return (double)(freq.QuadPart / 1000.);
    return -1.;
}
#endif

/////////////////////////////////////////////////////////////////
// Return in millisecond
double CM_Base::GetTimeMS()
{

#ifdef WIN32
    __int64 time_a = __rdtsc();
    return (double) time_a/(mygetTickFrequency()*1000.0);
#elif ANDROID
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double) now.tv_sec*1000.0 + now.tv_nsec/1000000.0;
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double) now.tv_sec*1000.0 + now.tv_nsec/1000000.0;
#endif

}

/////////////////////////////////////////////////////////////////////////////////
int CM_Base::DetectIntelGPU()
{
	// Get dxgi factory1 for enumarating graphics adapters
	IDXGIFactory1 * pFactory1 = NULL;
	int hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory1));
	if (FAILED(hr))
	    return CM_FAILURE;
	
	// Enumarate all graphics adapters 
	UINT i = 0;
	IDXGIAdapter1* pAdapter1 = NULL;
	while (hr = pFactory1->EnumAdapters1(i, &pAdapter1) != DXGI_ERROR_NOT_FOUND)
	{
	    DXGI_ADAPTER_DESC1 AdapterDesc1;
	    hr = pAdapter1->GetDesc1(&AdapterDesc1);

	    if (AdapterDesc1.VendorId == INTEL_VENDOR_ID) {
			wprintf(L"Detected %s\n", AdapterDesc1.Description);
			break;
		} else {
			D3DSafeRelease(&pAdapter1);
			pAdapter1 = NULL;
	    }
	    i++;
	}
	
	D3DSafeRelease(&pFactory1);
	if (pAdapter1 == NULL)
	    return CM_INTEL_GFX_NOTFOUND;
	else 
		return CM_SUCCESS;
}

////////////////////////////////////////////////////////////////
int CM_Base::DetectCM()
{
	char * buffer;
	DWORD length;
    HKEY hKey;

	// Find new MDF driver in registry key
    if (RegOpenKeyExA(HKEY_CLASSES_ROOT, "CLSID\\{EB3C4B33-C93C-4A5C-91DE-43FDB945FF80}\\MDF", 0, KEY_READ, &hKey) == ERROR_SUCCESS ||
		RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Intel\\MDF", 0, KEY_READ, &hKey) == ERROR_SUCCESS ) {

		if(RegQueryValueExA(hKey, "DriverStorePath", NULL, NULL, NULL, &length) == ERROR_SUCCESS) {
		    buffer = (char *)LocalAlloc(LPTR, length);
		    // query
			if(RegQueryValueExA(hKey, "DriverStorePath", NULL, NULL, (BYTE *) buffer, &length) == ERROR_SUCCESS) {
			    RegCloseKey(hKey);
				if (strstr(buffer, "\\System32\\DriverStore\\FileRepository\\")) {
					printf("Detected MDF driver\n");
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
	sprintf(FullPath, "%s\\System32\\igfx11cmrt64.dll", buffer); 
	int status = _access(FullPath, 0);
	if (status == 0) {
		printf("Detected MDF driver on this system.\n");
		return true;
	}
	LocalDiscard(buffer);
	printf("Could not detect MDF driver on this system.\n");
	return false;
}
#endif
