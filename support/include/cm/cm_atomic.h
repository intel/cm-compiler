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
static_assert(0, "CM:w:cm_atomic.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_ATOMIC_
#define _CLANG_CM_ATOMIC_

#include "cm_common.h"
#include "cm_internal.h"
#include "cm_traits.h"
#include "cm_util.h"

namespace details {

/// Checking Atomic ops with traits. By default, no error.
template <typename T, CmAtomicOpType Op> struct is_valid_atomic_op;

#define _ATOMIC_CHECK(Name, Type)                                              \
  template <typename T> struct is_valid_atomic_op<T, Name> {                   \
    static void check() {                                                      \
      CM_STATIC_ERROR(                                                         \
          (std::is_same<Type, typename std::remove_const<T>::type>::value),    \
          "data type must be " #Type " for " #Name);                           \
    }                                                                          \
  }

#define _ATOMIC_CHECK_TYPES(Name, Type1, Type2)                                \
  template <typename T> struct is_valid_atomic_op<T, Name> {                   \
    static void check() {                                                      \
      CM_STATIC_ERROR(                                                         \
          (std::is_same<Type1, typename std::remove_const<T>::type>::value ||  \
           std::is_same<Type2, typename std::remove_const<T>::type>::value),   \
          "data type must be " #Type1 "or " #Type2 " for " #Name);             \
    }                                                                          \
  }


_ATOMIC_CHECK_TYPES(ATOMIC_ADD, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_SUB, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_INC, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_DEC, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_MIN, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_MAX, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_XCHG, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_CMPXCHG, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_AND, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_OR, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_XOR, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_MINSINT, uint, int);
_ATOMIC_CHECK_TYPES(ATOMIC_MAXSINT, uint, int);
_ATOMIC_CHECK(ATOMIC_FMAX, float);
_ATOMIC_CHECK(ATOMIC_FMIN, float);
_ATOMIC_CHECK(ATOMIC_FCMPWR, float);
_ATOMIC_CHECK_TYPES(ATOMIC_PREDEC, uint, int);

#undef _ATOMIC_CHECK

/// Do split initialization conditionally:
///
/// if (N2 >= 2 * N1) {
///   v0 = v2.select<N1, 1>(0);
///   v1 = v2.select<N1, 1>(N1);
/// } else if (N2 > N1) {
///   v0 = v2.select<N1, 1>(0);
/// } else if (N2 == N1) {
///   v0 = v2;
/// } else {
///   // do nothing.
/// }
///
template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<N2 == N1, void>::type
if_init_level0(vector_ref<T, N1> v0, vector<T, N2> v2) {
  v0 = v2;
}
template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<N2 != N1, void>::type
if_init_level0(vector_ref<T, N1> v0, vector<T, N2> v2) {}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<(N2 > N1), void>::type
if_init_level1(vector_ref<T, N1> v0, vector<T, N2> v2) {
  v0 = v2.select<N1, 1>(0);
}
template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<!(N2 > N1), void>::type
if_init_level1(vector_ref<T, N1> v0, vector<T, N2> v2) {
  if_init_level0(v0, v2);
}

template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<(N2 >= 2 * N1), void>::type
if_split_init(vector_ref<T, N1> v0, vector_ref<T, N1> v1, vector<T, N2> v2) {
  v0 = v2.select<N1, 1>();
  v1 = v2.select<N1, 1>(N1);
}
template <typename T, int N1, int N2>
CM_NODEBUG CM_INLINE typename std::enable_if<!(N2 >= 2 * N1), void>::type
if_split_init(vector_ref<T, N1> v0, vector_ref<T, N1> v1, vector<T, N2> v2) {
  if_init_level1(v0, v2);
}

} // namespace details

/// \brief DWord atomic write.
///
/// \param index surface index, which must correspond to a buffer.
///
/// \param op the atomic operation kind. It must be a compile time constant.
///
/// \param globalOffset zero based global offset of a set of 8 scattered DWords
/// to be written to the surface. This offset is in units of DWords.
///
/// \param elementOffset zero based offset of each DWord (relative to the
/// global offset) to be written. This offset is in units of DWords.
///
/// \param src the source operands for the specified atomic operation.
///   # For ATOMIC_CMPXCHG, N must be 16.
///   # For ATOMIC_INC / ATOMIC_DEC, src is ignored.
///   # For all other cases, N must be 8.
///
/// \param v the data location to store the returned result, which correspond
/// to the old surface data value.
///
template <typename T, int N>
typename std::enable_if<details::is_fp_or_dword_type<T>::value, void>::type
write(SurfaceIndex index, CmAtomicOpType op, uint globalOffset,
      vector<uint, 8> elementOffset, vector<T, N> src, vector_ref<T, 8> ret)
    CM_DEPRECATED("use 'write_atomic' instead!");

template <typename T, int N>
typename std::enable_if<details::is_fp_or_dword_type<T>::value, void>::type
write(SurfaceIndex index, CmAtomicOpType op, uint globalOffset,
      vector<uint, 8> elementOffset, vector<T, N> src, const int null)
    CM_DEPRECATED("use 'write_atomic' instead!");

template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::is_fp_or_dword_type<T>::value, void>::type
write(SurfaceIndex index, CmAtomicOpType op, uint globalOffset,
      vector<uint, 8> elementOffset, vector<T, N> src, const int null) {
  // Special case where we want to ignore the return value - this is indicated
  // by the return value being NULL (note we actually treat any scalar integer
  // value as if it were NULL), and achieved by using a temp ret vector (which
  // will be optimized away).
  vector<T, 8> ret;
  write(index, op, globalOffset, elementOffset, src, ret);
}

template <typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<details::is_fp_or_dword_type<T>::value, void>::type
write(SurfaceIndex index, CmAtomicOpType op, uint globalOffset,
      vector<uint, 8> elementOffset, vector<T, N> src, vector_ref<T, 8> ret) {
  vector<uint, 8> _Offset = globalOffset + elementOffset;
  vector<T, 8> _Src0, _Src1;
  vector<ushort, 8> mask = 1;

#define _ATOMIC_WRITE(Op, Sz, Type)                                            \
  details::if_split_init(_Src0, _Src1, src);                                   \
  ret = details::__cm_intrinsic_impl_atomic_write<Op, Sz, Type>(               \
      mask, index, _Offset, _Src0, _Src1, ret)

  switch (op) {
  default:
    break;
  case ATOMIC_ADD:
    _ATOMIC_WRITE(ATOMIC_ADD, 8, uint);
    break;
  case ATOMIC_SUB:
    _ATOMIC_WRITE(ATOMIC_SUB, 8, uint);
    break;
  case ATOMIC_INC:
    ret = details::__cm_intrinsic_impl_atomic_write<ATOMIC_INC, 8, uint>(
        mask, index, _Offset, _Src0, _Src1, ret);
    break;
  case ATOMIC_DEC:
    ret = details::__cm_intrinsic_impl_atomic_write<ATOMIC_DEC, 8, uint>(
        mask, index, _Offset, _Src0, _Src1, ret);
    break;
  case ATOMIC_PREDEC:
    ret = details::__cm_intrinsic_impl_atomic_write<ATOMIC_DEC, 8, uint>(
      mask, index, _Offset, _Src0, _Src1, ret) - 1;
    break;
  case ATOMIC_MIN:
    _ATOMIC_WRITE(ATOMIC_MIN, 8, uint);
    break;
  case ATOMIC_MAX:
    _ATOMIC_WRITE(ATOMIC_MAX, 8, uint);
    break;
  case ATOMIC_XCHG:
    _ATOMIC_WRITE(ATOMIC_XCHG, 8, uint);
    break;
  case ATOMIC_CMPXCHG:
    _ATOMIC_WRITE(ATOMIC_CMPXCHG, 8, uint);
    break;
  case ATOMIC_AND:
    _ATOMIC_WRITE(ATOMIC_AND, 8, uint);
    break;
  case ATOMIC_OR:
    _ATOMIC_WRITE(ATOMIC_OR, 8, uint);
    break;
  case ATOMIC_XOR:
    _ATOMIC_WRITE(ATOMIC_XOR, 8, uint);
    break;
  case ATOMIC_MINSINT:
    _ATOMIC_WRITE(ATOMIC_MINSINT, 8, int);
    break;
  case ATOMIC_MAXSINT:
    _ATOMIC_WRITE(ATOMIC_MAXSINT, 8, int);
    break;
  case ATOMIC_FMAX:
    _ATOMIC_WRITE(ATOMIC_FMAX, 8, float);
    break;
  case ATOMIC_FMIN:
    _ATOMIC_WRITE(ATOMIC_FMIN, 8, float);
    break;
  case ATOMIC_FCMPWR:
    _ATOMIC_WRITE(ATOMIC_FCMPWR, 8, float);
    break;
  }

#undef _ATOMIC_WRITE
}

/// \brief DWord atomic write.
///
/// \param op the atomic operation kind.
///
/// \param index surface index, which must correspond to a buffer.
///
/// \param elementOffset zero based offset of each DWord to be written. This
/// offset is in units of DWords.
///
/// \param src0 the first source operand for the specified atomic operation.
///
/// \param src1 the second source operand for the specified atomic operation.
///
/// \param ret the data location to store the returned result, which corresponds
/// to the old surface data value.
///

// has return value, one source
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op!= ATOMIC_FCMPWR &&
     Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
                              details::isPowerOf2(N, 16), void>::type
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
             vector<T, N> src0, vector_ref<T, N> ret) {
  // Perform remaining element type checking
  details::is_valid_atomic_op<T, Op>::check();
  vector<ushort, N> mask = 1;
  vector<T, N> dummy;
  vector_ref<uint, N> _Ret = ret.format<uint>();
  _Ret = details::__cm_intrinsic_impl_atomic_write<Op, N, T>(
      mask, index, elementOffset, src0, dummy, ret);
}

// mask, has return value, one source
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op!= ATOMIC_FCMPWR &&
     Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
                              details::isPowerOf2(N, 16), void>::type
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset, vector<T, N> src0,
             vector_ref<T, N> ret) {
  // Perform remaining element type checking
  details::is_valid_atomic_op<T, Op>::check();
  vector<T, N> dummy;
  vector_ref<uint, N> _Ret = ret.format<uint>();
  _Ret = details::__cm_intrinsic_impl_atomic_write<Op, N, T>(
      mask, index, elementOffset, src0, dummy, ret);
}

// no return value, one source
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op!= ATOMIC_FCMPWR &&
     Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
                              details::isPowerOf2(N, 16), void>::type
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
             vector<T, N> src0) {
  vector<T, N> dummy;
  details::is_valid_atomic_op<T, Op>::check();
  vector<ushort, N> mask = 1;
  details::__cm_intrinsic_impl_atomic_write<Op, N, T>(
      mask, index, elementOffset, src0, dummy, dummy);
}

// mask, no return value, one source
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op!= ATOMIC_FCMPWR &&
     Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
                              details::isPowerOf2(N, 16), void>::type
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset, vector<T, N> src0) {
  vector<T, N> dummy;
  details::is_valid_atomic_op<T, Op>::check();
  details::__cm_intrinsic_impl_atomic_write<Op, N, T>(
      mask, index, elementOffset, src0, dummy, dummy);
}

// INC/DEC: return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            (details::is_dword_type<T>::value) &&
                            details::isPowerOf2(N, 16), void>::type
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
             vector_ref<T, N> ret) {
  vector<uint, N> dummy;
  vector_ref<uint, N> _Ret = ret.format<uint>();
  vector<ushort, N> mask = 1;
  _Ret = details::__cm_intrinsic_impl_atomic_write<Op, N, uint>(
      mask, index, elementOffset, dummy, dummy, _Ret);
}

// PREDEC: return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
(details::is_dword_type<T>::value) &&
details::isPowerOf2(N, 16), void>::type
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
  vector_ref<T, N> ret) {
  vector<uint, N> dummy;
  vector_ref<uint, N> _Ret = ret.format<uint>();
  vector<ushort, N> mask = 1;
  _Ret = details::__cm_intrinsic_impl_atomic_write<ATOMIC_DEC, N, uint>(
    mask, index, elementOffset, dummy, dummy, _Ret) - 1;
}

// INC/DEC: mask and return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            (details::is_dword_type<T>::value) &&
                            details::isPowerOf2(N, 16), void>::type
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset, vector_ref<T, N> ret) {
  vector<uint, N> dummy;
  vector_ref<uint, N> _Ret = ret.format<uint>();
  _Ret = details::__cm_intrinsic_impl_atomic_write<Op, N, uint>(
      mask, index, elementOffset, dummy, dummy, _Ret);
}

// PREDEC: mask and return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
(details::is_dword_type<T>::value) &&
details::isPowerOf2(N, 16), void>::type
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
  vector<uint, N> elementOffset, vector_ref<T, N> ret) {
  vector<uint, N> dummy;
  vector_ref<uint, N> _Ret = ret.format<uint>();
  _Ret = details::__cm_intrinsic_impl_atomic_write<ATOMIC_DEC, N, uint>(
    mask, index, elementOffset, dummy, dummy, _Ret) - 1;
}

// INC/DEC: no return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            (details::is_dword_type<T>::value) &&
                            details::isPowerOf2(N, 16), void>::type
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset) {
  vector<T, N> dummy;
  details::is_valid_atomic_op<T, Op>::check();
  vector<ushort, N> mask = 1;
  details::__cm_intrinsic_impl_atomic_write<Op, N, T>(
      mask, index, elementOffset, dummy, dummy, dummy);
}

// PREDEC: no return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
(details::is_dword_type<T>::value) &&
details::isPowerOf2(N, 16), void>::type
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset) {
  vector<T, N> dummy;
  details::is_valid_atomic_op<T, Op>::check();
  vector<ushort, N> mask = 1;
  details::__cm_intrinsic_impl_atomic_write<ATOMIC_DEC, N, T>(
    mask, index, elementOffset, dummy, dummy, dummy);
}

// INC/DEC: mask and no return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            (details::is_dword_type<T>::value) &&
                            details::isPowerOf2(N, 16),
                        void>::type
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset) {
  vector<T, N> dummy;
  details::is_valid_atomic_op<T, Op>::check();
  details::__cm_intrinsic_impl_atomic_write<Op, N, T>(
      mask, index, elementOffset, dummy, dummy, dummy);
}

// PREDEC: mask and no return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
(details::is_dword_type<T>::value) &&
details::isPowerOf2(N, 16),
void>::type
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
  vector<uint, N> elementOffset) {
  vector<T, N> dummy;
  details::is_valid_atomic_op<T, Op>::check();
  details::__cm_intrinsic_impl_atomic_write<ATOMIC_DEC, N, T>(
    mask, index, elementOffset, dummy, dummy, dummy);
}

// CMPXCHG: return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op == ATOMIC_CMPXCHG || Op == ATOMIC_FCMPWR) &&
               (details::is_fp_or_dword_type<T>::value) &&
                details::isPowerOf2(N, 16), void>::type
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
             vector<T, N> src0, vector<T, N> src1, vector_ref<T, N> ret) {
  vector_ref<T, N> _Ret = ret.format<T>();
  vector<ushort, N> mask = 1;
  _Ret = details::__cm_intrinsic_impl_atomic_write<Op, N, T>(
      mask, index, elementOffset, src0, src1, _Ret);
}

// CMPXCHG: mask and return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op == ATOMIC_CMPXCHG || Op == ATOMIC_FCMPWR) &&
               (details::is_fp_or_dword_type<T>::value) &&
                details::isPowerOf2(N, 16), void>::type
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset, vector<T, N> src0,
             vector<T, N> src1, vector_ref<T, N> ret) {
  vector_ref<T, N> _Ret = ret.format<uint>();
  _Ret = details::__cm_intrinsic_impl_atomic_write<Op, N, T>(
      mask, index, elementOffset, src0, src1, _Ret);
}

// CMPXCHG: no return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op == ATOMIC_CMPXCHG || Op == ATOMIC_FCMPWR) &&
               (details::is_fp_or_dword_type<T>::value) &&
                details::isPowerOf2(N, 16), void>::type
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
             vector<T, N> src0, vector<T, N> src1) {
  vector<T, N> dummy;
  write_atomic<Op, T>(index, elementOffset, src0, src1, dummy);
}

// CMPXCHG: mask and no return value
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op == ATOMIC_CMPXCHG || Op == ATOMIC_FCMPWR) &&
               (details::is_fp_or_dword_type<T>::value) &&
                details::isPowerOf2(N, 16), void>::type
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset, vector<T, N> src0,
             vector<T, N> src1) {
  vector<T, N> dummy;
  write_atomic<Op, T>(mask, index, elementOffset, src0, src1, dummy);
}

/// \brief Typed atomic write
///
/// \param Op template parameter Op determines the atomic operation to be
/// performed
///
/// \param mask (optional) predicate to specify on which channels to be performed.
///
/// \param surfIndex surface index, which must correspond to a 1D, 2D or 3D
/// surface.
///
/// \param ret vector containing the values in memory before the atomic
/// operation
/// was performed
///
/// \param u the x coordinates of the data elements to be read from surface,
/// which must be in unit of pixels. The size N must be 8
///
/// \param v (optional, default = 0) the y coordinates of the data elements to
/// be read from non-1D surface types; ignored otherwise.
///
/// \param r (optional, default = 0) the z coordinates of the data elements to
/// be read from 3D surface types; ignored otherwise.
///
/// The compiler generates code for GenX hardware to perform scattered aomic
/// write to the given offsets. Out-of-bound reads return zero, while
/// out-of-bound writes are dropped.
///

#if defined(CM_GEN7_5) || defined(CM_GEN8) || defined(CM_GEN8_5)
// Define a function to produce and error on unsupported architectures
// If no CM_GENn is defined (and possibly only CM_GENX) then the function will
// just appear to be unimplemented.
#define write_typed_atomic(...)                                                \
  CM_STATIC_ERROR(0, "write_typed_atomic is only supported for SKL+. Ensure "  \
                     "compile flags reflect this.");

#else

// Typed atomic {u}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<uint, N> u) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, dummy.select_all(), dummy.select_all(), u);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
  details::isPowerOf2(N, 8) &&
  details::is_dword_type<T>::value,
  void>::type
  write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
    vector<uint, N> u) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<ATOMIC_DEC>(
    mask, surfIndex, dummy.select_all(), dummy.select_all(), u) - 1;
}


// Predicated typed atomic {u}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<uint, N> u) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, dummy.select_all(), dummy.select_all(), u);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
  details::isPowerOf2(N, 8) &&
  details::is_dword_type<T>::value,
  void>::type
  write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
    vector_ref<T, N> ret, vector<uint, N> u) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<ATOMIC_DEC>(
    mask, surfIndex, dummy.select_all(), dummy.select_all(), u) - 1;
}

// Typed atomic {u, v}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<uint, N> u, vector<uint, N> v) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
  details::isPowerOf2(N, 8) &&
  details::is_dword_type<T>::value,
  void>::type
  write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
    vector<uint, N> u, vector<uint, N> v) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<ATOMIC_DEC>(
    mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v) - 1;
}

// Predicated typed atomic {u, v}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<uint, N> u, vector<uint, N> v) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
  details::isPowerOf2(N, 8) &&
  details::is_dword_type<T>::value,
  void>::type
  write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
    vector_ref<T, N> ret, vector<uint, N> u, vector<uint, N> v) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<ATOMIC_DEC>(
    mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v) - 1;
}

// Typed atomic {u, v, r}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<uint, N> u, vector<uint, N> v, vector<uint, N> r) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v, r);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
  details::isPowerOf2(N, 8) &&
  details::is_dword_type<T>::value,
  void>::type
  write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<ATOMIC_DEC>(
    mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v, r) - 1;
}

// Predicated typed atomic {u, v, r}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<uint, N> u, vector<uint, N> v,
                   vector<uint, N> r) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v, r);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
  details::isPowerOf2(N, 8) &&
  details::is_dword_type<T>::value,
  void>::type
  write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
    vector_ref<T, N> ret, vector<uint, N> u, vector<uint, N> v,
    vector<uint, N> r) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<ATOMIC_DEC>(
    mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v, r) - 1;
}

// Typed atomic {u, v, r, lod}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<uint, N> u, vector<uint, N> v, vector<uint, N> r,
                   vector<uint, N> lod) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v, r, lod);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
  details::isPowerOf2(N, 8) &&
  details::is_dword_type<T>::value,
  void>::type
  write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r,
    vector<uint, N> lod) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<ATOMIC_DEC>(
    mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v, r, lod) - 1;
}

// Predicated typed atomic {u, v, r, lod}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC) &&
                            details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<uint, N> u, vector<uint, N> v,
                   vector<uint, N> r, vector<uint, N> lod) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v, r, lod);
}

template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_PREDEC) &&
  details::isPowerOf2(N, 8) &&
  details::is_dword_type<T>::value,
  void>::type
  write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
    vector_ref<T, N> ret, vector<uint, N> u, vector<uint, N> v,
    vector<uint, N> r, vector<uint, N> lod) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<ATOMIC_DEC>(
    mask, surfIndex, dummy.select_all(), dummy.select_all(), u, v, r, lod) - 1;
}

// Typed atomic {u}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
        details::isPowerOf2(N, 8) && details::is_dword_type<T>::value,
    void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<T, N> src0, vector<uint, N> u) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, dummy.select_all(), u);
}

// Predicated typed atomic {u}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
        details::isPowerOf2(N, 8) && details::is_dword_type<T>::value,
    void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<T, N> src0, vector<uint, N> u) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, dummy.select_all(), u);
}

// Typed atomic {u, v}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
        details::isPowerOf2(N, 8) && details::is_dword_type<T>::value,
    void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<T, N> src0, vector<uint, N> u, vector<uint, N> v) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, dummy.select_all(), u, v);
}

// Predicated typed atomic {u, v}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
        details::isPowerOf2(N, 8) && details::is_dword_type<T>::value,
    void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<T, N> src0, vector<uint, N> u,
                   vector<uint, N> v) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, dummy.select_all(), u, v);
}

// Typed atomic {u, v, r}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
        details::isPowerOf2(N, 8) && details::is_dword_type<T>::value,
    void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<T, N> src0, vector<uint, N> u, vector<uint, N> v,
                   vector<uint, N> r) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, dummy.select_all(), u, v, r);
}

// Predicated typed atomic {u, v, r}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
        details::isPowerOf2(N, 8) && details::is_dword_type<T>::value,
    void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<T, N> src0, vector<uint, N> u,
                   vector<uint, N> v, vector<uint, N> r) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, dummy.select_all(), u, v, r);
}

// Typed atomic {u, v, r, lod}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
        details::isPowerOf2(N, 8) && details::is_dword_type<T>::value,
    void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<T, N> src0, vector<uint, N> u, vector<uint, N> v,
                   vector<uint, N> r, vector<uint, N> lod) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, dummy.select_all(), u, v, r, lod);
}

// Predicated typed atomic {u, v, r, lod}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<
    (Op != ATOMIC_CMPXCHG && Op != ATOMIC_INC && Op != ATOMIC_DEC && Op != ATOMIC_PREDEC) &&
        details::isPowerOf2(N, 8) && details::is_dword_type<T>::value,
    void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<T, N> src0, vector<uint, N> u,
                   vector<uint, N> v, vector<uint, N> r, vector<uint, N> lod) {
  vector<T, N> dummy;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, dummy.select_all(), u, v, r, lod);
}

// Typed atomic {u}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_CMPXCHG) && details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<T, N> src0, vector<T, N> src1, vector<uint, N> u) {
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(mask, surfIndex,
                                                            src0, src1, u);
}

// Predicated typed atomic {u}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_CMPXCHG) && details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
                   vector<uint, N> u) {
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(mask, surfIndex,
                                                            src0, src1, u);
}

// Typed atomic {u, v}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_CMPXCHG) && details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<T, N> src0, vector<T, N> src1, vector<uint, N> u,
                   vector<uint, N> v) {
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(mask, surfIndex,
                                                            src0, src1, u, v);
}

// Predicated typed atomic {u, v}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_CMPXCHG) && details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
                   vector<uint, N> u, vector<uint, N> v) {
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(mask, surfIndex,
                                                            src0, src1, u, v);
}

// Typed atomic {u, v, r}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_CMPXCHG) && details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<T, N> src0, vector<T, N> src1, vector<uint, N> u,
                   vector<uint, N> v, vector<uint, N> r) {
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, src1, u, v, r);
}

// Predicated typed atomic {u, v, r}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_CMPXCHG) && details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
                   vector<uint, N> u, vector<uint, N> v, vector<uint, N> r) {
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, src1, u, v, r);
}

// Typed atomic {u, v, r, lod}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_CMPXCHG) && details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret,
                   vector<T, N> src0, vector<T, N> src1, vector<uint, N> u,
                   vector<uint, N> v, vector<uint, N> r, vector<uint, N> lod) {
  vector<ushort, N> mask = 1;
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, src1, u, v, r, lod);
}

// Predicated typed atomic {u, v, r, lod}.
template <CmAtomicOpType Op, typename T, int N>
CM_NODEBUG CM_INLINE
typename std::enable_if<(Op == ATOMIC_CMPXCHG) && details::isPowerOf2(N, 8) &&
                            details::is_dword_type<T>::value,
                        void>::type
write_typed_atomic(vector<ushort, N> mask, SurfaceIndex surfIndex,
                   vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
                   vector<uint, N> u, vector<uint, N> v, vector<uint, N> r,
                   vector<uint, N> lod) {
  ret = details::__cm_intrinsic_impl_atomic_write_typed<Op>(
      mask, surfIndex, src0, src1, u, v, r, lod);
}
#endif

#endif
