/*========================== begin_copyright_notice ============================

Copyright (C) 2023-2026 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "GenX.h"

#include <unordered_map>
#include <unordered_set>

static inline constexpr uint32_t encodeGmdId(uint32_t Major, uint32_t Minor,
                                             uint32_t Revision) {
  return ((Major & 0x3ff) << 22 | (Minor & 0xff) << 14 | (Revision & 0x3f));
}

struct TargetProperties {
  bool HasFP64 = false;
  bool HasBFloat16 = false;
  bool HasSLMCasInt64 = false;
  bool HasBfn = false;
  bool HasDp4a = false;
  bool HasDpas = false;
  bool HasDpasw = false;
  bool HasDpasFp16 = false;
  bool HasDpasBf16 = false;
  bool HasDpasTf32 = false;
  bool HasSrndFp32ToFp16 = false;
  bool HasMoveBf8 = false;
  bool HasMoveHf8 = false;
  bool HasSrndFp16ToBf8 = false;
  bool HasDpasBf8 = false;
  bool HasDpasHf8 = false;
  bool HasDpasFp4 = false;
  bool HasBdpas = false;

  bool HasBF16Atomic = false;

  bool HasShuffleIndex4 = false;
  bool HasDownScale = false;
  bool HasLfsr = false;

  bool HasTanh = false;
  bool HasSigmoid = false;

  bool HasSrndBf16ToBf8 = false;
  bool HasSrndBf16ToHf8 = false;
  bool HasSrndFp16ToHf8 = false;

  bool HasDpasIntMix = false;
  bool HasDpasInt2 = false;
  bool HasDpasHalfAcc = false;
  bool HasDpasBF16Acc = false;

  bool Has3DSampleHalf = false;
  bool HasLsc2DLarge = false;
  bool HasLscL1L2L3Cache = false;
  unsigned GrfWidth = 256;
  std::unordered_set<unsigned> SupportedGrfNums;

  unsigned MaxSLMSize = 64;
};

// clang-format off
static const std::unordered_map<uint32_t, TargetProperties> TargetProps = {
  { encodeGmdId(35, 11, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasBf8 = true;
      P.HasDpasHf8 = true;
      P.HasDpasFp4 = true;
      P.HasBdpas = true;
      P.HasBF16Atomic = true;
      P.HasShuffleIndex4 = true;
      P.HasDownScale = true;
      P.HasLfsr = true;
      P.HasTanh = true;
      P.HasSigmoid = true;
      P.HasSrndBf16ToBf8 = true;
      P.HasSrndBf16ToHf8 = true;
      P.HasSrndFp16ToHf8 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.HasLsc2DLarge = true;
      P.HasLscL1L2L3Cache = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256,512};
      P.MaxSLMSize = 384;
      return P;
    }() },
  { encodeGmdId(35, 10, 4), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasBf8 = true;
      P.HasDpasHf8 = true;
      P.HasBdpas = true;
      P.HasBF16Atomic = true;
      P.HasShuffleIndex4 = true;
      P.HasDownScale = true;
      P.HasLfsr = true;
      P.HasTanh = true;
      P.HasSigmoid = true;
      P.HasSrndBf16ToBf8 = true;
      P.HasSrndBf16ToHf8 = true;
      P.HasSrndFp16ToHf8 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.HasLsc2DLarge = true;
      P.HasLscL1L2L3Cache = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256,320,448,512};
      P.MaxSLMSize = 192;
      return P;
    }() },
  { encodeGmdId(35, 10, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasBf8 = true;
      P.HasDpasHf8 = true;
      P.HasBdpas = true;
      P.HasBF16Atomic = true;
      P.HasShuffleIndex4 = true;
      P.HasDownScale = true;
      P.HasLfsr = true;
      P.HasTanh = true;
      P.HasSigmoid = true;
      P.HasSrndBf16ToBf8 = true;
      P.HasSrndBf16ToHf8 = true;
      P.HasSrndFp16ToHf8 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.HasLsc2DLarge = true;
      P.HasLscL1L2L3Cache = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256,320,448,512};
      P.MaxSLMSize = 192;
      return P;
    }() },
  { encodeGmdId(30, 5, 4), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 5, 1), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 5, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 4, 4), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 4, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 3, 1), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 3, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 1, 1), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 1, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 0, 4), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(30, 0, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasMoveBf8 = true;
      P.HasMoveHf8 = true;
      P.HasSrndFp16ToBf8 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {32,64,96,128,160,192,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(20, 4, 4), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(20, 4, 1), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(20, 4, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(20, 2, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(20, 1, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(12, 74, 4), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 74, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 71, 4), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 71, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 70, 4), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 70, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 61, 7), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(12, 60, 7), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(12, 60, 6), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(12, 60, 5), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(12, 60, 3), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasSLMCasInt64 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasTf32 = true;
      P.HasSrndFp32ToFp16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(12, 60, 1), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(12, 60, 0), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.HasDpasHalfAcc = true;
      P.HasDpasBF16Acc = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 512;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 128;
      return P;
    }() },
  { encodeGmdId(12, 57, 0), [] {
      TargetProperties P;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 56, 5), [] {
      TargetProperties P;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 56, 4), [] {
      TargetProperties P;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 56, 0), [] {
      TargetProperties P;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 55, 8), [] {
      TargetProperties P;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 55, 4), [] {
      TargetProperties P;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 55, 1), [] {
      TargetProperties P;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 55, 0), [] {
      TargetProperties P;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.HasDpasIntMix = true;
      P.HasDpasInt2 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 50, 4), [] {
      TargetProperties P;
      P.HasFP64 = true;
      P.HasBFloat16 = true;
      P.HasBfn = true;
      P.HasDp4a = true;
      P.HasDpas = true;
      P.HasDpasw = true;
      P.HasDpasFp16 = true;
      P.HasDpasBf16 = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128,256};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 10, 0), [] {
      TargetProperties P;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 4, 0), [] {
      TargetProperties P;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 3, 0), [] {
      TargetProperties P;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 2, 0), [] {
      TargetProperties P;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 1, 0), [] {
      TargetProperties P;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128};
      P.MaxSLMSize = 64;
      return P;
    }() },
  { encodeGmdId(12, 0, 0), [] {
      TargetProperties P;
      P.HasDp4a = true;
      P.Has3DSampleHalf = true;
      P.GrfWidth = 256;
      P.SupportedGrfNums = {128};
      P.MaxSLMSize = 64;
      return P;
    }() },
};
// clang-format on

void clang::targets::GenXTargetInfo::setCPUProperties() {
  auto CPUId = encodeGmdId(Major, Minor, Revision);
  auto It = TargetProps.find(CPUId);
  if (It == std::end(TargetProps))
    return;

  const auto &Prop = It->second;
  HasFP64 = Prop.HasFP64;
  HasBFloat16 = Prop.HasBFloat16;
  HasSLMCasInt64 = Prop.HasSLMCasInt64;

  HasBfn = Prop.HasBfn;
  HasDp4a = Prop.HasDp4a;
  HasDpas = Prop.HasDpas;
  HasDpasw = Prop.HasDpasw;
  HasDpasFp16 = Prop.HasDpasFp16;
  HasDpasBf16 = Prop.HasDpasBf16;
  HasDpasTf32 = Prop.HasDpasTf32;

  HasSrndFp32ToFp16 = Prop.HasSrndFp32ToFp16;

  HasMoveBf8 = Prop.HasMoveBf8;
  HasMoveHf8 = Prop.HasMoveHf8;

  HasSrndFp16ToBf8 = Prop.HasSrndFp16ToBf8;

  HasDpasBf8 = Prop.HasDpasBf8;
  HasDpasHf8 = Prop.HasDpasHf8;
  HasDpasFp4 = Prop.HasDpasFp4;
  HasBdpas = Prop.HasBdpas;

  HasBF16Atomic = Prop.HasBF16Atomic;

  HasShuffleIndex4 = Prop.HasShuffleIndex4;
  HasDownScale = Prop.HasDownScale;
  HasLfsr = Prop.HasLfsr;

  HasTanh = Prop.HasTanh;
  HasSigmoid = Prop.HasSigmoid;

  HasSrndBf16ToBf8 = Prop.HasSrndBf16ToBf8;
  HasSrndBf16ToHf8 = Prop.HasSrndBf16ToHf8;
  HasSrndFp16ToHf8 = Prop.HasSrndFp16ToHf8;

  HasDpasIntMix = Prop.HasDpasIntMix;
  HasDpasInt2 = Prop.HasDpasInt2;
  HasDpasHalfAcc = Prop.HasDpasHalfAcc;
  HasDpasBF16Acc = Prop.HasDpasBF16Acc;

  Has3DSampleHalf = Prop.Has3DSampleHalf;
  HasLsc2DLarge = Prop.HasLsc2DLarge;
  HasLscL1L2L3Cache = Prop.HasLscL1L2L3Cache;

  GrfWidth = Prop.GrfWidth;
  SupportedGrfNums = Prop.SupportedGrfNums;

  MaxSLMSize = Prop.MaxSLMSize;
}
