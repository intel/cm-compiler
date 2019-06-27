/*
 * Copyright (c) 2019, Intel Corporation
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

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_common.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_COMMON_H_
#define _CLANG_CM_COMMON_H_

// Define MDF version
#ifndef CM_1_0
#define CM_1_0 100
#endif
#ifndef CM_2_0
#define CM_2_0 200
#endif
#ifndef CM_2_1
#define CM_2_1 201
#endif
#ifndef CM_2_2
#define CM_2_2 202
#endif
#ifndef CM_2_3
#define CM_2_3 203
#endif
#ifndef CM_2_4
#define CM_2_4 204
#endif
#ifndef CM_3_0
#define CM_3_0 300
#endif
#ifndef CM_4_0
#define CM_4_0 400
#endif
#ifndef CM_5_0
#define CM_5_0 500
#endif
#ifndef CM_6_0
#define CM_6_0 600
#endif
#ifndef CM_7_0
#define CM_7_0 700
#endif

#ifndef __INTEL_MDF
#define __INTEL_MDF (CM_7_0 + 1)
#endif

#ifndef __INTEL_CM
#define __INTEL_CM (CM_7_0 + 1)
#endif

// Typedefs
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

#ifndef __int8
#define __int8 char
#endif

#ifndef __uint8
#define __uint8 unsigned char
#endif

#ifndef __int16
#define __int16 short
#endif

#ifndef __uint16
#define __uint16 unsigned short
#endif

#ifndef __int32
#define __int32 int
#endif

#ifndef __uint32
#define __uint32 unsigned int
#endif

#ifndef __int64
#define __int64 long long
#endif

#ifndef __uint64
#define __uint64 unsigned long long
#endif

typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef __uint8 uint8_t;
typedef __uint16 uint16_t;
typedef __uint32 uint32_t;
typedef __uint64 uint64_t;

#if __SIZEOF_POINTER__ == 8
typedef uint64_t uintptr_t;
typedef int64_t ptrdiff_t;
#elif __SIZEOF_POINTER__ == 4
typedef uint32_t uintptr_t;
typedef int32_t ptrdiff_t;
#else
#error improbably-sized pointers on this target
#endif

#ifdef CM_PTRSIZE
#if CM_PTRSIZE == 32
typedef uint32_t svmptr_t;
#elif CM_PTRSIZE == 64
typedef uint64_t svmptr_t;
#else
#error CM_PTRSIZE must be 32 or 64
#endif
#else
typedef uintptr_t svmptr_t;
#endif

// Enums
enum {
  _GENX_NOSAT = 0,
  _GENX_SAT
};

enum ChannelMaskType {
  _CM_R_ENABLE = 1,
  _CM_G_ENABLE = 2,
  _CM_GR_ENABLE = 3,
  _CM_B_ENABLE = 4,
  _CM_BR_ENABLE = 5,
  _CM_BG_ENABLE = 6,
  _CM_BGR_ENABLE = 7,
  _CM_A_ENABLE = 8,
  _CM_AR_ENABLE = 9,
  _CM_AG_ENABLE = 10,
  _CM_AGR_ENABLE = 11,
  _CM_AB_ENABLE = 12,
  _CM_ABR_ENABLE = 13,
  _CM_ABG_ENABLE = 14,
  _CM_ABGR_ENABLE = 15
};

#define CM_R_ENABLE ChannelMaskType::_CM_R_ENABLE
#define CM_G_ENABLE ChannelMaskType::_CM_G_ENABLE
#define CM_GR_ENABLE ChannelMaskType::_CM_GR_ENABLE
#define CM_B_ENABLE ChannelMaskType::_CM_B_ENABLE
#define CM_BR_ENABLE ChannelMaskType::_CM_BR_ENABLE
#define CM_BG_ENABLE ChannelMaskType::_CM_BG_ENABLE
#define CM_BGR_ENABLE ChannelMaskType::_CM_BGR_ENABLE
#define CM_A_ENABLE ChannelMaskType::_CM_A_ENABLE
#define CM_AR_ENABLE ChannelMaskType::_CM_AR_ENABLE
#define CM_AG_ENABLE ChannelMaskType::_CM_AG_ENABLE
#define CM_AGR_ENABLE ChannelMaskType::_CM_AGR_ENABLE
#define CM_AB_ENABLE ChannelMaskType::_CM_AB_ENABLE
#define CM_ABR_ENABLE ChannelMaskType::_CM_ABR_ENABLE
#define CM_ABG_ENABLE ChannelMaskType::_CM_ABG_ENABLE
#define CM_ABGR_ENABLE ChannelMaskType::_CM_ABGR_ENABLE

enum class SLM_ChannelMaskType {
  _SLM_R_ENABLE = 14,
  _SLM_G_ENABLE = 13,
  _SLM_GR_ENABLE = 12,
  _SLM_B_ENABLE = 11,
  _SLM_BR_ENABLE = 10,
  _SLM_BG_ENABLE = 9,
  _SLM_BGR_ENABLE = 8,
  _SLM_A_ENABLE = 7,
  _SLM_AR_ENABLE = 6,
  _SLM_AG_ENABLE = 5,
  _SLM_AGR_ENABLE = 4,
  _SLM_AB_ENABLE = 3,
  _SLM_ABR_ENABLE = 2,
  _SLM_ABG_ENABLE = 1,
  _SLM_ABGR_ENABLE = 0
};

#define SLM_R_ENABLE SLM_ChannelMaskType::_SLM_R_ENABLE
#define SLM_G_ENABLE SLM_ChannelMaskType::_SLM_G_ENABLE
#define SLM_GR_ENABLE SLM_ChannelMaskType::_SLM_GR_ENABLE
#define SLM_B_ENABLE SLM_ChannelMaskType::_SLM_B_ENABLE
#define SLM_BR_ENABLE SLM_ChannelMaskType::_SLM_BR_ENABLE
#define SLM_BG_ENABLE SLM_ChannelMaskType::_SLM_BG_ENABLE
#define SLM_BGR_ENABLE SLM_ChannelMaskType::_SLM_BGR_ENABLE
#define SLM_A_ENABLE SLM_ChannelMaskType::_SLM_A_ENABLE
#define SLM_AR_ENABLE SLM_ChannelMaskType::_SLM_AR_ENABLE
#define SLM_AG_ENABLE SLM_ChannelMaskType::_SLM_AG_ENABLE
#define SLM_AGR_ENABLE SLM_ChannelMaskType::_SLM_AGR_ENABLE
#define SLM_AB_ENABLE SLM_ChannelMaskType::_SLM_AB_ENABLE
#define SLM_ABR_ENABLE SLM_ChannelMaskType::_SLM_ABR_ENABLE
#define SLM_ABG_ENABLE SLM_ChannelMaskType::_SLM_ABG_ENABLE
#define SLM_ABGR_ENABLE SLM_ChannelMaskType::_SLM_ABGR_ENABLE

enum class CmBufferAttrib {
  _GENX_NONE = 0,
  _GENX_TOP_FIELD = 1,
  _GENX_BOTTOM_FIELD = 2,
  _GENX_DWALIGNED = 3,
  _GENX_MODIFIED = 4,
  _GENX_MODIFIED_TOP_FIELD = 5,
  _GENX_MODIFIED_BOTTOM_FIELD = 6,
  _GENX_MODIFIED_DWALIGNED = 7,
  _GENX_CONSTANT = 8,
  _GENX_CONSTANT_DWALIGNED = 9,
  _GENX_NUM_BUFFER_ATTRIB = 10
};

#define GENX_NONE CmBufferAttrib::_GENX_NONE
#define GENX_TOP_FIELD CmBufferAttrib::_GENX_TOP_FIELD
#define GENX_BOTTOM_FIELD CmBufferAttrib::_GENX_BOTTOM_FIELD
#define GENX_DWALIGNED CmBufferAttrib::_GENX_DWALIGNED
#define GENX_MODIFIED CmBufferAttrib::_GENX_MODIFIED
#define GENX_MODIFIED_TOP_FIELD CmBufferAttrib::_GENX_MODIFIE_TOP_FIELDD
#define GENX_MODIFIED_BOTTOM_FIELD CmBufferAttrib::_GENX_MODIFIED_BOTTOM_FIELD
#define GENX_MODIFIED_DWALIGNED CmBufferAttrib::_GENX_MODIFIED_DWALIGNED
#define GENX_CONSTANT CmBufferAttrib::_GENX_CONSTANT
#define GENX_CONSTANT_DWALIGNED CmBufferAttrib::_GENX_CONSTANT_DWALIGNED
#define GENX_NUM_BUFFER_ATTRIB CmBufferAttrib::_GENX_NUM_BUFFER_ATTRIB

enum class CmSurfacePlaneIndex {
  _GENX_SURFACE_Y_PLANE = 0,
  _GENX_SURFACE_U_PLANE = 1,
  _GENX_SURFACE_UV_PLANE = 1,
  _GENX_SURFACE_V_PLANE = 2
};

#define GENX_SURFACE_Y_PLANE CmSurfacePlaneIndex::_GENX_SURFACE_Y_PLANE
#define GENX_SURFACE_U_PLANE CmSurfacePlaneIndex::_GENX_SURFACE_U_PLANE
#define GENX_SURFACE_UV_PLANE CmSurfacePlaneIndex::_GENX_SURFACE_UV_PLANE
#define GENX_SURFACE_V_PLANE CmSurfacePlaneIndex::_GENX_SURFACE_V_PLANE

enum class OutputFormatControl {
  _CM_16_FULL = 0,
  _CM_16_DOWN_SAMPLE = 1,
  _CM_8_FULL = 2,
  _CM_8_DOWN_SAMPLE = 3
};

#define CM_16_FULL OutputFormatControl::_CM_16_FULL
#define CM_16_DOWN_SAMPLE OutputFormatControl::_CM_16_DOWN_SAMPLE
#define CM_8_FULL OutputFormatControl::_CM_8_FULL
#define CM_8_DOWN_SAMPLE OutputFormatControl::_CM_8_DOWN_SAMPLE

enum class AVSExecMode {
  _CM_AVS_16x4 = 0,
  _CM_AVS_8x4 = 1,
  _CM_AVS_16x8 = 2,
  _CM_AVS_4x4 = 3
};

#define CM_AVS_16x4 AVSExecMode::_CM_AVS_16x4
#define CM_AVS_8x4 AVSExecMode::_CM_AVS_8x4
#define CM_AVS_16x8 AVSExecMode::_CM_AVS_16x8
#define CM_AVS_4x4 AVSExecMode::_CM_AVS_4x4

enum class CONVExecMode {
  _CM_CONV_16x4 = 0,
  _CM_CONV_16x1 = 2,
  _CM_CONV_1x1 = 3 // 1pixel convolve only
};

#define CM_CONV_16x4 CONVExecMode::_CM_CONV_16x4
#define CM_CONV_16x1 CONVExecMode::_CM_CONV_16x1
#define CM_CONV_1x1 CONVExecMode::_CM_CONV_1x1

enum class EDExecMode {
  _CM_ED_64x4 = 0,
  _CM_ED_32x4 = 1,
  _CM_ED_64x1 = 2,
  _CM_ED_32x1 = 3
};

#define CM_ED_64x4 EDExecMode::_CM_ED_64x4
#define CM_ED_32x4 EDExecMode::_CM_ED_32x4
#define CM_ED_64x1 EDExecMode::_CM_ED_64x1
#define CM_ED_32x1 EDExecMode::_CM_ED_32x1

enum class MMFExecMode {
  _CM_MMF_16x4 = 0,
  _CM_MMF_16x1 = 2,
  _CM_MMF_1x1 = 3
};

#define CM_MMF_16x4 MMFExecMode::_CM_MMF_16x4
#define CM_MMF_16x1 MMFExecMode::_CM_MMF_16x1
#define CM_MMF_1x1 MMFExecMode::_CM_MMF_1x1

enum class MMFEnableMode {
  _CM_MINMAX_ENABLE = 0,
  _CM_MAX_ENABLE = 1,
  _CM_MIN_ENABLE = 2
};

#define CM_MINMAX_ENABLE MMFEnableMode::_CM_MINMAX_ENABLE
#define CM_MAX_ENABLE MMFEnableMode::_CM_MAX_ENABLE
#define CM_MIN_ENABLE MMFEnableMode::_CM_MIN_ENABLE

enum class LBPCreationExecMode {
  _CM_LBP_CREATION_5x5 = 0x2,
  _CM_LBP_CREATION_3x3 = 0x1,
  _CM_LBP_CREATION_BOTH = 0x0
};

#define CM_LBP_CREATION_5x5 LBPCreationExecMode::_CM_LBP_CREATION_5x5
#define CM_LBP_CREATION_3x3 LBPCreationExecMode::_CM_LBP_CREATION_3x3
#define CM_LBP_CREATION_BOTH LBPCreationExecMode::_CM_LBP_CREATION_BOTH

enum class CM_FORMAT_SIZE {
  _CM_HDC_FORMAT_16S = 0x0,
  _CM_HDC_FORMAT_8U = 0x1
};

#define CM_HDC_FORMAT_16S CM_FORMAT_SIZE::_CM_HDC_FORMAT_16S
#define CM_HDC_FORMAT_8U CM_FORMAT_SIZE::_CM_HDC_FORMAT_8U


enum class CmAtomicOpType {
  _ATOMIC_ADD = 0x0,
  _ATOMIC_SUB = 0x1,
  _ATOMIC_INC = 0x2,
  _ATOMIC_DEC = 0x3,
  _ATOMIC_MIN = 0x4,
  _ATOMIC_MAX = 0x5,
  _ATOMIC_XCHG = 0x6,
  _ATOMIC_CMPXCHG = 0x7,
  _ATOMIC_AND = 0x8,
  _ATOMIC_OR = 0x9,
  _ATOMIC_XOR = 0xa,
  _ATOMIC_MINSINT = 0xb,
  _ATOMIC_MAXSINT = 0xc,
  _ATOMIC_FMAX = 0x10,
  _ATOMIC_FMIN = 0x11,
  _ATOMIC_FCMPWR = 0x12,
  _ATOMIC_PREDEC = 0xff
};

#define ATOMIC_ADD CmAtomicOpType::_ATOMIC_ADD
#define ATOMIC_SUB CmAtomicOpType::_ATOMIC_SUB
#define ATOMIC_INC CmAtomicOpType::_ATOMIC_INC
#define ATOMIC_DEC CmAtomicOpType::_ATOMIC_DEC
#define ATOMIC_MIN CmAtomicOpType::_ATOMIC_MIN
#define ATOMIC_MAX CmAtomicOpType::_ATOMIC_MAX
#define ATOMIC_XCHG CmAtomicOpType::_ATOMIC_XCHG
#define ATOMIC_CMPXCHG CmAtomicOpType::_ATOMIC_CMPXCHG
#define ATOMIC_AND CmAtomicOpType::_ATOMIC_AND
#define ATOMIC_OR CmAtomicOpType::_ATOMIC_OR
#define ATOMIC_XOR CmAtomicOpType::_ATOMIC_XOR
#define ATOMIC_MINSINT CmAtomicOpType::_ATOMIC_MINSINT
#define ATOMIC_MAXSINT CmAtomicOpType::_ATOMIC_MAXSINT
#define ATOMIC_FMAX CmAtomicOpType::_ATOMIC_FMAX
#define ATOMIC_FMIN CmAtomicOpType::_ATOMIC_FMIN
#define ATOMIC_FCMPWR CmAtomicOpType::_ATOMIC_FCMPWR
#define ATOMIC_PREDEC CmAtomicOpType::_ATOMIC_PREDEC

enum class CM3DSampleOp : int {
  _CM_3D_SAMPLE = 0,
  _CM_3D_SAMPLE_B = 1,
  _CM_3D_SAMPLE_L = 2,
  _CM_3D_SAMPLE_C = 3,
  _CM_3D_SAMPLE_D = 4,
  _CM_3D_SAMPLE_B_C = 5,
  _CM_3D_SAMPLE_L_C = 6,
  _CM_3D_LOD = 9,
  _CM_3D_SAMPLE_D_C = 20,
  _CM_3D_SAMPLE_LZ = 24,
  _CM_3D_SAMPLE_C_LZ = 25,
  _CM_3D_SAMPLE_NULLMASK_ENABLE = 32,
  _CM_3D_SAMPLE_CPS_LOD_COMP_ENABLE = 64
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++1y-extensions"

inline constexpr CM3DSampleOp operator | (CM3DSampleOp lhs, CM3DSampleOp rhs) {
  return static_cast<CM3DSampleOp>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline constexpr CM3DSampleOp& operator |= (CM3DSampleOp &lhs, CM3DSampleOp rhs) {
  lhs = static_cast<CM3DSampleOp>(static_cast<int>(lhs) | static_cast<int>(rhs));
  return lhs;
}

#pragma clang diagnostic pop

#define CM_3D_SAMPLE CM3DSampleOp::_CM_3D_SAMPLE
#define CM_3D_SAMPLE_B CM3DSampleOp::_CM_3D_SAMPLE_B
#define CM_3D_SAMPLE_L CM3DSampleOp::_CM_3D_SAMPLE_L
#define CM_3D_SAMPLE_C CM3DSampleOp::_CM_3D_SAMPLE_C
#define CM_3D_SAMPLE_D CM3DSampleOp::_CM_3D_SAMPLE_D
#define CM_3D_SAMPLE_B_C CM3DSampleOp::_CM_3D_SAMPLE_B_C
#define CM_3D_SAMPLE_L_C CM3DSampleOp::_CM_3D_SAMPLE_L_C
#define CM_3D_LOD CM3DSampleOp::_CM_3D_LOD
#define CM_3D_SAMPLE_D_C CM3DSampleOp::_CM_3D_SAMPLE_D_C
#define CM_3D_SAMPLE_LZ CM3DSampleOp::_CM_3D_SAMPLE_LZ
#define CM_3D_SAMPLE_C_LZ CM3DSampleOp::_CM_3D_SAMPLE_C_LZ
#define CM_3D_SAMPLE_NULLMASK_ENABLE CM3DSampleOp::_CM_3D_SAMPLE_NULLMASK_ENABLE
#define CM_3D_SAMPLE_CPS_LOD_COMP_ENABLE CM3DSampleOp::_CM_3D_SAMPLE_CPS_LOD_COMP_ENABLE

enum class CM3DLoadOp : int {
  _CM_3D_LOAD = 7,
  _CM_3D_LOAD_LZ = 26,
  _CM_3D_LOAD_2DMS_W = 28,
  _CM_3D_LOAD_MCS = 29,
  _CM_3D_LOAD_NULLMASK_ENABLE = 32
};

inline CM3DLoadOp operator | (CM3DLoadOp lhs, CM3DLoadOp rhs) {
  return (CM3DLoadOp)(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline CM3DLoadOp& operator |= (CM3DLoadOp &lhs, CM3DLoadOp rhs) {
  lhs = (CM3DLoadOp)(static_cast<int>(lhs) | static_cast<int>(rhs));
  return lhs;
}

#define CM_3D_LOAD CM3DLoadOp::_CM_3D_LOAD
#define CM_3D_LOAD_LZ CM3DLoadOp::_CM_3D_LOAD_LZ
#define CM_3D_LOAD_2DMS_W CM3DLoadOp::_CM_3D_LOAD_2DMS_W
#define CM_3D_LOAD_MCS CM3DLoadOp::_CM_3D_LOAD_MCS
#define CM_3D_LOAD_NULLMASK_ENABLE CM3DLoadOp::_CM_3D_LOAD_NULLMASK_ENABLE

enum CmFloatControl {
  _CM_RTE = 0,      // Round to nearest or even
  _CM_RTP = 1 << 4, // Round towards +ve inf
  _CM_RTN = 2 << 4, // Round towards -ve inf
  _CM_RTZ = 3 << 4, // Round towards zero

  _CM_DENORM_FTZ      = 0,        // Denorm mode flush to zero
  _CM_DENORM_D_ALLOW  = 1 << 6,   // Denorm mode double allow
  _CM_DENORM_F_ALLOW  = 1 << 7,   // Denorm mode float allow
  _CM_DENORM_HF_ALLOW = 1 << 10,  // Denorm mode half allow

  _CM_FLOAT_MODE_IEEE    = 0,     // Single precision float IEEE mode
  _CM_FLOAT_MODE_ALT     = 1      // Single precision float ALT mode
};

#define CM_RTE CmFloatControl::_CM_RTE
#define CM_RTP CmFloatControl::_CM_RTP
#define CM_RTN CmFloatControl::_CM_RTN
#define CM_RTZ CmFloatControl::_CM_RTZ

#define CM_DENORM_FTZ       CmFloatControl::_CM_DENORM_FTZ
#define CM_DENORM_D_ALLOW   CmFloatControl::_CM_DENORM_D_ALLOW
#define CM_DENORM_F_ALLOW   CmFloatControl::_CM_DENORM_F_ALLOW
#define CM_DENORM_HF_ALLOW  CmFloatControl::_CM_DENORM_HF_ALLOW
#define CM_DENORM_ALLOW     (CmFloatControl::_CM_DENORM_D_ALLOW | CmFloatControl::_CM_DENORM_F_ALLOW | CmFloatControl::_CM_DENORM_HF_ALLOW)

#define CM_FLOAT_MODE_IEEE  CmFloatControl::_CM_FLOAT_MODE_IEEE
#define CM_FLOAT_MODE_ALT   CmFloatControl::_CM_FLOAT_MODE_ALT


// Macros
#define NULL 0
#define _GENX_MAIN_                     __declspec(genx_main)
#define _GENX_                          __declspec(genx)
#define _GENX_VOLATILE_                 __declspec(genx_volatile)
#define _GENX_VOLATILE_BINDING_(Offset) __declspec(genx_volatile(Offset))
#define _CM_BUILTIN_                    __declspec(cm_builtin)
#define SAT _GENX_SAT

// SIMT entry function attribute
#define _SIMT8_       __declspec(genx_SIMT(8))
#define _SIMT16_      __declspec(genx_SIMT(16))
#define _SIMT32_      __declspec(genx_SIMT(32))

// Fast-composition kernel attribute
#define _CM_CALLABLE_     __declspec(cm_callable)
#define _CM_ENTRY_        __declspec(cm_entry)

// Fast-composition argument attribute
#define _CM_INPUT_        __declspec(cm_input)
#define _CM_OUTPUT_       __declspec(cm_output)
#define _CM_INPUT_OUTPUT_ __declspec(cm_input_output)
#define _CM_OUTPUT_INPUT_ __declspec(cm_input_output)

#define _GENX_ROUNDING_MODE_(x) __declspec(cm_float_control(x))
#define _GENX_FLOAT_CONTROL_(x) __declspec(cm_float_control(x))

// Mark a function being deprecated.
#define CM_DEPRECATED(Msg) __attribute__((deprecated(Msg)))
// Mark a function being nodebug.
#define CM_NODEBUG __attribute__((nodebug))
// Mark a function being noinline
#define CM_NOINLINE __attribute__((noinline))
// Mark a function to be inlined
#define CM_INLINE inline

// CM_STATIC_ERROR() and CM_STATIC_WARNING() are wrappers around the C++
// static_assert mechanism that can be used to produce CM style diagnostics
// rather than C++ style asserts. They should be used in preference to
// static_assert within CM header files.
#define CM_STATIC_ERROR(C, M) static_assert((C), "CM:e:" M)
#define CM_STATIC_WARNING(C, M) static_assert((C), "CM:w:" M)

// cm_label() is deprecated
#define cm_label(...) CM_STATIC_WARNING(0, "cm_label() is deprecated")


#endif /* _CLANG_CM_COMMON_H_ */
