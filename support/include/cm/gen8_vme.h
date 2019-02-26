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
static_assert(0, "CM:w:gen8_vme.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_gen8_VME_H_
#define _CLANG_gen8_VME_H_

#include "cm_send.h"
#include "cm_util.h"

#define __SFID_VME 8
#define __SFID_CRE 13

#define S13_2TOS16(value)                                                      \
  (((value >> 2) & 0x1fff) | (((value >> 15) & 0x1) << 15))
#define S16TOS13_2(value)                                                      \
  (((value & 0x1fff) << 2) | (((value >> 15) & 0x1) << 15))

// Select one element from the input payload
#ifdef VME_Input_S1
#undef VME_Input_S1
#endif
#define VME_Input_S1(p, t, r, c) p.row(r).format<t>().select<1, 1>(c)

#ifdef VME_Input_G1
#undef VME_Input_G1
#endif
#define VME_Input_G1(p, t, r, c) p.row(r).format<t>().select<1, 1>(c)(0)

// Select one element from the output result
#ifdef VME_Output_S1
#undef VME_Output_S1
#endif
#define VME_Output_S1(p, t, r, c) p.row(r).format<t>().select<1, 1>(c)(0)

// Input

// SET methods

// Format = S15
#define VME_SET_UNIInput_Ref0X(p, v) (VME_Input_S1(p, ushort, 0, 0) = v)
#define VME_SET_UNIInput_Ref0Y(p, v) (VME_Input_S1(p, ushort, 0, 1) = v)

// Format = S15
#define VME_SET_UNIInput_Ref1X(p, v) (VME_Input_S1(p, ushort, 0, 2) = v)
#define VME_SET_UNIInput_Ref1Y(p, v) (VME_Input_S1(p, ushort, 0, 3) = v)

// Format = U16
#define VME_SET_UNIInput_SrcX(p, v) (VME_Input_S1(p, ushort, 0, 4) = v)

// Format = U16
#define VME_SET_UNIInput_SrcY(p, v) (VME_Input_S1(p, ushort, 0, 5) = v)

// Set - FieldBased
// Clear - FrameBased
#define VME_SET_UNIInput_SrcAccess(p) (VME_Input_S1(p, uchar, 0, 12) |= 0x40)
#define VME_CLEAR_UNIInput_SrcAccess(p) (VME_Input_S1(p, uchar, 0, 12) &= 0xBF)

// Set - FieldBased
// Clear - FrameBased
#define VME_SET_UNIInput_RefAccess(p) (VME_Input_S1(p, uchar, 0, 12) |= 0x80)
#define VME_CLEAR_UNIInput_RefAccess(p) (VME_Input_S1(p, uchar, 0, 12) &= 0x7F)

// Set - different paths
// Clear - same path
#define VME_SET_UNIInput_DualSearchPath(p)                                     \
  (VME_Input_S1(p, uchar, 0, 13) |= 0x08)
#define VME_CLEAR_UNIInput_DualSearchPath(p)                                   \
  (VME_Input_S1(p, uchar, 0, 13) &= 0xF7)

// #define INT_MODE 0x0
// #define HALF_PEL_MODE 0x1
// #define QUARTER_PEL_MODE 0x3
#define VME_SET_UNIInput_SubPelMode(p, v)                                      \
  (VME_Input_S1(p, uchar, 0, 13) =                                             \
       VME_Input_S1(p, uchar, 0, 13) & 0xCF | (v << 4))

// Set - 4MVP
// Clear - 1MVP
#define VME_SET_UNIInput_SkipModeType(p) (VME_Input_S1(p, uchar, 0, 13) |= 0x40)
#define VME_CLEAR_UNIInput_SkipModeType(p)                                     \
  (VME_Input_S1(p, uchar, 0, 13) &= 0xBF)

#define VME_SET_UNIInput_DisableFieldCacheAllocation(p)                        \
  (VME_Input_S1(p, uchar, 0, 13) |= 0x80)
#define VME_CLEAR_UNIInput_DisableFieldCacheAllocation(p)                      \
  (VME_Input_S1(p, uchar, 0, 13) &= 0x7F)

// Set - Chroma mode
// Clear - Luma mode
#define VME_SET_UNIInput_InterChromaMode(p)                                    \
  (VME_Input_S1(p, uchar, 0, 14) |= 0x1)
#define VME_CLEAR_UNIInput_InterChromaMode(p)                                  \
  (VME_Input_S1(p, uchar, 0, 14) &= 0xFE)

#define VME_SET_UNIInput_FTEnable(p) (VME_Input_S1(p, uchar, 0, 14) |= 0x2)
#define VME_CLEAR_UNIInput_FTEnable(p) (VME_Input_S1(p, uchar, 0, 14) &= 0xFD)

#define VME_SET_UNIInput_BMEDisableFBR(p) (VME_Input_S1(p, uchar, 0, 14) |= 0x4)
#define VME_CLEAR_UNIInput_BMEDisableFBR(p)                                    \
  (VME_Input_S1(p, uchar, 0, 14) &= 0xFB)

// v is a 7-bit mask with the following definition:
// xxxxxx1 : 16x16 sub-macroblock disabled
// xxxxx1x : 2x(16x8) sub-macroblock within 16x16 disabled
// xxxx1xx : 2x(8x16) sub-macroblock within 16x16 disabled
// xxx1xxx : 1x(8x8) sub-partition for 4x(8x8) within 16x16 disabled
// xx1xxxx : 2x(8x4) sub-partition for 4x(8x8) within 16x16 disabled
// x1xxxxx : 2x(4x8) sub-partition for 4x(8x8) within 16x16 disabled
// 1xxxxxx : 4x(4x4) sub-partition for 4x(8x8) within 16x16 disabled
#define VME_SET_UNIInput_SubMbPartMask(p, v) (VME_Input_S1(p, uchar, 0, 15) = v)

// The value must be a multiple of 4. Range = [20, 64]
#define VME_SET_UNIInput_RefW(p, v) (VME_Input_S1(p, uchar, 0, 22) = v)

// The value must be a multiple of 4. Range = [20, 64]
#define VME_SET_UNIInput_RefH(p, v) (VME_Input_S1(p, uchar, 0, 23) = v)

#define VME_SET_UNIInput_SkipModeEn(p) (VME_Input_S1(p, uchar, 1, 0) |= 0x1)
#define VME_CLEAR_UNIInput_SkipModeEn(p) (VME_Input_S1(p, uchar, 1, 0) &= 0xFE)

#define VME_SET_UNIInput_AdaptiveEn(p) (VME_Input_S1(p, uchar, 1, 0) |= 0x2)
#define VME_CLEAR_UNIInput_AdaptiveEn(p) (VME_Input_S1(p, uchar, 1, 0) &= 0xFD)

#define VME_SET_UNIInput_EarlyImeSuccessEn(p)                                  \
  (VME_Input_S1(p, uchar, 1, 0) |= 0x20)
#define VME_CLEAR_UNIInput_EarlyImeSuccessEn(p)                                \
  (VME_Input_S1(p, uchar, 1, 0) &= 0xDF)

#define VME_SET_UNIInput_T8x8FlagForInterEn(p)                                 \
  (VME_Input_S1(p, uchar, 1, 0) |= 0x80)
#define VME_CLEAR_UNIInput_T8x8FlagForInterEn(p)                               \
  (VME_Input_S1(p, uchar, 1, 0) &= 0x7F)

#define VME_SET_UNIInput_EarlyImeStop(p, v) (VME_Input_S1(p, uchar, 1, 3) = v)

#define VME_SET_UNIInput_MaxNumMVs(p, v) (VME_Input_S1(p, uchar, 1, 4) = v)

#define VME_SET_UNIInput_Ref0Polarity(p, v)                                    \
  (VME_Input_S1(p, uchar, 1, 5) = VME_Input_S1(p, uchar, 1, 5) & 0xF0 | v)
#define VME_SET_UNIInput_Ref1Polarity(p, v)                                    \
  (VME_Input_S1(p, uchar, 1, 5) =                                              \
       VME_Input_S1(p, uchar, 1, 5) & 0x0F | (v << 4))

// Format = U6, Valid Values: [16, 21, 32, 43, 48]
#define VME_SET_UNIInput_BiWeight(p, v) (VME_Input_S1(p, uchar, 1, 6) = v)

#define VME_SET_UNIInput_RefPixelBiasEnable(p)                                 \
  (VME_Input_S1(p, uchar, 1, 7) |= 0x20)
#define VME_CLEAR_UNIInput_RefPixelBiasEnable(p)                               \
  (VME_Input_S1(p, uchar, 1, 7) &= 0xDF)

#define VME_SET_UNIInput_UniMixDisable(p) (VME_Input_S1(p, uchar, 1, 7) |= 0x10)
#define VME_CLEAR_UNIInput_UniMixDisable(p)                                    \
  (VME_Input_S1(p, uchar, 1, 7) &= 0xEF)

// Format = U8, Valid range [1,63]
#define VME_SET_UNIInput_LenSP(p, v) (VME_Input_S1(p, uchar, 1, 8) = v)

// Format = U8, Valid range [1,63]
#define VME_SET_UNIInput_MaxNumSU(p, v) (VME_Input_S1(p, uchar, 1, 9) = v)

// Format = U4
#define VME_SET_UNIInput_StartCenter0(p, StartCenter0X, StartCenter0Y)         \
  (VME_Input_S1(p, uchar, 1, 10) = StartCenter0X | (StartCenter0Y << 4))

// Format = U4
#define VME_SET_UNIInput_StartCenter1(p, StartCenter1X, StartCenter1Y)         \
  (VME_Input_S1(p, uchar, 1, 11) = StartCenter1X | (StartCenter1Y << 4))

// Format = U8
#define VME_SET_UNIInput_WeightedSADCtrl0_3(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 12) = v)
#define VME_SET_UNIInput_WeightedSADCtrl4_7(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 13) = v)
#define VME_SET_UNIInput_WeightedSADCtrl8_11(p, v)                             \
  (VME_Input_S1(p, uchar, 1, 14) = v)
#define VME_SET_UNIInput_WeightedSADCtrl12_15(p, v)                            \
  (VME_Input_S1(p, uchar, 1, 15) = v)

// Format = U4
#define VME_SET_UNIInput_Blk0RefID(p, FwdBlk0RefID, BwdBlk0RefID)              \
  (VME_Input_S1(p, uchar, 1, 24) = FwdBlk0RefID | (BwdBlk0RefID << 4))

// Format = U4
#define VME_SET_UNIInput_Blk1RefID(p, FwdBlk1RefID, BwdBlk1RefID)              \
  (VME_Input_S1(p, uchar, 1, 25) = FwdBlk1RefID | (BwdBlk1RefID << 4))

// Format = U4
#define VME_SET_UNIInput_Blk2RefID(p, FwdBlk2RefID, BwdBlk2RefID)              \
  (VME_Input_S1(p, uchar, 1, 26) = FwdBlk2RefID | (BwdBlk2RefID << 4))

// Format = U4
#define VME_SET_UNIInput_Blk3RefID(p, FwdBlk3RefID, BwdBlk3RefID)              \
  (VME_Input_S1(p, uchar, 1, 27) = FwdBlk3RefID | (BwdBlk3RefID << 4))

#define VME_SET_UNIInput_IntraFlags(p, v) (VME_Input_S1(p, uchar, 1, 28) = v)

// v is a 8-bit mask with the following definition:
// Bit-7    Reserved : MBZ (for IntraPredAvailFlagF - F (pixel[-1,7]
// available
// for MbAff)
// Bit-6    Reserved : MBZ (for IntraPredAvailFlagA/E - A (left neighbor top
// half for MbAff)
// Bit-5    IntraPredAvailFlagE/A - A (Left neighbor or Left bottom half)
// Bit-4    IntraPredAvailFlagB - B (Upper neighbor)
// Bit-3    IntraPredAvailFlagC - C (Upper left neighbor)
// Bit-2    IntraPredAvailFlagD - D (Upper right neighbor)
// Bit-1:0  Reserved: MBZ (ChromaIntraPredMode)
#define VME_SET_UNIInput_MbIntraStruct(p, v) (VME_Input_S1(p, uchar, 1, 29) = v)

// v is a 2-bit value:
// 00: qpel [Qpel difference between MV and cost center: eff cost range 0-15pel]
// 01: hpel [Hpel difference between MV and cost center: eff cost range 0-31pel]
// 10: pel  [Pel  difference between MV and cost center: eff cost range 0-63pel]
// 11: 2pel [2Pel difference between MV and cost center: eff cost range
// 0-127pel]
#define VME_SET_UNIInput_MVCostScaleFactor(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 30) = VME_Input_S1(p, uchar, 1, 30) & 0xFC | v)

#define VME_SET_UNIInput_SrcFieldPolarity(p)                                   \
  (VME_Input_S1(p, uchar, 1, 30) |= 0x08)
#define VME_CLEAR_UNIInput_SrcFieldPolarity(p)                                 \
  (VME_Input_S1(p, uchar, 1, 30) &= 0xF7)

#define VME_SET_UNIInput_BilinearEnable(p)                                     \
  (VME_Input_S1(p, uchar, 1, 30) |= 0x04)
#define VME_CLEAR_UNIInput_BilinearEnable(p)                                   \
  (VME_Input_S1(p, uchar, 1, 30) &= 0xFB)

#define VME_SET_UNIInput_WeightedSADHAAR(p)                                    \
  (VME_Input_S1(p, uchar, 1, 30) |= 0x10)
#define VME_CLEAR_UNIInput_WeightedSADHAAR(p)                                  \
  (VME_Input_S1(p, uchar, 1, 30) &= 0xEF)

#define VME_SET_UNIInput_AConlyHAAR(p) (VME_Input_S1(p, uchar, 1, 30) |= 0x20)
#define VME_CLEAR_UNIInput_AConlyHAAR(p) (VME_Input_S1(p, uchar, 1, 30) &= 0xDF)

#define VME_SET_UNIInput_RefIDCostMode(p)                                      \
  (VME_Input_S1(p, uchar, 1, 30) |= 0x40)
#define VME_CLEAR_UNIInput_RefIDCostMode(p)                                    \
  (VME_Input_S1(p, uchar, 1, 30) &= 0xBF)

#define VME_SET_UNIInput_IDMShapeMode(p) (VME_Input_S1(p, uchar, 1, 30) |= 0x80)
#define VME_CLEAR_UNIInput_IDMShapeMode(p)                                     \
  (VME_Input_S1(p, uchar, 1, 30) &= 0x7F)

// v is a 8-bit mask with the following definition:
// xxxx xxx1: Ref0 Skip Center 0 is enabled [corresponds to M2.0]
// xxxx xx1x: Ref1 Skip Center 0 is enabled [corresponds to M2.1]
// xxxx x1xx: Ref0 Skip Center 1 is enabled [corresponds to M2.2]
// xxxx 1xxx: Ref1 Skip Center 1 is enabled [corresponds to M2.3]
// xxx1 xxxx: Ref0 Skip Center 2 is enabled [corresponds to M2.4]
// xx1x xxxx: Ref1 Skip Center 2 is enabled [corresponds to M2.5]
// x1xx xxxx: Ref0 Skip Center 3 is enabled [corresponds to M2.6]
// 1xxx xxxx: Ref1 Skip Center 3 is enabled [corresponds to M2.7]
#define VME_SET_UNIInput_SkipCenterMask(p, v)                                  \
  (VME_Input_S1(p, uchar, 1, 31) = v)

// NOTE: replace v with U4U4 type with
// (shift_count, shift_val) and set the value as (shift_val | (shift_count <<
// 4))

// Format = U4U4 (encoded value must fit in 12-bits)
#define VME_SET_UNIInput_Mode0Cost(p, v) (VME_Input_S1(p, uchar, 2, 0) = v)
#define VME_SET_UNIInput_Mode1Cost(p, v) (VME_Input_S1(p, uchar, 2, 1) = v)
#define VME_SET_UNIInput_Mode2Cost(p, v) (VME_Input_S1(p, uchar, 2, 2) = v)
#define VME_SET_UNIInput_Mode3Cost(p, v) (VME_Input_S1(p, uchar, 2, 3) = v)
#define VME_SET_UNIInput_Mode4Cost(p, v) (VME_Input_S1(p, uchar, 2, 4) = v)
#define VME_SET_UNIInput_Mode5Cost(p, v) (VME_Input_S1(p, uchar, 2, 5) = v)
#define VME_SET_UNIInput_Mode6Cost(p, v) (VME_Input_S1(p, uchar, 2, 6) = v)
#define VME_SET_UNIInput_Mode7Cost(p, v) (VME_Input_S1(p, uchar, 2, 7) = v)
#define VME_SET_UNIInput_Mode8Cost(p, v) (VME_Input_S1(p, uchar, 2, 8) = v)
#define VME_SET_UNIInput_Mode9Cost(p, v) (VME_Input_S1(p, uchar, 2, 9) = v)

// Format = U4U4 (encoded value must fit in 12-bits)
#define VME_SET_UNIInput_RefIDCost(p, v) (VME_Input_S1(p, uchar, 2, 10) = v)

// Format = U4U4 (encoded value must fit in 12-bits)
#define VME_SET_UNIInput_ChromaIntraModeCost(p, v)                             \
  (VME_Input_S1(p, uchar, 2, 11) = v)

// Format = U4U4 (encoded value must fit in 10-bits)
#define VME_SET_UNIInput_MV0Cost(p, v) (VME_Input_S1(p, uchar, 2, 12) = v)
#define VME_SET_UNIInput_MV1Cost(p, v) (VME_Input_S1(p, uchar, 2, 13) = v)
#define VME_SET_UNIInput_MV2Cost(p, v) (VME_Input_S1(p, uchar, 2, 14) = v)
#define VME_SET_UNIInput_MV3Cost(p, v) (VME_Input_S1(p, uchar, 2, 15) = v)
#define VME_SET_UNIInput_MV4Cost(p, v) (VME_Input_S1(p, uchar, 2, 16) = v)
#define VME_SET_UNIInput_MV5Cost(p, v) (VME_Input_S1(p, uchar, 2, 17) = v)
#define VME_SET_UNIInput_MV6Cost(p, v) (VME_Input_S1(p, uchar, 2, 18) = v)
#define VME_SET_UNIInput_MV7Cost(p, v) (VME_Input_S1(p, uchar, 2, 19) = v)

// Format = U8
#define VME_SET_UNIInput_FBRMbModeInput(p, v)                                  \
  (VME_Input_S1(p, uchar, 2, 20) = v)

// Format = U8
#define VME_SET_UNIInput_FBRSubMBShapeInput(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 21) = v)

// Format = U8
#define VME_SET_UNIInput_FBRSubPredModeInput(p, v)                             \
  (VME_Input_S1(p, uchar, 2, 22) = v)

// Format = U16
#define VME_SET_UNIInput_SICFwdTCoeffThreshold0(p, v)                          \
  (VME_Input_S1(p, ushort, 2, 12) = v)
// Format = U8
#define VME_SET_UNIInput_SICFwdTCoeffThreshold1(p, v)                          \
  (VME_Input_S1(p, uchar, 2, 26) = v)
#define VME_SET_UNIInput_SICFwdTCoeffThreshold2(p, v)                          \
  (VME_Input_S1(p, uchar, 2, 27) = v)
#define VME_SET_UNIInput_SICFwdTCoeffThreshold3(p, v)                          \
  (VME_Input_S1(p, uchar, 2, 28) = v)
#define VME_SET_UNIInput_SICFwdTCoeffThreshold4(p, v)                          \
  (VME_Input_S1(p, uchar, 2, 29) = v)
#define VME_SET_UNIInput_SICFwdTCoeffThreshold5(p, v)                          \
  (VME_Input_S1(p, uchar, 2, 30) = v)
#define VME_SET_UNIInput_SICFwdTCoeffThreshold6(p, v)                          \
  (VME_Input_S1(p, uchar, 2, 31) = v)

// Format = S13.2, DletaX Valid Range: [-2048.00 to 2047.75], DletaY Valid
// Range: [-512.00 to 511.75]
#define VME_SET_UNIInput_FWDCostCenter0(p, DeltaX, DeltaY)                     \
  (VME_Input_S1(p, uint, 3, 0) = DeltaX | (DeltaY << 16))
#define VME_SET_UNIInput_BWDCostCenter0(p, DeltaX, DeltaY)                     \
  (VME_Input_S1(p, uint, 3, 1) = DeltaX | (DeltaY << 16))
#define VME_SET_UNIInput_FWDCostCenter1(p, DeltaX, DeltaY)                     \
  (VME_Input_S1(p, uint, 3, 2) = DeltaX | (DeltaY << 16))
#define VME_SET_UNIInput_BWDCostCenter1(p, DeltaX, DeltaY)                     \
  (VME_Input_S1(p, uint, 3, 3) = DeltaX | (DeltaY << 16))
#define VME_SET_UNIInput_FWDCostCenter2(p, DeltaX, DeltaY)                     \
  (VME_Input_S1(p, uint, 3, 4) = DeltaX | (DeltaY << 16))
#define VME_SET_UNIInput_BWDCostCenter2(p, DeltaX, DeltaY)                     \
  (VME_Input_S1(p, uint, 3, 5) = DeltaX | (DeltaY << 16))
#define VME_SET_UNIInput_FWDCostCenter3(p, DeltaX, DeltaY)                     \
  (VME_Input_S1(p, uint, 3, 6) = DeltaX | (DeltaY << 16))
#define VME_SET_UNIInput_BWDCostCenter3(p, DeltaX, DeltaY)                     \
  (VME_Input_S1(p, uint, 3, 7) = DeltaX | (DeltaY << 16))

// Format = U8, with the following definition:
// [7:4] (Y) - specifies relative Y distance to the next SU from previous SU in
// units of SU
// [3:0] (X) - specifies relative X distance to the next SU from previous SU in
// units of SU
// Format = U8, with the following definition:
// [7:4] (Y) - specifies relative Y distance to the next SU from previous SU in
// units of SU
// [3:0] (X) - specifies relative X distance to the next SU from previous SU in
// units of SU
#define VME_SET_IMEInput_IMESearchPathDelta0(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 0) = v)
#define VME_SET_IMEInput_IMESearchPathDelta1(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 1) = v)
#define VME_SET_IMEInput_IMESearchPathDelta2(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 2) = v)
#define VME_SET_IMEInput_IMESearchPathDelta3(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 3) = v)
#define VME_SET_IMEInput_IMESearchPathDelta4(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 4) = v)
#define VME_SET_IMEInput_IMESearchPathDelta5(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 5) = v)
#define VME_SET_IMEInput_IMESearchPathDelta6(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 6) = v)
#define VME_SET_IMEInput_IMESearchPathDelta7(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 7) = v)
#define VME_SET_IMEInput_IMESearchPathDelta8(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 8) = v)
#define VME_SET_IMEInput_IMESearchPathDelta9(p, v)                             \
  (VME_Input_G1(p, uchar, 0, 9) = v)
#define VME_SET_IMEInput_IMESearchPathDelta10(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 10) = v)
#define VME_SET_IMEInput_IMESearchPathDelta11(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 11) = v)
#define VME_SET_IMEInput_IMESearchPathDelta12(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 12) = v)
#define VME_SET_IMEInput_IMESearchPathDelta13(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 13) = v)
#define VME_SET_IMEInput_IMESearchPathDelta14(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 14) = v)
#define VME_SET_IMEInput_IMESearchPathDelta15(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 15) = v)
#define VME_SET_IMEInput_IMESearchPathDelta16(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 16) = v)
#define VME_SET_IMEInput_IMESearchPathDelta17(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 17) = v)
#define VME_SET_IMEInput_IMESearchPathDelta18(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 18) = v)
#define VME_SET_IMEInput_IMESearchPathDelta19(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 19) = v)
#define VME_SET_IMEInput_IMESearchPathDelta20(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 20) = v)
#define VME_SET_IMEInput_IMESearchPathDelta21(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 21) = v)
#define VME_SET_IMEInput_IMESearchPathDelta22(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 22) = v)
#define VME_SET_IMEInput_IMESearchPathDelta23(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 23) = v)
#define VME_SET_IMEInput_IMESearchPathDelta24(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 24) = v)
#define VME_SET_IMEInput_IMESearchPathDelta25(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 25) = v)
#define VME_SET_IMEInput_IMESearchPathDelta26(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 26) = v)
#define VME_SET_IMEInput_IMESearchPathDelta27(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 27) = v)
#define VME_SET_IMEInput_IMESearchPathDelta28(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 28) = v)
#define VME_SET_IMEInput_IMESearchPathDelta29(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 29) = v)
#define VME_SET_IMEInput_IMESearchPathDelta30(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 30) = v)
#define VME_SET_IMEInput_IMESearchPathDelta31(p, v)                            \
  (VME_Input_G1(p, uchar, 0, 31) = v)

#define VME_SET_IMEInput_IMESearchPathDelta32(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 0) = v)
#define VME_SET_IMEInput_IMESearchPathDelta33(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 1) = v)
#define VME_SET_IMEInput_IMESearchPathDelta34(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 2) = v)
#define VME_SET_IMEInput_IMESearchPathDelta35(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 3) = v)
#define VME_SET_IMEInput_IMESearchPathDelta36(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 4) = v)
#define VME_SET_IMEInput_IMESearchPathDelta37(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 5) = v)
#define VME_SET_IMEInput_IMESearchPathDelta38(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 6) = v)
#define VME_SET_IMEInput_IMESearchPathDelta39(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 7) = v)
#define VME_SET_IMEInput_IMESearchPathDelta40(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 8) = v)
#define VME_SET_IMEInput_IMESearchPathDelta41(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 9) = v)
#define VME_SET_IMEInput_IMESearchPathDelta42(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 10) = v)
#define VME_SET_IMEInput_IMESearchPathDelta43(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 11) = v)
#define VME_SET_IMEInput_IMESearchPathDelta44(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 12) = v)
#define VME_SET_IMEInput_IMESearchPathDelta45(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 13) = v)
#define VME_SET_IMEInput_IMESearchPathDelta46(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 14) = v)
#define VME_SET_IMEInput_IMESearchPathDelta47(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 15) = v)
#define VME_SET_IMEInput_IMESearchPathDelta48(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 16) = v)
#define VME_SET_IMEInput_IMESearchPathDelta49(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 17) = v)
#define VME_SET_IMEInput_IMESearchPathDelta50(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 18) = v)
#define VME_SET_IMEInput_IMESearchPathDelta51(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 19) = v)
#define VME_SET_IMEInput_IMESearchPathDelta52(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 20) = v)
#define VME_SET_IMEInput_IMESearchPathDelta53(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 21) = v)
#define VME_SET_IMEInput_IMESearchPathDelta54(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 22) = v)
#define VME_SET_IMEInput_IMESearchPathDelta55(p, v)                            \
  (VME_Input_G1(p, uchar, 1, 23) = v)

// Format = U16
#define VME_SET_IMEInput_Rec0_16x8_0Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 2, 0) = v)
#define VME_SET_IMEInput_Rec0_16x8_1Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 2, 1) = v)
#define VME_SET_IMEInput_Rec0_8x16_0Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 2, 2) = v)
#define VME_SET_IMEInput_Rec0_8x16_1Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 2, 3) = v)
#define VME_SET_IMEInput_Rec0_8x8_0Distortion(p, v)                            \
  (VME_Input_S1(p, ushort, 2, 4) = v)
#define VME_SET_IMEInput_Rec0_8x8_1Distortion(p, v)                            \
  (VME_Input_S1(p, ushort, 2, 5) = v)
#define VME_SET_IMEInput_Rec0_8x8_2Distortion(p, v)                            \
  (VME_Input_S1(p, ushort, 2, 6) = v)
#define VME_SET_IMEInput_Rec0_8x8_3Distortion(p, v)                            \
  (VME_Input_S1(p, ushort, 2, 7) = v)
#define VME_SET_IMEInput_Rec0_16x16_Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 2, 8) = v)

// Format = U4
#define VME_SET_IMEInput_Rec0_16x16_RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 2, 18) = v)

// Format = S13.2
#define VME_SET_IMEInput_Rec0_16x16_X(p, v) (VME_Input_S1(p, ushort, 2, 10) = v)
#define VME_SET_IMEInput_Rec0_16x16_Y(p, v) (VME_Input_S1(p, ushort, 2, 11) = v)

// Format = U4
#define VME_SET_IMEInput_Rec0_16x8_0RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 2, 24) = VME_Input_S1(p, uchar, 2, 24) & 0xF0 | v)
#define VME_SET_IMEInput_Rec0_16x8_1RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 2, 24) =                                             \
       VME_Input_S1(p, uchar, 2, 24) & 0x0F | (v << 4))
#define VME_SET_IMEInput_Rec0_8x16_0RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 2, 25) = VME_Input_S1(p, uchar, 2, 25) & 0xF0 | v)
#define VME_SET_IMEInput_Rec0_8x16_1RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 2, 25) =                                             \
       VME_Input_S1(p, uchar, 2, 25) & 0x0F | (v << 4))
#define VME_SET_IMEInput_Rec0_8x8_0RefID(p, v)                                 \
  (VME_Input_S1(p, uchar, 2, 26) = VME_Input_S1(p, uchar, 2, 26) & 0xF0 | v)
#define VME_SET_IMEInput_Rec0_8x8_1RefID(p, v)                                 \
  (VME_Input_S1(p, uchar, 2, 26) =                                             \
       VME_Input_S1(p, uchar, 2, 26) & 0x0F | (v << 4))
#define VME_SET_IMEInput_Rec0_8x8_2RefID(p, v)                                 \
  (VME_Input_S1(p, uchar, 2, 27) = VME_Input_S1(p, uchar, 2, 27) & 0xF0 | v)
#define VME_SET_IMEInput_Rec0_8x8_3RefID(p, v)                                 \
  (VME_Input_S1(p, uchar, 2, 27) =                                             \
       VME_Input_S1(p, uchar, 2, 27) & 0x0F | (v << 4))

// Format = S13.2
#define VME_SET_IMEInput_Rec0_16x8_0X(p, v) (VME_Input_S1(p, ushort, 3, 0) = v)
#define VME_SET_IMEInput_Rec0_16x8_0Y(p, v) (VME_Input_S1(p, ushort, 3, 1) = v)
#define VME_SET_IMEInput_Rec0_16x8_1X(p, v) (VME_Input_S1(p, ushort, 3, 2) = v)
#define VME_SET_IMEInput_Rec0_16x8_1Y(p, v) (VME_Input_S1(p, ushort, 3, 3) = v)
#define VME_SET_IMEInput_Rec0_8x16_0X(p, v) (VME_Input_S1(p, ushort, 3, 4) = v)
#define VME_SET_IMEInput_Rec0_8x16_0Y(p, v) (VME_Input_S1(p, ushort, 3, 5) = v)
#define VME_SET_IMEInput_Rec0_8x16_1X(p, v) (VME_Input_S1(p, ushort, 3, 6) = v)
#define VME_SET_IMEInput_Rec0_8x16_1Y(p, v) (VME_Input_S1(p, ushort, 3, 7) = v)
#define VME_SET_IMEInput_Rec0_8x8_0X(p, v) (VME_Input_S1(p, ushort, 3, 8) = v)
#define VME_SET_IMEInput_Rec0_8x8_0Y(p, v) (VME_Input_S1(p, ushort, 3, 9) = v)
#define VME_SET_IMEInput_Rec0_8x8_1X(p, v) (VME_Input_S1(p, ushort, 3, 10) = v)
#define VME_SET_IMEInput_Rec0_8x8_1Y(p, v) (VME_Input_S1(p, ushort, 3, 11) = v)
#define VME_SET_IMEInput_Rec0_8x8_2X(p, v) (VME_Input_S1(p, ushort, 3, 12) = v)
#define VME_SET_IMEInput_Rec0_8x8_2Y(p, v) (VME_Input_S1(p, ushort, 3, 13) = v)
#define VME_SET_IMEInput_Rec0_8x8_3X(p, v) (VME_Input_S1(p, ushort, 3, 14) = v)
#define VME_SET_IMEInput_Rec0_8x8_3Y(p, v) (VME_Input_S1(p, ushort, 3, 15) = v)

// Format = U16
#define VME_SET_IMEInput_Rec1_16x8_0Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 4, 0) = v)
#define VME_SET_IMEInput_Rec1_16x8_1Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 4, 1) = v)
#define VME_SET_IMEInput_Rec1_8x16_0Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 4, 2) = v)
#define VME_SET_IMEInput_Rec1_8x16_1Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 4, 3) = v)
#define VME_SET_IMEInput_Rec1_8x8_0Distortion(p, v)                            \
  (VME_Input_S1(p, ushort, 4, 4) = v)
#define VME_SET_IMEInput_Rec1_8x8_1Distortion(p, v)                            \
  (VME_Input_S1(p, ushort, 4, 5) = v)
#define VME_SET_IMEInput_Rec1_8x8_2Distortion(p, v)                            \
  (VME_Input_S1(p, ushort, 4, 6) = v)
#define VME_SET_IMEInput_Rec1_8x8_3Distortion(p, v)                            \
  (VME_Input_S1(p, ushort, 4, 7) = v)
#define VME_SET_IMEInput_Rec1_16x16_Distortion(p, v)                           \
  (VME_Input_S1(p, ushort, 4, 8) = v)

// Format = U4
#define VME_SET_IMEInput_Rec1_16x16_RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 4, 18) = v)

// Format = S13.2
#define VME_SET_IMEInput_Rec1_16x16_X(p, v) (VME_Input_S1(p, ushort, 4, 10) = v)
#define VME_SET_IMEInput_Rec1_16x16_Y(p, v) (VME_Input_S1(p, ushort, 4, 11) = v)

// Format = U4
#define VME_SET_IMEInput_Rec1_16x8_0RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 4, 24) = VME_Input_S1(p, uchar, 4, 24) & 0xF0 | v)
#define VME_SET_IMEInput_Rec1_16x8_1RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 4, 24) =                                             \
       VME_Input_S1(p, uchar, 4, 24) & 0x0F | (v << 4))
#define VME_SET_IMEInput_Rec1_8x16_0RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 4, 25) = VME_Input_S1(p, uchar, 4, 25) & 0xF0 | v)
#define VME_SET_IMEInput_Rec1_8x16_1RefID(p, v)                                \
  (VME_Input_S1(p, uchar, 4, 25) =                                             \
       VME_Input_S1(p, uchar, 4, 25) & 0x0F | (v << 4))
#define VME_SET_IMEInput_Rec1_8x8_0RefID(p, v)                                 \
  (VME_Input_S1(p, uchar, 4, 26) = VME_Input_S1(p, uchar, 4, 26) & 0xF0 | v)
#define VME_SET_IMEInput_Rec1_8x8_1RefID(p, v)                                 \
  (VME_Input_S1(p, uchar, 4, 26) =                                             \
       VME_Input_S1(p, uchar, 4, 26) & 0x0F | (v << 4))
#define VME_SET_IMEInput_Rec1_8x8_2RefID(p, v)                                 \
  (VME_Input_S1(p, uchar, 4, 27) = VME_Input_S1(p, uchar, 4, 27) & 0xF0 | v)
#define VME_SET_IMEInput_Rec1_8x8_3RefID(p, v)                                 \
  (VME_Input_S1(p, uchar, 4, 27) =                                             \
       VME_Input_S1(p, uchar, 4, 27) & 0x0F | (v << 4))

// Format = S13.2
#define VME_SET_IMEInput_Rec1_16x8_0X(p, v) (VME_Input_S1(p, ushort, 5, 0) = v)
#define VME_SET_IMEInput_Rec1_16x8_0Y(p, v) (VME_Input_S1(p, ushort, 5, 1) = v)
#define VME_SET_IMEInput_Rec1_16x8_1X(p, v) (VME_Input_S1(p, ushort, 5, 2) = v)
#define VME_SET_IMEInput_Rec1_16x8_1Y(p, v) (VME_Input_S1(p, ushort, 5, 3) = v)
#define VME_SET_IMEInput_Rec1_8x16_0X(p, v) (VME_Input_S1(p, ushort, 5, 4) = v)
#define VME_SET_IMEInput_Rec1_8x16_0Y(p, v) (VME_Input_S1(p, ushort, 5, 5) = v)
#define VME_SET_IMEInput_Rec1_8x16_1X(p, v) (VME_Input_S1(p, ushort, 5, 6) = v)
#define VME_SET_IMEInput_Rec1_8x16_1Y(p, v) (VME_Input_S1(p, ushort, 5, 7) = v)
#define VME_SET_IMEInput_Rec1_8x8_0X(p, v) (VME_Input_S1(p, ushort, 5, 8) = v)
#define VME_SET_IMEInput_Rec1_8x8_0Y(p, v) (VME_Input_S1(p, ushort, 5, 9) = v)
#define VME_SET_IMEInput_Rec1_8x8_1X(p, v) (VME_Input_S1(p, ushort, 5, 10) = v)
#define VME_SET_IMEInput_Rec1_8x8_1Y(p, v) (VME_Input_S1(p, ushort, 5, 11) = v)
#define VME_SET_IMEInput_Rec1_8x8_2X(p, v) (VME_Input_S1(p, ushort, 5, 12) = v)
#define VME_SET_IMEInput_Rec1_8x8_2Y(p, v) (VME_Input_S1(p, ushort, 5, 13) = v)
#define VME_SET_IMEInput_Rec1_8x8_3X(p, v) (VME_Input_S1(p, ushort, 5, 14) = v)
#define VME_SET_IMEInput_Rec1_8x8_3Y(p, v) (VME_Input_S1(p, ushort, 5, 15) = v)

// Format = S13.2, Valid Range: [-2048.00 to 2047.75]
// For chroma skip: Format = S12.3, Hardware Range: [-1024.000 to 1023.875]
#define VME_SET_SICInput_Ref0SkipCenter0DeltaXY(p, DeltaX, DeltaY)             \
  (VME_Input_S1(p, uint, 0, 0) = DeltaX | (DeltaY << 16))
#define VME_SET_SICInput_Ref1SkipCenter0DeltaXY(p, DeltaX, DeltaY)             \
  (VME_Input_S1(p, uint, 0, 1) = DeltaX | (DeltaY << 16))
#define VME_SET_SICInput_Ref0SkipCenter1DeltaXY(p, DeltaX, DeltaY)             \
  (VME_Input_S1(p, uint, 0, 2) = DeltaX | (DeltaY << 16))
#define VME_SET_SICInput_Ref1SkipCenter1DeltaXY(p, DeltaX, DeltaY)             \
  (VME_Input_S1(p, uint, 0, 3) = DeltaX | (DeltaY << 16))
#define VME_SET_SICInput_Ref0SkipCenter2DeltaXY(p, DeltaX, DeltaY)             \
  (VME_Input_S1(p, uint, 0, 4) = DeltaX | (DeltaY << 16))
#define VME_SET_SICInput_Ref1SkipCenter2DeltaXY(p, DeltaX, DeltaY)             \
  (VME_Input_S1(p, uint, 0, 5) = DeltaX | (DeltaY << 16))
#define VME_SET_SICInput_Ref0SkipCenter3DeltaXY(p, DeltaX, DeltaY)             \
  (VME_Input_S1(p, uint, 0, 6) = DeltaX | (DeltaY << 16))
#define VME_SET_SICInput_Ref1SkipCenter3DeltaXY(p, DeltaX, DeltaY)             \
  (VME_Input_S1(p, uint, 0, 7) = DeltaX | (DeltaY << 16))

// 2-bit field with the following definition
// 00: Luma + Chroma enabled
// 01: Luma only
// 1X: Intra disabled
#define VME_SET_SICInput_IntraComputeType(p, v)                                \
  (VME_Input_S1(p, uchar, 1, 5) = v)

#define VME_SET_SICInput_CornerNeighborPixel0(p, v)                            \
  (VME_Input_S1(p, uchar, 1, 7) = v)

// NOTE: combine into one vector assignment
#define VME_SET_SICInput_NeighborPixelLum0(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 8) = v)
#define VME_SET_SICInput_NeighborPixelLum1(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 9) = v)
#define VME_SET_SICInput_NeighborPixelLum2(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 10) = v)
#define VME_SET_SICInput_NeighborPixelLum3(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 11) = v)
#define VME_SET_SICInput_NeighborPixelLum4(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 12) = v)
#define VME_SET_SICInput_NeighborPixelLum5(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 13) = v)
#define VME_SET_SICInput_NeighborPixelLum6(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 14) = v)
#define VME_SET_SICInput_NeighborPixelLum7(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 15) = v)
#define VME_SET_SICInput_NeighborPixelLum8(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 16) = v)
#define VME_SET_SICInput_NeighborPixelLum9(p, v)                               \
  (VME_Input_S1(p, uchar, 1, 17) = v)
#define VME_SET_SICInput_NeighborPixelLum10(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 18) = v)
#define VME_SET_SICInput_NeighborPixelLum11(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 19) = v)
#define VME_SET_SICInput_NeighborPixelLum12(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 20) = v)
#define VME_SET_SICInput_NeighborPixelLum13(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 21) = v)
#define VME_SET_SICInput_NeighborPixelLum14(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 22) = v)
#define VME_SET_SICInput_NeighborPixelLum15(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 23) = v)
#define VME_SET_SICInput_NeighborPixelLum16(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 24) = v)
#define VME_SET_SICInput_NeighborPixelLum17(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 25) = v)
#define VME_SET_SICInput_NeighborPixelLum18(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 26) = v)
#define VME_SET_SICInput_NeighborPixelLum19(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 27) = v)
#define VME_SET_SICInput_NeighborPixelLum20(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 28) = v)
#define VME_SET_SICInput_NeighborPixelLum21(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 29) = v)
#define VME_SET_SICInput_NeighborPixelLum22(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 30) = v)
#define VME_SET_SICInput_NeighborPixelLum23(p, v)                              \
  (VME_Input_S1(p, uchar, 1, 31) = v)
#define VME_SET_SICInput_NeighborPixelLum24(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 0) = v)
#define VME_SET_SICInput_NeighborPixelLum25(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 1) = v)
#define VME_SET_SICInput_NeighborPixelLum26(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 2) = v)
#define VME_SET_SICInput_NeighborPixelLum27(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 3) = v)
#define VME_SET_SICInput_NeighborPixelLum28(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 4) = v)
#define VME_SET_SICInput_NeighborPixelLum29(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 5) = v)
#define VME_SET_SICInput_NeighborPixelLum30(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 6) = v)
#define VME_SET_SICInput_NeighborPixelLum31(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 7) = v)
#define VME_SET_SICInput_NeighborPixelLum32(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 8) = v)
#define VME_SET_SICInput_NeighborPixelLum33(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 9) = v)
#define VME_SET_SICInput_NeighborPixelLum34(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 10) = v)
#define VME_SET_SICInput_NeighborPixelLum35(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 11) = v)
#define VME_SET_SICInput_NeighborPixelLum36(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 12) = v)
#define VME_SET_SICInput_NeighborPixelLum37(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 13) = v)
#define VME_SET_SICInput_NeighborPixelLum38(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 14) = v)

#define VME_SET_SICInput_CornerNeighborPixel1(p, v)                            \
  (VME_Input_S1(p, uchar, 2, 15) = v)

// Format = U4
#define VME_SET_SICInput_IntraMxMPredModeA5(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 16) = VME_Input_S1(p, uchar, 2, 16) & 0xF0 | v)
#define VME_SET_SICInput_IntraMxMPredModeA7(p, v)                              \
  (VME_Input_S1(p, uchar, 2, 16) =                                             \
       VME_Input_S1(p, uchar, 2, 16) & 0x0F | (v << 4))
#define VME_SET_SICInput_IntraMxMPredModeA13(p, v)                             \
  (VME_Input_S1(p, uchar, 2, 17) = VME_Input_S1(p, uchar, 2, 17) & 0xF0 | v)
#define VME_SET_SICInput_IntraMxMPredModeA15(p, v)                             \
  (VME_Input_S1(p, uchar, 2, 17) =                                             \
       VME_Input_S1(p, uchar, 2, 17) & 0x0F | (v << 4))
#define VME_SET_SICInput_IntraMxMPredModeB10(p, v)                             \
  (VME_Input_S1(p, uchar, 2, 18) = VME_Input_S1(p, uchar, 2, 18) & 0xF0 | v)
#define VME_SET_SICInput_IntraMxMPredModeB11(p, v)                             \
  (VME_Input_S1(p, uchar, 2, 18) =                                             \
       VME_Input_S1(p, uchar, 2, 18) & 0x0F | (v << 4))
#define VME_SET_SICInput_IntraMxMPredModeB14(p, v)                             \
  (VME_Input_S1(p, uchar, 2, 19) = VME_Input_S1(p, uchar, 2, 19) & 0xF0 | v)
#define VME_SET_SICInput_IntraMxMPredModeB15(p, v)                             \
  (VME_Input_S1(p, uchar, 2, 19) =                                             \
       VME_Input_S1(p, uchar, 2, 19) & 0x0F | (v << 4))

// Format = U8 pair
#define VME_SET_SICInput_CornerNeighborPixelChroma(p, v)                       \
  (VME_Input_S1(p, ushort, 2, 10) = v)

#if 0
// Format = U4U4
#define VME_SET_SICInput_PenaltyIntra4x4NonDC(p, v)                            \
  (VME_Input_S1(p, uchar, 2, 28) = v)
// Format = U2
#define VME_SET_SICInput_ScaleFactorIntra8x8NonDC(p, v)                        \
  (VME_Input_S1(p, uchar, 2, 29) = VME_Input_S1(p, uchar, 2, 29) & 0xFC | v)
// Format = U2
#define VME_SET_SICInput_ScaleFactorIntra16x16NonDC(p, v)                      \
  (VME_Input_S1(p, uchar, 2, 29) =                                             \
       VME_Input_S1(p, uchar, 2, 29) & 0xF3 | (v << 2))
#else
// BSPEC Update Rev1.6
// Format = U8
#define VME_SET_SICInput_PenaltyIntra4x4NonDC(p, v)                            \
  (VME_Input_G1(p, uchar, 2, 30) = v)
// Format = U8
#define VME_SET_SICInput_PenaltyIntra8x8NonDC(p, v)                            \
  (VME_Input_G1(p, uchar, 2, 29) = v)
// Format = U8
#define VME_SET_SICInput_PenaltyIntra16x16NonDC(p, v)                          \
  (VME_Input_G1(p, uchar, 2, 28) = v)
#endif

#define VME_SET_SICInput_NeighborPixelChroma0(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 0) = v)
#define VME_SET_SICInput_NeighborPixelChroma1(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 1) = v)
#define VME_SET_SICInput_NeighborPixelChroma2(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 2) = v)
#define VME_SET_SICInput_NeighborPixelChroma3(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 3) = v)

#define VME_SET_SICInput_NeighborPixelChroma4(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 4) = v)
#define VME_SET_SICInput_NeighborPixelChroma5(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 5) = v)
#define VME_SET_SICInput_NeighborPixelChroma6(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 6) = v)
#define VME_SET_SICInput_NeighborPixelChroma7(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 7) = v)

#define VME_SET_SICInput_NeighborPixelChroma8(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 8) = v)
#define VME_SET_SICInput_NeighborPixelChroma9(p, v)                            \
  (VME_Input_G1(p, uchar, 3, 9) = v)
#define VME_SET_SICInput_NeighborPixelChroma10(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 10) = v)
#define VME_SET_SICInput_NeighborPixelChroma11(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 11) = v)

#define VME_SET_SICInput_NeighborPixelChroma12(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 12) = v)
#define VME_SET_SICInput_NeighborPixelChroma13(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 13) = v)
#define VME_SET_SICInput_NeighborPixelChroma14(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 14) = v)
#define VME_SET_SICInput_NeighborPixelChroma15(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 15) = v)

#define VME_SET_SICInput_NeighborPixelChroma16(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 16) = v)
#define VME_SET_SICInput_NeighborPixelChroma17(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 17) = v)
#define VME_SET_SICInput_NeighborPixelChroma18(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 18) = v)
#define VME_SET_SICInput_NeighborPixelChroma19(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 19) = v)

#define VME_SET_SICInput_NeighborPixelChroma20(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 20) = v)
#define VME_SET_SICInput_NeighborPixelChroma21(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 21) = v)
#define VME_SET_SICInput_NeighborPixelChroma22(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 22) = v)
#define VME_SET_SICInput_NeighborPixelChroma23(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 23) = v)

#define VME_SET_SICInput_NeighborPixelChroma24(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 24) = v)
#define VME_SET_SICInput_NeighborPixelChroma25(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 25) = v)
#define VME_SET_SICInput_NeighborPixelChroma26(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 26) = v)
#define VME_SET_SICInput_NeighborPixelChroma27(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 27) = v)

#define VME_SET_SICInput_NeighborPixelChroma28(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 28) = v)
#define VME_SET_SICInput_NeighborPixelChroma29(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 29) = v)
#define VME_SET_SICInput_NeighborPixelChroma30(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 30) = v)
#define VME_SET_SICInput_NeighborPixelChroma31(p, v)                           \
  (VME_Input_G1(p, uchar, 3, 31) = v)

#define VME_SET_FBRInput_Ref0SubBlockX0(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 0) = v)
#define VME_SET_FBRInput_Ref0SubBlockY0(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 1) = v)
#define VME_SET_FBRInput_Ref1SubBlockX0(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 2) = v)
#define VME_SET_FBRInput_Ref1SubBlockY0(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 3) = v)

#define VME_SET_FBRInput_Ref0SubBlockX1(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 4) = v)
#define VME_SET_FBRInput_Ref0SubBlockY1(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 5) = v)
#define VME_SET_FBRInput_Ref1SubBlockX1(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 6) = v)
#define VME_SET_FBRInput_Ref1SubBlockY1(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 7) = v)

#define VME_SET_FBRInput_Ref0SubBlockX2(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 8) = v)
#define VME_SET_FBRInput_Ref0SubBlockY2(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 9) = v)
#define VME_SET_FBRInput_Ref1SubBlockX2(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 10) = v)
#define VME_SET_FBRInput_Ref1SubBlockY2(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 11) = v)

#define VME_SET_FBRInput_Ref0SubBlockX3(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 12) = v)
#define VME_SET_FBRInput_Ref0SubBlockY3(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 13) = v)
#define VME_SET_FBRInput_Ref1SubBlockX3(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 14) = v)
#define VME_SET_FBRInput_Ref1SubBlockY3(p, v)                                  \
  (VME_Input_S1(p, ushort, 0, 15) = v)

#define VME_SET_FBRInput_Ref0SubBlockX4(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 0) = v)
#define VME_SET_FBRInput_Ref0SubBlockY4(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 1) = v)
#define VME_SET_FBRInput_Ref1SubBlockX4(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 2) = v)
#define VME_SET_FBRInput_Ref1SubBlockY4(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 3) = v)

#define VME_SET_FBRInput_Ref0SubBlockX5(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 4) = v)
#define VME_SET_FBRInput_Ref0SubBlockY5(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 5) = v)
#define VME_SET_FBRInput_Ref1SubBlockX5(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 6) = v)
#define VME_SET_FBRInput_Ref1SubBlockY5(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 7) = v)

#define VME_SET_FBRInput_Ref0SubBlockX6(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 8) = v)
#define VME_SET_FBRInput_Ref0SubBlockY6(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 9) = v)
#define VME_SET_FBRInput_Ref1SubBlockX6(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 10) = v)
#define VME_SET_FBRInput_Ref1SubBlockY6(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 11) = v)

#define VME_SET_FBRInput_Ref0SubBlockX7(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 12) = v)
#define VME_SET_FBRInput_Ref0SubBlockY7(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 13) = v)
#define VME_SET_FBRInput_Ref1SubBlockX7(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 14) = v)
#define VME_SET_FBRInput_Ref1SubBlockY7(p, v)                                  \
  (VME_Input_S1(p, ushort, 1, 15) = v)

#define VME_SET_FBRInput_Ref0SubBlockX8(p, v)                                  \
  (VME_Input_S1(p, ushort, 2, 0) = v)
#define VME_SET_FBRInput_Ref0SubBlockY8(p, v)                                  \
  (VME_Input_S1(p, ushort, 2, 1) = v)
#define VME_SET_FBRInput_Ref1SubBlockX8(p, v)                                  \
  (VME_Input_S1(p, ushort, 2, 2) = v)
#define VME_SET_FBRInput_Ref1SubBlockY8(p, v)                                  \
  (VME_Input_S1(p, ushort, 2, 3) = v)

#define VME_SET_FBRInput_Ref0SubBlockX9(p, v)                                  \
  (VME_Input_S1(p, ushort, 2, 4) = v)
#define VME_SET_FBRInput_Ref0SubBlockY9(p, v)                                  \
  (VME_Input_S1(p, ushort, 2, 5) = v)
#define VME_SET_FBRInput_Ref1SubBlockX9(p, v)                                  \
  (VME_Input_S1(p, ushort, 2, 6) = v)
#define VME_SET_FBRInput_Ref1SubBlockY9(p, v)                                  \
  (VME_Input_S1(p, ushort, 2, 7) = v)

#define VME_SET_FBRInput_Ref0SubBlockX10(p, v)                                 \
  (VME_Input_S1(p, ushort, 2, 8) = v)
#define VME_SET_FBRInput_Ref0SubBlockY10(p, v)                                 \
  (VME_Input_S1(p, ushort, 2, 9) = v)
#define VME_SET_FBRInput_Ref1SubBlockX10(p, v)                                 \
  (VME_Input_S1(p, ushort, 2, 10) = v)
#define VME_SET_FBRInput_Ref1SubBlockY10(p, v)                                 \
  (VME_Input_S1(p, ushort, 2, 11) = v)

#define VME_SET_FBRInput_Ref0SubBlockX11(p, v)                                 \
  (VME_Input_S1(p, ushort, 2, 12) = v)
#define VME_SET_FBRInput_Ref0SubBlockY11(p, v)                                 \
  (VME_Input_S1(p, ushort, 2, 13) = v)
#define VME_SET_FBRInput_Ref1SubBlockX11(p, v)                                 \
  (VME_Input_S1(p, ushort, 2, 14) = v)
#define VME_SET_FBRInput_Ref1SubBlockY11(p, v)                                 \
  (VME_Input_S1(p, ushort, 2, 15) = v)

#define VME_SET_FBRInput_Ref0SubBlockX12(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 0) = v)
#define VME_SET_FBRInput_Ref0SubBlockY12(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 1) = v)
#define VME_SET_FBRInput_Ref1SubBlockX12(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 2) = v)
#define VME_SET_FBRInput_Ref1SubBlockY12(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 3) = v)

#define VME_SET_FBRInput_Ref0SubBlockX13(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 4) = v)
#define VME_SET_FBRInput_Ref0SubBlockY13(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 5) = v)
#define VME_SET_FBRInput_Ref1SubBlockX13(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 6) = v)
#define VME_SET_FBRInput_Ref1SubBlockY13(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 7) = v)

#define VME_SET_FBRInput_Ref0SubBlockX14(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 8) = v)
#define VME_SET_FBRInput_Ref0SubBlockY14(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 9) = v)
#define VME_SET_FBRInput_Ref1SubBlockX14(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 10) = v)
#define VME_SET_FBRInput_Ref1SubBlockY14(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 11) = v)

#define VME_SET_FBRInput_Ref0SubBlockX15(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 12) = v)
#define VME_SET_FBRInput_Ref0SubBlockY15(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 13) = v)
#define VME_SET_FBRInput_Ref1SubBlockX15(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 14) = v)
#define VME_SET_FBRInput_Ref1SubBlockY15(p, v)                                 \
  (VME_Input_S1(p, ushort, 3, 15) = v)

#define VME_SET_IDMInput_SrcMBPixelMaskRow0(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 0) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow1(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 1) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow2(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 2) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow3(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 3) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow4(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 4) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow5(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 5) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow6(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 6) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow7(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 7) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow8(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 8) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow9(p, v)                              \
  (VME_Input_S1(p, ushort, 0, 9) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow10(p, v)                             \
  (VME_Input_S1(p, ushort, 0, 10) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow11(p, v)                             \
  (VME_Input_S1(p, ushort, 0, 11) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow12(p, v)                             \
  (VME_Input_S1(p, ushort, 0, 12) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow13(p, v)                             \
  (VME_Input_S1(p, ushort, 0, 13) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow14(p, v)                             \
  (VME_Input_S1(p, ushort, 0, 14) = v)
#define VME_SET_IDMInput_SrcMBPixelMaskRow15(p, v)                             \
  (VME_Input_S1(p, ushort, 0, 15) = v)

// GET methods

// Format = I16
#define VME_GET_UNIInput_Ref0X(p) (VME_Input_G1(p, ushort, 0, 0))
#define VME_GET_UNIInput_Ref0Y(p) (VME_Input_G1(p, ushort, 0, 1))
#define VME_GET_UNIInput_Ref1X(p) (VME_Input_G1(p, ushort, 0, 2))
#define VME_GET_UNIInput_Ref1Y(p) (VME_Input_G1(p, ushort, 0, 3))

// Format = U16
#define VME_GET_UNIInput_SrcX(p) (VME_Input_G1(p, ushort, 0, 4))

// Format = U16
#define VME_GET_UNIInput_SrcY(p) (VME_Input_G1(p, ushort, 0, 5))

// Format = U8
#define VME_GET_UNIInput_SrcType(p) (VME_Input_G1(p, uchar, 0, 12))

// Format = U8
#define VME_GET_UNIInput_VmeModes(p) (VME_Input_G1(p, uchar, 0, 13))

// Format = U8
#define VME_GET_UNIInput_SadType(p) (VME_Input_G1(p, uchar, 0, 14))

// Format = U8
#define VME_GET_UNIInput_ShapeMask(p) (VME_Input_G1(p, uchar, 0, 15))

// The value must be a multiple of 4. Range = [20, 64]
#define VME_GET_UNIInput_RefW(p) (VME_Input_G1(p, uchar, 0, 22))

// The value must be a multiple of 4. Range = [20, 64]
#define VME_GET_UNIInput_RefH(p) (VME_Input_G1(p, uchar, 0, 23))

// Format = U8
#define VME_GET_UNIInput_VmeFlags(p) (VME_Input_G1(p, uchar, 1, 0))

// Format = U8
#define VME_GET_UNIInput_EarlyImeStop(p) (VME_Input_G1(p, uchar, 1, 3))

// Format = U8
#define VME_GET_UNIInput_MaxNumMVs(p) (VME_Input_G1(p, uchar, 1, 4))

#define VME_GET_UNIInput_Ref0Polarity(p) (VME_Input_G1(p, uchar, 1, 5) & 0x0F)
#define VME_GET_UNIInput_Ref1Polarity(p) (VME_Input_G1(p, uchar, 1, 5) >> 4)

// Format = U6, Valid Values: [16, 21, 32, 43, 48]
#define VME_GET_UNIInput_BiWeight(p) (VME_Input_G1(p, uchar, 1, 6))

#define VME_GET_UNIInput_RefPixelBiasEnable(p)                                 \
  ((VME_Input_G1(p, uchar, 1, 7) & 0x20) ? 1 : 0)

#define VME_GET_UNIInput_UniMixDisable(p)                                      \
  ((VME_Input_G1(p, uchar, 1, 7) & 0x10) ? 1 : 0)

// Format = U8
// #define VME_GET_UNIInput_BiDirMask(p) ((VME_Input_G1(p, uchar, 1, 7) & 0x10)
// ? 1 : 0)

// Format = U8, Valid range [1,63]
#define VME_GET_UNIInput_LenSP(p) (VME_Input_G1(p, uchar, 1, 8))

// Format = U8, Valid range [1,63]
#define VME_GET_UNIInput_MaxNumSU(p) (VME_Input_G1(p, uchar, 1, 9))

// Format = U8
#define VME_GET_UNIInput_StartCenter0(p) (VME_Input_G1(p, uchar, 1, 10))

// Format = U8
#define VME_GET_UNIInput_StartCenter1(p) (VME_Input_G1(p, uchar, 1, 11))

// Format = U8
#define VME_GET_UNIInput_WeightedSADCtrl0_3(p) (VME_Input_G1(p, uchar, 1, 12))
#define VME_GET_UNIInput_WeightedSADCtrl4_7(p) (VME_Input_G1(p, uchar, 1, 13))
#define VME_GET_UNIInput_WeightedSADCtrl8_11(p) (VME_Input_G1(p, uchar, 1, 14))
#define VME_GET_UNIInput_WeightedSADCtrl12_15(p) (VME_Input_G1(p, uchar, 1, 15))

// Format = U8
#define VME_GET_UNIInput_Blk0RefID(p) (VME_Input_G1(p, uchar, 1, 24))
#define VME_GET_UNIInput_Blk1RefID(p) (VME_Input_G1(p, uchar, 1, 25))
#define VME_GET_UNIInput_Blk2RefID(p) (VME_Input_G1(p, uchar, 1, 26))
#define VME_GET_UNIInput_Blk3RefID(p) (VME_Input_G1(p, uchar, 1, 27))

// Format = U8
#define VME_GET_UNIInput_IntraFlags(p) (VME_Input_G1(p, uchar, 1, 28))

// v is a 8-bit mask with the following definition:
// Bit-7    Reserved : MBZ (for IntraPredAvailFlagF - F (pixel[-1,7]
// available
// for MbAff)
// Bit-6    Reserved : MBZ (for IntraPredAvailFlagA/E - A (left neighbor top
// half for MbAff)
// Bit-5    IntraPredAvailFlagE/A - A (Left neighbor or Left bottom half)
// Bit-4    IntraPredAvailFlagB - B (Upper neighbor)
// Bit-3    IntraPredAvailFlagC - C (Upper left neighbor)
// Bit-2    IntraPredAvailFlagD - D (Upper right neighbor)
// Bit-1:0  Reserved: MBZ (ChromaIntraPredMode)
#define VME_GET_UNIInput_MbIntraStruct(p) (VME_Input_G1(p, uchar, 1, 29))

// Format = U8
#define VME_GET_UNIInput_MvFlags(p) (VME_Input_G1(p, uchar, 1, 30))

// v is a 8-bit mask with the following definition:
// xxxx xxx1: Ref0 Skip Center 0 is enabled [corresponds to M2.0]
// xxxx xx1x: Ref1 Skip Center 0 is enabled [corresponds to M2.1]
// xxxx x1xx: Ref0 Skip Center 1 is enabled [corresponds to M2.2]
// xxxx 1xxx: Ref1 Skip Center 1 is enabled [corresponds to M2.3]
// xxx1 xxxx: Ref0 Skip Center 2 is enabled [corresponds to M2.4]
// xx1x xxxx: Ref1 Skip Center 2 is enabled [corresponds to M2.5]
// x1xx xxxx: Ref0 Skip Center 3 is enabled [corresponds to M2.6]
// 1xxx xxxx: Ref1 Skip Center 3 is enabled [corresponds to M2.7]
#define VME_GET_UNIInput_SkipCenterMask(p) (VME_Input_G1(p, uchar, 1, 31))

// NOTE: replace v with U4U4 type with
// (shift_count, shift_val) and set the value as (shift_val | (shift_count <<
// 4))

// Format = U4U4 (encoded value must fit in 12-bits)
#define VME_GET_UNIInput_Mode0Cost(p) (VME_Input_G1(p, uchar, 2, 0))
#define VME_GET_UNIInput_Mode1Cost(p) (VME_Input_G1(p, uchar, 2, 1))
#define VME_GET_UNIInput_Mode2Cost(p) (VME_Input_G1(p, uchar, 2, 2))
#define VME_GET_UNIInput_Mode3Cost(p) (VME_Input_G1(p, uchar, 2, 3))
#define VME_GET_UNIInput_Mode4Cost(p) (VME_Input_G1(p, uchar, 2, 4))
#define VME_GET_UNIInput_Mode5Cost(p) (VME_Input_G1(p, uchar, 2, 5))
#define VME_GET_UNIInput_Mode6Cost(p) (VME_Input_G1(p, uchar, 2, 6))
#define VME_GET_UNIInput_Mode7Cost(p) (VME_Input_G1(p, uchar, 2, 7))
#define VME_GET_UNIInput_Mode8Cost(p) (VME_Input_G1(p, uchar, 2, 8))
#define VME_GET_UNIInput_Mode9Cost(p) (VME_Input_G1(p, uchar, 2, 9))

// Format = U4U4 (encoded value must fit in 12-bits)
#define VME_GET_UNIInput_RefIDCost(p) (VME_Input_G1(p, uchar, 2, 10))

// Format = U4U4 (encoded value must fit in 12-bits)
#define VME_GET_UNIInput_ChromaIntraModeCost(p) (VME_Input_G1(p, uchar, 2, 11))

// Format = U4U4 (encoded value must fit in 10-bits)
#define VME_GET_UNIInput_MV0Cost(p) (VME_Input_G1(p, uchar, 2, 12))
#define VME_GET_UNIInput_MV1Cost(p) (VME_Input_G1(p, uchar, 2, 13))
#define VME_GET_UNIInput_MV2Cost(p) (VME_Input_G1(p, uchar, 2, 14))
#define VME_GET_UNIInput_MV3Cost(p) (VME_Input_G1(p, uchar, 2, 15))
#define VME_GET_UNIInput_MV4Cost(p) (VME_Input_G1(p, uchar, 2, 16))
#define VME_GET_UNIInput_MV5Cost(p) (VME_Input_G1(p, uchar, 2, 17))
#define VME_GET_UNIInput_MV6Cost(p) (VME_Input_G1(p, uchar, 2, 18))
#define VME_GET_UNIInput_MV7Cost(p) (VME_Input_G1(p, uchar, 2, 19))

// Format = U8
#define VME_GET_UNIInput_FBRMbModeInput(p) (VME_Input_G1(p, uchar, 2, 20) & 0x3)

// Format = U8
#define VME_GET_UNIInput_FBRSubMBShapeInput(p) (VME_Input_G1(p, uchar, 2, 21))

// Format = U8
#define VME_GET_UNIInput_FBRSubPredModeInput(p) (VME_Input_G1(p, uchar, 2, 22))

// Format = U16
#define VME_GET_UNIInput_SICFwdTCoeffThreshold0(p)                             \
  (VME_Input_G1(p, ushort, 2, 12))
// Format = U8
#define VME_GET_UNIInput_SICFwdTCoeffThreshold1(p)                             \
  (VME_Input_G1(p, uchar, 2, 26))
#define VME_GET_UNIInput_SICFwdTCoeffThreshold2(p)                             \
  (VME_Input_G1(p, uchar, 2, 27))
#define VME_GET_UNIInput_SICFwdTCoeffThreshold3(p)                             \
  (VME_Input_G1(p, uchar, 2, 28))
#define VME_GET_UNIInput_SICFwdTCoeffThreshold4(p)                             \
  (VME_Input_G1(p, uchar, 2, 29))
#define VME_GET_UNIInput_SICFwdTCoeffThreshold5(p)                             \
  (VME_Input_G1(p, uchar, 2, 30))
#define VME_GET_UNIInput_SICFwdTCoeffThreshold6(p)                             \
  (VME_Input_G1(p, uchar, 2, 31))

// Return DeltaX in the lower 16 bit and DeltaY in the higher 16 bits
// Format = S13.2, DletaX Valid Range: [-2048.00 to 2047.75], DletaY Valid
// Range: [-512.00 to 511.75]
#define VME_GET_UNIInput_FWDCostCenter0_X(p) (VME_Input_G1(p, ushort, 3, 0))
#define VME_GET_UNIInput_FWDCostCenter0_Y(p) (VME_Input_G1(p, ushort, 3, 1))
#define VME_GET_UNIInput_BWDCostCenter0_X(p) (VME_Input_G1(p, ushort, 3, 2))
#define VME_GET_UNIInput_BWDCostCenter0_Y(p) (VME_Input_G1(p, ushort, 3, 3))
#define VME_GET_UNIInput_FWDCostCenter1_X(p) (VME_Input_G1(p, ushort, 3, 4))
#define VME_GET_UNIInput_FWDCostCenter1_Y(p) (VME_Input_G1(p, ushort, 3, 5))
#define VME_GET_UNIInput_BWDCostCenter1_X(p) (VME_Input_G1(p, ushort, 3, 6))
#define VME_GET_UNIInput_BWDCostCenter1_Y(p) (VME_Input_G1(p, ushort, 3, 7))
#define VME_GET_UNIInput_FWDCostCenter2_X(p) (VME_Input_G1(p, ushort, 3, 8))
#define VME_GET_UNIInput_FWDCostCenter2_Y(p) (VME_Input_G1(p, ushort, 3, 9))
#define VME_GET_UNIInput_BWDCostCenter2_X(p) (VME_Input_G1(p, ushort, 3, 10))
#define VME_GET_UNIInput_BWDCostCenter2_Y(p) (VME_Input_G1(p, ushort, 3, 11))
#define VME_GET_UNIInput_FWDCostCenter3_X(p) (VME_Input_G1(p, ushort, 3, 12))
#define VME_GET_UNIInput_FWDCostCenter3_Y(p) (VME_Input_G1(p, ushort, 3, 13))
#define VME_GET_UNIInput_BWDCostCenter3_X(p) (VME_Input_G1(p, ushort, 3, 14))
#define VME_GET_UNIInput_BWDCostCenter3_Y(p) (VME_Input_G1(p, ushort, 3, 15))

// Format = U8, with the following definition:
// [7:4] (Y) - specifies relative Y distance to the next SU from previous SU in
// units of SU
// [3:0] (X) - specifies relative X distance to the next SU from previous SU in
// units of SU
#define VME_GET_IMEInput_IMESearchPathDelta0(p) (VME_Input_G1(p, uchar, 0, 0))
#define VME_GET_IMEInput_IMESearchPathDelta1(p) (VME_Input_G1(p, uchar, 0, 1))
#define VME_GET_IMEInput_IMESearchPathDelta2(p) (VME_Input_G1(p, uchar, 0, 2))
#define VME_GET_IMEInput_IMESearchPathDelta3(p) (VME_Input_G1(p, uchar, 0, 3))
#define VME_GET_IMEInput_IMESearchPathDelta4(p) (VME_Input_G1(p, uchar, 0, 4))
#define VME_GET_IMEInput_IMESearchPathDelta5(p) (VME_Input_G1(p, uchar, 0, 5))
#define VME_GET_IMEInput_IMESearchPathDelta6(p) (VME_Input_G1(p, uchar, 0, 6))
#define VME_GET_IMEInput_IMESearchPathDelta7(p) (VME_Input_G1(p, uchar, 0, 7))
#define VME_GET_IMEInput_IMESearchPathDelta8(p) (VME_Input_G1(p, uchar, 0, 8))
#define VME_GET_IMEInput_IMESearchPathDelta9(p) (VME_Input_G1(p, uchar, 0, 9))
#define VME_GET_IMEInput_IMESearchPathDelta10(p) (VME_Input_G1(p, uchar, 0, 10))
#define VME_GET_IMEInput_IMESearchPathDelta11(p) (VME_Input_G1(p, uchar, 0, 11))
#define VME_GET_IMEInput_IMESearchPathDelta12(p) (VME_Input_G1(p, uchar, 0, 12))
#define VME_GET_IMEInput_IMESearchPathDelta13(p) (VME_Input_G1(p, uchar, 0, 13))
#define VME_GET_IMEInput_IMESearchPathDelta14(p) (VME_Input_G1(p, uchar, 0, 14))
#define VME_GET_IMEInput_IMESearchPathDelta15(p) (VME_Input_G1(p, uchar, 0, 15))
#define VME_GET_IMEInput_IMESearchPathDelta16(p) (VME_Input_G1(p, uchar, 0, 16))
#define VME_GET_IMEInput_IMESearchPathDelta17(p) (VME_Input_G1(p, uchar, 0, 17))
#define VME_GET_IMEInput_IMESearchPathDelta18(p) (VME_Input_G1(p, uchar, 0, 18))
#define VME_GET_IMEInput_IMESearchPathDelta19(p) (VME_Input_G1(p, uchar, 0, 19))
#define VME_GET_IMEInput_IMESearchPathDelta20(p) (VME_Input_G1(p, uchar, 0, 20))
#define VME_GET_IMEInput_IMESearchPathDelta21(p) (VME_Input_G1(p, uchar, 0, 21))
#define VME_GET_IMEInput_IMESearchPathDelta22(p) (VME_Input_G1(p, uchar, 0, 22))
#define VME_GET_IMEInput_IMESearchPathDelta23(p) (VME_Input_G1(p, uchar, 0, 23))
#define VME_GET_IMEInput_IMESearchPathDelta24(p) (VME_Input_G1(p, uchar, 0, 24))
#define VME_GET_IMEInput_IMESearchPathDelta25(p) (VME_Input_G1(p, uchar, 0, 25))
#define VME_GET_IMEInput_IMESearchPathDelta26(p) (VME_Input_G1(p, uchar, 0, 26))
#define VME_GET_IMEInput_IMESearchPathDelta27(p) (VME_Input_G1(p, uchar, 0, 27))
#define VME_GET_IMEInput_IMESearchPathDelta28(p) (VME_Input_G1(p, uchar, 0, 28))
#define VME_GET_IMEInput_IMESearchPathDelta29(p) (VME_Input_G1(p, uchar, 0, 29))
#define VME_GET_IMEInput_IMESearchPathDelta30(p) (VME_Input_G1(p, uchar, 0, 30))
#define VME_GET_IMEInput_IMESearchPathDelta31(p) (VME_Input_G1(p, uchar, 0, 31))

#define VME_GET_IMEInput_IMESearchPathDelta32(p) (VME_Input_G1(p, uchar, 1, 0))
#define VME_GET_IMEInput_IMESearchPathDelta33(p) (VME_Input_G1(p, uchar, 1, 1))
#define VME_GET_IMEInput_IMESearchPathDelta34(p) (VME_Input_G1(p, uchar, 1, 2))
#define VME_GET_IMEInput_IMESearchPathDelta35(p) (VME_Input_G1(p, uchar, 1, 3))
#define VME_GET_IMEInput_IMESearchPathDelta36(p) (VME_Input_G1(p, uchar, 1, 4))
#define VME_GET_IMEInput_IMESearchPathDelta37(p) (VME_Input_G1(p, uchar, 1, 5))
#define VME_GET_IMEInput_IMESearchPathDelta38(p) (VME_Input_G1(p, uchar, 1, 6))
#define VME_GET_IMEInput_IMESearchPathDelta39(p) (VME_Input_G1(p, uchar, 1, 7))
#define VME_GET_IMEInput_IMESearchPathDelta40(p) (VME_Input_G1(p, uchar, 1, 8))
#define VME_GET_IMEInput_IMESearchPathDelta41(p) (VME_Input_G1(p, uchar, 1, 9))
#define VME_GET_IMEInput_IMESearchPathDelta42(p) (VME_Input_G1(p, uchar, 1, 10))
#define VME_GET_IMEInput_IMESearchPathDelta43(p) (VME_Input_G1(p, uchar, 1, 11))
#define VME_GET_IMEInput_IMESearchPathDelta44(p) (VME_Input_G1(p, uchar, 1, 12))
#define VME_GET_IMEInput_IMESearchPathDelta45(p) (VME_Input_G1(p, uchar, 1, 13))
#define VME_GET_IMEInput_IMESearchPathDelta46(p) (VME_Input_G1(p, uchar, 1, 14))
#define VME_GET_IMEInput_IMESearchPathDelta47(p) (VME_Input_G1(p, uchar, 1, 15))
#define VME_GET_IMEInput_IMESearchPathDelta48(p) (VME_Input_G1(p, uchar, 1, 16))
#define VME_GET_IMEInput_IMESearchPathDelta49(p) (VME_Input_G1(p, uchar, 1, 17))
#define VME_GET_IMEInput_IMESearchPathDelta50(p) (VME_Input_G1(p, uchar, 1, 18))
#define VME_GET_IMEInput_IMESearchPathDelta51(p) (VME_Input_G1(p, uchar, 1, 19))
#define VME_GET_IMEInput_IMESearchPathDelta52(p) (VME_Input_G1(p, uchar, 1, 20))
#define VME_GET_IMEInput_IMESearchPathDelta53(p) (VME_Input_G1(p, uchar, 1, 21))
#define VME_GET_IMEInput_IMESearchPathDelta54(p) (VME_Input_G1(p, uchar, 1, 22))
#define VME_GET_IMEInput_IMESearchPathDelta55(p) (VME_Input_G1(p, uchar, 1, 23))

// Format = U16
#define VME_GET_IMEInput_Rec0_16x8_0Distortion(p)                              \
  (VME_Input_G1(p, ushort, 2, 0))
#define VME_GET_IMEInput_Rec0_16x8_1Distortion(p)                              \
  (VME_Input_G1(p, ushort, 2, 1))
#define VME_GET_IMEInput_Rec0_8x16_0Distortion(p)                              \
  (VME_Input_G1(p, ushort, 2, 2))
#define VME_GET_IMEInput_Rec0_8x16_1Distortion(p)                              \
  (VME_Input_G1(p, ushort, 2, 3))
#define VME_GET_IMEInput_Rec0_8x8_0Distortion(p) (VME_Input_G1(p, ushort, 2, 4))
#define VME_GET_IMEInput_Rec0_8x8_1Distortion(p) (VME_Input_G1(p, ushort, 2, 5))
#define VME_GET_IMEInput_Rec0_8x8_2Distortion(p) (VME_Input_G1(p, ushort, 2, 6))
#define VME_GET_IMEInput_Rec0_8x8_3Distortion(p) (VME_Input_G1(p, ushort, 2, 7))
#define VME_GET_IMEInput_Rec0_16x16_Distortion(p)                              \
  (VME_Input_G1(p, ushort, 2, 8))

// Format = U4
#define VME_GET_UNIInput_Rec0_16x16_RefID(p)                                   \
  (VME_Input_G1(p, uchar, 2, 18) & 0x0F)

// Format = S13.2
#define VME_GET_IMEInput_Rec0_16x16_X(p) (VME_Input_G1(p, ushort, 2, 10))
#define VME_GET_IMEInput_Rec0_16x16_Y(p) (VME_Input_G1(p, ushort, 2, 11))

// Format = U4
#define VME_GET_IMEInput_Rec0_16x8_0to1RefID(p) (VME_Input_G1(p, uchar, 2, 24))
#define VME_GET_IMEInput_Rec0_8x16_0to1RefID(p) (VME_Input_G1(p, uchar, 2, 25))
#define VME_GET_IMEInput_Rec0_8x8_0to1RefID(p) (VME_Input_G1(p, uchar, 2, 26))
#define VME_GET_IMEInput_Rec0_8x8_2to3RefID(p) (VME_Input_G1(p, uchar, 2, 27))

// Format = S13.2
#define VME_GET_IMEInput_Rec0_16x8_0X(p) (VME_Input_G1(p, ushort, 3, 0))
#define VME_GET_IMEInput_Rec0_16x8_0Y(p) (VME_Input_G1(p, ushort, 3, 1))
#define VME_GET_IMEInput_Rec0_16x8_1X(p) (VME_Input_G1(p, ushort, 3, 2))
#define VME_GET_IMEInput_Rec0_16x8_1Y(p) (VME_Input_G1(p, ushort, 3, 3))
#define VME_GET_IMEInput_Rec0_8x16_0X(p) (VME_Input_G1(p, ushort, 3, 4))
#define VME_GET_IMEInput_Rec0_8x16_0Y(p) (VME_Input_G1(p, ushort, 3, 5))
#define VME_GET_IMEInput_Rec0_8x16_1X(p) (VME_Input_G1(p, ushort, 3, 6))
#define VME_GET_IMEInput_Rec0_8x16_1Y(p) (VME_Input_G1(p, ushort, 3, 7))
#define VME_GET_IMEInput_Rec0_8x8_0X(p) (VME_Input_G1(p, ushort, 3, 8))
#define VME_GET_IMEInput_Rec0_8x8_0Y(p) (VME_Input_G1(p, ushort, 3, 9))
#define VME_GET_IMEInput_Rec0_8x8_1X(p) (VME_Input_G1(p, ushort, 3, 10))
#define VME_GET_IMEInput_Rec0_8x8_1Y(p) (VME_Input_G1(p, ushort, 3, 11))
#define VME_GET_IMEInput_Rec0_8x8_2X(p) (VME_Input_G1(p, ushort, 3, 12))
#define VME_GET_IMEInput_Rec0_8x8_2Y(p) (VME_Input_G1(p, ushort, 3, 13))
#define VME_GET_IMEInput_Rec0_8x8_3X(p) (VME_Input_G1(p, ushort, 3, 14))
#define VME_GET_IMEInput_Rec0_8x8_3Y(p) (VME_Input_G1(p, ushort, 3, 15))

// Format = U16
#define VME_GET_IMEInput_Rec1_16x8_0Distortion(p)                              \
  (VME_Input_G1(p, ushort, 4, 0))
#define VME_GET_IMEInput_Rec1_16x8_1Distortion(p)                              \
  (VME_Input_G1(p, ushort, 4, 1))
#define VME_GET_IMEInput_Rec1_8x16_0Distortion(p)                              \
  (VME_Input_G1(p, ushort, 4, 2))
#define VME_GET_IMEInput_Rec1_8x16_1Distortion(p)                              \
  (VME_Input_G1(p, ushort, 4, 3))
#define VME_GET_IMEInput_Rec1_8x8_0Distortion(p) (VME_Input_G1(p, ushort, 4, 4))
#define VME_GET_IMEInput_Rec1_8x8_1Distortion(p) (VME_Input_G1(p, ushort, 4, 5))
#define VME_GET_IMEInput_Rec1_8x8_2Distortion(p) (VME_Input_G1(p, ushort, 4, 6))
#define VME_GET_IMEInput_Rec1_8x8_3Distortion(p) (VME_Input_G1(p, ushort, 4, 7))
#define VME_GET_IMEInput_Rec1_16x16_Distortion(p)                              \
  (VME_Input_G1(p, ushort, 4, 8))

// Format = U4
#define VME_GET_UNIInput_Rec1_16x16_RefID(p) (VME_Input_G1(p, uchar, 4, 18))

// Format = S13.2
#define VME_GET_IMEInput_Rec1_16x16_X(p) (VME_Input_G1(p, ushort, 4, 10))
#define VME_GET_IMEInput_Rec1_16x16_Y(p) (VME_Input_G1(p, ushort, 4, 11))

// Format = U4
#define VME_GET_IMEInput_Rec1_16x8_0to1RefID(p) (VME_Input_G1(p, uchar, 4, 24))
#define VME_GET_IMEInput_Rec1_8x16_0to1RefID(p) (VME_Input_G1(p, uchar, 4, 25))
#define VME_GET_IMEInput_Rec1_8x8_0to1RefID(p) (VME_Input_G1(p, uchar, 4, 26))
#define VME_GET_IMEInput_Rec1_8x8_2to3RefID(p) (VME_Input_G1(p, uchar, 4, 27))

// Format = S13.2
#define VME_GET_IMEInput_Rec1_16x8_0X(p) (VME_Input_G1(p, ushort, 5, 0))
#define VME_GET_IMEInput_Rec1_16x8_0Y(p) (VME_Input_G1(p, ushort, 5, 1))
#define VME_GET_IMEInput_Rec1_16x8_1X(p) (VME_Input_G1(p, ushort, 5, 2))
#define VME_GET_IMEInput_Rec1_16x8_1Y(p) (VME_Input_G1(p, ushort, 5, 3))
#define VME_GET_IMEInput_Rec1_8x16_0X(p) (VME_Input_G1(p, ushort, 5, 4))
#define VME_GET_IMEInput_Rec1_8x16_0Y(p) (VME_Input_G1(p, ushort, 5, 5))
#define VME_GET_IMEInput_Rec1_8x16_1X(p) (VME_Input_G1(p, ushort, 5, 6))
#define VME_GET_IMEInput_Rec1_8x16_1Y(p) (VME_Input_G1(p, ushort, 5, 7))
#define VME_GET_IMEInput_Rec1_8x8_0X(p) (VME_Input_G1(p, ushort, 5, 8))
#define VME_GET_IMEInput_Rec1_8x8_0Y(p) (VME_Input_G1(p, ushort, 5, 9))
#define VME_GET_IMEInput_Rec1_8x8_1X(p) (VME_Input_G1(p, ushort, 5, 10))
#define VME_GET_IMEInput_Rec1_8x8_1Y(p) (VME_Input_G1(p, ushort, 5, 11))
#define VME_GET_IMEInput_Rec1_8x8_2X(p) (VME_Input_G1(p, ushort, 5, 12))
#define VME_GET_IMEInput_Rec1_8x8_2Y(p) (VME_Input_G1(p, ushort, 5, 13))
#define VME_GET_IMEInput_Rec1_8x8_3X(p) (VME_Input_G1(p, ushort, 5, 14))
#define VME_GET_IMEInput_Rec1_8x8_3Y(p) (VME_Input_G1(p, ushort, 5, 15))

// Format = S13.2, Valid Range: [-2048.00 to 2047.75]
// For chroma skip: Format = S12.3, Hardware Range: [-1024.000 to 1023.875]
#define VME_GET_SICInput_Ref0SkipCenter0DeltaX(p)                              \
  (VME_Input_G1(p, ushort, 0, 0))
#define VME_GET_SICInput_Ref0SkipCenter0DeltaY(p)                              \
  (VME_Input_G1(p, ushort, 0, 1))

#define VME_GET_SICInput_Ref1SkipCenter0DeltaX(p)                              \
  (VME_Input_G1(p, ushort, 0, 2))
#define VME_GET_SICInput_Ref1SkipCenter0DeltaY(p)                              \
  (VME_Input_G1(p, ushort, 0, 3))

#define VME_GET_SICInput_Ref0SkipCenter1DeltaX(p)                              \
  (VME_Input_G1(p, ushort, 0, 4))
#define VME_GET_SICInput_Ref0SkipCenter1DeltaY(p)                              \
  (VME_Input_G1(p, ushort, 0, 5))

#define VME_GET_SICInput_Ref1SkipCenter1DeltaX(p)                              \
  (VME_Input_G1(p, ushort, 0, 6))
#define VME_GET_SICInput_Ref1SkipCenter1DeltaY(p)                              \
  (VME_Input_G1(p, ushort, 0, 7))

#define VME_GET_SICInput_Ref0SkipCenter2DeltaX(p)                              \
  (VME_Input_G1(p, ushort, 0, 8))
#define VME_GET_SICInput_Ref0SkipCenter2DeltaY(p)                              \
  (VME_Input_G1(p, ushort, 0, 9))

#define VME_GET_SICInput_Ref1SkipCenter2DeltaX(p)                              \
  (VME_Input_G1(p, ushort, 0, 10))
#define VME_GET_SICInput_Ref1SkipCenter2DeltaY(p)                              \
  (VME_Input_G1(p, ushort, 0, 11))

#define VME_GET_SICInput_Ref0SkipCenter3DeltaX(p)                              \
  (VME_Input_G1(p, ushort, 0, 12))
#define VME_GET_SICInput_Ref0SkipCenter3DeltaY(p)                              \
  (VME_Input_G1(p, ushort, 0, 13))

#define VME_GET_SICInput_Ref1SkipCenter3DeltaX(p)                              \
  (VME_Input_G1(p, ushort, 0, 14))
#define VME_GET_SICInput_Ref1SkipCenter3DeltaY(p)                              \
  (VME_Input_G1(p, ushort, 0, 15))

#define VME_GET_SICInput_Intra4x4ModeMask(p) (VME_Input_G1(p, uchar, 1, 0))

#define VME_GET_SICInput_Intra8x8ModeMask(p) (VME_Input_G1(p, uchar, 1, 2))

#define VME_GET_SICInput_IntraModeMask(p) (VME_Input_G1(p, uchar, 1, 4))

// 2-bit field with the following definition
// 00: Luma + Chroma enabled
// 01: Luma only
// 1X: Intra disabled
#define VME_GET_SICInput_IntraComputeType(p)                                   \
  (VME_Input_G1(p, uchar, 1, 5) & 0x3)

#define VME_GET_SICInput_CornerNeighborPixel0(p) (VME_Input_G1(p, uchar, 1, 7))

#define VME_GET_SICInput_NeighborPixelLum0(p) (VME_Input_G1(p, uchar, 1, 8))
#define VME_GET_SICInput_NeighborPixelLum1(p) (VME_Input_G1(p, uchar, 1, 9))
#define VME_GET_SICInput_NeighborPixelLum2(p) (VME_Input_G1(p, uchar, 1, 10))
#define VME_GET_SICInput_NeighborPixelLum3(p) (VME_Input_G1(p, uchar, 1, 11))
#define VME_GET_SICInput_NeighborPixelLum4(p) (VME_Input_G1(p, uchar, 1, 12))
#define VME_GET_SICInput_NeighborPixelLum5(p) (VME_Input_G1(p, uchar, 1, 13))
#define VME_GET_SICInput_NeighborPixelLum6(p) (VME_Input_G1(p, uchar, 1, 14))
#define VME_GET_SICInput_NeighborPixelLum7(p) (VME_Input_G1(p, uchar, 1, 15))
#define VME_GET_SICInput_NeighborPixelLum8(p) (VME_Input_G1(p, uchar, 1, 16))
#define VME_GET_SICInput_NeighborPixelLum9(p) (VME_Input_G1(p, uchar, 1, 17))
#define VME_GET_SICInput_NeighborPixelLum10(p) (VME_Input_G1(p, uchar, 1, 18))
#define VME_GET_SICInput_NeighborPixelLum11(p) (VME_Input_G1(p, uchar, 1, 19))
#define VME_GET_SICInput_NeighborPixelLum12(p) (VME_Input_G1(p, uchar, 1, 20))
#define VME_GET_SICInput_NeighborPixelLum13(p) (VME_Input_G1(p, uchar, 1, 21))
#define VME_GET_SICInput_NeighborPixelLum14(p) (VME_Input_G1(p, uchar, 1, 22))
#define VME_GET_SICInput_NeighborPixelLum15(p) (VME_Input_G1(p, uchar, 1, 23))
#define VME_GET_SICInput_NeighborPixelLum16(p) (VME_Input_G1(p, uchar, 1, 24))
#define VME_GET_SICInput_NeighborPixelLum17(p) (VME_Input_G1(p, uchar, 1, 25))
#define VME_GET_SICInput_NeighborPixelLum18(p) (VME_Input_G1(p, uchar, 1, 26))
#define VME_GET_SICInput_NeighborPixelLum19(p) (VME_Input_G1(p, uchar, 1, 27))
#define VME_GET_SICInput_NeighborPixelLum20(p) (VME_Input_G1(p, uchar, 1, 28))
#define VME_GET_SICInput_NeighborPixelLum21(p) (VME_Input_G1(p, uchar, 1, 29))
#define VME_GET_SICInput_NeighborPixelLum22(p) (VME_Input_G1(p, uchar, 1, 30))
#define VME_GET_SICInput_NeighborPixelLum23(p) (VME_Input_G1(p, uchar, 1, 31))
#define VME_GET_SICInput_NeighborPixelLum24(p) (VME_Input_G1(p, uchar, 2, 0))
#define VME_GET_SICInput_NeighborPixelLum25(p) (VME_Input_G1(p, uchar, 2, 1))
#define VME_GET_SICInput_NeighborPixelLum26(p) (VME_Input_G1(p, uchar, 2, 2))
#define VME_GET_SICInput_NeighborPixelLum27(p) (VME_Input_G1(p, uchar, 2, 3))
#define VME_GET_SICInput_NeighborPixelLum28(p) (VME_Input_G1(p, uchar, 2, 4))
#define VME_GET_SICInput_NeighborPixelLum29(p) (VME_Input_G1(p, uchar, 2, 5))
#define VME_GET_SICInput_NeighborPixelLum30(p) (VME_Input_G1(p, uchar, 2, 6))
#define VME_GET_SICInput_NeighborPixelLum31(p) (VME_Input_G1(p, uchar, 2, 7))
#define VME_GET_SICInput_NeighborPixelLum32(p) (VME_Input_G1(p, uchar, 2, 8))
#define VME_GET_SICInput_NeighborPixelLum33(p) (VME_Input_G1(p, uchar, 2, 9))
#define VME_GET_SICInput_NeighborPixelLum34(p) (VME_Input_G1(p, uchar, 2, 10))
#define VME_GET_SICInput_NeighborPixelLum35(p) (VME_Input_G1(p, uchar, 2, 11))
#define VME_GET_SICInput_NeighborPixelLum36(p) (VME_Input_G1(p, uchar, 2, 12))
#define VME_GET_SICInput_NeighborPixelLum37(p) (VME_Input_G1(p, uchar, 2, 13))
#define VME_GET_SICInput_NeighborPixelLum38(p) (VME_Input_G1(p, uchar, 2, 14))

#define VME_GET_SICInput_CornerNeighborPixel1(p) (VME_Input_G1(p, uchar, 2, 15))

#define VME_GET_SICInput_LeftModes(p) (VME_Input_G1(p, ushort, 2, 8))

#define VME_GET_SICInput_TopModes(p) (VME_Input_G1(p, ushort, 2, 9))

// Format = U8 pair
#define VME_GET_SICInput_CornerNeighborPixelChroma(p)                          \
  (VME_Input_G1(p, ushort, 2, 10))

#if 0
// Format = U4U4
#define VME_GET_SICInput_PenaltyIntra4x4NonDC(p) (VME_Input_G1(p, uchar, 2, 28))
// Format = U2
#define VME_GET_SICInput_ScaleFactorIntra8x8NonDC(p)                           \
  (VME_Input_G1(p, uchar, 2, 29) & 0x3)
// Format = U2
#define VME_GET_SICInput_ScaleFactorIntra16x16NonDC(p)                         \
  ((VME_Input_G1(p, uchar, 2, 29) & 0xC) >> 2)
#else
// BSPEC Update Rev1.6
// Format = U8
#define VME_GET_SICInput_PenaltyIntra4x4NonDC(p) (VME_Input_G1(p, uchar, 2, 30))
// Format = U8
#define VME_GET_SICInput_PenaltyIntra8x8NonDC(p) (VME_Input_G1(p, uchar, 2, 29))
// Format = U8
#define VME_GET_SICInput_PenaltyIntra16x16NonDC(p)                             \
  (VME_Input_G1(p, uchar, 2, 28))
#endif

#define VME_GET_SICInput_NeighborPixelChroma0(p) (VME_Input_G1(p, uchar, 3, 0))
#define VME_GET_SICInput_NeighborPixelChroma1(p) (VME_Input_G1(p, uchar, 3, 1))
#define VME_GET_SICInput_NeighborPixelChroma2(p) (VME_Input_G1(p, uchar, 3, 2))
#define VME_GET_SICInput_NeighborPixelChroma3(p) (VME_Input_G1(p, uchar, 3, 3))

#define VME_GET_SICInput_NeighborPixelChroma4(p) (VME_Input_G1(p, uchar, 3, 4))
#define VME_GET_SICInput_NeighborPixelChroma5(p) (VME_Input_G1(p, uchar, 3, 5))
#define VME_GET_SICInput_NeighborPixelChroma6(p) (VME_Input_G1(p, uchar, 3, 6))
#define VME_GET_SICInput_NeighborPixelChroma7(p) (VME_Input_G1(p, uchar, 3, 7))

#define VME_GET_SICInput_NeighborPixelChroma8(p) (VME_Input_G1(p, uchar, 3, 8))
#define VME_GET_SICInput_NeighborPixelChroma9(p) (VME_Input_G1(p, uchar, 3, 9))
#define VME_GET_SICInput_NeighborPixelChroma10(p)                              \
  (VME_Input_G1(p, uchar, 3, 10))
#define VME_GET_SICInput_NeighborPixelChroma11(p)                              \
  (VME_Input_G1(p, uchar, 3, 11))

#define VME_GET_SICInput_NeighborPixelChroma12(p)                              \
  (VME_Input_G1(p, uchar, 3, 12))
#define VME_GET_SICInput_NeighborPixelChroma13(p)                              \
  (VME_Input_G1(p, uchar, 3, 13))
#define VME_GET_SICInput_NeighborPixelChroma14(p)                              \
  (VME_Input_G1(p, uchar, 3, 14))
#define VME_GET_SICInput_NeighborPixelChroma15(p)                              \
  (VME_Input_G1(p, uchar, 3, 15))

#define VME_GET_SICInput_NeighborPixelChroma16(p)                              \
  (VME_Input_G1(p, uchar, 3, 16))
#define VME_GET_SICInput_NeighborPixelChroma17(p)                              \
  (VME_Input_G1(p, uchar, 3, 17))
#define VME_GET_SICInput_NeighborPixelChroma18(p)                              \
  (VME_Input_G1(p, uchar, 3, 18))
#define VME_GET_SICInput_NeighborPixelChroma19(p)                              \
  (VME_Input_G1(p, uchar, 3, 19))

#define VME_GET_SICInput_NeighborPixelChroma20(p)                              \
  (VME_Input_G1(p, uchar, 3, 20))
#define VME_GET_SICInput_NeighborPixelChroma21(p)                              \
  (VME_Input_G1(p, uchar, 3, 21))
#define VME_GET_SICInput_NeighborPixelChroma22(p)                              \
  (VME_Input_G1(p, uchar, 3, 22))
#define VME_GET_SICInput_NeighborPixelChroma23(p)                              \
  (VME_Input_G1(p, uchar, 3, 23))

#define VME_GET_SICInput_NeighborPixelChroma24(p)                              \
  (VME_Input_G1(p, uchar, 3, 24))
#define VME_GET_SICInput_NeighborPixelChroma25(p)                              \
  (VME_Input_G1(p, uchar, 3, 25))
#define VME_GET_SICInput_NeighborPixelChroma26(p)                              \
  (VME_Input_G1(p, uchar, 3, 26))
#define VME_GET_SICInput_NeighborPixelChroma27(p)                              \
  (VME_Input_G1(p, uchar, 3, 27))

#define VME_GET_SICInput_NeighborPixelChroma28(p)                              \
  (VME_Input_G1(p, uchar, 3, 28))
#define VME_GET_SICInput_NeighborPixelChroma29(p)                              \
  (VME_Input_G1(p, uchar, 3, 29))
#define VME_GET_SICInput_NeighborPixelChroma30(p)                              \
  (VME_Input_G1(p, uchar, 3, 30))
#define VME_GET_SICInput_NeighborPixelChroma31(p)                              \
  (VME_Input_G1(p, uchar, 3, 31))

#define VME_GET_FBRInput_Ref0SubBlockX0(p) (VME_Input_G1(p, ushort, 0, 0))
#define VME_GET_FBRInput_Ref0SubBlockY0(p) (VME_Input_G1(p, ushort, 0, 1))
#define VME_GET_FBRInput_Ref1SubBlockX0(p) (VME_Input_G1(p, ushort, 0, 2))
#define VME_GET_FBRInput_Ref1SubBlockY0(p) (VME_Input_G1(p, ushort, 0, 3))

#define VME_GET_FBRInput_Ref0SubBlockX1(p) (VME_Input_G1(p, ushort, 0, 4))
#define VME_GET_FBRInput_Ref0SubBlockY1(p) (VME_Input_G1(p, ushort, 0, 5))
#define VME_GET_FBRInput_Ref1SubBlockX1(p) (VME_Input_G1(p, ushort, 0, 6))
#define VME_GET_FBRInput_Ref1SubBlockY1(p) (VME_Input_G1(p, ushort, 0, 7))

#define VME_GET_FBRInput_Ref0SubBlockX2(p) (VME_Input_G1(p, ushort, 0, 8))
#define VME_GET_FBRInput_Ref0SubBlockY2(p) (VME_Input_G1(p, ushort, 0, 9))
#define VME_GET_FBRInput_Ref1SubBlockX2(p) (VME_Input_G1(p, ushort, 0, 10))
#define VME_GET_FBRInput_Ref1SubBlockY2(p) (VME_Input_G1(p, ushort, 0, 11))

#define VME_GET_FBRInput_Ref0SubBlockX3(p) (VME_Input_G1(p, ushort, 0, 12))
#define VME_GET_FBRInput_Ref0SubBlockY3(p) (VME_Input_G1(p, ushort, 0, 13))
#define VME_GET_FBRInput_Ref1SubBlockX3(p) (VME_Input_G1(p, ushort, 0, 14))
#define VME_GET_FBRInput_Ref1SubBlockY3(p) (VME_Input_G1(p, ushort, 0, 15))

#define VME_GET_FBRInput_Ref0SubBlockX4(p) (VME_Input_G1(p, ushort, 1, 0))
#define VME_GET_FBRInput_Ref0SubBlockY4(p) (VME_Input_G1(p, ushort, 1, 1))
#define VME_GET_FBRInput_Ref1SubBlockX4(p) (VME_Input_G1(p, ushort, 1, 2))
#define VME_GET_FBRInput_Ref1SubBlockY4(p) (VME_Input_G1(p, ushort, 1, 3))

#define VME_GET_FBRInput_Ref0SubBlockX5(p) (VME_Input_G1(p, ushort, 1, 4))
#define VME_GET_FBRInput_Ref0SubBlockY5(p) (VME_Input_G1(p, ushort, 1, 5))
#define VME_GET_FBRInput_Ref1SubBlockX5(p) (VME_Input_G1(p, ushort, 1, 6))
#define VME_GET_FBRInput_Ref1SubBlockY5(p) (VME_Input_G1(p, ushort, 1, 7))

#define VME_GET_FBRInput_Ref0SubBlockX6(p) (VME_Input_G1(p, ushort, 1, 8))
#define VME_GET_FBRInput_Ref0SubBlockY6(p) (VME_Input_G1(p, ushort, 1, 9))
#define VME_GET_FBRInput_Ref1SubBlockX6(p) (VME_Input_G1(p, ushort, 1, 10))
#define VME_GET_FBRInput_Ref1SubBlockY6(p) (VME_Input_G1(p, ushort, 1, 11))

#define VME_GET_FBRInput_Ref0SubBlockX7(p) (VME_Input_G1(p, ushort, 1, 12))
#define VME_GET_FBRInput_Ref0SubBlockY7(p) (VME_Input_G1(p, ushort, 1, 13))
#define VME_GET_FBRInput_Ref1SubBlockX7(p) (VME_Input_G1(p, ushort, 1, 14))
#define VME_GET_FBRInput_Ref1SubBlockY7(p) (VME_Input_G1(p, ushort, 1, 15))

#define VME_GET_FBRInput_Ref0SubBlockX8(p) (VME_Input_G1(p, ushort, 2, 0))
#define VME_GET_FBRInput_Ref0SubBlockY8(p) (VME_Input_G1(p, ushort, 2, 1))
#define VME_GET_FBRInput_Ref1SubBlockX8(p) (VME_Input_G1(p, ushort, 2, 2))
#define VME_GET_FBRInput_Ref1SubBlockY8(p) (VME_Input_G1(p, ushort, 2, 3))

#define VME_GET_FBRInput_Ref0SubBlockX9(p) (VME_Input_G1(p, ushort, 2, 4))
#define VME_GET_FBRInput_Ref0SubBlockY9(p) (VME_Input_G1(p, ushort, 2, 5))
#define VME_GET_FBRInput_Ref1SubBlockX9(p) (VME_Input_G1(p, ushort, 2, 6))
#define VME_GET_FBRInput_Ref1SubBlockY9(p) (VME_Input_G1(p, ushort, 2, 7))

#define VME_GET_FBRInput_Ref0SubBlockX10(p) (VME_Input_G1(p, ushort, 2, 8))
#define VME_GET_FBRInput_Ref0SubBlockY10(p) (VME_Input_G1(p, ushort, 2, 9))
#define VME_GET_FBRInput_Ref1SubBlockX10(p) (VME_Input_G1(p, ushort, 2, 10))
#define VME_GET_FBRInput_Ref1SubBlockY10(p) (VME_Input_G1(p, ushort, 2, 11))

#define VME_GET_FBRInput_Ref0SubBlockX11(p) (VME_Input_G1(p, ushort, 2, 12))
#define VME_GET_FBRInput_Ref0SubBlockY11(p) (VME_Input_G1(p, ushort, 2, 13))
#define VME_GET_FBRInput_Ref1SubBlockX11(p) (VME_Input_G1(p, ushort, 2, 14))
#define VME_GET_FBRInput_Ref1SubBlockY11(p) (VME_Input_G1(p, ushort, 2, 15))

#define VME_GET_FBRInput_Ref0SubBlockX12(p) (VME_Input_G1(p, ushort, 3, 0))
#define VME_GET_FBRInput_Ref0SubBlockY12(p) (VME_Input_G1(p, ushort, 3, 1))
#define VME_GET_FBRInput_Ref1SubBlockX12(p) (VME_Input_G1(p, ushort, 3, 2))
#define VME_GET_FBRInput_Ref1SubBlockY12(p) (VME_Input_G1(p, ushort, 3, 3))

#define VME_GET_FBRInput_Ref0SubBlockX13(p) (VME_Input_G1(p, ushort, 3, 4))
#define VME_GET_FBRInput_Ref0SubBlockY13(p) (VME_Input_G1(p, ushort, 3, 5))
#define VME_GET_FBRInput_Ref1SubBlockX13(p) (VME_Input_G1(p, ushort, 3, 6))
#define VME_GET_FBRInput_Ref1SubBlockY13(p) (VME_Input_G1(p, ushort, 3, 7))

#define VME_GET_FBRInput_Ref0SubBlockX14(p) (VME_Input_G1(p, ushort, 3, 8))
#define VME_GET_FBRInput_Ref0SubBlockY14(p) (VME_Input_G1(p, ushort, 3, 9))
#define VME_GET_FBRInput_Ref1SubBlockX14(p) (VME_Input_G1(p, ushort, 3, 10))
#define VME_GET_FBRInput_Ref1SubBlockY14(p) (VME_Input_G1(p, ushort, 3, 11))

#define VME_GET_FBRInput_Ref0SubBlockX15(p) (VME_Input_G1(p, ushort, 3, 12))
#define VME_GET_FBRInput_Ref0SubBlockY15(p) (VME_Input_G1(p, ushort, 3, 13))
#define VME_GET_FBRInput_Ref1SubBlockX15(p) (VME_Input_G1(p, ushort, 3, 14))
#define VME_GET_FBRInput_Ref1SubBlockY15(p) (VME_Input_G1(p, ushort, 3, 15))

#define VME_GET_IDMInput_SrcMBPixelMaskRow0(p) (VME_Input_G1(p, ushort, 0, 0))
#define VME_GET_IDMInput_SrcMBPixelMaskRow1(p) (VME_Input_G1(p, ushort, 0, 1))
#define VME_GET_IDMInput_SrcMBPixelMaskRow2(p) (VME_Input_G1(p, ushort, 0, 2))
#define VME_GET_IDMInput_SrcMBPixelMaskRow3(p) (VME_Input_G1(p, ushort, 0, 3))
#define VME_GET_IDMInput_SrcMBPixelMaskRow4(p) (VME_Input_G1(p, ushort, 0, 4))
#define VME_GET_IDMInput_SrcMBPixelMaskRow5(p) (VME_Input_G1(p, ushort, 0, 5))
#define VME_GET_IDMInput_SrcMBPixelMaskRow6(p) (VME_Input_G1(p, ushort, 0, 6))
#define VME_GET_IDMInput_SrcMBPixelMaskRow7(p) (VME_Input_G1(p, ushort, 0, 7))
#define VME_GET_IDMInput_SrcMBPixelMaskRow8(p) (VME_Input_G1(p, ushort, 0, 8))
#define VME_GET_IDMInput_SrcMBPixelMaskRow9(p) (VME_Input_G1(p, ushort, 0, 9))
#define VME_GET_IDMInput_SrcMBPixelMaskRow10(p) (VME_Input_G1(p, ushort, 0, 10))
#define VME_GET_IDMInput_SrcMBPixelMaskRow11(p) (VME_Input_G1(p, ushort, 0, 11))
#define VME_GET_IDMInput_SrcMBPixelMaskRow12(p) (VME_Input_G1(p, ushort, 0, 12))
#define VME_GET_IDMInput_SrcMBPixelMaskRow13(p) (VME_Input_G1(p, ushort, 0, 13))
#define VME_GET_IDMInput_SrcMBPixelMaskRow14(p) (VME_Input_G1(p, ushort, 0, 14))
#define VME_GET_IDMInput_SrcMBPixelMaskRow15(p) (VME_Input_G1(p, ushort, 0, 15))

// Output

#define MODE_INTER_16X16 0x00
#define MODE_INTER_16X8 0x01
#define MODE_INTER_8X16 0x02
#define MODE_INTER_8X8 0x03
#define MODE_INTER_MINOR 0x03
#define VME_GET_UNIOutput_InterMbMode(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 0) & 0x3)

#define MODE_INTRA_16X16 0x00
#define MODE_INTRA_8X8 0x10
#define MODE_INTRA_4X4 0x20
#define MODE_INTRA_PCM 0x30
#define VME_GET_UNIOutput_IntraMbMode(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 0) & 0x30)

#define MODE_FIELD_MB_POLARITY 0x80
#define VME_GET_UNIOutput_FieldMbPolarityFlag(p, v)                            \
  (v = VME_Output_S1(p, uchar, 0, 0) & 0x80)

#define VME_GET_UNIOutput_InterMbType(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 1) & 0x1F)

#define VME_GET_UNIOutput_FieldMbFlag(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 1) & 0x40)

#define VME_GET_UNIOutput_IntraMbType(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 2) & 0x1F)

#define VME_GET_UNIOutput_IntraMbType(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 2) & 0x1F)

#define VME_GET_UNIOutput_MvQuantity(p, v)                                     \
  (v = VME_Output_S1(p, uchar, 0, 3) & 0x1F)

// Format = U16
#define VME_GET_UNIOutput_BestInterDistortion(p, v)                            \
  (v = VME_Output_S1(p, ushort, 0, 4))

// Format = U16
#define VME_GET_UNIOutput_SkipRawDistortion(p, v)                              \
  (v = VME_Output_S1(p, ushort, 0, 5))

// Format = U16
#define VME_GET_UNIOutput_BestIntraDistortion(p, v)                            \
  (v = VME_Output_S1(p, ushort, 0, 6))

// Format = U16
#define VME_GET_UNIOutput_BestChromaIntraDistortion(p, v)                      \
  (v = VME_Output_S1(p, ushort, 0, 7))

#define VME_GET_UNIOutput_LumaIntraPredMode0(p, v)                             \
  (v = VME_Output_S1(p, ushort, 0, 8))
#define VME_GET_UNIOutput_LumaIntraPredMode1(p, v)                             \
  (v = VME_Output_S1(p, ushort, 0, 9))
#define VME_GET_UNIOutput_LumaIntraPredMode2(p, v)                             \
  (v = VME_Output_S1(p, ushort, 0, 10))
#define VME_GET_UNIOutput_LumaIntraPredMode3(p, v)                             \
  (v = VME_Output_S1(p, ushort, 0, 11))

// 8-bit field with the following definition:
// 7    Reserved : MBZ (for IntraPredAvailFlagF - F (pixel[-1,7] available for
// MbAff)
// 6    Reserved : MBZ (for IntraPredAvailFlagA/E - A (left neighbor top half
// for MbAff)
// 5    IntraPredAvailFlagE/A - A (Left neighbor or Left bottom half)
// 4    IntraPredAvailFlagB - B (Upper neighbor)
// 3    IntraPredAvailFlagC - C (Upper left neighbor)
// 2    IntraPredAvailFlagD - D (Upper right neighbor)
// 1:0  ChromaIntraPredMode
#define VME_GET_UNIOutput_MbIntraStruct(p, v)                                  \
  (v = VME_Output_S1(p, uchar, 0, 24))

// 8-bit field with the following definition:
// Bits [1:0]: SubMbShape[0]
// Bits [3:2]: SubMbShape[1]
// Bits [5:4]: SubMbShape[2]
// Bits [7:6]: SubMbShape[3]
#define VME_GET_UNIOutput_SubMbShape(p, v) (v = VME_Output_S1(p, uchar, 0, 25))

// 8-bit field with the following definition:
// Bits [1:0]: SubMbPredMode[0]
// Bits [3:2]: SubMbPredMode[1]
// Bits [5:4]: SubMbPredMode[2]
// Bits [7:6]: SubMbPredMode[3]
#define VME_GET_UNIOutput_SubMbPredMode(p, v)                                  \
  (v = VME_Output_S1(p, uchar, 0, 26))

// v is a 4x16 short type matrix that stores the motion vectors as follows:
// MVa[0].x, MVa[0].y, MVb[0].x, MVb[0].x
// MVa[1].x, MVa[1].y, MVb[1].x, MVb[1].x
// ...
// MVa[15].x, MVa[15].y, MVb[15].x, MVb[15].x
#define VME_GET_UNIOutput_Mvs(p, v)                                            \
  (v = p.format<short, 7, 32 / sizeof(short)>().select<4, 1, 16, 1>(1, 0))

// v is a 1x16 short type matrix that stores the inter distortions as follows:
// InterDistortion[0], InterDistortion[1], ..., InterDistortion[15]
#define VME_GET_UNIOutput_InterDistortion(p, v)                                \
  (v = p.format<short, 7, 32 / sizeof(short)>().select<1, 1, 16, 1>(5, 0))

// Format = U4
#define VME_GET_UNIOutput_FwdBlk0RefID(p, v)                                   \
  (v = VME_Output_S1(p, uchar, 6, 0) & 0xF)
#define VME_GET_UNIOutput_BwdBlk0RefID(p, v)                                   \
  (v = (VME_Output_S1(p, uchar, 6, 0) >> 4) & 0xF)
#define VME_GET_UNIOutput_FwdBlk1RefID(p, v)                                   \
  (v = VME_Output_S1(p, uchar, 6, 1) & 0xF)
#define VME_GET_UNIOutput_BwdBlk1RefID(p, v)                                   \
  (v = (VME_Output_S1(p, uchar, 6, 1) >> 4) & 0xF)
#define VME_GET_UNIOutput_FwdBlk2RefID(p, v)                                   \
  (v = VME_Output_S1(p, uchar, 6, 2) & 0xF)
#define VME_GET_UNIOutput_BwdBlk2RefID(p, v)                                   \
  (v = (VME_Output_S1(p, uchar, 6, 2) >> 4) & 0xF)
#define VME_GET_UNIOutput_FwdBlk3RefID(p, v)                                   \
  (v = VME_Output_S1(p, uchar, 6, 3) & 0xF)
#define VME_GET_UNIOutput_BwdBlk3RefID(p, v)                                   \
  (v = (VME_Output_S1(p, uchar, 6, 3) >> 4) & 0xF)

#define VME_GET_UNIOutput_Blk0LumaNZC(p, v) (v = VME_Output_S1(p, uchar, 6, 4))
#define VME_GET_UNIOutput_Blk1LumaNZC(p, v) (v = VME_Output_S1(p, uchar, 6, 5))
#define VME_GET_UNIOutput_Blk2LumaNZC(p, v) (v = VME_Output_S1(p, uchar, 6, 6))
#define VME_GET_UNIOutput_Blk3LumaNZC(p, v) (v = VME_Output_S1(p, uchar, 6, 7))

// Format = U8
#define VME_GET_UNIOutput_Blk0ChromaCbNZC(p, v)                                \
  (v = VME_Output_S1(p, uchar, 6, 16))
#define VME_GET_UNIOutput_Blk0ChromaCrNZC(p, v)                                \
  (v = VME_Output_S1(p, uchar, 6, 17))

#define VME_GET_UNIOutput_SumInterDistL0(p, v)                                 \
  (v = VME_Output_S1(p, uint, 6, 6) & 0x3FFFFFF)
#define VME_GET_UNIOutput_SumInterDistL1(p, v)                                 \
  (v = VME_Output_S1(p, ushort, 6, 9) +                                        \
       ((VME_Output_S1(p, ushort, 0, 3) & 0x3FF) << 16))

#define VME_GET_UNIOutput_MaxRef0InterDist(p, v)                               \
  (v = VME_Output_S1(p, ushort, 6, 14))
#define VME_GET_UNIOutput_MaxRef1InterDist(p, v)                               \
  (v = VME_Output_S1(p, ushort, 6, 15))

#define MODE_INTER_16X16 0x00
#define MODE_INTER_16X8 0x01
#define MODE_INTER_8X16 0x02
#define MODE_INTER_8X8 0x03
#define MODE_INTER_MINOR 0x03
#define VME_GET_IMEOutput_InterMbMode(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 0) & 0x3)

#define MODE_INTRA_16X16 0x00
#define MODE_INTRA_8X8 0x10
#define MODE_INTRA_4X4 0x20
#define MODE_INTRA_PCM 0x30
#define VME_GET_IMEOutput_IntraMbMode(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 0) & 0x30)

#define MODE_FIELD_MB_POLARITY 0x80
#define VME_GET_IMEOutput_FieldMbPolarityFlag(p, v)                            \
  (v = VME_Output_S1(p, uchar, 0, 0) & 0x80)

#define VME_GET_IMEOutput_InterMbType(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 1) & 0x1F)

#define VME_GET_IMEOutput_FieldMbFlag(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 1) & 0x40)

#define VME_GET_IMEOutput_IntraMbType(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 2) & 0x1F)

#define VME_GET_IMEOutput_IntraMbType(p, v)                                    \
  (v = VME_Output_S1(p, uchar, 0, 2) & 0x1F)

#define VME_GET_IMEOutput_MvQuantity(p, v)                                     \
  (v = VME_Output_S1(p, uchar, 0, 3) & 0x1F)

// Format = U16
#define VME_GET_IMEOutput_BestInterDistortion(p, v)                            \
  (v = VME_Output_S1(p, ushort, 0, 4))

// Format = U16
#define VME_GET_IMEOutput_SkipRawDistortion(p, v)                              \
  (v = VME_Output_S1(p, ushort, 0, 5))

// Format = U16
#define VME_GET_IMEOutput_BestIntraDistortion(p, v)                            \
  (v = VME_Output_S1(p, ushort, 0, 6))

// Format = U16
#define VME_GET_IMEOutput_BestChromaIntraDistortion(p, v)                      \
  (v = VME_Output_S1(p, ushort, 0, 7))

#define VME_GET_IMEOutput_LumaIntraPredMode0(p, v)                             \
  (v = VME_Output_S1(p, ushort, 0, 8))
#define VME_GET_IMEOutput_LumaIntraPredMode1(p, v)                             \
  (v = VME_Output_S1(p, ushort, 0, 9))
#define VME_GET_IMEOutput_LumaIntraPredMode2(p, v)                             \
  (v = VME_Output_S1(p, ushort, 0, 10))
#define VME_GET_IMEOutput_LumaIntraPredMode3(p, v)                             \
  (v = VME_Output_S1(p, ushort, 0, 11))

// 8-bit field with the following definition:
// 7    Reserved : MBZ (for IntraPredAvailFlagF - F (pixel[-1,7] available for
// MbAff)
// 6    Reserved : MBZ (for IntraPredAvailFlagA/E - A (left neighbor top half
// for MbAff)
// 5    IntraPredAvailFlagE/A - A (Left neighbor or Left bottom half)
// 4    IntraPredAvailFlagB - B (Upper neighbor)
// 3    IntraPredAvailFlagC - C (Upper left neighbor)
// 2    IntraPredAvailFlagD - D (Upper right neighbor)
// 1:0  ChromaIntraPredMode
#define VME_GET_IMEOutput_MbIntraStruct(p, v)                                  \
  (v = VME_Output_S1(p, uchar, 0, 24))

// 8-bit field with the following definition:
// Bits [1:0]: SubMbShape[0]
// Bits [3:2]: SubMbShape[1]
// Bits [5:4]: SubMbShape[2]
// Bits [7:6]: SubMbShape[3]
#define VME_GET_IMEOutput_SubMbShape(p, v) (v = VME_Output_S1(p, uchar, 0, 25))

// 8-bit field with the following definition:
// Bits [1:0]: SubMbPredMode[0]
// Bits [3:2]: SubMbPredMode[1]
// Bits [5:4]: SubMbPredMode[2]
// Bits [7:6]: SubMbPredMode[3]
#define VME_GET_IMEOutput_SubMbPredMode(p, v)                                  \
  (v = VME_Output_S1(p, uchar, 0, 26))

// v is a 4x16 short type matrix that stores the motion vectors as follows:
// MVa[0].x, MVa[0].y, MVb[0].x, MVb[0].x
// MVa[1].x, MVa[1].y, MVb[1].x, MVb[1].x
// ...
// MVa[15].x, MVa[15].y, MVb[15].x, MVb[15].x
#define VME_GET_IMEOutput_Mvs(p, v)                                            \
  (v = p.format<short, 7, 32 / sizeof(short)>().select<4, 1, 16, 1>(1, 0))

// v is a 1x16 short type matrix that stores the inter distortions as follows:
// InterDistortion[0], InterDistortion[1], ..., InterDistortion[15]
#define VME_GET_IMEOutput_InterDistortion(p, v)                                \
  (v = p.format<short, 7, 32 / sizeof(short)>().select<1, 1, 16, 1>(5, 0))

// Format = U4
#define VME_GET_IMEOutput_FwdBlk0RefID(p, v)                                   \
  (v = VME_Output_S1(p, uchar, 6, 0) & 0xF)
#define VME_GET_IMEOutput_BwdBlk0RefID(p, v)                                   \
  (v = (VME_Output_S1(p, uchar, 6, 0) >> 4) & 0xF)
#define VME_GET_IMEOutput_FwdBlk1RefID(p, v)                                   \
  (v = VME_Output_S1(p, uchar, 6, 1) & 0xF)
#define VME_GET_IMEOutput_BwdBlk1RefID(p, v)                                   \
  (v = (VME_Output_S1(p, uchar, 6, 1) >> 4) & 0xF)
#define VME_GET_IMEOutput_FwdBlk2RefID(p, v)                                   \
  (v = VME_Output_S1(p, uchar, 6, 2) & 0xF)
#define VME_GET_IMEOutput_BwdBlk2RefID(p, v)                                   \
  (v = (VME_Output_S1(p, uchar, 6, 2) >> 4) & 0xF)
#define VME_GET_IMEOutput_FwdBlk3RefID(p, v)                                   \
  (v = VME_Output_S1(p, uchar, 6, 3) & 0xF)
#define VME_GET_IMEOutput_BwdBlk3RefID(p, v)                                   \
  (v = (VME_Output_S1(p, uchar, 6, 3) >> 4) & 0xF)

#define VME_GET_IMEOutput_Blk0LumaNZC(p, v) (v = VME_Output_S1(p, uchar, 6, 4))
#define VME_GET_IMEOutput_Blk1LumaNZC(p, v) (v = VME_Output_S1(p, uchar, 6, 5))
#define VME_GET_IMEOutput_Blk2LumaNZC(p, v) (v = VME_Output_S1(p, uchar, 6, 6))
#define VME_GET_IMEOutput_Blk3LumaNZC(p, v) (v = VME_Output_S1(p, uchar, 6, 7))

// Format = U8
#define VME_GET_IMEOutput_Blk0ChromaCbNZC(p, v)                                \
  (v = VME_Output_S1(p, uchar, 6, 16))
#define VME_GET_IMEOutput_Blk0ChromaCrNZC(p, v)                                \
  (v = VME_Output_S1(p, uchar, 6, 17))

// Format = U16
#define VME_GET_IMEOutput_Rec0_16x8_0Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 7, 0))
#define VME_GET_IMEOutput_Rec0_16x8_1Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 7, 1))
#define VME_GET_IMEOutput_Rec0_8x16_0Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 7, 2))
#define VME_GET_IMEOutput_Rec0_8x16_1Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 7, 3))
#define VME_GET_IMEOutput_Rec0_8x8_0Distortion(p, v)                           \
  (v = VME_Output_S1(p, ushort, 7, 4))
#define VME_GET_IMEOutput_Rec0_8x8_1Distortion(p, v)                           \
  (v = VME_Output_S1(p, ushort, 7, 5))
#define VME_GET_IMEOutput_Rec0_8x8_2Distortion(p, v)                           \
  (v = VME_Output_S1(p, ushort, 7, 6))
#define VME_GET_IMEOutput_Rec0_8x8_3Distortion(p, v)                           \
  (v = VME_Output_S1(p, ushort, 7, 7))
#define VME_GET_IMEOutput_Rec0_16x16_Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 7, 8))

// Format = U4
#define VME_GET_IMEOutput_Rec0_16x16_RefID(p, v)                               \
  (v = VME_Output_S1(p, uchar, 7, 18))

// Format = U16
#define VME_GET_IMEOutput_Rec0_16x16_X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 7, 10))
#define VME_GET_IMEOutput_Rec0_16x16_Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 7, 11))

// Format = U4
#define VME_GET_IMEOutput_Rec0_16x8_0RefID(p, v)                               \
  (v = VME_Output_S1(p, uchar, 7, 24) & 0xF)
#define VME_GET_IMEOutput_Rec0_16x8_1RefID(p, v)                               \
  (v = (VME_Output_S1(p, uchar, 7, 24) & 0xF0) >> 4)
#define VME_GET_IMEOutput_Rec0_8x16_0RefID(p, v)                               \
  (v = VME_Output_S1(p, uchar, 7, 25) & 0xF)
#define VME_GET_IMEOutput_Rec0_8x16_1RefID(p, v)                               \
  (v = (VME_Output_S1(p, uchar, 7, 25) & 0xF0) >> 4)
#define VME_GET_IMEOutput_Rec0_8x8_0RefID(p, v)                                \
  (v = VME_Output_S1(p, uchar, 7, 26) & 0xF)
#define VME_GET_IMEOutput_Rec0_8x8_1RefID(p, v)                                \
  (v = (VME_Output_S1(p, uchar, 7, 26) & 0xF0) >> 4)
#define VME_GET_IMEOutput_Rec0_8x8_2RefID(p, v)                                \
  (v = VME_Output_S1(p, uchar, 7, 27) & 0xF)
#define VME_GET_IMEOutput_Rec0_8x8_3RefID(p, v)                                \
  (v = (VME_Output_S1(p, uchar, 7, 27) & 0xF0) >> 4)

// Format = U16
#define VME_GET_IMEOutput_Rec0_16x8_0X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 8, 0))
#define VME_GET_IMEOutput_Rec0_16x8_0Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 8, 1))
#define VME_GET_IMEOutput_Rec0_16x8_1X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 8, 2))
#define VME_GET_IMEOutput_Rec0_16x8_1Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 8, 3))
#define VME_GET_IMEOutput_Rec0_8x16_0X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 8, 4))
#define VME_GET_IMEOutput_Rec0_8x16_0Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 8, 5))
#define VME_GET_IMEOutput_Rec0_8x16_1X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 8, 6))
#define VME_GET_IMEOutput_Rec0_8x16_1Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 8, 7))
#define VME_GET_IMEOutput_Rec0_8x8_0X(p, v) (v = VME_Output_S1(p, ushort, 8, 8))
#define VME_GET_IMEOutput_Rec0_8x8_0Y(p, v) (v = VME_Output_S1(p, ushort, 8, 9))
#define VME_GET_IMEOutput_Rec0_8x8_1X(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 8, 10))
#define VME_GET_IMEOutput_Rec0_8x8_1Y(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 8, 11))
#define VME_GET_IMEOutput_Rec0_8x8_2X(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 8, 12))
#define VME_GET_IMEOutput_Rec0_8x8_2Y(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 8, 13))
#define VME_GET_IMEOutput_Rec0_8x8_3X(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 8, 14))
#define VME_GET_IMEOutput_Rec0_8x8_3Y(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 8, 15))

// Format = U16
#define VME_GET_IMEOutput_Rec1_16x8_0Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 9, 0))
#define VME_GET_IMEOutput_Rec1_16x8_1Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 9, 1))
#define VME_GET_IMEOutput_Rec1_8x16_0Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 9, 2))
#define VME_GET_IMEOutput_Rec1_8x16_1Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 9, 3))
#define VME_GET_IMEOutput_Rec1_8x8_0Distortion(p, v)                           \
  (v = VME_Output_S1(p, ushort, 9, 4))
#define VME_GET_IMEOutput_Rec1_8x8_1Distortion(p, v)                           \
  (v = VME_Output_S1(p, ushort, 9, 5))
#define VME_GET_IMEOutput_Rec1_8x8_2Distortion(p, v)                           \
  (v = VME_Output_S1(p, ushort, 9, 6))
#define VME_GET_IMEOutput_Rec1_8x8_3Distortion(p, v)                           \
  (v = VME_Output_S1(p, ushort, 9, 7))
#define VME_GET_IMEOutput_Rec1_16x16_Distortion(p, v)                          \
  (v = VME_Output_S1(p, ushort, 9, 8))

// Format = U4
#define VME_GET_IMEOutput_Rec1_16x16_RefID(p, v)                               \
  (v = VME_Output_S1(p, uchar, 9, 18))

// Format = U16
#define VME_GET_IMEOutput_Rec1_16x16_X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 9, 10))
#define VME_GET_IMEOutput_Rec1_16x16_Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 9, 11))

// Format = U4
#define VME_GET_IMEOutput_Rec1_16x8_0RefID(p, v)                               \
  (v = VME_Output_S1(p, uchar, 9, 24) & 0xF)
#define VME_GET_IMEOutput_Rec1_16x8_1RefID(p, v)                               \
  (v = (VME_Output_S1(p, uchar, 9, 24) & 0xF0) >> 4)
#define VME_GET_IMEOutput_Rec1_8x16_0RefID(p, v)                               \
  (v = VME_Output_S1(p, uchar, 9, 25) & 0xF)
#define VME_GET_IMEOutput_Rec1_8x16_1RefID(p, v)                               \
  (v = (VME_Output_S1(p, uchar, 9, 25) & 0xF0) >> 4)
#define VME_GET_IMEOutput_Rec1_8x8_0RefID(p, v)                                \
  (v = VME_Output_S1(p, uchar, 9, 26) & 0xF)
#define VME_GET_IMEOutput_Rec1_8x8_1RefID(p, v)                                \
  (v = (VME_Output_S1(p, uchar, 9, 26) & 0xF0) >> 4)
#define VME_GET_IMEOutput_Rec1_8x8_2RefID(p, v)                                \
  (v = VME_Output_S1(p, uchar, 9, 27) & 0xF)
#define VME_GET_IMEOutput_Rec1_8x8_3RefID(p, v)                                \
  (v = (VME_Output_S1(p, uchar, 9, 27) & 0xF0) >> 4)

// Format = U16
#define VME_GET_IMEOutput_Rec1_16x8_0X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 10, 0))
#define VME_GET_IMEOutput_Rec1_16x8_0Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 10, 1))
#define VME_GET_IMEOutput_Rec1_16x8_1X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 10, 2))
#define VME_GET_IMEOutput_Rec1_16x8_1Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 10, 3))
#define VME_GET_IMEOutput_Rec1_8x16_0X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 10, 4))
#define VME_GET_IMEOutput_Rec1_8x16_0Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 10, 5))
#define VME_GET_IMEOutput_Rec1_8x16_1X(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 10, 6))
#define VME_GET_IMEOutput_Rec1_8x16_1Y(p, v)                                   \
  (v = VME_Output_S1(p, ushort, 10, 7))
#define VME_GET_IMEOutput_Rec1_8x8_0X(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 10, 8))
#define VME_GET_IMEOutput_Rec1_8x8_0Y(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 10, 9))
#define VME_GET_IMEOutput_Rec1_8x8_1X(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 10, 10))
#define VME_GET_IMEOutput_Rec1_8x8_1Y(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 10, 11))
#define VME_GET_IMEOutput_Rec1_8x8_2X(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 10, 12))
#define VME_GET_IMEOutput_Rec1_8x8_2Y(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 10, 13))
#define VME_GET_IMEOutput_Rec1_8x8_3X(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 10, 14))
#define VME_GET_IMEOutput_Rec1_8x8_3Y(p, v)                                    \
  (v = VME_Output_S1(p, ushort, 10, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 0, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 0, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 0, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 0, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 0, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 0, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 0, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 0, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 0, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 0, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 0, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 0, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 0, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 0, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 0, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock0_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 0, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 1, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 1, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 1, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 1, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 1, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 1, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 1, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 1, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 1, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 1, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 1, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 1, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 1, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 1, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 1, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock1_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 1, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 2, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 2, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 2, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 2, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 2, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 2, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 2, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 2, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 2, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 2, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 2, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 2, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 2, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 2, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 2, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock2_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 2, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 3, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 3, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 3, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 3, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 3, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 3, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 3, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 3, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 3, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 3, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 3, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 3, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 3, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 3, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 3, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock3_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 3, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 4, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 4, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 4, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 4, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 4, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 4, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 4, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 4, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 4, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 4, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 4, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 4, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 4, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 4, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 4, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock4_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 4, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 5, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 5, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 5, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 5, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 5, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 5, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 5, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 5, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 5, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 5, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 5, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 5, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 5, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 5, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 5, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock5_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 5, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 6, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 6, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 6, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 6, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 6, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 6, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 6, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 6, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 6, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 6, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 6, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 6, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 6, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 6, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 6, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock6_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 6, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 7, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 7, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 7, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 7, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 7, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 7, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 7, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 7, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 7, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 7, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 7, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 7, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 7, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 7, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 7, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock7_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 7, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 8, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 8, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 8, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 8, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 8, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 8, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 8, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 8, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 8, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 8, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 8, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 8, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 8, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 8, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 8, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock8_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 8, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint0(p, v)              \
  (v = VME_Output_S1(p, ushort, 9, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint1(p, v)              \
  (v = VME_Output_S1(p, ushort, 9, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoshort(p, v)             \
  (v = VME_Output_S1(p, ushort, 9, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint3(p, v)              \
  (v = VME_Output_S1(p, ushort, 9, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint4(p, v)              \
  (v = VME_Output_S1(p, ushort, 9, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint5(p, v)              \
  (v = VME_Output_S1(p, ushort, 9, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint6(p, v)              \
  (v = VME_Output_S1(p, ushort, 9, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint7(p, v)              \
  (v = VME_Output_S1(p, ushort, 9, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint8(p, v)              \
  (v = VME_Output_S1(p, ushort, 9, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint9(p, v)              \
  (v = VME_Output_S1(p, ushort, 9, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint10(p, v)             \
  (v = VME_Output_S1(p, ushort, 9, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint11(p, v)             \
  (v = VME_Output_S1(p, ushort, 9, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint12(p, v)             \
  (v = VME_Output_S1(p, ushort, 9, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint13(p, v)             \
  (v = VME_Output_S1(p, ushort, 9, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint14(p, v)             \
  (v = VME_Output_S1(p, ushort, 9, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock9_SearchPoint15(p, v)             \
  (v = VME_Output_S1(p, ushort, 9, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint0(p, v)             \
  (v = VME_Output_S1(p, ushort, 10, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint1(p, v)             \
  (v = VME_Output_S1(p, ushort, 10, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoshort(p, v)            \
  (v = VME_Output_S1(p, ushort, 10, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint3(p, v)             \
  (v = VME_Output_S1(p, ushort, 10, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint4(p, v)             \
  (v = VME_Output_S1(p, ushort, 10, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint5(p, v)             \
  (v = VME_Output_S1(p, ushort, 10, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint6(p, v)             \
  (v = VME_Output_S1(p, ushort, 10, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint7(p, v)             \
  (v = VME_Output_S1(p, ushort, 10, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint8(p, v)             \
  (v = VME_Output_S1(p, ushort, 10, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint9(p, v)             \
  (v = VME_Output_S1(p, ushort, 10, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint10(p, v)            \
  (v = VME_Output_S1(p, ushort, 10, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint11(p, v)            \
  (v = VME_Output_S1(p, ushort, 10, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint12(p, v)            \
  (v = VME_Output_S1(p, ushort, 10, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint13(p, v)            \
  (v = VME_Output_S1(p, ushort, 10, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint14(p, v)            \
  (v = VME_Output_S1(p, ushort, 10, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock10_SearchPoint15(p, v)            \
  (v = VME_Output_S1(p, ushort, 10, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint0(p, v)             \
  (v = VME_Output_S1(p, ushort, 11, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint1(p, v)             \
  (v = VME_Output_S1(p, ushort, 11, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoshort(p, v)            \
  (v = VME_Output_S1(p, ushort, 11, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint3(p, v)             \
  (v = VME_Output_S1(p, ushort, 11, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint4(p, v)             \
  (v = VME_Output_S1(p, ushort, 11, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint5(p, v)             \
  (v = VME_Output_S1(p, ushort, 11, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint6(p, v)             \
  (v = VME_Output_S1(p, ushort, 11, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint7(p, v)             \
  (v = VME_Output_S1(p, ushort, 11, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint8(p, v)             \
  (v = VME_Output_S1(p, ushort, 11, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint9(p, v)             \
  (v = VME_Output_S1(p, ushort, 11, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint10(p, v)            \
  (v = VME_Output_S1(p, ushort, 11, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint11(p, v)            \
  (v = VME_Output_S1(p, ushort, 11, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint12(p, v)            \
  (v = VME_Output_S1(p, ushort, 11, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint13(p, v)            \
  (v = VME_Output_S1(p, ushort, 11, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint14(p, v)            \
  (v = VME_Output_S1(p, ushort, 11, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock11_SearchPoint15(p, v)            \
  (v = VME_Output_S1(p, ushort, 11, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint0(p, v)             \
  (v = VME_Output_S1(p, ushort, 12, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint1(p, v)             \
  (v = VME_Output_S1(p, ushort, 12, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoshort(p, v)            \
  (v = VME_Output_S1(p, ushort, 12, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint3(p, v)             \
  (v = VME_Output_S1(p, ushort, 12, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint4(p, v)             \
  (v = VME_Output_S1(p, ushort, 12, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint5(p, v)             \
  (v = VME_Output_S1(p, ushort, 12, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint6(p, v)             \
  (v = VME_Output_S1(p, ushort, 12, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint7(p, v)             \
  (v = VME_Output_S1(p, ushort, 12, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint8(p, v)             \
  (v = VME_Output_S1(p, ushort, 12, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint9(p, v)             \
  (v = VME_Output_S1(p, ushort, 12, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint10(p, v)            \
  (v = VME_Output_S1(p, ushort, 12, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint11(p, v)            \
  (v = VME_Output_S1(p, ushort, 12, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint12(p, v)            \
  (v = VME_Output_S1(p, ushort, 12, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint13(p, v)            \
  (v = VME_Output_S1(p, ushort, 12, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint14(p, v)            \
  (v = VME_Output_S1(p, ushort, 12, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock12_SearchPoint15(p, v)            \
  (v = VME_Output_S1(p, ushort, 12, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint0(p, v)             \
  (v = VME_Output_S1(p, ushort, 13, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint1(p, v)             \
  (v = VME_Output_S1(p, ushort, 13, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoshort(p, v)            \
  (v = VME_Output_S1(p, ushort, 13, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint3(p, v)             \
  (v = VME_Output_S1(p, ushort, 13, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint4(p, v)             \
  (v = VME_Output_S1(p, ushort, 13, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint5(p, v)             \
  (v = VME_Output_S1(p, ushort, 13, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint6(p, v)             \
  (v = VME_Output_S1(p, ushort, 13, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint7(p, v)             \
  (v = VME_Output_S1(p, ushort, 13, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint8(p, v)             \
  (v = VME_Output_S1(p, ushort, 13, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint9(p, v)             \
  (v = VME_Output_S1(p, ushort, 13, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint10(p, v)            \
  (v = VME_Output_S1(p, ushort, 13, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint11(p, v)            \
  (v = VME_Output_S1(p, ushort, 13, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint12(p, v)            \
  (v = VME_Output_S1(p, ushort, 13, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint13(p, v)            \
  (v = VME_Output_S1(p, ushort, 13, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint14(p, v)            \
  (v = VME_Output_S1(p, ushort, 13, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock13_SearchPoint15(p, v)            \
  (v = VME_Output_S1(p, ushort, 13, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint0(p, v)             \
  (v = VME_Output_S1(p, ushort, 14, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint1(p, v)             \
  (v = VME_Output_S1(p, ushort, 14, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoshort(p, v)            \
  (v = VME_Output_S1(p, ushort, 14, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint3(p, v)             \
  (v = VME_Output_S1(p, ushort, 14, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint4(p, v)             \
  (v = VME_Output_S1(p, ushort, 14, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint5(p, v)             \
  (v = VME_Output_S1(p, ushort, 14, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint6(p, v)             \
  (v = VME_Output_S1(p, ushort, 14, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint7(p, v)             \
  (v = VME_Output_S1(p, ushort, 14, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint8(p, v)             \
  (v = VME_Output_S1(p, ushort, 14, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint9(p, v)             \
  (v = VME_Output_S1(p, ushort, 14, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint10(p, v)            \
  (v = VME_Output_S1(p, ushort, 14, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint11(p, v)            \
  (v = VME_Output_S1(p, ushort, 14, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint12(p, v)            \
  (v = VME_Output_S1(p, ushort, 14, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint13(p, v)            \
  (v = VME_Output_S1(p, ushort, 14, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint14(p, v)            \
  (v = VME_Output_S1(p, ushort, 14, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock14_SearchPoint15(p, v)            \
  (v = VME_Output_S1(p, ushort, 14, 15))

#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint0(p, v)             \
  (v = VME_Output_S1(p, ushort, 15, 0))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint1(p, v)             \
  (v = VME_Output_S1(p, ushort, 15, 1))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoshort(p, v)            \
  (v = VME_Output_S1(p, ushort, 15, 2))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint3(p, v)             \
  (v = VME_Output_S1(p, ushort, 15, 3))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint4(p, v)             \
  (v = VME_Output_S1(p, ushort, 15, 4))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint5(p, v)             \
  (v = VME_Output_S1(p, ushort, 15, 5))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint6(p, v)             \
  (v = VME_Output_S1(p, ushort, 15, 6))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint7(p, v)             \
  (v = VME_Output_S1(p, ushort, 15, 7))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint8(p, v)             \
  (v = VME_Output_S1(p, ushort, 15, 8))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint9(p, v)             \
  (v = VME_Output_S1(p, ushort, 15, 9))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint10(p, v)            \
  (v = VME_Output_S1(p, ushort, 15, 10))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint11(p, v)            \
  (v = VME_Output_S1(p, ushort, 15, 11))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint12(p, v)            \
  (v = VME_Output_S1(p, ushort, 15, 12))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint13(p, v)            \
  (v = VME_Output_S1(p, ushort, 15, 13))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint14(p, v)            \
  (v = VME_Output_S1(p, ushort, 15, 14))
#define VME_GET_IDMOutput_DistortionMeshBlock15_SearchPoint15(p, v)            \
  (v = VME_Output_S1(p, ushort, 15, 15))

// Set the default values for VME input payload

#define INIT_VME_UNIINPUT(p)                                                   \
  { /* M0 */                                                                   \
    VME_Input_S1(p, uint, 0, 0) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 0, 1) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 0, 2) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 0, 3) = 0x00A80000;                                 \
    VME_Input_S1(p, uint, 0, 4) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 0, 5) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 0, 6) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 0, 7) = 0x00000000;                                 \
    /* M1 */                                                                   \
    VME_Input_S1(p, uint, 1, 0) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 1, 1) = 0x00000008;                                 \
    VME_Input_S1(p, uint, 1, 2) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 1, 3) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 1, 4) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 1, 5) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 1, 6) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 1, 7) = 0x00400060;                                 \
    /* M2 */                                                                   \
    VME_Input_S1(p, uint, 2, 0) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 2, 1) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 2, 2) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 2, 3) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 2, 4) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 2, 5) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 2, 6) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 2, 7) = 0x00000000;                                 \
    /* M3 */                                                                   \
    VME_Input_S1(p, uint, 3, 0) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 3, 1) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 3, 2) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 3, 3) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 3, 4) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 3, 5) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 3, 6) = 0x00000000;                                 \
    VME_Input_S1(p, uint, 3, 7) = 0x00000000;                                 \
  }

// TBD: set different default values for each payload field
#define INIT_VME_IMEINPUT(p)                                                   \
  { p = 0; }

#define INIT_VME_SICINPUT(p)                                                   \
  { p = 0; }

#define INIT_VME_FBRINPUT(p)                                                   \
  { p = 0; }

#define INIT_VME_IDMINPUT(p)                                                   \
  { p = 0; }

#define RESET_VME_UNIINPUT(p)                                                  \
  { p.format<uint, 4, 8>() = 0; }

#define RESET_VME_IMEINPUT(p, sz)                                              \
  { p.format<uint, sz, 8>() = 0; }

#define RESET_VME_SICINPUT(p)                                                  \
  { p.format<uint, 4, 8>() = 0; }

#define RESET_VME_FBRINPUT(p)                                                  \
  { p.format<uint, 4, 8>() = 0; }

#define RESET_VME_IDMINPUT(p)                                                  \
  { p.format<uint, 1, 8>() = 0; }

// Run VME APIs

typedef enum _VMEStreamMode_ {
  VME_STREAM_DISABLE = 0,
  VME_STREAM_OUT = 1,
  VME_STREAM_IN = 2,
  VME_STREAM_IN_OUT = 3
} VMEStreamMode;

typedef enum _VMESearchCtrl_ {
  VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START = 0,
  VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START = 1,
  VME_SEARCH_SINGLE_REF_DUAL_REC = 3,
  VME_SEARCH_DUAL_REF_DUAL_REC = 7
} VMESearchCtrl;

typedef enum _VMEMsgType_ {
  VME_SIC_MSG = 0,
  VME_FBR_MSG = 1,
  VME_IME_MSG = 2,
  VME_IDM_MSG = 3
} VMEMsgType;

static CM_INLINE constexpr unsigned
getVMEInputSize(unsigned size, VMEStreamMode mode, VMESearchCtrl ctrl) {
  return (mode != VME_STREAM_IN && mode != VME_STREAM_IN_OUT)
             ? size + 2
             : (ctrl == VME_SEARCH_DUAL_REF_DUAL_REC) ? (size + 6) : (size + 4);
}

template <VMEStreamMode streamMode, VMESearchCtrl searchCtrl, int N1, int N2>
CM_INLINE void
_run_vme_ime_impl(matrix<uchar, 4, 32> UNIInput, matrix<uchar, N1, 32> IMEInput,
                  SurfaceIndex curSurfIndex, vector<short, 2> ref0,
                  vector<short, 2> ref1, vector<ushort, 16> costCenter,
                  matrix_ref<uchar, N2, 32> IMEOutput) {
  // HSW: 3; otherwise 4 to invoke below function
  constexpr unsigned inputSize = getVMEInputSize(4, streamMode, searchCtrl);
  constexpr unsigned remaining = (inputSize - 4) * 8;
  matrix<uint, inputSize, 8> src = 0;
  // mov  (24)    VX(0,0)<1>,  UNIInput
  src.format<uint>().select<32, 1>(0) =
      UNIInput.format<uint>().select<32, 1>(0);
  // mov  (16)   VX(3,0)<1>,  IMEInput
  src.format<uint>().select<remaining, 1>(32) =
      IMEInput.format<uint>().select<remaining, 1>(0);
  // and  (1)     VX(0,13)<1>, VX(0,13):ub, 0xF8
  src.format<uchar>()(13) &= 0xF8;
  // or   (1)     VX(0,13)<1>, VX(0,13):ub, searchCtrl
  src.format<uchar>()(13) |= (unsigned)searchCtrl;
  // mov  (2)     VA(0,0)<1>,  ref0
  // since ref0 is converted from UW to UD, move it as 1 UD
  src(0, 0) = ref0.format<uint>()(0);
  // mov  (2)     VA(0,2)<1>,  ref1
  // since ref1 is converted from UW to UD, move it as 1 UD
  src(0, 1) = ref1.format<uint>()(0);
  // mov  (8)     VA(3,0)<1>,  costCenter
  src.select<1, 1, 8, 1>(3, 0) = costCenter.format<uint>().select<8, 1>(0);

  unsigned fCtrl = 0;        // Bit 7-0 of message descriptor
  fCtrl += 0x2 << 13;        // Bit 14-13 of message descriptor
  fCtrl += streamMode << 15; // Bit 16-15 of message descriptor

  unsigned rspLength = 288 / 32;
  if ((streamMode != VME_STREAM_OUT) && (streamMode != VME_STREAM_IN_OUT)) {
    rspLength = 224 / 32;
  } else if (searchCtrl == VME_SEARCH_DUAL_REF_DUAL_REC) {
    rspLength = 352 / 32;
  }

  // Function control (bit 0:18)
  // Header present   (bit 19)
  // Response length  (bit 20:24)
  // Message length   (bit 25:28)
  uint Descriptor = (fCtrl & 0x7FFFF) + (1 << 19) + ((rspLength & 0x1f) << 20) +
                    ((inputSize & 0xF) << 25);

  Descriptor = cm_get_value(curSurfIndex) + Descriptor;
  /*Implementing VME based on cm_send() for gen7_5
    template <int N1, int N2>
    void cm_send(matrix_ref<ushort, N1, 16> IMEOutput, matrix<ushort, inputSize,
    16>src, uint exDesc, uint msgDesc, uint sendc);*/
  cm_send(IMEOutput.format<ushort, N2, 16>(),
          src.format<ushort, inputSize, 16>(), __SFID_VME, Descriptor, 0u);
}

template <int N1, int N2>
CM_NODEBUG CM_INLINE void
run_vme_ime(matrix<uchar, 4, 32> UNIInput, matrix<uchar, N1, 32> IMEInput,
            VMEStreamMode streamMode, VMESearchCtrl searchCtrl,
            SurfaceIndex curSurfIndex, vector<short, 2> ref0,
            vector<short, 2> ref1, vector<ushort, 16> costCenter,
            matrix_ref<uchar, N2, 32> IMEOutput) {
#define VME_IME(mode, ctrl)                                                    \
  _run_vme_ime_impl<mode, ctrl>(UNIInput, IMEInput, curSurfIndex, ref0, ref1,  \
                                costCenter, IMEOutput)
  if (N1 == 2 && N2 == 7) {
    if (streamMode == VME_STREAM_DISABLE) {
      if (searchCtrl == VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START)
        VME_IME(VME_STREAM_DISABLE,
                VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START);
      if (searchCtrl == VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START)
        VME_IME(VME_STREAM_DISABLE,
                VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START);
      if (searchCtrl == VME_SEARCH_SINGLE_REF_DUAL_REC)
        VME_IME(VME_STREAM_DISABLE, VME_SEARCH_SINGLE_REF_DUAL_REC);
      if (searchCtrl == VME_SEARCH_DUAL_REF_DUAL_REC)
        VME_IME(VME_STREAM_DISABLE, VME_SEARCH_DUAL_REF_DUAL_REC);
    }
  } else if (N1 == 2 && N2 == 9) {
    if (streamMode == VME_STREAM_OUT) {
      // searchCtrl != VME_SEARCH_DUAL_REF_DUAL_REC
      if (searchCtrl == VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START) {
        VME_IME(VME_STREAM_OUT, VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START);
      }
      if (searchCtrl == VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START) {
        VME_IME(VME_STREAM_OUT, VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START);
      }
      if (searchCtrl == VME_SEARCH_SINGLE_REF_DUAL_REC) {
        VME_IME(VME_STREAM_OUT, VME_SEARCH_SINGLE_REF_DUAL_REC);
      }
    }
  } else if (N1 == 2 && N2 == 11) {
    if (streamMode == VME_STREAM_OUT &&
        searchCtrl == VME_SEARCH_DUAL_REF_DUAL_REC) {
      VME_IME(VME_STREAM_OUT, VME_SEARCH_DUAL_REF_DUAL_REC);
    }
  } else if (N1 == 4 && N2 == 7) {
    if (streamMode == VME_STREAM_IN) {
      // searchCtrl != VME_SEARCH_DUAL_REF_DUAL_REC
      if (searchCtrl == VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START) {
        VME_IME(VME_STREAM_IN, VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START);
      }
      if (searchCtrl == VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START) {
        VME_IME(VME_STREAM_IN, VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START);
      }
      if (searchCtrl == VME_SEARCH_SINGLE_REF_DUAL_REC) {
        VME_IME(VME_STREAM_IN, VME_SEARCH_SINGLE_REF_DUAL_REC);
      }
    }
  } else if (N1 == 4 && N2 == 9) {
    if (streamMode == VME_STREAM_IN_OUT) {
      // searchCtrl != VME_SEARCH_DUAL_REF_DUAL_REC
      if (searchCtrl == VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START) {
        VME_IME(VME_STREAM_IN_OUT,
                VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START);
      }
      if (searchCtrl == VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START) {
        VME_IME(VME_STREAM_IN_OUT, VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START);
      }
      if (searchCtrl == VME_SEARCH_SINGLE_REF_DUAL_REC) {
        VME_IME(VME_STREAM_IN_OUT, VME_SEARCH_SINGLE_REF_DUAL_REC);
      }
    }
  } else if (N1 == 6 && N2 == 7) {
    if (streamMode == VME_STREAM_IN &&
        searchCtrl == VME_SEARCH_DUAL_REF_DUAL_REC) {
      VME_IME(VME_STREAM_IN, VME_SEARCH_DUAL_REF_DUAL_REC);
    }
  } else if (N1 == 6 && N2 == 11) {
    if (streamMode == VME_STREAM_IN_OUT &&
        searchCtrl == VME_SEARCH_DUAL_REF_DUAL_REC) {
      VME_IME(VME_STREAM_IN_OUT, VME_SEARCH_DUAL_REF_DUAL_REC);
    }
  }
}

template <typename Dummy = void>
CM_INLINE void
run_vme_fbr(matrix<uchar, 4, 32> UNIInput, matrix<uchar, 4, 32> FBRInput,
            SurfaceIndex curSurfIndex, uchar FBRMbMode, uchar FBRSubMbShape,
            uchar FBRSubPredMode, matrix_ref<uchar, 7, 32> FBROutput) {
  matrix<uint, 8, 8> src = 0;
  // mov(96)    VX(0, 0)<1>, UNIInput
  src.format<uint>().select<32, 1>(0) =
      UNIInput.format<uint>().select<32, 1>(0);
  // mov  (128)   VX(3,0)<1>,  FBRInput
  src.format<uint>().select<32, 1>(32) =
      FBRInput.format<uint>().select<32, 1>(0);
  // mov  (1)     VX(2,20)<1>, FBRMbMode
  src.format<uchar, 8, 32>()(2, 20) = FBRMbMode;
  // mov  (1)     VX(2,21)<1>, FBRSubMbShape
  src.format<uchar, 8, 32>()(2, 21) = FBRSubMbShape;
  //  mov  (1)     VX(2,22)<1>, FBRSubPredMode
  src.format<uchar, 8, 32>()(2, 22) = FBRSubPredMode;

  unsigned fCtrl = 0x3 << 13;
  unsigned rspLength = 7;
  unsigned msglength = 8;
  uint Descriptor = (fCtrl & 0x7FFFF) + (1 << 19) + ((rspLength & 0x1f) << 20) +
                    ((msglength & 0xF) << 25);

  Descriptor = cm_get_value(curSurfIndex) + Descriptor;
  cm_send(FBROutput.format<ushort, 7, 16>(), src.format<ushort, 8, 16>(),
          __SFID_CRE, Descriptor, 0u);
}

template <typename Dummy = void>
CM_INLINE void
run_vme_sic(matrix<uchar, 4, 32> UNIInput, matrix<uchar, 4, 32> SICInput,
            SurfaceIndex curSurfIndex, matrix_ref<uchar, 7, 32> UNIOutput) {
  matrix<uint, 8, 8> src = 0;
  // mov  (96)    VX(0,0)<1>,  UNIInput
  src.format<uint>().select<32, 1>(0) =
      UNIInput.format<uint>().select<32, 1>(0);
  // mov  (128)   VX(3,0)<1>,  SICInput
  src.format<uint>().select<32, 1>(32) =
      SICInput.format<uint>().select<32, 1>(0);
  unsigned fCtrl = 0x1 << 13;
  unsigned rspLength = 7;
  unsigned msgLength = 8;
  uint Descriptor = (fCtrl & 0x7FFFF) + (1 << 19) + ((rspLength & 0x1f) << 20) +
                    ((msgLength & 0xF) << 25);

  Descriptor = cm_get_value(curSurfIndex) + Descriptor;
  cm_send(UNIOutput.format<ushort, 7, 16>(), src.format<ushort, 8, 16>(),
          __SFID_CRE, Descriptor, 0u);
}

template <typename Dummy = void>
CM_INLINE void
run_vme_idm(matrix<uchar, 4, 32> UNIInput, matrix<uchar, 1, 32> IDMInput,
            SurfaceIndex curSurfIndex, matrix_ref<uchar, 16, 32> IDMOutput) {
  matrix<uint, 5, 8> src = 0;
  // mov(96)    VX(0, 0)<1>, UNIInput
  src.format<uint>().select<32, 1>(0) =
      UNIInput.format<uint>().select<32, 1>(0);
  // mov  (128)   VX(3,0)<1>,  IDMInput
  src.format<uint>().select<8, 1>(32) = IDMInput.format<uint>().select<8, 1>(0);

  unsigned fCtrl = 0;
  unsigned rspLength = 16;
  unsigned msglength = 5;
  uint Descriptor = (fCtrl & 0x7FFFF) + (1 << 19) + ((rspLength & 0x1f) << 20) +
                    ((msglength & 0xF) << 25);

  Descriptor = cm_get_value(curSurfIndex) + Descriptor;
  cm_send(IDMOutput.format<ushort, 16, 16>(), src.format<ushort, 5, 16>(),
          __SFID_VME, Descriptor, 0u);
}

#endif /* _CLANG_gen8_VME_H_ */
