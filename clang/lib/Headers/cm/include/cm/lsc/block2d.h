/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:lsc/block2d.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_LSC_BLOCK2D_H_
#define _CLANG_CM_LSC_BLOCK2D_H_

#include "helpers.h"

#include <cm/cm_has_instr.h>
#include <cm/cm_traits.h>
#include <cm/cm_util.h>

namespace lsc {
#define BLOCK_2D_DESC_ACCESSOR(field, type, index)                             \
  CM_INLINE block_2d_desc &set_##field(type Value) {                           \
    Payload.format<type>().select<1, 1>(index) = Value;                        \
    return *this;                                                              \
  }                                                                            \
  CM_INLINE type get_##field() const {                                         \
    vector<type, 1> Ret = Payload.format<type>().select<1, 1>(index);          \
    return Ret[0];                                                             \
  }

template <typename T, unsigned NumBlocks, unsigned BlockHeight,
          unsigned BlockWidth>
struct block_2d_desc {
  static_assert(NumBlocks != 0, "Number of blocks should not be zero");
  static_assert(BlockHeight != 0, "Block height should not be zero");
  static_assert(BlockWidth != 0, "Block width should not be zero");
  static_assert(details::is_cm_scalar<T>::value,
                "Block element type must be scalar");

  using value_type = T;
  static constexpr int num_blocks = NumBlocks;
  static constexpr int height = BlockHeight;
  static constexpr int width = BlockWidth;

  CM_INLINE block_2d_desc(T *Ptr, unsigned Height, unsigned Width,
                          unsigned Pitch, int BlockX, int BlockY)
      : block_2d_desc(reinterpret_cast<uint64_t>(Ptr), Height, Width, Pitch,
                      BlockX, BlockY) {}

  CM_INLINE block_2d_desc(__global T *Ptr, unsigned Height, unsigned Width,
                          unsigned Pitch, int BlockX, int BlockY)
      : block_2d_desc(reinterpret_cast<uint64_t>(Ptr), Height, Width, Pitch,
                      BlockX, BlockY) {}

  CM_INLINE block_2d_desc(uint64_t Base, unsigned Height, unsigned Width,
                          unsigned Pitch, int BlockX, int BlockY) {
    set_base(Base);
    set_height(Height);
    set_width(Width);
    set_pitch(Pitch);
    set_block_x(BlockX);
    set_block_y(BlockY);
    set_desc((NumBlocks - 1) << 16 | (BlockHeight - 1) << 8 | (BlockWidth - 1));
  }

  BLOCK_2D_DESC_ACCESSOR(base, uint64_t, 0)
  BLOCK_2D_DESC_ACCESSOR(width, uint32_t, 2)
  BLOCK_2D_DESC_ACCESSOR(height, uint32_t, 3)
  BLOCK_2D_DESC_ACCESSOR(pitch, uint32_t, 4)
  BLOCK_2D_DESC_ACCESSOR(block_x, int32_t, 5)
  BLOCK_2D_DESC_ACCESSOR(block_y, int32_t, 6)

  CM_INLINE block_2d_desc &set_base_ptr(T *Ptr) {
    return set_base(reinterpret_cast<uint64_t>(Ptr));
  }
  CM_INLINE T *get_base_ptr() const {
    return reinterpret_cast<T *>(get_base());
  }

  CM_INLINE vector<uint32_t, 16> get_raw_desc() const { return Payload; }

private:
  BLOCK_2D_DESC_ACCESSOR(desc, uint32_t, 7)

  vector<uint32_t, 16> Payload;
};
#undef BLOCK_2D_DESC_ACCESSOR

enum LoadOp {
  Normal = 0,
  Transpose = 1,
  VNNI = 2,
};
} // namespace lsc

namespace details {
template <typename T>
constexpr unsigned get_lsc_grf_width(lsc::LoadOp Op, unsigned BlockWidth,
                                     unsigned BlockHeight) {
  switch (Op) {
  case lsc::LoadOp::Normal:
    return BlockWidth;
  case lsc::LoadOp::Transpose:
    return BlockHeight;
  case lsc::LoadOp::VNNI: {
    auto ElementsPerDWord = DWORD / sizeof(T);
    return BlockWidth * ElementsPerDWord;
  }
  default:
    break;
  }
  return 0;
}

template <typename T>
constexpr unsigned get_lsc_grf_height(lsc::LoadOp Op, unsigned BlockWidth,
                                      unsigned BlockHeight) {
  switch (Op) {
  case lsc::LoadOp::Normal:
    return BlockHeight;
  case lsc::LoadOp::Transpose:
    return BlockWidth;
  case lsc::LoadOp::VNNI: {
    auto ElementsPerDWord = DWORD / sizeof(T);
    return (BlockHeight + ElementsPerDWord - 1) / ElementsPerDWord;
  }
  default:
    break;
  }
  return 0;
}

template <typename T>
constexpr unsigned get_lsc_grf_elements(lsc::LoadOp Op, unsigned BlockW,
                                        unsigned BlockH, unsigned NumBlocks) {
  auto GrfWidth = get_lsc_grf_width<T>(Op, BlockW, BlockH);
  auto GrfRowPitch = getNextPowerOf2(GrfWidth);

  auto GrfHeight = get_lsc_grf_height<T>(Op, BlockW, BlockH);

  auto GrfBlockSize = GrfRowPitch * GrfHeight;
  auto GrfBlockPitch =
      roundUpNextMultiple(GrfBlockSize, CM_GRF_WIDTH / (8 * sizeof(T)));

  auto GrfElements = GrfBlockPitch * NumBlocks;
  return GrfElements;
}

template <lsc::LoadOp Op, CacheHint L1H, CacheHint L2H,
          typename T, int NumBlocks, int BlockHeight, int BlockWidth>
CM_INLINE CM_NODEBUG void check_lsc_block_2d_load_desc() {
  CM_HAS_LSC_UNTYPED_2D_CONTROL;

  CM_STATIC_ERROR((lsc_check_cache_hint_load<L1H, L2H>()),
                  "unsupported cache hint");

  CM_STATIC_ERROR(NumBlocks == 1 || NumBlocks == 2 || NumBlocks == 4,
                  "Unsupported number of blocks");

  CM_STATIC_ERROR(sizeof(T) * BlockWidth % 4 == 0,
                  "Block width in bytes must be a multiple of DW (4 bytes)");

  if constexpr (Op == lsc::LoadOp::Transpose) {
    CM_STATIC_ERROR(
        sizeof(T) == DWORD || sizeof(T) == QWORD,
        "Transpose load is supported only for dword and qword types");
    CM_STATIC_ERROR(NumBlocks == 1,
                    "Transpose load is supported only for a single block");

    if constexpr (sizeof(T) == DWORD) {
      CM_STATIC_ERROR(BlockWidth <= 8, "Block width is unsupported");
      CM_STATIC_ERROR(BlockHeight <= 32, "Block height is unsupported");
    } else if constexpr (sizeof(T) == QWORD) {
      CM_STATIC_ERROR(BlockHeight == 8, "Block height is unsupported");
      CM_STATIC_ERROR(BlockWidth == 1 || BlockWidth == 2 || BlockWidth == 4,
                      "Block width is unsupported");
    }
  } else if constexpr (Op == lsc::LoadOp::VNNI) {
    constexpr auto ElementsPerDWord = DWORD / sizeof(T);
    CM_STATIC_ERROR(sizeof(T) == BYTE || sizeof(T) == WORD,
                    "VNNI load is supported only for byte and word types");
    CM_STATIC_ERROR(BlockWidth * NumBlocks * sizeof(T) <= 64,
                    "Block width is too large");
    CM_STATIC_ERROR(BlockHeight >= ElementsPerDWord && BlockHeight <= 32,
                    "Block height is unsupported");
  } else {
    CM_STATIC_ERROR(Op == lsc::LoadOp::Normal, "Unsupported load operation");
    CM_STATIC_ERROR(BlockWidth * NumBlocks * sizeof(T) <= 64,
                    "Block width is too large");
    CM_STATIC_ERROR(BlockHeight <= 32, "Block height is too large");
    if constexpr (sizeof(T) == DWORD)
      CM_STATIC_ERROR(NumBlocks == 1 || NumBlocks == 2,
                      "Unsupported number of blocks");
    else if constexpr (sizeof(T) == QWORD)
      CM_STATIC_ERROR(NumBlocks == 1, "Unsupported number of blocks");
  }

  CM_STATIC_ERROR((get_lsc_grf_elements<T>(Op, BlockWidth, BlockHeight,
                                           NumBlocks) <= 32 * CM_GRF_WIDTH / 8),
                  "Block operation cannot touch more than 32 GRFs");
}

template <CacheHint L1H, CacheHint L2H,
          typename T, int NumBlocks, int BlockHeight, int BlockWidth>
CM_INLINE CM_NODEBUG void check_lsc_block_2d_prefetch_desc() {
  CM_HAS_LSC_UNTYPED_2D_CONTROL;

  CM_STATIC_ERROR((lsc_check_cache_hint_prefetch<L1H, L2H>()),
                  "unsupported cache hint");

  CM_STATIC_ERROR(NumBlocks == 1 || NumBlocks == 2 || NumBlocks == 4,
                  "Unsupported number of blocks");

  CM_STATIC_ERROR(sizeof(T) * BlockWidth % 4 == 0,
                  "Block width in bytes must be a multiple of DW (4 bytes)");

  constexpr auto WidthBytes = NumBlocks * BlockWidth * sizeof(T);

  CM_STATIC_ERROR(WidthBytes <= 64, "Block width is too large");

  if constexpr (sizeof(T) == DWORD)
    CM_STATIC_ERROR(NumBlocks == 1 || NumBlocks == 2,
                    "Unsupported number of blocks");
  else if constexpr (sizeof(T) == QWORD)
    CM_STATIC_ERROR(NumBlocks == 1, "Unsupported number of blocks");

  CM_STATIC_ERROR(BlockHeight <= 32, "Block height is too large");
}

template <CacheHint L1H, CacheHint L2H,
          typename T, int BlockHeight, int BlockWidth>
CM_INLINE CM_NODEBUG void check_lsc_block_2d_store_desc() {
  CM_HAS_LSC_UNTYPED_2D_CONTROL;

  CM_STATIC_ERROR((lsc_check_cache_hint_store<L1H, L2H>()),
                  "unsupported cache hint");

  CM_STATIC_ERROR(sizeof(T) * BlockWidth % 4 == 0,
                  "Block width in bytes must be a multiple of DW (4 bytes)");
  CM_STATIC_ERROR(BlockWidth * sizeof(T) <= 64, "Block width is too large");
  CM_STATIC_ERROR(BlockHeight <= 8, "Block height is too large");
}

template <typename T, unsigned BlockH, unsigned BlockW, unsigned NumBlocks = 1,
          lsc::LoadOp Op = lsc::LoadOp::Normal>
using Block2DTy =
    vector<T, get_lsc_grf_elements<T>(Op, BlockW, BlockH, NumBlocks)>;

template <typename T, unsigned BlockH, unsigned BlockW, unsigned NumBlocks = 1,
          lsc::LoadOp Op = lsc::LoadOp::Normal>
using Block2DRefTy =
    vector_ref<T, get_lsc_grf_elements<T>(Op, BlockW, BlockH, NumBlocks)>;

using CacheHintsTy = vector<uint8_t, 2>;
CM_INLINE CM_NODEBUG CacheHintsTy get_cache_hint_vector(CacheHint L1H,
                                                        CacheHint L2H) {
  CacheHintsTy CacheHints = {static_cast<uint8_t>(L1H),
                             static_cast<uint8_t>(L2H)};
  return CacheHints;
}

template <typename T, unsigned BlockH, unsigned BlockW, unsigned NumBlocks>
Block2DTy<T, BlockH, BlockW, NumBlocks> __cm_intrinsic_impl_load_2d_ugm_desc(
    int16_t Pred, CacheHintsTy CacheHints, vector<uint32_t, 16> Desc,
    int32_t OffsetX, int32_t OffsetY,
    Block2DTy<T, BlockH, BlockW, NumBlocks> Passthru);

template <typename T, unsigned BlockH, unsigned BlockW, unsigned NumBlocks>
Block2DTy<T, BlockH, BlockW, NumBlocks, lsc::LoadOp::Transpose>
__cm_intrinsic_impl_load_2d_ugm_desc_transpose(
    int16_t Pred, CacheHintsTy CacheHints, vector<uint32_t, 16> Desc,
    int32_t OffsetX, int32_t OffsetY,
    Block2DTy<T, BlockH, BlockW, NumBlocks, lsc::LoadOp::Transpose> Passthru);

template <typename T, unsigned BlockH, unsigned BlockW, unsigned NumBlocks>
Block2DTy<T, BlockH, BlockW, NumBlocks, lsc::LoadOp::VNNI>
__cm_intrinsic_impl_load_2d_ugm_desc_vnni(
    int16_t Pred, CacheHintsTy CacheHints, vector<uint32_t, 16> Desc,
    int32_t OffsetX, int32_t OffsetY,
    Block2DTy<T, BlockH, BlockW, NumBlocks, lsc::LoadOp::VNNI> Passthru);

template <typename T, unsigned BlockH, unsigned BlockW, unsigned NumBlocks>
void __cm_intrinsic_impl_prefetch_2d_ugm_desc(int16_t Pred,
                                              CacheHintsTy CacheHints,
                                              vector<uint32_t, 16> Desc,
                                              int32_t OffsetX, int32_t OffsetY,
                                              T Dummy);

template <typename T, unsigned BlockH, unsigned BlockW, unsigned NumBlocks>
void __cm_intrinsic_impl_store_2d_ugm_desc(
    int16_t Pred, CacheHintsTy CacheHints, vector<uint32_t, 16> Desc,
    int32_t OffsetX, int32_t OffsetY,
    Block2DTy<T, BlockH, BlockW, NumBlocks> Src);

} // namespace details

template <lsc::LoadOp Op = lsc::LoadOp::Normal,
          CacheHint L1H = CacheHint::Default,
          CacheHint L2H = CacheHint::Default,
          int OffsetX = 0, int OffsetY = 0, typename T = int,
          unsigned NBlocks = 1, unsigned BlockH = 1, unsigned BlockW = 1>
CM_NODEBUG CM_INLINE void
cm_load(details::Block2DRefTy<T, BlockH, BlockW, NBlocks, Op> Res,
        const lsc::block_2d_desc<T, NBlocks, BlockH, BlockW> &Desc,
        int16_t Pred = 1) {
  using namespace details;
  using namespace lsc;

  check_lsc_block_2d_load_desc<Op, L1H, L2H, T, NBlocks, BlockH, BlockW>();
  auto CacheHints = get_cache_hint_vector(L1H, L2H);

  auto Payload = Desc.get_raw_desc();

  if constexpr (Op == LoadOp::Transpose) {
    Res = __cm_intrinsic_impl_load_2d_ugm_desc_transpose<T, BlockH, BlockW,
                                                         NBlocks>(
        Pred, CacheHints, Payload, OffsetX, OffsetY, Res);
  } else if constexpr (Op == LoadOp::VNNI) {
    Res = __cm_intrinsic_impl_load_2d_ugm_desc_vnni<T, BlockH, BlockW, NBlocks>(
        Pred, CacheHints, Payload, OffsetX, OffsetY, Res);
  } else {
    CM_STATIC_ERROR(Op == LoadOp::Normal, "Unsupported load operation");
    Res = __cm_intrinsic_impl_load_2d_ugm_desc<T, BlockH, BlockW, NBlocks>(
        Pred, CacheHints, Payload, OffsetX, OffsetY, Res);
  }
}

template <CacheHint L1H = CacheHint::Default,
          CacheHint L2H = CacheHint::Default,
          int OffsetX = 0, int OffsetY = 0, typename T = int,
          unsigned NBlocks = 1, unsigned BlockH = 1, unsigned BlockW = 1>
CM_NODEBUG CM_INLINE void
cm_load(details::Block2DRefTy<T, BlockH, BlockW, NBlocks> Res,
        const lsc::block_2d_desc<T, NBlocks, BlockH, BlockW> &Desc,
        int16_t Pred = 1) {
  cm_load<lsc::LoadOp::Normal, L1H, L2H, OffsetX, OffsetY, T, NBlocks, BlockH,
          BlockW>(Res, Desc, Pred);
}

template <CacheHint L1H = CacheHint::Default,
          CacheHint L2H = CacheHint::Default,
          int OffsetX = 0, int OffsetY = 0, typename T = int,
          unsigned NBlocks = 1, unsigned BlockH = 1, unsigned BlockW = 1>
CM_NODEBUG CM_INLINE void
cm_prefetch(const lsc::block_2d_desc<T, NBlocks, BlockH, BlockW> &Desc,
            int16_t Pred = 1) {
  using namespace details;
  using namespace lsc;

  check_lsc_block_2d_prefetch_desc<L1H, L2H, T, NBlocks, BlockH, BlockW>();
  auto CacheHints = get_cache_hint_vector(L1H, L2H);

  T Dummy; // Dummy variable to pass to the intrinsic. It is not used. It's left
           // uninitialized on purpose.

  auto Payload = Desc.get_raw_desc();
  __cm_intrinsic_impl_prefetch_2d_ugm_desc<T, BlockH, BlockW, NBlocks>(
      Pred, CacheHints, Payload, OffsetX, OffsetY, Dummy);
}

template <CacheHint L1H = CacheHint::Default,
          CacheHint L2H = CacheHint::Default,
          int OffsetX = 0, int OffsetY = 0, typename T = int,
          unsigned BlockH = 1, unsigned BlockW = 1>
CM_NODEBUG CM_INLINE void
cm_store(const lsc::block_2d_desc<T, 1, BlockH, BlockW> &Desc,
         details::Block2DTy<T, BlockH, BlockW> Src, int16_t Pred = 1) {
  using namespace details;
  using namespace lsc;

  check_lsc_block_2d_store_desc<L1H, L2H, T, BlockH, BlockW>();
  auto CacheHints = get_cache_hint_vector(L1H, L2H);

  auto Payload = Desc.get_raw_desc();

  __cm_intrinsic_impl_store_2d_ugm_desc<T, BlockH, BlockW, 1>(
      Pred, CacheHints, Payload, OffsetX, OffsetY, Src);
}

#endif // _CLANG_CM_LSC_BLOCK2D_H_
