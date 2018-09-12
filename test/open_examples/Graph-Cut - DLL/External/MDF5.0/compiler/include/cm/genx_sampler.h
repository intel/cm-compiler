/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: genx_sampler.h 24789 2011-01-06 18:45:39Z ayermolo $"
*** -----------------------------------------------------------------------------------------------
***
*** Copyright  (C) 1985-2007 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:             Kaiyu Chen
***                      
***                      
***                      
***
*** Description: Cm sampler APIs
***
*** -----------------------------------------------------------------------------------------------
**/

#ifndef CM_SAMPLER_H
#define CM_SAMPLER_H

#include "cm_def.h"
#include "cm_vm.h"
#include "cm_common.h"

#define SAMPLE16_ENABLED_R(MASK) (MASK & 0x1)
#define SAMPLE16_ENABLED_G(MASK) (MASK >> 1 & 0x1)
#define SAMPLE16_ENABLED_B(MASK) (MASK >> 2 & 0x1)
#define SAMPLE16_ENABLED_A(MASK) (MASK >> 3 & 0x1)
#define SAMPLE16_ENABLED_CHANNELS(MASK) ( SAMPLE16_ENABLED_R(MASK) \
                                          + SAMPLE16_ENABLED_G(MASK) \
                                          + SAMPLE16_ENABLED_B(MASK) \
                                          + SAMPLE16_ENABLED_A(MASK))

#define INDEX_2D(j, i, width) (j*width + i)
#define INDEX_3D(k, j, i, width, height) (k*height*width + j*width + i)
#define INDEX_4D(l, k, j, i, width, height, depth) (l*depth*height*width + k*height*width + j*width + i)

typedef enum _SamplerMapFilterType_ {
    FILTER_NEAREST,
    FILTER_LINEAR,
    FILTER_ANISOTROPIC,
    FILTER_MONO
} SamplerMapFilterType;

typedef enum _AddressControlType_ {
    ADDRESS_WRAP,
    ADDRESS_MIRROR,
    ADDRESS_CLAMP
} AddressControlType;

void CM_register_sampler(uint samplerIndex, SamplerMapFilterType minFilter,
                         SamplerMapFilterType magFilter, AddressControlType uControl,
                         AddressControlType vControl, AddressControlType wControl);

void CM_register_sampler(SamplerIndex samplerIndex, SamplerMapFilterType minFilter,
                         SamplerMapFilterType magFilter, AddressControlType uControl,
                         AddressControlType vControl, AddressControlType wControl);

void CM_register_sampler8x8( uint samplerIndex, void* samplerState );

void CM_sampler_sample16( SurfaceIndex surfIndex, SamplerIndex samplerIndex, ChannelMaskType channelMask, 
						 const vector<float, 16>* inpR, const vector<float, 16>* inpG, const vector<float, 16>* inpB, 
						 void *pDataOut  );

void CM_sampler_sample32( SurfaceIndex surfIndex, SamplerIndex samplerIndex, ChannelMaskType channelMask, 
						 const vector<float, 16>* inpR, const vector<float, 16>* inpG, const vector<float, 16>* inpB, 
						 void *pDataOut  );

inline unsigned int CM_GetNumOfChannels_Emu( ChannelMaskType channelMask )
{
	//
	// mapping writeback SamplerWriteback to matrix m
	//
	uint32_t nChannels = 0;
	switch ( channelMask )
	{
		case CM_R_ENABLE:
		case CM_G_ENABLE:
		case CM_B_ENABLE:
		case CM_A_ENABLE:
			nChannels = 1;
			break;
		case CM_GR_ENABLE:
		case CM_BR_ENABLE:
		case CM_BG_ENABLE:
		case CM_AR_ENABLE:
		case CM_AG_ENABLE:
		case CM_AB_ENABLE:
			nChannels = 2;
			break;
		case CM_BGR_ENABLE:
		case CM_AGR_ENABLE:
		case CM_ABR_ENABLE:
		case CM_ABG_ENABLE:
			nChannels = 3;
			break;
		case CM_ABGR_ENABLE:
			nChannels = 4;
			break;
		default:
			// should not get here
			nChannels = 4;
			break;
	}
    return nChannels;
}

#ifndef CM_EMU

template <typename RT, uint N>
CM_API extern void
sample16(matrix<RT, N, 16> &m, ChannelMaskType channelMask, uint surfIndex, uint samplerIndex, 
         const vector<float, 16> &u, const vector<float, 16> &v = 0, const vector<float, 16> &r = 0)
{
    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N>
CM_API extern void
sample16(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, uint surfIndex, uint samplerIndex, 
         const vector<float, 16> &u, const vector<float, 16> &v = 0, const vector<float, 16> &r = 0)
{
    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N>
CM_API extern void
load16(matrix<RT, N, 16> &m, ChannelMaskType channelMask, uint surfIndex,
       const vector<uint, 16> &u, const vector<uint, 16> &v = 0, const vector<uint, 16> &r = 0)
{
    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N>
CM_API extern void
load16(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, uint surfIndex,  
       const vector<uint, 16> &u, const vector<uint, 16> &v = 0, const vector<uint, 16> &r = 0)
{
    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N>
CM_API extern void
sample32(matrix<RT, N, 32> &m, ChannelMaskType channelMask, uint surfIndex, uint samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, OutputFormatControl ofc = CM_16_FULL)
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N>
CM_API extern void
sample32(matrix_ref<RT, N, 32> m, ChannelMaskType channelMask, uint surfIndex, uint samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, OutputFormatControl ofc = CM_16_FULL)
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

#else 

template <typename RT, uint N>
extern void
load16_1D_UNORM_emu(matrix<RT, N, 16> &m, ChannelMaskType channelMask, uchar *bsrc, uint width, 
                      const vector<uint, 16> &inpR, CmSurfaceFormatID surfFormat)
{
    int i, j, k;

    uchar *lut = (uchar *) malloc(width * sizeof(uchar));
    uchar *bsrct = (uchar *) bsrc;
    if (lut == NULL) {
        fprintf(stderr, "Error: memory allocation failure in sampler emulation!\n");
        exit(EXIT_FAILURE);
    }

    for (k = 0; k < width/16; k++) {
        if(surfFormat == R8G8B8A8_UNORM) {
            lut[INDEX_2D(k, 0, 4)] = *bsrct++; //R
            lut[INDEX_2D(k, 1, 4)] = *bsrct++; //G
            lut[INDEX_2D(k, 2, 4)] = *bsrct++; //B
            lut[INDEX_2D(k, 3, 4)] = *bsrct++; //A
        } else {
            lut[INDEX_2D(k, 2, 4)] = *bsrct++; //R
            lut[INDEX_2D(k, 1, 4)] = *bsrct++; //G
            lut[INDEX_2D(k, 0, 4)] = *bsrct++; //B
            lut[INDEX_2D(k, 3, 4)] = *bsrct++; //A
        }
    }

    for (i = 0; i < 16; i++) {
		float* fp;
        // Convert to addres range
        uint r = inpR(i)/4;
        for (j = 0, k = 0; j < 4; j++) {
            if ((channelMask & (1 << j)) == 0)
                continue;
            // Linear interpolation followed by normalization to [0.0f-1.0f] range
			fp = (float*)&m(k, i);
            *fp =  (lut[INDEX_2D(r, j, 4)])/255.0f;
            k++;
        }
    }

    free(lut);
}

template <typename RT, uint N>
extern void
load16_2D_UNORM_emu(matrix<RT, N, 16> &m, ChannelMaskType channelMask, uchar *bsrc, uint width, uint height, 
                      const vector<uint, 16> &inpR, const vector<uint, 16> &inpG, CmSurfaceFormatID surfFormat)
{
    int i, j, k;
    int columns = width/4;

    uchar *lut = (uchar *) malloc(height * width * sizeof(uchar));
    uchar *bsrct = (uchar *) bsrc;
    if (lut == NULL) {
        fprintf(stderr, "Error: memory allocation failure in sampler emulation!\n");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < height; j++) {
        for (k = 0; k < width/16; k++) {
            if(surfFormat == R8G8B8A8_UNORM) {
                lut[INDEX_3D(j, k, 0, 4, columns)] = *bsrct++; //R
                lut[INDEX_3D(j, k, 1, 4, columns)] = *bsrct++; //G
                lut[INDEX_3D(j, k, 2, 4, columns)] = *bsrct++; //B
                lut[INDEX_3D(j, k, 3, 4, columns)] = *bsrct++; //A
            } else {
                lut[INDEX_3D(j, k, 2, 4, columns)] = *bsrct++; //R
                lut[INDEX_3D(j, k, 1, 4, columns)] = *bsrct++; //G
                lut[INDEX_3D(j, k, 0, 4, columns)] = *bsrct++; //B
                lut[INDEX_3D(j, k, 3, 4, columns)] = *bsrct++; //A
            }
        }
    }

    for (i = 0; i < 16; i++) {
        // Convert to address range
        uint r = inpR(i)/4;
        uint g = inpG(i);
		float* fp;
        for (j = 0, k = 0; j < 4; j++) {
            if ((channelMask & (1 << j)) == 0)
                continue;
            // Linear interpolation followed by normalization to [0.0f-1.0f] range
			fp = (float*)&m(k, i);
            *fp =  (lut[INDEX_3D(g, r, j, 4, columns)])/255.0f;
            k++;
        }
    }

    free(lut);
}

template <typename RT, uint N>
extern void
load16_3D_UNORM_emu(matrix<RT, N, 16> &m, ChannelMaskType channelMask, uchar *bsrc, uint width, uint height, uint depth, 
                      const vector<uint, 16> &inpR, const vector<uint, 16> &inpG, const vector<uint, 16> &inpB, CmSurfaceFormatID surfFormat)
{
    int i, j, k;
    int columns = width/4;
    
    uchar *lut = (uchar *) malloc(depth * height * width * sizeof(uchar));
    uchar *bsrct = (uchar *) bsrc;
    if (lut == NULL) {
        fprintf(stderr, "Error: memory allocation failure in sampler emulation!\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < depth; i++) {
        for (j = 0; j < height; j++) {
            for (k = 0; k < width/16; k++) {
                if(surfFormat == R8G8B8A8_UNORM) {
                    lut[INDEX_4D(i, j, k, 0, 4, columns, height)] = *bsrct++; //R
                    lut[INDEX_4D(i, j, k, 1, 4, columns, height)] = *bsrct++; //G
                    lut[INDEX_4D(i, j, k, 2, 4, columns, height)] = *bsrct++; //B
                    lut[INDEX_4D(i, j, k, 3, 4, columns, height)] = *bsrct++; //A 
                } else {
                    lut[INDEX_4D(i, j, k, 2, 4, columns, height)] = *bsrct++; //R
                    lut[INDEX_4D(i, j, k, 1, 4, columns, height)] = *bsrct++; //G
                    lut[INDEX_4D(i, j, k, 0, 4, columns, height)] = *bsrct++; //B
                    lut[INDEX_4D(i, j, k, 3, 4, columns, height)] = *bsrct++; //A 
                }
            }
        }
    }

    for (i = 0; i < 16; i++) {
        // Convert to address range
        uint r = inpR(i)/4;
        uint g = inpG(i);
        uint b = inpB(i);
		float* fp;
        for (j = 0, k = 0; j < 4; j++) {
            if ((channelMask & (1 << j)) == 0)
                continue;
            // Linear interpolation followed by normalization to [0.0f-1.0f] range
			fp = (float*)&m(k, i);
            *fp =  (lut[INDEX_4D(b, g, r, j, 4, columns, height)])/255.0f;
            k++;
        }
    }

    free(lut);
}

template <typename RT, uint N>
extern void
load16_2D_UINT_emu(matrix<RT, N, 16> &m, ChannelMaskType channelMask, uchar *bsrc, uint width, uint height, 
                      const vector<uint, 16> &inpR, const vector<uint, 16> &inpG, CmSurfaceFormatID surfFormat)
{
    uint i, j, k;
    uint columns = width/4;

    uchar *lut = (uchar *) malloc(height * width * sizeof(uchar));
    uchar *bsrct = (uchar *) bsrc;
    if (lut == NULL) {
        fprintf(stderr, "Error: memory allocation failure in sampler emulation!\n");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < height; j++) {
		for (k = 0; k < columns; k++) {
            lut[INDEX_3D(j, k, 3, 4, columns)] = *bsrct++; //R
            lut[INDEX_3D(j, k, 2, 4, columns)] = *bsrct++; //G
            lut[INDEX_3D(j, k, 1, 4, columns)] = *bsrct++; //B
            lut[INDEX_3D(j, k, 0, 4, columns)] = *bsrct++; //A
        }
    }

    for (i = 0; i < 16; i++) {
        uint g = inpG(i);
		uint* uip = (uint*)&m(0, i);
		*uip = 0;
        for (j = 0, k = 0; j < 4; j++) {
			if ((channelMask & (1 << j)) == 0) 
				continue;
			*uip |= (uint)lut[INDEX_3D(g, inpR(i), j, 4, columns)] << ((3-k) * 8);
            k++;
        }
    }

    free(lut);
}


template <typename RT, uint N>
CM_API extern void
load16(matrix<RT, N, 16> &m, ChannelMaskType channelMask, uint surfIndex,
       const vector<uint, 16> &u, const vector<uint, 16> &v = 0, const vector<uint, 16> &r = 0)
{
    uint width, height, depth, pitch;
    CmSurfaceFormatID surfFormat;
    CmBufferType bclass;
    uchar *bsrc;

    
    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;

    if(N < SAMPLE16_ENABLED_CHANNELS(channelMask)) {
        fprintf(stderr, "Error: incorrect matrix size for sample16 - the number of rows is less than the number of enabled channels!\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(surfIndex);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "Error: cannot read surface %d !\n", surfIndex);
        exit(EXIT_FAILURE);
    }

    bclass = buff_iter->bclass;
    if ((bclass != GEN4_INPUT_BUFFER) && 
        (bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error: the sampler surface type of buffer %d is not GEN4_INPUT_BUFFER or GEN4_INPUT_OUTPUT_BUFFER!\n", buff_iter->id);
        exit(EXIT_FAILURE);
    }

    // Currently only limited emulation support is provided
    surfFormat = buff_iter->pixelFormat;
    if ((surfFormat != R8G8B8A8_UNORM) &&
        (surfFormat != B8G8R8A8_UNORM) &&
        (surfFormat != R8G8B8A8_UINT)) { 
        fprintf(stderr, "ERROR: currently only R8G8B8A8_UNORM/B8G8R8A8_UNORM/R8G8B8A8_UINT surface format is supported in sampler emulation.\n");
        exit(EXIT_FAILURE);
    }

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if (buff_iter->pitch == 0) {
        pitch = width;
    } else {
        pitch = buff_iter->pitch;
    }
    bsrc = (uchar *) buff_iter->p;

	if ( R8G8B8A8_UINT == surfFormat ) {
		if ((height == 1) && (depth == 1)) {
			fprintf(stderr, "ERROR: currently only R8G8B8A8_UINT 2D surface format is supported in sampler emulation.\n");
            exit(EXIT_FAILURE);
		} else if (depth == 1) {
			load16_2D_UINT_emu(m, channelMask, bsrc, width, height, u, v, surfFormat);
		} else {
			fprintf(stderr, "ERROR: currently only R8G8B8A8_UINT 2D surface format is supported in sampler emulation.\n");
            exit(EXIT_FAILURE);
		}
	}
	else {
		if ((height == 1) && (depth == 1)) {
			load16_1D_UNORM_emu(m, channelMask, bsrc, width, u, surfFormat);
		} else if (depth == 1) {
			load16_2D_UNORM_emu(m, channelMask, bsrc, width, height, u, v, surfFormat);
		} else {
			load16_3D_UNORM_emu(m, channelMask, bsrc, width, height, depth, u, v, r, surfFormat);
		}
	}

}

template <typename RT, uint N>
extern void
load16_1D_UNORM_emu(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, uchar *bsrc, uint width, 
                      const vector<uint, 16> &inpR, CmSurfaceFormatID surfFormat)
{
    int i, j, k;
    
    uchar *lut = (uchar *) malloc(width * sizeof(uchar));
    uchar *bsrct = (uchar *) bsrc;

    if (lut == NULL) {
        fprintf(stderr, "Error: memory allocation failure in sampler emulation!\n");
        exit(EXIT_FAILURE);
    }

    for (k = 0; k < width/16; k++) {
        if(surfFormat == R8G8B8A8_UNORM) {
            lut[INDEX_2D(k, 0, 4)] = *bsrct++; //R
            lut[INDEX_2D(k, 1, 4)] = *bsrct++; //G
            lut[INDEX_2D(k, 2, 4)] = *bsrct++; //B
            lut[INDEX_2D(k, 3, 4)] = *bsrct++; //A
        } else {
            lut[INDEX_2D(k, 2, 4)] = *bsrct++; //R
            lut[INDEX_2D(k, 1, 4)] = *bsrct++; //G
            lut[INDEX_2D(k, 0, 4)] = *bsrct++; //B
            lut[INDEX_2D(k, 3, 4)] = *bsrct++; //A
        }
    }

    for (i = 0; i < 16; i++) {
        // Convert to addres range
        uint r = inpR(i)/4;

        for (j = 0, k = 0; j < 4; j++) {
            if ((channelMask & (1 << j)) == 0)
                continue;
            // Linear interpolation followed by normalization to [0.0f-1.0f] range
            m(k, i) =  (lut[INDEX_2D(r, j, 4)])/255.0f;
            k++;
        }
    }

    free(lut);
}

template <typename RT, uint N>
extern void
load16_2D_UNORM_emu(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, uchar *bsrc, uint width, uint height, 
                      const vector<uint, 16> &inpR, const vector<uint, 16> &inpG, CmSurfaceFormatID surfFormat)
{
    int i, j, k;
    int columns = width/4;

    uchar *lut = (uchar *) malloc(height * width * sizeof(uchar));
    uchar *bsrct = (uchar *) bsrc;
    if (lut == NULL) {
        fprintf(stderr, "Error: memory allocation failure in sampler emulation!\n");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < height; j++) {
        for (k = 0; k < width/16; k++) {
            if(surfFormat == R8G8B8A8_UNORM) {
                lut[INDEX_3D(j, k, 0, 4, columns)] = *bsrct++; //R
                lut[INDEX_3D(j, k, 1, 4, columns)] = *bsrct++; //G
                lut[INDEX_3D(j, k, 2, 4, columns)] = *bsrct++; //B
                lut[INDEX_3D(j, k, 3, 4, columns)] = *bsrct++; //A
            } else {
                lut[INDEX_3D(j, k, 2, 4, columns)] = *bsrct++; //R
                lut[INDEX_3D(j, k, 1, 4, columns)] = *bsrct++; //G
                lut[INDEX_3D(j, k, 0, 4, columns)] = *bsrct++; //B
                lut[INDEX_3D(j, k, 3, 4, columns)] = *bsrct++; //A
            }
        }
    }

    for (i = 0; i < 16; i++) {
        // Convert to address range
        uint r = inpR(i)/4;
        uint g = inpG(i);

        for (j = 0, k = 0; j < 4; j++) {
            if ((channelMask & (1 << j)) == 0)
                continue;
            // Linear interpolation followed by normalization to [0.0f-1.0f] range
            m(k, i) =  (lut[INDEX_3D(g, r, j, 4, columns)])/255.0f;
            k++;
        }
    }

    free(lut);
}

template <typename RT, uint N>
extern void
load16_3D_UNORM_emu(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, uchar *bsrc, uint width, uint height, uint depth, 
                      const vector<uint, 16> &inpR, const vector<uint, 16> &inpG, const vector<uint, 16> &inpB, CmSurfaceFormatID surfFormat)
{
    int i, j, k;
    int columns = width/4;

    uchar *lut = (uchar *) malloc(depth * height * width * sizeof(uchar));
    uchar *bsrct = (uchar *) bsrc;
    if (lut == NULL) {
        fprintf(stderr, "Error: memory allocation failure in sampler emulation!\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < depth; i++) {
        for (j = 0; j < height; j++) {
            for (k = 0; k < width/16; k++) {
                if(surfFormat == R8G8B8A8_UNORM) {
                    lut[INDEX_4D(i, j, k, 0, 4, columns, height)] = *bsrct++; //R
                    lut[INDEX_4D(i, j, k, 1, 4, columns, height)] = *bsrct++; //G
                    lut[INDEX_4D(i, j, k, 2, 4, columns, height)] = *bsrct++; //B
                    lut[INDEX_4D(i, j, k, 3, 4, columns, height)] = *bsrct++; //A 
                } else {
                    lut[INDEX_4D(i, j, k, 2, 4, columns, height)] = *bsrct++; //R
                    lut[INDEX_4D(i, j, k, 1, 4, columns, height)] = *bsrct++; //G
                    lut[INDEX_4D(i, j, k, 0, 4, columns, height)] = *bsrct++; //B
                    lut[INDEX_4D(i, j, k, 3, 4, columns, height)] = *bsrct++; //A 
                }
            }
        }
    }

    for (i = 0; i < 16; i++) {
        // Convert to address range
        uint r = inpR(i)/4;
        uint g = inpG(i);
        uint b = inpB(i);

        for (j = 0, k = 0; j < 4; j++) {
            if ((channelMask & (1 << j)) == 0)
                continue;
            // Linear interpolation followed by normalization to [0.0f-1.0f] range
            m(k, i) =  (lut[INDEX_4D(b, g, r, j, 4, columns, height)])/255.0f;
            k++;
        }
    }

    free(lut);
}

template <typename RT, uint N>
CM_API extern void
load16_3D_UNORM_emu(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, SurfaceIndex surfIndex,
       const vector<uint, 16> &u, const vector<uint, 16> &v = 0, const vector<uint, 16> &r = 0)
{
    uint width, height, depth, pitch;
    CmSurfaceFormatID surfFormat;
    CmBufferType bclass;
    uchar *bsrc;

    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;

    if(N < SAMPLE16_ENABLED_CHANNELS(channelMask)) {
        fprintf(stderr, "Error: incorrect matrix size for sample16 - the number of rows is less than the number of enabled channels!\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "Error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    bclass = buff_iter->bclass;
    if ((bclass != GEN4_INPUT_BUFFER) && 
        (bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error: the sampler surface type of buffer %d is not GEN4_INPUT_BUFFER or GEN4_INPUT_OUTPUT_BUFFER!\n", buff_iter->id);
        exit(EXIT_FAILURE);
    }

    // Currently only limited emulation support is provided
    surfFormat = buff_iter->pixelFormat;
    if ((surfFormat != R8G8B8A8_UNORM) &&
        (surfFormat != B8G8R8A8_UNORM) &&
        (surfFormat != R8G8B8A8_UINT)) { 
        fprintf(stderr, "ERROR: currently only R8G8B8A8_UNORM/B8G8R8A8_UNORM/R8G8B8A8_UINT surface format is supported in sampler emulation.\n");
        exit(EXIT_FAILURE);
    }

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if (buff_iter->pitch == 0) {
        pitch = width;
    } else {
        pitch = buff_iter->pitch;
    }

    bsrc = (uchar *) buff_iter->p;

    if ((height == 1) && (depth == 1)) {
        load16_1D_UNORM_emu(m, channelMask, bsrc, width, u, surfFormat);
    } else if (depth == 1) {
        load16_2D_UNORM_emu(m, channelMask, bsrc, width, height, u, v, surfFormat);
    } else {
        load16_3D_UNORM_emu(m, channelMask, bsrc, width, height, depth, u, v, r, surfFormat);
    }
}

template <typename RT, uint N>
void sample16_UNORM_emu(matrix<RT, N, 16> &m, ChannelMaskType channelMask, const vector<float, 16>* inpR, const vector<float, 16>* inpG, const vector<float, 16>* inpB, 
						   SurfaceIndex iSurface, SamplerIndex iSampler )
{
	uint nWriteBackSize = 64; 
	uint32_t* SamplerWriteback = new uint32_t[ nWriteBackSize ];
	if ( SamplerWriteback == NULL )
	{
        fprintf( stderr, "Error: not enough memory!\n" );
        exit( EXIT_FAILURE );
	}
	memset( &SamplerWriteback[ 0 ], 0, nWriteBackSize * sizeof( uint32_t ) );

	CM_sampler_sample16( iSurface, iSampler, channelMask, inpR, inpG, inpB, SamplerWriteback );

	float *pIndex = ( float * )SamplerWriteback;

    unsigned int nChannels = CM_GetNumOfChannels_Emu( channelMask );

	for ( unsigned int iChannel = 0; iChannel < nChannels; ++iChannel )
	{
		for ( unsigned int i = 0; i < 16; i++ ) 
		{
			m( iChannel, i ) = *pIndex;
			++pIndex;
		}
	}
	
	if ( SamplerWriteback != NULL )
	{
		delete[] SamplerWriteback;
		SamplerWriteback = NULL;
	}
	return;
}

template <typename RT, uint N>
void sample16_UNORM_emu(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, const vector<float, 16>* inpR, const vector<float, 16>* inpG, const vector<float, 16>* inpB, 
						   SurfaceIndex iSurface, SamplerIndex iSampler )
{
	uint nWriteBackSize = 64; 
	uint32_t* SamplerWriteback = new uint32_t[ nWriteBackSize ];
	if ( SamplerWriteback == NULL )
	{
        fprintf( stderr, "Error: not enough memory!\n" );
        exit( EXIT_FAILURE );
	}
	memset( &SamplerWriteback[ 0 ], 0, nWriteBackSize * sizeof( uint32_t ) );

	CM_sampler_sample16( iSurface, iSampler, channelMask, inpR, inpG, inpB, SamplerWriteback );

	float *pIndex = ( float * )SamplerWriteback;

    unsigned int nChannels = CM_GetNumOfChannels_Emu( channelMask );

    for ( unsigned iChannel = 0; iChannel < nChannels; ++iChannel )
	{
		for ( unsigned i = 0; i < 16; i++ ) 
		{
			m( iChannel, i ) = *pIndex;
			++pIndex;
		}
	}

	if ( SamplerWriteback != NULL )
	{
		delete[] SamplerWriteback;
		SamplerWriteback = NULL;
	}
	return;
}

template <typename RT, uint N>
CM_API extern void
sample32(matrix<RT, N, 32> &m, ChannelMaskType channelMask, uint surfIndex, uint samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, OutputFormatControl ofc = CM_16_FULL)
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;

    fprintf(stderr, "Error: sample32 is not supported in emulation mode.\n");
}

template <typename RT, uint N>
CM_API extern void
sample32(matrix_ref<RT, N, 32> m, ChannelMaskType channelMask, uint surfIndex, uint samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, OutputFormatControl ofc = CM_16_FULL)
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;

    fprintf(stderr, "Error: sample32 is not supported in emulation mode.\n");
}

#endif /* CM_EMU */

#ifndef CM_EMU

template <typename RT, uint N>
CM_API extern void
sample16(matrix<RT, N, 16> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const vector<float, 16> &u, const vector<float, 16> &v = 0, const vector<float, 16> &r = 0)
{
    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N>
CM_API extern void
sample16(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const vector<float, 16> &u, const vector<float, 16> &v = 0, const vector<float, 16> &r = 0)
{
    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N>
CM_API extern void
load16(matrix<RT, N, 16> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, 
         const vector<uint, 16> &u, const vector<uint, 16> &v = 0, const vector<uint, 16> &r = 0)
{
    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N>
CM_API extern void
load16(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, SurfaceIndex surfIndex, 
         const vector<uint, 16> &u, const vector<uint, 16> &v = 0, const vector<uint, 16> &r = 0)
{
    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}


template <typename RT, uint N>
CM_API extern void
sample32(matrix<RT, N, 32> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, OutputFormatControl ofc = CM_16_FULL)
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N>
CM_API extern void
sample32(matrix_ref<RT, N, 32> m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, OutputFormatControl ofc = CM_16_FULL)
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

#else 

template <typename RT, uint N>
CM_API extern void
load16(matrix<RT, N, 16> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, 
         const vector<uint, 16> &u, const vector<uint, 16> &v = 0, const vector<uint, 16> &r = 0)
{
    load16(m, channelMask, surfIndex.get_data(), u, v, r);
}

template <typename RT, uint N>
CM_API extern void
load16(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, SurfaceIndex surfIndex, 
         const vector<uint, 16> &u, const vector<uint, 16> &v = 0, const vector<uint, 16> &r = 0)
{
    load16(m, channelMask, surfIndex.get_data(), u, v, r);
}

template <typename RT, uint N>
CM_API extern void
sample16(matrix_ref<RT, N, 16> m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const vector<float, 16> &u, const vector<float, 16> &v = 0, const vector<float, 16> &r = 0)
{
    uint width, height, depth, pitch;
    CmSurfaceFormatID surfFormat;
    CmBufferType bclass;
    uchar *bsrc;

    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;

    if(N < SAMPLE16_ENABLED_CHANNELS(channelMask)) {
        fprintf(stderr, "Error: incorrect matrix size for sample16 - the number of rows is less than the number of enabled channels!\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "Error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    bclass = buff_iter->bclass;
    if ((bclass != GEN4_INPUT_BUFFER) && 
        (bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error: the sampler surface type of buffer %d is not GEN4_INPUT_BUFFER or GEN4_INPUT_OUTPUT_BUFFER!\n", buff_iter->id);
        exit(EXIT_FAILURE);
    }

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if (buff_iter->pitch == 0) {
        pitch = width;
    } else {
        pitch = buff_iter->pitch;
    }

    bsrc = (uchar *) buff_iter->p;

    if ((height == 1) && (depth == 1)) {
        sample16_UNORM_emu(m, channelMask, &u, NULL, NULL, surfIndex, samplerIndex);
    } else if (depth == 1) {
        sample16_UNORM_emu(m, channelMask, &u, &v, NULL, surfIndex, samplerIndex);
    } else {
        sample16_UNORM_emu(m, channelMask, &u, &v, &r, surfIndex, samplerIndex);
    }
}

template <typename RT, uint N>
CM_API extern void
sample16(matrix<RT, N, 16> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const vector<float, 16> &u, const vector<float, 16> &v = 0, const vector<float, 16> &r = 0)
{
    uint width, height, depth, pitch;
    CmSurfaceFormatID surfFormat;
    CmBufferType bclass;
    uchar *bsrc;

    static const bool conformable = 
        check_true<is_fp_type<RT>::value>::value;

    if (N < SAMPLE16_ENABLED_CHANNELS(channelMask)) {
        fprintf(stderr, "Error: incorrect matrix size for sample16 - the number of rows is less than the number of enabled channels. \n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = 
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "Error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    bclass = buff_iter->bclass;
    if ((bclass != GEN4_INPUT_BUFFER) && 
        (bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error: the sampler surface type of buffer %d is not GEN4_INPUT_BUFFER or GEN4_INPUT_OUTPUT_BUFFER!\n", buff_iter->id);
        exit(EXIT_FAILURE);
    }

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if (buff_iter->pitch == 0) {
        pitch = width; 
    } else {
        pitch = buff_iter->pitch;
    }

    bsrc = (uchar *) buff_iter->p;

    if ((height == 1) && (depth == 1)) {
        sample16_UNORM_emu(m, channelMask, &u, NULL, NULL, surfIndex, samplerIndex);
    } else if (depth == 1) {
        sample16_UNORM_emu(m, channelMask, &u, &v, NULL, surfIndex, samplerIndex);
    } else {
        sample16_UNORM_emu(m, channelMask, &u, &v, &r, surfIndex, samplerIndex);
    }
}

template <typename RT, uint N>
CM_API extern void
sample32(matrix<RT, N, 32> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, OutputFormatControl ofc = CM_16_FULL)
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;

    fprintf(stderr, "Error: sample32 is not supported in emulation mode.\n");
}

template <typename RT, uint N>
CM_API extern void
sample32(matrix_ref<RT, N, 32> m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, OutputFormatControl ofc = CM_16_FULL)
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;

    fprintf(stderr, "Error: sample32 is not supported in emulation mode.\n");
}


CM_API extern void CM_register_sampler_surface_state( SurfaceIndex buf_id, void *src, uint width, uint height, 
                                      CmSurfaceFormatID surfFormat = R8G8B8A8_UINT, uint depth = 1, uint pitch = 0, uint surfaceType = 0 );

CM_API extern void CM_register_sampler8x8_surface_state( SurfaceIndex buf_id, void *src, uint width, uint height, 
                                      CmSurfaceFormatID surfFormat = R8G8B8A8_UINT, uint depth = 1, uint pitch = 0, uint surfaceType = 0 );

#endif /* CM_EMU */

#endif /* CM_SAMPLER_H */
