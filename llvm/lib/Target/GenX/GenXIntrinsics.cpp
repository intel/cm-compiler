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

//===----------------------------------------------------------------------===//
//
// This file contains a table of extra information about the llvm.genx.*
// intrinsics, used by the vISA register allocator and function writer to
// decide exactly what operand type to use. The more usual approach in an LLVM
// target is to have an intrinsic map to an instruction in instruction
// selection, then have register category information on the instruction. But
// we are not using the target independent code generator, we are generating
// code directly from LLVM IR.
//
//===----------------------------------------------------------------------===//
#include "visa_igc_common_header.h"
#include "IsaDescription.h"
#include "GenXIntrinsics.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

// In this table:
//
// Each ALU and shared function intrinsic has a record giving information
// about its operands, and how it is written as a vISA instruction. The
// record has an initial field giving the intrinsic ID, then a number of
// fields where each corresponds to a field in the vISA instruction.
//
// A field may be several values combined with the | operator. The first
// value is the operand category (GENERAL etc), or one of a set of
// non-register operand categories (LITERAL, BYTE), or END to terminate
// the record. Other modifier values may be combined, such as SIGNED.
// The LLVM IR argument index plus 1 is also combined in, or 0 for the
// return value.

// Video Analytics intrinsic helper macros, mainly to avoid large blocks
// of near-identical code in the intrinsics look-up table and also to
// aid readability.

#define VA_CONV_OP(intrin, vsop) \
  Intrinsic::intrin,                          \
  LITERAL | ISA_VA,                           \
  LITERAL | vsop,                             \
  SAMPLER | 1,                                \
  SURFACE | 2,                                \
  GENERAL | 3,                                \
  GENERAL | 4,                                \
  BYTE | 5,                                   \
  RAW | 0,                                    \
  END

#define VA_HDC_CONV_OP(intrin, vsop) \
  Intrinsic::intrin,                          \
  LITERAL | ISA_VA_SKL_PLUS,                  \
  LITERAL | vsop,                             \
  SAMPLER | 1,                                \
  SURFACE | 2,                                \
  GENERAL | 3,                                \
  GENERAL | 4,                                \
  SURFACE | 5,                                \
  GENERAL | 6,                                \
  GENERAL | 7,                                \
  END

#define VA_SKL_1DCONV_OP(intrin, vsop) \
  Intrinsic::intrin,                          \
  LITERAL | ISA_VA_SKL_PLUS,                  \
  LITERAL | vsop,                             \
  SAMPLER | 1,                                \
  SURFACE | 2,                                \
  GENERAL | 3,                                \
  GENERAL | 4,                                \
  BYTE | 5,                                   \
  RAW | 0,                                    \
  END

#define VA_SKL_HDC_1DCONV_OP(intrin, vsop) \
  Intrinsic::intrin,                          \
  LITERAL | ISA_VA_SKL_PLUS,                  \
  LITERAL | vsop,                             \
  SAMPLER | 1,                                \
  SURFACE | 2,                                \
  GENERAL | 3,                                \
  GENERAL | 4,                                \
  BYTE | 5,                                   \
  SURFACE | 6,                                \
  GENERAL | 7,                                \
  GENERAL | 8,                                \
  END

#define VA_SKL_1PIXELCONV_OP(intrin, mode, offsets) \
  Intrinsic::intrin,                          \
  LITERAL | ISA_VA_SKL_PLUS,                  \
  LITERAL | VA_OP_CODE_1PIXEL_CONVOLVE,       \
  SAMPLER | 1,                                \
  SURFACE | 2,                                \
  GENERAL | 3,                                \
  GENERAL | 4,                                \
  mode,                                       \
  offsets,                                    \
  RAW | 0,                                    \
  END

const GenXIntrinsicInfo::DescrElementType GenXIntrinsicInfo::Table[] = {

  // Region access intrinsics do not appear in this table

  //--------------------------------------------------------------------
  // ALU type conversion intrinsics

  // fptosi, saturated
  Intrinsic::genx_fptosi_sat,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | MODIFIER_ARITH | 1, // src0
  END,

  // fptoui, saturated
  Intrinsic::genx_fptoui_sat,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | MODIFIER_ARITH | 1, // src0
  END,

  // sat
  Intrinsic::genx_sat,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | MODIFIER_ARITH | 1, // src0
  END,

  // trunc, unsigned result, unsigned operand, saturated
  Intrinsic::genx_uutrunc_sat,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  END,

  // trunc, unsigned result, signed operand, saturated
  Intrinsic::genx_ustrunc_sat,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  END,

  // trunc, signed result, unsigned operand, saturated
  Intrinsic::genx_sutrunc_sat,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  END,

  // trunc, signed result, signed operand, saturated
  Intrinsic::genx_sstrunc_sat,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  END,

  //--------------------------------------------------------------------
  // ALU intrinsics

  // non-saturating add: intrinsic not needed

  // add, signed result, signed operands, saturated
  Intrinsic::genx_ssadd_sat,
  LITERAL | ISA_ADD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // add, signed result, unsigned operands, saturated
  Intrinsic::genx_suadd_sat,
  LITERAL | ISA_ADD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // add, unsigned result, signed operands, saturated
  Intrinsic::genx_usadd_sat,
  LITERAL | ISA_ADD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // add, unsigned result, unsigned operands, saturated
  Intrinsic::genx_uuadd_sat,
  LITERAL | ISA_ADD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // addc not implemented as it has two results

  // asr not needed as it never overflows so a saturating asr with destination
  // smaller than source can be implemented by LLVM instruction Asr then
  // llvm.genx.sstrunc.sat.

  // avg, signed result, signed operands
  Intrinsic::genx_ssavg,
  LITERAL | ISA_AVG, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // avg, signed result, unsigned operands
  Intrinsic::genx_suavg,
  LITERAL | ISA_AVG, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // avg, unsigned result, signed operands
  Intrinsic::genx_usavg,
  LITERAL | ISA_AVG, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // avg, unsigned result, unsigned operands
  Intrinsic::genx_uuavg,
  LITERAL | ISA_AVG, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // avg, signed result, signed operands, saturated
  Intrinsic::genx_ssavg_sat,
  LITERAL | ISA_AVG, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // avg, signed result, unsigned operands, saturated
  Intrinsic::genx_suavg_sat,
  LITERAL | ISA_AVG, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // avg, unsigned result, signed operands, saturated
  Intrinsic::genx_usavg_sat,
  LITERAL | ISA_AVG, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // avg, unsigned result, unsigned operands, saturated
  Intrinsic::genx_uuavg_sat,
  LITERAL | ISA_AVG, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // bfe, signed result and operands (no source modifier)
  Intrinsic::genx_sbfe,
  LITERAL | ISA_BFE, // opcode
  EXECSIZE_NOT2, // execution size
  IMPLICITPRED, // predication
  GENERAL | OWALIGNED | SIGNED | 0, // dst (return value)
  GENERAL | OWALIGNED | SIGNED | 1, // src0
  GENERAL | OWALIGNED | SIGNED | 2, // src1
  GENERAL | OWALIGNED | SIGNED | 3, // src2
  END,

  // bfe, unsigned result and operands (no source modifier)
  Intrinsic::genx_ubfe,
  LITERAL | ISA_BFE, // opcode
  EXECSIZE_NOT2, // execution size
  IMPLICITPRED, // predication
  GENERAL | OWALIGNED | UNSIGNED | 0, // dst (return value)
  GENERAL | OWALIGNED | UNSIGNED | 1, // src0
  GENERAL | OWALIGNED | UNSIGNED | 2, // src1
  GENERAL | OWALIGNED | UNSIGNED | 3, // src2
  END,

  // bfi (no source modifier)
  Intrinsic::genx_bfi,
  LITERAL | ISA_BFI, // opcode
  EXECSIZE_NOT2, // execution size
  IMPLICITPRED, // predication
  GENERAL | OWALIGNED | 0, // dst (return value)
  GENERAL | OWALIGNED | UNSIGNED | 1, // src0
  GENERAL | OWALIGNED | UNSIGNED | 2, // src1
  GENERAL | OWALIGNED | UNSIGNED | 3, // src2
  GENERAL | OWALIGNED | UNSIGNED | 4, // src3
  END,

  // bfrev (no source modifier)
  Intrinsic::genx_bfrev,
  LITERAL | ISA_BFREV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  END,

  // cbit (no source modifier)
  Intrinsic::genx_cbit,
  LITERAL | ISA_CBIT, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  END,

  // cmp: no intrinsic needed

  // cos
  Intrinsic::genx_cos,
  LITERAL | ISA_COS, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,
    
  // div: no intrinsic needed

  // dp2
  Intrinsic::genx_dp2,
  LITERAL | ISA_DP2, // opcode
  EXECSIZE_GE4, // execution size
  IMPLICITPRED, // predication
  GENERAL | STRIDE1 | OWALIGNED | 0, // dst (return value)
  GENERAL | STRIDE1 | OWALIGNED | 1, // src0
  GENERAL | STRIDE1 | OWALIGNED | 2, // src1
  END,

  // dp3
  Intrinsic::genx_dp3,
  LITERAL | ISA_DP3, // opcode
  EXECSIZE_GE4, // execution size
  IMPLICITPRED, // predication
  GENERAL | STRIDE1 | OWALIGNED | 0, // dst (return value)
  GENERAL | STRIDE1 | OWALIGNED | 1, // src0
  GENERAL | STRIDE1 | OWALIGNED | 2, // src1
  END,

  // dp4
  Intrinsic::genx_dp4,
  LITERAL | ISA_DP4, // opcode
  EXECSIZE_GE4, // execution size
  IMPLICITPRED, // predication
  GENERAL | STRIDE1 | OWALIGNED | 0, // dst (return value)
  GENERAL | STRIDE1 | OWALIGNED | 1, // src0
  GENERAL | STRIDE1 | OWALIGNED | 2, // src1
  END,

  // dph
  Intrinsic::genx_dph,
  LITERAL | ISA_DPH, // opcode
  EXECSIZE_GE4, // execution size
  IMPLICITPRED, // predication
  GENERAL | STRIDE1 | OWALIGNED | 0, // dst (return value)
  GENERAL | STRIDE1 | OWALIGNED | 1, // src0
  GENERAL | STRIDE1 | OWALIGNED | 2, // src1
  END,

  // exp
  Intrinsic::genx_exp,
  LITERAL | ISA_EXP, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // fbh, signed operand (no source modifier)
  Intrinsic::genx_sfbh,
  LITERAL | ISA_FBH, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | SIGNED | 1, // src0
  END,

  // fbh, unsigned operand (no source modifier)
  Intrinsic::genx_ufbh,
  LITERAL | ISA_FBH, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  END,

  // fbl (no source modifier)
  Intrinsic::genx_fbl,
  LITERAL | ISA_FBL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  END,

  // frc (no saturation)
  Intrinsic::genx_frc,
  LITERAL | ISA_FRC, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SATURATION_NOSAT | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // inv
  Intrinsic::genx_inv,
  LITERAL | ISA_INV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // line
  Intrinsic::genx_line,
  LITERAL | ISA_LINE, // opcode
  EXECSIZE_GE4, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | FIXED4 | NOIMM | 1, // src0
  GENERAL | 2, // src1
  END,

  // log
  Intrinsic::genx_log,
  LITERAL | ISA_LOG, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // lrp
  Intrinsic::genx_lrp,
  LITERAL | ISA_LRP, // opcode
  EXECSIZE_GE4, // execution size
  IMPLICITPRED, // predication
  GENERAL | OWALIGNED | CONTIGUOUS | 0, // dst (return value)
  GENERAL | OWALIGNED | SCALARORCONTIGUOUS | NOIMM | 1, // src0
  GENERAL | OWALIGNED | SCALARORCONTIGUOUS | NOIMM | 2, // src1
  GENERAL | OWALIGNED | SCALARORCONTIGUOUS | NOIMM | 3, // src2
  END,

  // lzd
  Intrinsic::genx_lzd,
  LITERAL | ISA_LZD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  END,

  Intrinsic::fma,
  LITERAL | ISA_MAD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | MODIFIER_ARITH | 1, // src0
  GENERAL | MODIFIER_ARITH | 2, // src1
  GENERAL | MODIFIER_ARITH | 3, // src2
  END,

  // mad, signed result, signed operands
  Intrinsic::genx_ssmad,
  LITERAL | ISA_MAD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  GENERAL | SIGNED | CONTIGUOUS | 3, // src2
  END,

  // mad, signed result, unsigned operands
  Intrinsic::genx_sumad,
  LITERAL | ISA_MAD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  GENERAL | UNSIGNED | CONTIGUOUS | 3, // src2
  END,

  // mad, unsigned result, signed operands
  Intrinsic::genx_usmad,
  LITERAL | ISA_MAD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  GENERAL | SIGNED | CONTIGUOUS | 3, // src2
  END,

  // mad, unsigned result, unsigned operands
  Intrinsic::genx_uumad,
  LITERAL | ISA_MAD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  GENERAL | UNSIGNED | CONTIGUOUS | 3, // src2
  END,

  // mad, signed result, signed operands, saturated
  Intrinsic::genx_ssmad_sat,
  LITERAL | ISA_MAD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  GENERAL | SIGNED | CONTIGUOUS | 3, // src2
  END,

  // mad, signed result, unsigned operands, saturated
  Intrinsic::genx_sumad_sat,
  LITERAL | ISA_MAD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  GENERAL | UNSIGNED | CONTIGUOUS | 3, // src2
  END,

  // mad, unsigned result, signed operands, saturated
  Intrinsic::genx_usmad_sat,
  LITERAL | ISA_MAD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  GENERAL | SIGNED | CONTIGUOUS | 3, // src2
  END,

  // mad, unsigned result, unsigned operands, saturated
  Intrinsic::genx_uumad_sat,
  LITERAL | ISA_MAD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  GENERAL | UNSIGNED | CONTIGUOUS | 3, // src2
  END,

  // max, signed result and operands
  // (No need to represent saturation in intrinsic as max cannot overflow
  // so saturation can be represented by separate intrinsic.)
  Intrinsic::genx_smax,
  LITERAL | ISA_FMINMAX, // opcode
  EXECSIZE, // execution size
  LITERAL | 1, // flag for max
  GENERAL | SIGNED | SATURATION_INTALLOWED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // max, unsigned result and operands
  // (No need to represent saturation in intrinsic as max cannot overflow
  // so saturation can be represented by separate intrinsic.)
  Intrinsic::genx_umax,
  LITERAL | ISA_FMINMAX, // opcode
  EXECSIZE, // execution size
  LITERAL | 1, // flag for max
  GENERAL | UNSIGNED | SATURATION_INTALLOWED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // max, float result and operands
  Intrinsic::genx_fmax,
  LITERAL | ISA_FMINMAX, // opcode
  EXECSIZE, // execution size
  LITERAL | 1, // flag for max
  GENERAL | 0, // dst (return value)
  GENERAL | MODIFIER_ARITH | 1, // src0
  GENERAL | MODIFIER_ARITH | 2, // src1
  END,

  // min, signed result and operands
  // (No need to represent saturation in intrinsic as min cannot overflow
  // so saturation can be represented by separate intrinsic.)
  Intrinsic::genx_smin,
  LITERAL | ISA_FMINMAX, // opcode
  EXECSIZE, // execution size
  LITERAL | 0, // flag for max
  GENERAL | SIGNED | SATURATION_INTALLOWED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // min, unsigned result and operands
  // (No need to represent saturation in intrinsic as min cannot overflow
  // so saturation can be represented by separate intrinsic.)
  Intrinsic::genx_umin,
  LITERAL | ISA_FMINMAX, // opcode
  EXECSIZE, // execution size
  LITERAL | 0, // flag for max
  GENERAL | UNSIGNED | SATURATION_INTALLOWED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // min, float result and operands
  Intrinsic::genx_fmin,
  LITERAL | ISA_FMINMAX, // opcode
  EXECSIZE, // execution size
  LITERAL | 0, // flag for max
  GENERAL | 0, // dst (return value)
  GENERAL | MODIFIER_ARITH | 1, // src0
  GENERAL | MODIFIER_ARITH | 2, // src1
  END,

  // mod: intrinsic not needed

  // mul, signed result, signed operands
  Intrinsic::genx_ssmul,
  LITERAL | ISA_MUL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // mul, signed result, signed and unsigned operands
  Intrinsic::genx_sumul,
  LITERAL | ISA_MUL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // mul, signed result, unsigned and signed operands
  Intrinsic::genx_usmul,
  LITERAL | ISA_MUL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // mul, signed result, unsigned operands
  Intrinsic::genx_uumul,
  LITERAL | ISA_MUL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // mul, signed result, signed operands, saturated
  Intrinsic::genx_ssmul_sat,
  LITERAL | ISA_MUL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // mul, signed result, unsigned operands, saturated
  Intrinsic::genx_sumul_sat,
  LITERAL | ISA_MUL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // mul, unsigned result, signed operands, saturated
  Intrinsic::genx_usmul_sat,
  LITERAL | ISA_MUL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // mul, unsigned result, unsigned operands, saturated
  Intrinsic::genx_uumul_sat,
  LITERAL | ISA_MUL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // mulh, signed result and operands
  Intrinsic::genx_smulh,
  LITERAL | ISA_MULH, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // mulh, unsigned result and operands
  Intrinsic::genx_umulh,
  LITERAL | ISA_MULH, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // not: intrinsic not needed

  // or: intrinsic not needed

  // pln
  Intrinsic::genx_pln,
  LITERAL | ISA_PLANE, // opcode
  EXECSIZE_GE8, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | OWALIGNED | FIXED4 | NOIMM | 1, // src0
  GENERAL | GRFALIGNED | TWICEWIDTH | NOIMM | 2, // src1
  END,

  // pow
  Intrinsic::genx_pow,
  LITERAL | ISA_POW, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  GENERAL | 2, // src1
  END,

  // rndd
  Intrinsic::genx_rndd,
  LITERAL | ISA_RNDD, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // rnde
  Intrinsic::genx_rnde,
  LITERAL | ISA_RNDE, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // rndu
  Intrinsic::genx_rndu,
  LITERAL | ISA_RNDU, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // rndz
  Intrinsic::genx_rndz,
  LITERAL | ISA_RNDZ, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // rsqrt
  Intrinsic::genx_rsqrt,
  LITERAL | ISA_RSQRT, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  Intrinsic::genx_ssad2,
  LITERAL | ISA_SAD2, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | SIGNED | 1, // src0
  GENERAL | SIGNED | 2, // src1
  END,

  Intrinsic::genx_usad2,
  LITERAL | ISA_SAD2, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  GENERAL | UNSIGNED | 2, // src1
  END,

  // sssad2add, signed
  Intrinsic::genx_sssad2add,
  LITERAL | ISA_SAD2ADD, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | SIGNED | 1, // src0
  GENERAL | SIGNED | 2, // src1
  GENERAL | SIGNED | 3, // src2
  END,

  // uusad2add, unsigned
  Intrinsic::genx_uusad2add,
  LITERAL | ISA_SAD2ADD, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  GENERAL | UNSIGNED | 2, // src1
  GENERAL | UNSIGNED | 3, // src2
  END,

  // susad2add, signed result, unsigned src
  Intrinsic::genx_susad2add,
  LITERAL | ISA_SAD2ADD, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  GENERAL | UNSIGNED | 2, // src1
  GENERAL | SIGNED | 3, // src2
  END,

  // ussad2add, unsigned result, signed src
  Intrinsic::genx_ussad2add,
  LITERAL | ISA_SAD2ADD, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | SIGNED | 1, // src0
  GENERAL | SIGNED | 2, // src1
  GENERAL | UNSIGNED | 3, // src2
  END,

  // sssad2add, signed, saturated
  Intrinsic::genx_sssad2add_sat,
  LITERAL | ISA_SAD2ADD, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | 1, // src0
  GENERAL | SIGNED | 2, // src1
  GENERAL | SIGNED | 3, // src2
  END,

  // uusad2add, unsigned, saturated
  Intrinsic::genx_uusad2add_sat,
  LITERAL | ISA_SAD2ADD, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  GENERAL | UNSIGNED | 2, // src1
  GENERAL | UNSIGNED | 3, // src2
  END,

  // susad2add, signed result, unsigned src, saturated
  Intrinsic::genx_susad2add_sat,
  LITERAL | ISA_SAD2ADD, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | 1, // src0
  GENERAL | UNSIGNED | 2, // src1
  GENERAL | SIGNED | 3, // src2
  END,

  // ussad2add, unsigned result, signed src, saturated
  Intrinsic::genx_ussad2add_sat,
  LITERAL | ISA_SAD2ADD, // opcode
  EXECSIZE_GE2, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | 1, // src0
  GENERAL | SIGNED | 2, // src1
  GENERAL | UNSIGNED | 3, // src2
  END,

  // shl, signed result, signed operands
  Intrinsic::genx_ssshl,
  LITERAL | ISA_SHL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // shl, signed result, unsigned operands
  Intrinsic::genx_sushl,
  LITERAL | ISA_SHL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // shl, unsigned result, signed operands
  Intrinsic::genx_usshl,
  LITERAL | ISA_SHL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // shl, unsigned result, unsigned operands
  Intrinsic::genx_uushl,
  LITERAL | ISA_SHL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // shl, signed result, signed operands, saturated
  Intrinsic::genx_ssshl_sat,
  LITERAL | ISA_SHL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // shl, signed result, unsigned operands, saturated
  Intrinsic::genx_sushl_sat,
  LITERAL | ISA_SHL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | SIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // shl, unsigned result, signed operands, saturated
  Intrinsic::genx_usshl_sat,
  LITERAL | ISA_SHL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | SIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | SIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // shl, unsigned result, unsigned operands, saturated
  Intrinsic::genx_uushl_sat,
  LITERAL | ISA_SHL, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | UNSIGNED | SATURATION_SATURATE | 0, // dst (return value)
  GENERAL | UNSIGNED | MODIFIER_ARITH | 1, // src0
  GENERAL | UNSIGNED | MODIFIER_ARITH | 2, // src1
  END,

  // shr not needed as it never overflows so a saturating shr with destination
  // smaller than source can be implemented by LLVM instruction Shr then
  // llvm.genx.uutrunc.sat.

  // sin
  Intrinsic::genx_sin,
  LITERAL | ISA_SIN, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // sqrt
  Intrinsic::genx_sqrt,
  LITERAL | ISA_SQRT, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // ieee_sqrt
  Intrinsic::genx_ieee_sqrt,
  LITERAL | ISA_SQRTM, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  // ieee_div
  Intrinsic::genx_ieee_div,
  LITERAL | ISA_DIVM, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  GENERAL | 2, // src1
  END,


  // subb not implemented as it has two results

  // xor: intrinsic not needed

  //--------------------------------------------------------------------
  // vISA reserved register intrinsics

#define VARNUM(a) LITERAL | (a), LITERAL | 0, LITERAL | 0, LITERAL | 0

  Intrinsic::genx_thread_x,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_X), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x21, LITERAL | 1, // v1
  END,

  Intrinsic::genx_thread_y,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_Y), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x21, LITERAL | 1, // v2
  END,

  Intrinsic::genx_group_id_x,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_GROUP_ID_X), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x21, LITERAL | 1, // v7
  END,

  Intrinsic::genx_group_id_y,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_GROUP_ID_Y), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x21, LITERAL | 1, // v8
  END,

  Intrinsic::genx_group_id_z,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_GROUP_ID_Z), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x21, LITERAL | 1, // v23
  END,
  
  Intrinsic::genx_timestamp,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_TSC), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x22, LITERAL | 1, // v11(0,0)<1;1,0>
  END,

  Intrinsic::genx_r0,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_R0), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x22, LITERAL | 1, // v12(0,0)<1;1,0>
  END,

  Intrinsic::genx_sr0,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_SR0), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x22, LITERAL | 1, // v13(0,0)<1;1,0>
  END,

  Intrinsic::genx_set_sr0_2,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE_NOMASK, // execution size
  IMPLICITPRED, // predication
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_SR0), LITERAL | 0,
  LITERAL | 2, LITERAL | 0, LITERAL | 2, // v13(0,2)<1>
  GENERAL | 1, // src (sr0.2 value)
  END,

  Intrinsic::genx_get_color,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_COLOR), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x22, LITERAL | 1, // v22(0,0)<1;1,0>
  END,

  Intrinsic::genx_get_hwid,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_HW_TID), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x21, LITERAL | 1, // v12(0,0)<0;1,0>
  END,

  Intrinsic::genx_set_pause,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_TSC), LITERAL | 0,
      LITERAL | 4, LITERAL | 0x22, LITERAL | 2, // v11(0,4)<1;1,1>
  GENERAL | 1, // src (pause length)
  END,

  Intrinsic::genx_dummy_mov,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  LITERAL | visa::CLASS_GENERAL, VARNUM(PREDEFINED_NULL), LITERAL | 0,
      LITERAL | 0, LITERAL | 0x22, LITERAL | 2, // v0(0,0)<1;1,1>
  GENERAL | 1, // src (dummy mov src)
  END,

  //--------------------------------------------------------------------
  // shared function intrinsics

  Intrinsic::genx_dword_atomic_add,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_ADD, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  URAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_sub,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_SUB, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  URAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_inc,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_INC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  NULLRAW, // src (null variable)
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_dec,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_DEC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  NULLRAW, // src (null variable)
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_min,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_MIN, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  URAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_max,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_MAX, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  URAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_xchg,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_XCHG, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  URAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_cmpxchg,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_CMPXCHG, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  URAW | 4, // src
  URAW | 5, // src1
  TWOADDR | 6, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_and,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_AND, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  URAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_or,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_OR, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  URAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_xor,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_XOR, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  URAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_imin,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_IMIN, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  SRAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  SRAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_imax,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_IMAX, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  SRAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  SRAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_fmax,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_FMAX, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  RAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  RAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_fmin,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_FMIN, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  RAW | 4, // src
  NULLRAW, // src1 (null variable)
  TWOADDR | 5, // not in vISA instruction: old value of result
  RAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_dword_atomic_fcmpwr,
  LITERAL | ISA_DWORD_ATOMIC, // opcode
  LITERAL | ATOMIC_FCMPWR, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // element offset
  RAW | 4, // src
  RAW | 5, // src1
  TWOADDR | 6, // not in vISA instruction: old value of result
  RAW | RAW_NULLALLOWED | 0, // dst
  END,


  Intrinsic::genx_typed_atomic_add,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_ADD, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_sub,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_SUB, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_inc,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_INC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // U
  URAW | RAW_NULLALLOWED | 4, // V
  URAW | RAW_NULLALLOWED | 5, // R
  URAW | RAW_NULLALLOWED | 6, // LOD
  NULLRAW, // src0 (null variable)
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_dec,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_DEC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 3, // U
  URAW | RAW_NULLALLOWED | 4, // V
  URAW | RAW_NULLALLOWED | 5, // R
  URAW | RAW_NULLALLOWED | 6, // LOD
  NULLRAW, // src0 (null variable)
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_min,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_MIN, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_max,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_MAX, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_xchg,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_XCHG, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_cmpxchg,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_CMPXCHG, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 5, // U
  URAW | RAW_NULLALLOWED | 6, // V
  URAW | RAW_NULLALLOWED | 7, // R
  URAW | RAW_NULLALLOWED | 8, // LOD
  URAW | 3, // src0
  URAW | 4, // src1
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_and,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_AND, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_or,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_OR, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_xor,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_XOR, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_imin,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_IMIN, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_imax,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_IMAX, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_fmax,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_FMAX, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_fmin,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_FMIN, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 4, // U
  URAW | RAW_NULLALLOWED | 5, // V
  URAW | RAW_NULLALLOWED | 6, // R
  URAW | RAW_NULLALLOWED | 7, // LOD
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  URAW | RAW_NULLALLOWED | 0, // dst
  END,

  Intrinsic::genx_typed_atomic_fcmpwr,
  LITERAL | ISA_3D_TYPED_ATOMIC, // opcode
  LITERAL | ATOMIC_FCMPWR, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size
  PREDICATION | 1, // predication
  SURFACE | 2, // surface index
  URAW | 5, // U
  URAW | RAW_NULLALLOWED | 6, // V
  URAW | RAW_NULLALLOWED | 7, // R
  URAW | RAW_NULLALLOWED | 8, // LOD
  URAW | 3, // src0
  URAW | 4, // src1
  URAW | RAW_NULLALLOWED | 0, // dst
  END,


  Intrinsic::genx_gather_scaled,
  LITERAL | ISA_GATHER_SCALED, // opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | 0, // block size (MBZ)
  BYTE | 2, // log2 num blocks
  SHORT | 3, // scale
  SURFACE | 4, // surface index
  GENERAL | UNSIGNED | 5, // global offset
  URAW | 6, // element offset
  TWOADDR | 7, // not in vISA instruction: old value of result
  RAW | 0, // dst (return value)
  END,

  Intrinsic::genx_gather4_scaled,
  LITERAL | ISA_GATHER4_SCALED, // opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  BYTE | 2, // channel mask
  SHORT | 3, // scale
  SURFACE | 4, // surface index
  GENERAL | UNSIGNED | 5, // global offset
  URAW | 6, // element offset
  TWOADDR | 7, // not in vISA instruction: old value of result
  RAW | 0, // dst (return value)
  END,

  Intrinsic::genx_gather4_typed,
  LITERAL | ISA_GATHER4_TYPED, // opcode
  EXECSIZE_FROM_ARG | 2, // execution size (must be 8)
  PREDICATION | 2, // predicate
  BYTE | 1, // channel mask
  SURFACE | 3, // surface index
  URAW | 4, // U pixel address
  URAW | RAW_NULLALLOWED | 5, // V pixel address
  URAW | RAW_NULLALLOWED | 6, // R pixel address
  NULLRAW, // LOD
  TWOADDR | 7, // not in vISA instruction: old value of result
  RAW | 0, // dst (return value)
  END,

  Intrinsic::genx_media_ld,
  LITERAL | ISA_MEDIA_LD, // opcode
  BYTE | 1, // modifiers
  SURFACE | 2, // surface index
  BYTE | 3, // plane
  BYTE | 4, // block width
  MEDIAHEIGHT | 4, // block height
  GENERAL | UNSIGNED | 5, // x offset
  GENERAL | UNSIGNED | 6, // y offset
  RAW | 0, // dst (return value)
  END,

  Intrinsic::genx_media_st,
  LITERAL | ISA_MEDIA_ST, // opcode
  BYTE | 1, // modifiers
  SURFACE | 2, // surface index
  BYTE | 3, // plane
  BYTE | 4, // block width
  MEDIAHEIGHT | 4, // block height
  GENERAL | UNSIGNED | 5, // x offset
  GENERAL | UNSIGNED | 6, // y offset
  RAW | 7, // src
  END,

  Intrinsic::genx_oword_ld,
  LITERAL | ISA_OWORD_LD, // opcode
  LOG2OWORDS | 0, // size, log2 number of owords
  BYTE | 1, // is_modified
  SURFACE | 2, // surface index
  GENERAL | UNSIGNED | 3, // offset
  RAW | 0, // dst (return value)
  END,

  Intrinsic::genx_oword_ld_unaligned,
  LITERAL | ISA_OWORD_LD_UNALIGNED, // opcode
  LOG2OWORDS | 0, // size, log2 number of owords
  BYTE | 1, // is_modified
  SURFACE | 2, // surface index
  GENERAL | UNSIGNED | 3, // offset
  RAW | 0, // dst (return value)
  END,

  Intrinsic::genx_oword_st,
  LITERAL | ISA_OWORD_ST, // opcode
  LOG2OWORDS | 3, // log2 number of owords
  SURFACE | 1, // surface index
  GENERAL | UNSIGNED | 2, // offset
  RAW | 3, // src
  END,

  Intrinsic::genx_scatter_scaled,
  LITERAL | ISA_SCATTER_SCALED, // opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | 0, // 1 byte block size (MBZ)
  BYTE | 2, // log2 num blocks
  SHORT | 3, // scale
  SURFACE | 4, // surface index
  GENERAL | UNSIGNED | 5, // global offset
  URAW | 6, // element offset
  RAW | 7, // src
  END,

  Intrinsic::genx_scatter4_scaled,
  LITERAL | ISA_SCATTER4_SCALED, // opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  BYTE | 2, // channel mask
  SHORT | 3, // scale
  SURFACE | 4, // surface index
  GENERAL | UNSIGNED | 5, // global offset
  URAW | 6, // element offset
  RAW | 7, // src
  END,

  Intrinsic::genx_scatter4_typed,
  LITERAL | ISA_SCATTER4_TYPED, // opcode
  EXECSIZE_FROM_ARG | 2, // execution size (must be 8)
  PREDICATION | 2, // predicate
  BYTE | 1, // channel mask
  SURFACE | 3, // surface index
  URAW | 4, // U pixel address
  URAW | RAW_NULLALLOWED | 5, // V pixel address
  URAW | RAW_NULLALLOWED | 6, // R pixel address
  NULLRAW, // LOD
  RAW | 7, // src
  END,

  Intrinsic::genx_svm_block_ld,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_BLOCK_LD, // sub-opcode
  LOG2OWORDS | 0, // log2 number of owords
  GENERAL | UNSIGNED | 1, // address
  RAW | 0, // dst
  END,

  Intrinsic::genx_svm_block_ld_unaligned,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_BLOCK_LD, // sub-opcode
  LOG2OWORDS_PLUS_8 | 0, // log2 number of owords with unaligned flag
  GENERAL | UNSIGNED | 1, // address
  RAW | 0, // dst
  END,

  Intrinsic::genx_svm_block_st,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_BLOCK_ST, // sub-opcode
  LOG2OWORDS | 2, // log2 number of owords
  GENERAL | UNSIGNED | 1, // address
  RAW | 2, // src
  END,

  Intrinsic::genx_svm_gather,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_GATHER, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  SVMGATHERBLOCKSIZE | 0, // block size inferred from dst
  BYTE | 2, // log2 num blocks
  URAW | 3, // address
  TWOADDR | 4, // not in vISA instruction: old value of result
  RAW | 0, // dst (return value)
  END,

  Intrinsic::genx_svm_gather4_scaled,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_GATHER4SCALED, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  BYTE | 2, // channel mask
  SHORT | 3, // scale
  GENERAL | UNSIGNED | 4, // address
  URAW | 5, // element offset
  TWOADDR | 6, // not in vISA instruction: old value of result
  RAW | 0, // dst (return value)
  END,

  Intrinsic::genx_svm_scatter,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_SCATTER, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  SVMGATHERBLOCKSIZE | 4, // block size inferred from dst
  BYTE | 2, // log2 num blocks
  URAW | 3, // address
  RAW | 4, // src
  END,

  Intrinsic::genx_svm_scatter4_scaled,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_SCATTER4SCALED, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  BYTE | 2, // channel mask
  SHORT | 3, // scale
  GENERAL | UNSIGNED | 4, // address
  URAW | 5, // element offset
  RAW | 6, // src
  END,

  Intrinsic::genx_svm_atomic_add,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_ADD, // sub-opcode
  URAW | 2, // address
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_sub,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_SUB, // sub-opcode
  URAW | 2, // address
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_min,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_MIN, // sub-opcode
  URAW | 2, // address
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_max,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_MAX, // sub-opcode
  URAW | 2, // address
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_xchg,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_XCHG, // sub-opcode
  URAW | 2, // address
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_and,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_AND, // sub-opcode
  URAW | 2, // address
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_or,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_OR, // sub-opcode
  URAW | 2, // address
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_xor,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_XOR, // sub-opcode
  URAW | 2, // address
  URAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_imin,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_IMIN, // sub-opcode
  URAW | 2, // address
  SRAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  SRAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_imax,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_IMAX, // sub-opcode
  URAW | 2, // address
  SRAW | 3, // src0
  NULLRAW, // src1 (null variable)
  TWOADDR | 4, // not in vISA instruction: old value of result
  SRAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_inc,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_INC, // sub-opcode
  URAW | 2, // address
  NULLRAW, // src0 (null variable)
  NULLRAW, // src1 (null variable)
  TWOADDR | 3, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_dec,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_DEC, // sub-opcode
  URAW | 2, // address
  NULLRAW, // src0 (null variable)
  NULLRAW, // src1 (null variable)
  TWOADDR | 3, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_svm_atomic_cmpxchg,
  LITERAL | ISA_SVM, // opcode
  LITERAL | SVM_ATOMIC, // sub-opcode
  EXECSIZE_FROM_ARG | 1, // execution size (must be 8 or 16)
  PREDICATION | 1, // predicate
  LITERAL | ATOMIC_CMPXCHG, // sub-opcode
  URAW | 2, // address
  URAW | 3, // src0
  URAW | 4, // src1
  TWOADDR | 5, // not in vISA instruction: old value of result
  URAW | 0, // dst
  END,

  Intrinsic::genx_load,
  LITERAL | ISA_LOAD, // opcode
  SAMPLECHMASK | 1, // channel mask + simd_mode inferred from pixel addr operands
  SURFACE | 2, // surface index
  RAW | 3, // U pixel address
  RAW | 4, // V pixel address
  RAW | 5, // R pixel address
  RAW | 0, // dst
  END,

  Intrinsic::genx_sample,
  LITERAL | ISA_SAMPLE, // opcode
  SAMPLECHMASK | 1, // channel mask + simd_mode inferred from pixel addr operands
  SAMPLER | 2, // sampler index
  SURFACE | 3, // surface index
  RAW | 4, // U pixel address
  RAW | 5, // V pixel address
  RAW | 6, // R pixel address
  RAW | 0, // dst
  END,

  Intrinsic::genx_sample_unorm,
  LITERAL | ISA_SAMPLE_UNORM, // opcode
  BYTE | 1, // channel mask
  SAMPLER | 2, // sampler index
  SURFACE | 3, // surface index
  GENERAL | 4, // U pixel address
  GENERAL | 5, // V pixel address
  GENERAL | 6, // deltaU
  GENERAL | 7, // deltaV
  RAW | 0, // dst
  END,

  Intrinsic::genx_avs,
  LITERAL | ISA_AVS, // opcode
  BYTE | 1, // channel mask
  SAMPLER | 2, // sampler index
  SURFACE | 3, // surface index
  GENERAL | 4, // U pixel address
  GENERAL | 5, // V pixel address
  GENERAL | 6, // deltaU
  GENERAL | 7, // deltaV
  GENERAL | 8, // u2d
  GENERAL | UNSIGNED | 9, // groupID
  GENERAL | UNSIGNED | 10, // verticalBlockNumber
  BYTE | 11, // output format control
  GENERAL | 12, // v2d
  BYTE | 13, // execMode
  GENERAL | UNSIGNED | 14, // IEFByPass
  RAW | 0, // dst
  END,

  Intrinsic::genx_3d_sample,
  LITERAL | ISA_3D_SAMPLE, // opcode
  BYTE | 1, // 3d sampling opcode
  EXECSIZE_FROM_ARG | 2, // execution size
  PREDICATION | 2, // predication
  BYTE | 3, // channel mask
  GENERAL | UNSIGNED | 4, // aoffimmi value
  SAMPLER | 5, // sampler index
  SURFACE | 6, // surface index
  RAW | 0, // destination
  ARGCOUNT | ARGCOUNTMIN1 | 7, // number of additional operands, minumum 1
  RAW | 7, // first operand (mandatory)
  RAW | 8, // additional optional operand
  RAW | 9, // additional optional operand
  RAW | 10, // additional optional operand
  RAW | 11, // additional optional operand
  RAW | 12, // additional optional operand
  RAW | 13, // additional optional operand
  RAW | 14, // additional optional operand
  RAW | 15, // additional optional operand
  RAW | 16, // additional optional operand
  RAW | 17, // additional optional operand
  RAW | 18, // additional optional operand
  RAW | 19, // additional optional operand
  RAW | 20, // additional optional operand
  RAW | 21, // additional optional operand
  END,

  Intrinsic::genx_3d_load,
  LITERAL | ISA_3D_LOAD, // opcode
  BYTE | 1, // 3d sampling opcode
  EXECSIZE_FROM_ARG | 2, // execution size
  PREDICATION | 2, // predication
  BYTE | 3, // channel mask
  GENERAL | UNSIGNED | 4, // aoffimmi value
  SURFACE | 5, // surface index
  RAW | 0, // destination
  ARGCOUNT | ARGCOUNTMIN1 | 6, // number of additional operands, minumum 1
  RAW | 6, // first operand (mandatory)
  RAW | 7, // additional optional operand
  RAW | 8, // additional optional operand
  RAW | 9, // additional optional operand
  RAW | 10, // additional optional operand
  RAW | 11, // additional optional operand
  RAW | 12, // additional optional operand
  RAW | 13, // additional optional operand
  RAW | 14, // additional optional operand
  RAW | 15, // additional optional operand
  RAW | 16, // additional optional operand
  RAW | 17, // additional optional operand
  RAW | 18, // additional optional operand
  RAW | 19, // additional optional operand
  RAW | 20, // additional optional operand
  END,

  VA_CONV_OP(genx_va_convolve2d, Convolve_FOPCODE),
  VA_CONV_OP(genx_va_erode,      ERODE_FOPCODE),
  VA_CONV_OP(genx_va_dilate,     Dilate_FOPCODE),

  VA_HDC_CONV_OP(genx_va_hdc_erode,   ISA_HDC_ERODE),
  VA_HDC_CONV_OP(genx_va_hdc_dilate,  ISA_HDC_DILATE),

  Intrinsic::genx_va_minmax,
  LITERAL | ISA_VA,               // primary opcode
  LITERAL | MINMAX_FOPCODE,       // sub-opcode
  SURFACE | 1,                    // surface index
  GENERAL | 2,                    // normalized x co-ordinate
  GENERAL | 3,                    // normalized y co-ordinate
  GENERAL | 4,                    // Min Max Enable
  RAW | 0,                        // Destination
  END,

  Intrinsic::genx_va_minmax_filter,
  LITERAL | ISA_VA,               // primary opcode
  LITERAL | MINMAXFILTER_FOPCODE, // sub-opcode
  SAMPLER | 1,                    // sampler index
  SURFACE | 2,                    // surface index
  GENERAL | 3,                    // normalized x co-ordinate
  GENERAL | 4,                    // normalized y co-ordinate
  BYTE | 5,                       // output size
  BYTE | 6,                       // return data format
  GENERAL | 7,                    // Min Max Enable
  RAW | 0,                        // Destination
  END,

  Intrinsic::genx_va_hdc_minmax_filter,
  LITERAL | ISA_VA_SKL_PLUS,
  LITERAL | ISA_HDC_MMF,
  SAMPLER | 1,                    // sampler index
  SURFACE | 2,                    // surface index
  GENERAL | 3,                    // normalized x co-ordinate
  GENERAL | 4,                    // normalized y co-ordinate
  BYTE | 5,                       // return data format
  BYTE | 6,                       // minmax enable mode
  SURFACE | 7,                    // destination surface
  GENERAL | 8,                    // destination x offset
  GENERAL | 9,                    // destination y offset
  END,

  Intrinsic::genx_va_bool_centroid,
  LITERAL | ISA_VA,                 // primary opcode
  LITERAL | BoolCentroid_FOPCODE,   // sub-opcode
  SURFACE | 1,                      // surface index
  GENERAL | 2,                      // normalized x co-ordinate
  GENERAL | 3,                      // normalized y co-ordinate
  GENERAL | 4,                      // vSize
  GENERAL | 5,                      // hSize
  RAW | 0,                          // Destination
  END,

  Intrinsic::genx_va_centroid,
  LITERAL | ISA_VA,                 // primary opcode
  LITERAL | Centroid_FOPCODE,       // sub-opcode
  SURFACE | 1,                      // surface index
  GENERAL | 2,                      // normalized x co-ordinate
  GENERAL | 3,                      // normalized y co-ordinate
  GENERAL | 4,                      // vSize
  RAW | 0,                          // Destination
  END,

  VA_SKL_1DCONV_OP(genx_va_1d_convolve_horizontal, VA_OP_CODE_1D_CONVOLVE_HORIZONTAL),
  VA_SKL_1DCONV_OP(genx_va_1d_convolve_vertical, VA_OP_CODE_1D_CONVOLVE_VERTICAL),

  VA_SKL_HDC_1DCONV_OP(genx_va_hdc_1d_convolve_horizontal, ISA_HDC_1DCONV_H),
  VA_SKL_HDC_1DCONV_OP(genx_va_hdc_1d_convolve_vertical, ISA_HDC_1DCONV_V),

  VA_SKL_1PIXELCONV_OP(genx_va_1pixel_convolve, BYTE | 5, RAW | 6),
  VA_SKL_1PIXELCONV_OP(genx_va_1pixel_convolve_1x1mode, LITERAL | 3, NULLRAW),

  Intrinsic::genx_va_hdc_1pixel_convolve,
  LITERAL | ISA_VA_SKL_PLUS,
  LITERAL | ISA_HDC_1PIXELCONV,
  SAMPLER | 1,                    // sampler
  SURFACE | 2,                    // surface index
  GENERAL | 3,                    // normalized x co-ordinate
  GENERAL | 4,                    // normalized y co-ordinate
  BYTE | 5,                       // pixel size
  RAW | 6,                        // offsets
  SURFACE | 7,                    // destination surface
  GENERAL | 8,                    // destination x offset
  GENERAL | 9,                    // destination y offset
  END,

  Intrinsic::genx_va_lbp_creation,
  LITERAL | ISA_VA_SKL_PLUS,
  LITERAL | VA_OP_CODE_LBP_CREATION,
  SURFACE | 1,                      // surface index
  GENERAL | 2,                      // normalized x co-ordinate
  GENERAL | 3,                      // normalized y co-ordinate
  BYTE | 4,                         // mode
  RAW | 0,                          // Destination
  END,

  Intrinsic::genx_va_hdc_lbp_creation,
  LITERAL | ISA_VA_SKL_PLUS,
  LITERAL | ISA_HDC_LBPCREATION,
  SURFACE | 1,                      // surface index
  GENERAL | 2,                      // normalized x co-ordinate
  GENERAL | 3,                      // normalized y co-ordinate
  BYTE | 4,                         // mode
  SURFACE | 5,                      // destination surface
  GENERAL | 6,                      // destination x offset
  GENERAL | 7,                      // destination y offset
  END,

  Intrinsic::genx_va_lbp_correlation,
  LITERAL | ISA_VA_SKL_PLUS,
  LITERAL | VA_OP_CODE_LBP_CORRELATION,
  SURFACE | 1,                      // surface index
  GENERAL | 2,                      // normalized x co-ordinate
  GENERAL | 3,                      // normalized y co-ordinate
  GENERAL | 4,                      // horizontal disparity
  RAW | 0,                          // Destination
  END,

  Intrinsic::genx_va_hdc_lbp_correlation,
  LITERAL | ISA_VA_SKL_PLUS,
  LITERAL | ISA_HDC_LBPCORRELATION,
  SURFACE | 1,                      // surface index
  GENERAL | 2,                      // normalized x co-ordinate
  GENERAL | 3,                      // normalized y co-ordinate
  GENERAL | 4,                      // horizontal disparity
  SURFACE | 5,                      // destination surface
  GENERAL | 6,                      // destination x offset
  GENERAL | 7,                      // destination y offset
  END,

  Intrinsic::genx_va_correlation_search,
  LITERAL | ISA_VA_SKL_PLUS,
  LITERAL | VA_OP_CODE_CORRELATION_SEARCH,
  SURFACE | 1,                      // surface index
  GENERAL | 2,                      // normalized x co-ordinate
  GENERAL | 3,                      // normalized y co-ordinate
  GENERAL | 4,                      // normalized vertical origin
  GENERAL | 5,                      // normalized horizontal origin
  GENERAL | 6,                      // x-direction size
  GENERAL | 7,                      // y-direction size
  GENERAL | 8,                      // x-direction search size
  GENERAL | 9,                      // y-direction search size
  RAW | 0,                          // Destination
  END,

  Intrinsic::genx_va_hdc_convolve2d,
  LITERAL | ISA_VA_SKL_PLUS,
  LITERAL | ISA_HDC_CONV,
  SAMPLER | 1,                      // sampler index
  SURFACE | 2,                      // surface index
  GENERAL | 3,                      // normalized x co-ordinate
  GENERAL | 4,                      // normalized y co-ordinate
  BYTE | 5,                         // properties
  SURFACE | 6,                      // destination surface
  GENERAL | 7,                      // destination x offset
  GENERAL | 8,                      // destination x offset
  END,

  Intrinsic::genx_va_flood_fill,
  LITERAL | ISA_VA_SKL_PLUS,
  LITERAL | VA_OP_CODE_FLOOD_FILL,
  BYTE | 1,                         // Is8Connect
  RAW | 2,                          // pixel mask horizontal direction
  GENERAL | 3,                      // pixel mask vertical direction left
  GENERAL | 4,                      // pixel mask vertical direction right
  GENERAL | 5,                      // loop count
  RAW | 0,                          // Destination
  END,

  Intrinsic::genx_barrier,
  LITERAL | ISA_BARRIER, // opcode
  ISBARRIER,                   // suppress the nobarrier attribute
  END,

  Intrinsic::genx_sbarrier,
  LITERAL | ISA_SBARRIER, // opcode
  BYTE | 1, // signal flag
  END,

  Intrinsic::genx_cache_flush,
  LITERAL | ISA_SAMPLR_CACHE_FLUSH, // opcode
  END,

  Intrinsic::genx_fence,
  LITERAL | ISA_FENCE, // opcode
  BYTE | 1, // mask
  END,

  Intrinsic::genx_wait,
  LITERAL | ISA_WAIT, // opcode
  GENERAL | UNSIGNED | 1, // mask
  END,

  Intrinsic::genx_yield,
  LITERAL | ISA_YIELD, // opcode
  END,

  Intrinsic::genx_raw_send,
  LITERAL | ISA_RAW_SEND, // opcode
  BYTE | 1, // modifier sendc
  EXECSIZE_FROM_ARG | 2, // exec size inferred from predicate
  PREDICATION | 2, // predicate
  INT | 3, // extended message descriptor
  NUMGRFS | 5, // numsrc inferred from src size
  NUMGRFS | 0, // numdst inferred from dst size
  GENERAL | UNSIGNED | 4, // desc
  RAW | 5, // src
  TWOADDR | 6, // not in vISA instruction: old value of result
  RAW | 0, // dst
  END,

  Intrinsic::genx_raw_send_noresult,
  LITERAL | ISA_RAW_SEND, // opcode
  BYTE | 1, // modifier sendc
  EXECSIZE_FROM_ARG | 2, // exec size inferred from predicate
  PREDICATION | 2, // predicate
  INT | 3, // extended message descriptor
  NUMGRFS | 5, // numsrc inferred from src size
  LITERAL | 0, // numdst
  GENERAL | UNSIGNED | 4, // desc
  RAW | 5, // src
  NULLRAW, // dst
  END,

  Intrinsic::genx_raw_sends,
  LITERAL | ISA_RAW_SENDS, // opcode
  BYTE | 1, // modifier sendc
  EXECSIZE_FROM_ARG | 2, // exec size inferred from predicate
  PREDICATION | 2, // predicate
  NUMGRFS | 5, // numsrc inferred from src size
  NUMGRFS | 6, // numsrc2 inferred from src2 size
  NUMGRFS | 0, // numdst inferred from dst size
  BYTE | 3, // FFID
  GENERAL | UNSIGNED | 3, // extended message descriptor
  GENERAL | UNSIGNED | 4, // desc
  RAW | 5, // src
  RAW | 6, // src2
  TWOADDR | 7, // not in vISA instruction: old value of result
  RAW | 0, // dst
  END,

  Intrinsic::genx_raw_sends_noresult,
  LITERAL | ISA_RAW_SENDS, // opcode
  BYTE | 1, // modifier sendc
  EXECSIZE_FROM_ARG | 2, // exec size inferred from predicate
  PREDICATION | 2, // predicate
  NUMGRFS | 5, // numsrc inferred from src size
  NUMGRFS | 6, // numsrc2 inferred from src2 size
  LITERAL | 0, // numdst
  BYTE | 3, // FFID
  GENERAL | UNSIGNED | 3, // extended message descriptor
  GENERAL | UNSIGNED | 4, // desc
  RAW | 5, // src
  RAW | 6, // src2
  NULLRAW | 7, // dst
  END,


  //--------------------------------------------------------------------
  // GenX backend internal intrinsics

  Intrinsic::genx_constanti,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  Intrinsic::genx_constantf,
  LITERAL | ISA_MOV, // opcode
  EXECSIZE, // execution size
  IMPLICITPRED, // predication
  GENERAL | 0, // dst (return value)
  GENERAL | 1, // src0
  END,

  Intrinsic::genx_constantpred,
  LITERAL | ISA_SETP, // opcode
  EXECSIZE, // execution size
  PREDICATE | 0, // dst (return value)
  CONSTVI1ASI32 | 1, // src0
  END,

  Intrinsic::genx_add_addr,
  LITERAL | ISA_ADDR_ADD, // opcode
  EXECSIZE, // execution size
  ADDRESS | 0, // dst (return value)
  ADDRESS | 1, // src0
  GENERAL | UNSIGNED | 2, // src1
  END,

  Intrinsic::genx_predefined_surface,
  LITERAL | ISA_MOVS, // opcode
  EXECSIZE, // execution size
  SURFACE | 0, // dst (return value)
  INT | 1, // src0
  END,

  END // end of table
};

GenXIntrinsicInfo::GenXIntrinsicInfo(unsigned IntrinId)
    : Args(0)
{
  const auto *p = Table;
  for (;;) {
    if (*p == END)
      break; // intrinsic not found; leave Args pointing at END field
    if (IntrinId == *p++)
      break;
    // Scan past the rest of this entry.
    while (*p++ != END)
      ;
  }
  // We have found the right entry.
  Args = p;
}

// Get the category and modifier for an arg idx (-1 means return value).
// The returned ArgInfo struct contains just the short read from the table,
// and has methods for accessing the various fields.
GenXIntrinsicInfo::ArgInfo GenXIntrinsicInfo::getArgInfo(int Idx)
{
  // Read through the fields in the table to find the one with the right
  // arg index...
  for (const auto *p = Args; *p; p++) {
    ArgInfo AI(*p);
    if (AI.isRealArgOrRet() && AI.getArgIdx() == Idx)
      return AI;
  }
  // Field with requested arg index was not found.
  return END;
}

// Return the starting point of any trailing null (zero) arguments
// for this call. If the intrinsic does not have a ARGCOUNT descriptor
// this will always return the number of operands to the call (ie, there
// is no trailing null zone), even if there are some trailing nulls.
unsigned GenXIntrinsicInfo::getTrailingNullZoneStart(CallInst* CI) {
  unsigned TrailingNullStart = CI->getNumArgOperands();

  const auto *p = Args;
  for (; *p; p ++) {
    ArgInfo AI(*p);
    if (AI.getCategory() == ARGCOUNT)
      break;
  }

  if (*p) {
    ArgInfo ACI(*p);
    unsigned BaseArg = ACI.getArgIdx();

    TrailingNullStart = BaseArg;
    for (unsigned Idx = BaseArg; Idx < CI->getNumArgOperands(); ++ Idx) {
      if (auto CA = dyn_cast<Constant>(CI->getArgOperand(Idx))) {
        if (CA->isNullValue())
          continue;
      }
      TrailingNullStart = Idx + 1;
    }

    if (TrailingNullStart < BaseArg + ACI.getArgCountMin())
      TrailingNullStart = BaseArg + ACI.getArgCountMin();
  }

  return TrailingNullStart;
}

/***********************************************************************
 * getExecSizeAllowedBits : get bitmap of which execsize values are allowed
 *                          for this intrinsic
 *
 * Return:  bit N set if execution size 1<<N is allowed
 */
unsigned GenXIntrinsicInfo::getExecSizeAllowedBits()
{
  for (const auto *p = Args; *p; p++) {
    if (!(*p & GENERAL)) {
      switch (*p & CATMASK) {
        case EXECSIZE:
          return 0x3f;
        case EXECSIZE_GE2:
          return 0x3e;
        case EXECSIZE_GE4:
          return 0x3c;
        case EXECSIZE_GE8:
          return 0x38;
        case EXECSIZE_NOT2:
          return 0x3d;
      }
    }
  }
  return 0x3f;
}


/***********************************************************************
 * getPredAllowed : determine if this intrinsic is allowed to have
 *                  a predicated destination mask.
 *
 * Return:  true if it permitted, false otherwise.
 */
bool GenXIntrinsicInfo::getPredAllowed()
{
  // Simply search the intrinsic description for an IMPLICITPRED
  // entry. Not very efficient, but the situations where this
  // check is needed are expected to be infrequent.
  for (const auto *p = getInstDesc(); *p; ++p) {
    ArgInfo AI(*p);
    if (AI.getCategory() == IMPLICITPRED)
      return true;
  }

  return false;
}

unsigned GenXIntrinsicInfo::getOverridedExecSize(CallInst *CI,
                                                 const GenXSubtarget *ST) {
  unsigned ID = CI->getCalledFunction()->getIntrinsicID();
  switch (ID) {
  default:
    break;
  case Intrinsic::genx_raw_send:
  case Intrinsic::genx_raw_sends:
  case Intrinsic::genx_raw_send_noresult:
  case Intrinsic::genx_raw_sends_noresult:
    return 16;
  }

  return 0;
}
