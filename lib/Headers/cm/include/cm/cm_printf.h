/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_printf.h should not be included explicitly - only "
               "<cm/cm.h> is required");
#endif

#ifndef _CM_PRINTF_H_
#define _CM_PRINTF_H_

#ifndef __CM_OCL_RUNTIME
#define printf(...) __cm_builtin_cm_printf(details::__cm_intrinsic_impl_predefined_surface(2), __VA_ARGS__)
#define cm_printf(...) __cm_builtin_cm_printf(details::__cm_intrinsic_impl_predefined_surface(2), __VA_ARGS__)
#endif

#define fprintf(BTID, ...) __cm_builtin_cm_printf(BTID, __VA_ARGS__)
#define cm_fprintf(BTID, ...) __cm_builtin_cm_printf(BTID, __VA_ARGS__)

#define CMPHF_VEC_ISZ  8
#define CMPHF_VEC_BSZ  (CMPHF_VEC_ISZ * 4) 
#define CMPHF_STR_SZ 128

namespace details {
//
// the following cmprint implementation is intended
// to replace the hard-coded implementation we added in the clang 
// source code
//
enum CM_Printf_Object_Type {
  CMPOT_Unknown = 0,
  CMPOT_Matrix = 1,
  CMPOT_Vector = 2,
  CMPOT_Scalar = 3,
  CMPOT_String = 4,
  CMPOT_Format = 5
};

enum CM_Printf_Data_Type {
  CMPDT_Char = 0,
  CMPDT_Uchar = 1,
  CMPDT_Float = 2,
  CMPDT_Int = 3,
  CMPDT_Uint = 4,
  CMPDT_Short = 5,
  CMPDT_Ushort = 6,
  CMPDT_Qword = 7,
  CMPDT_Uqword = 8,
  CMPDT_Double = 9
};

enum CM_Printf_Header_Field {
  CMPHF_ObjectTypeIndex = 0,
  CMPHF_DataTypeIndex = 1,
  CMPHF_DataLoValIndex = 6,
  CMPHF_DataHiValIndex = 7
};

using DataElemT = unsigned;
using OffsetT = DataElemT; // as offset is returned in data vector

constexpr unsigned PRINT_SURF_IDX = 2;

// base function
template<typename T>
CM_NODEBUG CM_INLINE
OffsetT _cm_pr_len(T value)
{
  OffsetT elem_len = 0;
  if constexpr(details::is_cm_scalar<T>::value)
     elem_len = CMPHF_VEC_BSZ;
  return elem_len;
}

// recursive size-measure function
template<typename T, typename... Targs>
CM_NODEBUG CM_INLINE
OffsetT _cm_pr_len(T value, Targs... Fargs)
{
  OffsetT elem_len = 0;
  if constexpr(details::is_cm_scalar<T>::value)
     elem_len = CMPHF_VEC_BSZ;
  else
     CM_STATIC_ERROR(details::is_cm_scalar<T>::value, "data-type not supported by cmprint");
  auto rem_len = _cm_pr_len(Fargs...);
  return (elem_len + rem_len);
}

// NOTE: is used in ISPC print
template<typename T>
CM_NODEBUG CM_INLINE
void _cm_print_args_raw(SurfaceIndex BTI, OffsetT offset, DataElemT low, DataElemT high = 0)
{
  vector<DataElemT, CMPHF_VEC_ISZ> data_vec = 0;
  data_vec(CMPHF_ObjectTypeIndex) = CMPOT_Scalar;
  if constexpr(details::is_byte_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Char;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Uchar;
    data_vec(CMPHF_DataLoValIndex) = low;
  }
  else if constexpr(details::is_word_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Short;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Ushort;
    data_vec(CMPHF_DataLoValIndex) = low;
  }
  else if constexpr(details::is_dword_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Int;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Uint;
    data_vec(CMPHF_DataLoValIndex) = low;
  }
  else if constexpr(details::is_qword_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Qword;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Uqword;
    data_vec(CMPHF_DataHiValIndex) = high;
    data_vec(CMPHF_DataLoValIndex) = low;
  }
  else if constexpr(details::is_fp_type<T>::value) {
    data_vec(CMPHF_DataTypeIndex) = CMPDT_Float;
    data_vec(CMPHF_DataLoValIndex) = low;
  }
  else if constexpr(details::is_df_type<T>::value) {
    data_vec(CMPHF_DataTypeIndex) = CMPDT_Double;
    data_vec(CMPHF_DataHiValIndex) = high;
    data_vec(CMPHF_DataLoValIndex) = low;
  }
  write(BTI, offset, data_vec);
}

// recursive buffer-write function 
template<typename T>
CM_NODEBUG CM_INLINE
void _cm_print_args(SurfaceIndex BTI, OffsetT offset, T value)
{
  vector<DataElemT, CMPHF_VEC_ISZ> data_vec = 0;
  data_vec(CMPHF_ObjectTypeIndex) = CMPOT_Scalar;
  if constexpr(details::is_byte_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Char;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Uchar;
    data_vec(CMPHF_DataLoValIndex) = (DataElemT)value;
  }
  else if constexpr(details::is_word_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Short;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Ushort;
    data_vec(CMPHF_DataLoValIndex) = (DataElemT)value;
  }
  else if constexpr(details::is_dword_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Int;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Uint;
    data_vec(CMPHF_DataLoValIndex) = (DataElemT)value;
  }
  else if constexpr(details::is_qword_type<T>::value) {
    union {
      long long ll;
      unsigned long long ull;
      DataElemT ui[2];
    } data;
    if constexpr(std::is_signed<T>::value) {
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Qword;
      data.ll = value;
    }
    else {
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Uqword;
      data.ull = value;
    }
        
    data_vec(CMPHF_DataHiValIndex) = data.ui[1]; // FIXME: pure UB
    data_vec(CMPHF_DataLoValIndex) = data.ui[0]; // FIXME: pure UB
  }
  else if constexpr(details::is_fp_type<T>::value) {
    union {
      float fp;
      DataElemT ui;
    } data;
    data.fp = value;
    data_vec(CMPHF_DataTypeIndex) = CMPDT_Float;
    data_vec(CMPHF_DataLoValIndex) = data.ui; // FIXME: pure UB
  }
  else if constexpr(details::is_df_type<T>::value) {
    union {
      double df;
      DataElemT ui[2];
    } data;
    data.df = value;
    data_vec(CMPHF_DataTypeIndex) = CMPDT_Double;
    data_vec(CMPHF_DataHiValIndex) = data.ui[1]; // FIXME: pure UB
    data_vec(CMPHF_DataLoValIndex) = data.ui[0]; // FIXME: pure UB
  }
  write(BTI, offset, data_vec);
}

template<typename T, typename... Targs>
CM_NODEBUG CM_INLINE
void _cm_print_args(SurfaceIndex BTI, OffsetT offset, T value, Targs... Fargs)
{
  _cm_print_args(BTI, offset, value);
  offset += CMPHF_VEC_BSZ;
  _cm_print_args(BTI, offset, Fargs...);
}
 
const ushort _addr_init[8] = {0, 4, 8, 12, 16, 20, 24, 28};

// NOTE: is used in ISPC print
CM_NODEBUG CM_INLINE
OffsetT _cm_print_init_offset(SurfaceIndex BTI, OffsetT total_len)
{
  vector<DataElemT, CMPHF_VEC_ISZ> addrs(details::_addr_init);
  vector<DataElemT, CMPHF_VEC_ISZ> in = 0;
  in(0) = total_len;
  vector<DataElemT, CMPHF_VEC_ISZ> out = 0;
  write_atomic<ATOMIC_ADD, DataElemT, CMPHF_VEC_ISZ>(BTI, addrs, in, out);
  return out(0);
}

// stores format string
// NOTE: is used in ISPC print
template<int N>
CM_NODEBUG CM_INLINE
void _cm_print_format(SurfaceIndex BTI, OffsetT offset, vector<char, N> str_vec)
{
  vector<DataElemT, CMPHF_VEC_ISZ> data_vec = 0;
  data_vec(details::CMPHF_ObjectTypeIndex) = details::CMPOT_Format;
  data_vec(details::CMPHF_DataTypeIndex) = details::CMPDT_Char;
  data_vec(details::CMPHF_DataLoValIndex) = N;
  write(BTI, offset, data_vec);
  offset += CMPHF_VEC_BSZ;
  write(BTI, offset, str_vec);
}

template<int N>
CM_NODEBUG CM_INLINE
void _cm_print_format(SurfaceIndex BTI, OffsetT offset, const char (&format)[N])
{
  vector<char, CMPHF_STR_SZ> str_vec = 0;
#pragma unroll
  for (int i = 0; i < N; ++i) {
    str_vec(i) = format[i];
  }
  _cm_print_format(BTI, offset, str_vec);
}

} //details

#ifndef __CM_OCL_RUNTIME
template<int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N<=128), void>::type
cmfprint(SurfaceIndex BTI, const char (&format)[N])
{
    details::OffsetT total_len = CMPHF_VEC_BSZ + CMPHF_STR_SZ;

  auto offset = details::_cm_print_init_offset(BTI, total_len);

  // write-out the format string
  details::_cm_print_format(BTI, offset, format);
}

#ifdef __CM_USE_OCL_SPEC_PRINTF
using ReturnType = int;
#else  // __CM_USE_OCL_SPEC_PRINTF
using ReturnType = void;
#endif // __CM_USE_OCL_SPEC_PRINTF

template<int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N<=128), ReturnType>::type
cmprint(const char (&format)[N])
{
  SurfaceIndex BTI = details::__cm_intrinsic_impl_predefined_surface(details::PRINT_SURF_IDX);
  cmfprint(BTI, format);
#ifdef __CM_USE_OCL_SPEC_PRINTF
  return 0;
#endif // __CM_USE_OCL_SPEC_PRINTF
}

template<int N, typename T, typename... Targs>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N<=128), void>::type
cmfprint(SurfaceIndex BTI, const char (&format)[N], T value, Targs... Fargs)
{
    details::OffsetT total_len = CMPHF_VEC_BSZ +
                       CMPHF_STR_SZ + details::_cm_pr_len(value, Fargs...);

  auto offset = details::_cm_print_init_offset(BTI, total_len);

  // write-out the format string
  details::_cm_print_format(BTI, offset, format);
  offset += (CMPHF_STR_SZ + CMPHF_VEC_BSZ);

  // write-out arguments
  details::_cm_print_args(BTI, offset, value, Fargs...);
}

template<unsigned N, typename T, typename... Targs>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N<=128), ReturnType>::type
cmprint(const char (&format)[N], T value, Targs... Fargs)
{
  SurfaceIndex BTI = details::__cm_intrinsic_impl_predefined_surface(details::PRINT_SURF_IDX);
  cmfprint(BTI, format, value, Fargs...);
#ifdef __CM_USE_OCL_SPEC_PRINTF
  return 0;
#endif // __CM_USE_OCL_SPEC_PRINTF
}

#endif /* __CM_OCL_RUNTIME */
#endif /* _CM_PRINTF_H_ */
