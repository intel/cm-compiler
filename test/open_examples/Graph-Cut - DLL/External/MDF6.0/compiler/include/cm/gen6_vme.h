/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: gen6_vme.h 25847 2011-07-27 18:20:12Z kchen24 $"
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
*** Authors:             Kaiyu Chen
***                      
***                      
***                      
***
*** Description: Cm VME APIs
***
*** -----------------------------------------------------------------------------------------------
**/

#ifndef CM_GEN6_VME_H
#define CM_GEN6_VME_H

#define U62TOU8(value)          ((value >> 2) & 0x3f)
#define S72TOS16(value)         (((value >> 2) & 0x3f) | (((value >> 8) & 0x1) << 15))
#define S15TOS16(value)         ((value & 0x3fff) | (((value >> 14) & 0x1) << 15))

#define S16S72(value)           ((value & 0x7f) << 2 | (((value >> 15) & 0x1) << 9))
#define S16S15(value)           ((value & 0x7fff) | (((value >> 15) & 0x1) << 15))

#define nNUM_INPUT_MRF    4
#define nNUM_OUTPUT_GRF   4
#define nNUM_OUTPUT_GRF_SHORT 1

typedef matrix<uint1, nNUM_INPUT_MRF, 32> vme_InputMrfType;
typedef matrix<uint1, nNUM_OUTPUT_GRF, 32> vme_OutputGrfType;
typedef matrix<uint1, nNUM_OUTPUT_GRF_SHORT, 32> vme_OutputGrfShortType;

// Select one element from the input payload 
#define VME_Input_S1(p, t, r, c) p.format<t, nNUM_INPUT_MRF, 32/sizeof(t)>().select<1,1,1,1>(r,c)
#define VME_Input_G1(p, t, r, c) p.format<t, nNUM_INPUT_MRF, 32/sizeof(t)>().select<1,1,1,1>(r,c)(0,0)

// Select one element from the output result
#define VME_Output_S1(p, t, r, c) p.row(r).format<t>().select<1,1>(c)(0)

// INPUT 

#ifndef CM_EMU  // Non-emulation mode begin

#define ST_SRC_16X16 0
#define ST_SRC_16X8  1
// #define ST_SRC_8X16  2   // HW n/a
#define ST_SRC_8X8   3
#define VME_SET_SrcType_SrcBlockSize(p, v) (VME_Input_S1(p, uint1, 0, 12) =  VME_Input_S1(p, uint1, 0, 12) & 0xFC | v)

#define ST_SRC_FRAME 0x00
#define ST_SRC_FIELD 0x40
#define VME_SET_SrcType_SrcBlockStructure(p, v) (VME_Input_S1(p, uint1, 0, 12) =  VME_Input_S1(p, uint1, 0, 12) & 0xBF | v)

#define ST_REF_FRAME 0x00
#define ST_REF_FIELD 0x80
#define VME_SET_SrcType_RefWindowStructure(p, v) (VME_Input_S1(p, uint1, 0, 12) =  VME_Input_S1(p, uint1, 0, 12) & 0x7F | v)

#define VM_MODE_SSS 0
#define VM_MODE_DSS 1
#define VM_MODE_DDS 3
#define VM_MODE_DDD 7
#define VME_SET_VmeModes_RunningMode(p, v) (VME_Input_S1(p, uint1, 0, 13) =  VME_Input_S1(p, uint1, 0, 13) & 0xF8 | v)

#define VM_2PATH_ENABLE 8
#define VME_SET_VmeModes_DualSearchPathMode(p) (VME_Input_S1(p, uint1, 0, 13) |= 0x08)
#define VME_CLEAR_VmeModes_DualSearchPathMode(p) (VME_Input_S1(p, uint1, 0, 13) &= 0xF7)

#define VM_INTEGER_PEL 0x00
#define VM_HALF_PEL 0x10
#define VM_QUARTER_PEL 0x30
#define VME_SET_VmeModes_SubPelMode(p, v) (VME_Input_S1(p, uint1, 0, 13) =  VME_Input_S1(p, uint1, 0, 13) & 0xCF | v)

#define VME_SET_DisableAlignedVMEReferenceFetch(p) (VME_Input_S1(p, uint1, 0, 14) |= 0x1)
#define VME_CLEAR_DisableAlignedVMEReferenceFetch(p) (VME_Input_S1(p, uint1, 0, 14) &= 0xFE)

#define VME_SET_DisableAlignedVMESourceFetch(p) (VME_Input_S1(p, uint1, 0, 14) |= 0x2)
#define VME_CLEAR_DisableAlignedVMESourceFetch(p) (VME_Input_S1(p, uint1, 0, 14) &= 0xFD)

#define DIST_INTRA_SAD 0
// #define DIST_INTRA_MSE 0x40 // HW n/a
#define DIST_INTRA_HAAR 0x80
// #define DIST_INTRA_HADAMARD 0xC0 // HW n/a
#define VME_SET_SadType_IntraDistMetric(p, v) (VME_Input_S1(p, uint1, 0, 14) =  VME_Input_S1(p, uint1, 0, 14) & 0x3F | v)

#define DIST_INTER_SAD 0
// #define DIST_INTER_MSE 0x10 // HW n/a
#define DIST_INTER_HAAR 0x20
// #define DIST_INTER_HADAMARD 0x30 // HW n/a
#define VME_SET_SadType_InterDistMetric(p, v) (VME_Input_S1(p, uint1, 0, 14) =  VME_Input_S1(p, uint1, 0, 14) & 0xCF | v)

#define ST_SKIPMV_CHECK_DISABLED 4 
#define VME_SET_SadType_SkipBoundaryCheckDis(p) (VME_Input_S1(p, uint1, 0, 14) |= 0x4)
#define VME_CLEAR_SadType_SkipBoundaryCheckDis(p) (VME_Input_S1(p, uint1, 0, 14) &= 0xFB)

// v can be a combination of the following
#define SHP_NO_16X16  0x01
#define SHP_NO_16X8   0x02
#define SHP_NO_8X16   0x04
#define SHP_NO_8X8    0x08
#define SHP_NO_8X4    0x10
#define SHP_NO_4X8    0x20
#define SHP_NO_4X4    0x40
#define SHP_BIDIR_AVS 0x80  // only for AVS
#define VME_SET_ShapeMask(p, v) (VME_Input_S1(p, uint1, 0, 15) = v) 

// The value must be a multiple of 4. Range = [20, 64]
#define VME_SET_RefW(p, v) (VME_Input_S1(p, uint1, 0, 22) = v)

// The value must be a multiple of 4. Range = [20, 64]
#define VME_SET_RefH(p, v) (VME_Input_S1(p, uint1, 0, 23) = v)

// VmeFlags
// #define VF_PB_SKIP_ENABLE 0x01
#define VME_SET_VmeFlags_SkipModeEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x01)
#define VME_CLEAR_VmeFlags_SkipModeEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xFE)

// #define VF_ADAPTIVE_ENABLE 0x02
#define VME_SET_VmeFlags_AdaptiveEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x02)
#define VME_CLEAR_VmeFlags_AdaptiveEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xFD)

// #define VF_BIMIX_DISABLE 0x04
#define VME_SET_VmeFlags_BiMixDis(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x04)
#define VME_CLEAR_VmeFlags_BiMixDis(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xFB)

// #define VF_EXTRA_CANDIDATE 0x08
#define VME_SET_VmeFlags_ExtraCandidateEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x08)  
#define VME_CLEAR_VmeFlags_ExtraCandidateEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xF7)  

// #define VF_EARLY_SUCCESS 0x10
#define VME_SET_VmeFlags_EarlySuccessEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x10)
#define VME_CLEAR_VmeFlags_EarlySuccessEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xEF)

// #define VF_IME_SUCCESS 0x20
#define VME_SET_VmeFlags_ImeSuccessEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x20)  
#define VME_CLEAR_VmeFlags_ImeSuccessEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0xDF)

// #define VF_QUITINTER_ENABLE 0x40
#define VME_SET_VmeFlags_QuitInterEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x40)
#define VME_CLEAR_VmeFlags_QuitInterEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0xBF)

// #define VF_T8X8_FOR_INTER 0x80  
#define VME_SET_VmeFlags_T8x8FlagForInterEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x80)  
#define VME_CLEAR_VmeFlags_T8x8FlagForInterEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0x7F) 

// Value type = U4PAIR 
#define VME_SET_EarlySkipSuc(p, shift_count, shift_val) (VME_Input_S1(p, uint1, 1, 1) = shift_val | (shift_count << 4))
#define VME_SET_EarlyFmeExit(p, shift_count, shift_val) (VME_Input_S1(p, uint1, 1, 2) = shift_val | (shift_count << 4))
#define VME_SET_EarlyImeStop(p, shift_count, shift_val) (VME_Input_S1(p, uint1, 1, 3) = shift_val | (shift_count << 4))

// Number of bits = 6; Range = [4, 32]
#define VME_SET_MaxMvs(p, v) (VME_Input_S1(p, uint1, 1, 4) = v)   

// Number of bits =6; Range = [16, 21, 32, 43, 48]
#define VME_SET_BiWeight(p, v) (VME_Input_S1(p, uint1, 1, 6) = v) 

// v can be a combination of the following
#define BID_NO_16X16 0x01
#define BID_NO_16X8_8X16 0x02
#define BID_NO_16X8 0x02
#define BID_NO_8X16 0x02
#define BID_NO_8X8 0x04
#define BID_NO_MINORS 0x08
#define BID_NO_SubMbPartMaskALL 0x0F 
#define VME_SET_BiDirMask_BiSubMbPartMask(p, v) (VME_Input_S1(p, uint1, 1, 7) |= v) 
#define VME_CLEAR_BiDirMask_BiSubMbPartMask(p, v) (VME_Input_S1(p, uint1, 1, 7) &= (~v))  

#define UNIDIR_NO_MIX 0x10
#define VME_SET_BiDirMask_UniMixDisable(p) (VME_Input_S1(p, uint1, 1, 7) |= 0x10) 
#define VME_CLEAR_BiDirMask_UniMixDisable(p) (VME_Input_S1(p, uint1, 1, 7) &= 0xEF) 

#define B_AdaptiveValidationControl 0x20
#define VME_SET_BiDirMask_AdaptiveValidationControl(p) (VME_Input_S1(p, uint1, 1, 7) |= 0x20) 
#define VME_CLEAR_BiDirMask_AdaptiveValidationControl(p) (VME_Input_S1(p, uint1, 1, 7) &= 0xDF) 

#define B_PRUNING_ENABLE 0x40  
#define VME_SET_BiDirMask_FBPrunEn(p) (VME_Input_S1(p, uint1, 1, 7) |= 0x40) 
#define VME_CLEAR_BiDirMask_FBPrunEn(p) (VME_Input_S1(p, uint1, 1, 7) &= 0xBF)  

#define B_RESHAPE_ENABLE 0x80
#define VME_SET_BiDirMask_ReshapeEnable(p) (VME_Input_S1(p, uint1, 1, 7) |= 0x80) 
#define VME_CLEAR_BiDirMask_ReshapeEnable(p) (VME_Input_S1(p, uint1, 1, 7) &= 0x7F) 

#define VME_SET_MaxLenSP(p, v) (VME_Input_S1(p, uint1, 1, 8) = v)
#define VME_SET_MaxNumSU(p, v) (VME_Input_S1(p, uint1, 1, 9) = v)

#define VME_SET_SPCenter0(p, X, Y) (VME_Input_S1(p, uint1, 1, 10) = X | (Y << 4)) 
#define VME_SET_SPCenter1(p, X, Y) (VME_Input_S1(p, uint1, 1, 11) = X | (Y << 4))

// Value type = U4PAIR 
#define VME_SET_BPrunTolerance(p, shift_count, shift_val) (VME_Input_S1(p, uint1, 1, 12) = shift_val | (shift_count << 4)) 

// Value type = U4PAIR
#define VME_SET_ExtraTolerance(p, shift_count, shift_val) (VME_Input_S1(p, uint1, 1, 13) = shift_val | (shift_count << 4))

// Value type = U4PAIR
#define VME_SET_ImeTooBad(p, shift_count, shift_val) (VME_Input_S1(p, uint1, 1, 14) = shift_val | (shift_count << 4))

// Value type = U4PAIR
#define VME_SET_ImeTooGood(p, shift_count, shift_val) (VME_Input_S1(p, uint1, 1, 15) = shift_val | (shift_count << 4))

// Number of bits = 10, Value type = I16, Format = S7.2
#define VME_SET_CostCenter0DeltaX(p, v) (VME_Input_S1(p, int2, 1, 8) = v)
#define VME_SET_CostCenter0DeltaY(p, v) (VME_Input_S1(p, int2, 1, 9) = v)
#define VME_SET_CostCenter1DeltaX(p, v) (VME_Input_S1(p, int2, 1, 10) = v)
#define VME_SET_CostCenter1DeltaY(p, v) (VME_Input_S1(p, int2, 1, 11)= v)

// Value type = U8, Format = U6.2
#define VME_SET_SkipCenterDelta00X(p, v) (VME_Input_S1(p, uint1, 1, 24) = v) 
#define VME_SET_SkipCenterDelta00Y(p, v) (VME_Input_S1(p, uint1, 1, 25) = v) 
#define VME_SET_SkipCenterDelta01X(p, v) (VME_Input_S1(p, uint1, 1, 26) = v) 
#define VME_SET_SkipCenterDelta01Y(p, v) (VME_Input_S1(p, uint1, 1, 27) = v) 

// v can be a combination of the following
#define INTRA_PARTITION_16X16 0x01
#define INTRA_PARTITION_8X8 0x02
#define INTRA_PARTITION_4X4 0x04
#define INTRA_PARTITION_NONAVS8X8 0x08
#define INTRA_PARTITION_NONAVS4X4 0x10
#define INTRA_PARTITION_ALL 0x1F
#define VME_SET_IntraFlags_IntraPartitionMaskDis(p, v) (VME_Input_S1(p, uint1, 1, 28) |= v) 
#define VME_CLEAR_IntraFlags_IntraPartitionMaskDis(p, v) (VME_Input_S1(p, uint1, 1, 28) &= (~v)) 

// #define NONSKIP_ZMV_ADDED 0x20  
#define VME_SET_IntraFlags_NonSkipZMVAdded(p) (VME_Input_S1(p, uint1, 1, 28) |= 0x20)
#define VME_CLEAR_IntraFlags_NonSkipZMVAdded(p) (VME_Input_S1(p, uint1, 1, 28) &= 0xDF) 

// #define NONSKIP_MODE_ADDED 0x40
#define VME_SET_IntraFlags_NonSkipModeAdded(p) (VME_Input_S1(p, uint1, 1, 28) |= 0x40)  
#define VME_CLEAR_IntraFlags_NonSkipModeAdded(p) (VME_Input_S1(p, uint1, 1, 28) &= 0xBF)  

// #define INTRA_TBSWAP_FLAG 0x80
#define VME_SET_IntraFlags_IntraCornerSwap(p) (VME_Input_S1(p, uint1, 1, 28) |= 0x80)  
#define VME_CLEAR_IntraFlags_IntraCornerSwap(p) (VME_Input_S1(p, uint1, 1, 28) &= 0x7F)

#define INTRA_AVAIL_F 0x80 
#define INTRA_AVAIL_E 0x40 
#define INTRA_AVAIL_A 0x20 
#define INTRA_AVAIL_B 0x10
#define INTRA_AVAIL_C 0x08 
#define INTRA_AVAIL_D 0x04
#define VME_SET_IntraAvail(p, v) (VME_Input_S1(p, uint1, 1, 29) = v) 

// Number of bits = 4
#define VME_SET_LeftModes_IntraMxMPredModeA5(p, v) (VME_Input_S1(p, uint1, 3, 16) = VME_Input_S1(p, uint1, 3, 16)  & 0xF0 | v) 
#define VME_SET_LeftModes_IntraMxMPredModeA7(p, v) (VME_Input_S1(p, uint1, 3, 16) = VME_Input_S1(p, uint1, 3, 16) & 0x0F | (v << 4))  
#define VME_SET_LeftModes_IntraMxMPredModeA13(p, v) (VME_Input_S1(p, uint1, 3, 17) = VME_Input_S1(p, uint1, 3, 17) & 0xF0 | v) 
#define VME_SET_LeftModes_IntraMxMPredModeA15(p, v) (VME_Input_S1(p, uint1, 3, 17) = VME_Input_S1(p, uint1, 3, 17) & 0x0F | (v << 4))  

// Number of bits = 4
#define VME_SET_TopModes_IntraMxMPredModeB10(p, v) (VME_Input_S1(p, uint1, 3, 18) = VME_Input_S1(p, uint1, 3, 18) & 0xF0 | v)  
#define VME_SET_TopModes_IntraMxMPredModeB11(p, v) (VME_Input_S1(p, uint1, 3, 18) = VME_Input_S1(p, uint1, 3, 18) & 0x0F | (v << 4)) 
#define VME_SET_TopModes_IntraMxMPredModeB14(p, v) (VME_Input_S1(p, uint1, 3, 19) = VME_Input_S1(p, uint1, 3, 19) & 0xF0 | v)  
#define VME_SET_TopModes_IntraMxMPredModeB15(p, v) (VME_Input_S1(p, uint1, 3, 19) = VME_Input_S1(p, uint1, 3, 19) & 0x0F | (v << 4))   

// Value type = U8, Format = U6.2
#define VME_SET_SkipCenter1(p, Delta10X, Delta10Y, Delta11X, Delta11Y) (VME_Input_S1(p, uint, 3, 5) = Delta10X | (Delta10Y << 8) | (Delta11X << 16) | (Delta11Y << 24)) 
#define VME_SET_SkipCenter2(p, Delta20X, Delta20Y, Delta21X, Delta21Y) (VME_Input_S1(p, uint, 3, 6) = Delta20X | (Delta20Y << 8) | (Delta21X << 16) | (Delta21Y << 24)) 
#define VME_SET_SkipCenter3(p, Delta30X, Delta30Y, Delta31X, Delta31Y) (VME_Input_S1(p, uint, 3, 7) = Delta30X | (Delta30Y << 8) | (Delta31X << 16) | (Delta31Y << 24)) 

#else   // Non-emulation mode end

// Emulation mode begin: Check if the input value is valid

#define ST_SRC_16X16 0x00
#define ST_SRC_16X8  0x01
// #define ST_SRC_8X16  0x02   // HW n/a
#define ST_SRC_8X8   0x03
#define VME_SET_SrcType_SrcBlockSize(p, v) { \
        if((v != 0) && (v != 1) && (v != 3)) { \
            printf("Warning: Unexpected input value for VME_SET_SrcType_SrcBlockSize(p, v) - v can only be 0x0, 0x1 or 0x3!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 12) =  VME_Input_S1(p, uint1, 0, 12) & 0xFC | v; \
    }

#define ST_SRC_FRAME 0x00
#define ST_SRC_FIELD 0x40
#define VME_SET_SrcType_SrcBlockStructure(p, v) { \
        if((v != 0) && (v != 0x40)) { \
            printf("Warning: Unexpected input value for VME_SET_SrcType_SrcBlockStructure(p, v) - v can only be 0x0 or 0x40!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 12) =  VME_Input_S1(p, uint1, 0, 12) & 0xBF | v; \
    }

#define ST_REF_FRAME 0x00
#define ST_REF_FIELD 0x80
#define VME_SET_SrcType_RefWindowStructure(p, v) { \
        if((v != 0) && (v != 0x80)) { \
            printf("Warning: Unexpected input value for VME_SET_SrcType_RefWindowStructure(p, v) - v can only be 0x0 or 0x80!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 12) =  VME_Input_S1(p, uint1, 0, 12) & 0x7F | v; \
    }

#define VM_MODE_SSS 0x00
#define VM_MODE_DSS 0x01
#define VM_MODE_DDS 0x03
#define VM_MODE_DDD 0x07
#define VME_SET_VmeModes_RunningMode(p, v) { \
        if((v != 0) && (v != 1) && (v != 3) && (v != 7)) { \
            printf("Warning: Unexpected input value for VME_SET_VmeModes_RunningMode(p, v) - v can only be 0x0, 0x1, 0x3 or 0x7!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 13) =  VME_Input_S1(p, uint1, 0, 13) & 0xF8 | v; \
    }

#define VM_2PATH_ENABLE 0x08
#define VME_SET_VmeModes_DualSearchPathMode(p) (VME_Input_S1(p, uint1, 0, 13) |= 0x08)
#define VME_CLEAR_VmeModes_DualSearchPathMode(p) (VME_Input_S1(p, uint1, 0, 13) &= 0xF7)

#define VME_SET_DisableAlignedVMEReferenceFetch(p) (VME_Input_S1(p, uint1, 0, 14) |= 0x1)
#define VME_CLEAR_DisableAlignedVMEReferenceFetch(p) (VME_Input_S1(p, uint1, 0, 14) &= 0xFE)

#define VME_SET_DisableAlignedVMESourceFetch(p) (VME_Input_S1(p, uint1, 0, 14) |= 0x2)
#define VME_CLEAR_DisableAlignedVMESourceFetch(p) (VME_Input_S1(p, uint1, 0, 14) &= 0xFD)

#define VM_INTEGER_PEL 0x00
#define VM_HALF_PEL 0x10
#define VM_QUARTER_PEL 0x30
#define VME_SET_VmeModes_SubPelMode(p, v) { \
        if((v != 0) && (v != 0x10) && (v != 0x30)) { \
            printf("Warning: Unexpected input value for VME_SET_VmeModes_SubPelMode(p, v) - v can only be 0x0, 0x10 or 0x30!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 13) =  VME_Input_S1(p, uint1, 0, 13) & 0xCF | v; \
    }

#define DIST_INTRA_SAD 0
// #define DIST_INTRA_MSE 0x40 // HW n/a
#define DIST_INTRA_HAAR 0x80
// #define DIST_INTRA_HADAMARD 0xC0 // HW n/a
#define VME_SET_SadType_IntraDistMetric(p, v) { \
        if((v != 0) && (v != 0x80)) { \
            printf("Warning: Unexpected input value for VME_SET_SadType_IntraDistMetric(p, v) - v can only be 0x0 or 0x80!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 14) =  VME_Input_S1(p, uint1, 0, 14) & 0x3F | v; \
    }

#define DIST_INTER_SAD 0
// #define DIST_INTER_MSE 0x10 // HW n/a
#define DIST_INTER_HAAR 0x20
// #define DIST_INTER_HADAMARD 0x30 // HW n/a
#define VME_SET_SadType_InterDistMetric(p, v) { \
        if((v != 0) && (v != 0x20)) { \
            printf("Warning: Unexpected input value for VME_SET_SadType_InterDistMetric(p, v) - v can only be 0x0 or 0x20!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 14) =  VME_Input_S1(p, uint1, 0, 14) & 0xCF | v; \
    }

// #define BLOCK_BASED_SKIP_EN 8 
// #define VME_SET_SadType_BlockBasedSkipEn(p, v) (VME_Input_S1(p, uint1, 0, 14) =  VME_Input_S1(p, uint1, 0, 14) & 0xF7 | v)

#define ST_SKIPMV_CHECK_DISABLED 4 
#define VME_SET_SadType_SkipBoundaryCheckDis(p) (VME_Input_S1(p, uint1, 0, 14) |= 0x4)
#define VME_CLEAR_SadType_SkipBoundaryCheckDis(p) (VME_Input_S1(p, uint1, 0, 14) &= 0xFB)

// #define ALIGNED_SOURCE_FETCH_DIS 2 
// #define VME_SET_SadType_AlignedSourceFetchDis(p, v) (VME_Input_S1(p, uint1, 0, 14) =  VME_Input_S1(p, uint1, 0, 14) & 0xFD | v)

// #define ALIGNED_REFERENCE_FETCH_DIS 1
// #define VME_SET_SadType_AlignedReferenceFetchDis(p, v) (VME_Input_S1(p, uint1, 0, 14) =  VME_Input_S1(p, uint1, 0, 14) & 0xFE | v)

// v can be a combination of the following values
#define SHP_NO_16X16  0x01
#define SHP_NO_16X8   0x02
#define SHP_NO_8X16   0x04
#define SHP_NO_8X8    0x08
#define SHP_NO_8X4    0x10
#define SHP_NO_4X8    0x20
#define SHP_NO_4X4    0x40
#define SHP_BIDIR_AVS 0x80  // Only for AVS
#define VME_SET_ShapeMask(p, v) { \
        if(v > 255) { \
            printf("Warning: Unexpected input value for VME_SET_ShapeMask(p, v) - v can only be a combination of 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40 or 0x80!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 15) = v; \
    }

// The value must be a multiple of 4. Range = [20, 64]
#define VME_SET_RefW(p, v) { \
        if((v%4 != 0) || (v < 20) || (v > 64)) { \
            printf("Warning: Unexpected input value for VME_SET_RefW(p, v) - v can only be a multiple of 4 and in the range [20, 64]!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 22) = v; \
    }

// The value must be a multiple of 4. Range = [20, 64]
#define VME_SET_RefH(p, v) { \
        if((v%4 != 0) || (v < 20) || (v > 64)) { \
            printf("Warning: Unexpected input value for VME_SET_RefH(p, v) - v can only be a multiple of 4 and in the range [20, 64]!\n"); \
        } \
        VME_Input_S1(p, uint1, 0, 23) = v; \
    }

// VmeFlags
// #define VF_PB_SKIP_ENABLE 0x01
#define VME_SET_VmeFlags_SkipModeEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x01)
#define VME_CLEAR_VmeFlags_SkipModeEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xFE)

// #define VF_ADAPTIVE_ENABLE 0x02
#define VME_SET_VmeFlags_AdaptiveEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x02)
#define VME_CLEAR_VmeFlags_AdaptiveEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xFD)

// #define VF_BIMIX_DISABLE 0x04
#define VME_SET_VmeFlags_BiMixDis(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x04)
#define VME_CLEAR_VmeFlags_BiMixDis(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xFB)

// #define VF_EXTRA_CANDIDATE 0x08
#define VME_SET_VmeFlags_ExtraCandidateEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x08)  
#define VME_CLEAR_VmeFlags_ExtraCandidateEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xF7)  

// #define VF_EARLY_SUCCESS 0x10
#define VME_SET_VmeFlags_EarlySuccessEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x10)
#define VME_CLEAR_VmeFlags_EarlySuccessEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0xEF)

// #define VF_IME_SUCCESS 0x20
#define VME_SET_VmeFlags_ImeSuccessEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x20)  
#define VME_CLEAR_VmeFlags_ImeSuccessEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0xDF)

// #define VF_QUITINTER_ENABLE 0x40
#define VME_SET_VmeFlags_QuitInterEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x40)
#define VME_CLEAR_VmeFlags_QuitInterEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0xBF)

// #define VF_T8X8_FOR_INTER 0x80  
#define VME_SET_VmeFlags_T8x8FlagForInterEn(p) (VME_Input_S1(p, uint1, 1, 0) |= 0x80)  
#define VME_CLEAR_VmeFlags_T8x8FlagForInterEn(p) (VME_Input_S1(p, uint1, 1, 0) &= 0x7F) 

// Value type = U4PAIR 
#define VME_SET_EarlySkipSuc(p, shift_count, shift_val) { \
        if((shift_count > 0xF) || (shift_val > 0xF)) { \
            printf("Warning: Unexpected input value for VME_SET_EarlySkipSuc(p, shift_count, shift_val) - shift_count and shift_val should fit in 4 bits!\n"); \
        } \
        if((shift_val << shift_count) > 0xFFF) { \
            printf("Warning: Unexpected input value for VME_SET_EarlySkipSuc(p, shift_count, shift_val) - the encoded value should fit in 12 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 1) = shift_val | (shift_count << 4); \
    }


#define VME_SET_EarlyFmeExit(p, shift_count, shift_val) { \
        if((shift_count > 0xF) || (shift_val > 0xF)) { \
            printf("Warning: Unexpected input value for VME_SET_EarlyFmeExit(p, shift_count, shift_val) - shift_count and shift_val should fit in 4 bits!\n"); \
        } \
        if((shift_val << shift_count) > 0xFFF) { \
            printf("Warning: Unexpected input value for VME_SET_EarlyFmeExit(p, shift_count, shift_val) - the encoded value should fit in 12 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 2) = shift_val | (shift_count << 4); \
    }

#define VME_SET_EarlyImeStop(p, shift_count, shift_val) { \
        if((shift_count > 0xF) || (shift_val > 0xF)) { \
            printf("Warning: Unexpected input value for VME_SET_EarlyImeStop(p, shift_count, shift_val) - shift_count and shift_val should fit in 4 bits!\n"); \
        } \
        if((shift_val << shift_count) > 0xFFF) { \
            printf("Warning: Unexpected input value for VME_SET_EarlyImeStop(p, shift_count, shift_val) - the encoded value should fit in 12 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 3) = shift_val | (shift_count << 4); \
    }

// Number of bits = 6; Range = [4, 32]
#define VME_SET_MaxMvs(p, v) { \
        if((v < 4) || (v > 32)) { \
            printf("Warning: Unexpected input value for VME_SET_MaxMvs(p, v) - v should be in the range [4, 32]!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 4) = v; \
    }

// Number of bits =6; Range = [16, 21, 32, 43, 48]
#define VME_SET_BiWeight(p, v) { \
        if((v != 16) && (v != 21) && (v != 32) && (v != 43) && (v != 48)) { \
            printf("Warning: Unexpected input value for VME_SET_BiWeight(p, v) - v can only be 16, 21, 32, 43 or 48!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 6) = v; \
    }

// v can be a combination of the following
#define BID_NO_16X16 0x01
#define BID_NO_16X8_8X16 0x02
#define BID_NO_16X8 0x02
#define BID_NO_8X16 0x02
#define BID_NO_8X8 0x04
#define BID_NO_MINORS 0x08
#define BID_NO_SubMbPartMaskALL 0x0F  // allows all bits to be cleared
#define VME_SET_BiDirMask_BiSubMbPartMask(p, v) { \
        if(v > 15) { \
            printf("Warning: Unexpected input value for VME_SET_BiDirMask_BiSubMbPartMask(p, v) - v can only be a combination of 0x01, 0x02, 0x04 or 0x08!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 7) |= v; \
    }
#define VME_CLEAR_BiDirMask_BiSubMbPartMask(p, v) { \
        if(v > 15) { \
            printf("Warning: Unexpected input value for VME_CLEAR_BiDirMask_BiSubMbPartMask(p, v) - v can only be a combination of 0x01, 0x02, 0x04 or 0x08!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 7) &= (~v); \
    }

#define UNIDIR_NO_MIX 0x10
#define VME_SET_BiDirMask_UniMixDisable(p) (VME_Input_S1(p, uint1, 1, 7) |= 0x10) 
#define VME_CLEAR_BiDirMask_UniMixDisable(p) (VME_Input_S1(p, uint1, 1, 7) &= 0xEF) 

#define AdaptiveValidationControl 0x20
#define VME_SET_BiDirMask_AdaptiveValidationControl(p) (VME_Input_S1(p, uint1, 1, 7) |= 0x20) 
#define VME_CLEAR_BiDirMask_AdaptiveValidationControl(p) (VME_Input_S1(p, uint1, 1, 7) &= 0xDF) 

#define B_PRUNING_ENABLE 0x40  
#define VME_SET_BiDirMask_FBPrunEn(p) (VME_Input_S1(p, uint1, 1, 7) |= 0x40) 
#define VME_CLEAR_BiDirMask_FBPrunEn(p) (VME_Input_S1(p, uint1, 1, 7) &= 0xBF)  

#define B_RESHAPE_ENABLE 0x80
#define VME_SET_BiDirMask_ReshapeEnable(p) (VME_Input_S1(p, uint1, 1, 7) |= 0x80) 
#define VME_CLEAR_BiDirMask_ReshapeEnable(p) (VME_Input_S1(p, uint1, 1, 7) &= 0x7F) 

#define VME_SET_MaxLenSP(p, v) { \
        if((v < 1) || (v > 63)) { \
            printf("Warning: Unexpected input value for VME_SET_MaxLenSP(p, v) - v should be in the range [1, 63]!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 8) = v; \
    }

#define VME_SET_MaxNumSU(p, v) { \
        if((v < 1) || (v > 63)) { \
            printf("Warning: Unexpected input value for VME_SET_MaxNumSU(p, v) - v should be in the range [1, 63]!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 9) = v; \
    }

#define VME_SET_SPCenter0(p, X, Y) { \
        if((X > 0xF) || (Y > 0xF)) { \
            printf("Warning: Unexpected input value for VME_SET_SPCenter0(p, X, Y) - X and Y should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 10) = X | (Y << 4); \
    }

#define VME_SET_SPCenter1(p, X, Y) { \
        if((X > 0xF) || (Y > 0xF)) { \
            printf("Warning: Unexpected input value for VME_SET_SPCenter1(p, X, Y) - X and Y should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 11) = X | (Y << 4); \
    }

// Value type = U4PAIR 
#define VME_SET_BPrunTolerance(p, shift_count, shift_val) { \
        if((shift_count > 0xF) || (shift_val > 0xF)) { \
            printf("Warning: Unexpected input value for VME_SET_BPrunTolerance(p, shift_count, shift_val) - shift_count and shift_val should fit in 4 bits!\n"); \
        } \
        if((shift_val << shift_count) > 0xFFF) { \
            printf("Warning: Unexpected input value for VME_SET_BPrunTolerance(p, shift_count, shift_val) - the encoded value should fit in 12 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 12) = shift_val | (shift_count << 4); \
    }

// Value type = U4PAIR
#define VME_SET_ExtraTolerance(p, shift_count, shift_val) { \
        if((shift_count > 0xF) || (shift_val > 0xF)) { \
            printf("Warning: Unexpected input value for VME_SET_ExtraTolerance(p, shift_count, shift_val) - shift_count and shift_val should fit in 4 bits!\n"); \
        } \
        if((shift_val << shift_count) > 0xFFF) { \
            printf("Warning: Unexpected input value for VME_SET_ExtraTolerance(p, shift_count, shift_val) - the encoded value should fit in 12 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 13) = shift_val | (shift_count << 4); \
    }

// Value type = U4PAIR
#define VME_SET_ImeTooBad(p, shift_count, shift_val)  { \
        if((shift_count > 0xF) || (shift_val > 0xF)) { \
            printf("Warning: Unexpected input value for VME_SET_ImeTooBad(p, shift_count, shift_val) - shift_count and shift_val should fit in 4 bits!\n"); \
        } \
        if((shift_val << shift_count) > 0xFFF) { \
            printf("Warning: Unexpected input value for VME_SET_ImeTooBad(p, shift_count, shift_val) - the encoded value should fit in 12 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 14) = shift_val | (shift_count << 4); \
    }

// Value type = U4PAIR
#define VME_SET_ImeTooGood(p, shift_count, shift_val) { \
        if((shift_count > 0xF) || (shift_val > 0xF)) { \
            printf("Warning: Unexpected input value for VME_SET_ImeTooGood(p, shift_count, shift_val) - shift_count and shift_val should fit in 4 bits!\n"); \
        } \
        if((shift_val << shift_count) > 0xFFF) { \
            printf("Warning: Unexpected input value for VME_SET_ImeTooGood(p, shift_count, shift_val) - the encoded value should fit in 12 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 15) = shift_val | (shift_count << 4); \
    }

// Number of bits = 10, Value type = I16, Format = S7.2
#define VME_SET_CostCenter0DeltaX(p, v) { \
        if(v > 0x3FF) { \
            printf("Warning: Unexpected input value for VME_SET_CostCenter0DeltaX(p, v) - v should fit in 10 bits!\n"); \
        } \
        VME_Input_S1(p, int2, 1, 8) = v; \
    }

#define VME_SET_CostCenter0DeltaY(p, v) { \
        if(v > 0x3FF) { \
            printf("Warning: Unexpected input value for VME_SET_CostCenter0DeltaY(p, v) - v should fit in 10 bits!\n"); \
        } \
        VME_Input_S1(p, int2, 1, 9) = v; \
    }

#define VME_SET_CostCenter1DeltaX(p, v) { \
        if(v > 0x3FF) { \
            printf("Warning: Unexpected input value for VME_SET_CostCenter1DeltaX(p, v) - v should fit in 10 bits!\n"); \
        } \
        VME_Input_S1(p, int2, 1, 10) = v; \
    }

#define VME_SET_CostCenter1DeltaY(p, v) { \
        if(v > 0x3FF) { \
            printf("Warning: Unexpected input value for VME_SET_CostCenter1DeltaY(p, v) - v should fit in 10 bits!\n"); \
        } \
        VME_Input_S1(p, int2, 1, 11)= v; \
    }

// Value type = U8, Format = U6.2
#define VME_SET_SkipCenterDelta00X(p, v) (VME_Input_S1(p, uint1, 1, 24) = v) 
#define VME_SET_SkipCenterDelta00Y(p, v) (VME_Input_S1(p, uint1, 1, 25) = v) 
#define VME_SET_SkipCenterDelta01X(p, v) (VME_Input_S1(p, uint1, 1, 26) = v) 
#define VME_SET_SkipCenterDelta01Y(p, v) (VME_Input_S1(p, uint1, 1, 27) = v) 

// v can be a combination of the following
#define INTRA_PARTITION_16X16 0x01
#define INTRA_PARTITION_8X8 0x02
#define INTRA_PARTITION_4X4 0x04
#define INTRA_PARTITION_NONAVS8X8 0x08
#define INTRA_PARTITION_NONAVS4X4 0x10
#define INTRA_PARTITION_ALL 0x1F
#define VME_SET_IntraFlags_IntraPartitionMaskDis(p, v) { \
        if(v > 0x1F) { \
            printf("Warning: Unexpected input value for VME_SET_IntraFlags_IntraPartitionMaskDis(p, v) - v can only be a combination of 0x01, 0x02, 0x04, 0x08 or 0x10!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 28) |= v; \
    }
#define VME_CLEAR_IntraFlags_IntraPartitionMaskDis(p, v) { \
        if(v > 0x1F) { \
            printf("Warning: Unexpected input value for VME_SET_IntraFlags_IntraPartitionMaskDis(p, v) - v can only be a combination of 0x01, 0x02, 0x04, 0x08 or 0x10!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 28) &= (~v); \
    }

#define VME_SET_IntraFlags_NonSkipZMVAdded(p) (VME_Input_S1(p, uint1, 1, 28) |= 0x20)
#define VME_CLEAR_IntraFlags_NonSkipZMVAdded(p) (VME_Input_S1(p, uint1, 1, 28) &= 0xDF) 

// #define NONSKIP_MODE_ADDED 0x40
#define VME_SET_IntraFlags_NonSkipModeAdded(p) (VME_Input_S1(p, uint1, 1, 28) |= 0x40)  
#define VME_CLEAR_IntraFlags_NonSkipModeAdded(p) (VME_Input_S1(p, uint1, 1, 28) &= 0xBF)  

// #define INTRA_TBSWAP_FLAG 0x80
#define VME_SET_IntraFlags_IntraCornerSwap(p) (VME_Input_S1(p, uint1, 1, 28) |= 0x80)  
#define VME_CLEAR_IntraFlags_IntraCornerSwap(p) (VME_Input_S1(p, uint1, 1, 28) &= 0x7F)

#define INTRA_AVAIL_F 0x80 
#define INTRA_AVAIL_E 0x40 
#define INTRA_AVAIL_A 0x20 
#define INTRA_AVAIL_B 0x10
#define INTRA_AVAIL_C 0x08 
#define INTRA_AVAIL_D 0x04
#define VME_SET_IntraAvail(p, v) { \
        if((v != 0x80) && (v != 0x40) && (v != 0x20) && (v != 0x10) && (v != 0x08) && (v != 0x04)) { \
            printf("Warning: Unexpected input value for VME_SET_IntraAvail(p, v) - v can only be 0x80, 0x40, 0x20, 0x10, 0x08 or 0x04!\n"); \
        } \
        VME_Input_S1(p, uint1, 1, 29) = v; \
    }

// Number of bits = 4
#define VME_SET_LeftModes_IntraMxMPredModeA5(p, v) { \
        if(v > 0xF) { \
            printf("Warning: Unexpected input value for VME_SET_LeftModes_IntraMxMPredModeA5(p, v) - v should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 3, 16) = VME_Input_S1(p, uint1, 3, 16)  & 0xF0 | v; \
    }

#define VME_SET_LeftModes_IntraMxMPredModeA7(p, v) { \
        if(v > 0xF) { \
            printf("Warning: Unexpected input value for VME_SET_LeftModes_IntraMxMPredModeA7(p, v) - v should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 3, 16) = VME_Input_S1(p, uint1, 3, 16) & 0x0F | (v << 4); \
    }

#define VME_SET_LeftModes_IntraMxMPredModeA13(p, v) { \
        if(v > 0xF) { \
            printf("Warning: Unexpected input value for VME_SET_LeftModes_IntraMxMPredModeA13(p, v) - v should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 3, 17) = VME_Input_S1(p, uint1, 3, 17) & 0xF0 | v; \
    }

#define VME_SET_LeftModes_IntraMxMPredModeA15(p, v) { \
        if(v > 0xF) { \
            printf("Warning: Unexpected input value for VME_SET_LeftModes_IntraMxMPredModeA15(p, v) - v should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 3, 17) = VME_Input_S1(p, uint1, 3, 17) & 0x0F | (v << 4); \
    }

// Number of bits = 4
#define VME_SET_TopModes_IntraMxMPredModeB10(p, v) { \
        if(v > 0xF) { \
            printf("Warning: Unexpected input value for VME_SET_TopModes_IntraMxMPredModeB10(p, v) - v should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 3, 18) = VME_Input_S1(p, uint1, 3, 18) & 0xF0 | v; \
    }

#define VME_SET_TopModes_IntraMxMPredModeB11(p, v) { \
        if(v > 0xF) { \
            printf("Warning: Unexpected input value for VME_SET_TopModes_IntraMxMPredModeB11(p, v) - v should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 3, 18) = VME_Input_S1(p, uint1, 3, 18) & 0x0F | (v << 4); \
    }

#define VME_SET_TopModes_IntraMxMPredModeB14(p, v) { \
        if(v > 0xF) { \
            printf("Warning: Unexpected input value for VME_SET_TopModes_IntraMxMPredModeB14(p, v) - v should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 3, 19) = VME_Input_S1(p, uint1, 3, 19) & 0xF0 | v; \
    }

#define VME_SET_TopModes_IntraMxMPredModeB15(p, v) { \
        if(v > 0xF) { \
            printf("Warning: Unexpected input value for VME_SET_TopModes_IntraMxMPredModeB15(p, v) - v should fit in 4 bits!\n"); \
        } \
        VME_Input_S1(p, uint1, 3, 19) = VME_Input_S1(p, uint1, 3, 19) & 0x0F | (v << 4); \
    }

#define VME_SET_SkipCenter1(p, Delta10X, Delta10Y, Delta11X, Delta11Y)  { \
        if((Delta10X > 0xFF) || (Delta10Y > 0xFF) || (Delta11X > 0xFF) || (Delta11Y > 0xFF)) { \
            printf("Warning: Unexpected input value for VME_SET_SkipCenter1(p, Delta10X, Delta10Y, Delta11X, Delta11Y) - each input value should fit in 8 bits!\n"); \
        } \
        VME_Input_S1(p, uint, 3, 5) = Delta10X | (Delta10Y << 8) | (Delta11X << 16) | (Delta11Y << 24); \
    }

#define VME_SET_SkipCenter2(p, Delta20X, Delta20Y, Delta21X, Delta21Y) { \
        if((Delta20X > 0xFF) || (Delta20Y > 0xFF) || (Delta21X > 0xFF) || (Delta21Y > 0xFF)) { \
            printf("Warning: Unexpected input value for VME_SET_SkipCenter2(p, Delta20X, Delta20Y, Delta21X, Delta21Y) - each input value should fit in 8 bits!\n"); \
        } \
        VME_Input_S1(p, uint, 3, 6) = Delta20X | (Delta20Y << 8) | (Delta21X << 16) | (Delta21Y << 24); \
    }

#define VME_SET_SkipCenter3(p, Delta30X, Delta30Y, Delta31X, Delta31Y) { \
        if((Delta30X > 0xFF) || (Delta30Y > 0xFF) || (Delta31X > 0xFF) || (Delta31Y > 0xFF)) { \
            printf("Warning: Unexpected input value for VME_SET_SkipCenter3(p, Delta30X, Delta30Y, Delta31X, Delta31Y) - each input value should fit in 8 bits!\n"); \
        } \
        VME_Input_S1(p, uint, 3, 7) = Delta30X | (Delta30Y << 8) | (Delta31X << 16) | (Delta31Y << 24); \
    }

#endif  // Emulation mode end

// Access macros for retrieving the input values

#define VME_GET_SrcType_SrcBlockSize(p) (VME_Input_G1(p, uint1, 0, 12) & 0x3)
#define VME_GET_SrcType_SrcBlockStructure(p) (VME_Input_G1(p, uint1, 0, 12) & 0x40)
#define VME_GET_SrcType_RefWindowStructure(p) (VME_Input_G1(p, uint1, 0, 12) & 0x80)

#define VME_GET_VmeModes_RunningMode(p) (VME_Input_G1(p, uint1, 0, 13) & 0x7)
#define VME_GET_VmeModes_DualSearchPathMode(p) (VME_Input_G1(p, uint1, 0, 13) & 0x08)
#define VME_GET_VmeModes_SubPelMode(p) (VME_Input_G1(p, uint1, 0, 13) & 0x30)

#define VME_GET_SadType_IntraDistMetric(p) (VME_Input_G1(p, uint1, 0, 14) & 0xC0)
#define VME_GET_SadType_InterDistMetric(p) (VME_Input_G1(p, uint1, 0, 14) & 0x30)
#define VME_GET_SadType_SkipBoundaryCheckDis(p) (VME_Input_G1(p, uint1, 0, 14) & 0x4)

#define VME_GET_ShapeMask(p) (VME_Input_G1(p, uint1, 0, 15))

#define VME_GET_RefW(p) (VME_Input_G1(p, uint1, 0, 22))

#define VME_GET_RefH(p) (VME_Input_G1(p, uint1, 0, 23))

#define VME_GET_VmeFlags_SkipModeEn(p) ((VME_Input_G1(p, uint1, 1, 0) & 0x01) ? 1 : 0)
#define VME_GET_VmeFlags_AdaptiveEn(p) ((VME_Input_G1(p, uint1, 1, 0) & 0x02) ? 1 : 0)
#define VME_GET_VmeFlags_BiMixDis(p) ((VME_Input_G1(p, uint1, 1, 0) & 0x04) ? 1 : 0)
#define VME_GET_VmeFlags_ExtraCandidateEn(p) ((VME_Input_G1(p, uint1, 1, 0) & 0x08) ? 1 : 0)
#define VME_GET_VmeFlags_EarlySuccessEn(p) ((VME_Input_G1(p, uint1, 1, 0) & 0x10) ? 1 : 0)
#define VME_GET_VmeFlags_ImeSuccessEn(p) ((VME_Input_G1(p, uint1, 1, 0) & 0x20) ? 1 : 0)
#define VME_GET_VmeFlags_QuitInterEn(p) ((VME_Input_G1(p, uint1, 1, 0) & 0x40) ? 1 : 0)
#define VME_GET_VmeFlags_T8x8FlagForInterEn(p) ((VME_Input_G1(p, uint1, 1, 0) & 0x80) ? 1 : 0)

#define VME_GET_EarlySkipSuc(p) (VME_Input_G1(p, uint1, 1, 1))
#define VME_GET_EarlyFmeExit(p) (VME_Input_G1(p, uint1, 1, 2))
#define VME_GET_EarlyImeStop(p) (VME_Input_G1(p, uint1, 1, 3))

#define VME_GET_MaxMvs(p) (VME_Input_G1(p, uint1, 1, 4))   

#define VME_GET_BiWeight(p) (VME_Input_G1(p, uint1, 1, 6)) 

#define VME_GET_BiDirMask_BiSubMbPartMask(p) (VME_Input_G1(p, uint1, 1, 7) & 0xF)
#define VME_GET_BiDirMask_UniMixDisable(p) ((VME_Input_G1(p, uint1, 1, 7) & 0x10) ? 1 : 0)
#define VME_GET_BiDirMask_AdaptiveValidationControl(p) ((VME_Input_G1(p, uint1, 1, 7) & 0x20) ? 1 : 0)
#define VME_GET_BiDirMask_FBPrunEn(p) ((VME_Input_G1(p, uint1, 1, 7) & 0x40) ? 1 : 0)
#define VME_GET_BiDirMask_ReshapeEnable(p) ((VME_Input_G1(p, uint1, 1, 7) & 0x80) ? 1 : 0)

#define VME_GET_MaxLenSP(p) (VME_Input_G1(p, uint1, 1, 8))

#define VME_GET_MaxNumSU(p) (VME_Input_G1(p, uint1, 1, 9))

#define VME_GET_SPCenter0(p) (VME_Input_G1(p, uint1, 1, 10)) 
#define VME_GET_SPCenter1(p) (VME_Input_G1(p, uint1, 1, 11))

#define VME_GET_BPrunTolerance(p) (VME_Input_G1(p, uint1, 1, 12)) 

#define VME_GET_ExtraTolerance(p) (VME_Input_G1(p, uint1, 1, 13))

#define VME_GET_ImeTooBad(p) (VME_Input_G1(p, uint1, 1, 14))

#define VME_GET_ImeTooGood(p) (VME_Input_G1(p, uint1, 1, 15))

#define VME_GET_CostCenter0DeltaX(p) (VME_Input_G1(p, int2, 1, 8))
#define VME_GET_CostCenter0DeltaY(p) (VME_Input_G1(p, int2, 1, 9))
#define VME_GET_CostCenter1DeltaX(p) (VME_Input_G1(p, int2, 1, 10))
#define VME_GET_CostCenter1DeltaY(p) (VME_Input_G1(p, int2, 1, 11))

#define VME_GET_SkipCenterDelta00X(p) (VME_Input_G1(p, uint1, 1, 24)) 
#define VME_GET_SkipCenterDelta00Y(p) (VME_Input_G1(p, uint1, 1, 25)) 
#define VME_GET_SkipCenterDelta01X(p) (VME_Input_G1(p, uint1, 1, 26)) 
#define VME_GET_SkipCenterDelta01Y(p) (VME_Input_G1(p, uint1, 1, 27)) 

#define VME_GET_IntraFlags_IntraPartitionMaskDis(p) (VME_Input_G1(p, uint1, 1, 28) & 0x1F) 
#define VME_GET_IntraFlags_NonSkipZMVAdded(p) ((VME_Input_G1(p, uint1, 1, 28) & 0x20) ? 1 : 0)
#define VME_GET_IntraFlags_NonSkipModeAdded(p) ((VME_Input_G1(p, uint1, 1, 28) & 0x40) ? 1 : 0) 
#define VME_GET_IntraFlags_IntraCornerSwap(p) ((VME_Input_G1(p, uint1, 1, 28) & 0x80) ? 1 : 0) 

#define VME_GET_IntraAvail_Input(p) (VME_Input_G1(p, uint1, 1, 29)) 

#define VME_GET_LeftModes_IntraMxMPredModeA5(p) (VME_Input_G1(p, uint1, 3, 16) & 0xF) 
#define VME_GET_LeftModes_IntraMxMPredModeA7(p) (VME_Input_G1(p, uint1, 3, 16) >> 4)  
#define VME_GET_LeftModes_IntraMxMPredModeA13(p) (VME_Input_G1(p, uint1, 3, 17) & 0xF) 
#define VME_GET_LeftModes_IntraMxMPredModeA15(p) (VME_Input_G1(p, uint1, 3, 17) >> 4)  

#define VME_GET_TopModes_IntraMxMPredModeB10(p) (VME_Input_G1(p, uint1, 3, 18) & 0xF)  
#define VME_GET_TopModes_IntraMxMPredModeB11(p) (VME_Input_G1(p, uint1, 3, 18) >> 4) 
#define VME_GET_TopModes_IntraMxMPredModeB14(p) (VME_Input_G1(p, uint1, 3, 19) & 0xF)  
#define VME_GET_TopModes_IntraMxMPredModeB15(p) (VME_Input_G1(p, uint1, 3, 19) >> 4)   

#define VME_GET_SkipCenter1(p) (VME_Input_G1(p, uint, 3, 5)) 
#define VME_GET_SkipCenter2(p) (VME_Input_G1(p, uint, 3, 6)) 
#define VME_GET_SkipCenter3(p) (VME_Input_G1(p, uint, 3, 7)) 

/* Add for get SkipCenter1 Delta value */
#define VME_GET_SkipCenter1Delta00X(p) (VME_Input_G1(p, uint1, 3, 20)) 
#define VME_GET_SkipCenter1Delta00Y(p) (VME_Input_G1(p, uint1, 3, 21)) 
#define VME_GET_SkipCenter1Delta01X(p) (VME_Input_G1(p, uint1, 3, 22)) 
#define VME_GET_SkipCenter1Delta01Y(p) (VME_Input_G1(p, uint1, 3, 23)) 
// Output

#define MODE_INTER_16X16 0x00
#define MODE_INTER_16X8 0x01
#define MODE_INTER_8X16 0x02
#define MODE_INTER_8X8 0x03
#define MODE_INTER_MINOR 0x03
#define VME_GET_MbMode_InterMbMode(p, v) (v = VME_Output_S1(p, uint1, 0, 0) & 0x3) 

#define MODE_SKIP_FLAG 0x04
#define VME_GET_MbMode_MbSkipFlag(p, v) (v = VME_Output_S1(p, uint1, 0, 0) & 0x4)

#define MODE_INTRA_16X16 0x00
#define MODE_INTRA_8X8 0x10
#define MODE_INTRA_4X4 0x20
#define MODE_INTRA_PCM 0x30
#define VME_GET_MbMode_IntraMbMode(p, v) (v = VME_Output_S1(p, uint1, 0, 0) & 0x30) 

#define MODE_FIELD_MB_POLARITY 0x80
#define VME_GET_MbMode_FieldMbPolarityFlag(p, v) (v = VME_Output_S1(p, uint1, 0, 0) & 0x80) 

#define TYPE_IS_INTRA 0x20
#define TYPE_IS_FIELD 0x40
#define TYPE_TRANSFORM8X8 0x80
#define TYPE_INTRA_16X16 0x35 // 0x21-0x38
#define TYPE_INTRA_16X16_021 0x35
#define TYPE_INTRA_16X16_121 0x36
#define TYPE_INTRA_16X16_221 0x37
#define TYPE_INTRA_16X16_321 0x38
#define TYPE_INTRA_8X8 0xA0
#define TYPE_INTRA_4X4 0x20
#define TYPE_INTRA_PCM 0x39 // NA
#define TYPE_INTER_16X16_0 0x01
#define TYPE_INTER_16X16_1 0x02
#define TYPE_INTER_16X16_2 0x03
#define TYPE_INTER_16X8_00 0x04
#define TYPE_INTER_16X8_11 0x06
#define TYPE_INTER_16X8_01 0x08
#define TYPE_INTER_16X8_10 0x0A
#define TYPE_INTER_16X8_02 0x0C
#define TYPE_INTER_16X8_12 0x0E
#define TYPE_INTER_16X8_20 0x10
#define TYPE_INTER_16X8_21 0x12
#define TYPE_INTER_16X8_22 0x14
#define TYPE_INTER_8X16_00 0x05
#define TYPE_INTER_8X16_11 0x07
#define TYPE_INTER_8X16_01 0x09
#define TYPE_INTER_8X16_10 0x0B
#define TYPE_INTER_8X16_02 0x0D
#define TYPE_INTER_8X16_12 0x0F
#define TYPE_INTER_8X16_20 0x11
#define TYPE_INTER_8X16_21 0x13
#define TYPE_INTER_8X16_22 0x15
#define TYPE_INTER_8X8 0x16
#define TYPE_INTER_OTHER 0x16
#ifndef TYPE_INTER_MIX_INTRA
#define TYPE_INTER_MIX_INTRA 0x18 // NA (for VC1) 
#endif
#if 1
#define VME_GET_MbType(p, v) (v = VME_Output_S1(p, uint1, 0, 1))
#else
#define VME_GET_MbType(p, v) (v = VME_Output_S1(p, uint1, 0, 1) & 0x1F)
#define VME_GET_MbType_IntraMbFlag(p, v) (v = VME_Output_S1(p, uint1, 0, 1) & 0x20) 
#define VME_GET_MbTypeFieldMbFlag(p, v) (v = VME_Output_S1(p, uint1, 0, 1) & 0x40)  
#define VME_GET_MbType_Transform8x8Flag(p, v) (v = VME_Output_S1(p, uint1, 0, 1) & 0x80) 
#endif 

#define MVSZ_NULL 0x00 
// #define MVSZ_1_MVS 0x10 // Reserved
// #define MVSZ_2_MVS 0x20 // Reserved
// #define MVSZ_4_MVS 0x30 // Reserved
#define MVSZ_8_MVS 0x40
// #define MVSZ_16_MVS 0x50 // Reserved
#define MVSZ_32_MVS 0x60
// #define MVSZ_PACKED 0x70 // Reserved
#define VME_GET_MvSizeFlag(p, v) (v = VME_Output_S1(p, uint1, 0, 2) & 0x70) 

// Number of bits = 5
#define VME_GET_MvQuantitye(p, v) (v = VME_Output_S1(p, uint1, 0, 3) & 0x1F) 

#define BORDER_REF0_LEFT 0x01
#define BORDER_REF0_RIGHT 0x02
#define BORDER_REF0_TOP 0x04
#define BORDER_REF0_BOTTOM 0x08
#define VME_GET_RefBorderMark_Ref0BorderReached(p, v) (v = VME_Output_S1(p, uint1, 0, 4) & 0xF)   

#define BORDER_REF1_LEFT 0x10
#define BORDER_REF1_RIGHT 0x20
#define BORDER_REF1_TOP 0x40
#define BORDER_REF1_BOTTOM 0x80
#define VME_GET_RefBorderMark_Ref1BorderReached(p, v) (v = VME_Output_S1(p, uint1, 0, 4) & 0xF0) 

#define VME_GET_NumSUinIME(p, v) (v = VME_Output_S1(p, uint1, 0, 5))

// Number of bits = 14
#define VME_GET_MinDist(p, v) (v = VME_Output_S1(p, uint2, 0, 3) & 0x3FFF) 
#define VME_GET_DistInter(p, v) (v = VME_Output_S1(p, uint2, 0, 4) & 0x3FFF)  

// Number of bits = 14
#define VME_GET_DistIntra16_SkipRawDistortion(p, v) (v = VME_Output_S1(p, uint2, 0, 5) & 0x3FFF)  
// Number of bits = 1
#define VME_GET_DistIntra16_SkipRawDistortionInvalid(p, v) (v = VME_Output_S1(p, uint2, 0, 5) & 0x8000) 

// Number of bits = 14
#define VME_GET_DistIntra8or4(p, v) (v = VME_Output_S1(p, uint2, 0, 6) & 0x3FFF)

// Number of bits = 4
#define BDirect8x8_0 0x10
#define BDirect8x8_1 0x20
#define BDirect8x8_2 0x40
#define BDirect8x8_3 0x80
#define VME_GET_D8x8Pattern(p, v) (v = VME_Output_S1(p, uint1, 0, 15) & 0xF0)   

#define VME_GET_IntraPredModes_LumaIntraPredModes0(p, v) (v = VME_Output_S1(p, uint2, 0, 8))
#define VME_GET_IntraPredModes_LumaIntraPredModes1(p, v) (v = VME_Output_S1(p, uint2, 0, 9))
#define VME_GET_IntraPredModes_LumaIntraPredModes2(p, v) (v = VME_Output_S1(p, uint2, 0, 10))
#define VME_GET_IntraPredModes_LumaIntraPredModes3(p, v) (v = VME_Output_S1(p, uint2, 0, 11))

#define INTRA_AVAIL_F 0x80 
#define INTRA_AVAIL_E 0x40
#define INTRA_AVAIL_A 0x20 
#define INTRA_AVAIL_B 0x10 
#define INTRA_AVAIL_C 0x08 
#define INTRA_AVAIL_D 0x04 
#define VME_GET_IntraAvail(p, v) (v = VME_Output_S1(p, uint1, 0, 24)) 

#define VME_GET_ClockCompute(p, v) (v = VME_Output_S1(p, uint1, 0, 25)) 
#define VME_GET_ClockStalled(p, v) (v = (VME_Output_S1(p, uint2, 0, 13) & 0x03FF)) 
#define VME_GET_AltNumSUinIME(p, v) (v = (VME_Output_S1(p, uint1, 0, 27) >> 2)) 

/* 4 2-bit components of the following value:
#define SUBSHP_NO_SPLIT 0
#define SUBSHP_TWO_8X4 1
#define SUBSHP_TWO_4X8 2
#define SUBSHP_FOUR_4X4 3
*/
#define VME_GET_MbSubShape(p, v) (v = VME_Output_S1(p, uint1, 0, 28)) 

/* 4 2-bit components of the following value:
#define SUBDIR_REF_0 0
#define SUBDIR_REF_1 1
#define SUBDIR_BIDIR 2
*/
#define VME_GET_MbSubPredMode(p, v) (v = VME_Output_S1(p, uint1, 0, 29)) 

#define PERFORM_SKIP 0x0001
#define PERFORM_IME 0x0002
#define PERFORM_1ST_FME 0x0004
#define PERFORM_2ND_FME 0x0008
#define PERFORM_1ST_BME 0x0010
#define PERFORM_2ND_BME 0x0020
#define PERFORM_INTRA 0x0040
#define VME_GET_VmeDecisionLog_SubFuncs(p, v) (v = VME_Output_S1(p, uint2, 0, 15) & 0x007F) 

#define OCCURRED_EARLY_SKIP 0x0080
#define OCCURRED_IME_STOP 0x0100
#define OCCURRED_TOO_GOOD 0x0200
#define OCCURRED_TOO_BAD 0x0400
#define OCCURRED_EARLY_FME 0x0800
#define VME_GET_VmeDecisionLog_EarlyExitCond(p, v) (v = VME_Output_S1(p, uint2, 0, 15) & 0x0F80)  

#define IMPROVED_FME 0x1000
#define IMPROVED_BME 0x2000
#define IMPROVED_ALT 0x4000
#define CAPPED_MAXMV 0x8000
#define VME_GET_VmeDecisionLog_SubModules(p, v) (v = VME_Output_S1(p, uint2, 0, 15) & 0xF000)  

#if 1 
#define VME_GET_Mvs(p, v) (v = p.format<uint2, nNUM_OUTPUT_GRF, 32/sizeof(uint2)>().select<2, 1, 16, 1>(1, 0))
#define VME_GET_Mvs_MinorFlag8(p, v) (v = p.format<uint2, nNUM_OUTPUT_GRF, 32/sizeof(uint2)>().select<1, 1, 8, 1>(1, 0))
#else
#define VME_GET_Mvs_Mva0X(p, v) (v = VME_Output_S1(p, uint2, 1, 0))
#define VME_GET_Mvs_Mva0Y(p, v) (v = VME_Output_S1(p, uint2, 1, 1))
#define VME_GET_Mvs_Mvb0X(p, v) (v = VME_Output_S1(p, uint2, 1, 2))
#define VME_GET_Mvs_Mvb0Y(p, v) (v = VME_Output_S1(p, uint2, 1, 3))
#define VME_GET_Mvs_Mva2To7(p, v) (v = p.format<uint2, nNUM_OUTPUT_GRF, 32/sizeof(uint2)>().select<1, 1, 6, 2>(1, 4))
#define VME_GET_Mvs_Mvb2To7(p, v) (v = p.format<uint2, nNUM_OUTPUT_GRF, 32/sizeof(uint2)>().select<1, 1, 6, 2>(1, 5))
#define VME_GET_Mvs_Mva8To15(p, v) (v = p.format<uint2, nNUM_OUTPUT_GRF, 32/sizeof(uint2)>().select<1, 1, 8, 2>(2, 0))
#define VME_GET_Mvs_Mvb8To15(p, v) (v = p.format<uint2, nNUM_OUTPUT_GRF, 32/sizeof(uint2)>().select<1, 1, 8, 2>(2, 1))
#endif

#if 1 
#define VME_GET_Distortion(p, v) (v = p.format<int2, nNUM_OUTPUT_GRF, 32/sizeof(int2)>().select<1,1,16,1>(3,0))
#else
#define VME_GET_Distortion0(p, v) (v = VME_Output_S1(p, uint2, 3, 0) & 0x3FFF)
#define VME_GET_Distortion1(p, v) (v = VME_Output_S1(p, uint2, 3, 1) & 0x3FFF)
#define VME_GET_Distortion2To15(p, v) (v = p.format<uint2, nNUM_OUTPUT_GRF, 32/sizeof(uint2)>().select<1,1,14,1>(3,2))
#endif

// Host API to load current/ref surface ID, search path and Lut data
extern uint curSurfIndex;
extern uint refSurfIndex;
extern uint refSurfIndex1;
CM_API extern void LoadSurfaceIdx(uint curIndex, uint refIndex);
CM_API extern void LoadSurfaceIdx(SurfaceIndex curIndex, SurfaceIndex refIndex);
CM_API extern uint GetCurrentSurfIdx();
CM_API extern uint GetRefSurfIdx();
CM_API extern uint GetRefSurfIdx1();

extern U8 FullSP[8][64];
extern U8 m_LutXY[8][4][8];
extern U8 m_LutMode[8][4][10];
CM_API extern void LoadSearchPaths(U8 search_paths[8][64], U8 luts_mode[8][4][10], U8 luts_xy[8][4][8]);


// Run VME

CM_API extern void
run_vme_inter(vme_InputMrfType &mInput, uint surfIndex, uint SPIndex, uint lutSubIndex,
        vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
        vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, vme_OutputGrfType &mOutput);

CM_API extern void
run_vme_intra(vme_InputMrfType &mInput, uint surfIndex, uint SPIndex, uint lutSubIndex,
        vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
        vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, vme_OutputGrfShortType &mOutput);

CM_API extern void
run_vme_all(vme_InputMrfType &mInput, uint surfIndex, uint SPIndex, uint lutSubIndex,
        vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
        vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, vme_OutputGrfType &mOutput);

CM_API extern void
run_vme_inter(vme_InputMrfType &mInput, SurfaceIndex surfIndex, VmeIndex SPIndex, uint lutSubIndex,
        vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
        vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, vme_OutputGrfType &mOutput);

CM_API extern void
run_vme_intra(vme_InputMrfType &mInput, SurfaceIndex surfIndex, VmeIndex SPIndex, uint lutSubIndex,
        vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
        vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, vme_OutputGrfShortType &mOutput);

CM_API extern void
run_vme_all(vme_InputMrfType &mInput, SurfaceIndex surfIndex, VmeIndex SPIndex, uint lutSubIndex,
        vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
        vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, vme_OutputGrfType &mOutput);

// Set the default values for VME input payload 
#define INIT_VME_INPUT(p)  { /* M0 */ \
                             VME_Input_S1(p, uint4, 0, 0) = 0x00000000;     /* Ref0(I16PAIR)(0:31)(Input Parameter) */ \
                             VME_Input_S1(p, uint4, 0, 1) = 0x00000000;     /* Ref1(I16PAIR)(0:31)(Input Parameter) */ \
                             VME_Input_S1(p, uint4, 0, 2) = 0x00000000;     /* SrcMB(I16PAIR)(0:31)(Input Parameter) */ \
                             VME_Input_S1(p, uint4, 0, 3) = 0x0F000000;     /* SrcType(U8)(0:7) + VmeModes(U8)(8:15) + SadType(U8)(16:23) + ShapeMask(U8)(24:31)*/ \
                             VME_Input_S1(p, uint4, 0, 4) = 0x00000000;     /* Reserved(0:31) */ \
                             VME_Input_S1(p, uint4, 0, 5) = 0x20200000;     /* DispatchID(0:7) + Reserved(8:15) + RefW(U8)(16:23) + RefH(U8)(24:31) */ \
                             VME_Input_S1(p, uint4, 0, 6) = 0x00000000;     /* Debug(0:31) */ \
                             VME_Input_S1(p, uint4, 0, 7) = 0x00000000;     /* Debug(0:31) */ \
                             /* M1 */ \
                             VME_Input_S1(p, uint4, 1, 0) = 0x00000000;     /* VmeFlags(U8)(0:7) + EarlySkipSuc(U4PAIR)(8:15) + EarlyFmeExit(U4PAIR)(16:23) + EarlyImeStop(U4PAIR)(24:31) */ \
                             VME_Input_S1(p, uint4, 1, 1) = 0x00200020;     /* MaxMvs(U8)(0:5) + Reserved(6:15) + BiWeight(U8)(16:21) + Extended32MVFlag(22) + ExtendedFormFlag(23) + BidirMask(U8)(24:31)*/ \
                             VME_Input_S1(p, uint4, 1, 2) = 0x22223030;     /* MaxLenSP(U8)(0:7) + MaxNumSU(U8)(8:15) + SPCenter[2](U8)(16:31) */ \
                             VME_Input_S1(p, uint4, 1, 3) = 0x00C10000;     /* BPrunTolerance(U4PAIR)(0:7) + ExtraTolerance(U4PAIR)(8:15) + ImeTooBad(U4PAIR)(16:23) + ImeTooGood(U4PAIR)(24:31) */ \
                             VME_Input_S1(p, uint4, 1, 4) = 0x00800080;     /* CostCenter[2] (Default Value = 32 = 00 1000 0000 in S7.2 format) */ \
                             VME_Input_S1(p, uint4, 1, 5) = 0x00800080;     /* CostCenter[2] (Default Value = 32 = 00 1000 0000 in S7.2 format) */ \
                             VME_Input_S1(p, uint4, 1, 6) = 0x80808080;     /* SkipCenter[2] (Default Value = 32 = 1000 0000 in U6.2 format) */ \
                             VME_Input_S1(p, uint4, 1, 7) = 0x00000000;     /* IntraFlags(U8)(0:7) + IntraAvail(U8)(8:15) + Reserved(16:31) */ \
                             /* M2 */ \
                             VME_Input_S1(p, uint4, 2, 0) = 0x00000000;     /* TopMinus8Pels[32](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 2, 1) = 0x00000000;     /* TopMinus8Pels[32](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 2, 2) = 0x00000000;     /* TopMinus8Pels[32](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 2, 3) = 0x00000000;     /* TopMinus8Pels[32](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 2, 4) = 0x00000000;     /* TopMinus8Pels[32](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 2, 5) = 0x00000000;     /* TopMinus8Pels[32](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 2, 6) = 0x00000000;     /* TopMinus8Pels[32](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 2, 7) = 0x00000000;     /* TopMinus8Pels[32](Input Parameter) */ \
                             /* M3 */ \
                             VME_Input_S1(p, uint4, 3, 0) = 0x00000000;     /* LeftPels[16](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 3, 1) = 0x00000000;     /* LeftPels[16](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 3, 2) = 0x00000000;     /* LeftPels[16](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 3, 3) = 0x00000000;     /* LeftPels[16](Input Parameter) */ \
                             VME_Input_S1(p, uint4, 3, 4) = 0x22222222;     /* LeftModes(U16)(0:15) + TopModes(U16)(16:31) */ \
                             VME_Input_S1(p, uint4, 3, 5) = 0x00000000;     /* SkipCenter1[2](U8PAIR) */ \
                             VME_Input_S1(p, uint4, 3, 6) = 0x00000000;     /* SkipCenter2[2](U8PAIR) */ \
                             VME_Input_S1(p, uint4, 3, 7) = 0x00000000;     /* SkipCenter3[2](U8PAIR) */ \
                          }

#endif /* CM_VME_H */
