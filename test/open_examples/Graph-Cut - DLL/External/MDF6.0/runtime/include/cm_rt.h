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

//Using CM_DX9 by default
#if (!defined(__GNUC__)) && (!defined(CM_DX11))
#ifndef CM_DX9
#define CM_DX9
#endif
#endif

#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif

#include "../../compiler/include/cm/cm_common.h"

#ifdef __GNUC__
#include "cm_rt_linux.h"
#else
#include "cm_rt_win.h"  
#endif


#define CM_KERNEL_FUNCTION(...) CM_KERNEL_FUNCTION2(__VA_ARGS__)

#define CM_RT_API 

#ifndef CM_1_0
#define CM_1_0          100
#endif 
#ifndef CM_2_0
#define CM_2_0          200
#endif
#ifndef CM_2_1
#define CM_2_1          201
#endif
#ifndef CM_2_2
#define CM_2_2          202
#endif
#ifndef CM_2_3
#define CM_2_3          203
#endif
#ifndef CM_2_4
#define CM_2_4          204
#endif
#ifndef CM_3_0
#define CM_3_0          300
#endif
#ifndef CM_4_0
#define CM_4_0          400
#endif
#ifndef CM_5_0
#define CM_5_0          500
#endif
#ifndef CM_6_0
#define CM_6_0          600
#endif

//Define MDF version for new feature
#ifndef __INTEL_MDF 
#define __INTEL_MDF     (CM_6_0 + 1)
#endif

typedef enum _CM_RETURN_CODE
{
    CM_SUCCESS                                  = 0,
    /*
     * RANGE -1 ~ -9999 FOR EXTERNAL ERROR CODE
     */
    CM_FAILURE                                  = -1,
    CM_NOT_IMPLEMENTED                          = -2,
    CM_SURFACE_ALLOCATION_FAILURE               = -3,
    CM_OUT_OF_HOST_MEMORY                       = -4,
    CM_SURFACE_FORMAT_NOT_SUPPORTED             = -5,
    CM_EXCEED_SURFACE_AMOUNT                    = -6,
    CM_EXCEED_KERNEL_ARG_AMOUNT                 = -7,
    CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE           = -8,
    CM_INVALID_ARG_INDEX                        = -9,
    CM_INVALID_ARG_VALUE                        = -10,
    CM_INVALID_ARG_SIZE                         = -11,
    CM_INVALID_THREAD_INDEX                     = -12,
    CM_INVALID_WIDTH                            = -13,
    CM_INVALID_HEIGHT                           = -14,
    CM_INVALID_DEPTH                            = -15,
    CM_INVALID_COMMON_ISA                       = -16,
    CM_D3DDEVICEMGR_OPEN_HANDLE_FAIL            = -17, // IDirect3DDeviceManager9::OpenDeviceHandle fail
    CM_D3DDEVICEMGR_DXVA2_E_NEW_VIDEO_DEVICE    = -18, // IDirect3DDeviceManager9::LockDevice return DXVA2_E_VIDEO_DEVICE_LOCKED
    CM_D3DDEVICEMGR_LOCK_DEVICE_FAIL            = -19, // IDirect3DDeviceManager9::LockDevice return other fail than DXVA2_E_VIDEO_DEVICE_LOCKED
    CM_EXCEED_SAMPLER_AMOUNT                    = -20,
    CM_EXCEED_MAX_KERNEL_PER_ENQUEUE            = -21,
    CM_EXCEED_MAX_KERNEL_SIZE_IN_BYTE           = -22,
    CM_EXCEED_MAX_THREAD_AMOUNT_PER_ENQUEUE     = -23,
    CM_EXCEED_VME_STATE_G6_AMOUNT               = -24,
    CM_INVALID_THREAD_SPACE                     = -25,
    CM_EXCEED_MAX_TIMEOUT                       = -26,
    CM_JITDLL_LOAD_FAILURE                      = -27,
    CM_JIT_COMPILE_FAILURE                      = -28,
    CM_JIT_COMPILESIM_FAILURE                   = -29,
    CM_INVALID_THREAD_GROUP_SPACE               = -30,
    CM_THREAD_ARG_NOT_ALLOWED                   = -31,
    CM_INVALID_GLOBAL_BUFFER_INDEX              = -32,
    CM_INVALID_BUFFER_HANDLER                   = -33,
    CM_EXCEED_MAX_SLM_SIZE                      = -34,
    CM_JITDLL_OLDER_THAN_ISA                    = -35,
    CM_INVALID_HARDWARE_THREAD_NUMBER           = -36,
    CM_GTPIN_INVOKE_FAILURE                     = -37,
    CM_INVALIDE_L3_CONFIGURATION                = -38,
    CM_INVALID_D3D11_TEXTURE2D_USAGE            = -39,
    CM_INTEL_GFX_NOTFOUND                       = -40,
    CM_GPUCOPY_INVALID_SYSMEM                   = -41,
    CM_GPUCOPY_INVALID_WIDTH                    = -42,
    CM_GPUCOPY_INVALID_STRIDE                   = -43,
    CM_EVENT_DRIVEN_FAILURE                     = -44,
    CM_LOCK_SURFACE_FAIL                        = -45, // Lock surface failed
    CM_INVALID_GENX_BINARY                      = -46,
    CM_FEATURE_NOT_SUPPORTED_IN_DRIVER          = -47, // driver out-of-sync
    CM_QUERY_DLL_VERSION_FAILURE                = -48, //Fail in getting DLL file version
    CM_KERNELPAYLOAD_PERTHREADARG_MUTEX_FAIL    = -49,
    CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL    = -50,
    CM_KERNELPAYLOAD_SETTING_FAILURE            = -51,
    CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX    = -52, 
    CM_NOT_SET_KERNEL_ARGUMENT                  = -53,
    CM_GPUCOPY_INVALID_SURFACES                 = -54,
    CM_GPUCOPY_INVALID_SIZE                     = -55,
    CM_GPUCOPY_OUT_OF_RESOURCE                  = -56,
    CM_DEVICE_INVALID_D3DDEVICE                 = -57,
    CM_SURFACE_DELAY_DESTROY                    = -58,
    CM_INVALID_VEBOX_STATE                      = -59,
    CM_INVALID_VEBOX_SURFACE                    = -60,
    CM_FEATURE_NOT_SUPPORTED_BY_HARDWARE        = -61,
    CM_RESOURCE_USAGE_NOT_SUPPORT_READWRITE     = -62,
    CM_MULTIPLE_MIPLEVELS_NOT_SUPPORTED         = -63,
    CM_INVALID_UMD_CONTEXT                      = -64,
    CM_INVALID_LIBVA_SURFACE                    = -65,
    CM_INVALID_LIBVA_INITIALIZE                 = -66,
    CM_KERNEL_THREADSPACE_NOT_SET               = -67,
    CM_INVALID_KERNEL_THREADSPACE               = -68,
    CM_KERNEL_THREADSPACE_THREADS_NOT_ASSOCIATED= -69,
    CM_KERNEL_THREADSPACE_INTEGRITY_FAILED      = -70,
    CM_INVALID_USERPROVIDED_GENBINARY           = -71,
    CM_INVALID_PRIVATE_DATA                     = -72,
    CM_INVALID_MOS_RESOURCE_HANDLE              = -73,
    CM_SURFACE_CACHED                           = -74,
    CM_SURFACE_IN_USE                           = -75,
    CM_INVALID_GPUCOPY_KERNEL                   = -76,
    CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN  = -77,
    CM_INVALID_MEDIA_WALKING_PATTERN            = -78,
    CM_FAILED_TO_ALLOCATE_SVM_BUFFER            = -79,
    CM_EXCEED_MAX_POWER_OPTION_FOR_PLATFORM     = -80,
    CM_INVALID_KERNEL_THREADGROUPSPACE          = -81,
    CM_INVALID_KERNEL_SPILL_CODE                = -82,
    CM_UMD_DRIVER_NOT_SUPPORTED                 = -83,
    CM_INVALID_GPU_FREQUENCY_VALUE              = -84,
    CM_SYSTEM_MEMORY_NOT_4KPAGE_ALIGNED         = -85,
    CM_KERNEL_ARG_SETTING_FAILED                = -86,
    CM_NO_AVAILABLE_SURFACE                     = -87,
    CM_VA_SURFACE_NOT_SUPPORTED                 = -88,
    CM_TOO_MUCH_THREADS                         = -89,
    CM_NULL_POINTER                             = -90,
    CM_EXCEED_MAX_NUM_2D_ALIASES                = -91,
    CM_INVALID_PARAM_SIZE                       = -92,
    CM_GT_UNSUPPORTED                           = -93,
    CM_GTPIN_FLAG_NO_LONGER_SUPPORTED           = -94,
    CM_PLATFORM_UNSUPPORTED_FOR_API             = -95,
    CM_TASK_MEDIA_RESET                         = -96,
    CM_KERNELPAYLOAD_SAMPLER_INVALID_BTINDEX    = -97,


    /*
     * RANGE <=-10000 FOR INTERNAL ERROR CODE
     */
    CM_INTERNAL_ERROR_CODE_OFFSET               = -10000,
} CM_RETURN_CODE;

#define _NAME_MERGE_(x, y)                      x ## y
#define _NAME_LABEL_(name, id)                  _NAME_MERGE_(name, id)
#define __CODEGEN_UNIQUE(name)                  _NAME_LABEL_(name, __LINE__)
#define BITFIELD_RANGE( startbit, endbit )     ((endbit)-(startbit)+1)
#define BITFIELD_BIT(bit)                        1

#define CM_MIN_SURF_WIDTH   1
#define CM_MIN_SURF_HEIGHT  1
#define CM_MIN_SURF_DEPTH   2

#define CM_MAX_1D_SURF_WIDTH  0X10000000 // 2^28

//IVB+
#define CM_MAX_2D_SURF_WIDTH_IVB_PLUS   16384
#define CM_MAX_2D_SURF_HEIGHT_IVB_PLUS  16384

#define CM_MAX_2D_SURF_WIDTH    CM_MAX_2D_SURF_WIDTH_IVB_PLUS
#define CM_MAX_2D_SURF_HEIGHT   CM_MAX_2D_SURF_HEIGHT_IVB_PLUS

#define CM_MAX_3D_SURF_WIDTH    2048
#define CM_MAX_3D_SURF_HEIGHT   2048
#define CM_MAX_3D_SURF_DEPTH    2048

#define CM_MAX_OPTION_SIZE_IN_BYTE          512
#define CM_MAX_KERNEL_NAME_SIZE_IN_BYTE     256
#define CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE   256

#define CM_MAX_THREADSPACE_WIDTH_FOR_MW        511
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MW       511
#define CM_MAX_THREADSPACE_WIDTH_FOR_MO        512
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MO       512
#define CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW  2047
#define CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW 2047

#define IVB_MAX_SLM_SIZE_PER_GROUP   16 // 64KB PER Group on Gen7

//Time in seconds before kernel should timeout
#define CM_MAX_TIMEOUT 2
//Time in milliseconds before kernel should timeout
#define CM_MAX_TIMEOUT_MS CM_MAX_TIMEOUT*1000

#define CM_NO_EVENT  ((CmEvent *)(-1))  //NO Event

typedef enum _CM_STATUS
{
    CM_STATUS_QUEUED         = 0,
    CM_STATUS_FLUSHED        = 1,
    CM_STATUS_FINISHED       = 2,
    CM_STATUS_STARTED        = 3
} CM_STATUS;

typedef struct _CM_SAMPLER_STATE
{
    CM_TEXTURE_FILTER_TYPE minFilterType;
    CM_TEXTURE_FILTER_TYPE magFilterType;   
    CM_TEXTURE_ADDRESS_TYPE addressU;   
    CM_TEXTURE_ADDRESS_TYPE addressV;   
    CM_TEXTURE_ADDRESS_TYPE addressW; 
} CM_SAMPLER_STATE;

typedef enum _CM_PIXEL_TYPE
{
    CM_PIXEL_UINT,
    CM_PIXEL_SINT,
    CM_PIXEL_OTHER
} CM_PIXEL_TYPE;

typedef struct _CM_SAMPLER_STATE_EX
{
    CM_TEXTURE_FILTER_TYPE minFilterType;
    CM_TEXTURE_FILTER_TYPE magFilterType;   
    CM_TEXTURE_ADDRESS_TYPE addressU;   
    CM_TEXTURE_ADDRESS_TYPE addressV;   
    CM_TEXTURE_ADDRESS_TYPE addressW;

    CM_PIXEL_TYPE SurfaceFormat;
    union {
        DWORD BorderColorRedU;
        INT BorderColorRedS;
        FLOAT BorderColorRedF;
    };

    union {
        DWORD BorderColorGreenU;
        INT BorderColorGreenS;
        FLOAT BorderColorGreenF;
    };

    union {
        DWORD BorderColorBlueU;
        INT BorderColorBlueS;
        FLOAT BorderColorBlueF;
    };
    
    union {
        DWORD BorderColorAlphaU;
        INT BorderColorAlphaS;
        FLOAT BorderColorAlphaF;
    };
} CM_SAMPLER_STATE_EX;

typedef enum _GPU_PLATFORM{
    PLATFORM_INTEL_UNKNOWN     = 0,
    PLATFORM_INTEL_SNB         = 1,   //Sandy Bridge
    PLATFORM_INTEL_IVB         = 2,   //Ivy Bridge
    PLATFORM_INTEL_HSW         = 3,   //Haswell
    PLATFORM_INTEL_BDW         = 4,   //Broadwell
    PLATFORM_INTEL_VLV         = 5,   //ValleyView
    PLATFORM_INTEL_CHV         = 6,   //CherryView
    PLATFORM_INTEL_SKL         = 7,   //SKL
    PLATFORM_INTEL_BXT         = 8,   //Broxton
    PLATFORM_INTEL_CNL         = 9,   //Cannonlake
    PLATFORM_INTEL_ICL         = 10,  //Icelake
    PLATFORM_INTEL_KBL         = 11,  //Kabylake
    PLATFORM_INTEL_GLV         = 12,  //Glenview
    PLATFORM_INTEL_ICLLP       = 13,  //IcelakeLP
    PLATFORM_INTEL_TGL         = 14,  //TigerLake
    PLATFORM_INTEL_TGLLP       = 15,  //TigerLakeLP
    PLATFORM_INTEL_GLK         = 16   //GeminiLake
} GPU_PLATFORM;

typedef enum _GPU_GT_PLATFORM{
    PLATFORM_INTEL_GT_UNKNOWN  = 0,
    PLATFORM_INTEL_GT1         = 1,
    PLATFORM_INTEL_GT2         = 2,
    PLATFORM_INTEL_GT3         = 3,
    PLATFORM_INTEL_GT4         = 4,
    PLATFORM_INTEL_GTVLV       = 5,
    PLATFORM_INTEL_GTVLVPLUS   = 6,
    PLATFORM_INTEL_GTCHV       = 7,
    PLATFORM_INTEL_GTA         = 8,
    PLATFORM_INTEL_GTC         = 9,
    PLATFORM_INTEL_GT1_5       = 10,
    PLATFORM_INTEL_GTX         = 11
} GPU_GT_PLATFORM;

typedef enum _CM_DEVICE_CAP_NAME
{
    CAP_KERNEL_COUNT_PER_TASK,
    CAP_KERNEL_BINARY_SIZE,
    CAP_SAMPLER_COUNT ,
    CAP_SAMPLER_COUNT_PER_KERNEL,
    CAP_BUFFER_COUNT ,
    CAP_SURFACE2D_COUNT,
    CAP_SURFACE3D_COUNT,
    CAP_SURFACE_COUNT_PER_KERNEL,
    CAP_ARG_COUNT_PER_KERNEL,
    CAP_ARG_SIZE_PER_KERNEL ,
    CAP_USER_DEFINED_THREAD_COUNT_PER_TASK,
    CAP_HW_THREAD_COUNT,
    CAP_SURFACE2D_FORMAT_COUNT,
    CAP_SURFACE2D_FORMATS,
    CAP_SURFACE3D_FORMAT_COUNT,
    CAP_SURFACE3D_FORMATS,
    CAP_VME_STATE_G6_COUNT,
    CAP_GPU_PLATFORM,
    CAP_GT_PLATFORM,
    CAP_MIN_FREQUENCY,
    CAP_MAX_FREQUENCY,
    CAP_L3_CONFIG,
    CAP_GPU_CURRENT_FREQUENCY,
    CAP_USER_DEFINED_THREAD_COUNT_PER_TASK_NO_THREAD_ARG,
    CAP_USER_DEFINED_THREAD_COUNT_PER_MEDIA_WALKER,
    CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP,
    CAP_SURFACE2DUP_COUNT,
    CAP_PLATFORM_INFO
} CM_DEVICE_CAP_NAME;

//------------------------------------------------------------------------------
//| CM HW platform info
//------------------------------------------------------------------------------
typedef struct _CM_PLATFORM_INFO
{
    UINT numSlices;
    UINT numSubSlices;
    UINT numEUsPerSubSlice;
    UINT numHWThreadsPerEU;
    UINT numMaxEUsPerPool;
}CM_PLATFORM_INFO, *PCM_PLATFORM_INFO;

typedef enum _CM_FASTCOPY_OPTION
{
    CM_FASTCOPY_OPTION_NONBLOCKING  = 0x00,
    CM_FASTCOPY_OPTION_BLOCKING     = 0x01
} CM_FASTCOPY_OPTION;

// CM RT DLL File Version
typedef struct _CM_DLL_FILE_VERSION
{
    WORD    wMANVERSION;
    WORD    wMANREVISION;
    WORD    wSUBREVISION;
    WORD    wBUILD_NUMBER; 
    //Version constructed as : "wMANVERSION.wMANREVISION.wSUBREVISION.wBUILD_NUMBER"
} CM_DLL_FILE_VERSION, *PCM_DLL_FILE_VERSION;

// Cm Device Create Option
#define CM_DEVICE_CREATE_OPTION_DEFAULT                     0
#define CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE       1
#define CM_DEVICE_CREATE_OPTION_TDR_DISABLE                 64  //Work only for DX11 so far

#define CM_DEVICE_CREATE_OPTION_SURFACE_REUSE_ENABLE        1024

#define CM_DEVICE_CONFIG_DISABLE_TASKFLUSHEDSEMAPHORE_OFFSET  6
#define CM_DEVICE_CONFIG_DISABLE_TASKFLUSHEDSEMAPHORE_MASK   (1<<CM_DEVICE_CONFIG_DISABLE_TASKFLUSHEDSEMAPHORE_OFFSET)
#define CM_DEVICE_CREATE_OPTION_TASKFLUSHEDSEMAPHORE_DISABLE  1   //to disable the semaphore for task flushed

#define NUM_SEARCH_PATH_STATES_G6       14
#define NUM_MBMODE_SETS_G6              4

typedef struct _VME_SEARCH_PATH_LUT_STATE_G6
{
    // DWORD 0
    union
    {
        struct
        {
            DWORD   SearchPathLocation_X_0  : 4;
            DWORD   SearchPathLocation_Y_0  : 4;
            DWORD   SearchPathLocation_X_1  : 4;
            DWORD   SearchPathLocation_Y_1  : 4;
            DWORD   SearchPathLocation_X_2  : 4;
            DWORD   SearchPathLocation_Y_2  : 4;
            DWORD   SearchPathLocation_X_3  : 4;
            DWORD   SearchPathLocation_Y_3  : 4;
        };
        struct
        {
            DWORD   Value;
        };
    };
} VME_SEARCH_PATH_LUT_STATE_G6, *PVME_SEARCH_PATH_LUT_STATE_G6;

typedef struct _VME_RD_LUT_SET_G6
{
    // DWORD 0
    union
    {
        struct
        {
            DWORD   LUT_MbMode_0    : 8;
            DWORD   LUT_MbMode_1    : 8;
            DWORD   LUT_MbMode_2    : 8;
            DWORD   LUT_MbMode_3    : 8;
        };
        struct
        {
            DWORD   Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD   LUT_MbMode_4    : 8;
            DWORD   LUT_MbMode_5    : 8;
            DWORD   LUT_MbMode_6    : 8;
            DWORD   LUT_MbMode_7    : 8;
        };
        struct
        {
            DWORD   Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct 
        {
            DWORD   LUT_MV_0        : 8;
            DWORD   LUT_MV_1        : 8;
            DWORD   LUT_MV_2        : 8;
            DWORD   LUT_MV_3        : 8;
        };
        struct
        {
            DWORD   Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            DWORD   LUT_MV_4        : 8;
            DWORD   LUT_MV_5        : 8;
            DWORD   LUT_MV_6        : 8;
            DWORD   LUT_MV_7        : 8;
        };
        struct
        {
            DWORD   Value;
        };
    } DW3;
} VME_RD_LUT_SET_G6, *PVME_RD_LUT_SET_G6;


typedef struct _VME_STATE_G6
{
    // DWORD 0 - DWORD 13
    VME_SEARCH_PATH_LUT_STATE_G6    SearchPath[NUM_SEARCH_PATH_STATES_G6];

    // DWORD 14
    union
    {
        struct
        {
            DWORD   LUT_MbMode_8_0  : 8;
            DWORD   LUT_MbMode_9_0  : 8;
            DWORD   LUT_MbMode_8_1  : 8;
            DWORD   LUT_MbMode_9_1  : 8;
        };
        struct
        {
            DWORD   Value;
        };
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            DWORD   LUT_MbMode_8_2  : 8;
            DWORD   LUT_MbMode_9_2  : 8;
            DWORD   LUT_MbMode_8_3  : 8;
            DWORD   LUT_MbMode_9_3  : 8;
        };
        struct
        {
            DWORD   Value;
        };
    } DW15;

    // DWORD 16 - DWORD 31
    VME_RD_LUT_SET_G6   RdLutSet[NUM_MBMODE_SETS_G6];
} VME_STATE_G6, *PVME_STATE_G6;

#define CM_MAX_DEPENDENCY_COUNT         8

typedef enum _CM_DEPENDENCY_PATTERN
{
    CM_NONE_DEPENDENCY          = 0,    //All threads run parallel, scanline dispatch
    CM_WAVEFRONT                = 1,
    CM_WAVEFRONT26              = 2,
    CM_VERTICAL_DEPENDENCY      = 3,
    CM_HORIZONTAL_DEPENDENCY    = 4,
    CM_WAVEFRONT26Z             = 5,
    CM_WAVEFRONT26X                         = 6,
        CM_WAVEFRONT26ZIG                   = 7,
    CM_WAVEFRONT26ZI            = 8
} CM_DEPENDENCY_PATTERN;

typedef enum _CM_WALKING_PATTERN
{
    CM_WALK_DEFAULT      = 0,
    CM_WALK_WAVEFRONT    = 1,
    CM_WALK_WAVEFRONT26  = 2,
    CM_WALK_VERTICAL     = 3,
    CM_WALK_HORIZONTAL   = 4,
        CM_WALK_WAVEFRONT26X = 5,
        CM_WALK_WAVEFRONT26ZIG = 6      
} CM_WALKING_PATTERN;

typedef struct _CM_DEPENDENCY
{
    UINT    count;
    INT     deltaX[CM_MAX_DEPENDENCY_COUNT];
    INT     deltaY[CM_MAX_DEPENDENCY_COUNT];
}CM_DEPENDENCY;

typedef enum _CM_26ZI_DISPATCH_PATTERN
{
    VVERTICAL_HVERTICAL_26           = 0,
    VVERTICAL_HHORIZONTAL_26         = 1,
    VVERTICAL26_HHORIZONTAL26        = 2,
    VVERTICAL1X26_HHORIZONTAL1X26    = 3
} CM_26ZI_DISPATCH_PATTERN;

typedef enum _CM_MW_GROUP_SELECT
{
    CM_MW_GROUP_NONE        = 0,
    CM_MW_GROUP_COLORLOOP   = 1,
    CM_MW_GROUP_INNERLOCAL  = 2,
    CM_MW_GROUP_MIDLOCAL    = 3,
    CM_MW_GROUP_OUTERLOCAL  = 4,
    CM_MW_GROUP_INNERGLOBAL = 5,
} CM_MW_GROUP_SELECT;

/**************** L3/Cache ***************/
typedef enum _MEMORY_OBJECT_CONTROL{
    // SNB
    MEMORY_OBJECT_CONTROL_USE_GTT_ENTRY,
    MEMORY_OBJECT_CONTROL_NEITHER_LLC_NOR_MLC,
    MEMORY_OBJECT_CONTROL_LLC_NOT_MLC,
    MEMORY_OBJECT_CONTROL_LLC_AND_MLC,

    // IVB
    MEMORY_OBJECT_CONTROL_FROM_GTT_ENTRY = MEMORY_OBJECT_CONTROL_USE_GTT_ENTRY,                                 // Caching dependent on pte
    MEMORY_OBJECT_CONTROL_L3,                                             // Cached in L3$
    MEMORY_OBJECT_CONTROL_LLC,                                            // Cached in LLC 
    MEMORY_OBJECT_CONTROL_LLC_L3,                                         // Cached in LLC & L3$

    // HSW
#ifdef CMRT_SIM
    MEMORY_OBJECT_CONTROL_USE_PTE, // Caching dependent on pte
#else
    MEMORY_OBJECT_CONTROL_USE_PTE = MEMORY_OBJECT_CONTROL_FROM_GTT_ENTRY, // Caching dependent on pte
#endif
    MEMORY_OBJECT_CONTROL_L3_USE_PTE,
    MEMORY_OBJECT_CONTROL_UC,                                             // Uncached
    MEMORY_OBJECT_CONTROL_L3_UC,
    MEMORY_OBJECT_CONTROL_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_L3_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_ELLC,
    MEMORY_OBJECT_CONTROL_L3_ELLC,

    // BDW
    MEMORY_OBJECT_CONTROL_BDW_ELLC_ONLY = 0,
    MEMORY_OBJECT_CONTROL_BDW_LLC_ONLY,
    MEMORY_OBJECT_CONTROL_BDW_LLC_ELLC_ALLOWED,
    MEMORY_OBJECT_CONTROL_BDW_L3_LLC_ELLC_ALLOWED,

    // SKL
    MEMORY_OBJECT_CONTROL_SKL_DEFAULT = 0,
    MEMORY_OBJECT_CONTROL_SKL_NO_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_CACHE,

    MEMORY_OBJECT_CONTROL_UNKNOWN = 0xff
} MEMORY_OBJECT_CONTROL;

typedef enum _MEMORY_TYPE {
    CM_USE_PTE,
    CM_UN_CACHEABLE,
    CM_WRITE_THROUGH,
    CM_WRITE_BACK,

    // BDW
    MEMORY_TYPE_BDW_UC_WITH_FENCE = 0,
    MEMORY_TYPE_BDW_UC,
    MEMORY_TYPE_BDW_WT,
    MEMORY_TYPE_BDW_WB

} MEMORY_TYPE;

#define SKL_L3_CONFIG_NUM 8
#define BDW_L3_CONFIG_NUM 8
#define CHV_L3_CONFIG_NUM 8
#define HSW_L3_CONFIG_NUM 12
#define IVB_2_L3_CONFIG_NUM 12
#define IVB_1_L3_CONFIG_NUM 12

typedef struct _L3_CONFIG_REGISTER_VALUES{

            UINT SQCREG1_VALUE;
            UINT CNTLREG2_VALUE;
            UINT CNTLREG3_VALUE;
            UINT CNTLREG_VALUE;

} L3_CONFIG_REGISTER_VALUES;

typedef enum _L3_SUGGEST_CONFIG
{
    IVB_L3_PLANE_DEFAULT,
    IVB_L3_PLANE_1,
    IVB_L3_PLANE_2,
    IVB_L3_PLANE_3,
    IVB_L3_PLANE_4,
    IVB_L3_PLANE_5,
    IVB_L3_PLANE_6,
    IVB_L3_PLANE_7,
    IVB_L3_PLANE_8,
    IVB_L3_PLANE_9,
    IVB_L3_PLANE_10,
    IVB_L3_PLANE_11,
    
    HSW_L3_PLANE_DEFAULT = IVB_L3_PLANE_DEFAULT,
    HSW_L3_PLANE_1,
    HSW_L3_PLANE_2,
    HSW_L3_PLANE_3,
    HSW_L3_PLANE_4,
    HSW_L3_PLANE_5,
    HSW_L3_PLANE_6,
    HSW_L3_PLANE_7,
    HSW_L3_PLANE_8,
    HSW_L3_PLANE_9,
    HSW_L3_PLANE_10,
    HSW_L3_PLANE_11,

    BDW_L3_PLANE_DEFAULT = IVB_L3_PLANE_DEFAULT,
    BDW_L3_PLANE_1,
    BDW_L3_PLANE_2,
    BDW_L3_PLANE_3,
    BDW_L3_PLANE_4,
    BDW_L3_PLANE_5,
    BDW_L3_PLANE_6,
    BDW_L3_PLANE_7,

    CHV_L3_PLANE_DEFAULT = IVB_L3_PLANE_DEFAULT,
    CHV_L3_PLANE_1,
    CHV_L3_PLANE_2,
    CHV_L3_PLANE_3,
    CHV_L3_PLANE_4,
    CHV_L3_PLANE_5,
    CHV_L3_PLANE_6,
    CHV_L3_PLANE_7,

    SKL_L3_PLANE_DEFAULT = IVB_L3_PLANE_DEFAULT,
    SKL_L3_PLANE_1,
    SKL_L3_PLANE_2,
    SKL_L3_PLANE_3,
    SKL_L3_PLANE_4,
    SKL_L3_PLANE_5,
    SKL_L3_PLANE_6,
    SKL_L3_PLANE_7,

    IVB_SLM_PLANE_DEFAULT = IVB_L3_PLANE_9,
    HSW_SLM_PLANE_DEFAULT = HSW_L3_PLANE_9,
    BDW_SLM_PLANE_DEFAULT = BDW_L3_PLANE_5,
    CHV_SLM_PLANE_DEFAULT = CHV_L3_PLANE_5,
    SKL_SLM_PLANE_DEFAULT = SKL_L3_PLANE_5
} L3_SUGGEST_CONFIG;

static const L3_CONFIG_REGISTER_VALUES IVB_L3_PLANE[ IVB_1_L3_CONFIG_NUM ] =
{                                               //SLM    URB   Rest     DC     RO     I/S     C     T      Sum
    { 0x01730000, 0x00080040, 0x00000000 },     //{0,    256,    0,     0,     256,    0,     0,    0,     512},
    { 0x00730000, 0x02040040, 0x00000000 },     //{0,    256,    0,   128,     128,    0,     0,    0,     512},
    { 0x00730000, 0x00800040, 0x00080410 },     //{0,    256,    0,    32,       0,   64,    32,  128,     512},
    { 0x00730000, 0x01000038, 0x00080410 },     //{0,    224,    0,    64,       0,   64,    32,  128,     512},
    { 0x00730000, 0x02000038, 0x00040410 },     //{0,    224,    0,   128,       0,   64,    32,   64,     512},
    { 0x00730000, 0x01000038, 0x00040420 },     //{0,    224,    0,    64,       0,  128,    32,   64,     512},
    { 0x01730000, 0x00000038, 0x00080420 },     //{0,    224,    0,     0,       0,  128,    32,  128,     512},
    { 0x01730000, 0x00000040, 0x00080020 },     //{0,    256,    0,     0,       0,  128,     0,  128,     512},
    { 0x00730000, 0x020400a1, 0x00000000 },     //{128,  128,    0,   128,     128,    0,     0,    0,     512},
    { 0x00730000, 0x010000a1, 0x00040810 },     //{128,  128,    0,    64,       0,   64,    64,   64,     512},
    { 0x00730000, 0x008000a1, 0x00080410 },     //{128,  128,    0,    32,       0,   64,    32,  128,     512},
    { 0x00730000, 0x008000a1, 0x00040420 }      //{128,  128,    0,    32,       0,  128,   32,    64,     512}
};

static const L3_CONFIG_REGISTER_VALUES HSW_L3_PLANE[ HSW_L3_CONFIG_NUM ] =
{                                               // SLM    URB   Rest   DC     RO     I/S      C    T      Sum
    { 0x01610000, 0x00080040, 0x00000000 },     //{0,    256,    0,     0,     256,    0,     0,    0,     512},
    { 0x00610000, 0x02040040, 0x00000000 },     //{0,    256,    0,   128,     128,    0,     0,    0,     512},
    { 0x00610000, 0x00800040, 0x00080410 },     //{0,    256,    0,    32,       0,   64,    32,  128,     512},
    { 0x00610000, 0x01000038, 0x00080410 },     //{0,    224,    0,    64,       0,   64,    32,  128,     512},
    { 0x00610000, 0x02000038, 0x00040410 },     //{0,    224,    0,   128,       0,   64,    32,   64,     512},
    { 0x00610000, 0x01000038, 0x00040420 },     //{0,    224,    0,    64,       0,  128,    32,   64,     512},
    { 0x01610000, 0x00000038, 0x00080420 },     //{0,    224,    0,     0,       0,  128,    32,  128,     512},
    { 0x01610000, 0x00000040, 0x00080020 },     //{0,    256,    0,     0,       0,  128,     0,  128,     512},
    { 0x00610000, 0x020400a1, 0x00000000 },     //{128,  128,    0,   128,     128,    0,     0,    0,     512},
    { 0x00610000, 0x010000a1, 0x00040810 },     //{128,  128,    0,    64,       0,   64,    64,   64,     512},
    { 0x00610000, 0x008000a1, 0x00080410 },     //{128,  128,    0,    32,       0,   64,    32,  128,     512},
    { 0x00610000, 0x008000a1, 0x00040420 }      //{128,  128,    0,    32,       0,  128,    32,   64,     512}
};

static const L3_CONFIG_REGISTER_VALUES BDW_L3_PLANE[ BDW_L3_CONFIG_NUM ] =
{                                                 // SLM    URB   Rest     DC     RO     I/S    C    T      Sum ( BDW GT2; for GT1, half of the values; for GT3, double the values )
    { 0, 0, 0, 0x60000060 },                      //{  0,   384,    384,     0,     0,    0,    0,    0,    768},
    { 0, 0, 0, 0x00410060 },                      //{  0,   384,      0,   128,   256,    0,    0,    0,    768},
    { 0, 0, 0, 0x00418040 },                      //{  0,   256,      0,   128,   384,    0,    0,    0,    768},
    { 0, 0, 0, 0x00020040 },                      //{  0,   256,      0,     0,   512,    0,    0,    0,    768},
    { 0, 0, 0, 0x80000040 },                      //{  0,   256,    512,     0,     0,    0,    0,    0,    768},
    { 0, 0, 0, 0x60000021 },                      //{192,   128,    384,     0,     0,    0,    0,    0,    768},
    { 0, 0, 0, 0x00410021 },                      //{192,   128,      0,   128,   256,    0,    0,    0,    768},
    { 0, 0, 0, 0x00808021 }                       //{192,   128,      0,   256,   128,    0,    0,    0,    768}
};

static const L3_CONFIG_REGISTER_VALUES CHV_L3_PLANE[ CHV_L3_CONFIG_NUM ] =
{                                                 // SLM    URB   Rest     DC     RO     I/S    C    T      Sum ( CHV GT1; for GT0.5, half of the values )
    { 0, 0, 0, 0x60000060 },                      //{  0,   192,    192,     0,     0,    0,    0,    0,    384},
    { 0, 0, 0, 0x00410060 },                      //{  0,   192,      0,    64,   128,    0,    0,    0,    384},
    { 0, 0, 0, 0x00418040 },                      //{  0,   128,      0,    64,   192,    0,    0,    0,    384},
    { 0, 0, 0, 0x00020040 },                      //{  0,   128,      0,     0,   256,    0,    0,    0,    384},
    { 0, 0, 0, 0x80000040 },                      //{  0,   128,    256,     0,     0,    0,    0,    0,    384},
    { 0, 0, 0, 0x60000021 },                      //{128,    64,    192,     0,     0,    0,    0,    0,    384},
    { 0, 0, 0, 0x00410021 },                      //{128,    64,      0,    64,   128,    0,    0,    0,    384},
    { 0, 0, 0, 0x00808021 }                       //{128,    64,      0,   128,    64,    0,    0,    0,    384}
};

static const L3_CONFIG_REGISTER_VALUES SKL_L3_PLANE[ SKL_L3_CONFIG_NUM ] =
{                                             // SLM    URB   Rest   DC    RO    I/S    C     T     Sum
    {0x00000000, 0x00000000, 0x00000000, 0x60000060},                             //{0,     48,    48,    0,    0,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00808060},                             //{0,     48,    0,    16,   32,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00818040},                             //{0,     32,    0,    16,   48,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00030040},                             //{0,     32,    0,     0,   64,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x80000040},                             //{0,     32,    64,    0,    0,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x60000121},                             //{32,    16,    48,    0,    0,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00410121},                            //{32,    16,    0,    16,   32,    0,    0,    0,    96},
    {0x00000000, 0x00000000, 0x00000000,0x00808121}                             //{32,    16,    0,    32,   16,    0,    0,    0,    96}
};
/***********START SAMPLER8X8******************/
//Sampler8x8 data structures

typedef enum _CM_SAMPLER8x8_SURFACE_
{
    CM_AVS_SURFACE = 0,
    CM_VA_SURFACE = 1
}CM_SAMPLER8x8_SURFACE;

typedef enum _CM_VA_PLUS_SURFACE_
{
    CM_VA_PLUS_OTHER = 0,
    CM_VA_PLUS_CORRECLATION_SEARCH_SURFACE = 1,
    CM_VA_PLUS_LBP_CORRELATION_SURFACE = 2
}CM_VA_PLUS_SURFACE;

typedef enum _CM_SURFACE_ADDRESS_CONTROL_MODE_
{
    CM_SURFACE_CLAMP = 0,
    CM_SURFACE_MIRROR = 1
}CM_SURFACE_ADDRESS_CONTROL_MODE;

typedef enum _CM_MESSAGE_SEQUENCE_
{
    CM_MS_1x1   = 0,
    CM_MS_16x1  = 1,
    CM_MS_16x4  = 2,
    CM_MS_32x1  = 3,
    CM_MS_32x4  = 4,
    CM_MS_64x1  = 5,
    CM_MS_64x4  = 6
}CM_MESSAGE_SEQUENCE;

typedef enum _CM_MIN_MAX_FILTER_CONTROL_
{
    CM_MIN_FILTER   = 0,
    CM_MAX_FILTER   = 1,
    CM_BOTH_FILTER  = 3
}CM_MIN_MAX_FILTER_CONTROL;

typedef enum _CM_VA_FUNCTION_
{
    CM_VA_MINMAXFILTER  = 0,
    CM_VA_DILATE        = 1,
    CM_VA_ERODE         = 2
} CM_VA_FUNCTION;

typedef enum _CM_EVENT_PROFILING_INFO
{
    CM_EVENT_PROFILING_HWSTART,
    CM_EVENT_PROFILING_HWEND,
    CM_EVENT_PROFILING_SUBMIT,
    CM_EVENT_PROFILING_COMPLETE,
    CM_EVENT_PROFILING_KERNELCOUNT,
    CM_EVENT_PROFILING_KERNELNAMES,
    CM_EVENT_PROFILING_THREADSPACE
}CM_EVENT_PROFILING_INFO;

//GT-PIN
typedef struct _CM_SURFACE_DETAILS{
    UINT        width; 
    UINT        height; 
    UINT        depth; 
    CM_SURFACE_FORMAT   format; 
    UINT        planeIndex;
    UINT        pitch; 
    UINT        slicePitch;
    UINT        SurfaceBaseAddress;
    UINT8       TiledSurface;
    UINT8       TileWalk;
    UINT        XOffset;
    UINT        YOffset; 
    
}CM_SURFACE_DETAILS;


/*
 *  AVS SAMPLER8x8 STATE
 */
typedef struct _CM_AVS_COEFF_TABLE{
    float   FilterCoeff_0_0;
    float   FilterCoeff_0_1;
    float   FilterCoeff_0_2;
    float   FilterCoeff_0_3;
    float   FilterCoeff_0_4;
    float   FilterCoeff_0_5;
    float   FilterCoeff_0_6;
    float   FilterCoeff_0_7;
}CM_AVS_COEFF_TABLE;

typedef struct _CM_AVS_INTERNEL_COEFF_TABLE{
    BYTE   FilterCoeff_0_0;
    BYTE   FilterCoeff_0_1;
    BYTE   FilterCoeff_0_2;
    BYTE   FilterCoeff_0_3;
    BYTE   FilterCoeff_0_4;
    BYTE   FilterCoeff_0_5;
    BYTE   FilterCoeff_0_6;
    BYTE   FilterCoeff_0_7;
}CM_AVS_INTERNEL_COEFF_TABLE;

#define CM_NUM_COEFF_ROWS 17
#define CM_NUM_COEFF_ROWS_SKL 32
typedef struct _CM_AVS_NONPIPLINED_STATE{
    bool BypassXAF;
    bool BypassYAF;
    BYTE DefaultSharpLvl;
    BYTE maxDerivative4Pixels;
    BYTE maxDerivative8Pixels;
    BYTE transitionArea4Pixels;
    BYTE transitionArea8Pixels;    
    CM_AVS_COEFF_TABLE Tbl0X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_COEFF_TABLE Tbl0Y[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_COEFF_TABLE Tbl1X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_COEFF_TABLE Tbl1Y[ CM_NUM_COEFF_ROWS_SKL ];
}CM_AVS_NONPIPLINED_STATE;

typedef struct _CM_AVS_INTERNEL_NONPIPLINED_STATE{
    bool BypassXAF;
    bool BypassYAF;
    BYTE DefaultSharpLvl;
    BYTE maxDerivative4Pixels;
    BYTE maxDerivative8Pixels;
    BYTE transitionArea4Pixels;
    BYTE transitionArea8Pixels;    
    CM_AVS_INTERNEL_COEFF_TABLE Tbl0X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_INTERNEL_COEFF_TABLE Tbl0Y[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_INTERNEL_COEFF_TABLE Tbl1X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_INTERNEL_COEFF_TABLE Tbl1Y[ CM_NUM_COEFF_ROWS_SKL ];
}CM_AVS_INTERNEL_NONPIPLINED_STATE;

typedef struct _CM_AVS_STATE_MSG{
    bool AVSTYPE; //true nearest, false adaptive    
    bool EightTapAFEnable; //HSW+
    bool BypassIEF; //ignored for BWL, moved to sampler8x8 payload.
    bool ShuffleOutputWriteback; //SKL mode only to be set when AVS msg sequence is 4x4 or 8x4
    bool HDCDirectWriteEnable;
    unsigned short GainFactor;
    unsigned char GlobalNoiseEstm;
    unsigned char StrongEdgeThr;
    unsigned char WeakEdgeThr;
    unsigned char StrongEdgeWght;
    unsigned char RegularWght;
    unsigned char NonEdgeWght;
    unsigned short wR3xCoefficient;
    unsigned short wR3cCoefficient;  
    unsigned short wR5xCoefficient;
    unsigned short wR5cxCoefficient;
    unsigned short wR5cCoefficient;    
    //For Non-piplined states
    unsigned short stateID;
    CM_AVS_NONPIPLINED_STATE * AvsState;
} CM_AVS_STATE_MSG;

/*
 *  CONVOLVE STATE DATA STRUCTURES
 */

typedef struct _CM_CONVOLVE_COEFF_TABLE{
    float   FilterCoeff_0_0;
    float   FilterCoeff_0_1;
    float   FilterCoeff_0_2;
    float   FilterCoeff_0_3;
    float   FilterCoeff_0_4;
    float   FilterCoeff_0_5;
    float   FilterCoeff_0_6;
    float   FilterCoeff_0_7;
    float   FilterCoeff_0_8;
    float   FilterCoeff_0_9;
    float   FilterCoeff_0_10;
    float   FilterCoeff_0_11;
    float   FilterCoeff_0_12;
    float   FilterCoeff_0_13;
    float   FilterCoeff_0_14;
    float   FilterCoeff_0_15;
    float   FilterCoeff_0_16;    
    float   FilterCoeff_0_17;
    float   FilterCoeff_0_18;
    float   FilterCoeff_0_19;
    float   FilterCoeff_0_20;
    float   FilterCoeff_0_21;
    float   FilterCoeff_0_22;
    float   FilterCoeff_0_23;
    float   FilterCoeff_0_24;
    float   FilterCoeff_0_25;
    float   FilterCoeff_0_26;
    float   FilterCoeff_0_27;
    float   FilterCoeff_0_28;
    float   FilterCoeff_0_29;
    float   FilterCoeff_0_30;
    float   FilterCoeff_0_31;
}CM_CONVOLVE_COEFF_TABLE;

//*-----------------------------------------------------------------------------
//| CM Convolve type for SKL+
//*-----------------------------------------------------------------------------
typedef enum _CM_CONVOLVE_SKL_TYPE
{
    CM_CONVOLVE_SKL_TYPE_2D = 0,
    CM_CONVOLVE_SKL_TYPE_1D = 1,
    CM_CONVOLVE_SKL_TYPE_1P = 2
} CM_CONVOLVE_SKL_TYPE;

#define CM_NUM_CONVOLVE_ROWS 16
#define CM_NUM_CONVOLVE_ROWS_SKL 32
typedef struct _CM_CONVOLVE_STATE_MSG{
        bool CoeffSize; //true 16-bit, false 8-bit
        byte SclDwnValue; //Scale down value
        byte Width; //Kernel Width
        byte Height; //Kernel Height   
        //SKL mode
        bool isVertical32Mode;
        bool isHorizontal32Mode;
        CM_CONVOLVE_SKL_TYPE nConvolveType;
    CM_CONVOLVE_COEFF_TABLE Table[CM_NUM_CONVOLVE_ROWS_SKL];
} CM_CONVOLVE_STATE_MSG;

/*
 *   MISC SAMPLER8x8 State
 */
typedef struct _CM_MISC_STATE {
    //DWORD 0
    union{
        struct{
            DWORD   Row0      : 16;
            DWORD   Reserved  : 8;
            DWORD   Width     : 4;
            DWORD   Height    : 4;
        };
        struct{
            DWORD value;
        };
    } DW0;

    //DWORD 1
    union{
        struct{
            DWORD   Row1      : 16;
            DWORD   Row2      : 16;
        };
        struct{
            DWORD value;
        };
    } DW1;

    //DWORD 2
    union{
        struct{
            DWORD   Row3      : 16;
            DWORD   Row4      : 16;
        };
        struct{
            DWORD value;
        };
    } DW2;

    //DWORD 3
    union{
        struct{
            DWORD   Row5      : 16;
            DWORD   Row6      : 16;
        };
        struct{
            DWORD value;
        };
    } DW3;

    //DWORD 4
    union{
        struct{
            DWORD   Row7      : 16;
            DWORD   Row8      : 16;
        };
        struct{
            DWORD value;
        };
    } DW4;

    //DWORD 5
    union{
        struct{
            DWORD   Row9      : 16;
            DWORD   Row10      : 16;
        };
        struct{
            DWORD value;
        };
    } DW5;

    //DWORD 6
    union{
        struct{
            DWORD   Row11      : 16;
            DWORD   Row12      : 16;
        };
        struct{
            DWORD value;
        };
    } DW6;

    //DWORD 7
    union{
        struct{
            DWORD   Row13      : 16;
            DWORD   Row14      : 16;
        };
        struct{
            DWORD value;
        };
    } DW7;
} CM_MISC_STATE;
 
typedef struct _CM_MISC_STATE_MSG{
    //DWORD 0
    union{
        struct{
            DWORD   Row0      : 16;
            DWORD   Reserved  : 8;
            DWORD   Width     : 4;
            DWORD   Height    : 4;
        };
        struct{
            DWORD value;
        };
    }DW0;

    //DWORD 1
    union{
        struct{
            DWORD   Row1      : 16;
            DWORD   Row2      : 16;
        };
        struct{
            DWORD value;
        };
    }DW1;

    //DWORD 2
    union{
        struct{
            DWORD   Row3      : 16;
            DWORD   Row4      : 16;
        };
        struct{
            DWORD value;
        };
    }DW2;

    //DWORD 3
    union{
        struct{
            DWORD   Row5      : 16;
            DWORD   Row6      : 16;
        };
        struct{
            DWORD value;
        };
    }DW3;

    //DWORD 4
    union{
        struct{
            DWORD   Row7      : 16;
            DWORD   Row8      : 16;
        };
        struct{
            DWORD value;
        };
    }DW4;

    //DWORD 5
    union{
        struct{
            DWORD   Row9      : 16;
            DWORD   Row10      : 16;
        };
        struct{
            DWORD value;
        };
    }DW5;

    //DWORD 6
    union{
        struct{
            DWORD   Row11      : 16;
            DWORD   Row12      : 16;
        };
        struct{
            DWORD value;
        };
    }DW6;

    //DWORD 7
    union{
        struct{
            DWORD   Row13      : 16;
            DWORD   Row14      : 16;
        };
        struct{
            DWORD value;
        };
    }DW7;
} CM_MISC_STATE_MSG;

typedef enum _CM_SAMPLER_STATE_TYPE_
{
    CM_SAMPLER8X8_AVS   = 0,
    CM_SAMPLER8X8_CONV  = 1,
    CM_SAMPLER8X8_MISC  = 3,
    CM_SAMPLER8X8_NONE
}CM_SAMPLER_STATE_TYPE;

typedef struct _CM_SAMPLER_8X8_DESCR{
    CM_SAMPLER_STATE_TYPE stateType;
    union
    {
        CM_AVS_STATE_MSG * avs;
        CM_CONVOLVE_STATE_MSG * conv;
        CM_MISC_STATE_MSG * misc; //ERODE/DILATE/MINMAX
    };
} CM_SAMPLER_8X8_DESCR;

typedef  struct _CM_HAL_AVS_PARAM {
    CM_AVS_STATE_MSG avs;
    CM_AVS_INTERNEL_NONPIPLINED_STATE avs_nonpipelined;
} CM_HAL_AVS_PARAM;

typedef struct _CM_SAMPLER_8X8_STATE {
    CM_SAMPLER_STATE_TYPE stateType;
    union {
        CM_HAL_AVS_PARAM           avs_state;
        CM_CONVOLVE_STATE_MSG convolve_state;
        CM_MISC_STATE                  misc_state;
    };
} CM_SAMPLER_8X8_STATE;

typedef struct _CM_HAL_SAMPLER_8X8_STATE_PARAM{
    CM_SAMPLER_8X8_STATE    sampler8x8State;
    DWORD                              dwHandle;                                       // [out] Handle 
} CM_HAL_SAMPLER_8X8_STATE_PARAM;

//GT-PINS urfaceDetails
typedef struct _CM_HAL_SURFACE_DETAILS{
    CM_SAMPLER_8X8_STATE    sampler8x8State;
    DWORD                   dwHandle;                                       // [out] Handle 
} CM_HAL_SURFACE_DETAILS;

// Defined in vol2b "Media"
typedef struct _VEBOX_DNDI_STATE_G75
{
    // DWORD 0
    union
    {
        struct
        {
            DWORD       DWordLength                         : 12;
            DWORD                                           : 4;
            DWORD       InstructionSubOpcodeB               : 5;
            DWORD       InstructionSubOpcodeA               : 3;
            DWORD       InstructionOpcode                   : 3;
            DWORD       InstructionPipeline                 : 2;
            DWORD       CommandType                         : 3;
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       DenoiseASDThreshold                 : 8; // U8
            DWORD       DnmhDelta                           : 4; // UINT4
            DWORD                                           : 4; // Reserved
            DWORD       DnmhHistoryMax                      : 8; // U8
            DWORD       DenoiseSTADThreshold                : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       SCMDenoiseThreshold                 : 8; // U8
            DWORD       DenoiseMovingPixelThreshold         : 5; // U5
            DWORD       STMMC2                              : 3; // U3
            DWORD       LowTemporalDifferenceThreshold      : 6; // U6
            DWORD                                           : 2; // Reserved
            DWORD       TemporalDifferenceThreshold         : 6; // U6
            DWORD                                           : 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            DWORD       BlockNoiseEstimateNoiseThreshold    : 8; // U8
            DWORD       BneEdgeTh                           : 4; // UINT4
            DWORD                                           : 2; // Reserved
            DWORD       SmoothMvTh                          : 2; // U2
            DWORD       SADTightTh                          : 4; // U4
            DWORD       CATSlopeMinus1                      : 4; // U4
            DWORD       GoodNeighborThreshold               : 6; // UINT6
            DWORD                                           : 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       MaximumSTMM                         : 8; // U8
            DWORD       MultiplierforVECM                   : 6; // U6
            DWORD                                           : 2;
            DWORD       BlendingConstantForSmallSTMM        : 8; // U8
            DWORD       BlendingConstantForLargeSTMM        : 7; // U7
            DWORD       STMMBlendingConstantSelect          : 1; // U1
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       SDIDelta                            : 8; // U8
            DWORD       SDIThreshold                        : 8; // U8
            DWORD       STMMOutputShift                     : 4; // U4
            DWORD       STMMShiftUp                         : 2; // U2
            DWORD       STMMShiftDown                       : 2; // U2
            DWORD       MinimumSTMM                         : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            DWORD       FMDTemporalDifferenceThreshold      : 8; // U8
            DWORD       SDIFallbackMode2Constant            : 8; // U8
            DWORD       SDIFallbackMode1T2Constant          : 8; // U8
            DWORD       SDIFallbackMode1T1Constant          : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD                                           : 3; // Reserved
            DWORD       DNDITopFirst                        : 1; // Enable
            DWORD                                           : 2; // Reserved
            DWORD       ProgressiveDN                       : 1; // Enable
            DWORD       MCDIEnable                          : 1;
            DWORD       FMDTearThreshold                    : 6; // U6
            DWORD       CATTh1                              : 2; // U2
            DWORD       FMD2VerticalDifferenceThreshold     : 8; // U8
            DWORD       FMD1VerticalDifferenceThreshold     : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            DWORD       SADTHA                              : 4; // U4
            DWORD       SADTHB                              : 4; // U4
            DWORD       FMDFirstFieldCurrentFrame           : 2; // U2
            DWORD       MCPixelConsistencyTh                : 6; // U6
            DWORD       FMDSecondFieldPreviousFrame         : 2; // U2
            DWORD                                           : 1; // Reserved
            DWORD       NeighborPixelTh                     : 4; // U4
            DWORD       DnmhHistoryInit                     : 6; // U6
            DWORD                                           : 3; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       ChromaLTDThreshold                  : 6; // U6
            DWORD       ChromaTDTheshold                    : 6; // U6
            DWORD       ChromaDenoiseEnable                 : 1; // Enable
            DWORD                                           : 3; // Reserved
            DWORD       ChromaDnSTADThreshold               : 8; // U8
            DWORD                                           : 8; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // Padding for 32-byte alignment, VEBOX_DNDI_STATE_G75 is 10 DWORDs
    DWORD dwPad[6];
} VEBOX_DNDI_STATE_G75, *PVEBOX_DNDI_STATE_G75;

// Defined in vol2b "Media"
typedef struct _VEBOX_IECP_STATE_G75
{
    // STD/STE state
    // DWORD 0
    union
    {
        struct
        {
            DWORD       STDEnable                       : 1;    
            DWORD       STEEnable                       : 1;    
            DWORD       OutputCtrl                      : 1;    
            DWORD                                       : 1;    
            DWORD       SatMax                          : 6;    // U6;
            DWORD       HueMax                          : 6;    // U6;
            DWORD       UMid                            : 8;    // U8;
            DWORD       VMid                            : 8;    // U8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       SinAlpha                        : 8;    // S0.7
            DWORD                                       : 2; 
            DWORD       CosAlpha                        : 8;    // S0.7    
            DWORD       HSMargin                        : 3;    // U3   
            DWORD       DiamondDu                       : 7;    // S7    
            DWORD       DiamondMargin                   : 3;    // U3    
            DWORD                                       : 1;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       DiamondDv                       : 7;    // S7  
            DWORD       DiamondTh                       : 6;    // U6  
            DWORD       DiamondAlpha                    : 8;    // U2.6  
            DWORD                                       : 11;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            DWORD                                       : 7;  
            DWORD       VYSTDEnable                     : 1;    
            DWORD       YPoint1                         : 8;    // U8   
            DWORD       YPoint2                         : 8;    // U8   
            DWORD       YPoint3                         : 8;    // U8   
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       YPoint4                         : 8;    // U8
            DWORD       YSlope1                         : 5;    // U2.3
            DWORD       YSlope2                         : 5;    // U2.3
            DWORD                                       : 14;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       INVMarginVYL                    : 16;    // U0.16
            DWORD       INVSkinTypesMargin              : 16;    // U0.16
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            DWORD       INVMarginVYU                    : 16;    // U0.16
            DWORD       P0L                             : 8;     // U8
            DWORD       P1L                             : 8;     // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD       P2L                             : 8;     // U8
            DWORD       P3L                             : 8;     // U8
            DWORD       B0L                             : 8;     // U8
            DWORD       B1L                             : 8;     // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            DWORD       B2L                             : 8;     // U8
            DWORD       B3L                             : 8;     // U8
            DWORD       S0L                             : 11;    // S2.8
            DWORD                                       : 5;     
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       S1L                             : 11;    // S2.8
            DWORD       S2L                             : 11;    // S2.8
            DWORD                                       : 10;     
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // DWORD 10
    union
    {
        struct
        {
            DWORD       S3L                             : 11;    // S2.8
            DWORD       P0U                             : 8;     // U8
            DWORD       P1U                             : 8;     // U8
            DWORD                                       : 5;     
        };
        struct
        {
            DWORD       Value;
        };
    } DW10;

    // DWORD 11
    union
    {
        struct
        {
            DWORD       P2U                             : 8;     // U8
            DWORD       P3U                             : 8;     // U8
            DWORD       B0U                             : 8;     // U8
            DWORD       B1U                             : 8;     // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW11;

    // DWORD 12
    union
    {
        struct
        {
            DWORD       B2U                             : 8;     // U8
            DWORD       B3U                             : 8;     // U8
            DWORD       S0U                             : 11;    // S2.8
            DWORD                                       : 5;     
        };
        struct
        {
            DWORD       Value;
        };
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            DWORD       S1U                             : 11;     // S2.8
            DWORD       S2U                             : 11;     // S2.8
            DWORD                                       : 10;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            DWORD       S3U                             : 11;     // S2.8
            DWORD       SkinTypesEnable                 : 1;
            DWORD       SkinTypesThresh                 : 8;      // U8
            DWORD       SkinTypesMargin                 : 8;      // U8
            DWORD                                       : 4;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            DWORD       SATP1                           : 7;     // S6
            DWORD       SATP2                           : 7;     // S6
            DWORD       SATP3                           : 7;     // S6
            DWORD       SATB1                           : 10;    // S7.2
            DWORD                                       : 1;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW15;

    // DWORD 16
    union
    {
        struct
        {
            DWORD       SATB2                           : 10;     // S7.2
            DWORD       SATB3                           : 10;     // S7.2
            DWORD       SATS0                           : 11;     // U3.8
            DWORD                                       : 1;    
        };
        struct
        {
            DWORD       Value;
        };
    } DW16;

    // DWORD 17
    union
    {
        struct
        {
            DWORD       SATS1                           : 11;     // U3.8
            DWORD       SATS2                           : 11;     // U3.8
            DWORD                                       : 10;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW17;

    // DWORD 18
    union
    {
        struct
        {
            DWORD       SATS3                           : 11;    // U3.8
            DWORD       HUEP1                           : 7;     // U3.8
            DWORD       HUEP2                           : 7;     // U3.8
            DWORD       HUEP3                           : 7;     // U3.8
        };
        struct
        {
            DWORD       Value;
        };
    } DW18;

    // DWORD 19
    union
    {
        struct
        {
            DWORD       HUEB1                           : 10;    // S7.2
            DWORD       HUEB2                           : 10;    // S7.2
            DWORD       HUEB3                           : 10;    // S7.2
            DWORD                                       : 2; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW19;

    // DWORD 20
    union
    {
        struct
        {
            DWORD       HUES0                           : 11;    // U3.8
            DWORD       HUES1                           : 11;    // U3.8
            DWORD                                       : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW20;

    // DWORD 21
    union
    {
        struct
        {
            DWORD       HUES2                           : 11;    // U3.8
            DWORD       HUES3                           : 11;    // U3.8
            DWORD                                       : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW21;

    // DWORD 22
    union
    {
        struct
        {
            DWORD       SATP1DARK                      : 7;     // S6
            DWORD       SATP2DARK                      : 7;     // S6
            DWORD       SATP3DARK                      : 7;     // S6
            DWORD       SATB1DARK                      : 10;    // S7.2
            DWORD                                      : 1; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW22;

    // DWORD 23
    union
    {
        struct
        {
            DWORD       SATB2DARK                      : 10;    // S7.2
            DWORD       SATB3DARK                      : 10;    // S7.2
            DWORD       SATS0DARK                      : 11;    // U3.8
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW23;

    // DWORD 24
    union
    {
        struct
        {
            DWORD       SATS1DARK                      : 11;    // U3.8
            DWORD       SATS2DARK                      : 11;    // U3.8
            DWORD                                      : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW24;

    // DWORD 25
    union
    {
        struct
        {
            DWORD       SATS3DARK                      : 11;   // U3.8
            DWORD       HUEP1DARK                      : 7;    // S6
            DWORD       HUEP2DARK                      : 7;    // S6
            DWORD       HUEP3DARK                      : 7;    // S6
        };
        struct
        {
            DWORD       Value;
        };
    } DW25;

    // DWORD 26
    union
    {
        struct
        {
            DWORD       HUEB1DARK                      : 10;    // S7.2
            DWORD       HUEB2DARK                      : 10;    // S7.2
            DWORD       HUEB3DARK                      : 10;    // S7.2
            DWORD                                      : 2; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW26;

    // DWORD 27
    union
    {
        struct
        {
            DWORD       HUES0DARK                      : 11;    // U3.8
            DWORD       HUES1DARK                      : 11;    // U3.8
            DWORD                                      : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW27;

    // DWORD 28
    union
    {
        struct
        {
            DWORD       HUES2DARK                      : 11;    // U3.8
            DWORD       HUES3DARK                      : 11;    // U3.8
            DWORD                                      : 10; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW28;

    // DWORD 29
    union
    {
        struct
        {
            DWORD       ACEEnable                       : 1;  
            DWORD       FullImageHistogram              : 1;
            DWORD       SkinThreshold                   : 5;    // U5
            DWORD                                       : 25; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW29;

    // DWORD 30
    union
    {
        struct
        {
            DWORD       Ymin                            : 8;  
            DWORD       Y1                              : 8;
            DWORD       Y2                              : 8;
            DWORD       Y3                              : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW30;

    // DWORD 31
    union
    {
        struct
        {
            DWORD       Y4                              : 8;  
            DWORD       Y5                              : 8;
            DWORD       Y6                              : 8;
            DWORD       Y7                              : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW31;

    // DWORD 32
    union
    {
        struct
        {
            DWORD       Y8                              : 8;  
            DWORD       Y9                              : 8;
            DWORD       Y10                             : 8;
            DWORD       Ymax                            : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW32;

    // DWORD 33
    union
    {
        struct
        {
            DWORD       B1                              : 8;  // U8
            DWORD       B2                              : 8;  // U8
            DWORD       B3                              : 8;  // U8
            DWORD       B4                              : 8;  // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW33;

    // DWORD 34
    union
    {
        struct
        {
            DWORD       B5                              : 8;  // U8
            DWORD       B6                              : 8;  // U8
            DWORD       B7                              : 8;  // U8
            DWORD       B8                              : 8;  // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW34;

    // DWORD 35
    union
    {
        struct
        {
            DWORD       B9                              : 8;   // U8
            DWORD       B10                             : 8;   // U8
            DWORD                                       : 16;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW35;

    // DWORD 36
    union
    {
        struct
        {
            DWORD       S0                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S1                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW36;

    // DWORD 37
    union
    {
        struct
        {
            DWORD       S2                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S3                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW37;

    // DWORD 38
    union
    {
        struct
        {
            DWORD       S4                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S5                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW38;

    // DWORD 39
    union
    {
        struct
        {
            DWORD       S6                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S7                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW39;

    // DWORD 40
    union
    {
        struct
        {
            DWORD       S8                              : 11;   // U11
            DWORD                                       : 5; 
            DWORD       S9                              : 11;   // U11
            DWORD                                       : 5; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW40;

    // DWORD 41
    union
    {
        struct
        {
            DWORD       S10                             : 11;   // U11
            DWORD                                       : 21; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW41;

    // TCC State
    // DWORD 42
    union
    {
        struct
        {
            DWORD                                       : 7; 
            DWORD       TCCEnable                       : 1; 
            DWORD       SatFactor1                      : 8;    // U8 
            DWORD       SatFactor2                      : 8;    // U8 
            DWORD       SatFactor3                      : 8;    // U8 
        };
        struct
        {
            DWORD       Value;
        };
    } DW42;

    // DWORD 43
    union
    {
        struct
        {
            DWORD                                       : 8; 
            DWORD       SatFactor4                      : 8;    // U8 
            DWORD       SatFactor5                      : 8;    // U8 
            DWORD       SatFactor6                      : 8;    // U8 
        };
        struct
        {
            DWORD       Value;
        };
    } DW43;

    // DWORD 44
    union
    {
        struct
        {
            DWORD       BaseColor1                      : 10;    // U10 
            DWORD       BaseColor2                      : 10;    // U10 
            DWORD       BaseColor3                      : 10;    // U10 
            DWORD                                       : 2; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW44;

    // DWORD 45
    union
    {
        struct
        {
            DWORD       BaseColor4                      : 10;    // U10 
            DWORD       BaseColor5                      : 10;    // U10 
            DWORD       BaseColor6                      : 10;    // U10 
            DWORD                                       : 2; 
        };
        struct
        {
            DWORD       Value;
        };
    } DW45;

    // DWORD 46
    union
    {
        struct
        {
            DWORD       ColorTransitSlope12              : 16;    // U16
            DWORD       ColorTransitSlope23              : 16;    // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW46;

    // DWORD 47
    union
    {
        struct
        {
            DWORD       ColorTransitSlope34              : 16;    // U16
            DWORD       ColorTransitSlope45              : 16;    // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW47;

    // DWORD 48
    union
    {
        struct
        {
            DWORD       ColorTransitSlope56              : 16;    // U16
            DWORD       ColorTransitSlope61              : 16;    // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW48;

    // DWORD 49
    union
    {
        struct
        {
            DWORD                                       : 2;    
            DWORD       ColorBias1                      : 10;    // U10
            DWORD       ColorBias2                      : 10;    // U10
            DWORD       ColorBias3                      : 10;    // U10
        };
        struct
        {
            DWORD       Value;
        };
    } DW49;

    // DWORD 50
    union
    {
        struct
        {
            DWORD                                       : 2;    
            DWORD       ColorBias4                      : 10;    // U10
            DWORD       ColorBias5                      : 10;    // U10
            DWORD       ColorBias6                      : 10;    // U10
        };
        struct
        {
            DWORD       Value;
        };
    } DW50;

    // DWORD 51
    union
    {
        struct
        {
            DWORD       STESlopeBits                    : 3;    // U3
            DWORD                                       : 5;
            DWORD       STEThreshold                    : 5;    // U5
            DWORD                                       : 3;
            DWORD       UVThresholdBits                 : 3;    // U5
            DWORD                                       : 5;
            DWORD       UVThreshold                     : 7;    // U7
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW51;

    // DWORD 52
    union
    {
        struct
        {
            DWORD       UVMaxColor                      : 9;    // U9
            DWORD                                       : 7;
            DWORD       InvUVMaxColor                   : 16;   // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW52;

    // ProcAmp
    // DWORD 53
    union
    {
        struct
        {
            DWORD       ProcAmpEnable                   : 1;    
            DWORD       Brightness                      : 12;    // S7.4
            DWORD                                       : 4;
            DWORD       Contrast                        : 11;    // U7.4
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW53;

    // DWORD 54
    union
    {
        struct
        {
            DWORD       SINCS                           : 16;    // S7.8    
            DWORD       COSCS                           : 16;    // S7.8
        };
        struct
        {
            DWORD       Value;
        };
    } DW54;

    // CSC State
    // DWORD 55
    union
    {
        struct
        {
            DWORD       TransformEnable                 : 1;
            DWORD       YUVChannelSwap                  : 1;
            DWORD                                       : 1;
            DWORD       C0                              : 13;  // S2.10
            DWORD       C1                              : 13;  // S2.10
            DWORD                                       : 3;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW55;

    // DWORD 56
    union
    {
        struct
        {
            DWORD       C2                              : 13;  // S2.10
            DWORD       C3                              : 13;  // S2.10
            DWORD                                       : 6;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW56;

    // DWORD 57
    union
    {
        struct
        {
            DWORD       C4                              : 13;  // S2.10
            DWORD       C5                              : 13;  // S2.10
            DWORD                                       : 6;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW57;

    // DWORD 58
    union
    {
        struct
        {
            DWORD       C6                              : 13;  // S2.10
            DWORD       C7                              : 13;  // S2.10
            DWORD                                       : 6;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW58;

    // DWORD 59
    union
    {
        struct
        {
            DWORD       C8                              : 13;   // S2.10
            DWORD                                       : 19;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW59;

    // DWORD 60
    union
    {
        struct
        {
            DWORD       OffsetIn1                       : 11;   // S8.2
            DWORD       OffsetOut1                      : 11;   // S8.2
            DWORD                                       : 10;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW60;

    // DWORD 61
    union
    {
        struct
        {
            DWORD       OffsetIn2                       : 11;   // S8.2
            DWORD       OffsetOut2                      : 11;   // S8.2
            DWORD                                       : 10;
        };
        struct
        {
            DWORD       Value;
        };
    } DW61;

    // DWORD 62
    union
    {
        struct
        {
            DWORD       OffsetIn3                       : 11;  // S8.2
            DWORD       OffsetOut3                      : 11;  // S8.2
            DWORD                                       : 10;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW62;

    // DWORD 63
    union
    {
        struct
        {
            DWORD       ColorPipeAlpha                  : 12;  // U12
            DWORD                                       : 4;
            DWORD       AlphaFromStateSelect            : 1;   
            DWORD                                       : 15;  
        };
        struct
        {
            DWORD       Value;
        };
    } DW63;

    // Area of Interest
    // DWORD 64
    union
    {
        struct
        {
            DWORD       AOIMinX                         : 16;  // U16
            DWORD       AOIMaxX                         : 16;  // U16
    };
        struct
        {
            DWORD       Value;
        };
    } DW64;

    // DWORD 65
    union
    {
        struct
        {
            DWORD       AOIMinY                         : 16;  // U16
            DWORD       AOIMaxY                         : 16;  // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW65;

    // Padding for 32-byte alignment, VEBOX_IECP_STATE_G75 is 66 DWORDs
    DWORD dwPad[6];
} VEBOX_IECP_STATE_G75, *PVEBOX_IECP_STATE_G75;

// Defined in vol2b "Media"
typedef struct _VEBOX_GAMUT_STATE_G75
{
    // GEC State
    // DWORD 0
    union
    {
        struct
        {
            DWORD       WeightingFactorForGainFactor    : 10;
            DWORD                                       : 5;
            DWORD       GlobalModeEnable                : 1;
            DWORD       GainFactorR                     : 9;
            DWORD                                       : 7;
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       GainFactorB                     : 7;
            DWORD                                       : 1;
            DWORD       GainFactorG                     : 7;
            DWORD                                       : 1;
            DWORD       AccurateColorComponentScaling   : 10;
            DWORD                                       : 6;
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       RedOffset                       : 8;
            DWORD       AccurateColorComponentOffset    : 8;
            DWORD       RedScaling                      : 10;
            DWORD                                           : 6;
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // 3x3 Transform Coefficient
    // DWORD 3
    union
    {
        struct
        {
            DWORD       C0Coeff                         : 15; 
            DWORD                                       : 1;
            DWORD       C1Coeff                         : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       C2Coeff                         : 15;
            DWORD                                       : 1;
            DWORD       C3Coeff                         : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       C4Coeff                         : 15;
            DWORD                                       : 1;
            DWORD       C5Coeff                         : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            DWORD       C6Coeff                         : 15;
            DWORD                                       : 1;
            DWORD       C7Coeff                         : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD       C8Coeff                         : 15;
            DWORD                                       : 17;
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;
    
    // PWL Values for Gamma Correction
    // DWORD 8
    union
    {
        struct
        {
            DWORD       PWLGammaPoint1                  : 8;
            DWORD       PWLGammaPoint2                  : 8;
            DWORD       PWLGammaPoint3                  : 8;
            DWORD       PWLGammaPoint4                  : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       PWLGammaPoint5                  : 8;
            DWORD       PWLGammaPoint6                  : 8;
            DWORD       PWLGammaPoint7                  : 8;
            DWORD       PWLGammaPoint8                  : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // DWORD 10
    union
    {
        struct
        {
            DWORD       PWLGammaPoint9                  : 8;
            DWORD       PWLGammaPoint10                 : 8;
            DWORD       PWLGammaPoint11                 : 8;
            DWORD                                       : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW10;

    // DWORD 11
    union
    {
        struct
        {
            DWORD       PWLGammaBias1                   : 8;
            DWORD       PWLGammaBias2                   : 8;
            DWORD       PWLGammaBias3                   : 8;
            DWORD       PWLGammaBias4                   : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW11;


    // DWORD 12
    union
    {
        struct
        {
            DWORD       PWLGammaBias5                   : 8;
            DWORD       PWLGammaBias6                   : 8;
            DWORD       PWLGammaBias7                   : 8;
            DWORD       PWLGammaBias8                   : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            DWORD       PWLGammaBias9                   : 8;
            DWORD       PWLGammaBias10                  : 8;   
            DWORD       PWLGammaBias11                  : 8;
            DWORD                                       : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            DWORD       PWLGammaSlope0                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope1                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            DWORD       PWLGammaSlope2                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope3                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW15;

    // DWORD 16
    union
    {
        struct
        {
            DWORD       PWLGammaSlope4                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope5                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW16;

    // DWORD 17
    union
    {
        struct
        {
            DWORD       PWLGammaSlope6                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope7                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW17;

    // DWORD 18
    union
    {
        struct
        {
            DWORD       PWLGammaSlope8                  : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope9                  : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW18;

    // DWORD 19
    union
    {
        struct
        {
            DWORD       PWLGammaSlope10                 : 12;
            DWORD                                       : 4; 
            DWORD       PWLGammaSlope11                 : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW19;

    // PWL Values for Inverse Gamma Correction
    // DWORD 20
    union
    {
        struct
        {
            DWORD       PWLInvGammaPoint1               : 8;
            DWORD       PWLInvGammaPoint2               : 8;
            DWORD       PWLInvGammaPoint3               : 8;
            DWORD       PWLInvGammaPoint4               : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW20;

    // DWORD 21
    union
    {
        struct
        {
            DWORD       PWLInvGammaPoint5               : 8;
            DWORD       PWLInvGammaPoint6               : 8;
            DWORD       PWLInvGammaPoint7               : 8;
            DWORD       PWLInvGammaPoint8               : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW21;

    // DWORD 22
    union
    {
        struct
        {
            DWORD       PWLInvGammaPoint9               : 8;
            DWORD       PWLInvGammaPoint10              : 8;
            DWORD       PWLInvGammaPoint11              : 8;
            DWORD                                       : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW22;

    // DWORD 23
    union
    {
        struct
        {
            DWORD       PWLInvGammaBias1                : 8;
            DWORD       PWLInvGammaBias2                : 8;
            DWORD       PWLInvGammaBias3                : 8;
            DWORD       PWLInvGammaBias4                : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW23;

    // DWORD 24
    union
    {
        struct
        {
            DWORD       PWLInvGammaBias5                : 8;
            DWORD       PWLInvGammaBias6                : 8;
            DWORD       PWLInvGammaBias7                : 8;
            DWORD       PWLInvGammaBias8                : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW24;

    // DWORD 25
    union
    {
        struct
        {
            DWORD       PWLInvGammaBias9                : 8;
            DWORD       PWLInvGammaBias10               : 8;
            DWORD       PWLInvGammaBias11               : 8;
            DWORD                                       : 8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW25;

    // DWORD 26
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope0               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope1               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW26;

    // DWORD 27
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope2               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope3               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW27;

    // DWORD 28
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope4               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope5               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW28;

    // DWORD 29
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope6               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope7               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW29;

    // DWORD 30
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope8               : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope9               : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW30;

    // DWORD 31
    union
    {
        struct
        {
            DWORD       PWLInvGammaSlope10              : 12;
            DWORD                                       : 4;
            DWORD       PWLInvGammaSlope11              : 12;
            DWORD                                       : 4;
        };
        struct
        {
            DWORD       Value;
        };
    } DW31;

    // Offset value for R, G, B for the Transform
    // DWORD 32
    union
    {
        struct
        {
            DWORD       OffsetInR                       : 15; 
            DWORD                                       : 1;
            DWORD       OffsetInG                       : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW32;

    // DWORD 33
    union
    {
        struct
        {
            DWORD       OffsetInB                       : 15; 
            DWORD                                       : 1;
            DWORD       OffsetOutB                      : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW33;

    // DWORD 34
    union
    {
        struct
        {
            DWORD       OffsetOutR                      : 15; 
            DWORD                                       : 1;
            DWORD       OffsetOutG                      : 15;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW34;

    // GCC State
    // DWORD 35
    union
    {
        struct
        {
            DWORD       OuterTriangleMappingLengthBelow : 10;  // U10
            DWORD       OuterTriangleMappingLength      : 10;  // U10
            DWORD       InnerTriangleMappingLength      : 10;  // U10
            DWORD       FullRangeMappingEnable          : 1;
            DWORD                                       : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW35;

    // DWORD 36
    union
    {
        struct
        {
            DWORD       InnerTriangleMappingLengthBelow : 10;   // U10
            DWORD                                       : 18;
            DWORD       CompressionLineShift            : 3;
            DWORD       xvYccDecEncEnable               : 1;
        };
        struct
        {
            DWORD       Value;
        };
    } DW36;

    // DWORD 37
    union
    {
        struct
        {
            DWORD       CpiOverride                     : 1;
            DWORD                                       : 10;
            DWORD       BasicModeScalingFactor          : 14;
            DWORD                                       : 4;
            DWORD       LumaChromaOnlyCorrection        : 1;
            DWORD       GCCBasicModeSelection           : 2;
        };
        struct
        {
            DWORD       Value;
        };
    } DW37;

    // Padding for 32-byte alignment, VEBOX_GAMUT_STATE_G75 is 38 DWORDs
    DWORD dwPad[2];
} VEBOX_GAMUT_STATE_G75, *PVEBOX_GAMUT_STATE_G75;

#define NUM_VERTEX_TABLE_ENTRIES_G75        512
// Defined in vol2b "Media"
typedef struct _VEBOX_VERTEX_TABLE_ENTRY_G75
{
    union
    {
        struct
        {
            DWORD       VertexTableEntryCv                  : 12;
            DWORD                                           : 4;
            DWORD       VertexTableEntryLv                  : 12;
            DWORD                                           : 4;
        };
        struct
        {
            DWORD       Value;
        };
    };
} VEBOX_VERTEX_TABLE_ENTRY_G75, *PVEBOX_VERTEX_TABLE_ENTRY_G75;

// Defined in vol2b "Media"
typedef struct _VEBOX_VERTEX_TABLE_G75
{
    VEBOX_VERTEX_TABLE_ENTRY_G75 VertexTableEntry[NUM_VERTEX_TABLE_ENTRIES_G75];
} VEBOX_VERTEX_TABLE_G75, *PVEBOX_VERTEX_TABLE_G75;

typedef struct _CM_VEBOX_STATE_G75
{
    union
    {
        struct
        {
            DWORD       ColorGamutExpansionEnable           : 1;    // 0
            DWORD       ColorGamutCompressionEnable         : 1;    // 1
            DWORD       GlobalIECPEnable                    : 1;    // 2
            DWORD       DNEnable                            : 1;    // 3
            DWORD       DIEnable                            : 1;    // 4
            DWORD       DNDIFirstFrame                      : 1;    // 5
            DWORD       DownsampleMethod422to420            : 1;    // 6
            DWORD       DownsampleMethod444to422            : 1;    // 7
            DWORD       DIOutputFrames                      : 2;    // 8:9
            DWORD       PipeSynchronizeDisable              : 1;    // 10
            DWORD                                           : 15;   // 25:11 Reserved
            DWORD       StateSurfaceControlBits             : 6;    // 31:26
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    VEBOX_DNDI_STATE_G75* pDndiState;
    VEBOX_IECP_STATE_G75* pIecpState;
    VEBOX_GAMUT_STATE_G75* pGamutState;
    VEBOX_VERTEX_TABLE_G75* pVertexTable;
}  CM_VEBOX_STATE_G75 , *PCM_VEBOX_STATE_G75;

typedef struct _VEBOX_DNDI_STATE_G8
{
    // DWORD 0
    union
    {
        struct
        {
            DWORD       DenoiseASDThreshold : 8; // U8
            DWORD       DnmhDelta : 4; // UINT4
        DWORD: 4; // Reserved
            DWORD       DnmhHistoryMax : 8; // U8
            DWORD       DenoiseSTADThreshold : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       SCMDenoiseThreshold : 8; // U8
            DWORD       DenoiseMovingPixelThreshold : 5; // U5
            DWORD       STMMC2 : 3; // U3
            DWORD       LowTemporalDifferenceThreshold : 6; // U6
        DWORD: 2; // Reserved
            DWORD       TemporalDifferenceThreshold : 6; // U6
        DWORD: 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       BlockNoiseEstimateNoiseThreshold : 8; // U8
            DWORD       BneEdgeTh : 4; // UINT4
        DWORD: 2; // Reserved
            DWORD       SmoothMvTh : 2; // U2
            DWORD       SADTightTh : 4; // U4
            DWORD       CATSlopeMinus1 : 4; // U4
            DWORD       GoodNeighborThreshold : 6; // UINT6
        DWORD: 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            DWORD       MaximumSTMM : 8; // U8
            DWORD       MultiplierforVECM : 6; // U6
        DWORD: 2;
            DWORD       BlendingConstantForSmallSTMM : 8; // U8
            DWORD       BlendingConstantForLargeSTMM : 7; // U7
            DWORD       STMMBlendingConstantSelect : 1; // U1
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       SDIDelta : 8; // U8
            DWORD       SDIThreshold : 8; // U8
            DWORD       STMMOutputShift : 4; // U4
            DWORD       STMMShiftUp : 2; // U2
            DWORD       STMMShiftDown : 2; // U2
            DWORD       MinimumSTMM : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       FMDTemporalDifferenceThreshold : 8; // U8
            DWORD       SDIFallbackMode2Constant : 8; // U8
            DWORD       SDIFallbackMode1T2Constant : 8; // U8
            DWORD       SDIFallbackMode1T1Constant : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
        DWORD: 3; // Reserved
            DWORD       DNDITopFirst : 1; // Enable
        DWORD: 2; // Reserved
            DWORD       ProgressiveDN : 1; // Enable
            DWORD       MCDIEnable : 1;
            DWORD       FMDTearThreshold : 6; // U6
            DWORD       CATTh1 : 2; // U2
            DWORD       FMD2VerticalDifferenceThreshold : 8; // U8
            DWORD       FMD1VerticalDifferenceThreshold : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD       SADTHA : 4; // U4
            DWORD       SADTHB : 4; // U4
            DWORD       FMDFirstFieldCurrentFrame : 2; // U2
            DWORD       MCPixelConsistencyTh : 6; // U6
            DWORD       FMDSecondFieldPreviousFrame : 2; // U2
        DWORD: 1; // Reserved
            DWORD       NeighborPixelTh : 4; // U4
            DWORD       DnmhHistoryInit : 6; // U6
        DWORD: 3; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            DWORD       ChromaLTDThreshold : 6; // U6
            DWORD       ChromaTDTheshold : 6; // U6
            DWORD       ChromaDenoiseEnable : 1; // Enable
        DWORD: 3; // Reserved
            DWORD       ChromaDnSTADThreshold : 8; // U8
        DWORD: 8; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       HotPixelThreshold : 8;
            DWORD       HotPixelCount : 4;
        DWORD: 20; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // Padding for 32-byte alignment, VEBOX_DNDI_STATE_G8 is 10 DWORDs
    DWORD dwPad[6];
} VEBOX_DNDI_STATE_G8, *PVEBOX_DNDI_STATE_G8;

// Defined in vol2b "Media"
typedef struct _VEBOX_IECP_STATE_G8
{
    // STD/STE state
    // DWORD 0
    union
    {
        struct
        {
            DWORD       STDEnable : BITFIELD_BIT(0);
            DWORD       STEEnable : BITFIELD_BIT(1);
            DWORD       OutputCtrl : BITFIELD_BIT(2);
        DWORD: BITFIELD_BIT(3);
            DWORD       SatMax : BITFIELD_RANGE(4, 9);      // U6;
            DWORD       HueMax : BITFIELD_RANGE(10, 15);    // U6;
            DWORD       UMid : BITFIELD_RANGE(16, 23);    // U8;
            DWORD       VMid : BITFIELD_RANGE(24, 31);    // U8;
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       SinAlpha : BITFIELD_RANGE(0, 7);    // S0.7
        DWORD: BITFIELD_RANGE(8, 9);
            DWORD       CosAlpha : BITFIELD_RANGE(10, 17);  // S0.7
            DWORD       HSMargin : BITFIELD_RANGE(18, 20);  // U3   
            DWORD       DiamondDu : BITFIELD_RANGE(21, 27);  // S7    
            DWORD       DiamondMargin : BITFIELD_RANGE(28, 30);  // U3    
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            DWORD       DiamondDv : BITFIELD_RANGE(0, 6);    // S8.0
            DWORD       DiamondTh : BITFIELD_RANGE(7, 12);   // U6  
            DWORD       DiamondAlpha : BITFIELD_RANGE(13, 20);  // U1.6
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 6);
            DWORD       VYSTDEnable : BITFIELD_BIT(7);
            DWORD       YPoint1 : BITFIELD_RANGE(8, 15);    // U8   
            DWORD       YPoint2 : BITFIELD_RANGE(16, 23);   // U8   
            DWORD       YPoint3 : BITFIELD_RANGE(24, 31);   // U8   
        };
        struct
        {
            DWORD       Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            DWORD       YPoint4 : BITFIELD_RANGE(0, 7);    // U8
            DWORD       YSlope1 : BITFIELD_RANGE(8, 12);   // U2.3
            DWORD       YSlope2 : BITFIELD_RANGE(13, 17);  // U2.3
        DWORD: BITFIELD_RANGE(18, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            DWORD       INVMarginVYL : BITFIELD_RANGE(0, 15);    // U0.16
            DWORD       INVSkinTypesMargin : BITFIELD_RANGE(16, 31);   // U0.16
        };
        struct
        {
            DWORD       Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            DWORD       INVMarginVYU : BITFIELD_RANGE(0, 15);    // U0.16
            DWORD       P0L : BITFIELD_RANGE(16, 23);   // U8
            DWORD       P1L : BITFIELD_RANGE(24, 31);   // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            DWORD       P2L : BITFIELD_RANGE(0, 7);     // U8
            DWORD       P3L : BITFIELD_RANGE(8, 15);    // U8
            DWORD       B0L : BITFIELD_RANGE(16, 23);   // U8
            DWORD       B1L : BITFIELD_RANGE(24, 31);   // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            DWORD       B2L : BITFIELD_RANGE(0, 7);     // U8
            DWORD       B3L : BITFIELD_RANGE(8, 15);    // U8
            DWORD       S0L : BITFIELD_RANGE(16, 26);   // S2.8
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            DWORD       S1L : BITFIELD_RANGE(0, 10);    // S2.8
            DWORD       S2L : BITFIELD_RANGE(11, 21);   // S2.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW9;

    // DWORD 10
    union
    {
        struct
        {
            DWORD       S3L : BITFIELD_RANGE(0, 10);    // S2.8
            DWORD       P0U : BITFIELD_RANGE(11, 18);   // U8
            DWORD       P1U : BITFIELD_RANGE(19, 26);   // U8
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW10;

    // DWORD 11
    union
    {
        struct
        {
            DWORD       P2U : BITFIELD_RANGE(0, 7);     // U8
            DWORD       P3U : BITFIELD_RANGE(8, 15);    // U8
            DWORD       B0U : BITFIELD_RANGE(16, 23);   // U8
            DWORD       B1U : BITFIELD_RANGE(24, 31);   // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW11;

    // DWORD 12
    union
    {
        struct
        {
            DWORD       B2U : BITFIELD_RANGE(0, 7);     // U8
            DWORD       B3U : BITFIELD_RANGE(8, 15);    // U8
            DWORD       S0U : BITFIELD_RANGE(16, 26);   // S2.8
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            DWORD       S1U : BITFIELD_RANGE(0, 10);     // S2.8
            DWORD       S2U : BITFIELD_RANGE(11, 21);    // S2.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            DWORD       S3U : BITFIELD_RANGE(0, 10);     // S2.8
            DWORD       SkinTypesEnable : BITFIELD_BIT(11);
            DWORD       SkinTypesThresh : BITFIELD_RANGE(12, 19);    // U8
            DWORD       SkinTypesMargin : BITFIELD_RANGE(20, 27);    // U8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            DWORD       SATP1 : BITFIELD_RANGE(0, 6);     // S6
            DWORD       SATP2 : BITFIELD_RANGE(7, 13);    // S6
            DWORD       SATP3 : BITFIELD_RANGE(14, 20);   // S6
            DWORD       SATB1 : BITFIELD_RANGE(21, 30);   // S2.7
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW15;

    // DWORD 16
    union
    {
        struct
        {
            DWORD       SATB2 : BITFIELD_RANGE(0, 9);     // S2.7
            DWORD       SATB3 : BITFIELD_RANGE(10, 19);   // S2.7
            DWORD       SATS0 : BITFIELD_RANGE(20, 30);   // U3.8
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW16;

    // DWORD 17
    union
    {
        struct
        {
            DWORD       SATS1 : BITFIELD_RANGE(0, 10);     // U3.8
            DWORD       SATS2 : BITFIELD_RANGE(11, 21);    // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW17;

    // DWORD 18
    union
    {
        struct
        {
            DWORD       SATS3 : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUEP1 : BITFIELD_RANGE(11, 17);   // U3.8
            DWORD       HUEP2 : BITFIELD_RANGE(18, 24);   // U3.8
            DWORD       HUEP3 : BITFIELD_RANGE(25, 31);   // U3.8
        };
        struct
        {
            DWORD       Value;
        };
    } DW18;

    // DWORD 19
    union
    {
        struct
        {
            DWORD       HUEB1 : BITFIELD_RANGE(0, 9);    // S2.7
            DWORD       HUEB2 : BITFIELD_RANGE(10, 19);  // S2.7
            DWORD       HUEB3 : BITFIELD_RANGE(20, 29);  // S2.7
        DWORD: BITFIELD_RANGE(30, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW19;

    // DWORD 20
    union
    {
        struct
        {
            DWORD       HUES0 : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUES1 : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW20;

    // DWORD 21
    union
    {
        struct
        {
            DWORD       HUES2 : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUES3 : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW21;

    // DWORD 22
    union
    {
        struct
        {
            DWORD       SATP1DARK : BITFIELD_RANGE(0, 6);     // S6
            DWORD       SATP2DARK : BITFIELD_RANGE(7, 13);    // S6
            DWORD       SATP3DARK : BITFIELD_RANGE(14, 20);   // S6
            DWORD       SATB1DARK : BITFIELD_RANGE(21, 30);   // S2.7
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW22;

    // DWORD 23
    union
    {
        struct
        {
            DWORD       SATB2DARK : BITFIELD_RANGE(0, 9);    // S2.7
            DWORD       SATB3DARK : BITFIELD_RANGE(10, 19);  // S2.7
            DWORD       SATS0DARK : BITFIELD_RANGE(20, 30);  // U3.8
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW23;

    // DWORD 24
    union
    {
        struct
        {
            DWORD       SATS1DARK : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       SATS2DARK : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW24;

    // DWORD 25
    union
    {
        struct
        {
            DWORD       SATS3DARK : BITFIELD_RANGE(0, 10);   // U3.8
            DWORD       HUEP1DARK : BITFIELD_RANGE(11, 17);  // S6
            DWORD       HUEP2DARK : BITFIELD_RANGE(18, 24);  // S6
            DWORD       HUEP3DARK : BITFIELD_RANGE(25, 31);  // S6
        };
        struct
        {
            DWORD       Value;
        };
    } DW25;

    // DWORD 26
    union
    {
        struct
        {
            DWORD       HUEB1DARK : BITFIELD_RANGE(0, 9);    // S2.7
            DWORD       HUEB2DARK : BITFIELD_RANGE(10, 19);  // S2.7
            DWORD       HUEB3DARK : BITFIELD_RANGE(20, 29);  // S2.7
        DWORD: BITFIELD_RANGE(30, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW26;

    // DWORD 27
    union
    {
        struct
        {
            DWORD       HUES0DARK : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUES1DARK : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW27;

    // DWORD 28
    union
    {
        struct
        {
            DWORD       HUES2DARK : BITFIELD_RANGE(0, 10);    // U3.8
            DWORD       HUES3DARK : BITFIELD_RANGE(11, 21);   // U3.8
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW28;

    // ACE state
    // DWORD 29
    union
    {
        struct
        {
            DWORD       ACEEnable : BITFIELD_BIT(0);
            DWORD       FullImageHistogram : BITFIELD_BIT(1);
            DWORD       SkinThreshold : BITFIELD_RANGE(2, 6);    // U5
        DWORD: BITFIELD_RANGE(7, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW29;

    // DWORD 30
    union
    {
        struct
        {
            DWORD       Ymin : BITFIELD_RANGE(0, 7);
            DWORD       Y1 : BITFIELD_RANGE(8, 15);
            DWORD       Y2 : BITFIELD_RANGE(16, 23);
            DWORD       Y3 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW30;

    // DWORD 31
    union
    {
        struct
        {
            DWORD       Y4 : BITFIELD_RANGE(0, 7);
            DWORD       Y5 : BITFIELD_RANGE(8, 15);
            DWORD       Y6 : BITFIELD_RANGE(16, 23);
            DWORD       Y7 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW31;

    // DWORD 32
    union
    {
        struct
        {
            DWORD       Y8 : BITFIELD_RANGE(0, 7);
            DWORD       Y9 : BITFIELD_RANGE(8, 15);
            DWORD       Y10 : BITFIELD_RANGE(16, 23);
            DWORD       Ymax : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW32;

    // DWORD 33
    union
    {
        struct
        {
            DWORD       B1 : BITFIELD_RANGE(0, 7);   // U8
            DWORD       B2 : BITFIELD_RANGE(8, 15);  // U8
            DWORD       B3 : BITFIELD_RANGE(16, 23); // U8
            DWORD       B4 : BITFIELD_RANGE(24, 31); // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW33;

    // DWORD 34
    union
    {
        struct
        {
            DWORD       B5 : BITFIELD_RANGE(0, 7);   // U8
            DWORD       B6 : BITFIELD_RANGE(8, 15);  // U8
            DWORD       B7 : BITFIELD_RANGE(16, 23); // U8
            DWORD       B8 : BITFIELD_RANGE(24, 31); // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW34;

    // DWORD 35
    union
    {
        struct
        {
            DWORD       B9 : BITFIELD_RANGE(0, 7);   // U8
            DWORD       B10 : BITFIELD_RANGE(8, 15);  // U8
        DWORD: BITFIELD_RANGE(16, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW35;

    // DWORD 36
    union
    {
        struct
        {
            DWORD       S0 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S1 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW36;

    // DWORD 37
    union
    {
        struct
        {
            DWORD       S2 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S3 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW37;

    // DWORD 38
    union
    {
        struct
        {
            DWORD       S4 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S5 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW38;

    // DWORD 39
    union
    {
        struct
        {
            DWORD       S6 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S7 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW39;

    // DWORD 40
    union
    {
        struct
        {
            DWORD       S8 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 15);
            DWORD       S9 : BITFIELD_RANGE(16, 26);  // U11
        DWORD: BITFIELD_RANGE(27, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW40;

    // DWORD 41
    union
    {
        struct
        {
            DWORD       S10 : BITFIELD_RANGE(0, 10);   // U11
        DWORD: BITFIELD_RANGE(11, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW41;

    // TCC State
    // DWORD 42
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 6);
            DWORD       TCCEnable : BITFIELD_BIT(7);
            DWORD       SatFactor1 : BITFIELD_RANGE(8, 15);    // U8 
            DWORD       SatFactor2 : BITFIELD_RANGE(16, 23);   // U8 
            DWORD       SatFactor3 : BITFIELD_RANGE(24, 31);   // U8 
        };
        struct
        {
            DWORD       Value;
        };
    } DW42;

    // DWORD 43
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 7);
            DWORD       SatFactor4 : BITFIELD_RANGE(8, 15);    // U8 
            DWORD       SatFactor5 : BITFIELD_RANGE(16, 23);   // U8 
            DWORD       SatFactor6 : BITFIELD_RANGE(24, 31);   // U8 
        };
        struct
        {
            DWORD       Value;
        };
    } DW43;

    // DWORD 44
    union
    {
        struct
        {
            DWORD       BaseColor1 : BITFIELD_RANGE(0, 9);    // U10 
            DWORD       BaseColor2 : BITFIELD_RANGE(10, 19);  // U10 
            DWORD       BaseColor3 : BITFIELD_RANGE(20, 29);  // U10 
        DWORD: BITFIELD_RANGE(30, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW44;

    // DWORD 45
    union
    {
        struct
        {
            DWORD       BaseColor4 : BITFIELD_RANGE(0, 9);    // U10 
            DWORD       BaseColor5 : BITFIELD_RANGE(10, 19);  // U10 
            DWORD       BaseColor6 : BITFIELD_RANGE(20, 29);  // U10 
        DWORD: BITFIELD_RANGE(30, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW45;

    // DWORD 46
    union
    {
        struct
        {
            DWORD       ColorTransitSlope12 : BITFIELD_RANGE(0, 15);    // U16
            DWORD       ColorTransitSlope23 : BITFIELD_RANGE(16, 31);   // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW46;

    // DWORD 47
    union
    {
        struct
        {
            DWORD       ColorTransitSlope34 : BITFIELD_RANGE(0, 15);    // U16
            DWORD       ColorTransitSlope45 : BITFIELD_RANGE(16, 31);   // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW47;

    // DWORD 48
    union
    {
        struct
        {
            DWORD       ColorTransitSlope56 : BITFIELD_RANGE(0, 15);    // U16
            DWORD       ColorTransitSlope61 : BITFIELD_RANGE(16, 31);   // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW48;

    // DWORD 49
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 1);
            DWORD       ColorBias1 : BITFIELD_RANGE(2, 11);    // U10
            DWORD       ColorBias2 : BITFIELD_RANGE(12, 21);   // U10
            DWORD       ColorBias3 : BITFIELD_RANGE(22, 31);   // U10
        };
        struct
        {
            DWORD       Value;
        };
    } DW49;

    // DWORD 50
    union
    {
        struct
        {
        DWORD: BITFIELD_RANGE(0, 1);
            DWORD       ColorBias4 : BITFIELD_RANGE(2, 11);    // U10
            DWORD       ColorBias5 : BITFIELD_RANGE(12, 21);   // U10
            DWORD       ColorBias6 : BITFIELD_RANGE(22, 31);   // U10
        };
        struct
        {
            DWORD       Value;
        };
    } DW50;

    // DWORD 51
    union
    {
        struct
        {
            DWORD       STESlopeBits : BITFIELD_RANGE(0, 2);    // U3
        DWORD: BITFIELD_RANGE(3, 7);
            DWORD       STEThreshold : BITFIELD_RANGE(8, 12);   // U5
        DWORD: BITFIELD_RANGE(13, 15);
            DWORD       UVThresholdBits : BITFIELD_RANGE(16, 18);  // U5
        DWORD: BITFIELD_RANGE(19, 23);
            DWORD       UVThreshold : BITFIELD_RANGE(24, 30);  // U7
        DWORD: BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW51;

    // DWORD 52
    union
    {
        struct
        {
            DWORD       UVMaxColor : BITFIELD_RANGE(0, 8);    // U9
        DWORD: BITFIELD_RANGE(9, 15);
            DWORD       InvUVMaxColor : BITFIELD_RANGE(16, 31);  // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW52;

    // ProcAmp State
    // DWORD 53
    union
    {
        struct
        {
            DWORD       ProcAmpEnable : BITFIELD_BIT(0);
            DWORD       Brightness : BITFIELD_RANGE(1, 12);    // S7.4
        DWORD: BITFIELD_RANGE(13, 16);
            DWORD       Contrast : BITFIELD_RANGE(17, 27);   // U7.4
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW53;

    // DWORD 54
    union
    {
        struct
        {
            DWORD       SINCS : BITFIELD_RANGE(0, 15);    // S7.8    
            DWORD       COSCS : BITFIELD_RANGE(16, 31);   // S7.8
        };
        struct
        {
            DWORD       Value;
        };
    } DW54;

    // CSC State
    // DWORD 55
    union
    {
        struct
        {
            DWORD       TransformEnable : BITFIELD_BIT(0);
            DWORD       YUVChannelSwap : BITFIELD_BIT(1);
        DWORD: BITFIELD_BIT(2);
            DWORD       C0 : BITFIELD_RANGE(3, 15);  // S2.10
            DWORD       C1 : BITFIELD_RANGE(16, 28); // S2.10
        DWORD: BITFIELD_RANGE(29, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW55;

    // DWORD 56
    union
    {
        struct
        {
            DWORD       C2 : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       C3 : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW56;

    // DWORD 57
    union
    {
        struct
        {
            DWORD       C4 : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       C5 : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW57;

    // DWORD 58
    union
    {
        struct
        {
            DWORD       C6 : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       C7 : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW58;

    // DWORD 59
    union
    {
        struct
        {
            DWORD       C8 : BITFIELD_RANGE(0, 12);   // S2.10
        DWORD: BITFIELD_RANGE(13, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW59;

    // DWORD 60
    union
    {
        struct
        {
            DWORD       OffsetIn1 : BITFIELD_RANGE(0, 10);   // S10
            DWORD       OffsetOut1 : BITFIELD_RANGE(11, 21);  // S10
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW60;

    // DWORD 61
    union
    {
        struct
        {
            DWORD       OffsetIn2 : BITFIELD_RANGE(0, 10);   // S10
            DWORD       OffsetOut2 : BITFIELD_RANGE(11, 21);  // S10
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW61;

    // DWORD 62
    union
    {
        struct
        {
            DWORD       OffsetIn3 : BITFIELD_RANGE(0, 10);  // S10
            DWORD       OffsetOut3 : BITFIELD_RANGE(11, 21); // S10
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW62;

    // DWORD 63
    union
    {
        struct
        {
            DWORD       ColorPipeAlpha : BITFIELD_RANGE(0, 11);  // U12
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       AlphaFromStateSelect : BITFIELD_BIT(16);
        DWORD: BITFIELD_RANGE(17, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW63;

    // Area of Interest
    // DWORD 64
    union
    {
        struct
        {
            DWORD       AOIMinX : BITFIELD_RANGE(0, 15);  // U16
            DWORD       AOIMaxX : BITFIELD_RANGE(16, 31); // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW64;

    // DWORD 65
    union
    {
        struct
        {
            DWORD       AOIMinY : BITFIELD_RANGE(0, 15);  // U16
            DWORD       AOIMaxY : BITFIELD_RANGE(16, 31); // U16
        };
        struct
        {
            DWORD       Value;
        };
    } DW65;

    // Color Correction Matrix
    // DWORD 66
    union
    {
        struct
        {
            DWORD       C1Coeff : BITFIELD_RANGE(0, 20);  // S8.12
        DWORD: BITFIELD_RANGE(21, 29);
            DWORD       VignetteCorrectionFormat : BITFIELD_BIT(30);
            DWORD       ColorCorrectionMatrixEnable : BITFIELD_BIT(31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW66;

    // DWORD 67
    union
    {
        struct
        {
            DWORD       C0Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW67;

    // DWORD 68
    union
    {
        struct
        {
            DWORD       C3Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW68;

    // DWORD 69
    union
    {
        struct
        {
            DWORD       C2Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW69;

    // DWORD 70
    union
    {
        struct
        {
            DWORD       C5Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW70;

    // DWORD 71
    union
    {
        struct
        {
            DWORD       C4Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW71;

    // DWORD 72
    union
    {
        struct
        {
            DWORD       C7Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW72;

    // DWORD 73
    union
    {
        struct
        {
            DWORD       C6Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW73;

    // DWORD 74
    union
    {
        struct
        {
            DWORD       C8Coeff : BITFIELD_RANGE(0, 20); // S8.12
        DWORD: BITFIELD_RANGE(21, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW74;

    // DWORD 75
    union
    {
        struct
        {
            DWORD       BlackPointOffsetR : BITFIELD_RANGE(0, 12); // S12
        DWORD: BITFIELD_RANGE(13, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW75;

    // DWORD 76
    union
    {
        struct
        {
            DWORD       BlackPointOffsetB : BITFIELD_RANGE(0, 12);  // S12
            DWORD       BlackPointOffsetG : BITFIELD_RANGE(13, 25); // S12
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW76;

    // Forward Gamma Correction
    // DWORD 77
    union
    {
        struct
        {
            DWORD       ForwardGammaCorrectionEnable : BITFIELD_BIT(0);
        DWORD: BITFIELD_RANGE(1, 7);
            DWORD       PWLFwdGammaPoint1 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaPoint2 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaPoint3 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW77;

    // DWORD 78
    union
    {
        struct
        {
            DWORD       PWLFwdGammaPoint4 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaPoint5 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaPoint6 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaPoint7 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW78;

    // DWORD 79
    union
    {
        struct
        {
            DWORD       PWLFwdGammaPoint8 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaPoint9 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaPoint10 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaPoint11 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW79;

    // DWORD 80
    union
    {
        struct
        {
            DWORD       PWLFwdGammaBias1 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaBias2 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaBias3 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaBias4 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW80;

    // DWORD 81
    union
    {
        struct
        {
            DWORD       PWLFwdGammaBias5 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaBias6 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaBias7 : BITFIELD_RANGE(16, 23);
            DWORD       PWLFwdGammaBias8 : BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW81;

    // DWORD 82
    union
    {
        struct
        {
            DWORD       PWLFwdGammaBias9 : BITFIELD_RANGE(0, 7);
            DWORD       PWLFwdGammaBias10 : BITFIELD_RANGE(8, 15);
            DWORD       PWLFwdGammaBias11 : BITFIELD_RANGE(16, 23);
        DWORD: BITFIELD_RANGE(24, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW82;

    // DWORD 83
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope0 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope1 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW83;

    // DWORD 84
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope2 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope3 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW84;

    // DWORD 85
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope4 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope5 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW85;

    // DWORD 86
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope6 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope7 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW86;

    // DWORD 87
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope8 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope9 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW87;

    // DWORD 88
    union
    {
        struct
        {
            DWORD       PWLFwdGammaSlope10 : BITFIELD_RANGE(0, 11);  // U4.8
        DWORD: BITFIELD_RANGE(12, 15);
            DWORD       PWLFwdGammaSlope11 : BITFIELD_RANGE(16, 27); // U4.8
        DWORD: BITFIELD_RANGE(28, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW88;

    // Front-End CSC
    // DWORD 89
    union
    {
        struct
        {
            DWORD       FrontEndCSCTransformEnable : BITFIELD_BIT(0);
        DWORD: BITFIELD_RANGE(1, 2);
            DWORD       FECSCC0Coeff : BITFIELD_RANGE(3, 15);  // S2.10
            DWORD       FECSCC1Coeff : BITFIELD_RANGE(16, 28); // S2.10
        DWORD: BITFIELD_RANGE(29, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW89;

    // DWORD 90
    union
    {
        struct
        {
            DWORD       FECSCC2Coeff : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       FECSCC3Coeff : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW90;

    // DWORD 91
    union
    {
        struct
        {
            DWORD       FECSCC4Coeff : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       FECSCC5Coeff : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW91;

    // DWORD 92
    union
    {
        struct
        {
            DWORD       FECSCC6Coeff : BITFIELD_RANGE(0, 12);  // S2.10
            DWORD       FECSCC7Coeff : BITFIELD_RANGE(13, 25); // S2.10
        DWORD: BITFIELD_RANGE(26, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW92;

    // DWORD 93
    union
    {
        struct
        {
            DWORD       FECSCC8Coeff : BITFIELD_RANGE(0, 12);  // S2.10
        DWORD: BITFIELD_RANGE(13, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW93;

    // DWORD 94
    union
    {
        struct
        {
            DWORD       FECSCCOffsetIn1 : BITFIELD_RANGE(0, 10);
            DWORD       FECSCCOffsetOut1 : BITFIELD_RANGE(11, 21);
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW94;

    // DWORD 95
    union
    {
        struct
        {
            DWORD       FECSCCOffsetIn2 : BITFIELD_RANGE(0, 10);
            DWORD       FECSCCOffsetOut2 : BITFIELD_RANGE(11, 21);
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW95;

    // DWORD 96
    union
    {
        struct
        {
            DWORD       FECSCCOffsetIn3 : BITFIELD_RANGE(0, 10);
            DWORD       FECSCCOffsetOut3 : BITFIELD_RANGE(11, 21);
        DWORD: BITFIELD_RANGE(22, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW96;

    // Padding for 32-byte alignment, VEBOX_IECP_STATE_G8 is 97 DWORDs
    DWORD dwPad[7];
} VEBOX_IECP_STATE_G8, *PVEBOX_IECP_STATE_G8;

typedef struct _VEBOX_CAPTURE_PIPE_STATE_G8
{
    // DWORD 0
    union
    {
        struct
        {
            DWORD       BadAvgMinCostTh : 8; // U8
            DWORD       THColorTh : 8; // U8
            DWORD       ScaleForAvgMinCost : 4; // U4
            DWORD       ShiftMinCost : 3; // U3
        DWORD: 1; // Reserved
            DWORD       GoodPixelTh : 6; // U6
        DWORD: 2; // Reserved
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            DWORD       BadTH3 : 4; // U4
        DWORD: 4; // Reserved
            DWORD       BadTH2 : 8; // U8
            DWORD       BadTH1 : 8; // U8
        DWORD: 4; // Reserved
            DWORD       ScaleForMinCost : 4; // U4
        };
        struct
        {
            DWORD       Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
        DWORD: 8; // Reserved
            DWORD       UVThresholdValue : 8; // U8
            DWORD       YOutlierValue : 8; // U8
            DWORD       YBrightValue : 8; // U8
        };
        struct
        {
            DWORD       Value;
        };
    } DW2;

    // Padding for 32-byte alignment, VEBOX_CAPTURE_PIPE_STATE_G8 is 3 DWORDs
    DWORD dwPad[5];
} VEBOX_CAPTURE_PIPE_STATE_G8, *PVEBOX_CAPTURE_PIPE_STATE_G8;


typedef struct _CM_VEBOX_STATE_G8
{
    union
    {
        struct
        {
            DWORD       ColorGamutExpansionEnable : 1;
            DWORD       ColorGamutCompressionEnable : 1;
            DWORD       GlobalIECPEnable : 1;
            DWORD       DNEnable : 1;
            DWORD       DIEnable : 1;
            DWORD       DNDIFirstFrame : 1;
            DWORD       DownsampleMethod422to420 : 1;
            DWORD       DownsampleMethod444to422 : 1;
            DWORD       DIOutputFrames : 2;
            DWORD       DemosaicEnable : 1;
            DWORD       VignetteEnable : 1;
            DWORD       AlphaPlaneEnable : 1;
            DWORD       HotPixelFilteringEnable : 1;
            DWORD       SingleSliceVeboxEnable : 1;
        DWORD: 11; // Reserved
            DWORD       StateSurfaceControlBits : 6;
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    VEBOX_DNDI_STATE_G8*         pDndiState;
    VEBOX_IECP_STATE_G8*         pIecpState;
    VEBOX_GAMUT_STATE_G75*       pGamutState;
    VEBOX_VERTEX_TABLE_G75*      pVertexTable;
    VEBOX_CAPTURE_PIPE_STATE_G8* pCapturePipe;
}  CM_VEBOX_STATE_G8, *PCM_VEBOX_STATE_G8;

typedef struct _VEBOX_SURFACE_CONTROL_BITS_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       EncryptedData : BITFIELD_BIT(0);
            DWORD       IndexToMemoryObjectControlStateMocsTables : BITFIELD_RANGE(1, 6);
            DWORD       MemoryCompressionEnable : BITFIELD_BIT(7);
            DWORD       MemoryCompressionMode : BITFIELD_BIT(8);
            DWORD       TiledResourceMode : BITFIELD_RANGE(9, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 31);
        };
        DWORD Value;
    } DW0;
} VEBOX_SURFACE_CONTROL_BITS_G9, *PVEBOX_SURFACE_CONTROL_BITS_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_DNDI_STATE_G9
{
    // Temporal DN
    // DWORD 0
    union {
        struct {
            DWORD       DenoiseMovingPixelThreshold : BITFIELD_RANGE(0, 4);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(5, 7);
            DWORD       DenoiseHistoryIncrease : BITFIELD_RANGE(8, 11);
            DWORD       DenoiseMaximumHistory : BITFIELD_RANGE(12, 19);
            DWORD       DenoiseStadThreshold : BITFIELD_RANGE(20, 31);
        };
        DWORD       Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       LowTemporalDifferenceThreshold : BITFIELD_RANGE(0, 9);
            DWORD       TemporalDifferenceThreshold : BITFIELD_RANGE(10, 19);
            DWORD       DenoiseAsdThreshold : BITFIELD_RANGE(20, 31);
        };
        DWORD       Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 9);
            DWORD       InitialDenoiseHistory : BITFIELD_RANGE(10, 15);
            DWORD       DenoiseThresholdForSumOfComplexityMeasure : BITFIELD_RANGE(16, 27);
            DWORD       ProgressiveDn : BITFIELD_BIT(28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW2;

    // Global Noise estimate and hot pixel detection
    // DWORD 3
    union {
        struct {
            DWORD       BlockNoiseEstimateNoiseThreshold : BITFIELD_RANGE(0, 11);
            DWORD       BlockNoiseEstimateEdgeThreshold : BITFIELD_RANGE(12, 19);
            DWORD       HotPixelThreshold : BITFIELD_RANGE(20, 27);
            DWORD       HotPixelCount : BITFIELD_RANGE(28, 31);
        };
        DWORD       Value;
    } DW3;

    // Chroma DN
    // DWORD 4
    union {
        struct {
            DWORD       ChromaLowTemporalDifferenceThreshold : BITFIELD_RANGE(0, 5);
            DWORD       ChromaTemporalDifferenceThreshold : BITFIELD_RANGE(6, 11);
            DWORD       ChromaDenoiseEnable : BITFIELD_BIT(12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       ChromaDenoiseStadThreshold : BITFIELD_RANGE(16, 23);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(24, 31);
        };
        DWORD       Value;
    } DW4;

    // 5x5 Spatial Denoise
    // DWORD 5
    union {
        struct {
            DWORD       DnWr0 : BITFIELD_RANGE(0, 4);
            DWORD       DnWr1 : BITFIELD_RANGE(5, 9);
            DWORD       DnWr2 : BITFIELD_RANGE(10, 14);
            DWORD       DnWr3 : BITFIELD_RANGE(15, 19);
            DWORD       DnWr4 : BITFIELD_RANGE(20, 24);
            DWORD       DnWr5 : BITFIELD_RANGE(25, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD       Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       DnThmin : BITFIELD_RANGE(0, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       DnThmax : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       DnDynThmin : BITFIELD_RANGE(0, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       DnPrt5 : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       DnPrt3 : BITFIELD_RANGE(0, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       DnPrt4 : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       DnPrt1 : BITFIELD_RANGE(0, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       DnPrt2 : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       DnWd20 : BITFIELD_RANGE(0, 4);
            DWORD       DnWd21 : BITFIELD_RANGE(5, 9);
            DWORD       DnWd22 : BITFIELD_RANGE(10, 14);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(15);
            DWORD       DnPrt0 : BITFIELD_RANGE(16, 28);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(29, 31);
        };
        DWORD       Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       DnWd00 : BITFIELD_RANGE(0, 4);
            DWORD       DnWd01 : BITFIELD_RANGE(5, 9);
            DWORD       DnWd02 : BITFIELD_RANGE(10, 14);
            DWORD       DnWd10 : BITFIELD_RANGE(15, 19);
            DWORD       DnWd11 : BITFIELD_RANGE(20, 24);
            DWORD       DnWd12 : BITFIELD_RANGE(25, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD       Value;
    } DW11;

    // FMD and DI
    // DWORD 12
    union {
        struct {
            DWORD       SmoothMvThreshold : BITFIELD_RANGE(0, 1);
            DWORD       SadTightThreshold : BITFIELD_RANGE(2, 5);
            DWORD       ContentAdaptiveThresholdSlope : BITFIELD_RANGE(6, 9);
            DWORD       StmmC2 : BITFIELD_RANGE(10, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 31);
        };
        DWORD       Value;
    } DW12;

    // DWORD 13
    union {
        struct {
            DWORD       MaximumStmm : BITFIELD_RANGE(0, 7);
            DWORD       MultiplierForVecm : BITFIELD_RANGE(8, 13);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(14, 15);
            DWORD       BlendingConstantAcrossTimeForSmallValuesOfStmm : BITFIELD_RANGE(16, 23);
            DWORD       BlendingConstantAcrossTimeForLargeValuesOfStmm : BITFIELD_RANGE(24, 30);
            DWORD       StmmBlendingConstantSelect : BITFIELD_BIT(31);
        };
        DWORD       Value;
    } DW13;

    // DWORD 14
    union {
        struct {
            DWORD       SdiDelta : BITFIELD_RANGE(0, 7);
            DWORD       SdiThreshold : BITFIELD_RANGE(8, 15);
            DWORD       StmmOutputShift : BITFIELD_RANGE(16, 19);
            DWORD       StmmShiftUp : BITFIELD_RANGE(20, 21);
            DWORD       StmmShiftDown : BITFIELD_RANGE(22, 23);
            DWORD       MinimumStmm : BITFIELD_RANGE(24, 31);
        };
        DWORD       Value;
    } DW14;

    // DWORD 15
    union {
        struct {
            DWORD       FmdTemporalDifferenceThreshold : BITFIELD_RANGE(0, 7);
            DWORD       SdiFallbackMode2ConstantAngle2X1 : BITFIELD_RANGE(8, 15);
            DWORD       SdiFallbackMode1T2Constant : BITFIELD_RANGE(16, 23);
            DWORD       SdiFallbackMode1T1Constant : BITFIELD_RANGE(24, 31);
        };
        DWORD       Value;
    } DW15;

    // DWORD 16
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 2);
            DWORD       DnDiTopFirst : BITFIELD_BIT(3);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(4, 6);
            DWORD       McdiEnable : BITFIELD_BIT(7);
            DWORD       FmdTearThreshold : BITFIELD_RANGE(8, 13);
            DWORD       CatThreshold : BITFIELD_RANGE(14, 15);
            DWORD       Fmd2VerticalDifferenceThreshold : BITFIELD_RANGE(16, 23);
            DWORD       Fmd1VerticalDifferenceThreshold : BITFIELD_RANGE(24, 31);
        };
        DWORD       Value;
    } DW16;

    // DWORD 17
    union {
        struct {
            DWORD       SadTha : BITFIELD_RANGE(0, 3);
            DWORD       SadThb : BITFIELD_RANGE(4, 7);
            DWORD       FmdFor1StFieldOfCurrentFrame : BITFIELD_RANGE(8, 9);
            DWORD       McPixelConsistencyThreshold : BITFIELD_RANGE(10, 15);
            DWORD       FmdFor2NdFieldOfPreviousFrame : BITFIELD_RANGE(16, 17);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(18);
            DWORD       NeighborPixelThreshold : BITFIELD_RANGE(19, 22);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(23, 31);
        };
        DWORD       Value;
    } DW17;
} VEBOX_DNDI_STATE_G9, *PVEBOX_DNDI_STATE_G9;

// Defined in vol2c "Vebox"
typedef struct _VEBOX_STD_STE_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       StdEnable : BITFIELD_BIT(0);
            DWORD       SteEnable : BITFIELD_BIT(1);
            DWORD       OutputControl : BITFIELD_BIT(2);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(3);
            DWORD       SatMax : BITFIELD_RANGE(4, 9);
            DWORD       HueMax : BITFIELD_RANGE(10, 15);
            DWORD       UMid : BITFIELD_RANGE(16, 23);
            DWORD       VMid : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       Sin : BITFIELD_RANGE(0, 7);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(8, 9);
            DWORD       Cos : BITFIELD_RANGE(10, 17);
            DWORD       HsMargin : BITFIELD_RANGE(18, 20);
            DWORD       DiamondDu : BITFIELD_RANGE(21, 27);
            DWORD       DiamondMargin : BITFIELD_RANGE(28, 30);
            DWORD       StdScoreOutput : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       DiamondDv : BITFIELD_RANGE(0, 6);
            DWORD       DiamondTh : BITFIELD_RANGE(7, 12);
            DWORD       DiamondAlpha : BITFIELD_RANGE(13, 20);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(21, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 6);
            DWORD       VyStdEnable : BITFIELD_BIT(7);
            DWORD       YPoint1 : BITFIELD_RANGE(8, 15);
            DWORD       YPoint2 : BITFIELD_RANGE(16, 23);
            DWORD       YPoint3 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       YPoint4 : BITFIELD_RANGE(0, 7);
            DWORD       YSlope1 : BITFIELD_RANGE(8, 12);
            DWORD       YSlope2 : BITFIELD_RANGE(13, 17);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(18, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       InvMarginVyl : BITFIELD_RANGE(0, 15);
            DWORD       InvSkinTypesMargin : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       InvMarginVyu : BITFIELD_RANGE(0, 15);
            DWORD       P0L : BITFIELD_RANGE(16, 23);
            DWORD       P1L : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       P2L : BITFIELD_RANGE(0, 7);
            DWORD       P3L : BITFIELD_RANGE(8, 15);
            DWORD       B0L : BITFIELD_RANGE(16, 23);
            DWORD       B1L : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       B2L : BITFIELD_RANGE(0, 7);
            DWORD       B3L : BITFIELD_RANGE(8, 15);
            DWORD       S0L : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       S1L : BITFIELD_RANGE(0, 10);
            DWORD       S2L : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       S3L : BITFIELD_RANGE(0, 10);
            DWORD       P0U : BITFIELD_RANGE(11, 18);
            DWORD       P1U : BITFIELD_RANGE(19, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       P2U : BITFIELD_RANGE(0, 7);
            DWORD       P3U : BITFIELD_RANGE(8, 15);
            DWORD       B0U : BITFIELD_RANGE(16, 23);
            DWORD       B1U : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW11;

    // DWORD 12
    union {
        struct {
            DWORD       B2U : BITFIELD_RANGE(0, 7);
            DWORD       B3U : BITFIELD_RANGE(8, 15);
            DWORD       S0U : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW12;

    // DWORD 13
    union {
        struct {
            DWORD       S1U : BITFIELD_RANGE(0, 10);
            DWORD       S2U : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW13;

    // DWORD 14
    union {
        struct {
            DWORD       S3U : BITFIELD_RANGE(0, 10);
            DWORD       SkinTypesEnable : BITFIELD_BIT(11);
            DWORD       SkinTypesThresh : BITFIELD_RANGE(12, 19);
            DWORD       SkinTypesMargin : BITFIELD_RANGE(20, 27);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(28, 31);
        };
        DWORD           Value;
    } DW14;

    // DWORD 15
    union {
        struct {
            DWORD       Satp1 : BITFIELD_RANGE(0, 6);
            DWORD       Satp2 : BITFIELD_RANGE(7, 13);
            DWORD       Satp3 : BITFIELD_RANGE(14, 20);
            DWORD       Satb1 : BITFIELD_RANGE(21, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW15;

    // DWORD 16
    union {
        struct {
            DWORD       Satb2 : BITFIELD_RANGE(0, 9);
            DWORD       Satb3 : BITFIELD_RANGE(10, 19);
            DWORD       Sats0 : BITFIELD_RANGE(20, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW16;

    // DWORD 17
    union {
        struct {
            DWORD       Sats1 : BITFIELD_RANGE(0, 10);
            DWORD       Sats2 : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW17;

    // DWORD 18
    union {
        struct {
            DWORD       Sats3 : BITFIELD_RANGE(0, 10);
            DWORD       Huep1 : BITFIELD_RANGE(11, 17);
            DWORD       Huep2 : BITFIELD_RANGE(18, 24);
            DWORD       Huep3 : BITFIELD_RANGE(25, 31);
        };
        DWORD           Value;
    } DW18;

    // DWORD 19
    union {
        struct {
            DWORD       Hueb1 : BITFIELD_RANGE(0, 9);
            DWORD       Hueb2 : BITFIELD_RANGE(10, 19);
            DWORD       Hueb3 : BITFIELD_RANGE(20, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW19;

    // DWORD 20
    union {
        struct {
            DWORD       Hues0 : BITFIELD_RANGE(0, 10);
            DWORD       Hues1 : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW20;

    // DWORD 21
    union {
        struct {
            DWORD       Hues2 : BITFIELD_RANGE(0, 10);
            DWORD       Hues3 : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW21;

    // DWORD 22
    union {
        struct {
            DWORD       Satp1Dark : BITFIELD_RANGE(0, 6);
            DWORD       Satp2Dark : BITFIELD_RANGE(7, 13);
            DWORD       Satp3Dark : BITFIELD_RANGE(14, 20);
            DWORD       Satb1Dark : BITFIELD_RANGE(21, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW22;

    // DWORD 23
    union {
        struct {
            DWORD       Satb2Dark : BITFIELD_RANGE(0, 9);
            DWORD       Satb3Dark : BITFIELD_RANGE(10, 19);
            DWORD       Sats0Dark : BITFIELD_RANGE(20, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW23;

    // DWORD 24
    union {
        struct {
            DWORD       Sats1Dark : BITFIELD_RANGE(0, 10);
            DWORD       Sats2Dark : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW24;

    // DWORD 25
    union {
        struct {
            DWORD       Sats3Dark : BITFIELD_RANGE(0, 10);
            DWORD       Huep1Dark : BITFIELD_RANGE(11, 17);
            DWORD       Huep2Dark : BITFIELD_RANGE(18, 24);
            DWORD       Huep3Dark : BITFIELD_RANGE(25, 31);
        };
        DWORD           Value;
    } DW25;

    // DWORD 26
    union {
        struct {
            DWORD       Hueb1Dark : BITFIELD_RANGE(0, 9);
            DWORD       Hueb2Dark : BITFIELD_RANGE(10, 19);
            DWORD       Hueb3Dark : BITFIELD_RANGE(20, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW26;

    // DWORD 27
    union {
        struct {
            DWORD       Hues0Dark : BITFIELD_RANGE(0, 10);
            DWORD       Hues1Dark : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW27;

    // DWORD 28
    union {
        struct {
            DWORD       Hues2Dark : BITFIELD_RANGE(0, 10);
            DWORD       Hues3Dark : BITFIELD_RANGE(11, 21);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW28;
} VEBOX_STD_STE_STATE_G9, *PVEBOX_STD_STE_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_ACE_LACE_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       AceEnable : BITFIELD_BIT(0);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(1);
            DWORD       SkinThreshold : BITFIELD_RANGE(2, 6);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(7, 11);
            DWORD       LaceHistogramEnable : BITFIELD_BIT(12);
            DWORD       LaceHistogramSize : BITFIELD_BIT(13);
            DWORD       LaceSingleHistogramSet : BITFIELD_RANGE(14, 15);
            DWORD       MinAceLuma : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       Ymin : BITFIELD_RANGE(0, 7);
            DWORD       Y1 : BITFIELD_RANGE(8, 15);
            DWORD       Y2 : BITFIELD_RANGE(16, 23);
            DWORD       Y3 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       Y4 : BITFIELD_RANGE(0, 7);
            DWORD       Y5 : BITFIELD_RANGE(8, 15);
            DWORD       Y6 : BITFIELD_RANGE(16, 23);
            DWORD       Y7 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       Y8 : BITFIELD_RANGE(0, 7);
            DWORD       Y9 : BITFIELD_RANGE(8, 15);
            DWORD       Y10 : BITFIELD_RANGE(16, 23);
            DWORD       Ymax : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       B1 : BITFIELD_RANGE(0, 7);
            DWORD       B2 : BITFIELD_RANGE(8, 15);
            DWORD       B3 : BITFIELD_RANGE(16, 23);
            DWORD       B4 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       B5 : BITFIELD_RANGE(0, 7);
            DWORD       B6 : BITFIELD_RANGE(8, 15);
            DWORD       B7 : BITFIELD_RANGE(16, 23);
            DWORD       B8 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       B9 : BITFIELD_RANGE(0, 7);
            DWORD       B10 : BITFIELD_RANGE(8, 15);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       S0 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S1 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       S2 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S3 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       S4 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S5 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       S6 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S7 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       S8 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       S9 : BITFIELD_RANGE(16, 26);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(27, 31);
        };
        DWORD           Value;
    } DW11;

    // DWORD 12
    union {
        struct {
            DWORD       S10 : BITFIELD_RANGE(0, 10);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(11, 15);
            DWORD       MaxAceLuma : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW12;

} VEBOX_ACE_LACE_STATE_G9, *PVEBOX_ACE_LACE_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_TCC_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 6);
            DWORD       TccEnable : BITFIELD_BIT(7);
            DWORD       SatFactor1 : BITFIELD_RANGE(8, 15);
            DWORD       SatFactor2 : BITFIELD_RANGE(16, 23);
            DWORD       SatFactor3 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 7);
            DWORD       SatFactor4 : BITFIELD_RANGE(8, 15);
            DWORD       SatFactor5 : BITFIELD_RANGE(16, 23);
            DWORD       SatFactor6 : BITFIELD_RANGE(24, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       BaseColor1 : BITFIELD_RANGE(0, 9);
            DWORD       BaseColor2 : BITFIELD_RANGE(10, 19);
            DWORD       BaseColor3 : BITFIELD_RANGE(20, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       BaseColo4 : BITFIELD_RANGE(0, 9);
            DWORD       BaseColor5 : BITFIELD_RANGE(10, 19);
            DWORD       BaseColor6 : BITFIELD_RANGE(20, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       ColorTransitSlope2 : BITFIELD_RANGE(0, 15);
            DWORD       ColorTransitSlope23 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       ColorTransitSlope34 : BITFIELD_RANGE(0, 15);
            DWORD       ColorTransitSlope45 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       ColorTransitSlope56 : BITFIELD_RANGE(0, 15);
            DWORD       ColorTransitSlope61 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 1);
            DWORD       ColorBias1 : BITFIELD_RANGE(2, 11);
            DWORD       ColorBias2 : BITFIELD_RANGE(12, 21);
            DWORD       ColorBias3 : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(0, 1);
            DWORD       ColorBias4 : BITFIELD_RANGE(2, 11);
            DWORD       ColorBias5 : BITFIELD_RANGE(12, 21);
            DWORD       ColorBias6 : BITFIELD_RANGE(22, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       SteSlopeBits : BITFIELD_RANGE(0, 2);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(3, 7);
            DWORD       SteThreshold : BITFIELD_RANGE(8, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 15);
            DWORD       UvThresholdBits : BITFIELD_RANGE(16, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 23);
            DWORD       UvThreshold : BITFIELD_RANGE(24, 30);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       UVMaxColor : BITFIELD_RANGE(0, 8);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(9, 15);
            DWORD       InvUvmaxColor : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW10;

} VEBOX_TCC_STATE_G9, *PVEBOX_TCC_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_PROCAMP_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       ProcampEnable : BITFIELD_BIT(0);
            DWORD       Brightness : BITFIELD_RANGE(1, 12);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(13, 16);
            DWORD       Contrast : BITFIELD_RANGE(17, 27);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(28, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       SinCS : BITFIELD_RANGE(0, 15);
            DWORD       CosCS : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW1;

} VEBOX_PROCAMP_STATE_G9, *PVEBOX_PROCAMP_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_CSC_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       C0 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 29);
            DWORD       YuvChannelSwap : BITFIELD_BIT(30);
            DWORD       TransformEnable : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       C1 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       C2 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       C3 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       C4 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       C5 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       C6 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       C7 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       C8 : BITFIELD_RANGE(0, 18);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       OffsetIn1 : BITFIELD_RANGE(0, 15);
            DWORD       OffsetOut1 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       OffsetIn2 : BITFIELD_RANGE(0, 15);
            DWORD       OffsetOut2 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       OffsetIn3 : BITFIELD_RANGE(0, 15);
            DWORD       OffsetOut3 : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW11;

} VEBOX_CSC_STATE_G9, *PVEBOX_CSC_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_ALPHA_AOI_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       ColorPipeAlpha : BITFIELD_RANGE(0, 15);
            DWORD       AlphaFromStateSelect : BITFIELD_BIT(16);
            DWORD       FullImageHistogram : BITFIELD_BIT(17);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(18, 31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       AoiMinX : BITFIELD_RANGE(0, 13);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(14, 15);
            DWORD       AoiMaxX : BITFIELD_RANGE(16, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       AoiMinY : BITFIELD_RANGE(0, 13);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(14, 15);
            DWORD       AoiMaxY : BITFIELD_RANGE(16, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD           Value;
    } DW2;

} VEBOX_ALPHA_AOI_STATE_G9, *PVEBOX_ALPHA_AOI_STATE_G9;


// Defined in vol2c "Vebox"
typedef struct _VEBOX_CCM_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       C1 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 30);
            DWORD       ColorCorrectionMatrixEnable : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       C0 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       C3 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       C2 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       C5 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       C4 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       C7 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       C6 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       C8 : BITFIELD_RANGE(0, 16);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(17, 31);
        };
        DWORD           Value;
    } DW8;

} VEBOX_CCM_STATE_G9, *PVEBOX_CCM_STATE_G9;

// Defined in vol2c "Vebox"
typedef struct _VEBOX_FRONT_END_CSC_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       FecscC0TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 30);
            DWORD       FrontEndCscTransformEnable : BITFIELD_BIT(31);
        };
        DWORD           Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       FecscC1TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       FecscC2TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       FecscC3TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       FecscC4TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       FecscC5TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       FecscC6TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW6;

    // DWORD 7
    union {
        struct {
            DWORD       FecscC7TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW7;

    // DWORD 8
    union {
        struct {
            DWORD       FecscC8TransformCoefficient : BITFIELD_RANGE(0, 18);
            DWORD       Rserved : BITFIELD_RANGE(19, 31);
        };
        DWORD           Value;
    } DW8;

    // DWORD 9
    union {
        struct {
            DWORD       FecScOffsetIn1OffsetInForYR : BITFIELD_RANGE(0, 15);
            DWORD       FecScOffsetOut1OffsetOutForYR : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW9;

    // DWORD 10
    union {
        struct {
            DWORD       FecScOffsetIn2OffsetOutForUG : BITFIELD_RANGE(0, 15);
            DWORD       FecScOffsetOut2OffsetOutForUG : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW10;

    // DWORD 11
    union {
        struct {
            DWORD       FecScOffsetIn3OffsetOutForVB : BITFIELD_RANGE(0, 15);
            DWORD       FecScOffsetOut3OffsetOutForVB : BITFIELD_RANGE(16, 31);
        };
        DWORD           Value;
    } DW11;

} VEBOX_FRONT_END_CSC_STATE_G9, *PVEBOX_FRONT_END_CSC_STATE_G9;

// Defined in vol2c "Vebox"
typedef struct _VEBOX_IECP_STATE_G9
{
    // DWORD 0_28
    VEBOX_STD_STE_STATE_G9                      StdSteState;

    // DWORD 29_41
    VEBOX_ACE_LACE_STATE_G9                     AceLaceState;

    // DWORD 42_52
    VEBOX_TCC_STATE_G9                          TccState;

    // DWORD 53_54
    VEBOX_PROCAMP_STATE_G9                      ProcAmpState;

    // DWORD 55_66
    VEBOX_CSC_STATE_G9                          CscState;

    // DWORD 67_69
    VEBOX_ALPHA_AOI_STATE_G9                    AlphaAoiState;

    // DWORD 70_78
    VEBOX_CCM_STATE_G9                          CcmState;

    // DWORD 79_90
    VEBOX_FRONT_END_CSC_STATE_G9                FrontEndCscState;
} VEBOX_IECP_STATE_G9, *PVEBOX_IECP_STATE_G9;


// Defined in vol2c "VEBOX"
typedef struct _VEBOX_CAPTURE_PIPE_STATE_G9
{
    // DWORD 0
    union {
        struct {
            DWORD       GoodPixelNeighborThreshold : BITFIELD_RANGE(0, 5);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(6, 7);
            DWORD       AverageColorThreshold : BITFIELD_RANGE(8, 15);
            DWORD       GreenImbalanceThreshold : BITFIELD_RANGE(16, 19);
            DWORD       ShiftMinCost : BITFIELD_RANGE(20, 22);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(23);
            DWORD       GoodPixelThreshold : BITFIELD_RANGE(24, 29);
            DWORD       __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(30, 31);
        };
        DWORD Value;
    } DW0;

    // DWORD 1
    union {
        struct {
            DWORD       BadColorThreshold3 : BITFIELD_RANGE(0, 3);
            DWORD       NumberBigPixelTheshold : BITFIELD_RANGE(4, 7);
            DWORD       BadColorThreshold2 : BITFIELD_RANGE(8, 15);
            DWORD       BadColorThreshold1 : BITFIELD_RANGE(16, 23);
            DWORD       GoodIntesityThreshold : BITFIELD_RANGE(24, 27);
            DWORD       ScaleForMinCost : BITFIELD_RANGE(28, 31);
        };
        DWORD Value;
    } DW1;

    // DWORD 2
    union {
        struct {
            DWORD       WhiteBalanceCorrectionEnable : BITFIELD_BIT(0);
            DWORD       BlackPointCorrectionEnable : BITFIELD_BIT(1);
            DWORD       VignetteCorrectionFormat : BITFIELD_BIT(2);
            DWORD       RgbHistogramEnable : BITFIELD_BIT(3);
            DWORD       BlackPointOffsetGreenBottomMsb : BITFIELD_BIT(4);
            DWORD       BlackPointOffsetBlueMsb : BITFIELD_BIT(5);
            DWORD       BlackPointOffsetGreenTopMsb : BITFIELD_BIT(6);
            DWORD       BlackPointOffsetRedMsb : BITFIELD_BIT(7);
            DWORD       UvThresholdValue : BITFIELD_RANGE(8, 15);
            DWORD       YOutlierValue : BITFIELD_RANGE(16, 23);
            DWORD       YBrightValue : BITFIELD_RANGE(24, 31);
        };
        DWORD Value;
    } DW2;

    // DWORD 3
    union {
        struct {
            DWORD       BlackPointOffsetGreenTop : BITFIELD_RANGE(0, 15);
            DWORD       BlackPointOffsetRed : BITFIELD_RANGE(16, 31);
        };
        DWORD Value;
    } DW3;

    // DWORD 4
    union {
        struct {
            DWORD       BlackPointOffsetGreenBottom : BITFIELD_RANGE(0, 15);
            DWORD       BlackPointOffsetBlue : BITFIELD_RANGE(16, 31);
        };
        DWORD Value;
    } DW4;

    // DWORD 5
    union {
        struct {
            DWORD       WhiteBalanceGreenTopCorrection : BITFIELD_RANGE(0, 15);
            DWORD       WhiteBalanceRedCorrection : BITFIELD_RANGE(16, 31);
        };
        DWORD Value;
    } DW5;

    // DWORD 6
    union {
        struct {
            DWORD       WhiteBalanceGreenBottomCorrection : BITFIELD_RANGE(0, 15);
            DWORD       WhiteBalanceBlueCorrection : BITFIELD_RANGE(16, 31);
        };
        DWORD Value;
    } DW6;

} VEBOX_CAPTURE_PIPE_STATE_G9, *PVEBOX_CAPTURE_PIPE_STATE_G9;

typedef struct _CM_VEBOX_STATE_G9
{
    union
    {
        struct
        {
            DWORD       ColorGamutExpansionEnable : 1;
            DWORD       ColorGamutCompressionEnable : 1;
            DWORD       GlobalIECPEnable : 1;
            DWORD       DNEnable : 1;
            DWORD       DIEnable : 1;
            DWORD       DNDIFirstFrame : 1;
            DWORD       DownsampleMethod422to420 : 1;
            DWORD       DownsampleMethod444to422 : 1;
            DWORD       DIOutputFrames : 2;
            DWORD       DemosaicEnable : 1;
            DWORD       VignetteEnable : 1;
            DWORD       AlphaPlaneEnable : 1;
            DWORD       HotPixelFilteringEnable : 1;
            DWORD       SingleSliceVeboxEnable : 1;
            DWORD       LaceCorrectionEnable : BITFIELD_BIT(16);
            DWORD       DisableEncoderStatistics : BITFIELD_BIT(17);
            DWORD       DisableTemporalDenoiseFilter : BITFIELD_BIT(18);
            DWORD       SinglePipeEnable : BITFIELD_BIT(19);
            DWORD      __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(20);
            DWORD       ForwardGammaCorrectionEnable : BITFIELD_BIT(21);
            DWORD        __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 24);
            DWORD       StateSurfaceControlBits : BITFIELD_RANGE(25, 31);
        };
        struct
        {
            DWORD       Value;
        };
    } DW0;

    VEBOX_DNDI_STATE_G9*         pDndiState;
    VEBOX_IECP_STATE_G9*         pIecpState;
    VEBOX_GAMUT_STATE_G75*       pGamutState;
    VEBOX_VERTEX_TABLE_G75*      pVertexTable;
    VEBOX_CAPTURE_PIPE_STATE_G9* pCapturePipe;
}  CM_VEBOX_STATE_G9, *PCM_VEBOX_STATE_G9;

typedef struct _CM_POWER_OPTION
{
    USHORT nSlice;                      // set number of slice to use: 0(default number), 1, 2...
    USHORT nSubSlice;                   // set number of subslice to use: 0(default number), 1, 2... 
    USHORT nEU;                         // set number of EU to use: 0(default number), 1, 2...
} CM_POWER_OPTION, *PCM_POWER_OPTION;

typedef struct _CM_SET_GPU_FREQUENCY
{
    ULONG                   nFrequencyInMHZ;        // set the Frequency in MHZ for GPU
    BOOL                    bEnableTurbo;           // if let the frequency to turbo or to be fixed
} CM_SET_GPU_FREQUENCY, *PCM_SET_GPU_FREQUENCY;

typedef enum {
    UN_PREEMPTABLE_MODE,
    COMMAND_BUFFER_MODE,
    THREAD_GROUP_MODE,
    MIDDLE_THREAD_MODE
} CM_HAL_PREEMPTION_MODE;

typedef enum _CM_ROTATION
{
    CM_ROTATION_IDENTITY = 0,      //!< Rotation 0 degrees
    CM_ROTATION_90,                //!< Rotation 90 degrees
    CM_ROTATION_180,               //!< Rotation 180 degrees
    CM_ROTATION_270,               //!< Rotation 270 degrees
} CM_ROTATION;

// to support new flag with current API
// new flag/field could be add to the end of this structure
// 
typedef struct _CM_FLAG {
    CM_ROTATION rotationFlag;
} CM_FLAG;

//!
//! CM Sampler8X8 
//!
class CmSampler8x8
{
public:
    CM_RT_API virtual INT GetIndex( SamplerIndex* & pIndex ) = 0 ;

};
/***********END SAMPLER8X8********************/
class CmEvent;
#define CM_CALLBACK __cdecl
typedef void (CM_CALLBACK *callback_function)(CmEvent*, void *);

class CmEvent
{
public:
    CM_RT_API virtual INT GetStatus( CM_STATUS& status) = 0 ;
    CM_RT_API virtual INT GetExecutionTime(UINT64& time) = 0;
    CM_RT_API virtual INT WaitForTaskFinished(DWORD dwTimeOutMs = CM_MAX_TIMEOUT_MS) = 0 ;
    CM_RT_API virtual INT WaitForTaskFlushed(DWORD dwTimeOutMs = CM_MAX_TIMEOUT_MS) = 0 ;
    CM_RT_API virtual INT GetSurfaceDetails( UINT kernIndex, UINT surfBTI,CM_SURFACE_DETAILS& outDetails )=0;
    CM_RT_API virtual INT GetProfilingInfo(CM_EVENT_PROFILING_INFO infoType, size_t paramSize, PVOID pInputValue, PVOID pValue) = 0;
};

class CmThreadSpace;
class CmThreadGroupSpace;

class CmKernel
{
public:       
    CM_RT_API virtual INT SetThreadCount(UINT count ) = 0;
    CM_RT_API virtual INT SetKernelArg(UINT index, size_t size, const void * pValue ) = 0;

    CM_RT_API virtual INT SetThreadArg(UINT threadId, UINT index, size_t size, const void * pValue ) = 0;
    CM_RT_API virtual INT SetStaticBuffer(UINT index, const void * pValue ) = 0;
    CM_RT_API virtual INT SetKernelPayloadData(size_t size, const void *pValue) = 0;
    CM_RT_API virtual INT SetKernelPayloadSurface(UINT surfaceCount, SurfaceIndex** pSurfaces) = 0;
    CM_RT_API virtual INT SetSurfaceBTI(SurfaceIndex* pSurface, UINT BTIndex) = 0;
    CM_RT_API virtual INT AssociateThreadSpace(CmThreadSpace* & pTS) = 0;
    CM_RT_API virtual INT AssociateThreadGroupSpace(CmThreadGroupSpace* & pTGS) = 0; 
    CM_RT_API virtual INT SetSamplerBTI(SamplerIndex* pSampler, UINT nIndex) = 0;
    CM_RT_API virtual INT DeAssociateThreadSpace(CmThreadSpace* & pTS) = 0;
    CM_RT_API virtual INT DeAssociateThreadGroupSpace(CmThreadGroupSpace* & pTGS) = 0;
    CM_RT_API virtual INT QuerySpillSize(unsigned int &spillSize) = 0;
};

class CmTask
{
public:       
    CM_RT_API virtual INT AddKernel(CmKernel *pKernel) = 0;
    CM_RT_API virtual INT Reset(void) = 0;
    CM_RT_API virtual INT AddSync(void) = 0;
    CM_RT_API virtual INT SetPowerOption( PCM_POWER_OPTION pCmPowerOption ) = 0;
    CM_RT_API virtual INT SetPreemptionMode(CM_HAL_PREEMPTION_MODE mode) = 0;
    CM_RT_API virtual INT AddConditionalEnd(SurfaceIndex* pConditionalSurface) = 0;
}; 

class CmProgram;
class SurfaceIndex;
class SamplerIndex;

class CmBuffer
{
public:
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT InitSurface(const DWORD initValue, CmEvent* pEvent) = 0;
    CM_RT_API virtual INT SetMemoryObjectControl(MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, UINT  age) = 0;
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl) = 0;
    CM_RT_API virtual INT SetAsConditionalSurface(DWORD comparisonValue, bool enableCompareMask = false) = 0;
};

class CmBufferUP
{
public:
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT SetMemoryObjectControl(MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, UINT  age) = 0;
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl) = 0;
    CM_RT_API virtual INT SetAsConditionalSurface(DWORD comparisonValue, bool enableCompareMask = false) = 0;
};

class CmBufferSVM
{
public:
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT GetAddress( void * &pAddr) = 0;
};

class CmSurface2D
{
public:    
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 

#ifdef CM_DX9
    CM_RT_API virtual INT GetD3DSurface( IDirect3DSurface9*& pD3DSurface) = 0;
#elif defined(CM_DX11)
    CM_RT_API virtual INT GetD3DSurface( ID3D11Texture2D*& pD3D11Texture2D) = 0;
#endif

    CM_RT_API virtual INT ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT ReadSurfaceStride( unsigned char* pSysMem, CmEvent* pEvent, const UINT stride, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT WriteSurfaceStride( const unsigned char* pSysMem, CmEvent* pEvent, const UINT stride, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT InitSurface(const DWORD initValue, CmEvent* pEvent) = 0;
#ifdef CM_DX11
    CM_RT_API virtual INT QuerySubresourceIndex(UINT& FirstArraySlice, UINT& FirstMipSlice) = 0;
#endif
    CM_RT_API virtual INT SetMemoryObjectControl(MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, UINT  age) = 0;
    CM_RT_API virtual INT SetSurfaceStateDimensions(UINT iWidth, UINT iHeight, SurfaceIndex* pSurfIndex = NULL) = 0;
#ifdef __GNUC__
    CM_RT_API virtual INT GetVaSurfaceID( VASurfaceID  &iVASurface) = 0;
#endif
    CM_RT_API virtual INT ReadSurfaceHybridStrides( unsigned char* pSysMem, CmEvent* pEvent, const UINT iWidthStride, const UINT iHeightStride, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL, UINT uiOption = 0 ) = 0;
    CM_RT_API virtual INT WriteSurfaceHybridStrides( const unsigned char* pSysMem, CmEvent* pEvent, const UINT iWidthStride, const UINT iHeightStride, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL, UINT uiOption = 0 ) = 0;
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl) = 0;
};

class CmSurface2DUP  
{
public:    
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT SetMemoryObjectControl(MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, UINT  age) = 0;
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl) = 0;
};

class CmSurface3D  
{
public:    
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT InitSurface(const DWORD initValue, CmEvent* pEvent) = 0;
    CM_RT_API virtual INT SetMemoryObjectControl(MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, UINT  age) = 0;
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl) = 0;
};

class CmSampler
{
public:
    CM_RT_API virtual INT GetIndex( SamplerIndex* & pIndex ) = 0 ;
};

class CmThreadSpace
{
public:
    CM_RT_API virtual INT AssociateThread( UINT x, UINT y, CmKernel* pKernel , UINT threadId ) = 0;
    CM_RT_API virtual INT SelectThreadDependencyPattern ( CM_DEPENDENCY_PATTERN pattern ) = 0;
    CM_RT_API virtual INT AssociateThreadWithMask( UINT x, UINT y, CmKernel* pKernel , UINT threadId, BYTE nDependencyMask ) = 0;
    CM_RT_API virtual INT SetThreadSpaceColorCount( UINT colorCount ) = 0;
    CM_RT_API virtual INT SelectMediaWalkingPattern( CM_WALKING_PATTERN pattern ) = 0;
    CM_RT_API virtual INT Set26ZIDispatchPattern( CM_26ZI_DISPATCH_PATTERN pattern ) = 0;
    CM_RT_API virtual INT Set26ZIMacroBlockSize( UINT width, UINT height )  = 0;
    CM_RT_API virtual INT SetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT groupSelect) = 0;
};

class CmThreadGroupSpace;

class CmVebox
{
public:
    CM_RT_API virtual INT SetState(const void* pVeBoxState) = 0;

    CM_RT_API virtual INT SetCurFrameInputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetCurFrameInputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetPrevFrameInputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetPrevFrameInputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetSTMMInputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetSTMMInputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetSTMMOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetSTMMOutputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetDenoisedCurFrameOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetDenoisedCurOutputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetCurFrameOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetCurFrameOutputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetPrevFrameOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetPrevFrameOutputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetStatisticsOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetStatisticsOutputSurfaceControlBits( const WORD ctrlBits ) = 0;
};

class CmQueue
{
public:    
    CM_RT_API virtual INT Enqueue( CmTask* pTask, CmEvent* & pEvent, const CmThreadSpace* pTS = NULL ) = 0;
    CM_RT_API virtual INT DestroyEvent( CmEvent* & pEvent ) = 0; 
    CM_RT_API virtual INT EnqueueWithGroup( CmTask* pTask, CmEvent* & pEvent, const CmThreadGroupSpace* pTGS = NULL )=0;
    CM_RT_API virtual INT EnqueueCopyCPUToGPU( CmSurface2D* pSurface, const unsigned char* pSysMem, CmEvent* & pEvent ) = 0; 
    CM_RT_API virtual INT EnqueueCopyGPUToCPU( CmSurface2D* pSurface, unsigned char* pSysMem, CmEvent* & pEvent ) = 0;
    CM_RT_API virtual INT EnqueueCopyCPUToGPUStride( CmSurface2D* pSurface, const unsigned char* pSysMem, const UINT stride, CmEvent* & pEvent ) = 0; 
    CM_RT_API virtual INT EnqueueCopyGPUToCPUStride( CmSurface2D* pSurface, unsigned char* pSysMem, const UINT stride, CmEvent* & pEvent ) = 0;
    CM_RT_API virtual INT EnqueueInitSurface2D( CmSurface2D* pSurface, const DWORD initValue, CmEvent* &pEvent ) = 0;
    CM_RT_API virtual INT EnqueueCopyGPUToGPU( CmSurface2D* pOutputSurface, CmSurface2D* pInputSurface, CmEvent* & pEvent ) = 0;
    CM_RT_API virtual INT EnqueueCopyCPUToCPU( unsigned char* pDstSysMem, unsigned char* pSrcSysMem, UINT size, CmEvent* & pEvent ) = 0;

    CM_RT_API virtual INT EnqueueCopyCPUToGPUFullStride( CmSurface2D* pSurface, const unsigned char* pSysMem, const UINT widthStride, const UINT heightStride, const UINT option, CmEvent* & pEvent ) = 0;
    CM_RT_API virtual INT EnqueueCopyGPUToCPUFullStride( CmSurface2D* pSurface, unsigned char* pSysMem, const UINT widthStride, const UINT heightStride, const UINT option, CmEvent* & pEvent ) = 0;

    CM_RT_API virtual INT EnqueueWithHints( CmTask* pTask, CmEvent* & pEvent, UINT hints = 0) = 0;
    CM_RT_API virtual INT EnqueueVebox( CmVebox* pVebox, CmEvent* & pEvent ) = 0;
};

class CmVmeState
{
public:
    CM_RT_API virtual INT GetIndex( VmeIndex* & pIndex ) = 0 ;
};

class CmDevice
{
public:

#ifdef CM_DX9
    CM_RT_API virtual INT GetD3DDeviceManager( IDirect3DDeviceManager9* & pDeviceManager )= 0;
#endif

    CM_RT_API virtual INT CreateBuffer(UINT size, CmBuffer* & pSurface )=0;
    CM_RT_API virtual INT CreateSurface2D(UINT width, UINT height, CM_SURFACE_FORMAT format, CmSurface2D* & pSurface ) = 0;
    CM_RT_API virtual INT CreateSurface3D(UINT width, UINT height, UINT depth, CM_SURFACE_FORMAT format, CmSurface3D* & pSurface ) = 0;

#ifdef CM_DX9
    CM_RT_API virtual INT CreateSurface2D( IDirect3DSurface9* pD3DSurf, CmSurface2D* & pSurface ) = 0;
    CM_RT_API virtual INT CreateSurface2D( IDirect3DSurface9** pD3DSurf, const UINT surfaceCount, CmSurface2D**  pSpurface )= 0;
#elif defined(CM_DX11)
    CM_RT_API virtual INT CreateSurface2D( ID3D11Texture2D* pD3DSurf, CmSurface2D* & pSurface ) = 0;
    CM_RT_API virtual INT CreateSurface2D( ID3D11Texture2D** pD3DSurf, const UINT surfaceCount, CmSurface2D**  pSpurface )= 0;
#elif defined (__GNUC__)
    CM_RT_API virtual INT CreateSurface2D( VASurfaceID iVASurface, CmSurface2D* & pSurface ) = 0;
    CM_RT_API virtual INT CreateSurface2D( VASurfaceID* iVASurface, const UINT surfaceCount, CmSurface2D** pSurface ) = 0;
#endif

    CM_RT_API virtual INT DestroySurface( CmBuffer* & pSurface) = 0;
    CM_RT_API virtual INT DestroySurface( CmSurface2D* & pSurface) = 0;
    CM_RT_API virtual INT DestroySurface( CmSurface3D* & pSurface) = 0;
  
    CM_RT_API virtual INT CreateQueue( CmQueue* & pQueue) = 0; 
    CM_RT_API virtual INT LoadProgram( void* pCommonISACode, const UINT size, CmProgram*& pProgram, const char* options = NULL ) = 0;
    CM_RT_API virtual INT CreateKernel( CmProgram* pProgram, const char* kernelName, CmKernel* & pKernel, const char* options = NULL) = 0;
    CM_RT_API virtual INT CreateKernel( CmProgram* pProgram, const char* kernelName, const void * fncPnt, CmKernel* & pKernel, const char* options = NULL) = 0;
    CM_RT_API virtual INT CreateSampler( const CM_SAMPLER_STATE & sampleState, CmSampler* & pSampler ) = 0;

    CM_RT_API virtual INT DestroyKernel( CmKernel*& pKernel) = 0;
    CM_RT_API virtual INT DestroySampler( CmSampler*& pSampler ) = 0;
    CM_RT_API virtual INT DestroyProgram( CmProgram*& pProgram ) = 0;
    CM_RT_API virtual INT DestroyThreadSpace( CmThreadSpace* & pTS ) = 0; 

    CM_RT_API virtual INT CreateTask(CmTask *& pTask)=0;
    CM_RT_API virtual INT DestroyTask(CmTask*& pTask)=0;

    CM_RT_API virtual INT GetCaps(CM_DEVICE_CAP_NAME capName, size_t& capValueSize, void* pCapValue ) = 0;

    CM_RT_API virtual INT CreateVmeStateG6( const VME_STATE_G6 & vmeState, CmVmeState* & pVmeState ) = 0;
    CM_RT_API virtual INT DestroyVmeStateG6( CmVmeState*& pVmeState ) = 0;

    CM_RT_API virtual INT CreateVmeSurfaceG6( CmSurface2D* pCurSurface, CmSurface2D* pForwardSurface, CmSurface2D* pBackwardSurface, SurfaceIndex* & pVmeIndex ) = 0;
    CM_RT_API virtual INT DestroyVmeSurfaceG6( SurfaceIndex* & pVmeIndex ) = 0;

    CM_RT_API virtual INT CreateThreadSpace( UINT width, UINT height, CmThreadSpace* & pTS ) = 0;

    CM_RT_API virtual INT CreateBufferUP(UINT size, void* pSystMem, CmBufferUP* & pSurface )=0;
    CM_RT_API virtual INT DestroyBufferUP( CmBufferUP* & pSurface) = 0;

    CM_RT_API virtual INT GetSurface2DInfo( UINT width, UINT height, CM_SURFACE_FORMAT format, UINT & pitch, UINT & physicalSize)= 0;
    CM_RT_API virtual INT CreateSurface2DUP( UINT width, UINT height, CM_SURFACE_FORMAT format, void* pSysMem, CmSurface2DUP* & pSurface )= 0;
    CM_RT_API virtual INT DestroySurface2DUP( CmSurface2DUP* & pSurface) = 0;

    CM_RT_API virtual INT CreateVmeSurfaceG7_5 ( CmSurface2D* pCurSurface, CmSurface2D** pForwardSurface, CmSurface2D** pBackwardSurface, const UINT surfaceCountForward, const UINT surfaceCountBackward, SurfaceIndex* & pVmeIndex )=0;
    CM_RT_API virtual INT DestroyVmeSurfaceG7_5( SurfaceIndex* & pVmeIndex ) = 0;
    CM_RT_API virtual INT CreateSampler8x8(const CM_SAMPLER_8X8_DESCR  & smplDescr, CmSampler8x8*& psmplrState)=0;
    CM_RT_API virtual INT DestroySampler8x8( CmSampler8x8*& pSampler )=0;
    CM_RT_API virtual INT CreateSampler8x8Surface(CmSurface2D* p2DSurface, SurfaceIndex* & pDIIndex, CM_SAMPLER8x8_SURFACE surf_type = CM_VA_SURFACE, CM_SURFACE_ADDRESS_CONTROL_MODE = CM_SURFACE_CLAMP )=0;
    CM_RT_API virtual INT DestroySampler8x8Surface(SurfaceIndex* & pDIIndex)=0;

    CM_RT_API virtual INT CreateThreadGroupSpace( UINT thrdSpaceWidth, UINT thrdSpaceHeight, UINT grpSpaceWidth, UINT grpSpaceHeight, CmThreadGroupSpace*& pTGS ) = 0;
    CM_RT_API virtual INT DestroyThreadGroupSpace(CmThreadGroupSpace*& pTGS) = 0;
    CM_RT_API virtual INT SetL3Config( L3_CONFIG_REGISTER_VALUES *l3_c) = 0;
    CM_RT_API virtual INT SetSuggestedL3Config( L3_SUGGEST_CONFIG l3_s_c) = 0;

    CM_RT_API virtual INT SetCaps(CM_DEVICE_CAP_NAME capName, size_t capValueSize, void* pCapValue) = 0;

    CM_RT_API virtual INT CreateGroupedVAPlusSurface(CmSurface2D* p2DSurface1, CmSurface2D* p2DSurface2, SurfaceIndex* & pDIIndex, CM_SURFACE_ADDRESS_CONTROL_MODE = CM_SURFACE_CLAMP)=0;
    CM_RT_API virtual INT DestroyGroupedVAPlusSurface(SurfaceIndex* & pDIIndex)=0;

#ifdef CM_DX11
    CM_RT_API virtual INT GetD3D11Device(ID3D11Device* &pD3D11Device) = 0;
#endif

    CM_RT_API virtual INT CreateSamplerSurface2D(CmSurface2D* p2DSurface, SurfaceIndex* & pSamplerSurfaceIndex) = 0;
    CM_RT_API virtual INT CreateSamplerSurface3D(CmSurface3D* p3DSurface, SurfaceIndex* & pSamplerSurfaceIndex) = 0;
    CM_RT_API virtual INT DestroySamplerSurface(SurfaceIndex* & pSamplerSurfaceIndex) = 0;
    CM_RT_API virtual INT GetRTDllVersion(CM_DLL_FILE_VERSION* pFileVersion) = 0;
    CM_RT_API virtual INT GetJITDllVersion(CM_DLL_FILE_VERSION* pFileVersion) = 0;

    CM_RT_API virtual INT InitPrintBuffer(size_t printbufsize = 1048576) = 0; 
    CM_RT_API virtual INT FlushPrintBuffer() = 0;

#ifdef CM_DX11
    CM_RT_API virtual INT CreateSurface2DSubresource( ID3D11Texture2D* pD3D11Texture2D, UINT subresourceCount, CmSurface2D** ppSurfaces, UINT& createdSurfaceCount, UINT option = 0 ) = 0;
    CM_RT_API virtual INT CreateSurface2DbySubresourceIndex( ID3D11Texture2D* pD3D11Texture2D, UINT FirstArraySlice, UINT FirstMipSlice, CmSurface2D* &pSurface) = 0;
#endif

    CM_RT_API virtual INT CreateVebox( CmVebox* & pVebox ) = 0; //HSW
    CM_RT_API virtual INT DestroyVebox( CmVebox* & pVebox ) = 0; //HSW

#ifdef CM_DX11
    CM_RT_API virtual INT CreateSurface2DInitData( UINT width, UINT height, CM_SURFACE_FORMAT format, CmSurface2D* & pSurface, D3D11_SUBRESOURCE_DATA * pInitData = NULL ) = 0;
#endif

    CM_RT_API virtual INT LoadProgramArray(void* pCombinedCISACode, const UINT CISACodeSize, void* pCombinedKernelBinaryCode, const UINT BinaryCodeSize, CmProgram** pProgramArray, const char* options = NULL ) = 0;

#ifdef __GNUC__
    CM_RT_API virtual INT GetVaDpy(VADisplay* & pva_dpy) = 0;
    CM_RT_API virtual INT CreateVaSurface2D( UINT width, UINT height, CM_SURFACE_FORMAT format, VASurfaceID & iVASurface, CmSurface2D* & pSurface) = 0;
#endif

    CM_RT_API virtual INT CreateBufferSVM(UINT size, void* & pSystMem, CmBufferSVM* & pSurface ) = 0;
    CM_RT_API virtual INT DestroyBufferSVM( CmBufferSVM* & pSurface) = 0;
    CM_RT_API virtual INT CreateSamplerSurface2DUP(CmSurface2DUP* p2DUPSurface, SurfaceIndex* & pSamplerSurfaceIndex) = 0;

    CM_RT_API virtual INT SetGPUThreadPriority(INT priority) = 0;
    CM_RT_API virtual INT GetGPUThreadPriority(INT *priority) = 0;

    CM_RT_API virtual INT SetGPUFrequency( PCM_SET_GPU_FREQUENCY pFreqParam ) = 0;

    CM_RT_API virtual INT CloneKernel( CmKernel * &pKernelDest, CmKernel * pKernelSrc ) = 0;
    CM_RT_API virtual CmKernel* CombineKernel( UINT nKernelNumber, ... ) = 0;

    CM_RT_API virtual INT CreateSurface2DAlias(CmSurface2D* p2DSurface, SurfaceIndex* &aliasSurfaceIndex) = 0;

    //adding new functions in the bottom is a must 
    CM_RT_API virtual INT CreateHevcVmeSurfaceG10 ( CmSurface2D* pCurSurface, CmSurface2D** pForwardSurface, CmSurface2D** pBackwardSurface, const UINT surfaceCountForward, const UINT surfaceCountBackward, SurfaceIndex* & pVmeIndex )=0;
    CM_RT_API virtual INT DestroyHevcVmeSurfaceG10( SurfaceIndex* & pVmeIndex )=0;
    CM_RT_API virtual INT CreateSamplerEx( const CM_SAMPLER_STATE_EX & sampleState, CmSampler* & pSampler ) = 0;
    CM_RT_API virtual INT FlushPrintBufferIntoFile(const char *filename) = 0;
    CM_RT_API virtual INT CreateThreadGroupSpaceEx(UINT thrdSpaceWidth, UINT thrdSpaceHeight, UINT thrdSpaceDepth, UINT grpSpaceWidth, UINT grpSpaceHeight, UINT grpSpaceDepth, CmThreadGroupSpace*& pTGS) = 0;

    CM_RT_API virtual INT CreateSampler8x8SurfaceEx(CmSurface2D* p2DSurface, SurfaceIndex* & pDIIndex, CM_SAMPLER8x8_SURFACE surf_type = CM_VA_SURFACE, CM_SURFACE_ADDRESS_CONTROL_MODE = CM_SURFACE_CLAMP, CM_FLAG* pFlag = NULL) = 0;
    CM_RT_API virtual INT CreateSamplerSurface2DEx(CmSurface2D* p2DSurface, SurfaceIndex* & pSamplerSurfaceIndex, CM_FLAG* pFlag = NULL) = 0;
    CM_RT_API virtual INT CreateSamplerSurface3DEx(CmSurface3D* p3DSurface, SurfaceIndex* & pSamplerSurfaceIndex, CM_FLAG* pFlag = NULL) = 0;
};

typedef void (*IMG_WALKER_FUNTYPE)(void* img, void* arg);

EXTERN_C CM_RT_API void  CMRT_SetHwDebugStatus(bool bInStatus);
EXTERN_C CM_RT_API bool CMRT_GetHwDebugStatus();
EXTERN_C CM_RT_API INT CMRT_GetSurfaceDetails(CmEvent* pEvent, UINT kernIndex, UINT surfBTI, CM_SURFACE_DETAILS& outDetails);
EXTERN_C CM_RT_API void CMRT_PrepareGTPinBuffers(void* ptr0, int size0InBytes, void* ptr1, int size1InBytes, void* ptr2, int size2InBytes);
EXTERN_C CM_RT_API void CMRT_SetGTPinArguments(char* commandLine, void* gtpinInvokeStruct);
EXTERN_C CM_RT_API void CMRT_EnableGTPinMarkers(void);

EXTERN_C CM_RT_API INT DestroyCmDevice(CmDevice* &pD);



EXTERN_C CM_RT_API UINT CMRT_GetKernelCount(CmEvent *pEvent);
EXTERN_C CM_RT_API INT CMRT_GetKernelName(CmEvent *pEvent, UINT index, char** KernelName);
EXTERN_C CM_RT_API INT CMRT_GetKernelThreadSpace(CmEvent *pEvent, UINT index, UINT* localWidth, UINT* localHeight, UINT* globalWidth, UINT* globalHeight);
EXTERN_C CM_RT_API INT CMRT_GetSubmitTime(CmEvent *pEvent, LARGE_INTEGER* time);
EXTERN_C CM_RT_API INT CMRT_GetHWStartTime(CmEvent *pEvent, LARGE_INTEGER* time);
EXTERN_C CM_RT_API INT CMRT_GetHWEndTime(CmEvent *pEvent, LARGE_INTEGER* time);
EXTERN_C CM_RT_API INT CMRT_GetCompleteTime(CmEvent *pEvent, LARGE_INTEGER* time);
EXTERN_C CM_RT_API INT CMRT_SetEventCallback(CmEvent* pEvent, callback_function function, void* user_data);
EXTERN_C CM_RT_API INT CMRT_Enqueue(CmQueue* pQueue, CmTask* pTask, CmEvent** pEvent, const CmThreadSpace* pTS = NULL);

#ifdef CM_DX9
    EXTERN_C CM_RT_API INT CreateCmDevice(CmDevice* &pD, UINT& version, IDirect3DDeviceManager9* pD3DDeviceMgr = NULL );
    EXTERN_C CM_RT_API INT CreateCmDeviceEx(CmDevice* &pD, UINT & version, IDirect3DDeviceManager9* pD3DDeviceMgr, UINT  DevCreateOption = CM_DEVICE_CREATE_OPTION_DEFAULT);
    EXTERN_C CM_RT_API INT CreateCmDeviceEmu(CmDevice* &pDevice, UINT& version, IDirect3DDeviceManager9* pD3DDeviceMgr = NULL );
    EXTERN_C CM_RT_API INT DestroyCmDeviceEmu(CmDevice* &pDevice, IDirect3DDeviceManager9* pD3DDeviceMgr = NULL );
    EXTERN_C CM_RT_API INT CreateCmDeviceSim(CmDevice* &pDevice, UINT& version, IDirect3DDeviceManager9* pD3DDeviceMgr = NULL );
    EXTERN_C CM_RT_API INT DestroyCmDeviceSim(CmDevice* &pDevice, IDirect3DDeviceManager9* pD3DDeviceMgr = NULL );
#ifdef CMRT_VISA_EMU
    EXTERN_C CM_RT_API INT CreateCmDeviceCisaEmu(CmDevice* &pDevice, UINT& version, IDirect3DDeviceManager9* pD3DDeviceMgr = NULL );
    EXTERN_C CM_RT_API INT DestroyCmDeviceCisaEmu(CmDevice* &pDevice, IDirect3DDeviceManager9* pD3DDeviceMgr = NULL );
#endif
#elif defined(CM_DX11)
    EXTERN_C CM_RT_API INT CreateCmDevice(CmDevice* &pD, UINT& version, ID3D11Device* pD3D11Device = NULL);
    EXTERN_C CM_RT_API INT CreateCmDeviceEx(CmDevice* &pD, UINT & version, ID3D11Device* pD3D11Device, UINT  DevCreateOption = CM_DEVICE_CREATE_OPTION_DEFAULT);
    EXTERN_C CM_RT_API INT CreateCmDeviceEmu(CmDevice* &pDevice, UINT& version, ID3D11Device* pD3DDeviceMgr = NULL );
    EXTERN_C CM_RT_API INT DestroyCmDeviceEmu(CmDevice* &pDevice, ID3D11Device* pD3DDeviceMgr = NULL );
    EXTERN_C CM_RT_API INT CreateCmDeviceSim(CmDevice* &pDevice, UINT& version, ID3D11Device* pD3DDeviceMgr = NULL );
    EXTERN_C CM_RT_API INT DestroyCmDeviceSim(CmDevice* &pDevice, ID3D11Device* pD3DDeviceMgr = NULL );
#elif defined(__GNUC__)
    EXTERN_C CM_RT_API INT CreateCmDevice(CmDevice* &pD, UINT& version, VADisplay va_dpy = NULL);
    EXTERN_C CM_RT_API INT CreateCmDeviceEx(CmDevice* &pD, UINT & version, VADisplay va_dpy, UINT  DevCreateOption = CM_DEVICE_CREATE_OPTION_DEFAULT);
    EXTERN_C CM_RT_API INT CreateCmDeviceEmu(CmDevice* &pDevice, UINT& version, VADisplay va_dpy = NULL);
    EXTERN_C CM_RT_API INT DestroyCmDeviceEmu(CmDevice* &pDevice);
    EXTERN_C CM_RT_API INT CreateCmDeviceSim(CmDevice* &pDevice, UINT& version);
    EXTERN_C CM_RT_API INT DestroyCmDeviceSim(CmDevice* &pDevice);
#endif

    EXTERN_C CM_RT_API INT CMRT_GetEnqueueTime( CmEvent *pEvent, LARGE_INTEGER* time );

#ifdef CMRT_EMU
    #define CreateCmDevice CreateCmDeviceEmu
    #define DestroyCmDevice DestroyCmDeviceEmu
#elif CMRT_SIM
    #define CreateCmDevice CreateCmDeviceSim
    #define DestroyCmDevice DestroyCmDeviceSim
#elif defined(CMRT_VISA_EMU) && defined(CM_DX9)
    #define CreateCmDevice CreateCmDeviceCisaEmu
    #define DestroyCmDevice DestroyCmDeviceCisaEmu
#endif
