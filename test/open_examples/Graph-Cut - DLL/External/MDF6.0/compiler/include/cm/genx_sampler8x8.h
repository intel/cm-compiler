/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: genx_sampler.h 24789 2011-01-06 18:45:39Z ayermolo $"
*** -----------------------------------------------------------------------------------------------
***
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:             Alexander Yermolovich
***
***
***
***
*** Description: Cm sampler8x8 APIs
***
*** -----------------------------------------------------------------------------------------------
**/
#ifndef CM_SAMPLER8x8_H
#define CM_SAMPLER8x8_H

#include "cm_def.h"
#include "cm_vm.h"
#include "cm_common.h"

typedef enum _AVSExecMode_
{   CM_AVS_16x4 = 0,
    CM_AVS_8x4  = 1,
    CM_AVS_16x8 = 2,
    CM_AVS_4x4  = 3
} AVSExecMode;

typedef enum _MMFExecMode_
{   CM_MMF_16x4 = 0,
    CM_MMF_16x1 = 2,
    CM_MMF_1x1  = 3
} MMFExecMode;

typedef enum _MMFEnableMode_
{
    CM_MINMAX_ENABLE = 0,
    CM_MAX_ENABLE    = 1,
    CM_MIN_ENABLE    = 2
} MMFEnableMode;

typedef enum _CONVExecMode_
{   CM_CONV_16x4 = 0,
    CM_CONV_16x1 = 2,
    CM_CONV_1x1  = 3 //1pixel convolve only
} CONVExecMode;

typedef enum _EDExecMode_
{   CM_ED_64x4 = 0,
    CM_ED_32x4 = 1,
    CM_ED_64x1 = 2,
    CM_ED_32x1 = 3
} EDExecMode;

typedef enum _EDMode_
{   CM_ERODE  = 4,
    CM_DILATE = 5
} EDMode;

typedef enum _LBPCreationExecMode_
{   CM_LBP_CREATION_5x5  = 0x2,
    CM_LBP_CREATION_3x3  = 0x1,
    CM_LBP_CREATION_BOTH = 0x0
} LBPCreationExecMode;

typedef enum _LBPCorrelationExecMode_
{   CM_LBP_CORRELATION_16x4 = 0x0,
    CM_LBP_CORRELATION_16x1 = 0x2
} LBPCorrelationExecMode;

typedef enum _CM_FORMAT_SIZE_
{
    CM_HDC_FORMAT_16S = 0x0,
    CM_HDC_FORMAT_8U = 0x1

} CM_FORMAT_SIZE;

#ifndef CM_EMU
/*
    sample8x8AVS(matrix<unsigned short, N, 64> &M, samplerType,  channelMask, surfIndex, samplerIndex, u, v, deltaU, deltaV, u2d,
                OutputFormatControl=0, v2d, AVSExecMode, EIFbypass=false); 
*/

template <typename RT, uint N, uint M>
CM_API extern void
cm_avs_sampler(matrix<RT, N, M> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, const float &u2d,
         int groupID = -1, short verticalBlockNumber = -1,
         OutputFormatControl outControl=CM_16_FULL, float v2d=0, AVSExecMode execMode=CM_AVS_16x4, bool IEFBypass=false )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N, uint M>
CM_API extern void
cm_avs_sampler(matrix_ref<RT, N, M> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
               const float &u, const float &v, const float &deltaU, const float &deltaV, const float &u2d,
               int groupID = -1, short verticalBlockNumber = -1,
               OutputFormatControl outControl=CM_16_FULL, float v2d=0, AVSExecMode execMode=CM_AVS_16x4, bool IEFBypass=false )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

/************MINMAXF START**********/
template <typename RT, uint N, uint M>
CM_API extern void
cm_va_min_max_filter(matrix<RT, N, M> &m, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, OutputFormatControl outControl, MMFExecMode execMode=CM_MMF_16x4, MMFEnableMode mmfEnableMode=CM_MINMAX_ENABLE )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************MINMAXF STOP***********/

/************MINMAX START**********/
template <typename RT, uint N>
CM_API extern void
cm_va_min_max(vector<RT, N> &m, SurfaceIndex surfIndex, 
         const float &u, const float &v, MMFEnableMode mmfEnableMode=CM_MINMAX_ENABLE  )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************MINMAX STOP***********/

/************CONVOLVE START**********/
template <typename RT, uint N, uint M>
CM_API extern void
cm_va_2d_convolve(matrix<RT, N, M> &m, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, CONVExecMode execMode)
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N, uint M>
CM_API extern void
cm_va_2d_convolve(matrix<RT, N, M> &m, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, CONVExecMode execMode, bool large_kernel )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************CONVOLVE STOP***********/

/************ERODE START**********/
template <typename RT, uint N>
CM_API extern void
cm_va_erode(vector<RT, N> &vect, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, EDExecMode execMode=CM_ED_64x4 )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************ERODE STOP***********/

/************DILATE START**********/
template <typename RT, uint N>
CM_API extern void
cm_va_dilate(vector<RT, N> &vect, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, EDExecMode execMode=CM_ED_64x4 )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************DILATE STOP***********/

/************CS START**********/
template <typename RT, uint N, uint M>
CM_API extern void
cm_va_boolean_centroid(matrix<RT, N, M> &m, SurfaceIndex surfIndex, 
         const float &u, const float &v, uchar vSize, uchar hSize )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************CS STOP***********/

/************Centroid START**********/
template <typename RT, uint N, uint M>
CM_API extern void
cm_va_centroid(matrix<RT, N, M> &m, SurfaceIndex surfIndex,
         const float &u, const float &v, uchar vSize )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

template <typename RT, uint N, uint M>
CM_API extern void
cm_va_centroid(matrix_ref<RT, N, M> &m, SurfaceIndex surfIndex,
               const float &u, const float &v, uchar vSize )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************Centroid STOP***********/
#else

void CM_sampler_sample8x8( SurfaceIndex surfaceIndex, SamplerIndex samplerIndex, ChannelMaskType channelMask, 
                            const float &u, const float &v, const float &deltaU, const float &deltaV, const float &u2d, void* pDataOut, 
                            int groupID = -1, short verticalBlockNumber = -1, OutputFormatControl outControl=CM_16_FULL, float v2d=0, 
                            AVSExecMode execMode=CM_AVS_16x4, bool IEFBypass=false );

template <typename RT, uint N, uint M>
CM_API extern void
cm_avs_sampler(matrix<RT, N, M> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, const float &deltaU, const float &deltaV, const float &u2d,
         int groupID = -1, short verticalBlockNumber = -1,
         OutputFormatControl outControl=CM_16_FULL, float v2d=0, AVSExecMode execMode=CM_AVS_16x4, bool IEFBypass=false )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;

    uint nWriteBackSize = 128U; 
    uint32_t* SamplerWriteback = new uint32_t[ nWriteBackSize ];
    memset( SamplerWriteback, 0, nWriteBackSize * sizeof( uint32_t ) );

    CM_sampler_sample8x8( surfIndex, samplerIndex, channelMask, u, v, deltaU, deltaV, u2d, SamplerWriteback, groupID, verticalBlockNumber, outControl, v2d, execMode, IEFBypass );

    ushort *pIndex = ( ushort * )SamplerWriteback;
    
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
    switch ( outControl )
    {
        // current Sampler.dll has channel mismatch issue, so we need to swap the channels for work around
        case CM_16_FULL:            
            for ( unsigned int iChannel = 0; iChannel < 4 * nChannels; ++iChannel )
            {
                for ( unsigned int i = iChannel / nChannels * 16; i < iChannel / nChannels * 16 + 16; ++i ) 
                {
                    m( iChannel % nChannels, i ) = *pIndex;
                    ++pIndex;
                }
            }
            break;
        default:
            break;
    }

    delete[] SamplerWriteback;
}

template <typename RT, uint N, uint M>
CM_API extern void
cm_avs_sampler(matrix_ref<RT, N, M> &m, ChannelMaskType channelMask, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
               const float &u, const float &v, const float &deltaU, const float &deltaV, const float &u2d,
               int groupID = -1, short verticalBlockNumber = -1,
               OutputFormatControl outControl=CM_16_FULL, float v2d=0, AVSExecMode execMode=CM_AVS_16x4, bool IEFBypass=false )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;

    uint nWriteBackSize = 128U; 
    uint32_t* SamplerWriteback = new uint32_t[ nWriteBackSize ];
    memset( SamplerWriteback, 0, nWriteBackSize * sizeof( uint32_t ) );

    CM_sampler_sample8x8( surfIndex, samplerIndex, channelMask, u, v, deltaU, deltaV, u2d, SamplerWriteback, groupID, verticalBlockNumber, outControl, v2d, execMode, IEFBypass );

    ushort *pIndex = ( ushort * )SamplerWriteback;
    
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
    switch ( outControl )
    {
        // current Sampler.dll has channel mismatch issue, so we need to swap the channels for work around
        case CM_16_FULL:            
            for ( unsigned int iChannel = 0; iChannel < 4 * nChannels; ++iChannel )
            {
                for ( unsigned int i = iChannel / nChannels * 16; i < iChannel / nChannels * 16 + 16; ++i ) 
                {
                    m( iChannel % nChannels, i ) = *pIndex;
                    ++pIndex;
                }
            }
            break;
        default:
            break;
    }

    delete[] SamplerWriteback;
}

/************MINMAXF START**********/
template <typename RT, uint N, uint M>
CM_API extern void
cm_va_min_max_filter(matrix<RT, N, M> &m, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, OutputFormatControl outControl, MMFExecMode execMode=CM_MMF_16x4, MMFEnableMode mmfEnableMode=CM_MINMAX_ENABLE )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "Error: sample8x8MMF is not supported in emulation mode.\n");
}
/************MINMAXF STOP***********/

/************MINMAX START**********/
template <typename RT, uint N>
CM_API extern void
cm_va_min_max(vector<RT, N> &m, SurfaceIndex surfIndex,
         const float &u, const float &v, MMFEnableMode mmfEnableMode=CM_MINMAX_ENABLE )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "Error: sample8x8MM is not supported in emulation mode.\n");
}
/************MINMAX STOP***********/

/************CONVOLVE START**********/
template <typename RT, uint N, uint M>
CM_API extern void
cm_va_2d_convolve(matrix<RT, N, M> &m, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, CONVExecMode execMode )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "Error: sample8x8CONV is not supported in emulation mode.\n");
}

template <typename RT, uint N, uint M>
CM_API extern void
cm_va_2d_convolve(matrix<RT, N, M> &m, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, CONVExecMode execMode, bool large_kernel )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "Error: sample8x8CONV is not supported in emulation mode.\n");
}
/************CONVOLVE STOP***********/

/************ERODE START**********/
template <typename RT, uint N>
CM_API extern void
cm_va_erode(vector<RT, N> &vect, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, EDExecMode execMode=CM_ED_64x4 )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "Error: sample8x8Erode is not supported in emulation mode.\n");
}
/************ERODE STOP***********/

/************DILATE START**********/
template <typename RT, uint N>
CM_API extern void
cm_va_dilate(vector<RT, N> &vect, SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, EDExecMode execMode=CM_ED_64x4 )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "Error: sample8x8Dilate is not supported in emulation mode.\n");
}
/************DILATE STOP***********/

/************CS START**********/
template <typename RT, uint N, uint M>
CM_API extern void
cm_va_boolean_centroid(matrix<RT, N, M> &m, SurfaceIndex surfIndex, 
         const float &u, const float &v, uchar vSize, uchar hSize )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "Error: sample8x8CS is not supported in emulation mode.\n");
}
/************CS STOP***********/

/************Centroid START**********/
template <typename RT, uint N, uint M>
CM_API extern void
cm_va_centroid(matrix<RT, N, M> &m, SurfaceIndex surfIndex, 
         const float &u, const float &v, uchar vSize )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "Error: sample8x8Centroid is not supported in emulation mode.\n");
}

template <typename RT, uint N, uint M>
CM_API extern void
cm_va_centroid(matrix_ref<RT, N, M> &m, SurfaceIndex surfIndex, 
               const float &u, const float &v, uchar vSize )
{
    static const bool conformable = 
        check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "Error: sample8x8Centroid is not supported in emulation mode.\n");
}
/************Centroid STOP***********/
#endif


////////////////////////////////////////////
///        SKL VA Functions Start        ///
////////////////////////////////////////////
#ifndef CM_EMU
////////////////////////////////////////////
/// 1DConvolution  SKL VA Function Start ///
////////////////////////////////////////////
template <uint N, uint M>
CM_API extern void
cm_va_1d_convolution(matrix<short, N, M> &m,
                    SurfaceIndex surfIndex, SamplerIndex sampIndex,
                    bool isHdirection, float u, float v, CONVExecMode execMode)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
////////////////////////////////////////////
///  1DConvolution  SKL VA Function End  ///
////////////////////////////////////////////

////////////////////////////////////////////
/// 1pixelConvolve SKL VA Function Start ///
////////////////////////////////////////////
template <uint N, uint M>
CM_API extern void
cm_va_1pixel_convolve(matrix<short, N, M>  &m,
                     SurfaceIndex surfIndex, SamplerIndex sampIndex,
                     float u, float v, CONVExecMode execMode, matrix<short, 1, 32> offsets)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

//1x1 mode
template <uint N, uint M>
CM_API extern void
cm_va_1pixel_convolve(matrix<short, N, M>  &m,
                     SurfaceIndex surfIndex, SamplerIndex sampIndex,
                     float u, float v, CONVExecMode execMode)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
////////////////////////////////////////////
///  1pixelConvolve SKL VA Function End  ///
////////////////////////////////////////////

////////////////////////////////////////////
///  lbpcreation SKL VA Function Start   ///
////////////////////////////////////////////
template <uint N, uint M>
CM_API extern void
cm_va_lbp_creation(matrix<uchar, N, M> &m,
                  SurfaceIndex surfIndex, 
                  float u, float v, LBPCreationExecMode execMode)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
////////////////////////////////////////////
///   lbpcreation SKL VA Function End    ///
////////////////////////////////////////////

////////////////////////////////////////////
/// lbpcorrelation SKL VA Function Start ///
////////////////////////////////////////////
template <uint N, uint M>
CM_API extern void
cm_va_lbp_correlation(matrix<uchar, N, M> &m,
                     SurfaceIndex surfIndex, 
                     float u, float v, short disparity)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
////////////////////////////////////////////
///  lbpcorrelation SKL VA Function End  ///
////////////////////////////////////////////

////////////////////////////////////////////
///   floodfill SKL VA Function Start    ///
////////////////////////////////////////////
template<typename RT>
CM_API extern void
cm_va_flood_fill(vector<RT, 8> &v,
                bool is8Connect,
                vector<ushort, 10> pixelMaskHDirection, ushort pixelMaskVDirectionLeft,
                ushort pixelMaskVDirectionRight, uchar loopCount)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
////////////////////////////////////////////
///   floodfill SKL VA Function End      ///
////////////////////////////////////////////

////////////////////////////////////////////
/// correlationSearch SKL VA Funct Start ///
////////////////////////////////////////////
template <uint N, uint M>
CM_API extern void
cm_va_correlation_search(matrix<int, N, M> &m,
                        SurfaceIndex surfIndex           , 
                        float        u                   , float        v                   ,
                        float        horizontalOrigin      , float        verticalOrigin    ,
                        uchar        xDirectionSize      , uchar        yDirectionSize      ,
                        uchar        xDirectionSearchSize, uchar        yDirectionSearchSize)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
////////////////////////////////////////////
///  correlationSearch SKL VA Funct End  ///
////////////////////////////////////////////
#else
/// TODO: Implement EMU Mode sampler8x8 VA functions for SKL here.
#endif
////////////////////////////////////////////
///         SKL VA Functions End         ///
////////////////////////////////////////////



////////////////////////////////////////////
///        SKL VA HDC Functions Start    ///
////////////////////////////////////////////
#ifndef CM_EMU
/************CONVOLVE START**********/
//template <typename RT>
CM_API extern void
cm_va_2d_convolve_hdc(SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, bool large_kernel,
         CM_FORMAT_SIZE size, SurfaceIndex destSurfIndex, const ushort &x_offset, const ushort &y_offset)
{
    static const bool conformable = 
        check_true<is_ushort_type<int>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************CONVOLVE STOP***********/

/************MINMAXF START**********/
CM_API extern void
cm_va_min_max_filter_hdc(SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, MMFEnableMode mmfEnableMode,
         CM_FORMAT_SIZE size, SurfaceIndex destSurfIndex, const ushort &x_offset, const ushort &y_offset)
{
    static const bool conformable = 
        check_true<is_ushort_type<int>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************MINMAXF STOP***********/

/************ERODE START**********/
CM_API extern void
cm_va_erode_hdc(SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, 
         SurfaceIndex destSurfIndex, const ushort &x_offset, const ushort &y_offset)
{
    static const bool conformable = 
        check_true<is_ushort_type<int>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************ERODE STOP***********/

/************DILATE START**********/
CM_API extern void
cm_va_dilate_hdc(SurfaceIndex surfIndex, SamplerIndex samplerIndex, 
         const float &u, const float &v, 
         SurfaceIndex destSurfIndex, const ushort &x_offset, const ushort &y_offset)
{
    static const bool conformable = 
        check_true<is_ushort_type<int>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
/************DILATE STOP***********/

////////////////////////////////////////////
/// 1DConvolution  SKL VA Function Start ///
////////////////////////////////////////////
CM_API extern void
cm_va_1d_convolution_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
                    bool isHdirection, float u, float v, 
                    CM_FORMAT_SIZE size, SurfaceIndex destSurfIndex, const ushort &x_offset, const ushort &y_offset)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
////////////////////////////////////////////
///  1DConvolution  SKL VA Function End  ///
////////////////////////////////////////////

////////////////////////////////////////////
/// 1pixelConvolve SKL VA Function Start ///
////////////////////////////////////////////
CM_API extern void
cm_va_1pixel_convolve_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
                     float u, float v, matrix<short, 1, 32> offsets,
                     CM_FORMAT_SIZE size, SurfaceIndex destSurfIndex, const ushort &x_offset, const ushort &y_offset)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}

////////////////////////////////////////////
///  lbpcreation SKL VA Function Start   ///
////////////////////////////////////////////
CM_API extern void
cm_va_lbp_creation_hdc(SurfaceIndex surfIndex, 
                  float u, float v, LBPCreationExecMode mode,
                  SurfaceIndex destSurfIndex, const ushort &x_offset, const ushort &y_offset)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
////////////////////////////////////////////
///   lbpcreation SKL VA Function End    ///
////////////////////////////////////////////

////////////////////////////////////////////
/// lbpcorrelation SKL VA Function Start ///
////////////////////////////////////////////
CM_API extern void
cm_va_lbp_correlation_hdc(SurfaceIndex surfIndex, 
                     float u, float v, short disparity,
                     SurfaceIndex destSurfIndex, const ushort &x_offset, const ushort &y_offset)
{   //static const bool conformable = check_true<is_ushort_type<RT>::value>::value;
    fprintf(stderr, "workarroundForIpoBugOrFeature\n");
}
////////////////////////////////////////////
///  lbpcorrelation SKL VA Function End  ///
////////////////////////////////////////////
#else
//TODO: Implement EMU Mode for HDC VA functions for SKL here.
#endif
////////////////////////////////////////////
///        SKL VA HDC Functions Start    ///
////////////////////////////////////////////
#endif

