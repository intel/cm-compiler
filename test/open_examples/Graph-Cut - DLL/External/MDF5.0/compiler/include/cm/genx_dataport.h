/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: genx_dataport.h 26308 2011-12-07 22:22:00Z ayermolo $"
*** -----------------------------------------------------------------------------------------------
***
*** Copyright  (C) 1985-2014 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:             Alexey V. Aliev
***                      Oleg Mirochnik
***                      
***                      
***
*** Description: Emulation of GenX data port
***
*** -----------------------------------------------------------------------------------------------
**/


#ifndef GENX_DATAPORT_H
#define GENX_DATAPORT_H

#ifndef NEW_CM_RT
#define NEW_CM_RT  // Defined for new CM Runtime APIs
#endif

#include "cm_list.h"
#include "cm_def.h"
#include "cm_vm.h"
#include "cm_common.h"

typedef enum _CmBufferType_
{
    DPE_INPUT_BUFFER         = 2,
    DPE_OUTPUT_BUFFER        = 3,
    GEN4_INPUT_BUFFER        = 4,
    GEN4_OUTPUT_BUFFER       = 5,
    GEN4_INPUT_OUTPUT_BUFFER = 6,
    GEN4_BUFFER              = 8,
    GEN4_SAMPLING_BUFFER     = 9,
    GEN6_VME_BUFFER          = 10
} CmBufferType;

typedef enum _CmBufferDescField_
{
    GEN4_FIELD_TILE_FORMAT     = 15,
    GEN4_FIELD_SURFACE_TYPE    = 16,
    GEN4_FIELD_SURFACE_FORMAT  = 17,
    GEN4_FIELD_SURFACE_PITCH   = 18,
    GEN4_FIELD_SURFACE_SIZE    = 19,
    GEN4_FIELD_SAMPLING_MIN_MODE = 20,
    GEN4_FIELD_SAMPLING_MIP_MODE = 21,
    GEN4_FIELD_SAMPLING_MAP_MODE = 22,
    GEN4_FIELD_PERSISTENT_MAP = 23,
    GEN4_FIELD_REGISTER_BUFFER = 24,
	GEN4_FIELD_SURFACE_ID = 25
} CmBufferDescField;

typedef enum _CmSurfaceFormatID
{
    R32G32B32A32_FLOAT                  = 0x000,
    R32G32B32A32_SINT                   = 0x001,
    R32G32B32A32_UINT                   = 0x002,
    R32G32B32A32_UNORM                  = 0x003,
    R32G32B32A32_SNORM                  = 0x004,
    R64G64_FLOAT                        = 0x005,
    R32G32B32X32_FLOAT                  = 0x006,
    R32G32B32A32_SSCALED                = 0x007,
    R32G32B32A32_USCALED                = 0x008,
	PLANAR_420_8                        = 0x00D, //13
    R32G32B32_FLOAT                     = 0x040,
    R32G32B32_SINT                      = 0x041,
    R32G32B32_UINT                      = 0x042,
    R32G32B32_UNORM                     = 0x043,
    R32G32B32_SNORM                     = 0x044,
    R32G32B32_SSCALED                   = 0x045,
    R32G32B32_USCALED                   = 0x046,
    R16G16B16A16_UNORM                  = 0x080,
    R16G16B16A16_SNORM                  = 0x081,
    R16G16B16A16_SINT                   = 0x082,
    R16G16B16A16_UINT                   = 0x083,
    R16G16B16A16_FLOAT                  = 0x084,
    R32G32_FLOAT                        = 0x085,
    R32G32_SINT                         = 0x086,
    R32G32_UINT                         = 0x087,
    R32_FLOAT_X8X24_TYPELESS            = 0x088,
    X32_TYPELESS_G8X24_UINT             = 0x089,
    L32A32_FLOAT                        = 0x08a,
    R32G32_UNORM                        = 0x08b,
    R32G32_SNORM                        = 0x08c,
    R64_FLOAT                           = 0x08d,
    R16G16B16X16_UNORM                  = 0x08e, 
    R16G16B16X16_FLOAT                  = 0x08f,
    A32X32_FLOAT                        = 0x090,
    L32X32_FLOAT                        = 0x091,
    I32X32_FLOAT                        = 0x092,
    R16G16B16A16_SSCALED                = 0x093,
    R16G16B16A16_USCALED                = 0x094,
    R32G32_SSCALED                      = 0x095,
    R32G32_USCALED                      = 0x096,
    B8G8R8A8_UNORM                      = 0x0c0,
    B8G8R8A8_UNORM_SRGB                 = 0x0c1,
    R10G10B10A2_UNORM                   = 0x0c2,
    R10G10B10A2_UNORM_SRGB              = 0x0c3,
    R10G10B10A2_UINT                    = 0x0c4,
    R10G10B10_SNORM_A2_UNORM            = 0x0c5,
    R10G10B10_SINT_A2_UINT              = 0x0c6,
    R8G8B8A8_UNORM                      = 0x0c7,
    R8G8B8A8_UNORM_SRGB                 = 0x0c8,
    R8G8B8A8_SNORM                      = 0x0c9,
    R8G8B8A8_SINT                       = 0x0ca,
    R8G8B8A8_UINT                       = 0x0cb,
    R16G16_UNORM                        = 0x0cc,
    R16G16_SNORM                        = 0x0cd,
    R16G16_SINT                         = 0x0ce,
    R16G16_UINT                         = 0x0cf,
    R16G16_FLOAT                        = 0x0d0,
    B10G10R10A2_UNORM                   = 0x0d1,
    B10G10R10A2_UNORM_SRGB              = 0x0d2,
    R11G11B10_FLOAT                     = 0x0d3,
    R32_SINT                            = 0x0d6,
    R32_UINT                            = 0x0d7,
    R32_FLOAT                           = 0x0d8,
    R24_UNORM_X8_TYPELESS               = 0x0d9,
    X24_TYPELESS_G8_UINT                = 0x0da,
    //R24X8_UNORM                       = 0x0db,
    L16A16_UNORM                        = 0x0df,
    I24X8_UNORM                         = 0x0e0,
    L24X8_UNORM                         = 0x0e1,
    A24X8_UNORM                         = 0x0e2,
    I32_FLOAT                           = 0x0e3,
    L32_FLOAT                           = 0x0e4,
    A32_FLOAT                           = 0x0e5,
    B8G8R8X8_UNORM                      = 0x0e9,
    B8G8R8X8_UNORM_SRGB                 = 0x0ea,
    R8G8B8X8_UNORM                      = 0x0eb,
    R8G8B8X8_UNORM_SRGB                 = 0x0ec,
    R9G9B9E5_SHAREDEXP                  = 0x0ed,
    B10G10R10X2_UNORM                   = 0x0ee,
    L16A16_FLOAT                        = 0x0f0,
    R32_UNORM                           = 0x0f1, 
    R32_SNORM                           = 0x0f2, 
    //B8G8R8A8_SNORM                    = 0x0f3,
    R10G10B10X2_USCALED                 = 0x0f3,  // TODO: REV4 why duplicate? looks like B8G8R8_SNORM is gone
    R8G8B8A8_SSCALED                    = 0x0f4,
    R8G8B8A8_USCALED                    = 0x0f5,
    R16G16_SSCALED                      = 0x0f6,
    R16G16_USCALED                      = 0x0f7,
    R32_SSCALED                         = 0x0f8,
    R32_USCALED                         = 0x0f9,
    R8G8B8A8_UNORM_YUV                  = 0x0fd,
    R8G8B8A8_UNORM_SNCK                 = 0x0fe,
    R8G8B8A8_UNORM_NOA                  = 0x0ff,
    B5G6R5_UNORM                        = 0x100,
    B5G6R5_UNORM_SRGB                   = 0x101,
    B5G5R5A1_UNORM                      = 0x102,
    B5G5R5A1_UNORM_SRGB                 = 0x103,
    B4G4R4A4_UNORM                      = 0x104,
    B4G4R4A4_UNORM_SRGB                 = 0x105,
    R8G8_UNORM                          = 0x106,
    R8G8_SNORM                          = 0x107,
    R8G8_SINT                           = 0x108,
    R8G8_UINT                           = 0x109,
    R16_UNORM                           = 0x10a,
    R16_SNORM                           = 0x10b,
    R16_SINT                            = 0x10c,
    R16_UINT                            = 0x10d,
    R16_FLOAT                           = 0x10e,
    I16_UNORM                           = 0x111,
    L16_UNORM                           = 0x112,
    A16_UNORM                           = 0x113,
    L8A8_UNORM                          = 0x114,
    I16_FLOAT                           = 0x115,
    L16_FLOAT                           = 0x116,
    A16_FLOAT                           = 0x117,
    R5G5_SNORM_B6_UNORM                 = 0x119,
    B5G5R5X1_UNORM                      = 0x11a,
    B5G5R5X1_UNORM_SRGB                 = 0x11b,
    R8G8_SSCALED                        = 0x11c,
    R8G8_USCALED                        = 0x11d,
    R16_SSCALED                         = 0x11e,
    R16_USCALED                         = 0x11f,
    R8G8_SNORM_DX9                      = 0x120,
    R16_FLOAT_DX9                       = 0x121,
    R8_UNORM                            = 0x140,
    R8_SNORM                            = 0x141,
    R8_SINT                             = 0x142,
    R8_UINT                             = 0x143,
    A8_UNORM                            = 0x144,
    I8_UNORM                            = 0x145,
    L8_UNORM                            = 0x146,
    P4A4_UNORM                          = 0x147,
    A4P4_UNORM                          = 0x148,
    R8_SSCALED                          = 0x149,
    R8_USCALED                          = 0x14A,
    R1_UNORM                            = 0x181,
    R1_UINT                             = 0x181,
    YCRCB_NORMAL                        = 0x182,
    R8G8_B8G8_UNORM                     = 0x182,
    YCRCB_SWAPUVY                       = 0x183,
    G8R8_G8B8_UNORM                     = 0x183,
    BC1_UNORM_DXT1                      = 0x186,
    BC2_UNORM_DXT2_3                    = 0x187,
    BC3_UNORM_DXT4_5                    = 0x188,
    BC4_UNORM_DXN1                      = 0x189,
    BC5_UNORM_DXN2                      = 0x18a,
    BC1_UNORM_SRGB_DXT1_SRGB            = 0x18b,
    BC2_UNORM_SRGB_DXT2_3_SRGB          = 0x18c,
    BC3_UNORM_SRGB_DXT4_5_SRGB          = 0x18d,
    MONO8                               = 0x18e,
    YCRCB_SWAPUV                        = 0x18f,
    YCRCB_SWAPY                         = 0x190,
    DXT1_RGB                            = 0x191,
    FXT1                                = 0x192,
    R8G8B8_UNORM                        = 0x193,
    R8G8B8_SNORM                        = 0x194,
    //R8G8B8_UINT                       = 0x195,
    //R8G8B8_SINT                       = 0x196,
    R8G8B8_SSCALED                      = 0x195,
    R8G8B8_USCALED                      = 0x196,
    R64G64B64A64_FLOAT                  = 0x197,
    R64G64B64_FLOAT                     = 0x198,
    BC4_SNORM                           = 0x199,
    BC5_SNORM                           = 0x19a,
    //BC5_SNORM_NONLINEAR               = 0x19b,
    R16G16B16_FLOAT                     = 0x19b,
    R16G16B16_UNORM                     = 0x19c,
    R16G16B16_SNORM                     = 0x19d,
    R16G16B16_SSCALED                   = 0x19e,
    R16G16B16_USCALED                   = 0x19f,
    //R16G16B16_SINT                    = 0x19e, // REV4 remove
    //R16G16B16_UINT                    = 0x19f, // REV4 remove
    INVALID_SURF_FORMAT                 = 0xfff
} CmSurfaceFormatID;

typedef enum _CmBufferAttrib_
{
    GENX_TOP_FIELD              = 1,
    GENX_BOTTOM_FIELD           = 2,
    GENX_DWALIGNED              = 3,
    GENX_MODIFIED               = 4,
    GENX_MODIFIED_TOP_FIELD     = 5,
    GENX_MODIFIED_BOTTOM_FIELD  = 6,
    GENX_MODIFIED_DWALIGNED     = 7,
    GENX_CONSTANT               = 8,
    GENX_CONSTANT_DWALIGNED     = 9,
    GENX_NUM_BUFFER_ATTRIB      = 10
} CmBufferAttrib;

typedef enum _CmSurfacePlaneIndex_
{
    GENX_SURFACE_Y_PLANE     = 0,
    GENX_SURFACE_U_PLANE     = 1,
    GENX_SURFACE_UV_PLANE    = 1,
    GENX_SURFACE_V_PLANE     = 2
} CmSurfacePlaneIndex;

typedef enum _CM_READ_SIZE_
{//Bspec encoding.
    CM_READ_1 = 0,
    CM_READ_2 = 1,
    CM_READ_4 = 2,
    CM_READ_8 = 3
} CM_READ_SIZE;

/* Note: width and height for CM_register_buffer() in bytes and
 * for read()/write() in elements! */
typedef struct CM_REG_BUFF_PARM_S
{
    int esize;
    int length;
    int offset;

    union {
        int     d32bit;
        short   d16bit;
        char    d8bit;
        void *  dptr;
    } data;
}
CM_REG_BUFF_PARM_S;

#define CM_GLOBAL_COHERENT_FENCE 1
#define CM_L3_FLUSH_INSTRUCTIONS 2
#define CM_L3_FLUSH_TEXTURE_DATA 4
#define CM_L3_FLUSH_CONSTANT_DATA 8
#define CM_L3_FLUSH_RW_DATA 16
#define CM_COMMIT_ENABLE 32

CM_API extern void cm_fence();
CM_API extern void cm_fence(unsigned char bit_mask);

CM_API extern void cm_wait(uchar mask = 0);

CM_API extern void cm_sampler_cache_flush();

#ifndef SIMDCF_ELEMENT_SKIP
    #ifdef CM_EMU
    #ifdef CM_GENX
    #include "cm_internal_emu.h"    //using namespace __CMInternal__
    #define SIMDCF_ELEMENT_SKIP(i) \
            if (__CMInternal__::getWorkingStack() && !__CMInternal__::getWorkingStack()->isEmpty()) \
    			if ((short)(__CMInternal__::getSIMDMarker() << (i)) >= 0) \
    				continue; \
    
    #else
    #define SIMDCF_ELEMENT_SKIP(i)
    #endif
    #else
    #define SIMDCF_ELEMENT_SKIP(i)
    #endif
#endif

#ifdef CM_EMU

CM_API extern ushort get_thread_origin_x(void); 
CM_API extern ushort get_thread_origin_y(void);
CM_API extern ushort get_color(void);
CM_API extern void set_thread_origin_x(ushort); 
CM_API extern void set_thread_origin_y(ushort);
CM_API extern void set_color(ushort);
CM_API extern void initialize_global_surface_index();
CM_API extern void set_global_surface_index(int index, SurfaceIndex *si);
CM_API extern SurfaceIndex * get_global_surface_index(int index);

#endif

#ifndef NEW_CM_RT 
CM_API extern void CM_register_buffer(int buf_id, CmBufferType bclass, void *src, uint width);
CM_API extern void CM_register_buffer(int buf_id, CmBufferType bclass, void *src, uint width, uint height, 
                                      CmSurfaceFormatID surfFormat = R8G8B8A8_UINT, uint depth = 1, uint pitch = 0);
//CM_API extern void _CM_typed_register_buffer(int num, CM_REG_BUFF_PARM_S * dat);
CM_API extern void CM_unregister_buffer(int buf_id); /* Note: Cm spec 1.0 extension */
CM_API extern void CM_modify_buffer(int buf_id, CmBufferDescField field,
                                    int value);
CM_API extern void CM_modify_buffer_emu(SurfaceIndex buf_id, CmBufferDescField field, int value);
#else
CM_API extern void CM_register_buffer(SurfaceIndex buf_id, CmBufferType bclass, void *src, uint width);
CM_API extern void CM_register_buffer(SurfaceIndex buf_id, CmBufferType bclass, void *src, uint width, uint height, 
                                      CmSurfaceFormatID surfFormat = R8G8B8A8_UINT, uint depth = 1, uint pitch = 0);
//CM_API extern void _CM_typed_register_buffer(SurfaceIndex num, CM_REG_BUFF_PARM_S * dat);
CM_API extern void CM_unregister_buffer(SurfaceIndex buf_id, bool copy=true); /* Note: Cm spec 1.0 extension */
CM_API extern void CM_modify_buffer(SurfaceIndex buf_id, CmBufferDescField field,
                                    int value);
CM_API extern void CM_modify_buffer_emu(SurfaceIndex buf_id, CmBufferDescField field, int value);
#endif

#define MODIFIED(A)                A, GENX_MODIFIED 
#define TOP_FIELD(A)               A, GENX_TOP_FIELD
#define BOTTOM_FIELD(A)            A, GENX_BOTTOM_FIELD
#define MODIFIED_TOP_FIELD(A)      A, GENX_MODIFIED_TOP_FIELD
#define MODIFIED_BOTTOM_FIELD(A)   A, GENX_MODIFIED_BOTTOM_FIELD
#define DWALIGNED(A)               A, GENX_DWALIGNED 
#define MODIFIED_DWALIGNED(A)      A, GENX_MODIFIED_DWALIGNED
#define CONSTANT(A)                A, GENX_CONSTANT
#define CONSTANT_DWALIGNED(A)      A, GENX_CONSTANT_DWALIGNED


#ifndef CM_EMU

extern bool workarroundForIpoBugOrFeature;

namespace CmSys{
struct iobuffer {
    int id;
    void *buf_desc;    /* Buffer descriptor */
}; 
extern cm_list<iobuffer> iobuffers;
extern cm_list<CmSys::iobuffer>::iterator search_buffer(int id);
}

#ifndef NEW_CM_RT

template <typename T, uint R, uint C>
CM_API extern bool
read(uint buf_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read(uint buf_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write(uint buf_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write(uint buf_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

#if 0 

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(uint buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id + plane_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(uint buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id + plane_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(uint buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id + plane_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(uint buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id + plane_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(uint buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id + plane_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(uint buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id + plane_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(uint buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id + plane_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(uint buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id + plane_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id + plane_id < 0xffff);
    }
}

#endif

template <typename T, uint S>
CM_API extern bool
read(uint buf_id, int offset, vector<T, S> &in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, offset, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
read(uint buf_id, int offset, vector_ref<T, S> in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, offset, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
read(uint buf_id, CmBufferAttrib buf_attrib, int offset, vector<T, S> &in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, offset, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
read(uint buf_id, CmBufferAttrib buf_attrib, int offset, vector_ref<T, S> in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, offset, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
write(uint buf_id, int offset, const vector<T, S> &out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, offset, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
write(uint buf_id, int offset, const vector_ref<T, S> out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, offset, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
write(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
write(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
write(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
write(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, int v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, int v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, int v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, int v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
}

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
} 

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
} 

CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, int v)
{
    // static const bool conformable1 = dwordtype<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
} 

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
} 

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
} 

CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, int v)
{
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id < 0xffff);
    }
} 

#else

#define CHECK_MEDIA_TRANSACTION(T,R,C) CM_STATIC_WARNING(sizeof(T)*C*R <= 256,\
                      "matrix size exceeds the maximum dataport transaction size of 256 bytes");\
    CM_STATIC_WARNING(sizeof(T)*C*R > 256 || sizeof(T)*C <= 64,\
                      "matrix width exceeds the maximum dataport transaction width of 64 bytes");\
    CM_STATIC_WARNING(sizeof(T)*C*R > 256 || !(sizeof(T)*C <= 4 && R > 64),\
                      "a maximum of 64 rows is supported for widths of 1-4 bytes");\
    CM_STATIC_WARNING(sizeof(T)*C*R > 256 || !(sizeof(T)*C > 4 && sizeof(T)*C <= 8 && R > 32),\
                      "a maximum of 32 rows is supported for widths of 5-8 bytes");\
    CM_STATIC_WARNING(sizeof(T)*C*R > 256 || !(sizeof(T)*C > 8 && sizeof(T)*C <= 16 && R > 16),\
                      "a maximum of 16 rows is supported for widths of 9-16 bytes");\
    CM_STATIC_WARNING(sizeof(T)*C*R > 256 || !(sizeof(T)*C > 16 && sizeof(T)*C <= 32 && R > 8),\
                      "a maximum of 8 rows is supported for widths of 17-32 bytes");\
    CM_STATIC_WARNING(sizeof(T)*C*R > 256 || !(sizeof(T)*C > 32 && sizeof(T)*C <= 64 && R > 4),\
                      "a maximum of 4 rows is supported for widths of 33-64 bytes");

template <typename T, uint R, uint C>
CM_API extern bool
read(SurfaceIndex & buf_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read(SurfaceIndex & buf_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write(SurfaceIndex & buf_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write(SurfaceIndex & buf_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id + plane_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id + plane_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id + plane_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id + plane_id, x_pos, y_pos, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id + plane_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id + plane_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id + plane_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() + plane_id < 0xffff);
    }
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    CHECK_MEDIA_TRANSACTION(T,R,C);
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id + plane_id, x_pos, y_pos, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() + plane_id < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
read(SurfaceIndex & buf_id, int offset, vector<T, S> &in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, offset, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
read(SurfaceIndex & buf_id, int offset, vector_ref<T, S> in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, offset, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int offset, vector<T, S> &in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, offset, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int offset, vector_ref<T, S> in)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, offset, in);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
write(SurfaceIndex & buf_id, int offset, const vector<T, S> &out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, offset, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint S>
CM_API extern bool
write(SurfaceIndex & buf_id, int offset, const vector_ref<T, S> out)
{
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, offset, out);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return read(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
write(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
write(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
write(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool 
write(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> v) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, global_offset, element_offset, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, int v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, int v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, int v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, int v)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, int v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write<T>(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, int v)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return write<T>(buf_id, op, global_offset, element_offset, src, v);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
} 

template <typename T, uint R, uint C>
CM_API extern bool
read_transpose(SurfaceIndex & buf_id, CM_READ_SIZE height, CM_READ_SIZE width, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}
template <typename T, uint R, uint C>
CM_API extern bool
read_transpose(SurfaceIndex & buf_id, CM_READ_SIZE height, CM_READ_SIZE width, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    static const bool conformable1 = dwordtype<T>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (buf_id.get_data() < 0xffff);
    }
}

/* Typed surface read */
template <typename RT, uint N1, uint N2>
CM_API extern bool 
read_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix<RT, N1, N2> &m, 
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
}

template <typename RT, uint N1, uint N2>
CM_API extern bool 
read_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix_ref<RT, N1, N2> &m, 
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
}

#if defined(__CLANG_CM)
template <typename RT, uint N2>
CM_API extern bool 
read_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     vector_ref<RT, N2> m, 
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
}
#endif

/* Typed surface write */
template <typename RT, uint N1, uint N2>
CM_API extern bool 
write_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix<RT, N1, N2> &m, 
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
}

template <typename RT, uint N1, uint N2>
CM_API extern bool 
write_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix_ref<RT, N1, N2> &m, 
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
}

/* Untyped surface read */
template <typename RT, uint N1, uint N2>
CM_API extern bool
read_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix<RT, N1, N2> &m, 
     const vector<uint, N2> &u) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
}

template <typename RT, uint N1, uint N2>
CM_API extern bool
read_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix_ref<RT, N1, N2> &m, 
     const vector<uint, N2> &u) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
}

/* Untyped surface write */
template <typename RT, uint N1, uint N2>
CM_API extern bool
write_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix<RT, N1, N2> &m, 
     const vector<uint, N2> &u) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
}

template <typename RT, uint N1, uint N2>
CM_API extern bool
write_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix_ref<RT, N1, N2> &m, 
     const vector<uint, N2> &u) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;
    if (workarroundForIpoBugOrFeature) {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
    else {
        printf("workarroundForIpoBugOrFeature\n");
        return (surfIndex.get_data() < 0xffff);
    }
}

#endif

#else /* CM_EMU */

namespace CmEmulSys{
struct iobuffer {
    int id;
    CmBufferType bclass;
    CmSurfaceFormatID pixelFormat;
    void *p;    /* i/o datas */
	void *p_volatile;
    int width;
    int height;
    int depth;
    int pitch;
}; 
CMRT_LIBCM_API extern cm_list<iobuffer> iobuffers;
CM_API extern cm_list<CmEmulSys::iobuffer>::iterator search_buffer(int id);
extern cm_list<CmEmulSys::iobuffer>::iterator search_buffer(void *src, CmBufferType bclass);
}

/*
 * Returns bytes per pixel value for specified surface format ID.
 */
inline uint CM_genx_bytes_per_pixel(CmSurfaceFormatID pixelFormat)
{
    switch(pixelFormat >> 6) {
        case 0:
            return 16;
        case 1:
            return 12;
        case 2:
            return 8;
        case 3:
            return 4;
        case 4:
            return 2;
        case 5:
            return 1;
        default:
            if( pixelFormat == YCRCB_NORMAL ||
                pixelFormat == YCRCB_SWAPY)
            {
                return 2;
            }
            return 4;
    }
}

#ifndef NEW_CM_RT

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
read(uint buf_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

#if 0
    if((x_pos % 4) != 0) {
        printf("Error reading buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
#endif

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }
#if 0
            while (x_pos_a > 0 && ((x_pos_a & (~(bpp-1))) + bpp) > width) {
                x_pos_a -= bpp;
            }
#else
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
#endif
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative
            
            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                /*
                According to bspec reads need to be DWORD (two pixel) allgined.
                */

                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;                

                /*
                Bspec goodness:
                For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                */
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
				if (x_pos_a < 0) {
					// Need to align x position to bbp
					int offset = x_pos % bpp;
					x_pos_a -= offset;
				}
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            } 

            if (x_pos_a >= width) {
#if 0
                // Right boundary returns 0 like Fulsim
                in(i,j) = 0;
#else

                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    /*
                    According to bspec reads need to be DWORD (two pixel) allgined.
                    */

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    /*
                        Bspec goodness:
                        For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                        For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                    */
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)buff_iter->p + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)buff_iter->p + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
#endif
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
read(uint buf_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

#if 0
    if((x_pos % 4) != 0) {
        printf("Error reading buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
#endif

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }
#if 0
            while (x_pos_a > 0 && ((x_pos_a & (~(bpp-1))) + bpp) > width) {
                x_pos_a -= bpp;
            }
#else
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
#endif
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative

            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                /*
                According to bspec reads need to be DWORD (two pixel) allgined.
                */

                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;                

                /*
                Bspec goodness:
                For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                */
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
				if (x_pos_a < 0) {
					// Need to align x position to bbp
					int offset = x_pos % bpp;
					x_pos_a -= offset;
				}
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            } 

            if (x_pos_a >= width) {
#if 0
                // Right boundary returns 0 like Fulsim
                in(i,j) = 0;
#else

                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    /*
                    According to bspec reads need to be DWORD (two pixel) allgined.
                    */

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    /*
                        Bspec goodness:
                        For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                        For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                    */
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)buff_iter->p + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)buff_iter->p + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
#endif
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

template <typename T, uint R, uint C>
CM_API extern bool
read(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib < GENX_TOP_FIELD) || (buf_attrib > GENX_MODIFIED_BOTTOM_FIELD)) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

#if 0
    if((x_pos % 4) != 0) {
        printf("Error reading buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
#endif

    if(buf_attrib >= GENX_MODIFIED) {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if((buf_attrib == GENX_BOTTOM_FIELD) || (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD)) {
        y_pos++;
    }

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if((buf_attrib == GENX_TOP_FIELD) || 
               (buf_attrib == GENX_BOTTOM_FIELD) || 
               (buf_attrib == GENX_MODIFIED_TOP_FIELD) ||
               (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD)) {
                y_pos_a = y_pos + i*2;
            } else {
                y_pos_a = y_pos + i;
            }
#if 0
            while (x_pos_a > 0 && ((x_pos_a & (~(bpp-1))) + bpp) > width) {
                x_pos_a -= bpp;
            }
#else
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
#endif
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative
            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                /*
                According to bspec reads need to be DWORD (two pixel) allgined.
                */

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;                

                /*
                Bspec goodness:
                For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                */
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
				if (x_pos_a < 0) {
					// Need to align x position to bbp
					int offset = x_pos % bpp;
					x_pos_a -= offset;
				}
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            } 

            if (x_pos_a >= width) {
#if 0
                // Right boundary returns 0 like Fulsim
                in(i,j) = 0;
#else

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    /*
                    According to bspec reads need to be DWORD (two pixel) allgined.
                    */

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    /*
                        Bspec goodness:
                        For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                        For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                    */
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)temp_buffer_pointer + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)temp_buffer_pointer + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
#endif
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                if(buf_attrib >= GENX_MODIFIED) {
                    in(i,j) = *((T*)((char*)buff_iter->p_volatile + offset));
                } else {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
read(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib < GENX_TOP_FIELD) || (buf_attrib > GENX_MODIFIED_BOTTOM_FIELD)) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

#if 0
    if((x_pos % 4) != 0) {
        printf("Error reading buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
#endif

    if(buf_attrib >= GENX_MODIFIED) {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if((buf_attrib == GENX_BOTTOM_FIELD) || (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD)) {
        y_pos++;
    }

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if((buf_attrib == GENX_TOP_FIELD) || 
               (buf_attrib == GENX_BOTTOM_FIELD) || 
               (buf_attrib == GENX_MODIFIED_TOP_FIELD) ||
               (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD)) {
                y_pos_a = y_pos + i*2;
            } else {
                y_pos_a = y_pos + i;
            }
#if 0
            while (x_pos_a > 0 && ((x_pos_a & (~(bpp-1))) + bpp) > width) {
                x_pos_a -= bpp;
            }
#else
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
#endif
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative
            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                /*
                According to bspec reads need to be DWORD (two pixel) allgined.
                */

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;                

                /*
                Bspec goodness:
                For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                */
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
				if (x_pos_a < 0) {
					// Need to align x position to bbp
					int offset = x_pos % bpp;
					x_pos_a -= offset;
				}
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            } 

            if (x_pos_a >= width) {
#if 0
                // Right boundary returns 0 like Fulsim
                in(i,j) = 0;
#else

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    /*
                    According to bspec reads need to be DWORD (two pixel) allgined.
                    */

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    /*
                        Bspec goodness:
                        For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                        For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                    */
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)temp_buffer_pointer + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)temp_buffer_pointer + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
#endif
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                if(buf_attrib >= GENX_MODIFIED) {
                    in(i,j) = *((T*)((char*)buff_iter->p_volatile + offset));
                } else {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
write(uint buf_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            {
                y_pos_a = y_pos + i;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
write(uint buf_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            {
                y_pos_a = y_pos + i;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
write(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib != GENX_TOP_FIELD) && (buf_attrib != GENX_BOTTOM_FIELD)) {
        printf("Error writing buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if(buf_attrib == GENX_BOTTOM_FIELD) {
        y_pos++;
    }

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            if((buf_attrib == GENX_TOP_FIELD) || 
               (buf_attrib == GENX_BOTTOM_FIELD)) {
                y_pos_a = y_pos + i*2;
            } else {
                y_pos_a = y_pos + i;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
write(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib != GENX_TOP_FIELD) && (buf_attrib != GENX_BOTTOM_FIELD)) {
        printf("Error writing buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if(buf_attrib == GENX_BOTTOM_FIELD) {
        y_pos++;
    }

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            if((buf_attrib == GENX_TOP_FIELD) || 
               (buf_attrib == GENX_BOTTOM_FIELD)) {
                y_pos_a = y_pos + i*2;
            } else {
                y_pos_a = y_pos + i;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

#if 0

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(uint buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(uint buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in) 
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(uint buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(uint buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(uint buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, x_pos, y_pos, out);
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(uint buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, x_pos, y_pos, out);
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(uint buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, out);
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(uint buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, out);
}

#endif

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API extern bool
read(uint buf_id, int offset, vector<T,S> &in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API extern bool
read(uint buf_id, int offset, vector_ref<T,S> in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API extern bool
read(uint buf_id, CmBufferAttrib buf_attrib, int offset, vector<T,S> &in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_DWALIGNED &&
       buf_attrib != GENX_MODIFIED_DWALIGNED && buf_attrib != GENX_CONSTANT &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib != GENX_DWALIGNED && buf_attrib != GENX_MODIFIED_DWALIGNED &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        if((offset % 16) != 0) {
            printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    else {
        if((offset % 4) != 0) {
            printf("Error reading buffer %d: offset must be 4-byte aligned!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    
    assert(height == 1);

    char* buff;

    if(buf_attrib == GENX_MODIFIED || buf_attrib == GENX_MODIFIED_DWALIGNED)
    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }

        buff = (char*) buff_iter->p_volatile;
    }
    else
    {
        buff = (char*) buff_iter->p;
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)(buff + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API extern bool
read(uint buf_id, CmBufferAttrib buf_attrib, int offset, vector_ref<T,S> in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_DWALIGNED &&
       buf_attrib != GENX_MODIFIED_DWALIGNED && buf_attrib != GENX_CONSTANT &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib != GENX_DWALIGNED && buf_attrib != GENX_MODIFIED_DWALIGNED &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        if((offset % 16) != 0) {
            printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    else {
        if((offset % 4) != 0) {
            printf("Error reading buffer %d: offset must be 4-byte aligned!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    
    assert(height == 1);

    char* buff;

    if(buf_attrib == GENX_MODIFIED || buf_attrib == GENX_MODIFIED_DWALIGNED)
    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
        buff = (char*) buff_iter->p_volatile;
    }
    else
    {
        buff = (char*) buff_iter->p;
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)(buff + pos));
            }
        }
    }

    return true;
}

/* This funtion writes OWord Block data to genx dataport */
template <typename T, uint S>
CM_API extern bool
write(uint buf_id, int offset, const vector<T,S> &out)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error writing buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((S * sizeofT) % 16) != 0) {
        printf("Error writing buffer %d: input vector size must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion writes OWord Block data to genx dataport */
template <typename T, uint S>
CM_API extern bool
write(uint buf_id, int offset, const vector_ref<T, S> out)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error writing buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((S * sizeofT) % 16) != 0) {
        printf("Error writing buffer %d: input vector size must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion reads scattered DWords from genx dataport */
template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p + pos));
			}
		}
    }

    return true;
}

/* This funtion reads scattered DWords from genx dataport */
template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
			}
		}
    }

    return true;
}

/* This funtion writes scattered DWords to genx dataport */
template <typename T, uint N>
CM_API extern bool 
write(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &out) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			break;
		}
		if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
			*((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
		} else {
			*((T*)( (char*)buff_iter->p + pos )) = out(i);
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
write(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> out) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*4;
		if (pos >= width*height) {
			break;
		}
		if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
			*((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
		} else {
			*((T*)( (char*)buff_iter->p + pos )) = out(i);
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
write(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &out) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
write(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> out) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion performs atomic scattered DWord write to genx dataport */
template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + src(i);
                    break;
                case ATOMIC_SUB:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - src(i);                 
                    break;
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;                 
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;                 
                    break;
                case ATOMIC_MIN:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((*((T*)((char*)buff_iter->p_volatile + pos)) == src(i+8))? src(i): *((T*)((char*)buff_iter->p_volatile + pos)));                 
                    break;
                case ATOMIC_AND:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) & src(i);
                    break;
                case ATOMIC_OR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) | src(i);
                    break;
                case ATOMIC_XOR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + src(i);
                    break;
                case ATOMIC_SUB:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - src(i);                 
                    break;
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;                 
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;                 
                    break;
                case ATOMIC_MIN:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) <= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) >= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((*((T*)((char*)buff_iter->p_volatile + pos)) == src(i+8))? src(i): *((T*)((char*)buff_iter->p_volatile + pos)));                 
                    break;
                case ATOMIC_AND:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) & src(i);
                    break;
                case ATOMIC_OR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) | src(i);
                    break;
                case ATOMIC_XOR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + src(i);
                    break;
                case ATOMIC_SUB:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - src(i);                 
                    break;
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;                 
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;                 
                    break;
                case ATOMIC_MIN:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) <= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) >= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((*((T*)((char*)buff_iter->p_volatile + pos)) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) & src(i);
                    break;
                case ATOMIC_OR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) | src(i);
                    break;
                case ATOMIC_XOR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + src(i);
                    break;
                case ATOMIC_SUB:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - src(i);                 
                    break;
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;                 
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;                 
                    break;
                case ATOMIC_MIN:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) <= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) >= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((*((T*)((char*)buff_iter->p_volatile + pos)) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) & src(i);
                    break;
                case ATOMIC_OR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) | src(i);
                    break;
                case ATOMIC_XOR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;                 
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;                 
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

#else

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
read(SurfaceIndex & buf_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    uint i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

#if 0
    if((x_pos % 4) != 0) {
        printf("Error reading buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
#endif

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }
#if 0
            while (x_pos_a > 0 && ((x_pos_a & (~(bpp-1))) + bpp) > width) {
                x_pos_a -= bpp;
            }
#else
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
#endif
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }

            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                /*
                According to bspec reads need to be DWORD (two pixel) allgined.
                */

                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width [%d]for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;                

                /*
                Bspec goodness:
                For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                */
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
				if (x_pos_a < 0) {
					// Need to align x position to bbp
					int offset = x_pos % bpp;
					x_pos_a -= offset;
				}
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            }    

            if (x_pos_a >= width) {
#if 0
                // Right boundary returns 0 like Fulsim
                in(i,j) = 0;
#else

                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    /*
                    According to bspec reads need to be DWORD (two pixel) allgined.
                    */

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width [%d] for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    /*
                        Bspec goodness:
                        For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                        For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                    */
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {
                    x_pos_a = x_pos_a - bpp;
                    for( uint byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(uint bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)buff_iter->p + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)buff_iter->p + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
#endif
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
read(SurfaceIndex & buf_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

#if 0
    if((x_pos % 4) != 0) {
        printf("Error reading buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
#endif

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }
#if 0
            while (x_pos_a > 0 && ((x_pos_a & (~(bpp-1))) + bpp) > width) {
                x_pos_a -= bpp;
            }
#else
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
#endif
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }

            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                /*
                According to bspec reads need to be DWORD (two pixel) allgined.
                */

                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;                

                /*
                Bspec goodness:
                For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                */
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
				if (x_pos_a < 0) {
					// Need to align x position to bbp
					int offset = x_pos % bpp;
					x_pos_a -= offset;
				}
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            }            

            if (x_pos_a >= width) {
#if 0
                // Right boundary returns 0 like Fulsim
                in(i,j) = 0;
#else

                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    /*
                    According to bspec reads need to be DWORD (two pixel) allgined.
                    */

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    /*
                        Bspec goodness:
                        For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                        For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                    */
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)buff_iter->p + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)buff_iter->p + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
#endif
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

template <typename T, uint R, uint C>
CM_API extern bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib < GENX_TOP_FIELD) || (buf_attrib > GENX_MODIFIED_BOTTOM_FIELD)) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

#if 0
    if((x_pos % 4) != 0) {
        printf("Error reading buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
#endif

    if(buf_attrib >= GENX_MODIFIED) {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if((buf_attrib == GENX_BOTTOM_FIELD) || (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD)) {
        y_pos++;
    }

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if((buf_attrib == GENX_TOP_FIELD) || 
               (buf_attrib == GENX_BOTTOM_FIELD) || 
               (buf_attrib == GENX_MODIFIED_TOP_FIELD) ||
               (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD)) {
                y_pos_a = y_pos + i*2;
            } else {
                y_pos_a = y_pos + i;
            }
#if 0
            while (x_pos_a > 0 && ((x_pos_a & (~(bpp-1))) + bpp) > width) {
                x_pos_a -= bpp;
            }
#else
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
#endif
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative
            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                /*
                According to bspec reads need to be DWORD (two pixel) allgined.
                */

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;                

                /*
                Bspec goodness:
                For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                */
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
				if (x_pos_a < 0) {
					// Need to align x position to bbp
					int offset = x_pos % bpp;
					x_pos_a -= offset;
				}
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            } 

            if (x_pos_a >= width) {
#if 0
                // Right boundary returns 0 like Fulsim
                in(i,j) = 0;
#else

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    /*
                    According to bspec reads need to be DWORD (two pixel) allgined.
                    */

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    /*
                        Bspec goodness:
                        For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                        For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                    */
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)temp_buffer_pointer + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)temp_buffer_pointer + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
#endif
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                if(buf_attrib >= GENX_MODIFIED) {
                    in(i,j) = *((T*)((char*)buff_iter->p_volatile + offset));
                } else {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib < GENX_TOP_FIELD) || (buf_attrib > GENX_MODIFIED_BOTTOM_FIELD)) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

#if 0
    if((x_pos % 4) != 0) {
        printf("Error reading buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
#endif

    if(buf_attrib >= GENX_MODIFIED) {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if((buf_attrib == GENX_BOTTOM_FIELD) || (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD)) {
        y_pos++;
    }

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if((buf_attrib == GENX_TOP_FIELD) || 
               (buf_attrib == GENX_BOTTOM_FIELD) || 
               (buf_attrib == GENX_MODIFIED_TOP_FIELD) ||
               (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD)) {
                y_pos_a = y_pos + i*2;
            } else {
                y_pos_a = y_pos + i;
            }
#if 0
            while (x_pos_a > 0 && ((x_pos_a & (~(bpp-1))) + bpp) > width) {
                x_pos_a -= bpp;
            }
#else
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
#endif
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative
            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                /*
                According to bspec reads need to be DWORD (two pixel) allgined.
                */

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;                

                /*
                Bspec goodness:
                For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                */
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
				if (x_pos_a < 0) {
					// Need to align x position to bbp
					int offset = x_pos % bpp;
					x_pos_a -= offset;
				}
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            } 

            if (x_pos_a >= width) {
#if 0
                // Right boundary returns 0 like Fulsim
                in(i,j) = 0;
#else

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    /*
                    According to bspec reads need to be DWORD (two pixel) allgined.
                    */

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    /*
                        Bspec goodness:
                        For a boundary dword Y0U0Y1V0, to replicate the left boundary, we get Y0U0Y0V0, and to replicate the right boundary, we get Y1U0Y1V0.
                        For a boundary dword U0Y0V0Y1, to replicate the left boundary, we get U0Y0V0Y0, and to replicate the right boundary, we get U0Y1V0Y1.
                    */
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)temp_buffer_pointer + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)temp_buffer_pointer + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
#endif
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                if(buf_attrib >= GENX_MODIFIED) {
                    in(i,j) = *((T*)((char*)buff_iter->p_volatile + offset));
                } else {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
write(SurfaceIndex & buf_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    uint i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            {
                y_pos_a = y_pos + i;
            }
			if ((int)x_pos_a < 0) {
                continue;
            }
            if ((int)y_pos_a < 0) {
                continue;
            }
            if ((int)(x_pos_a + sizeofT) > width) {
                continue;
            }
            if ((int)y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
write(SurfaceIndex & buf_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            {
                y_pos_a = y_pos + i;
            }
			if ((int)x_pos_a < 0) {
                continue;
            }
            if ((int)y_pos_a < 0) {
                continue;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
write(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib != GENX_TOP_FIELD) && (buf_attrib != GENX_BOTTOM_FIELD)) {
        printf("Error writing buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if(buf_attrib == GENX_BOTTOM_FIELD) {
        y_pos++;
    }

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            if((buf_attrib == GENX_TOP_FIELD) || 
               (buf_attrib == GENX_BOTTOM_FIELD)) {
                y_pos_a = y_pos + i*2;
            } else {
                y_pos_a = y_pos + i;
            }
			if ((int)x_pos_a < 0) {
                continue;
            }
            if ((int)y_pos_a < 0) {
                continue;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
write(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib != GENX_TOP_FIELD) && (buf_attrib != GENX_BOTTOM_FIELD)) {
        printf("Error writing buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if(buf_attrib == GENX_BOTTOM_FIELD) {
        y_pos++;
    }

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            if((buf_attrib == GENX_TOP_FIELD) || 
               (buf_attrib == GENX_BOTTOM_FIELD)) {
                y_pos_a = y_pos + i*2;
            } else {
                y_pos_a = y_pos + i;
            }
			if ((int)x_pos_a < 0) {
                continue;
            }
            if ((int)y_pos_a < 0) {
                continue;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in) 
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API extern bool
read_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, x_pos, y_pos, out);
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, x_pos, y_pos, out);
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, out);
}

template <typename T, uint R, uint C>
CM_API extern bool
write_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, out);
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API extern bool
read(SurfaceIndex & buf_id, int offset, vector<T,S> &in)
{
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API extern bool
read(SurfaceIndex & buf_id, int offset, vector_ref<T,S> in)
{
    uint i;
    int pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API extern bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int offset, vector<T,S> &in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
        CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_DWALIGNED &&
       buf_attrib != GENX_MODIFIED_DWALIGNED && buf_attrib != GENX_CONSTANT &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib != GENX_DWALIGNED && buf_attrib != GENX_MODIFIED_DWALIGNED &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        if((offset % 16) != 0) {
            printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    else {
        if((offset % 4) != 0) {
            printf("Error reading buffer %d: offset must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    
    assert(height == 1);

    char* buff;

    if(buf_attrib == GENX_MODIFIED || buf_attrib == GENX_MODIFIED_DWALIGNED)
    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }

        buff = (char*) buff_iter->p_volatile;
    }
    else
    {
        buff = (char*) buff_iter->p;
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)(buff + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API extern bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int offset, vector_ref<T,S> in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_DWALIGNED &&
       buf_attrib != GENX_MODIFIED_DWALIGNED && buf_attrib != GENX_CONSTANT &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib != GENX_DWALIGNED && buf_attrib != GENX_MODIFIED_DWALIGNED &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        if((offset % 16) != 0) {
            printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    else {
        if((offset % 4) != 0) {
            printf("Error reading buffer %d: offset must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    
    assert(height == 1);

    char* buff;

    if(buf_attrib == GENX_MODIFIED || buf_attrib == GENX_MODIFIED_DWALIGNED)
    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
        buff = (char*) buff_iter->p_volatile;
    }
    else
    {
        buff = (char*) buff_iter->p;
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)(buff + pos));
            }
        }
    }

    return true;
}

/* This funtion writes OWord Block data to genx dataport */
template <typename T, uint S>
CM_API extern bool
write(SurfaceIndex & buf_id, int offset, const vector<T,S> &out)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error writing buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((S * sizeofT) % 16) != 0) {
        printf("Error writing buffer %d: input vector size must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion writes OWord Block data to genx dataport */
template <typename T, uint S>
CM_API extern bool
write(SurfaceIndex & buf_id, int offset, const vector_ref<T, S> out)
{
    uint i;
    int pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error writing buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((S * sizeofT) % 16) != 0) {
        printf("Error writing buffer %d: input vector size must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion reads scattered DWords from genx dataport */
template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
	for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p + pos));
			}
		}
    }

    return true;
}

/* This funtion reads scattered DWords from genx dataport */
template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
			}
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			in(i) = width*height - 1;
		} else {
			{
				in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
			}
		}
    }

    return true;
}

/* This funtion writes scattered DWords to genx dataport */
template <typename T, uint N>
CM_API extern bool 
write(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &out) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			continue;
		}
		if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
			*((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
		} else {
			*((T*)( (char*)buff_iter->p + pos )) = out(i);
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
write(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> out) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    int pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			continue;
		}
		if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
			*((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
		} else {
			*((T*)( (char*)buff_iter->p + pos )) = out(i);
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
write(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &out) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
	for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			continue;
		}
		if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
			*((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
		} else {
			*((T*)( (char*)buff_iter->p + pos )) = out(i);
		}
    }

    return true;
}

template <typename T, uint N>
CM_API extern bool 
write(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> out) 
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
		SIMDCF_ELEMENT_SKIP(i);
		pos = (global_offset + element_offset(i))*sizeOfT;
		if (pos >= width*height) {
			continue;
		}
		if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
			*((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
		} else {
			*((T*)( (char*)buff_iter->p + pos )) = out(i);
		}
    }

    return true;
}

/* This funtion performs atomic scattered DWord write to genx dataport */
template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, int not_used)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + src(i);
                    break;
                case ATOMIC_SUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - src(i);                 
                    break;
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;                 
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;                 
                    break;
                case ATOMIC_MIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v <= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v >= (uint) src(i))? v: src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i+8))? src(i): v);                 
                    break;
                case ATOMIC_AND:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v & src(i);
                    break;
                case ATOMIC_OR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v | src(i);
                    break;
                case ATOMIC_XOR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v <= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v >= (int) src(i))? v: src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, int not_used)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + src(i);
                    break;
                case ATOMIC_SUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - src(i);                 
                    break;
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;                 
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;                 
                    break;
                case ATOMIC_MIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v <= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v >= (uint) src(i))? v: src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i+8))? src(i): v);                 
                    break;
                case ATOMIC_AND:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v & src(i);
                    break;
                case ATOMIC_OR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v | src(i);
                    break;
                case ATOMIC_XOR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v <= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v >= (int) src(i))? v: src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, int not_used)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + src(i);
                    break;
                case ATOMIC_SUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - src(i);                 
                    break;
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;                 
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;                 
                    break;
                case ATOMIC_MIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v <= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v >= (uint) src(i))? v: src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i+8))? src(i): v);                 
                    break;
                case ATOMIC_AND:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v & src(i);
                    break;
                case ATOMIC_OR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v | src(i);
                    break;
                case ATOMIC_XOR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v <= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v >= (int) src(i))? v: src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);                 
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));                 
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, int not_used)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + src(i);
                    break;
                case ATOMIC_SUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - src(i);                 
                    break;
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;                 
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;                 
                    break;
                case ATOMIC_MIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v <= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v >= (uint) src(i))? v: src(i)); 
                    }   
                    break;
                case ATOMIC_XCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);                 
                    break;
                case ATOMIC_CMPXCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i+8))? src(i): v);                 
                    break;
                case ATOMIC_AND:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v & src(i);
                    break;
                case ATOMIC_OR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v | src(i);
                    break;
                case ATOMIC_XOR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v <= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v >= (int) src(i))? v: src(i));   
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, int not_used)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;                 
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;                 
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

template <typename T>
CM_API extern bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, int not_used)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos; 
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    T v;
    for (i = 0; i < 8; i++) {
		SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;                 
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;                 
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
} 

/* This funtion reads Block and does a transpose data from genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
read_transpose(SurfaceIndex & buf_id, CM_READ_SIZE region_height, CM_READ_SIZE region_width, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    int i,j;
    int block_width = 0;
    int block_height = 0;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    
    int width = buff_iter->width;
    int height = buff_iter->height;

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    switch( region_height)
    {
    case CM_READ_1:
        {
            block_height = 1;
            break;
        }
    case CM_READ_2:
        {
            block_height = 2;
            break;
        }
    case CM_READ_4:
        {
            block_height = 4;
            break;
        }
    case CM_READ_8:
        {
            block_height = 8;
            break;
        }
    default:
        {
            printf("Error reading buffer %d: Invalid Block Height!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    switch( region_width)
    {
    case CM_READ_1:
        {
            block_width = 1;
            break;
        }
    case CM_READ_2:
        {
            block_width = 2;
            break;
        }
    case CM_READ_4:
        {
            block_width = 4;
            break;
        }
    case CM_READ_8:
        {
            block_width = 8;
            break;
        }
    default:
        {
            printf("Error reading buffer %d: Invalid Block Width!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    /*  
        Calcultating actual offsets
        offsets are given in number of blocks
    */
    
    x_pos = x_pos * block_width * 4;
    y_pos = y_pos * block_height;
    if(C != block_height || R != block_width)
    {
        printf("Error reading buffer %d: Block dimensions are not equal to matrix dimensions!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if(sizeofT != 4)
    {
        printf("Error reading buffer %d: Invalid matrix format!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
    }
    for (i = 0; i < block_height; i++) {
        for (j = 0; j < block_width; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }

            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }

            if (y_pos_a > height - 1 ||
                x_pos_a < 0 ||
                y_pos_a < 0 ||
                x_pos_a >= width) {
                in(j,i) = 0;
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(j,i) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    

    return true;
}

/* This funtion reads Block and does a transpose data from genx dataport */
template <typename T, uint R, uint C>
CM_API extern bool
read_transpose(SurfaceIndex & buf_id, CM_READ_SIZE region_height, CM_READ_SIZE region_width, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    int i,j;
    int block_width = 0;
    int block_height = 0;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    
    int width = buff_iter->width;
    int height = buff_iter->height;

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) && 
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    switch( region_height)
    {
    case CM_READ_1:
        {
            block_height = 1;
            break;
        }
    case CM_READ_2:
        {
            block_height = 2;
            break;
        }
    case CM_READ_4:
        {
            block_height = 4;
            break;
        }
    case CM_READ_8:
        {
            block_height = 8;
            break;
        }
    default:
        {
            printf("Error reading buffer %d: Invalid Block Height!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    switch( region_width)
    {
    case CM_READ_1:
        {
            block_width = 1;
            break;
        }
    case CM_READ_2:
        {
            block_width = 2;
            break;
        }
    case CM_READ_4:
        {
            block_width = 4;
            break;
        }
    case CM_READ_8:
        {
            block_width = 8;
            break;
        }
    default:
        {
            printf("Error reading buffer %d: Invalid Block Width!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    /*  
        Calcultating actual offsets
        offsets are given in number of blocks
    */
    
    x_pos = x_pos * block_width * 4;
    y_pos = y_pos * block_height;
    if(C != block_height || R != block_width)
    {
        printf("Error reading buffer %d: Block dimensions are not equal to matrix dimensions!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if(sizeofT != 4)
    {
        printf("Error reading buffer %d: Invalid matrix format!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
    }
    for (i = 0; i < block_height; i++) {
        for (j = 0; j < block_width; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }

            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }

            if (y_pos_a > height - 1 ||
                x_pos_a < 0 ||
                y_pos_a < 0 ||
                x_pos_a >= width) {
                in(j,i) = 0;
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(j,i) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    

    return true;
}

/* Typed surface read */
template <typename RT, uint N1, uint N2>
CM_API extern bool 
read_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix_ref<RT, N1, N2> &m, 
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;

    uchar *baseOffset, *byteOffset;
    uchar numColors=0, color[4]={0,0,0,0}, colorNext=0;

    if (channelMask & 0x1) {color[0]=1; numColors++;}
    if ((channelMask >> 1) & 0x1) {color[1]=1; numColors++;}
    if ((channelMask >> 2) & 0x1) {color[2]=1; numColors++;}
    if ((channelMask >> 3) & 0x1) {color[3]=1; numColors++;}

    if (numColors == 0) {
        fprintf(stderr, "read_typed error: At least one"
                "destination channel has to be read.\n");
        exit(EXIT_FAILURE);
    }

    if (N1 < numColors) {
        fprintf(stderr, "read_typed error: destination matrix"
                "does not have enough space to hold data.\n");
        exit(EXIT_FAILURE);
    }

    if (N2 != 8 && N2 != 16) {
        fprintf(stderr, "read_typed error: offset vector size"
                "must be 8 or 16.\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "Error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    uint width, height, depth, pitch, data_size;
    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;

    // FIXME: add support for more surface formats
    if ((surfFormat == R32_SINT) ||
        (surfFormat == R32_UINT) ||
        (surfFormat == R32_FLOAT)) {

        if(channelMask != CM_R_ENABLE) {
            fprintf(stderr, "read_typed error: only CM_R_ENABLE is supported for R32_SINT/R32_UINT/R32_FLOAT surface format.\n");
            exit(EXIT_FAILURE);
        }

        data_size = 4;
    } else if (surfFormat == R8G8B8A8_UINT) {
        data_size = 1;
    } else {
        fprintf(stderr, "read_typed error: only R32_SINT/R32_UINT/R32_FLOAT/R8G8B8A8_UINT surface formats are supported.\n");
        exit(EXIT_FAILURE);
    }

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if ((height == 1) && (depth == 1)) {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + data_size * channel);
                if(data_size*(u(i) + channel) >= width) {
                    m(colorNext, i) = 0;
                } else {
                    // FIXME: add support for different surface formats
                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (unsigned char *)byteOffset ), N2, i);
                    } else {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (RT *)byteOffset ), N2, i);
                    }
                }
            }
            colorNext++;
        }
    } else if (depth == 1) {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + v(i) * width + data_size * channel);
                if(data_size*(u(i) + channel) >= width ||
                   v(i) >= height) {
                    m(colorNext, i) = 0;
                } else {
                    // FIXME: add support for different surface formats
                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (unsigned char *)byteOffset ), N2, i);
                    } else {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (RT *)byteOffset ), N2, i);
                    }
                }
            }
            colorNext++;
        }
    } else {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + v(i) * width + r(i) * width * height + data_size * channel);
                if(data_size*(u(i) + channel) >= width ||
                   v(i) >= height || 
                   r(i) >= depth) {
                    m(colorNext, i) = 0;
                } else {
                    // FIXME: add support for different surface formats
                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (unsigned char *)byteOffset ), N2, i);
                    } else {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (RT *)byteOffset ), N2, i);
                    }
                }
            }
            colorNext++;
        }
    }

    return true;
}

template <typename RT, uint N1, uint N2>
CM_API extern bool 
read_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix<RT, N1, N2> &m, 
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0) 
{
    matrix_ref<RT, N1, N2> m_ref = m;
    return read_typed(surfIndex, channelMask, m_ref, u, v, r);
}

/* Typed surface write */
template <typename RT, uint N1, uint N2>
CM_API extern bool 
write_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix_ref<RT, N1, N2> &m, 
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;

    uchar *baseOffset, *byteOffset;
    uchar numColors=0, color[4]={0,0,0,0}, colorNext=0;

    if (channelMask & 0x1) {color[0]=1; numColors++;}
    if ((channelMask >> 1) & 0x1) {color[1]=1; numColors++;}
    if ((channelMask >> 2) & 0x1) {color[2]=1; numColors++;}
    if ((channelMask >> 3) & 0x1) {color[3]=1; numColors++;}

    if (numColors == 0) {
        fprintf(stderr, "write_typed error: At least one"
                "destination channel has to be read.\n");
        exit(EXIT_FAILURE);
    }

    if (N1 < numColors) {
        fprintf(stderr, "write_typed error: source matrix"
                "does not have enough space to hold data.\n");
        exit(EXIT_FAILURE);
    }

    if (N2 != 8 && N2 != 16) {
        fprintf(stderr, "write_typed error: offset vector size"
                "must be 8 or 16.\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "write_typed error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    uint width, height, depth, data_size;
    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;

    // FIXME: add support for more surface formats
    if ((surfFormat == R32_SINT) ||
        (surfFormat == R32_UINT) ||
        (surfFormat == R32_FLOAT)) {

        if(channelMask != CM_R_ENABLE) {
            fprintf(stderr, "write_typed error: only CM_R_ENABLE is supported for R32_SINT/R32_UINT/R32_FLOAT surface format.\n");
            exit(EXIT_FAILURE);
        }

        data_size = 4;
    } else if (surfFormat == R8G8B8A8_UINT) {
        data_size = 1;
    } else {
        fprintf(stderr, "write_typed error: only R32_SINT/R32_UINT/R32_FLOAT/R8G8B8A8_UINT surface formats are supported.\n");
        exit(EXIT_FAILURE);
    }

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if ((height == 1) && (depth == 1)) {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + data_size * channel);
                if(data_size*(u(i) + channel) >= width) {
                    break;
                } else {
                    // FIXME: add support for different surface formats
                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(*( (unsigned char *)byteOffset ) = m(colorNext, i), N2, i);
                    } else {
                        SIMDCF_WRAPPER(*( (RT *)byteOffset ) = m(colorNext, i), N2, i);
                    }
                }
            }
            colorNext++;
        }
    } else if (depth == 1) {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + v(i) * width + data_size * channel);
                if(data_size*(u(i) + channel) >= width ||
                   v(i) >= height) {
                    break;
                } else {
                    // FIXME: add support for different surface formats
                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(*( (unsigned char *)byteOffset ) = m(colorNext, i), N2, i);
                    } else {
                        SIMDCF_WRAPPER(*( (RT *)byteOffset ) = m(colorNext, i), N2, i);
                    }
                }
            }
            colorNext++;
        }
    } else {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + v(i) * width + r(i) * width * height + data_size * channel);
                if(data_size*(u(i) + channel) >= width ||
                   v(i) >= height || 
                   r(i) >= depth) {
                    break;
                } else {
                    // FIXME: add support for different surface formats
                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(*( (unsigned char *)byteOffset ) = m(colorNext, i), N2, i);
                    } else {
                        SIMDCF_WRAPPER(*( (RT *)byteOffset ) = m(colorNext, i), N2, i);
                    }
                }
            }
            colorNext++;
        }
    }

    return true;
}

template <typename RT, uint N1, uint N2>
CM_API extern bool 
write_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix<RT, N1, N2> &m, 
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0) 
{
    matrix_ref<RT, N1, N2> m_ref = m;
    return write_typed(surfIndex, channelMask, m_ref, u, v, r);
}

/* Untyped surface read */
template <typename RT, uint N1, uint N2>
CM_API extern bool
read_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix_ref<RT, N1, N2> &m, 
     const vector<uint, N2> &u) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;

    uchar *baseOffset, *byteOffset;
    uchar numColors=0, color[4]={0,0,0,0}, colorNext=0;

    if (channelMask & 0x1) {color[0]=1; numColors++;}
    if ((channelMask >> 1) & 0x1) {color[1]=1; numColors++;}
    if ((channelMask >> 2) & 0x1) {color[2]=1; numColors++;}
    if ((channelMask >> 3) & 0x1) {color[3]=1; numColors++;}

    if (numColors == 0) {
        fprintf(stderr, "read_untyped error: At least one "
                "destination channel has to be read.\n");
        exit(EXIT_FAILURE);
    }

    if (N1 < numColors) {
        fprintf(stderr, "read_untyped error: destination matrix "
                "does not have enough space to hold data.\n");
        exit(EXIT_FAILURE);
    }

    if (N2 != 8 && N2 != 16) {
        fprintf(stderr, "read_untyped error: offset vector size "
                "must be 8 or 16.\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "read_untyped error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    uint width, height, depth, pitch;
    CmSurfaceFormatID surfFormat;

    // FIXME: pass valid format from CMRT
#if 0
    surfFormat = buff_iter->pixelFormat;
    if (surfFormat != Format_RAW) { 
        fprintf(stderr, "read_untyped error: only RAW format is supported for read_untyped.\n");
        exit(EXIT_FAILURE);
    }
#endif

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if (height != 1 || depth != 1) {
        fprintf(stderr, "read_untyped error: invalid input surface type.\n");
        exit(EXIT_FAILURE);
    }

    baseOffset = (uchar *) buff_iter->p_volatile;

    for (uint j=0; j<4; j++) {
        if (color[j] == 0) continue;
        for (uint i=0; i<N2; i++) {
            int offset = sizeof(RT) * (u(i) + j);
            byteOffset = baseOffset +  offset;
            if(offset >= width) {
                m(colorNext, i) = 0;
            } else {
                m(colorNext, i) = *( (RT *)byteOffset );
            }
        }
        colorNext++;
    }

    return true;
}

template <typename RT, uint N1, uint N2>
CM_API extern bool
read_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix<RT, N1, N2> &m, 
     const vector<uint, N2> &u) 
{
    matrix_ref<RT, N1, N2> m_ref = m;
    return read_untyped(surfIndex, channelMask, m_ref, u);
}

/* Untyped surface write */
template <typename RT, uint N1, uint N2>
CM_API extern bool
write_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix_ref<RT, N1, N2> &m, 
     const vector<uint, N2> &u) 
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;

    uchar *baseOffset, *byteOffset;
    uchar numColors=0, color[4]={0,0,0,0}, colorNext=0;

    if(channelMask != CM_R_ENABLE && 
       channelMask != CM_GR_ENABLE && 
       channelMask != CM_BGR_ENABLE &&
       channelMask != CM_ABGR_ENABLE) {
        fprintf(stderr, "write_untyped error: Invalid channel mask.\n");
        exit(EXIT_FAILURE);
    }

    if (channelMask & 0x1) {color[0]=1; numColors++;}
    if ((channelMask >> 1) & 0x1) {color[1]=1; numColors++;}
    if ((channelMask >> 2) & 0x1) {color[2]=1; numColors++;}
    if ((channelMask >> 3) & 0x1) {color[3]=1; numColors++;}

    if (numColors == 0) {
        fprintf(stderr, "write_untyped error: At least one "
                "destination channel has to be read.\n");
        exit(EXIT_FAILURE);
    }

    if (N1 < numColors) {
        fprintf(stderr, "write_untyped error: source matrix "
                "does not have enough space to hold data.\n");
        exit(EXIT_FAILURE);
    }

    if (N2 != 8 && N2 != 16) {
        fprintf(stderr, "write_untyped error: offset vector size "
                "must be 8 or 16.\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "write_untyped error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    uint width, height, depth, pitch;
    CmSurfaceFormatID surfFormat;

    // FIXME: pass valid format from CMRT
#if 0
    surfFormat = buff_iter->pixelFormat;
    if (surfFormat != Format_RAW) { 
        fprintf(stderr, "read_untyped error: only RAW format is supported for read_untyped.\n");
        exit(EXIT_FAILURE);
    }
#endif

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if (height != 1 || depth != 1) {
        fprintf(stderr, "write_untyped error: invalid input surface type.\n");
        exit(EXIT_FAILURE);
    }

    baseOffset = (uchar *) buff_iter->p_volatile;

    for (uint j=0; j<4; j++) {
        if (color[j] == 0) continue;
        for (uint i=0; i<N2; i++) {
            int offset = sizeof(RT) * (u(i) + j);
            byteOffset = baseOffset +  offset;
            if(offset >= width) {
                break;
            } else {
                *( (RT *)byteOffset ) = m(colorNext, i);
            }
        }
        colorNext++;
    }

    return true;
}

template <typename RT, uint N1, uint N2>
CM_API extern bool
write_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask, 
     matrix<RT, N1, N2> &m, 
     const vector<uint, N2> &u) 
{
    matrix_ref<RT, N1, N2> m_ref = m;
    return write_untyped(surfIndex, channelMask, m_ref, u);
}

#endif

#endif /* CM_EMU */

#endif /* GENX_DATAPORT_H */
