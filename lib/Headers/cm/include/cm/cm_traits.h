/*========================== begin_copyright_notice ============================

Copyright (C) 2014-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_traits.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_TRAITS_H_
#define _CLANG_CM_TRAITS_H_

namespace std {

template<bool B, typename T, typename F>
struct conditional { using type = T; };

template<typename T, typename F>
struct conditional<false, T, F> { using type = F; };

template <bool, typename T = void> struct enable_if {};
template <typename T> struct enable_if<true, T> {
  typedef T type;
};

template <bool b, typename T = void>
using enable_if_t = typename enable_if<b, T>::type;

template <typename T, T v> struct integral_constant {
  typedef T value_type;
  static const value_type value = v;
  typedef integral_constant<T, v> type;
  operator value_type() { return value; }
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

template <typename T, typename U> struct is_same : public false_type {};
template <typename T> struct is_same<T, T> : public true_type {};

template <typename T> struct remove_const {
  typedef T type;
};
template <typename T> struct remove_const<const T> {
  typedef T type;
};

template <typename T> struct is_integral_impl : false_type {};
template <> struct is_integral_impl<bool> : true_type {};
template <> struct is_integral_impl<char> : true_type {};
template <> struct is_integral_impl<signed char> : true_type {};
template <> struct is_integral_impl<unsigned char> : true_type {};
template <> struct is_integral_impl<short> : true_type {};
template <> struct is_integral_impl<unsigned short> : true_type {};
template <> struct is_integral_impl<int> : true_type {};
template <> struct is_integral_impl<unsigned int> : true_type {};
template <> struct is_integral_impl<long> : true_type {};
template <> struct is_integral_impl<unsigned long> : true_type {};
template <> struct is_integral_impl<long long> : true_type {};
template <> struct is_integral_impl<unsigned long long> : true_type {};

template <typename T>
struct is_integral : is_integral_impl<typename std::remove_const<T>::type> {};

template <typename T>
struct is_signed
    : integral_constant<bool, is_integral<T>::value && (T(-1) < T(0))> {};

template <typename T>
struct is_unsigned
    : integral_constant<bool, is_integral<T>::value && (T(-1) > T(0))> {};

template <typename T> struct is_long_long_impl : false_type {};
template <> struct is_long_long_impl<unsigned long long> : true_type {};
template <> struct is_long_long_impl<long long> : true_type {};

template <typename T> struct is_long_long : is_long_long_impl<T> {};

template <typename T>
struct is_floating_point
    : integral_constant<
          bool,
          std::is_same<half,  typename std::remove_const<T>::type>::value ||
          std::is_same<float,  typename std::remove_const<T>::type>::value ||
          std::is_same<double, typename std::remove_const<T>::type>::value> {};

template<typename T> struct is_pointer_impl : false_type {};
template<typename T> struct is_pointer_impl<T*> : true_type {};

template<typename T>
struct is_pointer : is_pointer_impl<typename std::remove_const<T>::type> {};

// Extends to cm vector/matrix types.
template <typename T, int N> struct is_floating_point<vector<T, N> > {
  static const bool value = is_floating_point<T>::value;
};

template <typename T, int N> struct is_floating_point<vector_ref<T, N> > {
  static const bool value = is_floating_point<T>::value;
};

template <typename T, int N1, int N2>
struct is_floating_point<matrix<T, N1, N2> > {
  static const bool value = is_floating_point<T>::value;
};

template <typename T, int N1, int N2>
struct is_floating_point<matrix_ref<T, N1, N2> > {
  static const bool value = is_floating_point<T>::value;
};

template <typename T, int N> struct is_integral<vector<T, N> > {
  static const bool value = is_integral<T>::value;
};

template <typename T, int N> struct is_integral<vector_ref<T, N> > {
  static const bool value = is_integral<T>::value;
};

template <typename T, int N1, int N2> struct is_integral<matrix<T, N1, N2> > {
  static const bool value = is_integral<T>::value;
};

template <typename T, int N1, int N2>
struct is_integral<matrix_ref<T, N1, N2> > {
  static const bool value = is_integral<T>::value;
};

template< typename T > struct remove_reference { typedef T type; };
template< typename T > struct remove_reference<T&> { typedef T type; };
template< typename T > struct remove_reference<T&&> { typedef T type; };

template<typename T>
using remove_reference_t = typename remove_reference<T>::type;

template <typename T> T&& forward(remove_reference_t<T>& arg) noexcept;
template <typename T> T&& forward(remove_reference_t<T>&& arg) noexcept;

template< typename T > using remove_reference_t = typename remove_reference<T>::type;

template <typename T>
CM_INLINE T&& forward(remove_reference_t<T>& t) noexcept
{
  return static_cast<T&&>(t);
}

template <typename T>
CM_INLINE T&& forward(remove_reference_t<T>&& t) noexcept
{
  return static_cast<T&&>(t);
}

} // namespace std

namespace details {

template <typename T> struct is_cm_vector {
  static const bool value = false;
};

template <typename T, int N> struct is_cm_vector<vector<T, N> > {
  static const bool value = true;
};

template <typename T, int N> struct is_cm_vector<vector_ref<T, N> > {
  static const bool value = true;
};

template <typename T, int N1, int N2> struct is_cm_vector<matrix<T, N1, N2> > {
  static const bool value = true;
};

template <typename T, int N1, int N2>
struct is_cm_vector<matrix_ref<T, N1, N2> > {
  static const bool value = true;
};

template <typename T>
struct is_cm_scalar
    : std::integral_constant<
          bool,
          std::is_same<float, typename std::remove_const<T>::type>::value ||
              std::is_same<double,
                           typename std::remove_const<T>::type>::value ||
              std::is_same<half,
                           typename std::remove_const<T>::type>::value ||
              std::is_same<char, typename std::remove_const<T>::type>::value ||
              std::is_same<signed char,
                           typename std::remove_const<T>::type>::value ||
              std::is_same<unsigned char,
                           typename std::remove_const<T>::type>::value ||
              std::is_same<short, typename std::remove_const<T>::type>::value ||
              std::is_same<unsigned short,
                           typename std::remove_const<T>::type>::value ||
              std::is_same<int, typename std::remove_const<T>::type>::value ||
              std::is_same<unsigned int,
                           typename std::remove_const<T>::type>::value ||
              std::is_same<long, typename std::remove_const<T>::type>::value ||
              std::is_same<unsigned long,
                           typename std::remove_const<T>::type>::value ||
              std::is_same<long long,
                           typename std::remove_const<T>::type>::value ||
              std::is_same<unsigned long long,
                           typename std::remove_const<T>::type>::value> {};

template <typename T>
struct is_dword_type
    : std::integral_constant<
          bool, std::is_same<int, typename std::remove_const<T>::type>::value ||
                    std::is_same<unsigned int, typename std::remove_const<
                                                   T>::type>::value> {};

template <typename T, int N> struct is_dword_type<vector<T, N> > {
  static const bool value = is_dword_type<T>::value;
};

template <typename T, int N> struct is_dword_type<vector_ref<T, N> > {
  static const bool value = is_dword_type<T>::value;
};

template <typename T, int N1, int N2> struct is_dword_type<matrix<T, N1, N2> > {
  static const bool value = is_dword_type<T>::value;
};

template <typename T, int N1, int N2>
struct is_dword_type<matrix_ref<T, N1, N2> > {
  static const bool value = is_dword_type<T>::value;
};

template <typename T>
struct is_word_type
    : std::integral_constant<
          bool,
          std::is_same<short, typename std::remove_const<T>::type>::value ||
              std::is_same<unsigned short,
                           typename std::remove_const<T>::type>::value> {};

template <typename T, int N> struct is_word_type<vector<T, N> > {
  static const bool value = is_word_type<T>::value;
};

template <typename T, int N> struct is_word_type<vector_ref<T, N> > {
  static const bool value = is_word_type<T>::value;
};

template <typename T, int N1, int N2> struct is_word_type<matrix<T, N1, N2> > {
  static const bool value = is_word_type<T>::value;
};

template <typename T, int N1, int N2>
struct is_word_type<matrix_ref<T, N1, N2> > {
  static const bool value = is_word_type<T>::value;
};

template <typename T>
struct is_byte_type
    : std::integral_constant<
          bool,
          std::is_same<char, typename std::remove_const<T>::type>::value ||
              std::is_same<unsigned char,
                           typename std::remove_const<T>::type>::value> {};

template <typename T, int N> struct is_byte_type<vector<T, N> > {
  static const bool value = is_byte_type<T>::value;
};

template <typename T, int N> struct is_byte_type<vector_ref<T, N> > {
  static const bool value = is_byte_type<T>::value;
};

template <typename T, int N1, int N2> struct is_byte_type<matrix<T, N1, N2> > {
  static const bool value = is_byte_type<T>::value;
};

template <typename T, int N1, int N2>
struct is_byte_type<matrix_ref<T, N1, N2> > {
  static const bool value = is_byte_type<T>::value;
};

template <typename T>
struct is_fp_type
    : std::integral_constant<
          bool,
          std::is_same<float, typename std::remove_const<T>::type>::value> {};

template <typename T>
struct is_df_type
    : std::integral_constant<
          bool,
          std::is_same<double, typename std::remove_const<T>::type>::value> {};

template <typename T>
struct is_fp_or_dword_type
    : std::integral_constant<
          bool,
          std::is_same<float, typename std::remove_const<T>::type>::value ||
              std::is_same<int, typename std::remove_const<T>::type>::value ||
              std::is_same<unsigned int,
                           typename std::remove_const<T>::type>::value> {};

template <typename T>
struct is_qword_type
    : std::integral_constant<
          bool, std::is_same<long long, typename std::remove_const<T>::type>::value ||
          std::is_same<unsigned long long, typename std::remove_const<T>::type>::value> {};

template <typename T, int N> struct is_qword_type<vector<T, N> > {
  static const bool value = is_qword_type<T>::value;
};

template <typename T, int N> struct is_qword_type<vector_ref<T, N> > {
  static const bool value = is_qword_type<T>::value;
};

template <typename T, int N1, int N2> struct is_qword_type<matrix<T, N1, N2> > {
  static const bool value = is_qword_type<T>::value;
};

template <typename T, int N1, int N2>
struct is_qword_type<matrix_ref<T, N1, N2> > {
  static const bool value = is_qword_type<T>::value;
};

// Extends to cm vector/matrix types.
template <typename T, int N> struct is_fp_or_dword_type<vector<T, N> > {
  static const bool value = is_fp_or_dword_type<T>::value;
};

template <typename T, int N> struct is_fp_or_dword_type<vector_ref<T, N> > {
  static const bool value = is_fp_or_dword_type<T>::value;
};

template <typename T, int N1, int N2>
struct is_fp_or_dword_type<matrix<T, N1, N2> > {
  static const bool value = is_fp_or_dword_type<T>::value;
};

template <typename T, int N1, int N2>
struct is_fp_or_dword_type<matrix_ref<T, N1, N2> > {
  static const bool value = is_fp_or_dword_type<T>::value;
};


template <typename T>
struct is_half_type
    : std::integral_constant<bool,
                            std::is_same<half,
                                typename std::remove_const<T>::type>::value> {
};
template <typename T>
struct is_char_type
    : std::integral_constant<
          bool,
          std::is_same<char, typename std::remove_const<T>::type>::value> {};
template <typename T>
struct is_float_type
    : std::integral_constant<
          bool,
          std::is_same<float, typename std::remove_const<T>::type>::value> {};
template <typename T>
struct is_short_type
    : std::integral_constant<
          bool,
          std::is_same<short, typename std::remove_const<T>::type>::value> {};

template <typename T>
struct is_float_or_half
    : std::integral_constant<
          bool,
          std::is_same<half, typename std::remove_const<T>::type>::value ||
              std::is_same<float, typename std::remove_const<T>::type>::value> {
};

// compile-time checks if first template parameter is equal for any other
template <typename...> struct is_one_of {
  static constexpr bool value = false;
};

template <typename Checked, typename First, typename... Other>
struct is_one_of<Checked, First, Other...> {
  static constexpr bool value =
      std::is_same<typename std::remove_const<Checked>::type,
                   typename std::remove_const<First>::type>::value ||
      is_one_of<Checked, Other...>::value;
};
template <typename Checked, typename... T>
inline constexpr bool is_one_of_v = is_one_of<Checked, T...>::value;

// compile-time checks if compile-time known  element of enum class is equal for
// any other compile-time known elements of enum
template <typename enumClass, enumClass... E> struct is_one_of_enum {
  static constexpr bool value = false;
};

template <typename enumClass, enumClass Checked, enumClass First,
          enumClass... Else>
struct is_one_of_enum<enumClass, Checked, First, Else...> {
  static constexpr bool value =
      (Checked == First) || is_one_of_enum<enumClass, Checked, Else...>::value;
};
template <typename enumClass, enumClass... T>
inline constexpr bool is_one_of_enum_v = is_one_of_enum<enumClass, T...>::value;

// Extends to cm vector/matrix types.
template <typename T, int N> struct is_float_or_half<vector<T, N> > {
  static const bool value = is_float_or_half<T>::value;
};

template <typename T, int N> struct is_float_or_half<vector_ref<T, N> > {
  static const bool value = is_float_or_half<T>::value;
};

template <typename T, int N1, int N2>
struct is_float_or_half<matrix<T, N1, N2> > {
  static const bool value = is_float_or_half<T>::value;
};

template <typename T, int N1, int N2>
struct is_float_or_half<matrix_ref<T, N1, N2> > {
  static const bool value = is_float_or_half<T>::value;
};

template <typename T> struct dword_type {
  typedef T type;
};
template <> struct dword_type<char> {
  typedef int type;
};
template <> struct dword_type<short> {
  typedef int type;
};
template <> struct dword_type<uchar> {
  typedef uint type;
};
template <> struct dword_type<ushort> {
  typedef uint type;
};

template <typename T> struct byte_type {
  typedef T type;
};
template <> struct byte_type<short> {
  typedef char type;
};
template <> struct byte_type<int> {
  typedef char type;
};
template <> struct byte_type<ushort> {
  typedef uchar type;
};
template <> struct byte_type<uint> {
  typedef uchar type;
};

template <typename T> struct word_type {
  typedef T type;
};
template <> struct word_type<char> {
  typedef short type;
};
template <> struct word_type<int> {
  typedef short type;
};
template <> struct word_type<uchar> {
  typedef ushort type;
};
template <> struct word_type<uint> {
  typedef ushort type;
};

/// make_signed / make_unsigned - similar to the STL class template of the same name
template <typename T> struct make_signed {
  typedef T type;
};
template <> struct make_signed<unsigned long long> {
  typedef long long type;
};
template <> struct make_signed<unsigned int> {
  typedef int type;
};
template <> struct make_signed<unsigned short> {
  typedef short type;
};
template <> struct make_signed<unsigned char> {
  typedef char type;
};

template <typename T> struct make_unsigned {
  typedef T type;
};
template <> struct make_unsigned<long long> {
  typedef unsigned long long type;
};
template <> struct make_unsigned<int> {
  typedef unsigned int type;
};
template <> struct make_unsigned<short> {
  typedef unsigned short type;
};
template <> struct make_unsigned<char> {
  typedef unsigned char type;
};

/// Transorms input type to a non-_ref type (matrix_ref->matrix, vector_ref->vector)
template <typename T> struct remove_ref { using type = T; };
template <typename T, int N> struct remove_ref<vector_ref<T, N> > {
  using type = vector<T, N>;
};
template <typename T, int N, int M> struct remove_ref<matrix_ref<T, N, M> > {
  using type = matrix<T, N, M>;
};
template <typename T> using remove_ref_t = typename remove_ref<T>::type;

/// Convert types into vector types
template <typename T> struct vector_type {
  typedef vector<T, 1> type;
};
template <typename T, int N> struct vector_type<vector<T, N> > {
  typedef vector<T, N> type;
};
template <typename T, int N> struct vector_type<vector_ref<T, N> > {
  typedef vector<T, N> type;
};
template <typename T, int N, int M> struct vector_type<matrix<T, N, M> > {
  typedef vector<T, N *M> type;
};
template <typename T, int N, int M> struct vector_type<matrix_ref<T, N, M> > {
  typedef vector<T, N *M> type;
};
template <typename T> struct vector_type<T &> {
  typedef typename vector_type<T>::type type;
};
template <typename T> struct vector_type<T &&> {
  typedef typename vector_type<T>::type type;
};
template <typename T> struct vector_type<const T> {
  typedef typename vector_type<T>::type type;
};

/// Get the computation type.
template <typename T1, typename T2> struct computation_type {
  // Currently only arithmetic operations are needed.
  typedef decltype(T1() + T2()) type;
};

// Get the common type.
template <typename T1, typename T2> struct common_type {
  typedef decltype(true ? T1() : T2()) type;
};

// Base case for checking if a type U is one of the types.
template <typename U> constexpr bool is_type() { return false; }

template <typename U, typename T, typename... Ts> constexpr bool is_type() {
  using _U = typename std::remove_const<U>::type;
  using _T = typename std::remove_const<T>::type;
  return std::is_same<_U, _T>::value || is_type<_U, Ts...>();
}

} // details

#endif // _CLANG_CM_TRAITS_H_
