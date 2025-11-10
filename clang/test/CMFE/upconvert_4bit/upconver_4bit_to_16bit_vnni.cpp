/*========================== begin_copyright_notice ============================

Copyright (C) 2024-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -march=cri -DWIDTH=16 -emit-llvm -S -o %t.simd16.ll -- %s
// RUN: FileCheck --check-prefix=SIMD16 %s --input-file=%t.simd16.ll

// RUN: %cmc -march=cri -DWIDTH=32 -emit-llvm -S -o %t.simd32.ll -- %s
// RUN: FileCheck --check-prefix=SIMD32 %s --input-file=%t.simd32.ll

// SIMD16: [[SRC0:%[^ ]+]] = call <16 x i8> @llvm.genx.rdregioni.v16i8.v64i8.i16(<64 x i8> %{{[^,]+}}, i32 0, i32 16, i32 4, i16 0, i32 undef)
// SIMD16: call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i32.v16i8(<16 x i32> %{{[^,]+}}, <16 x i8> [[SRC0]])
// SIMD16: [[SRC1:%[^ ]+]] = call <16 x i8> @llvm.genx.rdregioni.v16i8.v64i8.i16(<64 x i8> %{{[^,]+}}, i32 0, i32 16, i32 4, i16 1, i32 undef)
// SIMD16: call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i32.v16i8(<16 x i32> %{{[^,]+}}, <16 x i8> [[SRC1]])
// SIMD16: [[SRC2:%[^ ]+]] = call <16 x i8> @llvm.genx.rdregioni.v16i8.v64i8.i16(<64 x i8> %{{[^,]+}}, i32 0, i32 16, i32 4, i16 2, i32 undef)
// SIMD16: call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i32.v16i8(<16 x i32> %{{[^,]+}}, <16 x i8> [[SRC2]])
// SIMD16: [[SRC3:%[^ ]+]] = call <16 x i8> @llvm.genx.rdregioni.v16i8.v64i8.i16(<64 x i8> %{{[^,]+}}, i32 0, i32 16, i32 4, i16 3, i32 undef)
// sIMD16: call <16 x i32> @llvm.genx.packed.4bit.upconvert.lut.v16i32.v16i8(<16 x i32> %{{[^,]+}}, <16 x i8> [[SRC3]])

// SIMD32: [[SRC0:%[^ ]+]] = call <32 x i8> @llvm.genx.rdregioni.v32i8.v128i8.i16(<128 x i8> %{{[^,]+}}, i32 0, i32 32, i32 4, i16 0, i32 undef)
// SIMD32: call <32 x i32> @llvm.genx.packed.4bit.upconvert.lut.v32i32.v32i8(<16 x i32> %{{[^,]+}}, <32 x i8> [[SRC0]])
// SIMD32: [[SRC1:%[^ ]+]] = call <32 x i8> @llvm.genx.rdregioni.v32i8.v128i8.i16(<128 x i8> %{{[^,]+}}, i32 0, i32 32, i32 4, i16 1, i32 undef)
// SIMD32: call <32 x i32> @llvm.genx.packed.4bit.upconvert.lut.v32i32.v32i8(<16 x i32> %{{[^,]+}}, <32 x i8> [[SRC1]])
// SIMD32: [[SRC2:%[^ ]+]] = call <32 x i8> @llvm.genx.rdregioni.v32i8.v128i8.i16(<128 x i8> %{{[^,]+}}, i32 0, i32 32, i32 4, i16 2, i32 undef)
// SIMD32: call <32 x i32> @llvm.genx.packed.4bit.upconvert.lut.v32i32.v32i8(<16 x i32> %{{[^,]+}}, <32 x i8> [[SRC2]])
// SIMD32: [[SRC3:%[^ ]+]] = call <32 x i8> @llvm.genx.rdregioni.v32i8.v128i8.i16(<128 x i8> %{{[^,]+}}, i32 0, i32 32, i32 4, i16 3, i32 undef)
// SIMD32: call <32 x i32> @llvm.genx.packed.4bit.upconvert.lut.v32i32.v32i8(<16 x i32> %{{[^,]+}}, <32 x i8> [[SRC3]])

constexpr uint Width = WIDTH;

_GENX_VOLATILE_ vector<uint, Width> Src;
_GENX_VOLATILE_ matrix<uint, 4, Width> Dst;

_GENX_MAIN_ void kernel() {
  vector_ref<uchar, Width * 4> SrcByte = Src.format<uchar>();

  vector<uint32_t, 16> LUT = {0x00000000, 0x3f803f80, 0x40004000, 0x40804080,
                              0x41004100, 0x41804180, 0x42004200, 0x42804280,
                              0x7fff7fff, 0xbf80bf80, 0xc000c000, 0xc080c080,
                              0xc100c100, 0xc180c180, 0xc200c200, 0xc280c280};

  Dst.row(0) = cm_upconvert_4bit_lut<0>(LUT, SrcByte);
  Dst.row(1) = cm_upconvert_4bit_lut<1>(LUT, SrcByte);
  Dst.row(2) = cm_upconvert_4bit_lut<2>(LUT, SrcByte);
  Dst.row(3) = cm_upconvert_4bit_lut<3>(LUT, SrcByte);
}
