/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

SPDX-License-Identifier: MIT
============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_printfocl.h should not be included explicitly - only "
               "<cm/cm.h> is required");
#endif

#ifndef _CM_PRINTFOCL_H_
#define _CM_PRINTFOCL_H_

#define CMPHFOCL_VEC_ISZ  2
#define CMPHFOCL_STR_SZ 4

namespace details {
//
// cmprint implementation for OCL runtime
//

enum CM_Printf_Header_Field_OCL {
  CMPHFOCL_DataTypeIndex = 0,
  CMPHFOCL_DataLoValIndex = 1,
  CMPHFOCL_DataHiValIndex = 2
};

/*****************************************************************************\
ENUM: SHADER_PRINTF_TYPE
Type of printf argument. Changes to this enum must be co-ordinated with Runtime.
\*****************************************************************************/
enum SHADER_PRINTF_TYPE {
  SHADER_PRINTF_INVALID,
  SHADER_PRINTF_BYTE,
  SHADER_PRINTF_SHORT,
  SHADER_PRINTF_INT,
  SHADER_PRINTF_FLOAT,
  SHADER_PRINTF_STRING_LITERAL,
  SHADER_PRINTF_LONG,
  SHADER_PRINTF_POINTER,
  SHADER_PRINTF_DOUBLE,
  SHADER_PRINTF_VECTOR_BYTE,
  SHADER_PRINTF_VECTOR_SHORT,
  SHADER_PRINTF_VECTOR_INT,
  SHADER_PRINTF_VECTOR_LONG,
  SHADER_PRINTF_VECTOR_FLOAT,
  SHADER_PRINTF_VECTOR_DOUBLE,
  NUM_SHADER_PRINTF_TYPES
};

template <typename T>
struct is_printf_string : std::false_type {};

template <unsigned N>
struct is_printf_string <const char(&)[N]> : std::integral_constant <bool, N <= 256> {};

// Propagetes address data form vector vAddr[N] to vRepAddr[N*Rep]
// vRepAddr[j*Rep + i] = vAddr[j] + i * incr
template <int Rep, int N>
CM_NODEBUG CM_INLINE
vector<svmptr_t, N*Rep> _cm_addr_extend(vector<svmptr_t, N> vAddr, int incr) {
  vector<svmptr_t, Rep * N> vRepAddr;
#pragma unroll
  for (int i = 0; i < Rep; ++i)
    vRepAddr.select<N, Rep>(i) = vAddr + i * incr;
  return vRepAddr;
}
// we need specialisation for N = 1 due to restrictions on "select" operation:
// The horizontal / vertical stride must be 1 if the horizontal / vertical size is 1.
template <int Rep>
CM_NODEBUG CM_INLINE
vector<svmptr_t, Rep> _cm_addr_extend(vector<svmptr_t, 1> vAddr, int incr) {
  vector<svmptr_t, Rep> vRepAddr;
#pragma unroll
  for (int i = 0; i < Rep; ++i)
    vRepAddr(i) = vAddr(0) + i * incr;
  return vRepAddr;
}

// cm_svm_scatter_write call with agruments transformed
// to satisfy alignment restrictions
template <typename TA, typename T0, int N>
CM_NODEBUG CM_INLINE
void _cm_svm_scatter_write_aligned(vector<svmptr_t, N> vAddr, vector<T0, N> src) {
  if constexpr (sizeof(T0) > sizeof(TA)) {
    constexpr auto Rep = sizeof(T0) / sizeof(TA);
    cm_svm_scatter_write(_cm_addr_extend<Rep>(vAddr, sizeof(TA)), src.format<TA>());
  }
  else
    cm_svm_scatter_write(vAddr, src);
}

using PBuff = vector<svmptr_t, 1>;
using DataElemT = unsigned;
using OffsetT = DataElemT; // as offset is returned in data vector

template<typename T>
CM_NODEBUG CM_INLINE
constexpr SHADER_PRINTF_TYPE
_cm_print_type_ocl()
{
  if constexpr (is_byte_type<std::remove_reference_t<T>>::value ||
    is_word_type<std::remove_reference_t<T>>::value ||
    is_dword_type<std::remove_reference_t<T>>::value)
    return SHADER_PRINTF_INT;
  else if constexpr (is_printf_string<T>::value)
    return SHADER_PRINTF_STRING_LITERAL;
  else if constexpr (is_fp_type<std::remove_reference_t<T>>::value)
    return SHADER_PRINTF_FLOAT;
  else if constexpr (is_df_type<std::remove_reference_t<T>>::value)
    return SHADER_PRINTF_DOUBLE;
  else if constexpr (is_qword_type<std::remove_reference_t<T>>::value)
    return SHADER_PRINTF_LONG;
  else if constexpr (std::is_pointer<std::remove_reference_t<T>>::value)
    return SHADER_PRINTF_POINTER;
  else
    return SHADER_PRINTF_INVALID;
}

template<typename T>
CM_NODEBUG CM_INLINE
constexpr int _cm_print_size()
{
  CM_STATIC_ERROR(_cm_print_type_ocl<T>() != SHADER_PRINTF_INVALID,
    "data-type not supported by cmprint");
  if constexpr (_cm_print_type_ocl<T>() == SHADER_PRINTF_INT)
    return sizeof(int);
  else if constexpr (_cm_print_type_ocl<T>() == SHADER_PRINTF_POINTER)
    return sizeof(uint64_t);
  else if constexpr (_cm_print_type_ocl<T>() == SHADER_PRINTF_STRING_LITERAL)
    return sizeof(DataElemT);
  else
    return sizeof(T);
}

// tail function
CM_NODEBUG CM_INLINE
OffsetT _cm_pr_len_ocl()
{
  return 0;
}

// recursive size-measure function
template<typename T, typename... Targs>
CM_NODEBUG CM_INLINE
OffsetT _cm_pr_len_ocl(T &&value, Targs&&... Fargs)
{
  auto rem_len = _cm_pr_len_ocl(std::forward<Targs>(Fargs)...);
  return (sizeof(DataElemT) + _cm_print_size<T>() + rem_len);
}

// helper function for ISPC print
// TODO: unify with _cm_print_args_ocl
template<typename T>
CM_NODEBUG CM_INLINE
void cm_write_arg_ocl(svmptr_t offset, DataElemT low, DataElemT high)
{
  vector<DataElemT, CMPHFOCL_VEC_ISZ> data_vec = 0;
  if constexpr(is_byte_type<T>::value) {
    data_vec(CMPHFOCL_DataTypeIndex) = SHADER_PRINTF_BYTE;
    data_vec(CMPHFOCL_DataLoValIndex) = low;
  }
  else if constexpr(is_word_type<T>::value) {
    data_vec(CMPHFOCL_DataTypeIndex) = SHADER_PRINTF_SHORT;
    data_vec(CMPHFOCL_DataLoValIndex) = low;
  }
  else if constexpr(is_dword_type<T>::value) {
    data_vec(CMPHFOCL_DataTypeIndex) = SHADER_PRINTF_INT;
    data_vec(CMPHFOCL_DataLoValIndex) = low;
  }
  else if constexpr (is_printf_string<T>::value) {
    data_vec(CMPHFOCL_DataTypeIndex) = SHADER_PRINTF_STRING_LITERAL;
    data_vec(CMPHFOCL_DataLoValIndex) = low;
  }

  vector<svmptr_t, CMPHFOCL_VEC_ISZ> addr_vec;
  addr_vec(0) = offset;
  addr_vec(1) = offset + 4;
  cm_svm_scatter_write(addr_vec, data_vec);
}

// recursive buffer-write function
template<typename T>
CM_NODEBUG CM_INLINE
void _cm_print_args_ocl(PBuff PBP, OffsetT offset, T &&value)
{
  vector<DataElemT, 1> data_type = _cm_print_type_ocl<T>();
  vector<svmptr_t, 1> Vaddr = PBP(0) + offset;
  cm_svm_scatter_write(Vaddr, data_type);
  Vaddr(0) += sizeof(DataElemT);
  if constexpr (is_printf_string<T>::value) {
    vector<DataElemT, 1> Tmp = static_cast<DataElemT>(cm_print_format_index(value));
    cm_svm_scatter_write(Vaddr, Tmp);
  }
  else if constexpr (_cm_print_type_ocl<T>() == SHADER_PRINTF_INT) {
    vector<int, 1> Tmp = static_cast<int>(value);
    cm_svm_scatter_write(Vaddr, Tmp);
  }
  else if constexpr (_cm_print_type_ocl<T>() == SHADER_PRINTF_POINTER) {
    vector<uint64_t, 1> Tmp = reinterpret_cast<uint64_t>(value);
    _cm_svm_scatter_write_aligned<DataElemT>(Vaddr, Tmp);
  }
  else {
    vector<std::remove_reference_t<T>, 1> Tmp = value;
    _cm_svm_scatter_write_aligned<DataElemT>(Vaddr, Tmp);
  }
}

template<typename T, typename... Targs>
CM_NODEBUG CM_INLINE
void _cm_print_args_ocl(PBuff PBP, OffsetT offset, T &&value, Targs&&... Fargs)
{
  _cm_print_args_ocl(PBP, offset, std::forward<T>(value));
  offset += sizeof(DataElemT) + _cm_print_size<T>();
  _cm_print_args_ocl(PBP, offset, std::forward<Targs>(Fargs)...);
}

const ushort _addr_offset[8] = { 0, 4, 8, 12, 16, 20, 24, 28 };

CM_NODEBUG CM_INLINE
OffsetT _cm_print_init_offset_ocl(PBuff PBP, OffsetT total_len)
{
  vector<DataElemT, 8> out;
  constexpr CmAtomicOpType op1 = ATOMIC_ADD;
  vector<DataElemT, 8> total = 0;
  vector<ushort, 8> offsets(_addr_offset);
  total(0) = total_len;
  vector<svmptr_t, 8> Vaddr = PBP(0) + offsets;

  //write_atomic<ATOMIC_ADD, DataElemT, CMPHF_VEC_ISZ>(PBP, addrs, in, out);
  cm_svm_atomic<DataElemT, 8>(ATOMIC_ADD, Vaddr, out, total);
  return out(0);
}

} //details

#ifdef __CM_OCL_RUNTIME
template<int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N<=256), int>::type
cmfprint(details::PBuff PBP, const char (&format)[N])
{
    details::OffsetT total_len = CMPHFOCL_STR_SZ;

  auto offset = details::_cm_print_init_offset_ocl(PBP, total_len);

  // write-out the index of format string
  vector<uint, 1> findex = cm_print_format_index(format);
  vector<svmptr_t, 1> Vaddr;
  Vaddr(0) = PBP(0) + offset;
  cm_svm_scatter_write(Vaddr, findex);
  offset += CMPHFOCL_STR_SZ;
  return total_len;
}

template<int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N<=256), int>::type
cmprint(const char (&format)[N])
{
  details::PBuff PBP = cm_print_buffer();
  return cmfprint(PBP, format);
}

template<int N, typename T, typename... Targs>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N<=256), int>::type
cmfprint(details::PBuff PBP, const char (&format)[N], T &&value, Targs&&... Fargs)
{
  details::OffsetT total_len = CMPHFOCL_STR_SZ +
      details::_cm_pr_len_ocl(std::forward<T>(value), std::forward<Targs>(Fargs)...);

  auto offset = details::_cm_print_init_offset_ocl(PBP, total_len);

  // write-out the index of format string
  vector<uint, 1> findex = cm_print_format_index(format);
  vector<svmptr_t, 1> Vaddr;
  Vaddr(0) = PBP(0) + offset;
  cm_svm_scatter_write(Vaddr, findex);
  offset += CMPHFOCL_STR_SZ;

  // write-out arguments
  details::_cm_print_args_ocl(PBP, offset, std::forward<T>(value), std::forward<Targs>(Fargs)...);
  return total_len;
}

template<unsigned N, typename T, typename... Targs>
CM_NODEBUG CM_INLINE
typename std::enable_if<(N<=256), int>::type
cmprint(const char (&format)[N], T &&value, Targs&&... Fargs)
{
  details::PBuff PBP = cm_print_buffer();
  return cmfprint(PBP, format, std::forward<T>(value), std::forward<Targs>(Fargs)...);
}

template<typename... Targs>
CM_INLINE auto printf(Targs&&... Fargs)
{
  return cmprint(std::forward<Targs>(Fargs)...);
}

template<typename... Targs>
CM_INLINE auto cm_printf(Targs&&... Fargs)
{
  return cmprint(std::forward<Targs>(Fargs)...);
}
#endif /* __CM_OCL_RUNTIME */
#endif /* _CM_PRINTFOCL_H_ */
