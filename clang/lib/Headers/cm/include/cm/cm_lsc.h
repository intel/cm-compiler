/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_lsc.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_LSC_H_
#define _CLANG_CM_LSC_H_

#include "cm_common.h"
#include "cm_internal.h"
#include "cm_has_instr.h"

#define CM_LSC_REPLICATE_MASK(VectorSize)                                      \
  __attribute__((genx_replicate_mask(details::lsc_vector_size<VectorSize>())))

/// \brief Data prefetch.
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param N The number of channels (platform dependent)
///
/// @param Idx Surface index, which must correspond to a buffer.
///
/// @param Offset zero based offset of the input buffer in bytes.
///
/// @param Pred Predicate
///
template <VectorSize VS = VectorSize::N1, DataSize DS = DataSize::U32,
          CacheHint L1H = CacheHint::Cached,
          CacheHint L3H = CacheHint::Cached,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void cm_prefetch(SurfaceIndex Idx,
                                      vector<unsigned, N> Offset,
                                      vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Prefetch, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr int ImmOffset = 0;
  __cm_intrinsic_impl_prefetch_bti<DS, VS, ImmOffset, L1H, L3H, N>(Idx, Offset,
                                                                   Pred);
}

/// flat-address prefetch
template <VectorSize VS = VectorSize::N1, DataSize DS = DataSize::U32,
          CacheHint L1H = CacheHint::Cached,
          CacheHint L3H = CacheHint::Cached,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void cm_ptr_prefetch(const unsigned *const Ptr,
                                          vector<unsigned, N> Offset,
                                          vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Prefetch, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr int ImmOffset = 0;
  uint64_t Addr = (uint64_t)Ptr;
  __cm_intrinsic_impl_prefetch_flat<DS, VS, ImmOffset, L1H, L3H, N>(
      Addr, Offset, Pred);
}

/// Surface-based Block prefetch.
template <VectorSize VS, DataSize DS = DataSize::U32,
          CacheHint L1H = CacheHint::Cached,
          CacheHint L3H = CacheHint::Cached>
CM_NODEBUG CM_INLINE void cm_prefetch(SurfaceIndex Idx, unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_WARNING(details::always_false<decltype(VS)>(),
                    "Please use new interface with explicit NElts");
  CM_STATIC_ERROR(DS == DataSize::U32 || DS == DataSize::U64,
                  "Transposed prefetch can work only with U32 and U64 data sizes");
  CM_STATIC_ERROR(
      (details::lsc_check_cache_hint<details::LSCAction::Prefetch, L1H, L3H>()),
      "unsupported cache hint");
  constexpr int _ImmOffset = 0;
  details::__cm_intrinsic_impl_block_prefetch_bti<DS, VS, _ImmOffset, L1H, L3H>(
      Idx, Offset);
}

// Surface-based block prefetch, new interface
template <int NElts, DataSize DS = DataSize::U32,
          CacheHint L1H = CacheHint::Cached, CacheHint L3H = CacheHint::Cached>
CM_NODEBUG CM_INLINE void cm_prefetch(SurfaceIndex Idx, unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_ERROR(DS == DataSize::U32 || DS == DataSize::U64,
                  "Transposed prefetch can work only with U32 and U64 data sizes");
  CM_STATIC_ERROR(
      (details::lsc_check_cache_hint<details::LSCAction::Prefetch, L1H, L3H>()),
      "unsupported cache hint");
  constexpr int _ImmOffset = 0;
  constexpr VectorSize VS = details::lsc_vector_size<NElts>();
  details::__cm_intrinsic_impl_block_prefetch_bti<DS, VS, _ImmOffset, L1H, L3H>(
      Idx, Offset);
}

/// Flat-address Block prefetch.
template <VectorSize VS, DataSize DS = DataSize::U32,
          CacheHint L1H = CacheHint::Cached,
          CacheHint L3H = CacheHint::Cached>
CM_NODEBUG CM_INLINE void cm_ptr_prefetch(const unsigned *const Ptr,
                                          unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_WARNING(details::always_false<decltype(VS)>(),
                    "Please use new interface with explicit NElts");
  CM_STATIC_ERROR(DS == DataSize::U32 || DS == DataSize::U64,
                  "Transposed prefetch can work only with U32 and U64 data sizes");
  CM_STATIC_ERROR(
      (details::lsc_check_cache_hint<details::LSCAction::Prefetch, L1H, L3H>()),
      "unsupported cache hint");
  constexpr int _ImmOffset = 0;
  uint64_t _Addr = (uint64_t)Ptr;
  details::__cm_intrinsic_impl_block_prefetch_flat<DS, VS, _ImmOffset, L1H,
                                                   L3H>(_Addr, Offset);
}

/// Flat-address Block prefetch, new interface
template <int NElts, DataSize DS = DataSize::U32,
          CacheHint L1H = CacheHint::Cached, CacheHint L3H = CacheHint::Cached>
CM_NODEBUG CM_INLINE void cm_ptr_prefetch(const unsigned *const Ptr,
                                          unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_ERROR(DS == DataSize::U32 || DS == DataSize::U64,
                  "Transposed prefetch can work only with U32 and U64 data sizes");
  CM_STATIC_ERROR(
      (details::lsc_check_cache_hint<details::LSCAction::Prefetch, L1H, L3H>()),
      "unsupported cache hint");
  constexpr int ImmOffset = 0;
  constexpr VectorSize VS = details::lsc_vector_size<NElts>();
  uint64_t Addr = (uint64_t)Ptr;
  details::__cm_intrinsic_impl_block_prefetch_flat<DS, VS, ImmOffset, L1H,
                                                   L3H>(Addr, Offset);
}

/// \brief Data Read.
///
/// @param T The return element data type.
///
/// @param N The number of channels (platform dependent)
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Pred Predicate
///
/// @param Idx Surface index, which must correspond to a buffer.
///
/// @param Offset zero based offset of the input buffer in bytes.
///
/// BTI non-transposed load
template <typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE CM_LSC_REPLICATE_MASK(VS) auto cm_load(
    SurfaceIndex Idx, vector<unsigned, N> Offset, vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Load, L1H, L3H>()),
                  "unsupported cache hint");
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, VS);
  using _MessTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  auto _TmpRes =
      __cm_intrinsic_impl_load_bti<_MessTy, _DS, VS, _ImmOffset, L1H, L3H,
                                   _Transposed, N>(Idx, Offset, Pred);
  return lsc_format_ret<T, _MessTy, _RetTy>(_TmpRes);
}

/// Flat-address non-transposed load
template <typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto cm_ptr_load(const T *const Ptr,
                                      vector<unsigned, N> Offset,
                                      vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Load, L1H, L3H>()),
                  "unsupported cache hint");
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, VS);
  using _MessTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  uint64_t _Addr = (uint64_t)Ptr;
  auto _TmpRes =
      __cm_intrinsic_impl_load_flat<_MessTy, _DS, VS, _ImmOffset, L1H, L3H,
                                    _Transposed, N>(_Addr, Offset, Pred);
  return lsc_format_ret<T, _MessTy, _RetTy>(_TmpRes);
}

// Block-load with a SurfaceIndex
template <typename T, VectorSize VS, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_NODEBUG CM_INLINE auto cm_load(SurfaceIndex Idx, unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_WARNING(details::always_false<decltype(VS)>(),
                    "Please use new interface with explicit NElts");
  CM_STATIC_ERROR(
      (details::lsc_check_cache_hint<details::LSCAction::Load, L1H, L3H>()),
      "unsupported cache hint");
  using _RetTy = decltype(details::lsc_data_type<T, 1, VS>());
  static_assert(VS != VectorSize::N0, "invalid vector size");
  constexpr DataSize _DS = details::lsc_data_size<T, DS>();
  CM_STATIC_ERROR(_DS == DataSize::U32 || _DS == DataSize::U64,
                  "Transposed load can work only with U32 and U64 data sizes");
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = true;
  return details::__cm_intrinsic_impl_block_load_bti<
      _RetTy, _DS, VS, _ImmOffset, L1H, L3H, _Transposed>(Idx, Offset);
}

// Block-load with a SurfaceIndex, new interface
template <typename T, int NElts, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_NODEBUG CM_INLINE auto cm_load(SurfaceIndex Idx, unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_ERROR(
      (details::lsc_check_cache_hint<details::LSCAction::Load, L1H, L3H>()),
      "unsupported cache hint");
  constexpr VectorSize VS = details::lsc_vector_size<NElts>();
  using _RetTy = decltype(details::lsc_data_type<T, 1, VS>());
  static_assert(VS != VectorSize::N0, "invalid vector size");
  constexpr DataSize _DS = details::lsc_data_size<T, DS>();
  CM_STATIC_ERROR(_DS == DataSize::U32 || _DS == DataSize::U64,
                  "Transposed load can work only with U32 and U64 data sizes");
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = true;
  return details::__cm_intrinsic_impl_block_load_bti<
      _RetTy, _DS, VS, _ImmOffset, L1H, L3H, _Transposed>(Idx, Offset);
}

// Block-load with a base-pointer to the buffer
template <typename T, VectorSize VS, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_NODEBUG CM_INLINE auto cm_ptr_load(const T *const Ptr, unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_ERROR(
      (details::lsc_check_cache_hint<details::LSCAction::Load, L1H, L3H>()),
      "unsupported cache hint");
  CM_STATIC_WARNING(details::always_false<decltype(VS)>(),
                    "Please use new interface with explicit NElts");
  using _RetTy = decltype(details::lsc_data_type<T, 1, VS>());
  static_assert(VS != VectorSize::N0, "invalid vector size");
  constexpr DataSize _DS = details::lsc_data_size<T, DS>();
  CM_STATIC_ERROR(_DS == DataSize::U32 || _DS == DataSize::U64,
                  "Transposed load can work only with U32 and U64 data sizes");
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = true;
  uint64_t _Addr = (uint64_t)Ptr;
  return details::__cm_intrinsic_impl_block_load_flat<
      _RetTy, _DS, VS, _ImmOffset, L1H, L3H, _Transposed>(_Addr, Offset);
}
// Block-load with a base-pointer to the buffer, new interface
template <typename T, int NElts, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_NODEBUG CM_INLINE auto cm_ptr_load(const T *const Ptr, unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_ERROR(
      (details::lsc_check_cache_hint<details::LSCAction::Load, L1H, L3H>()),
      "unsupported cache hint");
  constexpr VectorSize VS = details::lsc_vector_size<NElts>();
  using _RetTy = decltype(details::lsc_data_type<T, 1, VS>());
  static_assert(VS != VectorSize::N0, "invalid vector size");
  constexpr DataSize _DS = details::lsc_data_size<T, DS>();
  CM_STATIC_ERROR(_DS == DataSize::U32 || _DS == DataSize::U64,
                  "Transposed load can work only with U32 and U64 data sizes");
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = true;
  uint64_t _Addr = (uint64_t)Ptr;
  return details::__cm_intrinsic_impl_block_load_flat<
      _RetTy, _DS, VS, _ImmOffset, L1H, L3H, _Transposed>(_Addr, Offset);
}

/// BTI non-transposed quad load
///   * vector size is always 4 for quad so it is not specified
///   * store is always transposed, so no block version
template <typename T, ChannelMaskType Mask, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto cm_load4(SurfaceIndex Idx, vector<unsigned, N> Offset,
                                   vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Load, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr VectorSize _VS =
      details::lsc_get_vector_size_from_channel_mask<Mask>();
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, _VS);
  using _MessTy = decltype(lsc_data_type_ext<T, N, _VS>());
  using _RetTy = decltype(lsc_data_type<T, N, _VS>());
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  auto _TmpRes =
      __cm_intrinsic_impl_load4_bti<_MessTy, _DS, _VS, _ImmOffset,
                                    L1H, L3H, _Transposed, N>(Idx, Offset, Pred,
                                                              Mask);
  return lsc_format_ret<T, _MessTy, _RetTy>(_TmpRes);
}

/// Flat-address non-transposed quad load
template <typename T, ChannelMaskType Mask, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto cm_ptr_load4(const T *const Ptr,
                                       vector<unsigned, N> Offset,
                                       vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Load, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr VectorSize _VS =
      details::lsc_get_vector_size_from_channel_mask<Mask>();
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, _VS);
  using _MessTy = decltype(lsc_data_type_ext<T, N, _VS>());
  using _RetTy = decltype(lsc_data_type_ext<T, N, _VS>());
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  uint64_t _Addr = (uint64_t)Ptr;
  auto _TmpRes =
      __cm_intrinsic_impl_load4_flat<_MessTy, _DS, _VS, _ImmOffset, L1H, L3H,
                                     _Transposed, N>(_Addr, Offset, Pred, Mask);
  return lsc_format_ret<T, _MessTy, _RetTy>(_TmpRes);
}

/// \brief Data Write.
///
/// @param T The element data type.
///
/// @param N The number of channels (platform dependent)
///
/// @param NElts The number of element to store (for block store only)
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Pred Predicate
///
/// @param Idx Surface index, which must correspond to a buffer.
///
/// @param Offset zero based offset of the output buffer in bytes.
///
/// @param Data data to write.
///
template <typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void
cm_store(SurfaceIndex Idx, vector<unsigned, N> Offset,
         vector<T, N * details::lsc_vector_size<VS>()> Data,
         vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Store, L1H, L3H>()),
                  "unsupported cache hint");
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, VS);
  constexpr DataSize _DS = lsc_expand_ds(details::lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  using _StTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _StTy _TmpData = Data.format<_CastTy>();
  details::__cm_intrinsic_impl_store_bti<typename lsc_expand_type<T>::type, _DS,
                                         VS, _ImmOffset, L1H, L3H, _Transposed,
                                         N>(Idx, Offset, _TmpData, Pred);
}
/// Flat-address store using a base-address to a buffer
template <typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void
cm_ptr_store(T *Ptr, vector<unsigned, N> Offset,
             vector<T, N * details::lsc_vector_size<VS>()> Data,
             vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Store, L1H, L3H>()),
                  "unsupported cache hint");
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, VS);
  constexpr DataSize _DS = lsc_expand_ds(details::lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  uint64_t _Addr = (uint64_t)Ptr;
  using _StTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _StTy _TmpData = Data.format<_CastTy>();
  details::__cm_intrinsic_impl_store_flat<typename lsc_expand_type<T>::type,
                                          _DS, VS, _ImmOffset, L1H, L3H,
                                          _Transposed, N>(_Addr, Offset,
                                                          _TmpData, Pred);
}

/// Quad version of BTI store:
///   * vector size is always 4 for quad so it is not specified
///   * store is always transposed, so no block version
template <typename T, ChannelMaskType Mask, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void cm_store4(
    SurfaceIndex Idx, vector<unsigned, N> Offset,
    vector<T, N * details::lsc_get_num_elements_from_channel_mask<Mask>()> Data,
    vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Store, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr VectorSize _VS =
      details::lsc_get_vector_size_from_channel_mask<Mask>();
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, _VS);
  constexpr DataSize _DS = lsc_expand_ds(details::lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  using _StTy = decltype(lsc_data_type_ext<T, N, _VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _StTy _TmpData = Data.format<_CastTy>();
  details::__cm_intrinsic_impl_store4_bti<typename lsc_expand_type<T>::type,
                                          _DS, _VS, _ImmOffset, L1H,
                                          L3H, _Transposed, N>(
      Idx, Offset, _TmpData, Pred, Mask);
}

/// Quad version of flat store
template <typename T, ChannelMaskType Mask, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void cm_ptr_store4(
    T *Ptr, vector<unsigned, N> Offset,
    vector<T, N * details::lsc_get_num_elements_from_channel_mask<Mask>()> Data,
    vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Store, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr VectorSize _VS =
      details::lsc_get_vector_size_from_channel_mask<Mask>();
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, _VS);
  constexpr DataSize _DS = lsc_expand_ds(details::lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  uint64_t _Addr = (uint64_t)Ptr;
  using _StTy = decltype(lsc_data_type_ext<T, N, _VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _StTy _TmpData = Data.format<_CastTy>();
  details::__cm_intrinsic_impl_store4_flat<typename lsc_expand_type<T>::type,
                                           _DS, _VS, _ImmOffset, L1H, L3H,
                                           _Transposed, N>(
      _Addr, Offset, _TmpData, Pred, Mask);
}

/// Block store with a SurfaceIndex.
template <typename T, int NElts, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_NODEBUG CM_INLINE void cm_store(SurfaceIndex Idx, unsigned Offset,
                                   vector<T, NElts> Data) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Store, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr DataSize _DS = lsc_data_size<T, DS>();
  CM_STATIC_ERROR(_DS == DataSize::U32 || _DS == DataSize::U64,
                  "Transposed store can work only with U32 and U64 data sizes");
  constexpr VectorSize _VS = lsc_vector_size<NElts>();
  static_assert(_VS != VectorSize::N0, "invalid vector size");
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = true;
  __cm_intrinsic_impl_block_store_bti<T, _DS, _VS, _ImmOffset, L1H, L3H,
                                      _Transposed>(Idx, Offset, Data);
}

/// Block store with a base pointer.
template <typename T, int NElts, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_NODEBUG CM_INLINE void cm_ptr_store(T *ptr, unsigned Offset,
                                       vector<T, NElts> Data) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Store, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr DataSize _DS = lsc_data_size<T, DS>();
  CM_STATIC_ERROR(_DS == DataSize::U32 || _DS == DataSize::U64,
                  "Transposed store can work only with U32 and U64 data sizes");
  constexpr VectorSize _VS = lsc_vector_size<NElts>();
  static_assert(_VS != VectorSize::N0, "invalid vector size");
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = true;
  uint64_t _Addr = (uint64_t)ptr;
  __cm_intrinsic_impl_block_store_flat<T, _DS, _VS, _ImmOffset, L1H, L3H,
                                       _Transposed>(_Addr, Offset, Data);
}

/// \brief SLM Data Read.
///
/// @param T The return element data type.
///
/// @param N The number of channels (platform dependent)
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param Pred Predicate
///
/// @param Offset zero based offset of the input SLM buffer in bytes.
///

// Non-transposed SLM load
template <typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto cm_load_slm(vector<unsigned, N> Offset,
                                      vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, VS);
  using _MessTy = decltype(details::lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  constexpr DataSize _DS = lsc_expand_ds(details::lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  auto _TmpRes =
      details::__cm_intrinsic_impl_load_slm<_MessTy, _DS, VS, _ImmOffset,
                                            _Transposed>(Offset, Pred);
  return lsc_format_ret<T, _MessTy, _RetTy>(_TmpRes);
}

// Block-load with a base-pointer to the SLM
template <typename T, VectorSize VS, DataSize DS = DataSize::Default>
CM_NODEBUG CM_INLINE auto cm_load_slm(unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_WARNING(details::always_false<decltype(VS)>(),
                    "Please use new interface with explicit NElts");
  using namespace details;
  using _RetTy = decltype(lsc_data_type<T, 1, VS>());
  static_assert(VS != VectorSize::N0, "invalid vector size");
  constexpr DataSize _DS = lsc_data_size<T, DS>();
  CM_STATIC_ERROR(_DS == DataSize::U32 || _DS == DataSize::U64,
                  "Transposed load can work only with U32 and U64 data sizes");
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = true;
  return __cm_intrinsic_impl_block_load_slm<_RetTy, _DS, VS, _ImmOffset,
                                            _Transposed>(Offset);
}

// Block-load with a base-pointer to the SLM, new interface
template <typename T, int NElts, DataSize DS = DataSize::Default>
CM_NODEBUG CM_INLINE auto cm_load_slm(unsigned Offset) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  constexpr VectorSize VS = details::lsc_vector_size<NElts>();
  using _RetTy = decltype(lsc_data_type<T, 1, VS>());
  static_assert(VS != VectorSize::N0, "invalid vector size");
  constexpr DataSize _DS = lsc_data_size<T, DS>();
  CM_STATIC_ERROR(_DS == DataSize::U32 || _DS == DataSize::U64,
                  "Transposed load can work only with U32 and U64 data sizes");
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = true;
  return __cm_intrinsic_impl_block_load_slm<_RetTy, _DS, VS, _ImmOffset,
                                            _Transposed>(Offset);
}

// Non-transposed SLM quad load
template <typename T, ChannelMaskType Mask, DataSize DS = DataSize::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto cm_load4_slm(vector<unsigned, N> Offset,
                                       vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  constexpr VectorSize _VS =
      details::lsc_get_vector_size_from_channel_mask<Mask>();
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, _VS);
  using _MessTy = decltype(details::lsc_data_type_ext<T, N, _VS>());
  using _RetTy = decltype(lsc_data_type<T, N, _VS>());
  constexpr DataSize DS_ = lsc_expand_ds(details::lsc_data_size<T, DS>());
  constexpr int ImmOffset = 0;
  constexpr bool Transposed = false;
  auto _TmpRes =
      details::__cm_intrinsic_impl_load4_slm<_MessTy, DS_, _VS,
                                             ImmOffset, Transposed>(Offset,
                                                                    Pred, Mask);
  auto _Formatted = _TmpRes.format<T>();
  constexpr int stride = _Formatted.n_elems() / _TmpRes.n_elems();
  _RetTy _Res = _Formatted.select<_TmpRes.n_elems(), stride>(0);
  return _Res;
}

/// \brief SLM Data Write.
///
/// @param T The element data type.
///
/// @param N The number of channels (platform dependent)
///
/// @param NElts The number of element to store (for block store only)
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param Pred Predicate
///
/// @param Offset zero based offset of the output SLM buffer in bytes.
///
/// @param Data data to write.
///
template <typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void
cm_store_slm(vector<unsigned, N> Offset,
             vector<T, N * details::lsc_vector_size<VS>()> Data,
             vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  CM_STATIC_WARNING(details::always_false<decltype(VS)>(),
                    "Please use new interface with explicit NElts");
  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, VS);
  constexpr DataSize _DS = lsc_expand_ds(details::lsc_data_size<T, DS>());
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  using _StTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _StTy _TmpData = Data.format<_CastTy>();
  details::__cm_intrinsic_impl_store_slm<typename lsc_expand_type<T>::type, _DS,
                                         VS, _ImmOffset, _Transposed, N>(
      Offset, _TmpData, Pred);
}

// explicit NElts version
template <typename T, int NElts, DataSize DS = DataSize::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void
cm_store_slm(vector<unsigned, N> Offset,
             vector<T, N * details::lsc_vector_size<NElts>()> Data,
             vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(
      N, details::lsc_vector_size<NElts>());
  constexpr DataSize DS_ = lsc_expand_ds(details::lsc_data_size<T, DS>());
  constexpr int ImmOffset = 0;
  constexpr bool Transposed = false;
  constexpr VectorSize VS = details::lsc_vector_size<NElts>();
  using _StTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _StTy _TmpData = Data.format<_CastTy>();
  details::__cm_intrinsic_impl_store_slm<typename lsc_expand_type<T>::type, DS_,
                                         VS, ImmOffset, Transposed, N>(
      Offset, _TmpData, Pred);
}

// Block version.
template <typename T, int NElts, DataSize DS = DataSize::Default>
CM_NODEBUG CM_INLINE void cm_store_slm(unsigned Offset, vector<T, NElts> Data) {
  CM_HAS_LSC_CONTROL;

  constexpr DataSize _DS = details::lsc_data_size<T, DS>();
  CM_STATIC_ERROR(_DS == DataSize::U32 || _DS == DataSize::U64,
                  "Transposed store can work only with U32 and U64 data sizes");
  constexpr VectorSize _VS = details::lsc_vector_size<NElts>();
  static_assert(_VS != VectorSize::N0, "invalid vector size");
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = true;
  details::__cm_intrinsic_impl_block_store_slm<T, _DS, _VS, _ImmOffset,
                                               _Transposed>(Offset, Data);
}

// Quad version
template <typename T, ChannelMaskType Mask, DataSize DS = DataSize::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void cm_store4_slm(
    vector<unsigned, N> Offset,
    vector<T, N * details::lsc_get_num_elements_from_channel_mask<Mask>()> Data,
    vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  constexpr DataSize _DS = lsc_expand_ds(details::lsc_data_size<T, DS>());
  constexpr VectorSize _VS =
      details::lsc_get_vector_size_from_channel_mask<Mask>();
  CM_HAS_LSC_NON_TRANSPOSE_MESSAGES_WITH_NON_DEFAULT_SIMT_CONTROL(N, _VS);
  constexpr int _ImmOffset = 0;
  constexpr bool _Transposed = false;
  using _StTy = decltype(lsc_data_type_ext<T, N, _VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _StTy _TmpData = Data.format<_CastTy>();
  details::__cm_intrinsic_impl_store4_slm<typename lsc_expand_type<T>::type,
                                          _DS, _VS, _ImmOffset, _Transposed, N>(
      Offset, _TmpData, Pred, Mask);
}

/// \brief 2D Block Read (flat)
///
/// @param T The element data type.
///
/// @param N The data size
///
/// @param Width The block width in number of elements
///
/// @param Height The block height
///
/// @param NBlks, The number of blocks
///
/// @param Transposed Is Transposed or not
///
/// @param Transformed apply VNNI transform or not
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Ptr Surface base address
///
/// @param SurfaceWidth the surface width minus 1 in bytes
///
/// @param SurfaceHeight the surface height minus 1 in rows
///
/// @param SurfacePitch the surface pitch minus 1 in bytes
///
/// @param X zero based X-coordinate of the left upper rectangle corner in
/// number of elements.
///
/// @param Y zero based Y-coordinate of the left upper rectangle corner in rows.
///
/// @param Data Data to store.
///
/// @return vector of type T and size N. Size is specified with padding.
/// see details::getBlock2dDataSize for details
template <typename T, int Width, int Height = 1, int NBlks = 1,
          bool Transposed = false, bool Transformed = false,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::getBlock2dDataSize<T, NBlks, Height, Width,
                                              Transposed, Transformed>()>
CM_NODEBUG CM_INLINE vector<T, N>
cm_ptr_load(T *Ptr, unsigned SurfaceWidth, unsigned SurfaceHeight,
            unsigned SurfacePitch, int X, int Y) {
  CM_HAS_LSC_UNTYPED_2D_CONTROL;

  CM_STATIC_ERROR(!Transposed || !Transformed,
                  "Transposed and transformed is not supported");
  CM_STATIC_ERROR(!Transposed || (Transposed && NBlks == 1),
                  "Transposed expected to be 1 block only");
  CM_STATIC_ERROR(
      (details::lsc_check_cache_hint<details::LSCAction::Load, L1H, L3H>()),
      "unsupported cache hint");
  uintptr_t Base = reinterpret_cast<uintptr_t>(Ptr);

  // Calculate number of elements with padding
  constexpr int vnni_elements = sizeof(uint32_t) / sizeof(T);
  constexpr int grf_width = Transposed    ? Height
                            : Transformed ? Width * vnni_elements
                                          : Width;
  constexpr int grf_row_pitch = details::getNextPowerOf2(grf_width);
  constexpr int grf_height =
      Transposed ? Width
                 : (Transformed ? (Height + vnni_elements - 1) / vnni_elements
                                : Height);
  constexpr int grf_block_elements = grf_row_pitch * grf_height;
  constexpr int grf_block_pitch =
      details::roundUpNextMultiple(grf_block_elements, 64 / sizeof(T));
  constexpr int grf_elements = grf_block_pitch * NBlks;

  constexpr int dst_block_elements = grf_width * grf_height;
  constexpr int dst_elements = dst_block_elements * NBlks;

  CM_STATIC_ERROR(N == grf_elements || N == dst_elements,
                  "Incorrect element count");

  vector<T, grf_elements> raw = details::__cm_intrinsic_impl_block_load2d_flat<
      T, NBlks, Width, Height, Transposed, Transformed, L1H, L3H, grf_elements>(
      Base, SurfaceWidth, SurfaceHeight, SurfacePitch, X, Y);

  // If no padding is observed, then return as read
  if constexpr (grf_elements == N)
    return raw;

  // HW restrictions force data which is read to contain padding filled with
  // garbage for 2d lsc loads. This code eliminates such padding. For details
  // see documentation for LSC_UNTYPED (LOAD_BLOCK2D).
  vector<T, dst_elements> dst;

#pragma unroll
  for (int i = 0; i < NBlks; i++) {
    auto dst_block =
        dst.template select<dst_block_elements, 1>(i * dst_block_elements);

    auto raw_block =
        raw.template select<grf_block_elements, 1>(i * grf_block_pitch);
    auto raw_block_2d =
        raw_block.template format<T, grf_height, grf_row_pitch>();

    dst_block = raw_block_2d.template select<grf_height, 1, grf_width, 1>(0, 0);
  }

  return dst;
}

// convenient overload to not break legacy
template <typename T, int Width, int Height = 1, int NBlks = 1,
          bool Transposed = false, bool Transformed = false,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::getBlock2dDataSize<T, NBlks, Height, Width,
                                              Transposed, Transformed>()>
CM_NODEBUG CM_INLINE vector<T, N> cm_load(T *Ptr, unsigned SurfaceWidth,
                                          unsigned SurfaceHeight,
                                          unsigned SurfacePitch, int X, int Y) {
  return cm_ptr_load<T, Width, Height, NBlks, Transposed, Transformed, L1H, L3H,
                     N>(Ptr, SurfaceWidth, SurfaceHeight, SurfacePitch, X, Y);
}

/// \brief 2D Block Prefetch (flat)
template <typename T, int Width, int Height = 1, int NBlks = 1,
          CacheHint L1H = CacheHint::Cached,
          CacheHint L3H = CacheHint::Cached>
CM_NODEBUG CM_INLINE void cm_ptr_prefetch(T *Ptr, unsigned SurfaceWidth,
                                          unsigned SurfaceHeight,
                                          unsigned SurfacePitch, int X, int Y) {
  CM_HAS_LSC_UNTYPED_2D_CONTROL;

  using namespace details;
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Prefetch, L1H, L3H>()),
                  "unsupported cache hint");
  uintptr_t Base = reinterpret_cast<uintptr_t>(Ptr);
  __cm_intrinsic_impl_block_prefetch2d_flat<T, NBlks, Width, Height, L1H, L3H>(
      Base, SurfaceWidth, SurfaceHeight, SurfacePitch, X, Y);
}

/// convenient overload to not break legacy
template <typename T, int Width, int Height = 1, int NBlks = 1,
          CacheHint L1H = CacheHint::Cached,
          CacheHint L3H = CacheHint::Cached>
CM_NODEBUG CM_INLINE void cm_prefetch(T *Ptr, unsigned SurfaceWidth,
                                      unsigned SurfaceHeight,
                                      unsigned SurfacePitch, int X, int Y) {
  return cm_ptr_prefetch<T, Width, Height, NBlks, L1H, L3H>(
      Ptr, SurfaceWidth, SurfaceHeight, SurfacePitch, X, Y);
}

/// \brief 2D Block Store (flat)
template <typename T, int Width, int Height = 1,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::getBlock2dDataSize<T, 1 /*NBlks*/, Height, Width,
                                              false /*Transposed*/,
                                              false /*Transformed*/>()>
CM_NODEBUG CM_INLINE void
cm_ptr_store(T *Ptr, unsigned SurfaceWidth, unsigned SurfaceHeight,
             unsigned SurfacePitch, int X, int Y, vector<T, N> Data) {
  CM_HAS_LSC_UNTYPED_2D_CONTROL;

  using namespace details;
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Store, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr int NBlks = 1;
  uintptr_t Base = reinterpret_cast<uintptr_t>(Ptr);

  constexpr int Pitch = details::getNextPowerOf2(Width);
  matrix<T, Height, Pitch> raw;

  if constexpr (raw.n_elems() == Data.n_elems())
    raw = Data;
  else {
    auto data_2d = Data.template format<T, Height, Width>();
    raw.template select<Height, 1, Width, 1>(0, 0) = data_2d;
  }

  __cm_intrinsic_impl_block_store2d_flat<T, NBlks, Width, Height, L1H, L3H,
                                         raw.n_elems()>(
      Base, SurfaceWidth, SurfaceHeight, SurfacePitch, X, Y,
      raw.template format<T>());
}

/// convenient overload to not break legacy
template <typename T, int Width, int Height = 1,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::getBlock2dDataSize<T, 1 /*NBlks*/, Height, Width,
                                              false /*Transposed*/,
                                              false /*Transformed*/>()>
CM_NODEBUG CM_INLINE void
cm_store(T *Ptr, unsigned SurfaceWidth, unsigned SurfaceHeight,
         unsigned SurfacePitch, int X, int Y, vector<T, N> Data) {
  cm_ptr_store<T, Width, Height, L1H, L3H, N>(
      Ptr, SurfaceWidth, SurfaceHeight, SurfacePitch, X, Y, Data);
}


/// \brief LSC Atomic.
///
/// @param T The element data type.
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Pred Predicate
///
/// @param Idx Surface index, which must correspond to a buffer.
///
/// @param Offset zero based byte offset of the input buffer or SLM byte offset
///
template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto cm_atomic(SurfaceIndex Idx,
                                    vector<unsigned, N> Offset,
                                    vector<ushort, N> Pred = 1) ->
    typename std::enable_if<
        details::lsc_atomic_nsrcs<Op>() == 0,
        vector<T, N * details::lsc_vector_size<VS>()> >::type {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Atomic, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr bool _Transposed = false;
  using _IntRetTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  auto _TmpRes =
      __cm_intrinsic_impl_lsc_atomic_bti<Op, _DS, VS, _Transposed, L1H, L3H,
                                         _IntRetTy, N>(Pred, Idx, Offset);
  return lsc_format_ret<T, _IntRetTy, _RetTy>(_TmpRes);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto
cm_atomic(SurfaceIndex Idx, vector<unsigned, N> Offset,
          vector<T, N * details::lsc_vector_size<VS>()> Src0,
          vector<ushort, N> Pred = 1) ->
    typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 1,
                            decltype(Src0)>::type {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Atomic, L1H, L3H>()),
                  "unsupported cache hint");
  CM_STATIC_ERROR(lsc_check_atomic_src<T>(),
                  "unsupported type for lsc atomic source or dest arguments");
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr bool _Transposed = false;
  using _IntRetTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _SrcTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  _SrcTy _TmpSrc0 = Src0.format<_CastTy>();
  auto _TmpRes = __cm_intrinsic_impl_lsc_atomic_bti<Op, _DS, VS, _Transposed,
                                                    L1H, L3H, _IntRetTy, N>(
      Pred, Idx, Offset, _TmpSrc0);
  return lsc_format_ret<T, _IntRetTy, _RetTy>(_TmpRes);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto
cm_atomic(SurfaceIndex Idx, vector<unsigned, N> Offset,
          vector<T, N * details::lsc_vector_size<VS>()> Src0,
          vector<T, N * details::lsc_vector_size<VS>()> Src1,
          vector<ushort, N> Pred = 1) ->
    typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 2,
                            decltype(Src0)>::type {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Atomic, L1H, L3H>()),
                  "unsupported cache hint");
  CM_STATIC_ERROR(lsc_check_atomic_src<T>(),
                  "unsupported type for lsc atomic source or dest arguments");
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  using _IntRetTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _SrcTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _SrcTy _TmpSrc0 = Src0.format<_CastTy>();
  _SrcTy _TmpSrc1 = Src1.format<_CastTy>();
  constexpr bool _Transposed = false;
  auto _TmpRes = __cm_intrinsic_impl_lsc_atomic_bti<Op, _DS, VS, _Transposed,
                                                    L1H, L3H, _IntRetTy, N>(
      Pred, Idx, Offset, _TmpSrc0, _TmpSrc1);
  return lsc_format_ret<T, _IntRetTy, _RetTy>(_TmpRes);
}

// flat-address atomic
template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto cm_ptr_atomic(T *Ptr, vector<unsigned, N> Offset,
                                        vector<ushort, N> Pred = 1) ->
    typename std::enable_if<
        details::lsc_atomic_nsrcs<Op>() == 0,
        vector<T, N * details::lsc_vector_size<VS>()> >::type {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Atomic, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr bool _Transposed = false;
  uint64_t _Addr = (uint64_t)Ptr;
  using _IntRetTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  auto _TmpRes =
      __cm_intrinsic_impl_lsc_atomic_flat<Op, _DS, VS, _Transposed, L1H, L3H,
                                          _IntRetTy, N>(Pred, _Addr, Offset);
  return lsc_format_ret<T, _IntRetTy, _RetTy>(_TmpRes);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto
cm_ptr_atomic(T *Ptr, vector<unsigned, N> Offset,
              vector<T, N * details::lsc_vector_size<VS>()> Src0,
              vector<ushort, N> Pred = 1) ->
    typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 1,
                            decltype(Src0)>::type {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Atomic, L1H, L3H>()),
                  "unsupported cache hint");
  CM_STATIC_ERROR(lsc_check_atomic_src<T>(),
                  "unsupported type for lsc atomic source or dest arguments");
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr bool _Transposed = false;
  uint64_t _Addr = (uint64_t)Ptr;
  using _IntRetTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _SrcTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _SrcTy _TmpSrc0 = Src0.format<_CastTy>();
  auto _TmpRes = __cm_intrinsic_impl_lsc_atomic_flat<Op, _DS, VS, _Transposed,
                                                     L1H, L3H, _IntRetTy, N>(
      Pred, _Addr, Offset, _TmpSrc0);
  return lsc_format_ret<T, _IntRetTy, _RetTy>(_TmpRes);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto
cm_ptr_atomic(T *Ptr, vector<unsigned, N> Offset,
              vector<T, N * details::lsc_vector_size<VS>()> Src0,
              vector<T, N * details::lsc_vector_size<VS>()> Src1,
              vector<ushort, N> Pred = 1) ->
    typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 2,
                            decltype(Src0)>::type {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Atomic, L1H, L3H>()),
                  "unsupported cache hint");
  CM_STATIC_ERROR(lsc_check_atomic_src<T>(),
                  "unsupported type for lsc atomic source or dest arguments");
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  using _IntRetTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _SrcTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _SrcTy _TmpSrc0 = Src0.format<_CastTy>();
  _SrcTy _TmpSrc1 = Src1.format<_CastTy>();
  constexpr bool _Transposed = false;
  uint64_t _Addr = (uint64_t)Ptr;
  auto _TmpRes = __cm_intrinsic_impl_lsc_atomic_flat<Op, _DS, VS, _Transposed,
                                                     L1H, L3H, _IntRetTy, N>(
      Pred, _Addr, Offset, _TmpSrc0, _TmpSrc1);
  return lsc_format_ret<T, _IntRetTy, _RetTy>(_TmpRes);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto cm_atomic_slm(vector<unsigned, N> Offset,
                                        vector<ushort, N> Pred = 1) ->
    typename std::enable_if<
        details::lsc_atomic_nsrcs<Op>() == 0,
        vector<T, N * details::lsc_vector_size<VS>()> >::type {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Atomic, L1H, L3H>()),
                  "unsupported cache hint");
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr bool _Transposed = false;
  using _IntRetTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  auto _TmpRes =
      __cm_intrinsic_impl_lsc_atomic_slm<Op, _DS, VS, _Transposed, L1H, L3H,
                                         _IntRetTy, N>(Pred, Offset);
  return lsc_format_ret<T, _IntRetTy, _RetTy>(_TmpRes);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto
cm_atomic_slm(vector<unsigned, N> Offset,
              vector<T, N * details::lsc_vector_size<VS>()> Src0,
              vector<ushort, N> Pred = 1) ->
    typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 1,
                            decltype(Src0)>::type {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Atomic, L1H, L3H>()),
                  "unsupported cache hint");
  CM_STATIC_ERROR(lsc_check_atomic_src<T>(),
                  "unsupported type for lsc atomic source or dest arguments");
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  constexpr bool _Transposed = false;
  using _IntRetTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _SrcTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _SrcTy _TmpSrc0 = Src0.format<_CastTy>();
  auto _TmpRes =
      __cm_intrinsic_impl_lsc_atomic_slm<Op, _DS, VS, _Transposed, L1H, L3H,
                                         _IntRetTy, N>(Pred, Offset, _TmpSrc0);
  return lsc_format_ret<T, _IntRetTy, _RetTy>(_TmpRes);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE auto
cm_atomic_slm(vector<unsigned, N> Offset,
              vector<T, N * details::lsc_vector_size<VS>()> Src0,
              vector<T, N * details::lsc_vector_size<VS>()> Src1,
              vector<ushort, N> Pred = 1) ->
    typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 2,
                            decltype(Src0)>::type {
  CM_HAS_LSC_CONTROL;

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  CM_STATIC_ERROR((lsc_check_cache_hint<LSCAction::Atomic, L1H, L3H>()),
                  "unsupported cache hint");
  CM_STATIC_ERROR(lsc_check_atomic_src<T>(),
                  "unsupported type for lsc atomic source or dest arguments");
  constexpr DataSize _DS = lsc_expand_ds(lsc_data_size<T, DS>());
  using _IntRetTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _SrcTy = decltype(lsc_data_type_ext<T, N, VS>());
  using _RetTy = decltype(lsc_data_type<T, N, VS>());
  using _CastTy = typename lsc_bitcast_type<T>::type;
  _SrcTy _TmpSrc0 = Src0.format<_CastTy>();
  _SrcTy _TmpSrc1 = Src1.format<_CastTy>();
  constexpr bool _Transposed = false;
  auto _TmpRes = __cm_intrinsic_impl_lsc_atomic_slm<Op, _DS, VS, _Transposed,
                                                    L1H, L3H, _IntRetTy, N>(
      Pred, Offset, _TmpSrc0, _TmpSrc1);
  return lsc_format_ret<T, _IntRetTy, _RetTy>(_TmpRes);
}

///
/// LSC Fence built-in
///
/// \brief LSC Fence.
///
/// @param N The number of channels (platform dependent)
///
/// @param Sfid shaded funnction
///
/// @param FenceOp
///
/// @param Scope
///
template <LSC_SFID Sfid = LSC_SFID::LSC_UGM,
          LSC_FENCE_OP FenceOp = LSC_FENCE_OP::LSC_FENCE_OP_NONE,
          LSC_SCOPE Scope = LSC_SCOPE::LSC_SCOPE_GROUP,
          int N = details::lsc_default_simt()>
CM_NODEBUG CM_INLINE void cm_fence(vector<ushort, N> Pred = 1) {
  CM_HAS_LSC_CONTROL;//control platform version

  using namespace details;
  CM_STATIC_ERROR(lsc_check_simt<N>(), "unexpected number of channels");
  __cm_intrinsic_impl_lsc_fence<Sfid, FenceOp, Scope, N>(Pred);
}


#endif // _CLANG_CM_LSC_H_
