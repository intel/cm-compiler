/*
 * Copyright (c) 2019, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_printf.h should not be included explicitly - only "
               "<cm/cm.h> is required");
#endif

#ifndef _CM_PRINTF_H_
#define _CM_PRINTF_H_

#define printf(...) __cm_builtin_cm_printf(details::__cm_intrinsic_impl_predefined_surface(2), __VA_ARGS__)
#define cm_printf(...) __cm_builtin_cm_printf(details::__cm_intrinsic_impl_predefined_surface(2), __VA_ARGS__)

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

// base function
template<typename T>
CM_NODEBUG CM_INLINE
unsigned _cm_pr_len(T value)
{
  unsigned elem_len = 0;
  if constexpr(details::is_cm_scalar<T>::value)
     elem_len = CMPHF_VEC_BSZ;
  return elem_len;
}

// recursive size-measure function
template<typename T, typename... Targs>
CM_NODEBUG CM_INLINE
unsigned _cm_pr_len(T value, Targs... Fargs)
{
  unsigned elem_len = 0;
  if constexpr(details::is_cm_scalar<T>::value)
     elem_len = CMPHF_VEC_BSZ;
  else
     CM_STATIC_ERROR(details::is_cm_scalar<T>::value, "data-type not supported by cmprint");
  unsigned rem_len = _cm_pr_len(Fargs...);
  return (elem_len + rem_len);
}

// recursive buffer-write function 
template<typename T>
CM_NODEBUG CM_INLINE
void _cm_print(SurfaceIndex BTI, unsigned offset, T value)
{
  vector<unsigned, CMPHF_VEC_ISZ> data_vec = 0;
  data_vec(CMPHF_ObjectTypeIndex) = CMPOT_Scalar;
  if constexpr(details::is_byte_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Char;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Uchar;
    data_vec(CMPHF_DataLoValIndex) = (unsigned)value;
  }
  else if constexpr(details::is_word_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Short;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Ushort;
    data_vec(CMPHF_DataLoValIndex) = (unsigned)value;
  }
  else if constexpr(details::is_dword_type<T>::value) {
    if constexpr(std::is_signed<T>::value)
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Int;
    else
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Uint;
    data_vec(CMPHF_DataLoValIndex) = (unsigned)value;
  }
  else if constexpr(details::is_qword_type<T>::value) {
    union {
      long long ll;
      unsigned long long ull;
      unsigned ui[2];
    } data;
    if constexpr(std::is_signed<T>::value) {
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Qword;
      data.ll = value;
    }
    else {
      data_vec(CMPHF_DataTypeIndex) = CMPDT_Uqword;
      data.ull = value;
    }
        
    data_vec(CMPHF_DataHiValIndex) = data.ui[1];
    data_vec(CMPHF_DataLoValIndex) = data.ui[0];
  }
  else if constexpr(details::is_fp_type<T>::value) {
    union {
      float fp;
      unsigned ui;
    } data;
    data.fp = value;
    data_vec(CMPHF_DataTypeIndex) = CMPDT_Float;
    data_vec(CMPHF_DataLoValIndex) = data.ui;
  }
  else if constexpr(details::is_df_type<T>::value) {
    union {
      double df;
      unsigned ui[2];
    } data;
    data.df = value;
    data_vec(CMPHF_DataTypeIndex) = CMPDT_Double;
    data_vec(CMPHF_DataHiValIndex) = data.ui[1];
    data_vec(CMPHF_DataLoValIndex) = data.ui[0];
  }
  write(BTI, offset, data_vec);
}

template<typename T, typename... Targs>
CM_NODEBUG CM_INLINE
void _cm_print(SurfaceIndex BTI, unsigned offset, T value, Targs... Fargs)
{
  _cm_print(BTI, offset, value);
  offset += CMPHF_VEC_BSZ;
  _cm_print(BTI, offset, Fargs...);
}
 
const ushort _addr_init[8] = {0, 4, 8, 12, 16, 20, 24, 28};

}

// variadic function
template<int N, typename T, typename... Targs>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N<=128), void>::type
cmfprint(SurfaceIndex BTI, const char (&format)[N], T value, Targs... Fargs)
{
  unsigned total_len = CMPHF_VEC_BSZ +
                       CMPHF_STR_SZ + details::_cm_pr_len(value, Fargs...);

  // atomic-add
  vector<unsigned, CMPHF_VEC_ISZ> addrs(details::_addr_init);
  vector<unsigned, CMPHF_VEC_ISZ> in = 0;
  in(0) = total_len;
  vector<unsigned, CMPHF_VEC_ISZ> out = 0;
  write_atomic<ATOMIC_ADD, unsigned, CMPHF_VEC_ISZ>(BTI, addrs, in, out);
  unsigned offset = out(0);

  // write-out the format string
  vector<unsigned, CMPHF_VEC_ISZ> data_vec = 0;
  data_vec(details::CMPHF_ObjectTypeIndex) = details::CMPOT_Format;
  data_vec(details::CMPHF_DataTypeIndex) = details::CMPDT_Char;
  data_vec(details::CMPHF_DataLoValIndex) = N;
  write(BTI, offset, data_vec);
  offset += CMPHF_VEC_BSZ;
  vector<char, CMPHF_STR_SZ> str_vec = 0;
#pragma unroll
  for (int i = 0; i < N; ++i) {
    str_vec(i) = format[i];
  }
  write(BTI, offset, str_vec);
  offset += CMPHF_STR_SZ;

  // write-out arguments
  details::_cm_print(BTI, offset, value, Fargs...);
}

// variadic function
template<unsigned N, typename T, typename... Targs>
CM_NODEBUG CM_INLINE
//typename std::enable_if<(N<=128), void>::type
void
cmprint(const char (&format)[N], T value, Targs... Fargs)
{
  unsigned total_len = CMPHF_VEC_BSZ +
                       CMPHF_STR_SZ + details::_cm_pr_len(value, Fargs...);

  SurfaceIndex BTI = details::__cm_intrinsic_impl_predefined_surface(2);
  // atomic-add
  vector<unsigned, CMPHF_VEC_ISZ> addrs(details::_addr_init);
  vector<unsigned, CMPHF_VEC_ISZ> in = 0;
  in(0) = total_len;
  vector<unsigned, CMPHF_VEC_ISZ> out = 0;
  write_atomic<ATOMIC_ADD, unsigned, CMPHF_VEC_ISZ>(BTI, addrs, in, out);
  unsigned offset = out(0);

  // write-out the format string
  vector<unsigned, CMPHF_VEC_ISZ> data_vec = 0;
  data_vec(details::CMPHF_ObjectTypeIndex) = details::CMPOT_Format;
  data_vec(details::CMPHF_DataTypeIndex) = details::CMPDT_Char;
  data_vec(details::CMPHF_DataLoValIndex) = N;
  write(BTI, offset, data_vec);
  offset += CMPHF_VEC_BSZ;
  vector<char, CMPHF_STR_SZ> str_vec = 0;
#pragma unroll
  for (int i = 0; i < N; ++i) {
    str_vec(i) = format[i];
  }
  write(BTI, offset, str_vec);
  offset += CMPHF_STR_SZ;

  // write-out arguments
  details::_cm_print(BTI, offset, value, Fargs...);
}

#endif /* _CM_PRINTF_H_ */
