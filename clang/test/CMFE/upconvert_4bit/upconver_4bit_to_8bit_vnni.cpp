/*========================== begin_copyright_notice ============================

Copyright (C) 2024-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=cri -DWIDTH=16 -emit-llvm -S -o %t.simd16.ll -- %s
// RUN: FileCheck --check-prefix=SIMD16 %s --input-file=%t.simd16.ll

// RUN: %cmc -march=cri -DWIDTH=32 -emit-llvm -S -o %t.simd32.ll -- %s
// RUN: FileCheck --check-prefix=SIMD32 %s --input-file=%t.simd32.ll

// SIMD16: [[SRC0:%[^ ]+]] = call <16 x i16> @llvm.genx.rdregioni.v16i16.v32i16.i16(<32 x i16> %{{[^,]+}}, i32 0, i32 16, i32 2, i16 0, i32 undef)
// SIMD16: call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i32.v16i16(<16 x i32> %{{[^,]+}}, <16 x i16> [[SRC0]])
// SIMD16: [[SRC1:%[^ ]+]] = call <16 x i16> @llvm.genx.rdregioni.v16i16.v32i16.i16(<32 x i16> %{{[^,]+}}, i32 0, i32 16, i32 2, i16 2, i32 undef)
// SIMD16: call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i32.v16i16(<16 x i32> %{{[^,]+}}, <16 x i16> [[SRC1]])

// SIMD32: [[SRC0:%[^ ]+]] = call <32 x i16> @llvm.genx.rdregioni.v32i16.v64i16.i16(<64 x i16> %{{[^,]+}}, i32 0, i32 32, i32 2, i16 0, i32 undef)
// SIMD32: call <32 x i32> @llvm.genx.packed.4bit.upconvert.lut.v32i32.v32i16(<16 x i32> %{{[^,]+}}, <32 x i16> [[SRC0]])
// SIMD32: [[SRC1:%[^ ]+]] = call <32 x i16> @llvm.genx.rdregioni.v32i16.v64i16.i16(<64 x i16> %{{[^,]+}}, i32 0, i32 32, i32 2, i16 2, i32 undef)
// SIMD32: call <32 x i32> @llvm.genx.packed.4bit.upconvert.lut.v32i32.v32i16(<16 x i32> %{{[^,]+}}, <32 x i16> [[SRC1]])

constexpr uint Width = WIDTH;

_GENX_VOLATILE_ vector<uint, Width> Src;
_GENX_VOLATILE_ matrix<uint, 2, Width> Dst;

_GENX_MAIN_ void kernel() {
  vector_ref<ushort, Width * 2> SrcWord = Src.format<ushort>();

  vector<uint32_t, 16> LUT = {0x00000000, 0x3c3c3c3c, 0x40404040, 0x44444444,
                              0x48484848, 0x4c4c4c4c, 0x50505050, 0x54545454,
                              0x7f7f7f7f, 0xbcbcbcbc, 0xc0c0c0c0, 0xc4c4c4c4,
                              0xc8c8c8c8, 0xcccccccc, 0xd0d0d0d0, 0xd4d4d4d4};

  Dst.row(0) = cm_upconvert_4bit_lut<0>(LUT, SrcWord);
  Dst.row(1) = cm_upconvert_4bit_lut<1>(LUT, SrcWord);
}
