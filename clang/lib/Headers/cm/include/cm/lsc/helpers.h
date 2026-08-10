/*========================== begin_copyright_notice ============================

Copyright (C) 2024-2026 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:lsc/helpers.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_LSC_HELPERS_H_
#define _CLANG_CM_LSC_HELPERS_H_

#include <cm/cm_common.h>

namespace details {
constexpr CacheHint lsc_combine_l2_l3_hints(CacheHint L2H, CacheHint L3H) {
  int l2h = static_cast<int>(L2H);
  int l3h = static_cast<int>(L3H);
  int combined = l2h | (l3h << 4);
  return static_cast<CacheHint>(combined);
}

template <CacheHint Hint> class CacheHintWrap {
private:
  template <CacheHint...> class is_one_of_t;

  template <CacheHint Last>
  struct is_one_of_t<Last>
      : public std::conditional<Last == Hint, std::true_type,
                                std::false_type>::type {};

  template <CacheHint Head, CacheHint... Tail>
  struct is_one_of_t<Head, Tail...>
      : public std::conditional<Head == Hint, std::true_type,
                                is_one_of_t<Tail...>>::type {};

public:
  constexpr operator CacheHint() const { return Hint; }

  template <CacheHint... Hints> static constexpr bool is_one_of() {
    return is_one_of_t<Hints...>::value;
  }
};

template <CacheHint Val>
constexpr bool are_all_equal_to(CacheHint First, CacheHint Second) {
  return First == Val && Second == Val;
}

template <CacheHint Val>
constexpr bool are_all_equal_to(CacheHint First, CacheHint Second,
                                CacheHint Third) {
  return First == Val && Second == Val && Third == Val;
}

template <CacheHint L1, CacheHint L2>
constexpr bool lsc_check_cache_hint_prefetch() {
  constexpr CacheHintWrap<L1> L1H;
  constexpr CacheHintWrap<L2> L2H;
  bool Res = are_all_equal_to<CacheHint::Default>(L1H, L2H) ||
             (L1H.is_one_of<CacheHint::Uncached, CacheHint::Cached,
                            CacheHint::Streaming>() &&
              L2H.is_one_of<CacheHint::Uncached, CacheHint::Cached>() &&
              !are_all_equal_to<CacheHint::Uncached>(L1H, L2H));
#ifdef CM_HAS_LSC_L1L3CC_HINT
  Res = Res || (L1H.is_one_of<CacheHint::Uncached, CacheHint::Cached>() &&
                L2H == CacheHint::ConstCached);
#endif // CM_HAS_LSC_L1L3CC_HINT
  return Res;
}

template <CacheHint L1, CacheHint L2, CacheHint L3>
constexpr bool lsc_check_cache_hint_prefetch() {
  constexpr CacheHintWrap<L1> L1H;
  constexpr CacheHintWrap<L2> L2H;
  constexpr CacheHintWrap<L3> L3H;
  bool Res = lsc_check_cache_hint_prefetch<L1H, L2H>();
#ifdef CM_HAS_LSC_L1L2L3_CACHE
  Res = are_all_equal_to<CacheHint::Default>(L1H, L2H, L3H) ||
        (Res && L2H != CacheHint::Default &&
         L3H.is_one_of<CacheHint::Default, CacheHint::Uncached,
                       CacheHint::Cached>());
  Res = Res || (are_all_equal_to<CacheHint::Uncached>(L1H, L2H) &&
                L3 == CacheHint::Cached);
#else  // CM_HAS_LSC_L1L2L3_CACHE
  Res = Res && L3H == CacheHint::Default;
#endif // CM_HAS_LSC_L1L2L3_CACHE
  return Res;
}

template <CacheHint L1, CacheHint L2>
constexpr bool lsc_check_cache_hint_load() {
  constexpr CacheHintWrap<L1> L1H;
  constexpr CacheHintWrap<L2> L2H;
  bool Res = are_all_equal_to<CacheHint::Default>(L1H, L2H) ||
             (L1H.is_one_of<CacheHint::Uncached, CacheHint::Cached,
                            CacheHint::Streaming>() &&
              L2H.is_one_of<CacheHint::Uncached, CacheHint::Cached>());
#ifdef CM_HAS_LSC_L1L3CC_HINT
  Res = Res || (L1H.is_one_of<CacheHint::Uncached, CacheHint::Cached>() &&
                L2H == CacheHint::ConstCached);
#endif // CM_HAS_LSC_L1L3CC_HINT
#ifdef CM_HAS_LSC_LOAD_L1RI_L3RI_HINT
  Res = Res || are_all_equal_to<CacheHint::ReadInvalidate>(L1H, L2H);
#endif // CM_HAS_LSC_LOAD_L1RI_L3RI_HINT
#ifdef CM_HAS_LSC_LOAD_L1RI_L3CA_HINT
  Res = Res || (L1H == CacheHint::ReadInvalidate && L2H == CacheHint::Cached);
#endif // CM_HAS_LSC_LOAD_L1RI_L3CA_HINT
  return Res;
}

template <CacheHint L1, CacheHint L2, CacheHint L3>
constexpr bool lsc_check_cache_hint_load() {
  constexpr CacheHintWrap<L1> L1H;
  constexpr CacheHintWrap<L2> L2H;
  constexpr CacheHintWrap<L3> L3H;
  bool Res = lsc_check_cache_hint_load<L1H, L2H>();
#ifdef CM_HAS_LSC_L1L2L3_CACHE
  Res = are_all_equal_to<CacheHint::Default>(L1H, L2H, L3H) ||
        (Res && !are_all_equal_to<CacheHint::Default>(L1H, L2H) &&
         L2H != CacheHint::Default &&
         L3H.is_one_of<CacheHint::Default, CacheHint::Uncached,
                       CacheHint::Cached>());
  Res = Res || are_all_equal_to<CacheHint::ReadInvalidate>(L1H, L2H, L3H);
#else  // CM_HAS_LSC_L1L2L3_CACHE
  Res = Res && L3H == CacheHint::Default;
#endif // CM_HAS_LSC_L1L2L3_CACHE
  return Res;
}

template <CacheHint L1, CacheHint L2>
constexpr bool lsc_check_cache_hint_store() {
  constexpr CacheHintWrap<L1> L1H;
  constexpr CacheHintWrap<L2> L2H;
  bool Res = are_all_equal_to<CacheHint::Default>(L1H, L2H) ||
             are_all_equal_to<CacheHint::WriteBack>(L1H, L2H) ||
             (L1H.is_one_of<CacheHint::Uncached, CacheHint::WriteThrough,
                            CacheHint::Streaming>() &&
              L2H.is_one_of<CacheHint::Uncached, CacheHint::WriteBack>());
  return Res;
}

template <CacheHint L1, CacheHint L2, CacheHint L3>
constexpr bool lsc_check_cache_hint_store() {
  constexpr CacheHintWrap<L1> L1H;
  constexpr CacheHintWrap<L2> L2H;
  constexpr CacheHintWrap<L3> L3H;
#ifdef CM_HAS_LSC_L1L2L3_CACHE
  bool Res = are_all_equal_to<CacheHint::Default>(L1H, L2H, L3H) ||
             (L1H.is_one_of<CacheHint::Uncached, CacheHint::WriteBack,
                            CacheHint::WriteThrough, CacheHint::Streaming>() &&
              L2H.is_one_of<CacheHint::Uncached, CacheHint::WriteBack>() &&
              L3H.is_one_of<CacheHint::Default, CacheHint::Uncached,
                            CacheHint::WriteBack>() &&
              !are_all_equal_to<CacheHint::WriteBack>(L2H, L3H));
  return Res;
#else  // CM_HAS_LSC_L1L2L3_CACHE
  return lsc_check_cache_hint_store<L1H, L2H>() && L3H == CacheHint::Default;
#endif // CM_HAS_LSC_L1L2L3_CACHE
}

template <CacheHint L1, CacheHint L2>
constexpr bool lsc_check_cache_hint_atomic() {
  constexpr CacheHintWrap<L1> L1H;
  constexpr CacheHintWrap<L2> L2H;
  return are_all_equal_to<CacheHint::Default>(L1H, L2H) ||
         (L1H == CacheHint::Uncached &&
          L2H.is_one_of<CacheHint::Uncached, CacheHint::WriteBack>());
}

template <CacheHint L1, CacheHint L2, CacheHint L3>
constexpr bool lsc_check_cache_hint_atomic() {
  constexpr CacheHintWrap<L1> L1H;
  constexpr CacheHintWrap<L2> L2H;
  constexpr CacheHintWrap<L3> L3H;
#ifdef CM_HAS_LSC_L1L2L3_CACHE
  return are_all_equal_to<CacheHint::Default>(L1H, L2H, L3H) ||
         (L1H == CacheHint::Uncached &&
          L2H.is_one_of<CacheHint::Uncached, CacheHint::WriteBack>() &&
          L3H.is_one_of<CacheHint::Default, CacheHint::Uncached,
                        CacheHint::WriteBack>() &&
          !are_all_equal_to<CacheHint::WriteBack>(L2H, L3H));
#else  // CM_HAS_LSC_L1L2L3_CACHE
  return lsc_check_cache_hint_atomic<L1H, L2H>() && L3H == CacheHint::Default;
#endif // CM_HAS_LSC_L1L2L3_CACHE
}
} // namespace details

#endif // _CLANG_CM_LSC_HELPERS_H_
