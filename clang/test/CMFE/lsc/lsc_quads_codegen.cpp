/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// RUN: %cmc -g0 -march=pvc -Qxcm_revid=5 -emit-llvm -S -o %t.ll -- %s
// RUN: FileCheck %s --input-file %t.ll


// CHECK-LABEL: @test_load4_slm
extern "C" _GENX_MAIN_ void test_load4_slm(vector<unsigned, 32> offsets) {
  constexpr auto SLMSize = 1024 * sizeof(unsigned int);
  cm_slm_init(SLMSize);
  auto buffer = cm_slm_alloc(SLMSize);
  auto data = cm_load4_slm<unsigned, ChannelMaskType::_CM_ABGR_ENABLE>(buffer + offsets);
  (void)data;
// CHECK: {{[^)]+}} = call <128 x i32> @llvm.genx.lsc.load.quad.slm.v128i32.v32i1.v32i32(<32 x i1> {{[^)]+}}, i8 2, i8 0, i8 0, i16 1, i32 0, i8 3, i8 4, i8 1, i8 15, <32 x i32> {{[^)]+}}, i32 0)
}

// CHECK-LABEL: @test_load4_bti
extern "C" _GENX_MAIN_ void test_load4_bti(SurfaceIndex buffer [[type("buffer_t")]],
                                              vector<unsigned, 16> offsets) {
  auto data =
      cm_load4<unsigned int, ChannelMaskType::_CM_G_ENABLE, DataSize::Default,
               CacheHint::Streaming, CacheHint::Cached>(buffer, offsets);
  (void)data;
// CHECK: {{[^)]+}} = call <16 x i32> @llvm.genx.lsc.load.quad.bti.v16i32.v16i1.v16i32(<16 x i1> {{[^)]+}}, i8 2, i8 5, i8 2, i16 1, i32 0, i8 3, i8 1, i8 1, i8 2, <16 x i32> {{[^)]+}}, i32 {{[^)]+}})
}

// CHECK-LABEL: @test_load4_ptr
extern "C" _GENX_MAIN_ void test_load4_ptr(unsigned int *buffer
                                           [[type("svmptr_t")]],
                                           vector<unsigned, 16> offsets) {
  auto data =
      cm_ptr_load4<unsigned int, ChannelMaskType::_CM_G_ENABLE,
                   DataSize::Default, CacheHint::Streaming, CacheHint::Cached>(
          buffer, offsets);
  (void)data;
// CHECK: {{[^)]+}} = call <16 x i32> @llvm.genx.lsc.load.quad.stateless.v16i32.v16i1.v16i64(<16 x i1> {{[^)]+}}, i8 2, i8 5, i8 2, i16 1, i32 0, i8 3, i8 1, i8 1, i8 2, <16 x i64> {{[^)]+}}, i32 0)
}

// CHECK-LABEL: @test_store4_slm
extern "C" _GENX_MAIN_ void test_store4_slm(vector<unsigned, 16> offsets) {
  constexpr auto SLMSize = 1024 * sizeof(unsigned int);
  cm_slm_init(SLMSize);
  auto buffer = cm_slm_alloc(SLMSize);
  vector<unsigned int, 16 * 3> data = 5;
  cm_store4_slm<unsigned int, ChannelMaskType::_CM_BGR_ENABLE>(buffer + offsets,
                                                               data);
// CHECK: call void @llvm.genx.lsc.store.quad.slm.v16i1.v16i32.v48i32(<16 x i1> {{[^)]+}}, i8 6, i8 0, i8 0, i16 1, i32 0, i8 3, i8 3, i8 1, i8 7, <16 x i32> {{[^)]+}}, <48 x i32> {{[^)]+}}, i32 0)
}

// CHECK-LABEL: @test_store4_bti
extern "C" _GENX_MAIN_ void test_store4_bti(SurfaceIndex buffer
                                            [[type("buffer_t")]],
                                            vector<unsigned, 32> offsets) {
  vector<unsigned int, 32 * 2> data = 5;
  cm_store4<unsigned int, ChannelMaskType::_CM_GR_ENABLE, DataSize::Default,
            CacheHint::WriteBack, CacheHint::WriteBack>(buffer, offsets, data);
// CHECK: call void @llvm.genx.lsc.store.quad.bti.v32i1.v32i32.v64i32(<32 x i1> {{[^)]+}}, i8 6, i8 3, i8 3, i16 1, i32 0, i8 3, i8 2, i8 1, i8 3, <32 x i32> {{[^)]+}}, <64 x i32> {{[^)]+}}, i32 {{[^)]+}})
}

// CHECK-LABEL: @test_store4_ptr
extern "C" _GENX_MAIN_ void test_store4_ptr(unsigned int *buffer
                                            [[type("svmptr_t")]],
                                            vector<unsigned, 16> offsets) {
  vector<unsigned int, 16 * 3> data = 5;
  cm_ptr_store4<unsigned int, ChannelMaskType::_CM_BGR_ENABLE,
                DataSize::Default, CacheHint::WriteThrough,
                CacheHint::Uncached>(buffer, offsets, data);
// CHECK: call void @llvm.genx.lsc.store.quad.stateless.v16i1.v16i64.v48i32(<16 x i1> {{[^)]+}}, i8 6, i8 4, i8 1, i16 1, i32 0, i8 3, i8 3, i8 1, i8 7, <16 x i64> {{[^)]+}}, <48 x i32> {{[^)]+}}, i32 0)
}
