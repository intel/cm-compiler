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

//===-- GenXVisa.h - vISA defines -----------------------------------------===//
//===----------------------------------------------------------------------===//
//
// This file contains defines for vISA and the vISA writer.
//
//===----------------------------------------------------------------------===//
#ifndef GENXVISA_H
#define GENXVISA_H

#include "GenX.h"
#include "GenXBaling.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include <map>
#include <string>
#include <vector>
#include "GenXModule.h"

namespace llvm {
  namespace visa {

    // Opcode declarations
    enum {
      ISA_ADD                = 0x1 ,
      ISA_AVG                = 0x2 ,
      ISA_DIV                = 0x3 ,
      ISA_DP2                = 0x4 ,
      ISA_DP3                = 0x5 ,
      ISA_DP4                = 0x6 ,
      ISA_DPH                = 0x7 ,
      ISA_EXP                = 0x8 ,
      ISA_FRC                = 0x9 ,
      ISA_LINE               = 0xA ,
      ISA_LOG                = 0xB ,
      ISA_MAD                = 0xC ,
      ISA_MULH               = 0xD ,
      ISA_LRP                = 0xE ,
      ISA_MOD                = 0xF ,
      ISA_MUL                = 0x10,
      ISA_POW                = 0x11,
      ISA_RNDD               = 0x12,
      ISA_RNDU               = 0x13,
      ISA_RNDE               = 0x14,
      ISA_RNDZ               = 0x15,
      ISA_SAD2               = 0x16,
      ISA_SIN                = 0x17,
      ISA_COS                = 0x18,
      ISA_SQRT               = 0x19,
      ISA_RSQRT              = 0x1A,
      ISA_INV                = 0x1B,
      ISA_LZD                = 0x1F,
      ISA_AND                = 0x20,
      ISA_OR                 = 0x21,
      ISA_XOR                = 0x22,
      ISA_NOT                = 0x23,
      ISA_SHL                = 0x24,
      ISA_SHR                = 0x25,
      ISA_ASR                = 0x26,
      ISA_CBIT               = 0x27,
      ISA_ADDR_ADD           = 0x28,
      ISA_MOV                = 0x29,
      ISA_SEL                = 0x2A,
      ISA_SETP               = 0x2B,
      ISA_CMP                = 0x2C,
      ISA_MOVS               = 0x2D,
      ISA_FBL                = 0x2E,
      ISA_FBH                = 0x2F,
      ISA_SUBROUTINE         = 0x30,
      ISA_LABEL              = 0x31,
      ISA_JMP                = 0x32,
      ISA_CALL               = 0x33,
      ISA_RET                = 0x34,
      ISA_OWORD_LD           = 0x35,
      ISA_OWORD_ST           = 0x36,
      ISA_MEDIA_LD           = 0x37,
      ISA_MEDIA_ST           = 0x38,
      ISA_GATHER             = 0x39,
      ISA_SCATTER            = 0x3A,
      ISA_SCATTER_ATOMIC     = 0x3B,
      ISA_OWORD_LD_UNALIGNED = 0x3C,
      ISA_GATHER4            = 0x3D,
      ISA_SCATTER4           = 0x3E,
      ISA_TRANSPOSE_LD       = 0x3F,
      ISA_SAMPLE             = 0x40,
      ISA_SAMPLE_UNORM       = 0x41,
      ISA_LOAD               = 0x42,
      ISA_AVS                = 0x43,
      ISA_VA                 = 0x44,
      ISA_FMINMAX            = 0x45,
      ISA_BFE                = 0x46,
      ISA_BFI                = 0x47,
      ISA_BFREV              = 0x48,
      ISA_ADDC               = 0x49,
      ISA_SUBB               = 0x4A,
      ISA_GATHER4_TYPED      = 0x4B,
      ISA_SCATTER4_TYPED     = 0x4C,
      ISA_VA_SKL_PLUS        = 0x4D,
      ISA_SVM                = 0x4E,
      ISA_RESERVED_4F        = 0x4F,
      ISA_VME                = 0x50,
      ISA_FILE               = 0x51,
      ISA_LOC                = 0x52,
      ISA_VME_IVB            = 0x53,
      ISA_VME_IME            = 0x54,
      ISA_VME_SIC            = 0x55,
      ISA_VME_FBR            = 0x56,
      ISA_VME_IDM            = 0x57,
      ISA_CISA_INLINE        = 0x58,
      ISA_BARRIER            = 0x59,
      ISA_SAMPLR_CACHE_FLUSH = 0x5A,
      ISA_WAIT               = 0x5B,
      ISA_FENCE              = 0x5C,
      ISA_RAW_SEND           = 0x5D,
      ISA_COMMENT            = 0x5E,
      ISA_YIELD              = 0x5F,
      ISA_IF                 = 0x60,
      ISA_ELSE               = 0x61,
      ISA_ENDIF              = 0x62,
      ISA_DO                 = 0x63,
      ISA_WHILE              = 0x64,
      ISA_BREAK              = 0x65,
      ISA_CONT               = 0x66,
      ISA_FCALL              = 0x67,
      ISA_FRET               = 0x68,
      ISA_SWITCHJMP          = 0x69,
      ISA_SAD2ADD            = 0x6A,
      ISA_PLANE              = 0x6B,
      ISA_GOTO               = 0x6C,
      ISA_3D_SAMPLE          = 0x6D,
      ISA_3D_LOAD            = 0x6E,
      ISA_3D_GATHER4         = 0x6F,
      ISA_3D_INFO            = 0x70,
      ISA_3D_RT_WRITE        = 0x71,
      ISA_3D_URB_WRITE       = 0x72,
      ISA_3D_TYPED_ATOMIC    = 0x73,
      ISA_GATHER4_SCALED     = 0x74, // vISA spec incorrectly says 0x75 (bug 4383)
      ISA_SCATTER4_SCALED    = 0x75, // vISA spec incorrectly says 0x74 (bug 4383)
      ISA_STRBUFLD_SCALED    = 0x76,
      ISA_STRBUFST_SCALED    = 0x77,
      ISA_GATHER_SCALED      = 0x78,
      ISA_SCATTER_SCALED     = 0x79,
      ISA_RAW_SENDS          = 0x7A,
      ISA_LIFETIME           = 0x7B,
      ISA_SBARRIER           = 0x7C,
      ISA_DWORD_ATOMIC       = 0x7D,
      ISA_SQRTM              = 0x7E,
      ISA_DIVM               = 0x7F,
    };

    enum {
      ISA_VA_AVS             = 0x00,
      ISA_VA_CONV            = 0x01,
      ISA_VA_MM              = 0x02,
      ISA_VA_MMF             = 0x03,
      ISA_VA_ERODE           = 0x04,
      ISA_VA_DILATE          = 0x05,
      ISA_VA_BOOLC           = 0x06,
      ISA_VA_CENTROID        = 0x07,
      ISA_VAPLUS_1DCONV_V    = 0x08,
      ISA_VAPLUS_1DCONV_H    = 0x09,
      ISA_VAPLUS_1PIXELCONV  = 0xA,
      ISA_VAPLUS_FLOODFILL   = 0xB,
      ISA_VAPLUS_LBPCREAT    = 0xC,
      ISA_VAPLUS_LBPCORRE    = 0xD,
      ISA_VAPLUS_CORRESEARCH = 0xF,
      ISA_HDC_CONV           = 0x10,
      ISA_HDC_MMF            = 0x11,
      ISA_HDC_ERODE          = 0x12,
      ISA_HDC_DILATE         = 0x13,
      ISA_HDC_LBPCORRE       = 0x14,
      ISA_HDC_LBPCREATE      = 0x15,
      ISA_HDC_1DCONV_H       = 0x16,
      ISA_HDC_1DCONV_V       = 0x17,
      ISA_HDC_1PIXELCONV     = 0x18,
      ISA_VA_UNDEFINED       = 0x19
    };

    // vISA relational operators
    enum { EQ, NE, GT, GE, LT, LE };

    // vISA types
    enum {
      TYPE_UD, TYPE_D, TYPE_UW, TYPE_W, TYPE_UB, TYPE_B, TYPE_DF, TYPE_F,
      TYPE_V, TYPE_VF, TYPE_Bool, TYPE_UQ, TYPE_UV, TYPE_Q, TYPE_HF };

    enum {
      CLASS_GENERAL, CLASS_ADDRESS, CLASS_PREDICATE, CLASS_INDIRECT,
      CLASS_IMMEDIATE = 5, CLASS_STATE };

    // vISA vector operand modifiers
    enum { MOD_ABS = 0x8, MOD_NEG = 0x10, MOD_NEGABS = 0x18,
      MOD_SAT = 0x20, MOD_NOT = 0x28 };

    enum { VISA_NUM_RESERVED_REGS = 32,
           VISA_NUM_RESERVED_PREDICATES = 1,
           VISA_NUM_RESERVED_SURFACES = 6 };

    enum {
      VISA_RESERVED_SURFACE_T0 = 0, // Shared local memory access
      VISA_RESERVED_SURFACE_T1,
      VISA_RESERVED_SURFACE_T2,
      VISA_RESERVED_SURFACE_T3,
      VISA_RESERVED_SURFACE_T4,
      VISA_RESERVED_SURFACE_T5
    };

    /// For a fixed surface index, return its vISA equivalent, or -1 if none.
    inline int getReservedSurfaceIndex(Value *Idx) {
      if (ConstantInt *CI = dyn_cast_or_null<ConstantInt>(Idx)) {
        switch ((uint32_t)CI->getZExtValue()) {
          case 254: // 254 is SLM, which is T0 in vISA
            return VISA_RESERVED_SURFACE_T0;
          case 255: // 255 is stateless, which is T5 in vISA
            return VISA_RESERVED_SURFACE_T5;
        }
      }
      return -1;
    }

    inline int getT5() { return 255; }

    enum { VISA_MAX_GENERAL_REGS = 65536 * 256 - 1,
           VISA_MAX_ADDRESS_REGS = 4096,
           VISA_MAX_PREDICATE_REGS = 4096,
           VISA_MAX_SAMPLER_REGS = 32 - 1,
           VISA_MAX_SURFACE_REGS = 256,
           VISA_MAX_VME_REGS = 16 };

    enum { VISA_WIDTH_GENERAL_REG = 32 };

    enum { VISA_ABI_INPUT_REGS_RESERVED = 1,
           VISA_ABI_INPUT_REGS_MAX = 128 };

    // vISA atomic opcodes
    enum {
      ATOMIC_ADD, ATOMIC_SUB, ATOMIC_INC, ATOMIC_DEC, ATOMIC_MIN, ATOMIC_MAX,
      ATOMIC_XCHG, ATOMIC_CMPXCHG, ATOMIC_AND, ATOMIC_OR, ATOMIC_XOR,
      ATOMIC_IMIN, ATOMIC_IMAX, ATOMIC_FMAX = 16, ATOMIC_FMIN, ATOMIC_FCMPWR,
    };

    // vISA SVM sub-opcodes
    enum {
      SVM_BLOCK_LD = 1,
      SVM_BLOCK_ST = 2,
      SVM_GATHER = 3,
      SVM_GATHER4_SCALED = 6,
      SVM_SCATTER = 4,
      SVM_SCATTER4_SCALED = 7,
      SVM_ATOMIC = 5
    };

    // vISA predefined registers
    enum {
      PREDEF_NULL = 0,
      PREDEF_X = 1,
      PREDEF_Y = 2,
      PREDEF_GROUP_ID_X = 3,
      PREDEF_GROUP_ID_Y = 4,
      PREDEF_GROUP_ID_Z = 5,
      PREDEF_TSC = 6,
      PREDEF_R0 = 7,
      PREDEF_ARG = 8,
      PREDEF_RET = 9,
      PREDEF_FE_SP = 10,
      PREDEF_FE_FP = 11,
      PREDEF_HW_TID = 12,
      PREDEF_SR0 = 13,
      PREDEF_CR0 = 14,
      PREDEF_CE0 = 25,
      PREDEF_DBG = 16,
      PREDEF_COLOR = 17
    };

  } // end namespace Visa

} // end namespace llvm
#endif // ndef GENXVISA_H
