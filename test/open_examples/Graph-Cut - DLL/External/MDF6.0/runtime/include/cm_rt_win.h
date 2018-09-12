/**             
***
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
*** ----------------------------------------------------------------------------
**/  
#pragma once

#include "windows.h"
#include "winbase.h"
#include "stdio.h"

#include "typeinfo"
#define CM_ATTRIBUTE(attribute) __declspec(attribute)
#define CM_TYPE_NAME(type)  typeid(type).name()

inline void * CM_ALIGNED_MALLOC(size_t size, size_t alignment) 
{
    return _aligned_malloc(size, alignment);
}

inline void CM_ALIGNED_FREE(void * memory) 
{
    _aligned_free(memory);
}

#if !CM_METRO
//multi-thread API: 
#define THREAD_HANDLE HANDLE
inline void CM_THREAD_CREATE(THREAD_HANDLE *handle, void * start_routine, void * arg) 
{
    handle[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)start_routine, (LPVOID )arg, 0, NULL);
}
inline void CM_THREAD_EXIT(void * retval) 
{
    ExitThread(0);
}

inline int CM_THREAD_JOIN(THREAD_HANDLE *handle_array, int thread_cnt) 
{
    DWORD ret = WaitForMultipleObjects( thread_cnt, handle_array, true, INFINITE );
    return ret;
}
#endif

#ifdef CM_DX9
    #include <d3d9.h>
    #include <dxva2api.h>

    #define CM_SURFACE_FORMAT                       D3DFORMAT

    #define CM_SURFACE_FORMAT_UNKNOWN               D3DFMT_UNKNOWN
    #define CM_SURFACE_FORMAT_A8R8G8B8              D3DFMT_A8R8G8B8
    #define CM_SURFACE_FORMAT_X8R8G8B8              D3DFMT_X8R8G8B8
    #define CM_SURFACE_FORMAT_A8B8G8R8              D3DFMT_A8B8G8R8 
    #define CM_SURFACE_FORMAT_A8                    D3DFMT_A8
    #define CM_SURFACE_FORMAT_P8                    D3DFMT_P8
    #define CM_SURFACE_FORMAT_R32F                  D3DFMT_R32F
    #define CM_SURFACE_FORMAT_NV12                  (D3DFORMAT)MAKEFOURCC('N','V','1','2')
    #define CM_SURFACE_FORMAT_P016                  (D3DFORMAT)MAKEFOURCC('P','0','1','6')
    #define CM_SURFACE_FORMAT_P010                  (D3DFORMAT)MAKEFOURCC('P','0','1','0')
    #define CM_SURFACE_FORMAT_UYVY                  D3DFMT_UYVY
    #define CM_SURFACE_FORMAT_YUY2                  D3DFMT_YUY2
    #define CM_SURFACE_FORMAT_V8U8                  D3DFMT_V8U8
    #define CM_SURFACE_FORMAT_A8L8                  D3DFMT_A8L8
    #define CM_SURFACE_FORMAT_D16                   D3DFMT_D16
    #define CM_SURFACE_FORMAT_L16                   D3DFMT_L16
    #define CM_SURFACE_FORMAT_A16B16G16R16          D3DFMT_A16B16G16R16
    #define CM_SURFACE_FORMAT_A16B16G16R16F         D3DFMT_A16B16G16R16F
    #define CM_SURFACE_FORMAT_R10G10B10A2           D3DFMT_A2B10G10R10
    #define CM_SURFACE_FORMAT_IRW0                  (D3DFORMAT)MAKEFOURCC('I','R','W','0')
    #define CM_SURFACE_FORMAT_IRW1                  (D3DFORMAT)MAKEFOURCC('I','R','W','1')
    #define CM_SURFACE_FORMAT_IRW2                  (D3DFORMAT)MAKEFOURCC('I','R','W','2')
    #define CM_SURFACE_FORMAT_IRW3                  (D3DFORMAT)MAKEFOURCC('I','R','W','3')
    #define CM_SURFACE_FORMAT_R16_FLOAT             D3DFMT_R16F  //beryl
    #define CM_SURFACE_FORMAT_A8P8                  D3DFMT_A8P8 
    #define CM_SURFACE_FORMAT_I420                  (D3DFORMAT)MAKEFOURCC('I','4','2','0')  
    #define CM_SURFACE_FORMAT_IMC3                  (D3DFORMAT)MAKEFOURCC('I','M','C','3')  
    #define CM_SURFACE_FORMAT_IA44                  (D3DFORMAT)MAKEFOURCC('I','A','4','4') 
    #define CM_SURFACE_FORMAT_AI44                  (D3DFORMAT)MAKEFOURCC('A','I','4','4') 
    #define CM_SURFACE_FORMAT_P216                  (D3DFORMAT)MAKEFOURCC('P','2','1','6') 
    
    #define CM_TEXTURE_ADDRESS_TYPE                 D3DTEXTUREADDRESS

    #define CM_TEXTURE_ADDRESS_WRAP                 D3DTADDRESS_WRAP
    #define CM_TEXTURE_ADDRESS_MIRROR               D3DTADDRESS_MIRROR
    #define CM_TEXTURE_ADDRESS_CLAMP                D3DTADDRESS_CLAMP
    #define CM_TEXTURE_ADDRESS_BORDER               D3DTADDRESS_BORDER
    #define CM_TEXTURE_ADDRESS_MIRRORONCE           D3DTADDRESS_MIRRORONCE

    #define CM_TEXTURE_FILTER_TYPE                  D3DTEXTUREFILTERTYPE

    #define CM_TEXTURE_FILTER_TYPE_NONE             D3DTEXF_NONE
    #define CM_TEXTURE_FILTER_TYPE_POINT            D3DTEXF_POINT
    #define CM_TEXTURE_FILTER_TYPE_LINEAR           D3DTEXF_LINEAR
    #define CM_TEXTURE_FILTER_TYPE_ANISOTROPIC      D3DTEXF_ANISOTROPIC
    #define CM_TEXTURE_FILTER_TYPE_FLATCUBIC        D3DTEXF_FLATCUBIC
    #define CM_TEXTURE_FILTER_TYPE_GAUSSIANCUBIC    D3DTEXF_GAUSSIANCUBIC
    #define CM_TEXTURE_FILTER_TYPE_PYRAMIDALQUAD    D3DTEXF_PYRAMIDALQUAD
    #define CM_TEXTURE_FILTER_TYPE_GAUSSIANQUAD     D3DTEXF_GAUSSIANQUAD
    #define CM_TEXTURE_FILTER_TYPE_CONVOLUTIONMONO  D3DTEXF_CONVOLUTIONMONO
#elif defined(CM_DX11)
    #include <d3d11.h>

    #define CM_SURFACE_FORMAT                       DXGI_FORMAT

    #define CM_SURFACE_FORMAT_UNKNOWN               DXGI_FORMAT_UNKNOWN
    #define CM_SURFACE_FORMAT_A8R8G8B8              DXGI_FORMAT_B8G8R8A8_UNORM
    #define CM_SURFACE_FORMAT_X8R8G8B8              DXGI_FORMAT_B8G8R8X8_UNORM
    #define CM_SURFACE_FORMAT_A8B8G8R8              DXGI_FORMAT_R8G8B8A8_UNORM  
    #define CM_SURFACE_FORMAT_A8                    DXGI_FORMAT_A8_UNORM
    #define CM_SURFACE_FORMAT_P8                    DXGI_FORMAT_P8
    #define CM_SURFACE_FORMAT_R32F                  DXGI_FORMAT_R32_FLOAT
    #define CM_SURFACE_FORMAT_NV12                  DXGI_FORMAT_NV12
    #define CM_SURFACE_FORMAT_P016                  DXGI_FORMAT_P016   
    #define CM_SURFACE_FORMAT_P010                  DXGI_FORMAT_P010  
    #define CM_SURFACE_FORMAT_UYVY                  DXGI_FORMAT_R8G8_B8G8_UNORM
    #define CM_SURFACE_FORMAT_YUY2                  DXGI_FORMAT_YUY2
    #define CM_SURFACE_FORMAT_V8U8                  DXGI_FORMAT_R8G8_SNORM
    #define CM_SURFACE_FORMAT_R8_UINT               DXGI_FORMAT_R8_UINT
    #define CM_SURFACE_FORMAT_R16_SINT              DXGI_FORMAT_R16_SINT
    #define CM_SURFACE_FORMAT_R16_UNORM             DXGI_FORMAT_R16_UNORM
    #define CM_SURFACE_FORMAT_R32_UINT              DXGI_FORMAT_R32_UINT
    #define CM_SURFACE_FORMAT_R32_SINT              DXGI_FORMAT_R32_SINT
    #define CM_SURFACE_FORMAT_R8G8_UNORM            DXGI_FORMAT_R8G8_UNORM
    #define CM_SURFACE_FORMAT_R16_UINT              DXGI_FORMAT_R16_UINT
    #define CM_SURFACE_FORMAT_A16B16G16R16          DXGI_FORMAT_R16G16B16A16_UNORM
    #define CM_SURFACE_FORMAT_A16B16G16R16F         DXGI_FORMAT_R16G16B16A16_FLOAT
    #define CM_SURFACE_FORMAT_R10G10B10A2           DXGI_FORMAT_R10G10B10A2_UNORM 
    #define CM_SURFACE_FORMAT_R16_TYPELESS          DXGI_FORMAT_R16_TYPELESS
    #define CM_SURFACE_FORMAT_AYUV                  DXGI_FORMAT_AYUV
    #define CM_SURFACE_FORMAT_R16G16_UNORM          DXGI_FORMAT_R16G16_UNORM
    #define CM_SURFACE_FORMAT_R16_FLOAT             DXGI_FORMAT_R16_FLOAT   //beryl
    #define CM_SURFACE_FORMAT_A8P8                  DXGI_FORMAT_A8P8 
    #define CM_SURFACE_FORMAT_IA44                  DXGI_FORMAT_IA44
    #define CM_SURFACE_FORMAT_AI44                  DXGI_FORMAT_AI44

    #define CM_TEXTURE_ADDRESS_TYPE                 D3D11_TEXTURE_ADDRESS_MODE

    #define CM_TEXTURE_ADDRESS_WRAP                 D3D11_TEXTURE_ADDRESS_WRAP
    #define CM_TEXTURE_ADDRESS_MIRROR               D3D11_TEXTURE_ADDRESS_MIRROR
    #define CM_TEXTURE_ADDRESS_CLAMP                D3D11_TEXTURE_ADDRESS_CLAMP
    #define CM_TEXTURE_ADDRESS_BORDER               D3D11_TEXTURE_ADDRESS_BORDER
    #define CM_TEXTURE_ADDRESS_MIRRORONCE           D3D11_TEXTURE_ADDRESS_MIRROR_ONCE

    #define CM_TEXTURE_FILTER_TYPE                  D3D11_FILTER_TYPE

    #define CM_TEXTURE_FILTER_TYPE_POINT            D3D11_FILTER_TYPE_POINT
    #define CM_TEXTURE_FILTER_TYPE_LINEAR           D3D11_FILTER_TYPE_LINEAR
#endif

#ifdef CMRT_EMU
    // FIXME CM_KERNEL_FUNCTION macro provides wrong kernel name w/ unexplicit template like CM_KERNEL_FUNCTION(kernel<T>)
    #define CM_KERNEL_FUNCTION2(...) #__VA_ARGS__, (void *)(void (__cdecl *) (void))__VA_ARGS__
#else
    #define CM_KERNEL_FUNCTION2(...) #__VA_ARGS__
#endif

#ifdef CMRT_EMU
    #define _NAME(...) #__VA_ARGS__, (void (__cdecl *)(void))__VA_ARGS__
#else
    #define _NAME(...) #__VA_ARGS__
#endif

#define OS_WIN_RS 10014310

/******************************************************************************\
Function:    GetWinVer
Description: Determines the version of windows os
Returns:     UINSIGNED INT 99 9 99999 ( MajorVersion x 1000 000 MinorVersion x 100 000 OSBuildNumber)
\******************************************************************************/
inline unsigned int GetWinVer( void )
{
    unsigned int Result = 0;
    LONG( WINAPI *pfnRtlGetVersion )( RTL_OSVERSIONINFOW* );
    ( FARPROC& )pfnRtlGetVersion = GetProcAddress( GetModuleHandleW( ( L"ntdll.dll" ) ), "RtlGetVersion" );
    if ( pfnRtlGetVersion )
    {
        RTL_OSVERSIONINFOW RTLOSVersionInfo = { 0 };
        RTLOSVersionInfo.dwOSVersionInfoSize = sizeof( RTLOSVersionInfo );
        // Get the OS version
        if ( pfnRtlGetVersion( &RTLOSVersionInfo ) == 0 )
        {
            Result = RTLOSVersionInfo.dwBuildNumber;
            Result += ( RTLOSVersionInfo.dwMinorVersion * 100000 );
            Result += ( RTLOSVersionInfo.dwMajorVersion * 1000000 );
        }
    }
    return Result;
}

inline bool LoadCMRTDLL()
{
    if ( GetWinVer() < OS_WIN_RS )  // only proceed for OS version RS1+
    {
        return false;
    }

#if defined( CMRT_EMU ) || defined( CMRT_SIM )
    return false;
#endif

    HKEY hKey;
    WCHAR pBuffer[ MAX_PATH ];
    DWORD dwBufferSize = sizeof( pBuffer );
    ULONG nError;
    REGSAM  samDesired = KEY_READ;

    if ( GetSystemWow64Directory( ( LPTSTR )pBuffer, MAX_PATH ) > 0 )  // 64-bit OS
    {
        samDesired |= KEY_WOW64_64KEY;
    }

    LONG lRes = RegOpenKeyExW( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Intel\\MDF", 0, samDesired, &hKey );
    if ( lRes != ERROR_SUCCESS )
    {
        return false;
    }

    nError = RegQueryValueExW( hKey, L"DriverStorePath", 0, NULL, ( LPBYTE )pBuffer, &dwBufferSize );
    if ( ERROR_SUCCESS == nError )
    {
#ifdef CM_DX11
        wcscat_s( pBuffer, MAX_PATH, L"\\igfx11cmrt" );
#else if CM_DX9
        wcscat_s( pBuffer, MAX_PATH, L"\\igfxcmrt" );
#endif

        if ( sizeof( void * ) == 4 )
        {
            wcscat_s( pBuffer, MAX_PATH, L"32.dll" );
        }
        else
        {
            wcscat_s( pBuffer, MAX_PATH, L"64.dll" );
        }

        HMODULE handle = LoadLibraryExW( pBuffer, NULL, 0 );
        if ( handle == NULL )
        {
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

static bool g_MDF_LoadCMRTDLL_Result = LoadCMRTDLL();