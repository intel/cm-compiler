/*========================== begin_copyright_notice ============================

Copyright (C) 2014-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:cm_send.h should not be included explicitly");
#endif

#ifndef _CLANG_CM_SEND_H_
#define _CLANG_CM_SEND_H_

#include "cm_common.h"
#include "cm_traits.h"
#include "cm_util.h"

namespace details {
template <typename T, unsigned Width>
using PayloadRegTy =
    std::enable_if_t<is_dword_type<T>::value && isPowerOf2(Width, CM_GRF_WIDTH),
                     vector<T, Width> >;
} // namespace details

/// Access the thread payload register r0.
template <typename T = uint32_t, unsigned Width = 8>
details::PayloadRegTy<T, Width> cm_get_r0();

/// Access the thread status register sr0.
template <typename T = void> vector<uint, 4> cm_get_sr0();

template <typename T>
typename std::enable_if<std::is_same<T, SurfaceIndex>::value ||
                            std::is_same<T, SamplerIndex>::value ||
                            std::is_same<T, VmeIndex>::value,
                        uint>::type
cm_get_value(T index);

uint cm_unmask_begin();
void cm_unmask_end(uint tmp);

/// \brief Raw send and split send.
///
/// @rspVar the matrix that stores the message response data.
///
/// @msgVar the matrix that stores the message payload data.
///
/// @msgVar2 the matrix that stores the second part of the message payload data
/// in a split send (Gen9+).
///
/// @exDesc the extended message descriptor, which must be a compile time
/// constant.
///
/// @msgDesc the message descriptor for the send.
///
/// @sendc the flag that indicates whether sendc should be used. It must be a
/// compile time constant (0 indicates send should be used, while 1 means
/// sendc).
///
template <typename T, int N1, typename U, int N2>
typename std::enable_if<sizeof(T) * N1 % 32 == 0 &&
                        sizeof(U) * N2 % 32 == 0>::type
cm_send(vector_ref<T, N1> rspVar, vector<U, N2> msgVar, uint exDesc,
        uint msgDesc, uint sendc);

template <typename T, int N1, int N2, typename U, int N3, int N4>
typename std::enable_if<sizeof(T) * N1 * N2 % 32 == 0 &&
                        sizeof(U) * N3 * N4 % 32 == 0>::type
cm_send(matrix_ref<T, N1, N2> rspVar, matrix<U, N3, N4> msgVar, uint exDesc,
        uint msgDesc, uint sendc);

template <typename T, int N>
typename std::enable_if<sizeof(T) * N % 32 == 0>::type
cm_send(int dummy, vector<T, N> msgVar, uint exDesc, uint msgDesc, uint sendc);

template <typename T, int N1, int N2>
typename std::enable_if<sizeof(T) * N1 * N2 % 32 == 0>::type
cm_send(int dummy, matrix<T, N1, N2> msgVar, uint exDesc, uint msgDesc,
        uint sendc);

// Split send.
template <typename U1, int N1, typename U2, int N2, typename U3, int N3>
typename std::enable_if<sizeof(U1) * N1 % 32 == 0 &&
                        sizeof(U2) * N2 % 32 == 0 &&
                        sizeof(U3) * N3 % 32 == 0>::type
cm_sends(vector_ref<U1, N1> rspVar, vector<U2, N2> msgVar,
         vector<U3, N3> msgVar2, uint exDesc, uint msgDesc, uint sendc);

template <typename U1, int N1, int N2, typename U2, int N3, int N4, typename U3,
          int N5, int N6>
typename std::enable_if<sizeof(U1) * N1 * N2 % 32 == 0 &&
                        sizeof(U2) * N3 * N4 % 32 == 0 &&
                        sizeof(U3) * N5 * N6 % 32 == 0>::type
cm_sends(matrix_ref<U1, N1, N2> rspVar, matrix<U2, N3, N4> msgVar,
         matrix<U3, N5, N6> msgVar2, uint exDesc, uint msgDesc, uint sendc);

template <typename U1, int N1, typename U2, int N2>
typename std::enable_if<sizeof(U1) * N1 % 32 == 0 &&
                        sizeof(U2) * N2 % 32 == 0>::type
cm_sends(int dummy, vector<U1, N1> msgVar, vector<U2, N2> msgVar2, uint exDesc,
         uint msgDesc, uint sendc);

template <typename U1, int N1, int N2, typename U2, int N3, int N4>
typename std::enable_if<sizeof(U1) * N1 * N2 % 32 == 0 &&
                        sizeof(U2) * N3 * N4 % 32 == 0>::type
cm_sends(int dummy, matrix<U1, N1, N2> msgVar, matrix<U2, N3, N4> msgVar2,
         uint exDesc, uint msgDesc, uint sendc);

template <typename U1, int N1, typename U2, int N2,
  typename U3, int N3, int N = 16>
typename std::enable_if<sizeof(U1) * N1 % 32 == 0 &&
  sizeof(U2) * N2 % 32 == 0 &&
  sizeof(U3) * N3 % 32 == 0>::type
  cm_raw_send(vector_ref<U1, N1> rspVar,
    vector<U2, N2> msgVar,
    vector<U3, N3> msgVar2,
    uint exDesc, uint msgDesc,
    uchar execSize,  uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1);

template <typename U1, int N1, int N2, typename U2, int N3,
  int N4, typename U3, int N5, int N6, int N = 16>
typename std::enable_if<sizeof(U1) * N1 * N2 % 32 == 0 &&
  sizeof(U2) * N3 * N4 % 32 == 0 &&
  sizeof(U3) * N5 * N6 % 32 == 0>::type
  cm_raw_send(matrix_ref<U1, N1, N2> rspVar,
    matrix<U2, N3, N4> msgVar,
    matrix<U3, N5, N6> msgVar2,
    uint exDesc, uint msgDesc,
    uchar execSize, uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1);

template <typename U2, int N2, typename U3, int N3, int N = 16>
typename std::enable_if<sizeof(U2) * N2 % 32 == 0 &&
  sizeof(U3) * N3 % 32 == 0>::type
  cm_raw_send(
    int dummyDst,
    vector<U2, N2> msgVar,
    vector<U3, N3> msgVar2,
    uint exDesc, uint msgDesc,
    uchar execSize, uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1);

template <typename U2, int N3, int N4, typename U3,
  int N5, int N6, int N = 16>
typename std::enable_if<sizeof(U2) * N3 * N4 % 32 == 0 &&
  sizeof(U3) * N5 * N6 % 32 == 0>::type
  cm_raw_send(
    int dummyDst,
    matrix<U2, N3, N4> msgVar,
    matrix<U3, N5, N6> msgVar2,
    uint exDesc, uint msgDesc,
    uchar execSize, uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1);

template <typename U1, int N1, typename U2, int N2, int N = 16>
typename std::enable_if<sizeof(U1) * N1 % 32 == 0 &&
  sizeof(U2) * N2 % 32 == 0>::type
  cm_raw_send(vector_ref<U1, N1> rspVar,
    vector<U2, N2> msgVar,
    int dummySrc1,
    uint exDesc, uint msgDesc,
    uchar execSize, uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1);

template <typename U1, int N1, int N2, typename U2, int N3,
  int N4, int N = 16>
typename std::enable_if<sizeof(U1) * N1 * N2 % 32 == 0 &&
  sizeof(U2) * N3 * N4 % 32 == 0>::type
  cm_raw_send(matrix_ref<U1, N1, N2> rspVar,
    matrix<U2, N3, N4> msgVar,
    int dummySrc1,
    uint exDesc, uint msgDesc,
    uchar execSize, uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1);

template <typename U2, int N2, int N = 16>
typename std::enable_if<sizeof(U2) * N2 % 32 == 0>::type
  cm_raw_send(
    int dummyDst,
    vector<U2, N2> msgVar,
    int dummySrc1,
    uint exDesc, uint msgDesc,
    uchar execSize, uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1);

template <typename U2, int N3, int N4, int N = 16>
typename std::enable_if<sizeof(U2) * N3 * N4 % 32 == 0>::type
  cm_raw_send(
    int dummyDst,
    matrix<U2, N3, N4> msgVar,
    int dummySrc1,
    uint exDesc, uint msgDesc,
    uchar execSize, uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1);

#if __CM_INTEL_TARGET_MAJOR >= 35
namespace details {

template <int ExecSize, typename DstTy, int DstWidth, typename Src0Ty,
          int Src0Width, typename Src1Ty, int Src1Width>
vector<DstTy, DstWidth> __cm_intrinsic_impl_raw_sendg(
    uint16_t DstBytes, bool IsConditional, bool IsEOT, uint8_t SFID,
    vector<ushort, ExecSize> Pred, vector<Src0Ty, Src0Width> Src0,
    uint16_t Src0Bytes, vector<Src1Ty, Src1Width> Src1, uint16_t Src1Bytes,
    uint64_t Ind0, uint64_t Ind1, uint64_t Desc,
    vector<DstTy, DstWidth> Passthru);
} // namespace details

template <uint8_t SFID, uint64_t Desc, int ExecSize, typename DstTy,
          int DstWidth, typename Src0Ty, int Src0Width, typename Src1Ty,
          int Src1Width>
CM_NODEBUG CM_INLINE void cm_raw_sendg(vector_ref<DstTy, DstWidth> Dst,
                                       vector<Src0Ty, Src0Width> Src0,
                                       vector<Src1Ty, Src1Width> Src1,
                                       vector<uint16_t, ExecSize> Mask = 1) {
  using namespace details;
  constexpr uint16_t DstSize = sizeof(DstTy) * DstWidth;
  constexpr uint16_t Src0Size = sizeof(Src0Ty) * Src0Width;
  constexpr uint16_t Src1Size = sizeof(Src1Ty) * Src1Width;

  uint64_t Undef; // It's explicitly uninitialized

  bool IsConditional = false;
  bool IsEOT = false;

  Dst = __cm_intrinsic_impl_raw_sendg(DstSize, IsConditional, IsEOT, SFID, Mask,
                                      Src0, Src0Size, Src1, Src1Size, Undef,
                                      Undef, Desc, Dst);
}

template <uint8_t SFID, uint64_t Desc, int ExecSize, typename Src0Ty,
          int Src0Width, typename Src1Ty, int Src1Width>
CM_NODEBUG CM_INLINE void cm_raw_sendg(int NullDst,
                                       vector<Src0Ty, Src0Width> Src0,
                                       vector<Src1Ty, Src1Width> Src1,
                                       vector<uint16_t, ExecSize> Mask = 1) {
  using namespace details;
  constexpr uint16_t Src0Size = sizeof(Src0Ty) * Src0Width;
  constexpr uint16_t Src1Size = sizeof(Src1Ty) * Src1Width;

  vector<int, 16> UndefVector; // It's explicitly uninitialized
  uint64_t Undef;              // It's explicitly uninitialized

  bool IsConditional = false;
  bool IsEOT = false;

  __cm_intrinsic_impl_raw_sendg(0, IsConditional, IsEOT, SFID, Mask, Src0,
                                Src0Size, Src1, Src1Size, Undef, Undef, Desc,
                                UndefVector);
}

template <uint8_t SFID, uint64_t Desc, int ExecSize, typename DstTy,
          int DstWidth, typename Src0Ty, int Src0Width>
CM_NODEBUG CM_INLINE void
cm_raw_sendg(vector_ref<DstTy, DstWidth> Dst, vector<Src0Ty, Src0Width> Src0,
             int NullSrc1 = 0, vector<uint16_t, ExecSize> Mask = 1) {
  using namespace details;
  constexpr uint16_t DstSize = sizeof(DstTy) * DstWidth;
  constexpr uint16_t Src0Size = sizeof(Src0Ty) * Src0Width;

  vector<int, 16> UndefVector; // It's explicitly uninitialized
  uint64_t Undef;              // It's explicitly uninitialized

  bool IsConditional = false;
  bool IsEOT = false;

  Dst = __cm_intrinsic_impl_raw_sendg(DstSize, IsConditional, IsEOT, SFID, Mask,
                                      Src0, Src0Size, UndefVector, 0, Undef,
                                      Undef, Desc, Dst);
}

template <uint8_t SFID, uint64_t Desc, int ExecSize, typename Src0Ty,
          int Src0Width>
CM_NODEBUG CM_INLINE void
cm_raw_sendg(int NullDst, vector<Src0Ty, Src0Width> Src0, int NullSrc1,
             vector<uint16_t, ExecSize> Mask = 1) {
  using namespace details;
  constexpr uint16_t Src0Size = sizeof(Src0Ty) * Src0Width;

  vector<int, 16> UndefVector; // It's explicitly uninitialized
  uint64_t Undef;              // It's explicitly uninitialized

  bool IsConditional = false;
  bool IsEOT = false;

  __cm_intrinsic_impl_raw_sendg(0, IsConditional, IsEOT, SFID, Mask, Src0,
                                Src0Size, UndefVector, 0, Undef, Undef, Desc,
                                UndefVector);
}

template <uint8_t SFID, uint64_t Desc, int ExecSize, typename DstTy,
          int DstWidth, typename Src0Ty, int Src0Width, typename Src1Ty,
          int Src1Width>
CM_NODEBUG CM_INLINE void
cm_raw_sendg(vector_ref<DstTy, DstWidth> Dst, vector<Src0Ty, Src0Width> Src0,
             vector<Src1Ty, Src1Width> Src1, uint64_t Ind0,
             vector<uint16_t, ExecSize> Mask = 1) {
  using namespace details;
  constexpr uint16_t DstSize = sizeof(DstTy) * DstWidth;
  constexpr uint16_t Src0Size = sizeof(Src0Ty) * Src0Width;
  constexpr uint16_t Src1Size = sizeof(Src1Ty) * Src1Width;

  uint64_t Undef; // It's explicitly uninitialized

  bool IsConditional = false;
  bool IsEOT = false;

  Dst = __cm_intrinsic_impl_raw_sendg(DstSize, IsConditional, IsEOT, SFID, Mask,
                                      Src0, Src0Size, Src1, Src1Size, Ind0,
                                      Undef, Desc, Dst);
}

template <uint8_t SFID, uint64_t Desc, int ExecSize, typename Src0Ty,
          int Src0Width, typename Src1Ty, int Src1Width>
CM_NODEBUG CM_INLINE void
cm_raw_sendg(int NullDst, vector<Src0Ty, Src0Width> Src0,
             vector<Src1Ty, Src1Width> Src1, uint64_t Ind0,
             vector<uint16_t, ExecSize> Mask = 1) {
  using namespace details;
  constexpr uint16_t Src0Size = sizeof(Src0Ty) * Src0Width;
  constexpr uint16_t Src1Size = sizeof(Src1Ty) * Src1Width;

  vector<int, 16> UndefVector; // It's explicitly uninitialized
  uint64_t Undef;              // It's explicitly uninitialized

  bool IsConditional = false;
  bool IsEOT = false;

  __cm_intrinsic_impl_raw_sendg(0, IsConditional, IsEOT, SFID, Mask, Src0,
                                Src0Size, Src1, Src1Size, Ind0, Undef, Desc,
                                UndefVector);
}

template <uint8_t SFID, uint64_t Desc, int ExecSize, typename DstTy,
          int DstWidth, typename Src0Ty, int Src0Width>
CM_NODEBUG CM_INLINE void
cm_raw_sendg(vector_ref<DstTy, DstWidth> Dst, vector<Src0Ty, Src0Width> Src0,
             int NullSrc1, uint64_t Ind0, vector<uint16_t, ExecSize> Mask = 1) {
  using namespace details;
  constexpr uint16_t DstSize = sizeof(DstTy) * DstWidth;
  constexpr uint16_t Src0Size = sizeof(Src0Ty) * Src0Width;

  vector<int, 16> UndefVector; // It's explicitly uninitialized
  uint64_t Undef;              // It's explicitly uninitialized

  bool IsConditional = false;
  bool IsEOT = false;

  Dst = __cm_intrinsic_impl_raw_sendg(DstSize, IsConditional, IsEOT, SFID, Mask,
                                      Src0, Src0Size, UndefVector, 0, Ind0,
                                      Undef, Desc, Dst);
}

template <uint8_t SFID, uint64_t Desc, int ExecSize, typename Src0Ty,
          int Src0Width>
CM_NODEBUG CM_INLINE void
cm_raw_sendg(int NullDst, vector<Src0Ty, Src0Width> Src0, int NullSrc1,
             uint64_t Ind0, vector<uint16_t, ExecSize> Mask = 1) {
  using namespace details;
  constexpr uint16_t Src0Size = sizeof(Src0Ty) * Src0Width;

  vector<int, 16> UndefVector; // It's explicitly uninitialized
  uint64_t Undef;              // It's explicitly uninitialized

  bool IsConditional = false;
  bool IsEOT = false;

  __cm_intrinsic_impl_raw_sendg(0, IsConditional, IsEOT, SFID, Mask, Src0,
                                Src0Size, UndefVector, 0, Ind0, Undef, Desc,
                                UndefVector);
}
#endif // __CM_INTEL_TARGET_MAJOR >= 35
#endif
