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
static_assert(0, "CM:w:cm_internal.h should not be included explicitly - only "
                 "<cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_INTERNAL_H_
#define _CLANG_CM_INTERNAL_H_

#include "cm_common.h"
#include "cm_util.h"

namespace details {

template <typename T0, typename T1, int SZ>
vector<T0, SZ> __cm_intrinsic_impl_sat(vector<T1, SZ> src);

template <typename T0, typename T1> T0 __cm_intrinsic_impl_sat(T1 src);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_abs(vector<T, SZ> src0);

template <typename T0, typename T1, int SZ>
vector<T0, SZ> __cm_intrinsic_impl_add(vector<T1, SZ> src0, vector<T1, SZ> src1,
                                       int flag);

template <typename T0, typename T1, int SZ>
vector<T0, SZ> __cm_intrinsic_impl_mul(vector<T1, SZ> src0, vector<T1, SZ> src1,
                                       int flag);

template <typename T0, typename T1, int SZ>
vector<T0, SZ> __cm_intrinsic_impl_avg(vector<T1, SZ> src0, vector<T1, SZ> src1,
                                       int flag);

template <typename T0, typename T1, int SZ>
vector<T0, SZ> __cm_intrinsic_impl_shl(vector<T1, SZ> src0, vector<T1, SZ> src1,
                                       int flag);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_imul(vector<T, SZ> src0, vector<T, SZ> src1);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_dp2(vector<float, SZ> src0,
                                          vector<float, SZ> src1);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_dp3(vector<float, SZ> src0,
                                          vector<float, SZ> src1);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_dp4(vector<float, SZ> src0,
                                          vector<float, SZ> src1);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_dph(vector<float, SZ> src0,
                                          vector<float, SZ> src1);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_frc(vector<float, SZ> src0);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_line(vector<float, 4> src0,
                                           vector<float, SZ> src1);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_max(vector<T, SZ> src0, vector<T, SZ> src1);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_lzd(vector<T, SZ> src0);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_min(vector<T, SZ> src0, vector<T, SZ> src1);

//sad2 has 8-bit inputs and 16-bit result, no need for saturation
template <typename T0, typename T1, int SZ>
vector<T0, SZ> __cm_intrinsic_impl_sad2(vector<T1, SZ> src0,
                                        vector<T1, SZ> src1);

template <typename T0, typename T1, int SZ>
vector<T0, SZ> __cm_intrinsic_impl_sada2(vector<T1, SZ> src0,
                                         vector<T1, SZ> src1,
                                         vector<T0, SZ> src2, int flag);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_lrp(vector<float, SZ> src0,
                                          vector<float, SZ> src1,
                                          vector<float, SZ> src2);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_pln(vector<float, 4> src0,
                                          vector<float, SZ * 2> src1);

template <typename T0, typename T1, int SZ>
vector<T0, SZ> __cm_intrinsic_impl_bfrev(vector<T1, SZ> src0);

template <typename T, int SZ>
vector<unsigned int, SZ> __cm_intrinsic_impl_cbit(vector<T, SZ> src0);

template <typename T0, int SZ>
vector<T0, SZ>
__cm_intrinsic_impl_bfins(vector<T0, SZ> src0, vector<T0, SZ> src1,
                          vector<T0, SZ> src2, vector<T0, SZ> src3);

template <typename T0, int SZ>
vector<T0, SZ> __cm_intrinsic_impl_bfext(vector<T0, SZ> src0,
                                         vector<T0, SZ> src1,
                                         vector<T0, SZ> src2);

template <int SZ>
vector<uint, SZ> __cm_intrinsic_impl_fbl(vector<uint, SZ> src0);

template <typename T0, int SZ>
vector<int, SZ> __cm_intrinsic_impl_sfbh(vector<T0, SZ> src0);

template <typename T0, int SZ>
vector<uint, SZ> __cm_intrinsic_impl_ufbh(vector<T0, SZ> src0);

template <typename T0, typename T1, int SZ>
T0 __cm_intrinsic_impl_sum(vector<T1, SZ> src);
template <typename T0, typename T1, int SZ>
T0 __cm_intrinsic_impl_sum_sat(vector<T1, SZ> src);
template <typename T0, typename T1, int SZ>
T0 __cm_intrinsic_impl_prod(vector<T1, SZ> src);
template <typename T0, typename T1, int SZ>
T0 __cm_intrinsic_impl_prod_sat(vector<T1, SZ> src);
template <typename T, int SZ>
T __cm_intrinsic_impl_reduced_min(vector<T, SZ> src);
template <typename T, int SZ>
T __cm_intrinsic_impl_reduced_max(vector<T, SZ> src);

template <int SZ>
vector<half, SZ> __cm_intrinsic_impl_inv(vector<half, SZ> src0);
template <int SZ>
vector<half, SZ> __cm_intrinsic_impl_log(vector<half, SZ> src0);
template <int SZ>
vector<half, SZ> __cm_intrinsic_impl_exp(vector<half, SZ> src0);
template <int SZ>
vector<half, SZ> __cm_intrinsic_impl_sqrt(vector<half, SZ> src0);
template <int SZ>
vector<half, SZ> __cm_intrinsic_impl_rsqrt(vector<half, SZ> src0);
template <int SZ>
vector<half, SZ> __cm_intrinsic_impl_sin(vector<half, SZ> src0);
template <int SZ>
vector<half, SZ> __cm_intrinsic_impl_cos(vector<half, SZ> src0);
template <int SZ>
vector<half, SZ> __cm_intrinsic_impl_pow(vector<half, SZ> src0,
                                         vector<half, SZ> src1);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_inv(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_log(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_exp(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_sqrt(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_sqrt_ieee(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_rsqrt(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_sin(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_cos(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_pow(vector<float, SZ> src0,
                                          vector<float, SZ> src1);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_div_ieee(vector<float, SZ> src0,
                                               vector<float, SZ> src1);

template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_rndd(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_rndu(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_rnde(vector<float, SZ> src0);
template <int SZ>
vector<float, SZ> __cm_intrinsic_impl_rndz(vector<float, SZ> src0);

template <int SZ>
vector<double, SZ> __cm_intrinsic_impl_sqrt_ieee(vector<double, SZ> src0);
template <int SZ>
vector<double, SZ> __cm_intrinsic_impl_div_ieee(vector<double, SZ> src0,
                                                vector<double, SZ> src1);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_oword_read(SurfaceIndex index, int offset);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_oword_read_dwaligned(SurfaceIndex index,
                                                       int offset);

template <typename T, int SZ>
void __cm_intrinsic_impl_oword_write(SurfaceIndex index, int offset,
                                     vector<T, SZ> src);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_slm_oword_read(uint slmBuffer, int offset);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_slm_oword_read_dwaligned(uint slmBuffer,
                                                       int offset);

template <typename T, int SZ>
void __cm_intrinsic_impl_slm_oword_write(uint slmBuffer, int offset,
                                     vector<T, SZ> src);

template <typename T, int N, int M, int _M, CmBufferAttrib attr>
matrix<T, N, _M> __cm_intrinsic_impl_media_read(SurfaceIndex index, int X,
                                                int Y);

template <typename T, int N, int M, int _M, CmBufferAttrib attr>
void __cm_intrinsic_impl_media_write(SurfaceIndex index, int X, int Y,
                                     matrix<T, N, _M> src);

template <typename T, int N, int M, CmBufferAttrib attr,
          CmSurfacePlaneIndex plane>
matrix<T, N, M> __cm_intrinsic_impl_read_plane(SurfaceIndex index, int X,
                                               int Y);

template <typename T, int N, int M, int _M, CmBufferAttrib attr,
          CmSurfacePlaneIndex plane>
void __cm_intrinsic_impl_write_plane(SurfaceIndex index, int X, int Y,
                                     matrix<T, N, _M> src);

template <typename T, int N, ChannelMaskType Mask>
matrix<T, N, 16>
__cm_intrinsic_impl_sample16(SamplerIndex sampIndex, SurfaceIndex surfIndex,
                             vector<float, 16> u, vector<float, 16> v,
                             vector<float, 16> r);

template <int N, ChannelMaskType Mask, OutputFormatControl Ofc>
matrix<ushort, N, 32>
__cm_intrinsic_impl_sample32(SamplerIndex sampIndex, SurfaceIndex surfIndex,
                             float u, float v, float deltaU, float deltaV);

template <typename T, int N, ChannelMaskType Mask>
matrix<T, N, 16>
__cm_intrinsic_impl_load16(SurfaceIndex surfIndex, vector<uint, 16> u,
                           vector<uint, 16> v, vector<uint, 16> r);

template <CmAtomicOpType Op, int N, typename T>
vector<T, N> __cm_intrinsic_impl_atomic_write(vector<ushort, N> mask,
                                              SurfaceIndex index,
                                              vector<uint, N> elementOffset,
                                              vector<T, N> src0,
                                              vector<T, N> src1,
                                              vector<T, N> oldVal);

template <CmAtomicOpType Op, typename T, int N, typename... Args>
vector<T, N>
__cm_intrinsic_impl_atomic_write_typed(vector<ushort, N> mask,
                                       SurfaceIndex surfIndex,
                                       vector<T, N> src0, vector<T, N> src1,
                                       vector<uint, N> u, Args... args);

template <typename T0, typename T1, int N>
vector<T1, N> __cm_intrinsic_impl_scatter_read(SurfaceIndex index,
                                               uint globalOffset,
                                               vector<uint, N> elementOffset,
                                               vector<T1, N> oldVal, T0 dummy);

template <typename T0, typename T1, int N>
void __cm_intrinsic_impl_scatter_write(SurfaceIndex index, uint globalOffset,
                                       vector<uint, N> elementOffset,
                                       vector<T1, N> data, T0 dummy);

// This is gather_scaled intrinsic and with surface index T0 and scale 0.
template <typename T, int N, int NBlocks>
vector<T, N> __cm_intrinsic_impl_slm_read(uint globalOffsetInBytes,
                                          vector<uint, N> elementOffsetInBytes,
                                          vector<T, N> data);

// This is scatter_scaled intrinsic with surface index T0 and scale 0.
template <typename T, int N, int NBlocks>
void __cm_intrinsic_impl_slm_write(uint globalOffsetInBytes,
                                   vector<uint, N> elementOffsetInBytes,
                                   vector<T, N> data);

// SVM support
template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_svm_block_read(uint64_t addr);

template <typename T, int SZ>
vector<T, SZ> __cm_intrinsic_impl_svm_block_read_unaligned(uint64_t addr);

template <typename T, int SZ>
void __cm_intrinsic_impl_svm_block_write(uint64_t addr, vector<T, SZ> src);

template <typename T, int N>
vector<T, N> __cm_intrinsic_impl_svm_scatter_read(vector<uint64_t, N> vAddr,
                                                  vector<T, N> oldVal);

template <typename T, int N>
void __cm_intrinsic_impl_svm_scatter_write(vector<uint64_t, N> vAddr,
                                           vector<T, N> src);

template <CmAtomicOpType Op, typename T, int N>
vector<T, N> __cm_intrinsic_impl_svm_atomic(vector<uint64_t, N> vAddr,
                                            vector<T, N> oldVal);

template <CmAtomicOpType Op, typename T, int N>
vector<T, N> __cm_intrinsic_impl_svm_atomic(vector<uint64_t, N> vAddr,
                                            vector<T, N> src0,
                                            vector<T, N> oldVal);

template <CmAtomicOpType Op, typename T, int N>
vector<T, N>
__cm_intrinsic_impl_svm_atomic(vector<uint64_t, N> vAddr, vector<T, N> src0,
                               vector<T, N> src1, vector<T, N> oldVal);

template <typename T, int N>
bool __cm_intrinsic_impl_simdfork_any(vector<T, N> t, const char *filename,
                                      unsigned line);
template <typename T, int R, int C>
bool __cm_intrinsic_impl_simdfork_any(matrix<T, R, C> t, const char *filename,
                                      unsigned line);

template <typename T = void>
bool __cm_intrinsic_impl_simdfork_any(int t, const char *filename, unsigned line);

template <typename T, int N>
bool __cm_intrinsic_impl_simdcf_any(vector<T, N> t, const char *filename,
                                    unsigned line);
template <typename T, int R, int C>
bool __cm_intrinsic_impl_simdcf_any(matrix<T, R, C> t, const char *filename,
                                    unsigned line);

template <typename T = void>
bool __cm_intrinsic_impl_simdcf_any(int t, const char *filename, unsigned line);

template <typename T0, typename T1, int N>
vector<T0, N> __cm_intrinsic_impl_simdcf_predgen(vector<T0, N> arg0, T1 arg1);

template <typename T0, int N>
vector<T0, N> __cm_intrinsic_impl_simdcf_predmin(vector<T0, N> arg0);

template <typename T0, int N>
vector<T0, N> __cm_intrinsic_impl_simdcf_predmax(vector<T0, N> arg0);

template <int N> uint __cm_intrinsic_impl_pack_mask(vector<ushort, N> src0);

template <int N1, int N2>
uint __cm_intrinsic_impl_pack_mask(matrix<ushort, N1, N2> src0);

template <typename T, int N>
vector<ushort, N> __cm_intrinsic_impl_unpack_mask(uint src0);

// Dummy mov inserts a mov to a null location from the src0
// Initial intended use is to allow for dependencies for results from wait_event
// intrinsic call
template <typename T> void __cm_intrinsic_impl_dummy_mov(T src0);

// Predefined surface support.
SurfaceIndex __cm_intrinsic_impl_predefined_surface(unsigned id);


} // namespace details

#endif
