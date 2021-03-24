/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

#ifndef CM_CMTL_HINT_H
#define CM_CMTL_HINT_H

#include "../cm_common.h"

/* fpclass_tag - tag class, it is meant to hint cm template library functions
 * which class of floating point their arguments can be. For many functions
 * this information can help to choose more efficient algorithm (e.g. it will
 * allow to exclude certain checks).
 *
 * There are 5 following floating point classes: nan, infinite, subnormal,
 * zero, normal.
 *
 * The user himself shouldn't create fpclass_tag object by direct
 * call of its constructor. One should use predefined tags from
 * fplclass_tag_def, and get all other tags by combining predefined ones with
 * bitwise or "|", and "&", inversion "~" operations.
 * fpclass_tag_def provide basis of 5 basic floating point classes. Besides it
 * has 2 additional useful tags: special - for nan or infinity special values,
 * general - for all possible floating point values.
 *
 * Example:
 *  auto zero_subn = fpclass_tag_def::zero | fpclass_tag_def::subnormal;
 *  auto zero_norm = ~fpclass_tag_def::special & ~fpclass_tag_def::subnormal;
 *  vector<float, 16> argf = .....;
 *  vector<double, 7> argb = .....;
 *
 *  // user hints foo that all elements of arg is either zero or subnormal
 *  // values, e.g. surrounding code is by design works only with small values.
 *  vector<float, 16> resf = math::foo(argf, zero_subn);
 *  // user hints bar that all elements of arg are not special and not
 *  // subnormal values, which means that they are either normal or zero
 *  // values.
 *  vector<double, 7> resb = math::bar(argb, zero_norm);
 *
 * Tags can be equality compared. Also there is special method "has", that
 * takes another tag as an argument and returns whether the argument is
 * fully included in the tag.
 *
 * ______________________________________________
 * Information for CM template library developers
 *
 * Functions should take tag by value. Though the class is encoded in the type
 * of the tag, and user can work only with types, working with objects of
 * the tag is more convenient.
 *
 * Nevertheless inside the template function analysis of the tag must be
 * compile time, so there are several utils to make it convenient for
 * the library developers too.
 *
 * Each operation with tag object doubled with operation with tag type.
 *
 * Example:
 *  template<typename TagT, typename OtherTagT>
 *  void foo(TagT tag, OtherTagT other_tag) {
 *    static_assert(is_fpclass_tag_v<TagT>, "wrong tag");
 *
 *    auto not_tag = ~tag;
 *    using not_tag_t = TagT::not_t;
 *
 *    auto or_tag = tag | other_tag;
 *    using or_tag_t = or_t<TagT, OtherTagT>
 *
 *    auto and_tag = tag & other_tag;
 *    using and_tag_t = and_t<TagT, OtherTagT>
 *
 *    bool has_tag = tag.has(other_tag);
 *    bool has_tag_cexpr = TagT::template has_v<OtherTagT>;
 *
 *    bool eq_tag = (tag == other_tag);
 *    bool eq_tag_cexpr = equal_v<TagT, OtherTagT>;
 *  }
 *
 * Default tags from fpclass_tag_def are doubled with default types of those
 * tags: nan_t, infinity_t, etc.
 *
 * It is recommended to deduce full tag type and adding static_assert, like in
 * the example above. Do not write something like this:
 *  template<unsigned fpclass>
 *  void bar(fpclass_tag<fpclass> tag) {
 *    // some explicit work with fpclass
 *    .....
 *
 * Functors that work with tag types are intentionally made to take the tag
 * type as a whole, thus fpclass_tag's template parameter meaning can be
 * changed internally in hint's implementation.
 * Don't use this template parameter directly, externally there's no meaning
 * in it.
 */

namespace cmtl {
namespace hint {

namespace detail {
enum : unsigned {
  nan       = 1u,
  infinite  = 1u << 1,
  subnormal = 1u << 2,
  zero      = 1u << 3,
  normal    = 1u << 4,
  general   = nan | infinite | subnormal | zero | normal
};
} // namespace detail

template<unsigned fpclass>
class fpclass_tag : public std::integral_constant<unsigned, fpclass> {
  static_assert(fpclass <= detail::general,
      "invalid floating point class tag");

  template<typename RHS>
  struct has_impl;

  template<unsigned rhs>
  struct has_impl<fpclass_tag<rhs>> {
    static constexpr bool value = ((fpclass & rhs) == rhs);
  };


public:

  CM_INLINE constexpr fpclass_tag() {}

  using not_t = fpclass_tag<~fpclass & detail::general>;

  // is RHS fully included in *this
  template<typename RHS>
  static constexpr bool has_v = has_impl<RHS>::value;

  CM_INLINE constexpr not_t operator~() const {
    return {};
  }

  template<unsigned rhs>
  CM_INLINE constexpr bool has(fpclass_tag<rhs>) const {
    return has_v<fpclass_tag<rhs>>;
  }
};

namespace detail {

template<typename LHS, typename RHS>
struct or_impl;

template<unsigned lhs, unsigned rhs>
struct or_impl<fpclass_tag<lhs>, fpclass_tag<rhs>> {
  using type = fpclass_tag<lhs | rhs>;
};

template<typename LHS, typename RHS>
struct and_impl;

template<unsigned lhs, unsigned rhs>
struct and_impl<fpclass_tag<lhs>, fpclass_tag<rhs>> {
  using type = fpclass_tag<lhs & rhs>;
};

template<typename LHS, typename RHS>
struct equal_impl;

template<unsigned lhs, unsigned rhs>
struct equal_impl<fpclass_tag<lhs>, fpclass_tag<rhs>> {
  static constexpr bool value = (lhs == rhs);
};

} // namespace detail

template<typename LHS, typename RHS>
using or_t = typename detail::or_impl<LHS, RHS>::type;

template<typename LHS, typename RHS>
using and_t = typename detail::and_impl<LHS, RHS>::type;

template<typename LHS, typename RHS>
CM_INLINE constexpr bool equal_v = detail::equal_impl<LHS, RHS>::value;

template<unsigned lhs, unsigned rhs>
CM_INLINE constexpr bool operator==(fpclass_tag<lhs>,
                                    fpclass_tag<rhs>) {
  return equal_v<fpclass_tag<lhs>, fpclass_tag<rhs>>;
}

template<unsigned lhs, unsigned rhs>
CM_INLINE constexpr bool operator!=(fpclass_tag<lhs> lhs_v,
                                    fpclass_tag<rhs> rhs_v) {
  return !(lhs_v == rhs_v);
}

template<unsigned lhs, unsigned rhs>
CM_INLINE constexpr auto operator|(fpclass_tag<lhs>, fpclass_tag<rhs>) {
  return or_t<fpclass_tag<lhs>, fpclass_tag<rhs>>{};
}

template<unsigned lhs, unsigned rhs>
CM_INLINE constexpr auto operator&(fpclass_tag<lhs>, fpclass_tag<rhs>) {
  return and_t<fpclass_tag<lhs>, fpclass_tag<rhs>>{};
}

struct fpclass_tag_def {
  // none must not be provided as a tag, thus there's only none_t type for
  // library developers and no none tag object.
  using none_t      = fpclass_tag<0>;
  using nan_t       = fpclass_tag<detail::nan>;
  using infinite_t  = fpclass_tag<detail::infinite>;
  using subnormal_t = fpclass_tag<detail::subnormal>;
  using zero_t      = fpclass_tag<detail::zero>;
  using normal_t    = fpclass_tag<detail::normal>;
  using general_t   = fpclass_tag<detail::general>;

  static constexpr nan_t        nan{};
  static constexpr infinite_t   infinite{};
  static constexpr subnormal_t  subnormal{};
  static constexpr zero_t       zero{};
  static constexpr normal_t     normal{};
  static constexpr general_t    general{};

  using special_t = or_t<nan_t, infinite_t>;
  static constexpr special_t special{};
};

template<typename T>
struct is_fpclass_tag : std::false_type {};

template<unsigned fpclass_internal>
struct is_fpclass_tag<fpclass_tag<fpclass_internal>> : std::true_type {};

template<typename T>
CM_INLINE constexpr bool is_fpclass_tag_v = is_fpclass_tag<T>::value;

} // namespace hint
} // namespace cmtl
#endif // CM_CMTL_HINT_H
