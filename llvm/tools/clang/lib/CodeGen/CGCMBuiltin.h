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

//===----------------------------------------------------------------------===//
// CM builtin code generation.
//===----------------------------------------------------------------------===//

#ifndef CLANG_CODEGEN_CM_BUILTIN_H
#define CLANG_CODEGEN_CM_BUILTIN_H

namespace clang {
namespace CodeGen {

/// \brief All supported cm builtin.
enum CMBuiltinKind {
  // This is sorted according to the language specification.
  // Overloaded builtins share the same enum value.
  CMBK_none, // not a builtin
  // This is a standalone implementation builtin used by other
  // CM builtin's implementation.
  CMBK_cm_sat_impl,
  CMBK_simdcf_any_impl,
  CMBK_simdcf_predgen_impl,
  CMBK_simdcf_predmin_impl,
  CMBK_simdcf_predmax_impl,
  // CM builtins
  CMBK_cm_abs, CMBK_cm_abs_impl,
  CMBK_cm_add, CMBK_cm_add_impl,
  CMBK_cm_mul, CMBK_cm_mul_impl,
  CMBK_cm_avg, CMBK_cm_avg_impl,
  CMBK_cm_dp2, CMBK_cm_dp2_impl,
  CMBK_cm_dp3, CMBK_cm_dp3_impl,
  CMBK_cm_dp4, CMBK_cm_dp4_impl,
  CMBK_cm_dph, CMBK_cm_dph_impl,
  CMBK_cm_frc, CMBK_cm_frc_impl,
  CMBK_cm_line, CMBK_cm_line_impl,
  CMBK_cm_lzd, CMBK_cm_lzd_impl,
  CMBK_cm_max, CMBK_cm_max_impl,
  CMBK_cm_min, CMBK_cm_min_impl,
  CMBK_cm_rndd, CMBK_cm_rndd_impl,
  CMBK_cm_rndu, CMBK_cm_rndu_impl,
  CMBK_cm_rnde, CMBK_cm_rnde_impl,
  CMBK_cm_rndz, CMBK_cm_rndz_impl,
  CMBK_cm_sad2, CMBK_cm_sad2_impl,
  CMBK_cm_sada2, CMBK_cm_sada2_impl,
  CMBK_cm_sum, CMBK_cm_sum_impl, CMBK_cm_sum_sat_impl,
  CMBK_cm_reduced_min, CMBK_cm_reduced_min_impl,
  CMBK_cm_reduced_max, CMBK_cm_reduced_max_impl,
  CMBK_cm_prod, CMBK_cm_prod_impl, CMBK_cm_prod_sat_impl,
  CMBK_cm_inv, CMBK_cm_inv_impl,
  CMBK_cm_log, CMBK_cm_log_impl,
  CMBK_cm_exp, CMBK_cm_exp_impl,
  CMBK_cm_sqrt, CMBK_cm_sqrt_impl,
  CMBK_cm_rsqrt, CMBK_cm_rsqrt_impl,
  CMBK_cm_sqrt_ieee, CMBK_cm_sqrt_ieee_impl,
  CMBK_cm_pow, CMBK_cm_pow_impl,
  CMBK_cm_sin, CMBK_cm_sin_impl,
  CMBK_cm_cos, CMBK_cm_cos_impl,
  CMBK_cm_sincos,
  CMBK_cm_atan2,
  CMBK_cm_atan2_fast,
  CMBK_cm_div_ieee, CMBK_cm_div_ieee_impl,
  CMBK_cm_imul, CMBK_cm_imul_impl,
  CMBK_cm_pack_mask,
  CMBK_cm_unpack_mask,
  CMBK_cm_fbl, CMBK_cm_fbl_impl,
  CMBK_cm_fbh, CMBK_cm_sfbh_impl, CMBK_cm_ufbh_impl,
  CMBK_cm_rdtsc,
  CMBK_cm_shl, CMBK_cm_shl_impl,
  CMBK_cm_pln, CMBK_cm_pln_impl,
  CMBK_cm_lrp, CMBK_cm_lrp_impl,
  CMBK_cm_cbit, CMBK_cm_cbit_impl,
  CMBK_cm_bfrev, CMBK_cm_bfrev_impl,
  CMBK_cm_bfins, CMBK_cm_bfins_impl,
  CMBK_cm_bfext, CMBK_cm_bfext_impl,
  // Data port interface.
  CMBK_oword_read_impl,
  CMBK_oword_read_dwaligned_impl,
  CMBK_oword_write_impl,
  CMBK_media_read_impl,
  CMBK_media_write_impl,
  CMBK_scatter_read_impl,
  CMBK_scatter_write_impl,
  CMBK_read,
  CMBK_write,
  CMBK_read_plane,
  CMBK_write_plane,
  CMBK_read_transpose,
  CMBK_read_untyped,
  CMBK_write_untyped,
  CMBK_read_typed,
  CMBK_write_typed,
  // new API for atomic writes
  CMBK_write_atomic,
  CMBK_write_atomic_impl,
  CMBK_write_atomic_typed,
  CMBK_write_atomic_typed_impl,
  // Shared local memory and groups interface.
  CMBK_slm_oword_read_impl,
  CMBK_slm_oword_read_dwaligned_impl,
  CMBK_slm_oword_write_impl,
  CMBK_cm_slm_write, CMBK_cm_slm_write_impl,
  CMBK_cm_slm_write4,
  CMBK_cm_slm_write4_scaled,
  CMBK_cm_slm_read, CMBK_cm_slm_read_impl,
  CMBK_cm_slm_read4,
  CMBK_cm_slm_read4_scaled,
  CMBK_cm_slm_atomic,
  // Sampler interface.
  CMBK_sample16, CMBK_sample16_impl,
  CMBK_sample32, CMBK_sample32_impl,
  CMBK_cm_3d_sample,
  CMBK_load16, CMBK_load16_impl,
  CMBK_cm_3d_load,
  // Shared virtual memory interface
  CMBK_svm_block_read_impl,
  CMBK_svm_block_read_unaligned_impl,
  CMBK_svm_block_write_impl,
  CMBK_svm_scatter_read_impl,
  CMBK_svm_scatter_write_impl,
  CMBK_cm_svm_atomic,
  CMBK_cm_svm_atomic_impl,
  // Adaptie video scaling.
  CMBK_cm_avs_sampler,
  CMBK_cm_va_2d_convolve,
  CMBK_cm_va_min_max_filter,
  CMBK_cm_va_min_max,
  CMBK_cm_va_erode,
  CMBK_cm_va_dilate,
  CMBK_cm_va_centroid,
  CMBK_cm_va_boolean_centroid,
  CMBK_cm_va_1d_convolution,
  CMBK_cm_va_1pixel_convolve,
  CMBK_cm_va_lbp_creation,
  CMBK_cm_va_lbp_correlation,
  CMBK_cm_va_flood_fill,
  CMBK_cm_va_correlation_search,
  CMBK_cm_va_2d_convolve_hdc,
  CMBK_cm_va_min_max_filter_hdc,
  CMBK_cm_va_erode_hdc,
  CMBK_cm_va_dilate_hdc,
  CMBK_cm_va_1d_convolution_hdc,
  CMBK_cm_va_1pixel_convolve_hdc,
  CMBK_cm_va_lbp_creation_hdc,
  CMBK_cm_va_lbp_correlation_hdc,
  // VME interface
  // TODO.
  // Synchronization and misc. functions.
  CMBK_cm_send,
  CMBK_cm_sends,
  CMBK_cm_get_value,
  CMBK_cm_get_r0,
  CMBK_cm_get_sr0,
  CMBK_rdregion,
  CMBK_wrregion,
  CMBK_cm_label,
  CMBK_predefined_surface
};

} // namespace CodeGen
} // namespace clang

#endif
