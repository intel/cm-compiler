/*========================== begin_copyright_notice ============================

Copyright (C) 2014-2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// CM builtin code generation.

#include "CGCM.h"
#include "CodeGenFunction.h"

#include "clang/AST/DeclTemplate.h"
#include "clang/Basic/Builtins.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Intrinsics.h"

#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include "llvm/GenXIntrinsics/GenXMetadata.h"

using namespace clang;
using namespace CodeGen;

namespace {

// Constant in number of bytes.
enum { BYTE = 1, WORD = 2, DWORD = 4, QWORD = 8, OWORD = 16, GRF = 32 };

// cm_fence()/cm_slm_fence(): always enable fence_global by setting this bit 1
enum {
  CM_FLUSH_GLOBAL_BIT_MASK = 0x01,
};

typedef enum _CmBufferAttrib_ {
  GENX_NONE = 0,
  GENX_TOP_FIELD = 1,
  GENX_BOTTOM_FIELD = 2,
  GENX_DWALIGNED = 3,
  GENX_MODIFIED = 4,
  GENX_MODIFIED_TOP_FIELD = 5,
  GENX_MODIFIED_BOTTOM_FIELD = 6,
  GENX_MODIFIED_DWALIGNED = 7,
  GENX_CONSTANT = 8,
  GENX_CONSTANT_DWALIGNED = 9,
  GENX_NUM_BUFFER_ATTRIB = 10
} CmBufferAttrib;

// media load inst modifiers
typedef enum {
  MEDIA_LD_nomod = 0x0,
  MEDIA_LD_modified = 0x1,
  MEDIA_LD_top = 0x2,
  MEDIA_LD_bottom = 0x3,
  MEDIA_LD_top_mod = 0x4,
  MEDIA_LD_bottom_mod = 0x5,
  MEDIA_LD_Mod_NUM
} MEDIA_LD_mod;

// media store inst modifiers
typedef enum {
  MEDIA_ST_nomod = 0x0,
  MEDIA_ST_reserved = 0x1,
  MEDIA_ST_top = 0x2,
  MEDIA_ST_bottom = 0x3,
  MEDIA_ST_Mod_NUM
} MEDIA_ST_mod;

// DO NOT MODIFY THE ENCODING.
typedef enum {
  CM_R_ENABLE = 1,
  CM_G_ENABLE = 2,
  CM_GR_ENABLE = 3,
  CM_B_ENABLE = 4,
  CM_BR_ENABLE = 5,
  CM_BG_ENABLE = 6,
  CM_BGR_ENABLE = 7,
  CM_A_ENABLE = 8,
  CM_AR_ENABLE = 9,
  CM_AG_ENABLE = 10,
  CM_AGR_ENABLE = 11,
  CM_AB_ENABLE = 12,
  CM_ABR_ENABLE = 13,
  CM_ABG_ENABLE = 14,
  CM_ABGR_ENABLE = 15
} ChannelMaskType;

/// Returns the number of color enabled.
inline unsigned getNumberOfColors(ChannelMaskType M) {
  switch (M) {
  default:
    break;
  case CM_R_ENABLE:
  case CM_G_ENABLE:
  case CM_B_ENABLE:
  case CM_A_ENABLE:
    return 1;
  case CM_GR_ENABLE:
  case CM_BR_ENABLE:
  case CM_BG_ENABLE:
  case CM_AR_ENABLE:
  case CM_AG_ENABLE:
  case CM_AB_ENABLE:
    return 2;
  case CM_BGR_ENABLE:
  case CM_AGR_ENABLE:
  case CM_ABR_ENABLE:
  case CM_ABG_ENABLE:
    return 3;
  case CM_ABGR_ENABLE:
    return 4;
  }

  return 0;
}

} // namespace

/// Get the integral value encoded as a non-type template argument.
template <typename T = unsigned>
static T getIntegralValue(const FunctionDecl *FD, unsigned Index) {
  assert(FD && FD->isTemplateInstantiation());
  const TemplateArgumentList *TempArgs = FD->getTemplateSpecializationArgs();
  if (std::is_unsigned<T>())
    return TempArgs->get(Index).getAsIntegral().getZExtValue();
  return TempArgs->get(Index).getAsIntegral().getSExtValue();
}

CMBuiltinKind CGCMRuntime::getCMBuiltinKind(StringRef MangledName) const {
  using llvm::StringSwitch;
  // FIXME: We may need to tag builtin functions/templates in Sema.
  CMBuiltinKind Kind =
      StringSwitch<CMBuiltinKind>(MangledName)
          .StartsWith("_Z4read", CMBK_read)
          .StartsWith("_Z5write", CMBK_write)
          .StartsWith("_Z12read_untyped", CMBK_read_untyped)
          .StartsWith("_Z13write_untyped", CMBK_write_untyped)
          .StartsWith("_Z10read_typed", CMBK_read_typed)
          .StartsWith("_Z11write_typed", CMBK_write_typed)
          .StartsWith("_Z11cm_slm_read", CMBK_cm_slm_read)
          .StartsWith("_Z12cm_slm_write", CMBK_cm_slm_write)
          .StartsWith("_Z19cm_slm_read4_scaled", CMBK_cm_slm_read4_scaled)
          .StartsWith("_Z12cm_slm_read4", CMBK_cm_slm_read4)
          .StartsWith("_Z20cm_slm_write4_scaled", CMBK_cm_slm_write4_scaled)
          .StartsWith("_Z13cm_slm_write4", CMBK_cm_slm_write4)
          .StartsWith("_Z13cm_slm_atomic", CMBK_cm_slm_atomic)
          .StartsWith("_Z12cm_ptr_read4", CMBK_cm_svm_read4_impl)
          .StartsWith("_Z13cm_ptr_write4", CMBK_cm_svm_write4_impl)
          .StartsWith("_Z6cm_abs", CMBK_cm_abs)
          .StartsWith("_Z6cm_add", CMBK_cm_add)
          .StartsWith("_Z6cm_mul", CMBK_cm_mul)
          .StartsWith("_Z11cm_div_ieee", CMBK_cm_div_ieee)
          .StartsWith("_Z6cm_dp2", CMBK_cm_dp2)
          .StartsWith("_Z6cm_dp3", CMBK_cm_dp3)
          .StartsWith("_Z6cm_dp4", CMBK_cm_dp4)
          .StartsWith("_Z6cm_dph", CMBK_cm_dph)
          .StartsWith("_Z7cm_line", CMBK_cm_line)
          .StartsWith("_Z6cm_max", CMBK_cm_max)
          .StartsWith("_Z6cm_min", CMBK_cm_min)
          .StartsWith("_Z6cm_inv", CMBK_cm_inv)
          .StartsWith("_Z6cm_log", CMBK_cm_log)
          .StartsWith("_Z6cm_exp", CMBK_cm_exp)
          .StartsWith("_Z7cm_sqrt", CMBK_cm_sqrt)
          .StartsWith("_Z8cm_rsqrt", CMBK_cm_rsqrt)
          .StartsWith("_Z12cm_sqrt_ieee", CMBK_cm_sqrt_ieee)
          .StartsWith("_Z6cm_pow", CMBK_cm_pow)
          .StartsWith("_Z6cm_sin", CMBK_cm_sin)
          .StartsWith("_Z6cm_cos", CMBK_cm_cos)
          .StartsWith("_Z7cm_rndd", CMBK_cm_rndd)
          .StartsWith("_Z7cm_rndu", CMBK_cm_rndu)
          .StartsWith("_Z7cm_rnde", CMBK_cm_rnde)
          .StartsWith("_Z7cm_rndz", CMBK_cm_rndz)
          .StartsWith("_Z6cm_sum", CMBK_cm_sum)
          .StartsWith("_Z7cm_prod", CMBK_cm_prod)
          .StartsWith("_Z14cm_reduced_min", CMBK_cm_reduced_min)
          .StartsWith("_Z14cm_reduced_max", CMBK_cm_reduced_max)
          .StartsWith("_Z8sample16", CMBK_sample16)
          .StartsWith("_Z8sample32", CMBK_sample32)
          .StartsWith("_Z6load16", CMBK_load16)
          .StartsWith("_Z18write_atomic_typed", CMBK_write_atomic_typed)
          .StartsWith("_Z12write_atomic", CMBK_write_atomic)
          .StartsWith("_Z8cm_rdtsc", CMBK_cm_rdtsc)
          .StartsWith("_Z11cm_raw_send", CMBK_cm_raw_send)
          .StartsWith("_Z7cm_send", CMBK_cm_send)
          .StartsWith("_Z8cm_sends", CMBK_cm_sends)
          .StartsWith("_Z9cm_get_r0", CMBK_cm_get_r0)
          .StartsWith("_Z10cm_get_sr0", CMBK_cm_get_sr0)
          .StartsWith("_Z15cm_unmask_begin", CMBK_cm_unmask_begin)
          .StartsWith("_Z13cm_unmask_end", CMBK_cm_unmask_end)
          .StartsWith("_Z12cm_get_value", CMBK_cm_get_value)
          .StartsWith("_Z14cm_avs_sampler", CMBK_cm_avs_sampler)
          .StartsWith("_Z17cm_va_2d_convolve", CMBK_cm_va_2d_convolve)
          .StartsWith("_Z21cm_va_2d_convolve_hdc", CMBK_cm_va_2d_convolve_hdc)
          .StartsWith("_Z11cm_va_erode", CMBK_cm_va_erode)
          .StartsWith("_Z15cm_va_erode_hdc", CMBK_cm_va_erode_hdc)
          .StartsWith("_Z12cm_va_dilate", CMBK_cm_va_dilate)
          .StartsWith("_Z16cm_va_dilate_hdc", CMBK_cm_va_dilate_hdc)
          .StartsWith("_Z13cm_va_min_max", CMBK_cm_va_min_max)
          .StartsWith("_Z20cm_va_min_max_filter", CMBK_cm_va_min_max_filter)
          .StartsWith("_Z24cm_va_min_max_filter_hdc",
                      CMBK_cm_va_min_max_filter_hdc)
          .StartsWith("_Z22cm_va_boolean_centroid", CMBK_cm_va_boolean_centroid)
          .StartsWith("_Z14cm_va_centroid", CMBK_cm_va_centroid)
          .StartsWith("_Z20cm_va_1d_convolution", CMBK_cm_va_1d_convolution)
          .StartsWith("_Z24cm_va_1d_convolution_hdc",
                      CMBK_cm_va_1d_convolution_hdc)
          .StartsWith("_Z21cm_va_1pixel_convolve", CMBK_cm_va_1pixel_convolve)
          .StartsWith("_Z25cm_va_1pixel_convolve_hdc",
                      CMBK_cm_va_1pixel_convolve_hdc)
          .StartsWith("_Z18cm_va_lbp_creation", CMBK_cm_va_lbp_creation)
          .StartsWith("_Z22cm_va_lbp_creation_hdc", CMBK_cm_va_lbp_creation_hdc)
          .StartsWith("_Z21cm_va_lbp_correlation", CMBK_cm_va_lbp_correlation)
          .StartsWith("_Z25cm_va_lbp_correlation_hdc",
                      CMBK_cm_va_lbp_correlation_hdc)
          .StartsWith("_Z24cm_va_correlation_search",
                      CMBK_cm_va_correlation_search)
          .StartsWith("_Z16cm_va_flood_fill", CMBK_cm_va_flood_fill)
          .StartsWith("_Z12cm_3d_sample", CMBK_cm_3d_sample)
          .StartsWith("_Z10cm_3d_load", CMBK_cm_3d_load)
          .StartsWith("_Z13cm_svm_atomic", CMBK_cm_svm_atomic)
          .StartsWith("_Z6cm_shl", CMBK_cm_shl)
          .StartsWith("_Z7cm_dp4a", CMBK_cm_dp4a)
          .StartsWith("_Z6cm_bfn", CMBK_cm_bfn)
          .StartsWith("_Z7cm_dpas", CMBK_cm_dpas)
          .Default(CMBK_none);

  // Handle implementation intrinsics.
  if (Kind == CMBK_none) {
    // Skip the namespace prefix. String switch should always match the longest
    // name sharing the same prefix.
    size_t Start = MangledName.find("__cm_intrinsic_impl");
    StringRef MangledImplName = MangledName.substr(Start);
    // Order matters here. If there are two strings to check for and one is a
    // prefix of the other, the longer one must be checked for first.
    Kind =
        StringSwitch<CMBuiltinKind>(MangledImplName)
            .StartsWith("__cm_intrinsic_impl_bfn", CMBK_cm_bfn_impl)
            .StartsWith("__cm_intrinsic_impl_dpasw_nosrc0",
                        CMBK_cm_dpasw_nosrc0_impl)
            .StartsWith("__cm_intrinsic_impl_dpasw", CMBK_cm_dpasw_impl)
            .StartsWith("__cm_intrinsic_impl_dpas_nosrc0",
                        CMBK_cm_dpas_nosrc0_impl)
            .StartsWith("__cm_intrinsic_impl_dpas", CMBK_cm_dpas2_impl)
            .StartsWith("__cm_intrinsic_impl_tf32_cvt", CMBK_cm_tf32_cvt_impl)
            .StartsWith("__cm_intrinsic_impl_srnd", CMBK_cm_srnd_impl)
            .StartsWith("__cm_intrinsic_impl_prefetch_bti",
                        CMBK_cm_prefetch_impl)
            .StartsWith("__cm_intrinsic_impl_block_prefetch_bti",
                        CMBK_cm_block_prefetch_impl)
            .StartsWith("__cm_intrinsic_impl_prefetch_flat",
                        CMBK_cm_prefetch_flat_impl)
            .StartsWith("__cm_intrinsic_impl_block_prefetch_flat",
                        CMBK_cm_block_prefetch_flat_impl)
            .StartsWith("__cm_intrinsic_impl_load_bti", CMBK_cm_load_impl)
            .StartsWith("__cm_intrinsic_impl_load4_bti", CMBK_cm_load4_impl)
            .StartsWith("__cm_intrinsic_impl_block_load_bti",
                        CMBK_cm_block_load_impl)
            .StartsWith("__cm_intrinsic_impl_store_bti", CMBK_cm_store_impl)
            .StartsWith("__cm_intrinsic_impl_store4_bti", CMBK_cm_store4_impl)
            .StartsWith("__cm_intrinsic_impl_block_store_bti",
                        CMBK_cm_block_store_impl)
            .StartsWith("__cm_intrinsic_impl_load_flat", CMBK_cm_load_flat_impl)
            .StartsWith("__cm_intrinsic_impl_load4_flat",
                        CMBK_cm_load4_flat_impl)
            .StartsWith("__cm_intrinsic_impl_block_load_flat",
                        CMBK_cm_block_load_flat_impl)
            .StartsWith("__cm_intrinsic_impl_store_flat",
                        CMBK_cm_store_flat_impl)
            .StartsWith("__cm_intrinsic_impl_store4_flat",
                        CMBK_cm_store4_flat_impl)
            .StartsWith("__cm_intrinsic_impl_block_store_flat",
                        CMBK_cm_block_store_flat_impl)
            .StartsWith("__cm_intrinsic_impl_load_slm", CMBK_cm_load_slm_impl)
            .StartsWith("__cm_intrinsic_impl_load4_slm", CMBK_cm_load4_slm_impl)
            .StartsWith("__cm_intrinsic_impl_store_slm", CMBK_cm_store_slm_impl)
            .StartsWith("__cm_intrinsic_impl_store4_slm",
                        CMBK_cm_store4_slm_impl)
            .StartsWith("__cm_intrinsic_impl_block_prefetch2d_flat",
                        CMBK_cm_block_prefetch2d_flat_impl)
            .StartsWith("__cm_intrinsic_impl_block_load2d_flat",
                        CMBK_cm_block_load2d_flat_impl)
            .StartsWith("__cm_intrinsic_impl_block_store2d_flat",
                        CMBK_cm_block_store2d_flat_impl)
            .StartsWith("__cm_intrinsic_impl_load4_typed_bti",
                        CMBK_cm_load4_typed_impl)
            .StartsWith("__cm_intrinsic_impl_store4_typed_bti",
                        CMBK_cm_store4_typed_impl)
            .StartsWith("__cm_intrinsic_impl_prefetch4_typed_bti",
                        CMBK_cm_prefetch4_typed_impl)
            .StartsWith("__cm_intrinsic_impl_prefetch2d_bti",
                        CMBK_cm_prefetch2d_bti_impl)
            .StartsWith("__cm_intrinsic_impl_load2d_bti",
                        CMBK_cm_load2d_bti_impl)
            .StartsWith("__cm_intrinsic_impl_store2d_bti",
                        CMBK_cm_store2d_bti_impl)
            .StartsWith("__cm_intrinsic_impl_lsc_atomic_bti",
                        CMBK_cm_atomic_bti_impl)
            .StartsWith("__cm_intrinsic_impl_lsc_atomic_slm",
                        CMBK_cm_atomic_slm_impl)
            .StartsWith("__cm_intrinsic_impl_lsc_atomic_flat",
                        CMBK_cm_atomic_flat_impl)
            .StartsWith("__cm_intrinsic_impl_lsc_fence", CMBK_cm_lsc_fence_impl)
            .StartsWith("__cm_intrinsic_impl_dp4a", CMBK_cm_dp4a_impl)
            .StartsWith("__cm_intrinsic_impl_oword_read_dwaligned",
                        CMBK_oword_read_dwaligned_impl)
            .StartsWith("__cm_intrinsic_impl_oword_read", CMBK_oword_read_impl)
            .StartsWith("__cm_intrinsic_impl_oword_write",
                        CMBK_oword_write_impl)
            .StartsWith("__cm_intrinsic_impl_media_read", CMBK_media_read_impl)
            .StartsWith("__cm_intrinsic_impl_media_write",
                        CMBK_media_write_impl)
            .StartsWith("__cm_intrinsic_impl_read_plane", CMBK_read_plane)
            .StartsWith("__cm_intrinsic_impl_write_plane", CMBK_write_plane)
            .StartsWith("__cm_intrinsic_impl_scatter_read",
                        CMBK_scatter_read_impl)
            .StartsWith("__cm_intrinsic_impl_scatter_write",
                        CMBK_scatter_write_impl)
            .StartsWith("__cm_intrinsic_impl_sat", CMBK_cm_sat_impl)
            .StartsWith("__cm_intrinsic_impl_abs", CMBK_cm_abs_impl)
            .StartsWith("__cm_intrinsic_impl_addc", CMBK_cm_addc_impl)
            .StartsWith("__cm_intrinsic_impl_subb", CMBK_cm_subb_impl)
            .StartsWith("__cm_intrinsic_impl_add", CMBK_cm_add_impl)
            .StartsWith("__cm_intrinsic_impl_mul", CMBK_cm_mul_impl)
            .StartsWith("__cm_intrinsic_impl_div_ieee", CMBK_cm_div_ieee_impl)
            .StartsWith("__cm_intrinsic_impl_avg", CMBK_cm_avg_impl)
            .StartsWith("__cm_intrinsic_impl_dp2", CMBK_cm_dp2_impl)
            .StartsWith("__cm_intrinsic_impl_dp3", CMBK_cm_dp3_impl)
            .StartsWith("__cm_intrinsic_impl_dp4", CMBK_cm_dp4_impl)
            .StartsWith("__cm_intrinsic_impl_dph", CMBK_cm_dph_impl)
            .StartsWith("__cm_intrinsic_impl_line", CMBK_cm_line_impl)
            .StartsWith("__cm_intrinsic_impl_fbl", CMBK_cm_fbl_impl)
            .StartsWith("__cm_intrinsic_impl_sfbh", CMBK_cm_sfbh_impl)
            .StartsWith("__cm_intrinsic_impl_ufbh", CMBK_cm_ufbh_impl)
            .StartsWith("__cm_intrinsic_impl_frc", CMBK_cm_frc_impl)
            .StartsWith("__cm_intrinsic_impl_lzd", CMBK_cm_lzd_impl)
            .StartsWith("__cm_intrinsic_impl_max", CMBK_cm_max_impl)
            .StartsWith("__cm_intrinsic_impl_min", CMBK_cm_min_impl)
            .StartsWith("__cm_intrinsic_impl_inv", CMBK_cm_inv_impl)
            .StartsWith("__cm_intrinsic_impl_log", CMBK_cm_log_impl)
            .StartsWith("__cm_intrinsic_impl_exp", CMBK_cm_exp_impl)
            .StartsWith("__cm_intrinsic_impl_sqrt_ieee", CMBK_cm_sqrt_ieee_impl)
            .StartsWith("__cm_intrinsic_impl_sqrt", CMBK_cm_sqrt_impl)
            .StartsWith("__cm_intrinsic_impl_rsqrt", CMBK_cm_rsqrt_impl)
            .StartsWith("__cm_intrinsic_impl_pow", CMBK_cm_pow_impl)
            .StartsWith("__cm_intrinsic_impl_sin", CMBK_cm_sin_impl)
            .StartsWith("__cm_intrinsic_impl_cos", CMBK_cm_cos_impl)
            .StartsWith("__cm_intrinsic_impl_imad", CMBK_cm_imad_impl)
            .StartsWith("__cm_intrinsic_impl_rndd", CMBK_cm_rndd_impl)
            .StartsWith("__cm_intrinsic_impl_rndu", CMBK_cm_rndu_impl)
            .StartsWith("__cm_intrinsic_impl_rnde", CMBK_cm_rnde_impl)
            .StartsWith("__cm_intrinsic_impl_rndz", CMBK_cm_rndz_impl)
            .StartsWith("__cm_intrinsic_impl_sum_sat", CMBK_cm_sum_sat_impl)
            .StartsWith("__cm_intrinsic_impl_sum", CMBK_cm_sum_impl)
            .StartsWith("__cm_intrinsic_impl_prod_sat", CMBK_cm_prod_sat_impl)
            .StartsWith("__cm_intrinsic_impl_prod", CMBK_cm_prod_impl)
            .StartsWith("__cm_intrinsic_impl_reduced_min",
                        CMBK_cm_reduced_min_impl)
            .StartsWith("__cm_intrinsic_impl_reduced_max",
                        CMBK_cm_reduced_max_impl)
            .StartsWith("__cm_intrinsic_impl_sample16", CMBK_sample16_impl)
            .StartsWith("__cm_intrinsic_impl_sample32", CMBK_sample32_impl)
            .StartsWith("__cm_intrinsic_impl_load16", CMBK_load16_impl)
            .StartsWith("__cm_intrinsic_impl_atomic_write_typed",
                        CMBK_write_atomic_typed_impl)
            .StartsWith("__cm_intrinsic_impl_atomic_write",
                        CMBK_write_atomic_impl)
            .StartsWith("__cm_intrinsic_impl_simdcf_any", CMBK_simdcf_any_impl)
            .StartsWith("__cm_intrinsic_impl_simdcf_predgen",
                        CMBK_simdcf_predgen_impl)
            .StartsWith("__cm_intrinsic_impl_simdcf_predmin",
                        CMBK_simdcf_predmin_impl)
            .StartsWith("__cm_intrinsic_impl_simdcf_predmax",
                        CMBK_simdcf_predmax_impl)
            .StartsWith("__cm_intrinsic_impl_shl", CMBK_cm_shl_impl)
            .StartsWith("__cm_intrinsic_impl_rol", CMBK_cm_rol_impl)
            .StartsWith("__cm_intrinsic_impl_ror", CMBK_cm_ror_impl)
            .StartsWith("__cm_intrinsic_impl_sad2", CMBK_cm_sad2_impl)
            .StartsWith("__cm_intrinsic_impl_sada2", CMBK_cm_sada2_impl)
            .StartsWith("__cm_intrinsic_impl_lrp", CMBK_cm_lrp_impl)
            .StartsWith("__cm_intrinsic_impl_pln", CMBK_cm_pln_impl)
            .StartsWith("__cm_intrinsic_impl_bfrev", CMBK_cm_bfrev_impl)
            .StartsWith("__cm_intrinsic_impl_cbit", CMBK_cm_cbit_impl)
            .StartsWith("__cm_intrinsic_impl_bfins", CMBK_cm_bfins_impl)
            .StartsWith("__cm_intrinsic_impl_bfext", CMBK_cm_bfext_impl)
            .StartsWith("__cm_intrinsic_impl_pack_mask", CMBK_cm_pack_mask)
            .StartsWith("__cm_intrinsic_impl_unpack_mask", CMBK_cm_unpack_mask)
            .StartsWith("__cm_intrinsic_impl_predefined_surface",
                        CMBK_predefined_surface)
            .StartsWith("__cm_intrinsic_impl_svm_atomic",
                        CMBK_cm_svm_atomic_impl)
            .StartsWith("__cm_intrinsic_impl_rdregion", CMBK_rdregion)
            .StartsWith("__cm_intrinsic_impl_wrregion", CMBK_wrregion)
            .Default(CMBK_none);
  }
  // Handle other builtin implementations.
  if (Kind == CMBK_none) {
    size_t Start = MangledName.find("__cm_builtin_impl");
    StringRef MangledImplName = MangledName.substr(Start);
    Kind =
        StringSwitch<CMBuiltinKind>(MangledImplName)
            .StartsWith("__cm_builtin_impl_load", CMBK_load_impl)
            .StartsWith("__cm_builtin_impl_store", CMBK_store_impl)
            .StartsWith("__cm_builtin_impl_gather", CMBK_gather_impl)
            .StartsWith("__cm_builtin_impl_scatter", CMBK_scatter_impl)
            // For cm_library support.
            .StartsWith("__cm_builtin_impl_svm_gather", CMBK_gather_svm_impl)
            .StartsWith("__cm_builtin_impl_svm_scatter", CMBK_scatter_svm_impl)
            .Default(CMBK_none);
  }
  return Kind;
}

static llvm::Value *EmitGenXIntrinsicCall(CodeGenFunction &CGF, unsigned ID) {
  assert(ID > llvm::GenXIntrinsic::not_genx_intrinsic && "Bad genx intrinsic");
  return CGF.Builder.CreateCall(CGF.CGM.getGenXIntrinsic(ID));
}

static RValue EmitGenXIntrinsicCall(CodeGenFunction &CGF, const CallExpr *E,
                                    unsigned ID0, unsigned ID1, unsigned ID2) {
  const Expr *Arg = E->getArg(0);
  llvm::Value *Val = CGF.EmitAnyExpr(Arg).getScalarVal();
  if (llvm::ConstantInt *CI = dyn_cast<llvm::ConstantInt>(Val)) {
    uint64_t Dim = CI->getZExtValue();
    if (Dim == 0)
      return RValue::get(EmitGenXIntrinsicCall(CGF, ID0));
    else if (Dim == 1)
      return RValue::get(EmitGenXIntrinsicCall(CGF, ID1));
    else if (Dim == 2)
      return RValue::get(EmitGenXIntrinsicCall(CGF, ID2));

    CGF.CGM.Error(Arg->getExprLoc(),
                  "0, 1 or 2 expected for dimension argument");
    return RValue::get(0);
  }

  // For variable dim argument, compare it with 0 and 1 (all others get 2)
  llvm::Value *C0 = CGF.Builder.CreateICmpEQ(
      Val, llvm::Constant::getNullValue(Val->getType()), "cmp0");
  llvm::Value *C1 = CGF.Builder.CreateICmpEQ(
      Val, llvm::Constant::getIntegerValue(Val->getType(), llvm::APInt(32, 1)),
      "cmp1");
  llvm::Value *V0 = EmitGenXIntrinsicCall(CGF, ID0);
  llvm::Value *V1 = EmitGenXIntrinsicCall(CGF, ID1);
  llvm::Value *V2 = EmitGenXIntrinsicCall(CGF, ID2);

  llvm::Value *res1 = CGF.Builder.CreateSelect(C1, V1, V2);
  return RValue::get(CGF.Builder.CreateSelect(C0, V0, res1));
}

static RValue EmitIntrinsicCallIndexed(CodeGenFunction &CGF, const CallExpr *E,
                                       llvm::Function *Fn) {
  const Expr *Arg = E->getArg(0);
  llvm::Value *Val = CGF.EmitAnyExpr(Arg).getScalarVal();

  llvm::Value *IntrinVec = CGF.Builder.CreateCall(Fn);

  return RValue::get(CGF.Builder.CreateExtractElement(IntrinVec, Val));
}

/// Return the MDNode that has the SLM size attribute.
static llvm::MDNode *getSLMSizeMDNode(llvm::Function *F) {
  llvm::NamedMDNode *Nodes =
      F->getParent()->getNamedMetadata(llvm::genx::FunctionMD::GenXKernels);
  assert(Nodes);
  for (auto Node : Nodes->operands()) {
    if (Node->getNumOperands() > llvm::genx::KernelMDOp::SLMSize &&
        getVal(Node->getOperand(llvm::genx::KernelMDOp::FunctionRef)) == F)
      return Node;
  }
  return nullptr;
}

RValue CGCMRuntime::EmitCMBuiltin(CodeGenFunction &CGF, unsigned ID,
                                  const CallExpr *E) {
  unsigned IID = llvm::GenXIntrinsic::not_genx_intrinsic;
  llvm::Function *Fn = 0;

  switch (ID) {
  default:
    break;
  case Builtin::BIget_thread_origin_x:
    IID = llvm::GenXIntrinsic::genx_thread_x;
    break;
  case Builtin::BIget_thread_origin_y:
    IID = llvm::GenXIntrinsic::genx_thread_y;
    break;
  case Builtin::BIget_color:
    IID = llvm::GenXIntrinsic::genx_get_color;
    break;
  case Builtin::BIcm_lane_id:
    IID = llvm::GenXIntrinsic::genx_lane_id;
    break;
  case Builtin::BIcm_scoreboard_bti:
    IID = llvm::GenXIntrinsic::genx_get_scoreboard_bti;
    break;
  case Builtin::BIcm_scoreboard_deltas:
    IID = llvm::GenXIntrinsic::genx_get_scoreboard_deltas;
    break;
  case Builtin::BIcm_scoreboard_depcnt:
    IID = llvm::GenXIntrinsic::genx_get_scoreboard_depcnt;
    break;
  case Builtin::BIcm_pause: {
    // cm_pause has a single argument - generate the call here
    Fn = CGF.CGM.getGenXIntrinsic(llvm::GenXIntrinsic::genx_set_pause);
    const Expr *ArgE = E->getArg(0);
    llvm::Value *Arg = CGF.EmitAnyExpr(ArgE).getScalarVal();
    return RValue::get(CGF.Builder.CreateCall(Fn, Arg, ""));
  }
  case Builtin::BI__cm_builtin_dummy_mov: {
    // intrinsic has a single argument - generate the call here
    Fn = CGF.CGM.getGenXIntrinsic(llvm::GenXIntrinsic::genx_dummy_mov);
    const Expr *ArgE = E->getArg(0);
    llvm::Value *Arg = CGF.EmitAnyExpr(ArgE).getScalarVal();
    return RValue::get(CGF.Builder.CreateCall(Fn, Arg, ""));
  }
  // cm_fence(), cm_slm_fence() and __cm_builtin_cm_wait() have variable
  // number of arguments: 0 or 1. Therefore we can't specify the arg type
  // (unsigned char) in Builtins.def.
  case Builtin::BIcm_fence:
  case Builtin::BIcm_slm_fence:
    Fn = CGF.CGM.getGenXIntrinsic(llvm::GenXIntrinsic::genx_fence);
    if (E->getNumArgs() == 1) {
      const Expr *Arg = E->getArg(0);
      Expr::EvalResult MaskResult;
      if (!Arg->EvaluateAsInt(MaskResult, CGF.getContext())) {
        Error(Arg->getExprLoc(), "integeral constant expected for masks");
        return RValue::get(0);
      }
      llvm::APSInt Mask = MaskResult.Val.getInt();
      unsigned char MaskVal = static_cast<unsigned char>(Mask.getZExtValue());
      // Clear reserved bits and only use first 5 bits
      MaskVal &= 0xFF;
      if (ID == Builtin::BIcm_slm_fence)
        MaskVal |= 0x20; // Enable SLM mode

      return RValue::get(CGF.Builder.CreateCall(
          Fn,
          llvm::ConstantInt::get(Fn->getFunctionType()->getParamType(0),
                                 MaskVal | CM_FLUSH_GLOBAL_BIT_MASK),
          ""));
    } else if (E->getNumArgs() == 0) {
      return RValue::get(CGF.Builder.CreateCall(
          Fn, llvm::ConstantInt::get(Fn->getFunctionType()->getParamType(0),
                                     CM_FLUSH_GLOBAL_BIT_MASK)));
    } else {
      Error(E->getExprLoc(), "One or zero mask argument expected");
      return RValue::get(0);
    }

  case Builtin::BI__cm_builtin_cm_wait:
    Fn = CGF.CGM.getGenXIntrinsic(llvm::GenXIntrinsic::genx_wait);
    if (E->getNumArgs() == 1) {
      const Expr *Arg = E->getArg(0);
      if (!Arg->getType()->isIntegralOrEnumerationType()) {
        Error(Arg->getExprLoc(), "integeral mask argument expected");
        return RValue::get(0);
      }
      llvm::Value *Mask = CGF.EmitAnyExpr(Arg).getScalarVal();
      llvm::Instruction::CastOps CastOp;
      if (getCastOpKind(CastOp, CGF, CGF.getContext().UnsignedCharTy,
                        Arg->getType())) {
        llvm::Type *MaskTy = Fn->getFunctionType()->getParamType(0);
        Mask = CGF.Builder.CreateCast(CastOp, Mask, MaskTy);
      }

      return RValue::get(CGF.Builder.CreateCall(Fn, Mask));
    } else if (E->getNumArgs() == 0) {
      return RValue::get(CGF.Builder.CreateCall(
          Fn,
          llvm::ConstantInt::get(Fn->getFunctionType()->getParamType(0), 0)));
    } else {
      Error(E->getExprLoc(), "One or zero mask argument expected");
      return RValue::get(0);
    }
  case Builtin::BIcm_nbarrier_init:
    EmitBuiltinNBarrierInit(CGF, E);
    return RValue::get(0);
  case Builtin::BIcm_nbarrier_wait: {
    Fn = CGF.CGM.getGenXIntrinsic(llvm::GenXIntrinsic::genx_nbarrier);
    if (E->getNumArgs() == 1) {
      SmallVector<llvm::Value *, 8> Args;
      Args.push_back(llvm::ConstantInt::get(CGF.Int8Ty, 0));
      const Expr *ArgE = E->getArg(0);
      llvm::Value *Id = CGF.EmitAnyExpr(ArgE).getScalarVal();
      Args.push_back(Id);
      Args.push_back(llvm::ConstantInt::get(CGF.Int8Ty, 0));
      return RValue::get(CGF.Builder.CreateCall(Fn, Args, ""));
    } else {
      Error(E->getExprLoc(), "One barrier id argument expected");
      return RValue::get(0);
    }
  }
  case Builtin::BIcm_slm_init:
    EmitBuiltinSLMInit(CGF, E);
    return RValue::get(0);
  case Builtin::BIcm_slm_alloc:
    return RValue::get(EmitBuiltinSLMAlloc(CGF, E));
  case Builtin::BIcm_slm_free:
    return RValue::get(EmitBuiltinSLMFree(CGF, E));
  case Builtin::BIcm_yield:
    Fn = CGF.CGM.getGenXIntrinsic(llvm::GenXIntrinsic::genx_yield);
    CGF.Builder.CreateCall(Fn);
    return RValue::get(0);
  }

  if (IID == llvm::GenXIntrinsic::not_genx_intrinsic)
    llvm_unreachable("not implemented yet");

  return RValue::get(EmitGenXIntrinsicCall(CGF, IID));
}

llvm::AllocaInst *CGCMRuntime::getOrCreateSLMIndexVar(CodeGenFunction &CGF) {
  auto iter = SLMAllocas.find(CGF.CurFn);
  if (iter != SLMAllocas.end())
    return iter->second;

  // Create and initialize the slm index variable.
  auto IndexVar = new llvm::AllocaInst(CGF.Int32Ty, /*AddrSpace*/ 0, nullptr,
                                       "slm.index", CGF.AllocaInsertPt);
  uint64_t BitWidth = CGF.Int32Ty->getIntegerBitWidth();
  llvm::Align Alignment{BitWidth / 8ull};
  IndexVar->setAlignment(Alignment);
  SLMAllocas.insert(std::make_pair(CGF.CurFn, IndexVar));

  // r0[27:24] Shared Local Memory Index.
  //
  // Indicates the starting index for the shared local memory for the thread
  // group. Each index points to the start of a 4K memory block, 16
  // possibilities cover the entire 64K shared memory per half-slice.
  //
  // Initialize it now.
  {
    // We intentionally emit the initialization code in the first basic block.
    // That is, initialization will always be performed and it should be.
    CGBuilderTy Builder(CGF, CGF.AllocaInsertPt);
    Builder.CreateDefaultAlignedStore(llvm::Constant::getNullValue(CGF.Int32Ty),
                                      IndexVar);
  }

  return IndexVar;
}

static void checkSLMSize(CGCMRuntime &CMRT, SourceLocation Loc,
                         llvm::Function *F) {
  uint64_t SLMSize = 0;
  if (llvm::MDNode *Node = getSLMSizeMDNode(F)) {
    llvm::Value *SzVal =
        getVal(Node->getOperand(llvm::genx::KernelMDOp::SLMSize));
    if (auto *CI = dyn_cast_or_null<llvm::ConstantInt>(SzVal))
      SLMSize = CI->getZExtValue();
  }

  // The maximal size is determined by platform
  auto &TOpts = CMRT.CGM.getTarget().getTargetOpts();
  unsigned MaxSLMSize = TOpts.CMMaxSLMSize * 1024; // in bytes

  if (SLMSize == 0)
    CMRT.Error(Loc, "use slm, but slm is not initialized");
  else if (SLMSize > MaxSLMSize)
    CMRT.Error(Loc, "use slm, but slm size is too large");
}

void CGCMRuntime::EmitBuiltinSLMInit(CodeGenFunction &CGF, const CallExpr *E) {
  // We check whether this call is inside a kernel function.
  if (!CGF.CurFuncDecl->hasAttr<CMGenxMainAttr>()) {
    Error(E->getExprLoc(), "cm_slm_init shall only be called in a kernel");
    return;
  }

  const Expr *Arg = E->getArg(0);
  Expr::EvalResult SizeResult;
  if (!Arg->EvaluateAsInt(SizeResult, CGF.getContext())) {
    Error(Arg->getExprLoc(), "integral constant expected for slm size");
    return;
  }
  llvm::APSInt Size = SizeResult.Val.getInt();
  // Size in bytes being requested.
  uint64_t NewVal = Size.getZExtValue();
  if (NewVal == 0) {
    Error(Arg->getExprLoc(), "zero slm bytes being requested");
    return;
  }

  // find the corresponding kernel metadata and set the SLM size.
  if (llvm::MDNode *Node = getSLMSizeMDNode(CGF.CurFn)) {
    if (llvm::Value *OldSz =
            getVal(Node->getOperand(llvm::genx::KernelMDOp::SLMSize))) {
      assert(isa<llvm::ConstantInt>(OldSz) && "integer constant expected");
      llvm::Value *NewSz = llvm::ConstantInt::get(OldSz->getType(), NewVal);
      uint64_t OldVal = cast<llvm::ConstantInt>(OldSz)->getZExtValue();
      if (OldVal < NewVal)
        Node->replaceOperandWith(llvm::genx::KernelMDOp::SLMSize, getMD(NewSz));
    }
  }

  // Initialize the index variable.
  getOrCreateSLMIndexVar(CGF);
}

llvm::Value *CGCMRuntime::EmitBuiltinSLMAlloc(CodeGenFunction &CGF,
                                              const CallExpr *E) {
  // We check whether this call is inside a kernel function.
  if (!CGF.CurFuncDecl->hasAttr<CMGenxMainAttr>()) {
    Error(E->getExprLoc(), "cm_slm_alloc shall only be called in a kernel");
    return llvm::Constant::getNullValue(CGF.Int32Ty);
  }

  // Sanity checking.
  checkSLMSize(*this, E->getExprLoc(), CGF.CurFn);

  llvm::AllocaInst *IndexVar = getOrCreateSLMIndexVar(CGF);
  llvm::Value *CurIndex = CGF.Builder.CreateDefaultAlignedLoad(IndexVar);
  llvm::Value *OffsetInBytes = CGF.EmitAnyExpr(E->getArg(0)).getScalarVal();
  // FIXME: We are not aligning the index. Should it be at least dword-aligned?
  llvm::Value *NextIndex = CGF.Builder.CreateAdd(CurIndex, OffsetInBytes);
  CGF.Builder.CreateDefaultAlignedStore(NextIndex, IndexVar);
  return CurIndex;
}

llvm::Value *CGCMRuntime::EmitBuiltinSLMFree(CodeGenFunction &CGF,
                                             const CallExpr *E) {
  // We check whether this call is inside a kernel function.
  if (!CGF.CurFuncDecl->hasAttr<CMGenxMainAttr>()) {
    Error(E->getExprLoc(), "cm_slm_free shall only be called in a kernel");
    return llvm::Constant::getNullValue(CGF.Int32Ty);
  }

  // sanity checking.
  checkSLMSize(*this, E->getExprLoc(), CGF.CurFn);

  llvm::AllocaInst *IndexVar = getOrCreateSLMIndexVar(CGF);
  llvm::Value *CurIndex = CGF.Builder.CreateDefaultAlignedLoad(IndexVar);
  llvm::Value *OffsetInBytes = CGF.EmitAnyExpr(E->getArg(0)).getScalarVal();
  // FIXME: We are not aligning the index. Should it be at least dword-aligned?
  llvm::Value *NextIndex = CGF.Builder.CreateSub(CurIndex, OffsetInBytes);
  CGF.Builder.CreateDefaultAlignedStore(NextIndex, IndexVar);
  return NextIndex;
}

void CGCMRuntime::EmitBuiltinNBarrierInit(CodeGenFunction &CGF,
                                          const CallExpr *E) {
  // We check whether this call is inside a kernel function.
  if (!CGF.CurFuncDecl->hasAttr<CMGenxMainAttr>()) {
    Error(E->getExprLoc(), "cm_nbarrier_init shall only be called in a kernel");
    return;
  }

  const Expr *Arg = E->getArg(0);
  llvm::APSInt Size(8);
  Expr::EvalResult SizeResult;
  if (!Arg->EvaluateAsInt(SizeResult, CGF.getContext())) {
    Error(Arg->getExprLoc(), "integral constant expected for nbarrier count");
    return;
  }
  Size = SizeResult.Val.getInt();

  // Size in bytes being requested.
  uint32_t NewVal = Size.getZExtValue();
  if (NewVal == 0) {
    Error(Arg->getExprLoc(), "zero nbarrier count being requested");
    return;
  }

  // Update named barrier count in kernel metadata.
  if (llvm::MDNode *Node = getSLMSizeMDNode(CGF.CurFn)) {
    if (llvm::Value *OldSz =
            getVal(Node->getOperand(llvm::genx::KernelMDOp::NBarrierCnt))) {
      assert(isa<llvm::ConstantInt>(OldSz) && "integer constant expected");
      llvm::Value *NewSz = llvm::ConstantInt::get(OldSz->getType(), NewVal);
      uint64_t OldVal = cast<llvm::ConstantInt>(OldSz)->getZExtValue();
      if (OldVal < NewVal)
        Node->replaceOperandWith(llvm::genx::KernelMDOp::NBarrierCnt,
                                 getMD(NewSz));
    }
  }
}

RValue CGCMRuntime::EmitCMCallExpr(CodeGenFunction &CGF, const CallExpr *E,
                                   ReturnValueSlot ReturnValue) {

  const FunctionDecl *FD = E->getDirectCallee();
  CGCallee Callee = CGF.EmitCallee(E->getCallee());
  if (FD && FD->hasAttr<CMGenxMainAttr>()) {
    if (FD->hasAttr<CMCallableAttr>()) {
      if (llvm::Function *Fn =
              llvm::dyn_cast<llvm::Function>(Callee.getFunctionPointer())) {
        if (!Fn->hasFnAttribute("CMCallable"))
          Fn->addFnAttr("CMCallable", FD->getName());
      }
    }
  }

  // Cleanup scope for passing CM reference arguments.
  CodeGenFunction::RunCleanupsScope Scope(CGF);

  // We first emit the call normally, and then transform this call into a proper
  // form. For different builtin, may transfrom it differently.
  RValue RV;
  if (Callee.isBuiltin())
    RV = CGF.EmitBuiltinExpr(Callee.getBuiltinDecl(), Callee.getBuiltinID(), E,
                             ReturnValue);
  else
    RV = CGF.EmitCall(E->getCallee()->getType(), Callee, E, ReturnValue);

  if (!FD)
    return RV;

  CMBuiltinKind Kind = getCMBuiltinKind(FD);
  switch (Kind) {
  default:
    break;
  case CMBK_cm_sat_impl:
    return RValue::get(HandleBuiltinSaturateImpl(getCurCMCallInfo(), Kind));
  case CMBK_read:
  case CMBK_write:
  case CMBK_cm_abs:
  case CMBK_cm_add:
  case CMBK_cm_mul:
  case CMBK_cm_div_ieee:
  case CMBK_cm_dp2:
  case CMBK_cm_dp3:
  case CMBK_cm_dp4:
  case CMBK_cm_line:
  case CMBK_cm_max:
  case CMBK_cm_min:
  case CMBK_cm_inv:
  case CMBK_cm_log:
  case CMBK_cm_exp:
  case CMBK_cm_sqrt:
  case CMBK_cm_rsqrt:
  case CMBK_cm_sqrt_ieee:
  case CMBK_cm_pow:
  case CMBK_cm_sin:
  case CMBK_cm_cos:
  case CMBK_cm_sum:
  case CMBK_cm_prod:
  case CMBK_cm_reduced_min:
  case CMBK_cm_reduced_max:
  case CMBK_sample16:
  case CMBK_sample32:
  case CMBK_load16:
  case CMBK_write_atomic:
  case CMBK_write_atomic_typed:
  case CMBK_cm_slm_read:
  case CMBK_cm_slm_write:
  case CMBK_cm_dp4a:
  case CMBK_cm_bfn:
  case CMBK_cm_dpas: // old variant
  case CMBK_cm_dpas2:
    HandleBuiltinInterface(getCurCMCallInfo());
    return RV;
  case CMBK_oword_read_impl:
  case CMBK_oword_read_dwaligned_impl:
    return RValue::get(HandleBuiltinOWordReadImpl(getCurCMCallInfo(), Kind));
  case CMBK_oword_write_impl:
    HandleBuiltinOWordWriteImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_media_read_impl:
    return RValue::get(HandleBuiltinMediaReadImpl(getCurCMCallInfo()));
  case CMBK_media_write_impl:
    HandleBuiltinMediaWriteImpl(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_read_plane:
    return RValue::get(HandleBuiltinMediaReadPlane(getCurCMCallInfo()));
  case CMBK_write_plane:
    HandleBuiltinMediaWritePlane(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_read_untyped:
  case CMBK_write_untyped:
    HandleBuiltinReadWriteUntypedImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_read_typed:
    HandleBuiltinReadTypedImpl(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_write_typed:
    HandleBuiltinWriteTypedImpl(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_scatter_read_impl:
    return RValue::get(HandleBuiltinScatterReadWriteImpl(getCurCMCallInfo()));
  case CMBK_scatter_write_impl:
    HandleBuiltinScatterReadWriteImpl(getCurCMCallInfo(), /*IsWrite*/ true);
    return RValue::get(0);
  case CMBK_cm_svm_read4_impl:
    HandleBuiltinSVMRead4Impl(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_svm_write4_impl:
    HandleBuiltinSVMWrite4Impl(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_svm_atomic:
    HandleBuiltinSVMAtomicOpImpl(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_abs_impl:
    return RValue::get(HandleBuiltinAbsImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_addc_impl:
  case CMBK_cm_subb_impl:
    return RValue::get(HandleBuiltinAddcSubbImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_add_impl:
  case CMBK_cm_mul_impl:
    return RValue::get(HandleBuiltinMulAddImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_shl_impl:
    return RValue::get(HandleBuiltinShlImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_rol_impl:
  case CMBK_cm_ror_impl:
    return RValue::get(HandleBuiltinRolRorImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_sad2_impl:
    return RValue::get(HandleBuiltinSad2Impl(getCurCMCallInfo(), Kind));
  case CMBK_cm_sada2_impl:
    return RValue::get(HandleBuiltinSad2AddImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_lrp_impl:
    return RValue::get(HandleBuiltinLrpImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_pln_impl:
    return RValue::get(HandleBuiltinPlnImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_bfrev_impl:
    return RValue::get(
        HandleBuiltinBitFieldReverseImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_cbit_impl:
    return RValue::get(HandleBuiltinCountBitsImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_bfins_impl:
    return RValue::get(
        HandleBuiltinBitFieldInsertImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_bfext_impl:
    return RValue::get(
        HandleBuiltinBitFieldExtractImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_avg_impl:
    return RValue::get(HandleBuiltinAvgImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_dp2_impl:
  case CMBK_cm_dp3_impl:
  case CMBK_cm_dp4_impl:
  case CMBK_cm_dph_impl:
    return RValue::get(HandleBuiltinDotProductImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_line_impl:
    return RValue::get(HandleBuiltinLineImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_fbl_impl:
  case CMBK_cm_sfbh_impl:
  case CMBK_cm_ufbh_impl:
    return RValue::get(HandleBuiltinFblFbhImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_frc_impl:
    return RValue::get(HandleBuiltinFrcImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_lzd_impl:
    return RValue::get(HandleBuiltinLzdImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_max_impl:
  case CMBK_cm_min_impl:
    return RValue::get(HandleBuiltinMinMaxImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_inv_impl:
  case CMBK_cm_log_impl:
  case CMBK_cm_exp_impl:
  case CMBK_cm_sqrt_impl:
  case CMBK_cm_rsqrt_impl:
  case CMBK_cm_sqrt_ieee_impl:
  case CMBK_cm_div_ieee_impl:
  case CMBK_cm_pow_impl:
  case CMBK_cm_sin_impl:
  case CMBK_cm_cos_impl:
    return RValue::get(HandleBuiltinExtendedMathImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_imad_impl:
    return RValue::get(HandleBuiltinIMadImpl(getCurCMCallInfo()));
  case CMBK_cm_rndd_impl:
  case CMBK_cm_rndu_impl:
  case CMBK_cm_rnde_impl:
  case CMBK_cm_rndz_impl:
    return RValue::get(HandleBuiltinRoundingImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_sum_impl:
  case CMBK_cm_sum_sat_impl:
  case CMBK_cm_prod_impl:
  case CMBK_cm_prod_sat_impl:
  case CMBK_cm_reduced_min_impl:
  case CMBK_cm_reduced_max_impl:
    return RValue::get(HandleBuiltinReductionImpl(getCurCMCallInfo(), Kind));
  case CMBK_sample16_impl:
    return RValue::get(HandleBuiltinSample16Impl(getCurCMCallInfo(), Kind));
  case CMBK_sample32_impl:
    return RValue::get(HandleBuiltinSample32Impl(getCurCMCallInfo(), Kind));
  case CMBK_cm_3d_sample:
  case CMBK_cm_3d_load:
    HandleBuiltin3dOperationImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_load16_impl:
    return RValue::get(HandleBuiltinLoad16Impl(getCurCMCallInfo(), Kind));
  case CMBK_write_atomic_impl:
    return RValue::get(HandleBuiltinWriteAtomicImpl(getCurCMCallInfo(), Kind));
  case CMBK_write_atomic_typed_impl:
    return RValue::get(
        HandleBuiltinWriteAtomicTypedImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_pack_mask:
    return RValue::get(HandleBuiltinPackMaskImpl(getCurCMCallInfo()));
  case CMBK_cm_unpack_mask:
    return RValue::get(HandleBuiltinUnPackMaskImpl(getCurCMCallInfo()));
  case CMBK_cm_send:
    return RValue::get(HandleBuiltinSendImpl(getCurCMCallInfo()));
  case CMBK_cm_sends:
    return RValue::get(HandleBuiltinSendsImpl(getCurCMCallInfo()));
  case CMBK_cm_raw_send:
    return RValue::get(HandleBuiltinRawSendImpl(getCurCMCallInfo()));
  case CMBK_cm_get_r0:
    return RValue::get(HandleBuiltinGetR0Impl(getCurCMCallInfo()));
  case CMBK_cm_get_sr0:
    return RValue::get(HandleBuiltinGetSR0Impl(getCurCMCallInfo()));
  case CMBK_cm_unmask_begin:
    return RValue::get(HandleBuiltinUnmaskBeginImpl(getCurCMCallInfo()));
  case CMBK_cm_unmask_end:
    return RValue::get(HandleBuiltinUnmaskEndImpl(getCurCMCallInfo()));
  case CMBK_cm_get_value:
    return RValue::get(HandleBuiltinGetValueImpl(getCurCMCallInfo()));
  case CMBK_cm_slm_read4:
    HandleBuiltinSLMRead4(getCurCMCallInfo(), true);
    return RValue::get(0);
  case CMBK_cm_slm_read4_scaled:
    HandleBuiltinSLMRead4(getCurCMCallInfo(), false);
    return RValue::get(0);
  case CMBK_cm_slm_write4:
    HandleBuiltinSLMWrite4(getCurCMCallInfo(), true);
    return RValue::get(0);
  case CMBK_cm_slm_write4_scaled:
    HandleBuiltinSLMWrite4(getCurCMCallInfo(), false);
    return RValue::get(0);
  case CMBK_cm_slm_atomic:
    HandleBuiltinSLMAtomic(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_avs_sampler:
    HandleBuiltinAVSSampler(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_va_2d_convolve:
  case CMBK_cm_va_2d_convolve_hdc:
    HandleBuiltinVA2dConvolve(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_va_erode:
  case CMBK_cm_va_dilate:
    HandleBuiltinVAErodeDilate(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_va_erode_hdc:
  case CMBK_cm_va_dilate_hdc:
    HandleBuiltinVAErodeDilateHdc(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_va_min_max:
    HandleBuiltinVAMinMax(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_va_min_max_filter:
    HandleBuiltinVAMinMaxFilter(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_va_min_max_filter_hdc:
    HandleBuiltinVAMinMaxFilterHdc(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_va_boolean_centroid:
  case CMBK_cm_va_centroid:
    HandleBuiltinVACentroid(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_va_1d_convolution:
  case CMBK_cm_va_1d_convolution_hdc:
    HandleBuiltinVA1dConvolution(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_va_1pixel_convolve:
    HandleBuiltinVA1PixelConvolve(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_va_1pixel_convolve_hdc:
    HandleBuiltinVA1PixelConvolveHdc(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_va_lbp_creation:
  case CMBK_cm_va_lbp_creation_hdc:
    HandleBuiltinVALbpCreation(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_va_lbp_correlation:
  case CMBK_cm_va_lbp_correlation_hdc:
    HandleBuiltinVALbpCorrelation(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_va_correlation_search:
    HandleBuiltinVACorrelationSearch(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_va_flood_fill:
    HandleBuiltinVAFloodFill(getCurCMCallInfo());
    return RValue::get(0);
  case CMBK_cm_rdtsc:
    return RValue::get(HandleBuiltinRDTSC(getCurCMCallInfo()));
  case CMBK_simdcf_any_impl:
    return RValue::get(HandleBuiltinSimdcfAnyImpl(getCurCMCallInfo()));
  case CMBK_simdcf_predgen_impl:
    return RValue::get(
        HandleBuiltinSimdcfGenericPredicationImpl(getCurCMCallInfo()));
  case CMBK_simdcf_predmin_impl:
  case CMBK_simdcf_predmax_impl:
    return RValue::get(
        HandleBuiltinSimdcfMinMaxPredicationImpl(getCurCMCallInfo(), Kind));
  case CMBK_predefined_surface:
    return RValue::get(HandlePredefinedSurface(getCurCMCallInfo()));
  case CMBK_cm_svm_atomic_impl:
    return RValue::get(HandleBuiltinSVMAtomicImpl(getCurCMCallInfo()));
  case CMBK_rdregion:
    return RValue::get(HandleBuiltinRdregionImpl(getCurCMCallInfo()));
  case CMBK_wrregion:
    return RValue::get(HandleBuiltinWrregionImpl(getCurCMCallInfo()));
  case CMBK_cm_dp4a_impl:
    return RValue::get(HandleBuiltinDP4AImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_bfn_impl:
    return RValue::get(HandleBuiltinBFNImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_dpas_impl:
  case CMBK_cm_dpas_nosrc0_impl:
  case CMBK_cm_dpasw_impl:
  case CMBK_cm_dpasw_nosrc0_impl:
    return RValue::get(HandleBuiltinDPASImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_dpas2_impl:
    return RValue::get(HandleBuiltinDPAS2Impl(getCurCMCallInfo(), Kind));
  case CMBK_cm_tf32_cvt_impl:
    return RValue::get(HandleBuiltinTF32CVTImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_srnd_impl:
    return RValue::get(HandleBuiltinSRNDImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_load_impl:
  case CMBK_cm_load4_impl:
  case CMBK_cm_block_load_impl:
  case CMBK_cm_load_flat_impl:
  case CMBK_cm_load4_flat_impl:
  case CMBK_cm_block_load_flat_impl:
  case CMBK_cm_load_slm_impl:
  case CMBK_cm_load4_slm_impl:
  case CMBK_cm_atomic_bti_impl:
  case CMBK_cm_atomic_slm_impl:
  case CMBK_cm_atomic_flat_impl:
    return RValue::get(HandleBuiltinLSCImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_store_impl:
  case CMBK_cm_store4_impl:
  case CMBK_cm_block_store_impl:
  case CMBK_cm_store_flat_impl:
  case CMBK_cm_store4_flat_impl:
  case CMBK_cm_block_store_flat_impl:
  case CMBK_cm_prefetch_impl:
  case CMBK_cm_block_prefetch_impl:
  case CMBK_cm_prefetch_flat_impl:
  case CMBK_cm_block_prefetch_flat_impl:
  case CMBK_cm_store_slm_impl:
  case CMBK_cm_store4_slm_impl:
    HandleBuiltinLSCImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_block_load2d_flat_impl:
    return RValue::get(HandleBuiltinLSC2dImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_block_store2d_flat_impl:
  case CMBK_cm_block_prefetch2d_flat_impl:
    HandleBuiltinLSC2dImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_load4_typed_impl:
    return RValue::get(HandleBuiltinLSCTypedImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_store4_typed_impl:
  case CMBK_cm_prefetch4_typed_impl:
    HandleBuiltinLSCTypedImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_load2d_bti_impl:
    return RValue::get(HandleBuiltinLSCTyped2DImpl(getCurCMCallInfo(), Kind));
  case CMBK_cm_store2d_bti_impl:
  case CMBK_cm_prefetch2d_bti_impl:
    HandleBuiltinLSCTyped2DImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_cm_lsc_fence_impl:
    HandleBuiltinLscFenceImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_scatter_impl:
  case CMBK_scatter_svm_impl:
    HandleBuiltinScatterImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_gather_impl:
  case CMBK_gather_svm_impl:
    return RValue::get(HandleBuiltinGatherImpl(getCurCMCallInfo(), Kind));
  case CMBK_store_impl:
    HandleBuiltinStoreImpl(getCurCMCallInfo(), Kind);
    return RValue::get(0);
  case CMBK_load_impl:
    return RValue::get(HandleBuiltinLoadImpl(getCurCMCallInfo(), Kind));
  }

  // Returns the normal call rvalue.
  return RV;
}

void CGCMRuntime::Error(SourceLocation Loc, StringRef Msg) {
  CGM.Error(Loc, Msg);
}

llvm::Function *CGCMRuntime::getGenXIntrinsic(unsigned ID,
                                              ArrayRef<llvm::Type *> Tys) {
  return CGM.getGenXIntrinsic(ID, Tys);
}

/// This is calling the CM builtin template implemented in the header file.
void CGCMRuntime::HandleBuiltinInterface(CMCallInfo &CallInfo) {
  llvm::Function *CalledFn = CallInfo.CI->getCalledFunction();
  CalledFn->addFnAttr(llvm::Attribute::AlwaysInline);
  CalledFn->setLinkage(llvm::Function::InternalLinkage);
}

static bool getSatIntrinsicID(unsigned &ID, QualType DstType,
                              QualType SrcType) {
  assert(!DstType->isFloatingType() && "not expected saturation case");

  DstType = DstType->getCanonicalTypeUnqualified();
  SrcType = SrcType->getCanonicalTypeUnqualified();

  // float->int etc.
  if (DstType->isIntegerType() && SrcType->isFloatingType()) {
    ID = DstType->isUnsignedIntegerType()
             ? llvm::GenXIntrinsic::genx_fptoui_sat
             : llvm::GenXIntrinsic::genx_fptosi_sat;
    return true;
  }

  // any int->any int (including same types)
  if (DstType->isIntegerType() && SrcType->isIntegerType()) {
    if (DstType->isUnsignedIntegerType())
      ID = SrcType->isUnsignedIntegerType()
               ? llvm::GenXIntrinsic::genx_uutrunc_sat
               : llvm::GenXIntrinsic::genx_ustrunc_sat;
    else
      ID = SrcType->isUnsignedIntegerType()
               ? llvm::GenXIntrinsic::genx_sutrunc_sat
               : llvm::GenXIntrinsic::genx_sstrunc_sat;
    return true;
  }

  llvm_unreachable("invalid conversion");
}

// This function implements the following saturation templates
//
// template <typename T0, typename T1>
// vector<T0, N> saturate(vector<T1, N> src);
//
// template <typename T0, typename T1>
// T0 saturate(T1 src);
//
// For different T0 and T1, a proper genx intrinsic is used for saturation.
llvm::Value *CGCMRuntime::HandleBuiltinSaturateImpl(CMCallInfo &CallInfo,
                                                    CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_sat_impl);
  const CallExpr *CE = CallInfo.CE;
  QualType ToType = CE->getType();
  assert(CE->getNumArgs() == 1 && "wrong number of arguments");
  QualType FromType = CE->getArg(0)->getType();

  if (FromType->isCMVectorMatrixType()) {
    assert(ToType->isCMVectorMatrixType());
    ToType = ToType->getCMVectorMatrixElementType();
    FromType = FromType->getCMVectorMatrixElementType();
  }

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg = CI->getArgOperand(0);
  llvm::Value *Result = 0;

  // When the destination is of floating point type and the source operand has a
  // different type, then do saturation in two steps. E.g.
  //
  // f32Val = sat(f64Val);
  //
  // will be emitted as,
  //
  // f32temp = fptrunc f64Val
  // f32Val = genx.sat f32temp
  //
  unsigned ID = llvm::GenXIntrinsic::not_genx_intrinsic;
  CGBuilderTy &Builder = CallInfo.CGF->Builder;
  if (ToType->isFloatingType()) {
    llvm::Function *Fn =
        getGenXIntrinsic(llvm::GenXIntrinsic::genx_sat, CI->getType());

    llvm::Instruction::CastOps OpKind;
    if (getCastOpKind(OpKind, *CallInfo.CGF, ToType, FromType)) {
      Arg = Builder.CreateCast(OpKind, Arg, CI->getType(), CI->getName());
      cast<llvm::Instruction>(Arg)->setDebugLoc(CI->getDebugLoc());
    }

    llvm::CallInst *NewCI = Builder.CreateCall(Fn, Arg, CI->getName());
    NewCI->setDebugLoc(CI->getDebugLoc());
    Result = NewCI;
  } else if (getSatIntrinsicID(ID, ToType, FromType)) {
    llvm::Type *Tys[2] = {CI->getType(), Arg->getType()};
    llvm::Function *Fn = getGenXIntrinsic(ID, Tys);
    llvm::CallInst *NewCI = Builder.CreateCall(Fn, Arg, CI->getName());
    NewCI->setDebugLoc(CI->getDebugLoc());
    Result = NewCI;
  } else {
    // no saturation is needed. Just replace this satuaration with a noop inst.
    Result = Builder.CreateBitCast(Arg, Arg->getType());
  }

  CI->eraseFromParent();
  return Result;
}

/// This is calling the CM builtin template declaration used in the header
/// file. We replace this call by calling corresponding genx intrinsics.
llvm::Value *CGCMRuntime::HandleBuiltinDotProductImpl(CMCallInfo &CallInfo,
                                                      CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_dp2_impl || Kind == CMBK_cm_dp3_impl ||
         Kind == CMBK_cm_dp4_impl || Kind == CMBK_cm_dph_impl);
  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind);

  // Overload with its return type.
  llvm::Function *CalledFn = CallInfo.CI->getCalledFunction();
  llvm::Type *RetTy = CalledFn->getReturnType();
  assert(RetTy->isFPOrFPVectorTy());

  llvm::Function *GenxFn = getGenXIntrinsic(ID, RetTy);

  assert(CallInfo.CI->getNumArgOperands() == 2);
  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Args[2] = {CI->getArgOperand(0), CI->getArgOperand(1)};

  CGBuilderTy Builder(*CallInfo.CGF, CI);
  llvm::CallInst *Result = Builder.CreateCall(GenxFn, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

llvm::Value *CGCMRuntime::HandleBuiltinLineImpl(CMCallInfo &CallInfo,
                                                CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_line_impl);

  // Overload with its return type.
  llvm::Function *CalledFn = CallInfo.CI->getCalledFunction();
  llvm::Type *RetTy = CalledFn->getReturnType();
  assert(RetTy->isFPOrFPVectorTy());

  llvm::Function *GenxFn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_line, RetTy);

  assert(CallInfo.CI->getNumArgOperands() == 2);
  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Args[2] = {CI->getArgOperand(0), CI->getArgOperand(1)};

  CGBuilderTy Builder(*CallInfo.CGF, CI);
  llvm::CallInst *NewCI = Builder.CreateCall(GenxFn, Args, CI->getName());
  NewCI->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess builtin cm_intrinsic_impl_fbl and cm_intrinsic_impl_fbh.
///
/// template <unsigned int SZ>
/// vector<unsigned , SZ>
/// __cm_intrinsic_impl_fbl(vector<unsigned int, SZ> src0);
///
/// template <int SZ>
/// vector<int , SZ>
/// __cm_intrinsic_impl_sfbh(vector<int, SZ> src0);
///
/// template <unsigned int SZ>
/// vector<unsigned , SZ>
/// __cm_intrinsic_impl_ufbh(vector<unsigned int, SZ> src0);
///
llvm::Value *CGCMRuntime::HandleBuiltinFblFbhImpl(CMCallInfo &CallInfo,
                                                  CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_fbl_impl || Kind == CMBK_cm_sfbh_impl ||
         Kind == CMBK_cm_ufbh_impl);
  assert(CallInfo.CE->getType()->isCMVectorMatrixType());
  assert(CallInfo.CE->getNumArgs() == 1);

  unsigned ID = 0;
  switch (Kind) {
  default:
    llvm_unreachable("unexpected kind for HandleBuiltinFblFbhImpl");
  case CMBK_cm_fbl_impl:
    ID = llvm::GenXIntrinsic::genx_fbl;
    break;
  case CMBK_cm_sfbh_impl:
    ID = llvm::GenXIntrinsic::genx_sfbh;
    break;
  case CMBK_cm_ufbh_impl:
    ID = llvm::GenXIntrinsic::genx_ufbh;
    break;
  }

  llvm::CallInst *CI = CallInfo.CI;

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  llvm::Function *F = getGenXIntrinsic(ID, CI->getType());
  llvm::CallInst *Result =
      Builder.CreateCall(F, CI->getArgOperand(0), CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_intrinsic_impl_frc.
///
/// template <int SZ>
/// vector<float, SZ>
/// __cm_intrinsic_impl_frc(vector<float, SZ> src0);
///
llvm::Value *CGCMRuntime::HandleBuiltinFrcImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_frc_impl);
  assert(CallInfo.CE->getType()->isCMVectorMatrixType());
  assert(CallInfo.CE->getNumArgs() == 1);

  llvm::CallInst *CI = CallInfo.CI;

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  llvm::Function *F =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_frc, CI->getType());
  llvm::CallInst *Result =
      Builder.CreateCall(F, CI->getArgOperand(0), CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_intrinsic_impl_lzd.
///
/// template <typename T, int SZ>
/// vector<T, SZ>
/// __cm_intrinsic_impl_lzd(vector<T, SZ> src0);
///
llvm::Value *CGCMRuntime::HandleBuiltinLzdImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_lzd_impl);
  assert(CallInfo.CE->getType()->isCMVectorMatrixType());
  assert(CallInfo.CE->getNumArgs() == 1);

  llvm::CallInst *CI = CallInfo.CI;

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  llvm::Type *Tys[] = {CI->getType()};

  llvm::Function *F = getGenXIntrinsic(llvm::GenXIntrinsic::genx_lzd, Tys);
  llvm::CallInst *Result =
      Builder.CreateCall(F, CI->getArgOperand(0), CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

llvm::Value *CGCMRuntime::HandleBuiltinMinMaxImpl(CMCallInfo &CallInfo,
                                                  CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_max_impl || Kind == CMBK_cm_min_impl);
  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind);

  // Overload with its return type and src0's type.
  llvm::Function *CalledFn = CallInfo.CI->getCalledFunction();
  llvm::Type *RetTy = CalledFn->getReturnType();

  llvm::Type *Tys[2] = {RetTy, CalledFn->getFunctionType()->getParamType(0)};
  llvm::Function *GenxFn = getGenXIntrinsic(ID, Tys);

  assert(CallInfo.CI->getNumArgOperands() == 2);
  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Args[2] = {CI->getArgOperand(0), CI->getArgOperand(1)};

  CGBuilderTy &Builder = CallInfo.CGF->Builder;
  llvm::CallInst *NewCI = Builder.CreateCall(GenxFn, Args, CI->getName());
  NewCI->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

///
/// template <typename T, int SZ>
/// vector<T, SZ> __cm_intrinsic_impl_<name>(vector<T, SZ> src0);
///
static llvm::Value *EmitBuiltinCommonOneArg(CGCMRuntime &CMRT, unsigned ID,
                                            CGCMRuntime::CMCallInfo &CallInfo) {

  // Overload with its return type.
  llvm::Function *CalledFn = CallInfo.CI->getCalledFunction();
  llvm::Type *RetTy = CalledFn->getReturnType();
  llvm::Function *GenxFn = CMRT.getGenXIntrinsic(ID, RetTy);

  assert(CallInfo.CI->getNumArgOperands() == 1);
  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg = CI->getArgOperand(0);

  CGBuilderTy Builder(*CallInfo.CGF, CI);
  llvm::CallInst *NewCI = Builder.CreateCall(GenxFn, Arg, CI->getName());
  NewCI->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

///
/// template <typename T, int SZ>
/// vector<T, SZ>
/// __cm_intrinsic_impl_<name>(vector<T, SZ> src0, vector<T, SZ> src1);
///
static llvm::Value *EmitBuiltinCommonTwoArg(CGCMRuntime &CMRT, unsigned ID,
                                            CGCMRuntime::CMCallInfo &CallInfo) {

  // Overload with its return type.
  llvm::Function *CalledFn = CallInfo.CI->getCalledFunction();
  llvm::Type *RetTy = CalledFn->getReturnType();
  llvm::Function *Fn = CMRT.getGenXIntrinsic(ID, RetTy);

  assert(CallInfo.CI->getNumArgOperands() == 2);
  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Args[] = {CI->getArgOperand(0), CI->getArgOperand(1)};

  CGBuilderTy Builder(*CallInfo.CGF, CI);
  llvm::CallInst *NewCI = Builder.CreateCall(Fn, Args, CI->getName());
  NewCI->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess builtin cm_inv, cm_log, etc.
llvm::Value *CGCMRuntime::HandleBuiltinExtendedMathImpl(CMCallInfo &CallInfo,
                                                        CMBuiltinKind Kind) {
  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind);
  if (Kind == CMBK_cm_pow_impl || Kind == CMBK_cm_div_ieee_impl)
    return EmitBuiltinCommonTwoArg(*this, ID, CallInfo);
  return EmitBuiltinCommonOneArg(*this, ID, CallInfo);
}

/// \brief Postprocess builtin cm_abs.
///
/// template <typename T, int SZ>
/// vector<T, SZ>
/// __cm_intrinsic_impl_abs(vector<T, SZ> src0);
///
llvm::Value *CGCMRuntime::HandleBuiltinAbsImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_abs_impl);
  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind);
  return EmitBuiltinCommonOneArg(*this, ID, CallInfo);
}

/// \brief Postprocess builtin cm_mul and cm_add.
///
/// template <typename T0, typename T1, int SZ>
/// vector<T0, SZ>
/// __cm_intrinsic_impl_add(vector<T1, SZ> src0, vector<T1, SZ> src1, int flag);
///
/// template <typename T0, typename T1, int SZ>
/// vector<T0, SZ>
/// __cm_intrinsic_impl_mul(vector<T1, SZ> src0, vector<T1, SZ> src1, int flag);
///
llvm::Value *CGCMRuntime::HandleBuiltinMulAddImpl(CMCallInfo &CallInfo,
                                                  CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_add_impl || Kind == CMBK_cm_mul_impl);

  QualType T0 = CallInfo.CE->getType();
  assert(T0->isCMVectorMatrixType());
  T0 = T0->getCMVectorMatrixElementType();

  assert(CallInfo.CE->getNumArgs() == 3);
  QualType T1 = CallInfo.CE->getArg(0)->getType();
  assert(T1->isCMVectorMatrixType());
  T1 = T1->getCMVectorMatrixElementType();

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);
  llvm::Value *Flag = CI->getArgOperand(2);

  CGBuilderTy Builder(*CallInfo.CGF, CI);
  llvm::Value *Result = 0;
  llvm::Function *SatFn = 0;

  // Check the saturation flag, compare it with 0.
  llvm::Value *CMP = Builder.CreateICmpEQ(
      Flag, llvm::Constant::getNullValue(Flag->getType()), "cmp");

  // Compute the cast kind from T1 to T0
  llvm::Instruction::CastOps CastOp;
  bool NeedsConv = getCastOpKind(CastOp, *CallInfo.CGF, T0, T1);

  if (T1->isFloatingType()) {
    // No genx intrinsic needed. For example
    //
    // cm_add<float>(float, float, SAT)   => genx.sat(fadd(arg0, arg1))
    // cm_add<float>(double, double, SAT) => genx.sat(fptrunc fadd(arg0, arg1))
    // cm_add<double>(float, float, SAT)  => genx.sat(fpext fadd(arg0, arg1))
    // cm_add<int>(float, float, SAT)     => genx.fptosi.sat(fadd(arg0, arg1))
    if (Kind == CMBK_cm_add_impl)
      Result = Builder.CreateFAdd(Arg0, Arg1, CI->getName());
    else
      Result = Builder.CreateFMul(Arg0, Arg1, CI->getName());
    cast<llvm::Instruction>(Result)->setDebugLoc(CI->getDebugLoc());

    if (T0->isFloatingType()) {
      if (NeedsConv)
        Result = Builder.CreateCast(CastOp, Result, CI->getType(), "conv");

      SatFn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_sat, CI->getType());
      llvm::Instruction *SatVal = Builder.CreateCall(SatFn, Result, "sat");
      Result = Builder.CreateSelect(CMP, Result, SatVal);
    } else {
      assert(T0->isIntegerType());
      // This is a subtle case, we need to introduce a temporary, since for
      // float to int, there are conversion saturations. This is different
      // from the above float case, where there is no conversion saturation.
      assert(NeedsConv);
      llvm::Value *NonSatVal =
          Builder.CreateCast(CastOp, Result, CI->getType(), "conv");

      unsigned ID = llvm::GenXIntrinsic::not_genx_intrinsic;
      getSatIntrinsicID(ID, T0, T1);
      assert(ID != llvm::GenXIntrinsic::not_genx_intrinsic);
      llvm::Type *Tys[] = {CI->getType(), Arg0->getType()};
      SatFn = getGenXIntrinsic(ID, Tys);
      llvm::Instruction *SatVal = Builder.CreateCall(SatFn, Result, "sat");
      Result = Builder.CreateSelect(CMP, NonSatVal, SatVal);
    }
  } else {
    // Src type is integer.
    assert(T1->isIntegerType());

    // saturated ID
    unsigned ID1 = GetGenxIntrinsicID(CallInfo, Kind, true);

    if (T0->isFloatingType()) {
      // cm_add<float>(int, int, SAT) => genx.sat(sitofp(genx.ssadd.sat(int,
      // int))
      llvm::Type *Tys[] = {Arg0->getType(), Arg0->getType()};

      // The non-saturating case does not need an intrinsic. Just use a normal
      // add/mul.
      llvm::Value *R0 = nullptr;
      if (Kind == CMBK_cm_add_impl)
        R0 = Builder.CreateAdd(Arg0, Arg1, CI->getName());
      else
        R0 = Builder.CreateMul(Arg0, Arg1, CI->getName());
      cast<llvm::Instruction>(R0)->setDebugLoc(CI->getDebugLoc());
      // We definitely need an int-to-fp conversin here.
      assert(NeedsConv);
      llvm::Value *Conv0 =
          Builder.CreateCast(CastOp, R0, CI->getType(), "conv");

      llvm::Value *Args[] = {Arg0, Arg1};
      llvm::Function *F1 = getGenXIntrinsic(ID1, Tys);
      llvm::CallInst *R1 = Builder.CreateCall(F1, Args, CI->getName());
      R1->setDebugLoc(CI->getDebugLoc());
      llvm::Value *Conv1 =
          Builder.CreateCast(CastOp, R1, CI->getType(), "conv");

      SatFn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_sat, CI->getType());
      llvm::Instruction *SatVal = Builder.CreateCall(SatFn, Conv1, "sat");

      // The final result is Conv0 or SatVal.
      Result = Builder.CreateSelect(CMP, Conv0, SatVal);
    } else {
      // cm_add<int>(int, int, SAT) => genx.ssadd.sat(int, int)
      assert(T0->isIntegerType());
      // The non-saturating case does not need an intrinsic. Just promote/demote
      // the args to the destination type and use a normal add/mul.
      auto ConvertedArg0 =
          Builder.CreateCast(CastOp, Arg0, CI->getType(), "conv");
      cast<llvm::Instruction>(ConvertedArg0)->setDebugLoc(CI->getDebugLoc());
      auto ConvertedArg1 =
          Builder.CreateCast(CastOp, Arg1, CI->getType(), "conv");
      cast<llvm::Instruction>(ConvertedArg1)->setDebugLoc(CI->getDebugLoc());
      llvm::Value *R0 = nullptr;
      if (Kind == CMBK_cm_add_impl)
        R0 = Builder.CreateAdd(ConvertedArg0, ConvertedArg1, CI->getName());
      else
        R0 = Builder.CreateMul(ConvertedArg0, ConvertedArg1, CI->getName());
      cast<llvm::Instruction>(R0)->setDebugLoc(CI->getDebugLoc());

      llvm::Type *Tys[] = {CI->getType(), Arg0->getType()};
      llvm::Value *Args[] = {Arg0, Arg1};
      llvm::Function *F1 = getGenXIntrinsic(ID1, Tys);
      llvm::CallInst *R1 = Builder.CreateCall(F1, Args, CI->getName());
      R1->setDebugLoc(CI->getDebugLoc());

      // The final result depends on the saturation flag.
      Result = Builder.CreateSelect(CMP, R0, R1);
    }
  }

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_addc and cm_subb.
///
/// template <int SZ>
/// __cm_intrinsic_impl_addc(vector<unsigned, SZ> src0,
///                          vector<unsigned, SZ> src1,
///                          vector_ref<unsigned, SZ> carry);
///
/// template <int SZ>
/// __cm_intrinsic_impl_subb(vector<unsigned, SZ> src0,
///                          vector<unsigned, SZ> src1,
///                          vector_ref<unsigned, SZ> carry);
///
llvm::Value *CGCMRuntime::HandleBuiltinAddcSubbImpl(CMCallInfo &CallInfo,
                                                    CMBuiltinKind Kind) {
  assert((Kind == CMBK_cm_addc_impl || Kind == CMBK_cm_subb_impl) &&
         "addc or subb builtin is expected");
  assert(CallInfo.CE->getNumArgs() == 3 &&
         "addc and subb must have 3 arguments");

  QualType VecType = CallInfo.CE->getType();
  assert(VecType->isCMIntegerVectorMatrixType() &&
         "addc and subb are defined for only integer types");

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Type *Ty = CI->getType();
  assert(Ty->isVectorTy() && Ty->isIntOrIntVectorTy(32) &&
         "addc and subb support only 32bit integers");

  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);
  llvm::Value *Carry = CI->getArgOperand(2);

  CGBuilderTy Builder{*CallInfo.CGF, CI};

  unsigned ID = Kind == CMBK_cm_addc_impl ? llvm::GenXIntrinsic::genx_addc
                                          : llvm::GenXIntrinsic::genx_subb;
  llvm::Type *Tys[] = {Arg0->getType(), Arg0->getType()};
  llvm::Value *Args[] = {Arg0, Arg1};
  llvm::Function *F = getGenXIntrinsic(ID, Tys);
  llvm::Value *Result = Builder.CreateCall(F, Args, CI->getName());

  llvm::Value *CarryValue = Builder.CreateExtractValue(Result, 0);
  Builder.CreateDefaultAlignedStore(CarryValue, Carry);
  llvm::Value *R = Builder.CreateExtractValue(Result, 1);

  CI->eraseFromParent();
  return R;
}

/// \brief Postprocess builtin cm_avg.
///
/// template <typename T0, typename T1, int SZ>
/// vector<T0, SZ>
/// __cm_intrinsic_impl_avg(vector<T1, SZ> src0, vector<T1, SZ> src1, int flag);
///
llvm::Value *CGCMRuntime::HandleBuiltinAvgImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_avg_impl);

  QualType T0 = CallInfo.CE->getType();
  assert(T0->isCMVectorMatrixType());
  T0 = T0->getCMVectorMatrixElementType();

  assert(CallInfo.CE->getNumArgs() == 3);
  QualType T1 = CallInfo.CE->getArg(0)->getType();
  assert(T1->isCMVectorMatrixType());
  T1 = T1->getCMVectorMatrixElementType();

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);
  llvm::Value *Flag = CI->getArgOperand(2);

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // Check the saturation flag, compare it with 0.
  llvm::Value *CMP = Builder.CreateICmpEQ(
      Flag, llvm::Constant::getNullValue(Flag->getType()), "cmp");

  // non-saturated ID
  unsigned ID0 = GetGenxIntrinsicID(CallInfo, Kind, false);

  // saturated ID
  unsigned ID1 = GetGenxIntrinsicID(CallInfo, Kind, true);

  // cm_avg<int>(int, int, SAT) => genx.ssavg.sat(int, int)
  llvm::Type *Tys[] = {CI->getType(), Arg0->getType()};
  llvm::Value *Args[] = {Arg0, Arg1};

  llvm::Function *F0 = getGenXIntrinsic(ID0, Tys);
  llvm::CallInst *R0 = Builder.CreateCall(F0, Args, CI->getName());
  R0->setDebugLoc(CI->getDebugLoc());

  llvm::Function *F1 = getGenXIntrinsic(ID1, Tys);
  llvm::CallInst *R1 = Builder.CreateCall(F1, Args, CI->getName());
  R1->setDebugLoc(CI->getDebugLoc());

  // The final result depends on the saturation flag.
  llvm::Value *Result = Builder.CreateSelect(CMP, R0, R1);

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_imad.
///
/// template <typename T, int SZ>
/// vector<T, SZ> __cm_intrinsic_impl_imad(vector_ref<T, SZ> lo,
/// vector<T, SZ> src0, vector<T, SZ> src1, vector<T, SZ> src2);
///
/// template <typename T>
/// T __cm_intrinsic_impl_imad(T& lo, T src0, T src1, T src2);
///
llvm::Value *CGCMRuntime::HandleBuiltinIMadImpl(CMCallInfo &CallInfo) {
  assert(CallInfo.CE->getNumArgs() == 4 && "builtin must have 4 arguments");
  QualType QTy = CallInfo.CE->getType();
  llvm::CallInst *CI = CallInfo.CI;
  llvm::Type *Ty = CI->getType();
  assert(Ty->isIntOrIntVectorTy(32) && "imad support only 32bit integers");

  llvm::Value *LoPtr = CI->getArgOperand(0);
  llvm::Value *Src0 = CI->getArgOperand(1);
  llvm::Value *Src1 = CI->getArgOperand(2);
  llvm::Value *Src2 = CI->getArgOperand(3);
  assert(LoPtr->getType()->isPointerTy());

  CGBuilderTy Builder{*CallInfo.CGF, CI};

  clang::QualType ElType =
      QTy->isCMVectorMatrixType() ? QTy->getCMVectorMatrixElementType() : QTy;
  unsigned ID = ElType->isUnsignedIntegerType()
                    ? llvm::GenXIntrinsic::genx_uimad
                    : llvm::GenXIntrinsic::genx_simad;
  llvm::Type *Tys[] = {Src0->getType(), Src0->getType()};
  llvm::Value *Args[] = {Src0, Src1, Src2};
  llvm::Function *IMadFunc = getGenXIntrinsic(ID, Tys);
  llvm::Value *IMad = Builder.CreateCall(IMadFunc, Args, CI->getName());

  llvm::Value *LoValue = Builder.CreateExtractValue(IMad, 1);
  llvm::Value *HiValue = Builder.CreateExtractValue(IMad, 0);

  Builder.CreateDefaultAlignedStore(LoValue, LoPtr);
  CI->eraseFromParent();
  return HiValue;
}

/// \brief Postprocess builtin cm_shl.
///
/// template <typename T0, typename T1, int SZ>
/// vector<T0, SZ>
/// __cm_intrinsic_impl_shl(vector<T1, SZ> src0, vector<T1, SZ> src1, int flag);
///
llvm::Value *CGCMRuntime::HandleBuiltinShlImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_shl_impl);

  QualType T0 = CallInfo.CE->getType();
  assert(T0->isCMVectorMatrixType());
  T0 = T0->getCMVectorMatrixElementType();

  assert(CallInfo.CE->getNumArgs() == 3);
  QualType T1 = CallInfo.CE->getArg(0)->getType();
  assert(T1->isCMVectorMatrixType());
  T1 = T1->getCMVectorMatrixElementType();

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);
  llvm::Value *Flag = CI->getArgOperand(2);

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // Check the saturation flag, compare it with 0.
  llvm::Value *CMP = Builder.CreateICmpEQ(
      Flag, llvm::Constant::getNullValue(Flag->getType()), "cmp");

  // non-saturated ID
  unsigned ID0 = GetGenxIntrinsicID(CallInfo, Kind, false);

  // saturated ID
  unsigned ID1 = GetGenxIntrinsicID(CallInfo, Kind, true);

  // cm_shl<int>(int, int, SAT) => genx.ssshl.sat(int, int)
  llvm::Type *Tys[] = {CI->getType(), Arg0->getType()};
  llvm::Value *Args[] = {Arg0, Arg1};

  llvm::Function *F0 = getGenXIntrinsic(ID0, Tys);
  llvm::CallInst *R0 = Builder.CreateCall(F0, Args, CI->getName());
  R0->setDebugLoc(CI->getDebugLoc());

  llvm::Function *F1 = getGenXIntrinsic(ID1, Tys);
  llvm::CallInst *R1 = Builder.CreateCall(F1, Args, CI->getName());
  R1->setDebugLoc(CI->getDebugLoc());

  // The final result depends on the saturation flag.
  llvm::Value *Result = Builder.CreateSelect(CMP, R0, R1);

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_rol and cm_ror.
///
/// template <typename T0, typename T1, int SZ>
/// vector<T0, SZ>
/// __cm_intrinsic_impl_rol(vector<T1, SZ> src0, vector<T1, SZ> src1);
///
/// template <typename T0, typename T1, int SZ>
/// vector<T0, SZ>
/// __cm_intrinsic_impl_ror(vector<T1, SZ> src0, vector<T1, SZ> src1);
///
llvm::Value *CGCMRuntime::HandleBuiltinRolRorImpl(CMCallInfo &CallInfo,
                                                  CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_rol_impl || Kind == CMBK_cm_ror_impl);

  assert(CallInfo.CE->getType()->isCMVectorMatrixType()); // T0

  assert(CallInfo.CE->getNumArgs() == 2);
  assert(CallInfo.CE->getArg(0)->getType()->isCMVectorMatrixType()); // T1

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind, false);

  // cm_rol<int>(int, int) => genx.rol(int, int)
  // cm_ror<int>(int, int) => genx.ror(int, int)
  llvm::Type *Tys[] = {CI->getType(), Arg0->getType()};
  llvm::Value *Args[] = {Arg0, Arg1};

  llvm::Function *F = getGenXIntrinsic(ID, Tys);
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_sad2.
///
/// template <typename T0, typename T1, int SZ>
/// vector<T0, SZ>
/// __cm_intrinsic_impl_sad2(vector<T1, SZ> src0, vector<T1, SZ> src1);
///
llvm::Value *CGCMRuntime::HandleBuiltinSad2Impl(CMCallInfo &CallInfo,
                                                CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_sad2_impl);

  QualType T0 = CallInfo.CE->getType();
  assert(T0->isCMVectorMatrixType());
  T0 = T0->getCMVectorMatrixElementType();

  assert(CallInfo.CE->getNumArgs() == 2);
  QualType T1 = CallInfo.CE->getArg(0)->getType();
  assert(T1->isCMVectorMatrixType());
  T1 = T1->getCMVectorMatrixElementType();

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind, false);

  // cm_sad2<int>(int, int, SAT) => genx.*sad2(int, int)
  llvm::Type *Tys[] = {CI->getType(), Arg0->getType()};
  llvm::Value *Args[] = {Arg0, Arg1};

  llvm::Function *F = getGenXIntrinsic(ID, Tys);
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_sada2.
///
/// template <typename T0, typename T1, int SZ>
/// vector<T0, SZ>
/// __cm_intrinsic_impl_sada2(vector<T1, SZ> src0, vector<T1, SZ> src1,
/// vector<T0, SZ>, int flag);
///
llvm::Value *CGCMRuntime::HandleBuiltinSad2AddImpl(CMCallInfo &CallInfo,
                                                   CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_sada2_impl);

  QualType T0 = CallInfo.CE->getType();
  assert(T0->isCMVectorMatrixType());
  T0 = T0->getCMVectorMatrixElementType();

  assert(CallInfo.CE->getNumArgs() == 4);
  QualType T1 = CallInfo.CE->getArg(0)->getType();
  assert(T1->isCMVectorMatrixType());
  T1 = T1->getCMVectorMatrixElementType();

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);
  llvm::Value *Arg2 = CI->getArgOperand(2);
  llvm::Value *Flag = CI->getArgOperand(3);

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // Check the saturation flag, compare it with 0.
  llvm::Value *CMP = Builder.CreateICmpEQ(
      Flag, llvm::Constant::getNullValue(Flag->getType()), "cmp");

  // non-saturated ID
  unsigned ID0 = GetGenxIntrinsicID(CallInfo, Kind, false);

  // saturated ID
  unsigned ID1 = GetGenxIntrinsicID(CallInfo, Kind, true);

  // cm_sada2<int>(int, int, SAT) => genx.ssad2add.sat(int, int)
  llvm::Type *Tys[] = {CI->getType(), Arg0->getType()};
  llvm::Value *Args[] = {Arg0, Arg1, Arg2};

  llvm::Function *F0 = getGenXIntrinsic(ID0, Tys);
  llvm::CallInst *R0 = Builder.CreateCall(F0, Args, CI->getName());
  R0->setDebugLoc(CI->getDebugLoc());

  llvm::Function *F1 = getGenXIntrinsic(ID1, Tys);
  llvm::CallInst *R1 = Builder.CreateCall(F1, Args, CI->getName());
  R1->setDebugLoc(CI->getDebugLoc());

  // The final result depends on the saturation flag.
  llvm::Value *Result = Builder.CreateSelect(CMP, R0, R1);

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_lrp.
///
/// template <int SZ>
/// vector<float, SZ>
/// __cm_intrinsic_impl_lrp(vector<float, SZ> src0, vector<float, SZ> src1,
/// vector<float, SZ> src2);
///
llvm::Value *CGCMRuntime::HandleBuiltinLrpImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_lrp_impl);
  assert(CallInfo.CE->getType()->isCMVectorMatrixType());
  assert(
      CallInfo.CE->getType()->getCMVectorMatrixElementType()->isFloatingType());
  assert(CallInfo.CE->getNumArgs() == 3);

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Args[] = {CI->getArgOperand(0), CI->getArgOperand(1),
                         CI->getArgOperand(2)};

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // cm_lrp(float, float, float) => genx.lrp(float, float, float)
  llvm::Type *Tys[] = {CI->getType()};

  llvm::Function *F = getGenXIntrinsic(llvm::GenXIntrinsic::genx_lrp, Tys);
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_pln.
///
/// template <int SZ>
/// vector<float, SZ>
/// __cm_intrinsic_impl_pln(vector<float, 4> src0, vector<float, SZ * 2> src1);
///
llvm::Value *CGCMRuntime::HandleBuiltinPlnImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_pln_impl);
  assert(CallInfo.CE->getType()->isCMVectorMatrixType());
  assert(
      CallInfo.CE->getType()->getCMVectorMatrixElementType()->isFloatingType());
  assert(CallInfo.CE->getNumArgs() == 2);

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);

#if _DEBUG
  QualType T0 = CallInfo.CE->getArg(0)->getType();
  QualType T1 = CallInfo.CE->getArg(1)->getType();

  assert(T0->isCMVectorMatrixType());
  assert(T0->getCMVectorMatrixElementType()->isFloatingType());
  assert(T0->getAs<CMVectorType>()->getNumElements() == 4);
  assert(T1->isCMVectorMatrixType());
  assert(T1->getCMVectorMatrixElementType()->isFloatingType());
  assert(T1->getAs<CMVectorType>()->getNumElements() >= 16 &&
         T1->getAs<CMVectorType>()->getNumElements() % 16 == 0);
  assert(CallInfo.CE->getType()->getAs<CMVectorType>()->getNumElements() ==
         T1->getAs<CMVectorType>()->getNumElements() / 2);
#endif

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // cm_pln(float, float) => genx.pln(float, float)
  llvm::Type *Tys[] = {CI->getType(), Arg1->getType()};
  llvm::Value *Args[] = {Arg0, Arg1};
  llvm::Function *F = getGenXIntrinsic(llvm::GenXIntrinsic::genx_pln, Tys);
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_cbit.
///
/// template <typename T, int SZ>
/// vector<unsigned int, SZ>
/// __cm_intrinsic_impl_cbit(vector<T, SZ> src0);
///
llvm::Value *CGCMRuntime::HandleBuiltinCountBitsImpl(CMCallInfo &CallInfo,
                                                     CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_cbit_impl);

  assert(CallInfo.CE->getType()->isCMVectorMatrixType());
  assert(CallInfo.CE->getNumArgs() == 1);

  llvm::CallInst *CI = CallInfo.CI;

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // cm_cbit<uint>(anyint) => genx.cbit(anyint)
  llvm::Type *Tys[] = {CI->getType(), CI->getOperand(0)->getType()};
  llvm::Function *F = getGenXIntrinsic(llvm::GenXIntrinsic::genx_cbit, Tys);
  llvm::CallInst *Result =
      Builder.CreateCall(F, CI->getOperand(0), CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_bfrev.
///
/// template <typename T0, typename T1, int SZ>
/// vector<T0, SZ>
/// __cm_intrinsic_impl_bfrev(vector<T1, SZ> src0);
///
llvm::Value *CGCMRuntime::HandleBuiltinBitFieldReverseImpl(CMCallInfo &CallInfo,
                                                           CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_bfrev_impl);

  assert(CallInfo.CE->getType()->isCMVectorMatrixType());
  assert(CallInfo.CE->getNumArgs() == 1);

  llvm::CallInst *CI = CallInfo.CI;

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // cm_bfrev<int>(int) => genx.bfrev(int)
  llvm::Function *F =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_bfrev, CI->getType());
  llvm::CallInst *Result =
      Builder.CreateCall(F, CI->getOperand(0), CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_bf_insert
///
/// template <typename T0, int SZ>
/// vector<T0, SZ>
/// _cm_intrinsic_impl_bfins(vector<T0, SZ> src0, vector<T0, SZ> src1,
/// vector<T0, SZ> src2, vector<T0, SZ> src3);
///
llvm::Value *CGCMRuntime::HandleBuiltinBitFieldInsertImpl(CMCallInfo &CallInfo,
                                                          CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_bfins_impl);

  assert(CallInfo.CE->getType()->isCMVectorMatrixType());
  assert(CallInfo.CE->getNumArgs() == 4);

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Args[] = {CI->getArgOperand(0), CI->getArgOperand(1),
                         CI->getArgOperand(2), CI->getArgOperand(3)};

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // cm_bf_insert<int>(int,int,int,int) => genx.bfi(int,int,int,int)
  llvm::Function *F =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_bfi, CI->getType());
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_bf_extract
///
/// template <typename T0, int SZ>
/// vector<T0, SZ>
/// _cm_intrinsic_impl_bfext(vector<T0, SZ> src0, vector<T0, SZ> src1,
/// vector<T0, SZ> src2);
///
llvm::Value *CGCMRuntime::HandleBuiltinBitFieldExtractImpl(CMCallInfo &CallInfo,
                                                           CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_bfext_impl);

  assert(CallInfo.CE->getType()->isCMVectorMatrixType());
  assert(CallInfo.CE->getNumArgs() == 3);

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Args[] = {CI->getArgOperand(0), CI->getArgOperand(1),
                         CI->getArgOperand(2)};

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // cm_bf_extract<int>(int,int,int) => genx.sbfe(int,int,int)
  llvm::Function *F = getGenXIntrinsic(
      GetGenxIntrinsicID(CallInfo, Kind, false), CI->getType());
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Returns the corresponding genx intrinsic ID for this call.
unsigned CGCMRuntime::GetGenxIntrinsicID(CMCallInfo &CallInfo,
                                         CMBuiltinKind Kind, bool IsSaturated) {
  // The return element type.
  QualType T0 = CallInfo.CE->getType();
  if (T0->isCMVectorMatrixType())
    T0 = T0->getCMVectorMatrixElementType();

  unsigned ID = llvm::GenXIntrinsic::not_genx_intrinsic;
  switch (Kind) {
  default:
    llvm_unreachable("not implemented yet");
  case CMBK_cm_abs_impl:
    if (T0->isFloatingType())
      ID = llvm::GenXIntrinsic::genx_absf;
    else
      ID = llvm::GenXIntrinsic::genx_absi;
    break;
  case CMBK_cm_avg_impl: {
    QualType T1 = CallInfo.CE->getArg(0)->getType();
    if (T1->isCMVectorMatrixType())
      T1 = T1->getCMVectorMatrixElementType();
    // Both source and destination must have integral type.
    assert(T1->isIntegerType() && T0->isIntegerType());

    unsigned IDs[] = {llvm::GenXIntrinsic::genx_ssavg,
                      llvm::GenXIntrinsic::genx_suavg,
                      llvm::GenXIntrinsic::genx_usavg,
                      llvm::GenXIntrinsic::genx_uuavg,
                      llvm::GenXIntrinsic::genx_ssavg_sat,
                      llvm::GenXIntrinsic::genx_suavg_sat,
                      llvm::GenXIntrinsic::genx_usavg_sat,
                      llvm::GenXIntrinsic::genx_uuavg_sat};
    // int (int, int);
    // int (unsigned, unsigned);
    // unsigned (int, int);
    // unsigned (unsigned, unsigned);
    bool DstSigned = T0->isSignedIntegerType();
    bool SrcSigned = T1->isSignedIntegerType();
    // Intrinsic ID offset
    unsigned Offset = IsSaturated ? 4 : 0;
    unsigned DstOffSet = DstSigned ? 0 : 2;
    unsigned SrcOffset = SrcSigned ? 0 : 1;
    ID = IDs[Offset + DstOffSet + SrcOffset];
    break;
  }
  case CMBK_cm_add_impl:
  case CMBK_cm_mul_impl: {
    if (!IsSaturated) {
      ID = 0; // intrinsic not used for non-saturating cm_add/cm_mul
      break;
    }
    unsigned IDs[] = {
        llvm::GenXIntrinsic::genx_ssadd_sat,
        llvm::GenXIntrinsic::genx_suadd_sat,
        llvm::GenXIntrinsic::genx_usadd_sat,
        llvm::GenXIntrinsic::genx_uuadd_sat,
        llvm::GenXIntrinsic::genx_ssmul_sat,
        llvm::GenXIntrinsic::genx_sumul_sat,
        llvm::GenXIntrinsic::genx_usmul_sat,
        llvm::GenXIntrinsic::genx_uumul_sat,
    };

    // Intrinsic ID offset
    unsigned Offset = (Kind == CMBK_cm_add_impl) ? 0 : 4;

    QualType T1 = CallInfo.CE->getArg(0)->getType();
    if (T1->isCMVectorMatrixType())
      T1 = T1->getCMVectorMatrixElementType();

    bool SrcIntegeral = T1->isIntegerType();
    bool DstIntegeral = T0->isIntegerType();

    // Both source and destination have integral type.
    if (DstIntegeral && SrcIntegeral) {
      // int (int, int);
      // int (unsigned, unsigned);
      // unsigned (int, int);
      // unsigned (unsigned, unsigned);
      bool DstSigned = T0->isSignedIntegerType();
      bool SrcSigned = T1->isSignedIntegerType();
      unsigned SubOffset = DstSigned ? 0 : 2;
      SubOffset += (SrcSigned ? 0 : 1);
      ID = IDs[Offset + SubOffset];
    } else if (!DstIntegeral && SrcIntegeral) {
      // float (int, int);
      // float (unsigned, unsigned);
      // They are emitted in two steps.
      if (T1->isSignedIntegerType())
        ID = IDs[Offset + 0];
      else
        ID = IDs[Offset + 3];
    } else {
      // int (float, float);
      // float (float, float);
      // no intrinsic for addition, but a saturation intrinsic.
      assert(!SrcIntegeral);
    }
    break;
  }
  case CMBK_cm_shl_impl: {
    unsigned IDs[] = {
        llvm::GenXIntrinsic::genx_ssshl,
        llvm::GenXIntrinsic::genx_sushl,
        llvm::GenXIntrinsic::genx_usshl,
        llvm::GenXIntrinsic::genx_uushl,
        llvm::GenXIntrinsic::genx_ssshl_sat,
        llvm::GenXIntrinsic::genx_sushl_sat,
        llvm::GenXIntrinsic::genx_usshl_sat,
        llvm::GenXIntrinsic::genx_uushl_sat,
    };

    // Initialize ID offset for the saturated version
    unsigned Offset = (IsSaturated ? 4 : 0);

    QualType T1 = CallInfo.CE->getArg(0)->getType();
    if (T1->isCMVectorMatrixType())
      T1 = T1->getCMVectorMatrixElementType();

    assert(T1->isIntegerType());
    assert(T0->isIntegerType());

    // int (int, int);
    // int (unsigned, unsigned);
    // unsigned (int, int);
    // unsigned (unsigned, unsigned);
    bool DstSigned = T0->isSignedIntegerType();
    bool SrcSigned = T1->isSignedIntegerType();
    unsigned SubOffset = DstSigned ? 0 : 2;
    SubOffset += (SrcSigned ? 0 : 1);
    ID = IDs[Offset + SubOffset];
    break;
  }
  case CMBK_cm_rol_impl:
    ID = llvm::GenXIntrinsic::genx_rol;
    break;
  case CMBK_cm_ror_impl:
    ID = llvm::GenXIntrinsic::genx_ror;
    break;
  case CMBK_cm_sad2_impl: {
    QualType T1 = CallInfo.CE->getArg(0)->getType();
    if (T1->isCMVectorMatrixType())
      T1 = T1->getCMVectorMatrixElementType();

    ID = T1->isSignedIntegerType() ? llvm::GenXIntrinsic::genx_ssad2
                                   : llvm::GenXIntrinsic::genx_usad2;
    break;
  }
  case CMBK_cm_sada2_impl: {
    QualType TR = CallInfo.CE->getType();
    if (TR->isCMVectorMatrixType())
      TR = TR->getCMVectorMatrixElementType();

    QualType T1 = CallInfo.CE->getArg(0)->getType();
    if (T1->isCMVectorMatrixType())
      T1 = T1->getCMVectorMatrixElementType();

    if (TR->isSignedIntegerType() && T1->isSignedIntegerType()) {
      ID = IsSaturated ? llvm::GenXIntrinsic::genx_sssad2add_sat
                       : llvm::GenXIntrinsic::genx_sssad2add;
    } else if (TR->isSignedIntegerType() && !T1->isSignedIntegerType()) {
      ID = IsSaturated ? llvm::GenXIntrinsic::genx_susad2add_sat
                       : llvm::GenXIntrinsic::genx_susad2add;
    } else if (!TR->isSignedIntegerType() && T1->isSignedIntegerType()) {
      ID = IsSaturated ? llvm::GenXIntrinsic::genx_ussad2add_sat
                       : llvm::GenXIntrinsic::genx_ussad2add;
    } else {
      ID = IsSaturated ? llvm::GenXIntrinsic::genx_uusad2add_sat
                       : llvm::GenXIntrinsic::genx_uusad2add;
    }
    break;
  }
  case CMBK_cm_bfext_impl: {
    QualType TR = CallInfo.CE->getType();
    if (TR->isCMVectorMatrixType())
      TR = TR->getCMVectorMatrixElementType();

    ID = TR->isSignedIntegerType() ? llvm::GenXIntrinsic::genx_sbfe
                                   : llvm::GenXIntrinsic::genx_ubfe;
    break;
  }
  case CMBK_cm_sum_impl:
  case CMBK_cm_sum_sat_impl:
  case CMBK_cm_prod_impl:
  case CMBK_cm_prod_sat_impl:
    llvm_unreachable("handled separately");
    break;
  case CMBK_cm_dp2_impl:
    ID = llvm::GenXIntrinsic::genx_dp2;
    break;
  case CMBK_cm_dp3_impl:
    ID = llvm::GenXIntrinsic::genx_dp3;
    break;
  case CMBK_cm_dp4_impl:
    ID = llvm::GenXIntrinsic::genx_dp4;
    break;
  case CMBK_cm_dph_impl:
    ID = llvm::GenXIntrinsic::genx_dph;
    break;
  case CMBK_cm_line_impl:
    ID = llvm::GenXIntrinsic::genx_line;
    break;
  case CMBK_cm_max_impl:
  case CMBK_cm_reduced_max_impl:
    if (T0->isFloatingType())
      ID = llvm::GenXIntrinsic::genx_fmax;
    else if (T0->isUnsignedIntegerType())
      ID = llvm::GenXIntrinsic::genx_umax;
    else if (T0->isSignedIntegerType())
      ID = llvm::GenXIntrinsic::genx_smax;
    break;
  case CMBK_cm_min_impl:
  case CMBK_cm_reduced_min_impl:
    if (T0->isFloatingType())
      ID = llvm::GenXIntrinsic::genx_fmin;
    else if (T0->isUnsignedIntegerType())
      ID = llvm::GenXIntrinsic::genx_umin;
    else if (T0->isSignedIntegerType())
      ID = llvm::GenXIntrinsic::genx_smin;
    break;
  case CMBK_cm_inv_impl:
    ID = llvm::GenXIntrinsic::genx_inv;
    break;
  case CMBK_cm_log_impl:
    ID = llvm::GenXIntrinsic::genx_log;
    break;
  case CMBK_cm_exp_impl:
    ID = llvm::GenXIntrinsic::genx_exp;
    break;
  case CMBK_cm_sqrt_impl:
    ID = llvm::GenXIntrinsic::genx_sqrt;
    break;
  case CMBK_cm_rsqrt_impl:
    ID = llvm::GenXIntrinsic::genx_rsqrt;
    break;
  case CMBK_cm_sqrt_ieee_impl:
    ID = llvm::GenXIntrinsic::genx_ieee_sqrt;
    break;
  case CMBK_cm_div_ieee_impl:
    ID = llvm::GenXIntrinsic::genx_ieee_div;
    break;
  case CMBK_cm_pow_impl:
    ID = llvm::GenXIntrinsic::genx_pow;
    break;
  case CMBK_cm_sin_impl:
    ID = llvm::GenXIntrinsic::genx_sin;
    break;
  case CMBK_cm_cos_impl:
    ID = llvm::GenXIntrinsic::genx_cos;
    break;
  case CMBK_cm_rndd_impl:
    ID = llvm::GenXIntrinsic::genx_rndd;
    break;
  case CMBK_cm_rndu_impl:
    ID = llvm::GenXIntrinsic::genx_rndu;
    break;
  case CMBK_cm_rnde_impl:
    ID = llvm::GenXIntrinsic::genx_rnde;
    break;
  case CMBK_cm_rndz_impl:
    ID = llvm::GenXIntrinsic::genx_rndz;
    break;
  case CMBK_cm_dp4a_impl: {
    unsigned Dp4aIDs[] = {
        llvm::GenXIntrinsic::genx_ssdp4a,
        llvm::GenXIntrinsic::genx_sudp4a,
        llvm::GenXIntrinsic::genx_usdp4a,
        llvm::GenXIntrinsic::genx_uudp4a,
        llvm::GenXIntrinsic::genx_ssdp4a_sat,
        llvm::GenXIntrinsic::genx_sudp4a_sat,
        llvm::GenXIntrinsic::genx_usdp4a_sat,
        llvm::GenXIntrinsic::genx_uudp4a_sat,
    };

    // Initialize ID offset for the saturated version
    unsigned Dp4aOffset = (IsSaturated ? 4 : 0);

    QualType T1 = CallInfo.CE->getArg(0)->getType();
    if (T1->isCMVectorMatrixType())
      T1 = T1->getCMVectorMatrixElementType();

    assert(T1->isIntegerType());
    assert(T0->isIntegerType());

    bool Dp4aDstSigned = T0->isSignedIntegerType();
    bool Dp4aSrcSigned = T1->isSignedIntegerType();
    unsigned Dp4aSubOffset = Dp4aDstSigned ? 0 : 2;
    Dp4aSubOffset += (Dp4aSrcSigned ? 0 : 1);
    ID = Dp4aIDs[Dp4aOffset + Dp4aSubOffset];
    break;
  }
  case CMBK_cm_bfn_impl:
    ID = llvm::GenXIntrinsic::genx_bfn;
    break;
  case CMBK_cm_dpas_impl:
    ID = llvm::GenXIntrinsic::genx_dpas;
    break;
  case CMBK_cm_dpas2_impl:
    ID = llvm::GenXIntrinsic::genx_dpas2;
    break;
  case CMBK_cm_dpas_nosrc0_impl:
    ID = llvm::GenXIntrinsic::genx_dpas_nosrc0;
    break;
  case CMBK_cm_dpasw_impl:
    ID = llvm::GenXIntrinsic::genx_dpasw;
    break;
  case CMBK_cm_dpasw_nosrc0_impl:
    ID = llvm::GenXIntrinsic::genx_dpasw_nosrc0;
    break;
  case CMBK_cm_tf32_cvt:
    ID = llvm::GenXIntrinsic::genx_tf32_cvt;
    break;
  case CMBK_cm_srnd:
    ID = llvm::GenXIntrinsic::genx_srnd;
    break;
  case CMBK_sample16_impl:
    ID = llvm::GenXIntrinsic::genx_sample;
    break;
  case CMBK_sample32_impl:
    ID = llvm::GenXIntrinsic::genx_sample_unorm;
    break;
  case CMBK_load16_impl:
    ID = llvm::GenXIntrinsic::genx_load;
    break;
  case CMBK_cm_3d_sample:
    ID = llvm::GenXIntrinsic::genx_3d_sample;
    break;
  case CMBK_cm_3d_load:
    ID = llvm::GenXIntrinsic::genx_3d_load;
    break;
  case CMBK_write_atomic_impl:
    llvm_unreachable("handle seperately");
    break;
  case CMBK_write_atomic_typed_impl:
    llvm_unreachable("handle seperately");
    break;
  }
  return ID;
}

/// \brief Postprocess builtin rounding implementations.
///
/// template <int SZ> vector<float, SZ>
/// __cm_intrinsic_impl_rndd(vector<float, SZ> src0);
/// __cm_intrinsic_impl_rndu(vector<float, SZ> src0);
/// __cm_intrinsic_impl_rnde(vector<float, SZ> src0);
/// __cm_intrinsic_impl_rndz(vector<float, SZ> src0);
///
llvm::Value *CGCMRuntime::HandleBuiltinRoundingImpl(CMCallInfo &CallInfo,
                                                    CMBuiltinKind Kind) {
  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind);
  return EmitBuiltinCommonOneArg(*this, ID, CallInfo);
}

namespace {

/// A generic algorithm to emit an optimal reduction sequence.
///
/// It maitains a pair of vectors to be reduced. The first vector size is always
/// a power of 2 (including 1), and its size is equal to or larger than that of
/// the second vector. When the second vector is nullptr, it implies there is no
/// extra values to be reduced. In this case, the reduction is performed solely
/// on the first vector (a complete binary tree reduction). For example,
///
/// Initialization: splits the input vector
///
/// <0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10>
///
/// into
///
/// <0, 1, 2, 3, 4, 5, 6, 7>, <8, 9, 10>
///
/// Loop: binary reduction inside the first vector:
///
/// <4, 6, 8, 10>, <8, 9, 10>
///
/// The size of the first vector is still larger than that of the second.
/// Loop back again. And produces
///
/// <12, 16>, <8, 9, 10>
///
/// Now the vector size of the first value is no larger than that of the
/// second, it performs a reduction between these two vectors and produces:
///
/// <20, 25>, <10>
///
/// Loop back again. And produces
///
/// <45>, <10>
///
/// Again, the first vector size is no larger than that of the second, it
/// performs a reduction between these two vectors and produces:
///
/// <55>, null
///
/// This reduction process stops when the first vector size is 1 and the
/// second vector is null (nothing to reduce).
///
/// There are some extra complexity due to integer overflow and saturation.
/// Currently, the old compiler is doing the following:
///
/// (1) Find a proper excution type:
///
/// - W for B/UB types and # elements is <= 256
/// - D for B/UB types and # elements is > 256
/// - D for W/UW inputs
/// - D for D inputs
/// - UD for UD inputs
///
/// (2) The saturation is performed for the last reduction.
///
/// This somehow behaves similar to other vISA ops, intermediate computations
/// are performed in a higher precision and the truncation happens in the very
/// end. This also implies behavior between *B/*W and *D are different when
/// there is any integer overflow.
///
class CMReductionEmitter {
  CGCMRuntime &CMRT;

  /// Cached CodeGenFunction object.
  CodeGenFunction &CGF;

  /// The reduction intrinsic call information.
  CGCMRuntime::CMCallInfo &CallInfo;

  /// The reduction kind.
  CMBuiltinKind Kind;

  /// A pair of vectors to be reduced.
  llvm::Value *V1;
  llvm::Value *V2;

  /// The execution element type for intermediate reductions, which may be
  /// different from the input type for cm_sum/cm_prod.
  QualType ToEltType;

  /// The input vector type, only initialized when ToEltType is not null.
  QualType FromEltType;

public:
  CMReductionEmitter(CGCMRuntime &CMRT, CGCMRuntime::CMCallInfo &CI,
                     CMBuiltinKind K)
      : CMRT(CMRT), CGF(*CI.CGF), CallInfo(CI), Kind(K), V1(0), V2(0) {
    init();
  }

  /// The interface of the reduction codegen.
  llvm::Value *Emit();

private:
  void init();

  unsigned getV1Size() const {
    if (auto *VTy = dyn_cast<llvm::FixedVectorType>(V1->getType())) {
      unsigned N1 = VTy->getNumElements();
      assert(llvm::isPowerOf2_32(N1) && "not a power of 2");
      return N1;
    }
    return 1;
  }

  unsigned getV2Size() const {
    if (!V2)
      return 0;
    if (auto *VTy = dyn_cast<llvm::FixedVectorType>(V2->getType()))
      return VTy->getNumElements();
    return 1;
  }

  bool isSaturated() const {
    // Saturation for reduced_min or reduced_max handled separately.
    return (Kind == CMBK_cm_sum_sat_impl || Kind == CMBK_cm_prod_sat_impl);
  }

  bool isAdd() const {
    return (Kind == CMBK_cm_sum_impl || Kind == CMBK_cm_sum_sat_impl);
  }

  /// Returns true if the reduction to be performed is the last reduction.
  /// That is, (N1 = 1 and N2 = 1) or (N1 = 2 and N2 = 0).
  bool isLastReduction() const {
    unsigned N1 = getV1Size();
    unsigned N2 = getV2Size();
    return (N1 == 1 && N2 == 1) || (N1 == 2 && N2 == 0);
  }

  /// Perform one reduction between V1 and V2.
  void reduceInter();

  /// Perform one reduction between first and second half within V1.
  void reduceIntra();

  /// Perform the reduction op, the size of LHS is the same as that of RHS.
  llvm::Value *reduce(llvm::Value *LHS, llvm::Value *RHS);

  llvm::Value *reduceAddMul(llvm::Value *LHS, llvm::Value *RHS);

  /// Read a consecutive section of elements.
  llvm::Value *readRegion(llvm::Value *V, unsigned Sz, unsigned Offset = 0);
};

} // namespace

void CMReductionEmitter::init() {
  // Initialize the reduction data.
  llvm::Value *OpVal = CallInfo.CI->getArgOperand(0);
  auto *OpTy = cast<llvm::FixedVectorType>(OpVal->getType());
  unsigned N = OpTy->getNumElements();
  if (llvm::isPowerOf2_32(N)) {
    V1 = OpVal;
    V2 = 0;
  } else {
    unsigned N1 = 1u << llvm::Log2_32(N);
    assert(N > N1);
    V1 = readRegion(OpVal, N1);
    V2 = readRegion(OpVal, N - N1, N1);
  }

  // Initialize the execution element type.
  // TODO: promote half to float or double.
  QualType ArgType = CallInfo.CE->getArg(0)->getType();
  assert(ArgType->isCMVectorType());
  QualType EltType = ArgType->getCMVectorMatrixElementType();
  if (EltType->isIntegerType()) {
    unsigned SizeInBytes = CGF.getContext().getTypeSize(EltType) / 8u;
    ASTContext &Ctx = CGF.getContext();
    if (SizeInBytes == 1) {
      const unsigned Threshold = 256;
      ToEltType = (N <= Threshold) ? Ctx.ShortTy : Ctx.IntTy;
      FromEltType = EltType;
    } else if (SizeInBytes == 2) {
      ToEltType =
          EltType->isSignedIntegerType() ? Ctx.IntTy : Ctx.UnsignedIntTy;
      FromEltType = EltType;
    }
  }
}

llvm::Value *CMReductionEmitter::Emit() {
  while (true) {
    unsigned N1 = getV1Size();
    unsigned N2 = getV2Size();

    // The stopping condition is N1 = 1 and N2 = 0.
    if (N1 == 1 && N2 == 0)
      break;

    // Performan an intra reduction first. This halves the size of V1.
    if (N1 > 1)
      reduceIntra();

    // If the size of V2 is no less than that of V1, do an inter reduction.
    // This reduces N1 number of elements from V2.
    if (N2 >= getV1Size())
      reduceInter();
  }

  // Read the return value from the result vector of size 1.
  V1 = readRegion(V1, 1u);

  // llvm type comparison.
  // For cm_sum<TYPE1>(vector<TYPE2, sz>v, SAT_NOSAT), TYPE1 is the required
  // return type. But the current result V1 is holding the work type which
  // could be 1xTYPE2, or 1xTYPE3 which is the up converted type for TYPE2
  // to meet saturation's need. Furthermore, V1 is the llvm type and we have
  // to conclude its corresponding clang type carefully, so that we can compare
  // TYPE1 with V1 type and decide whether to do the casting.
  // Plus there is one special case here:
  // if the vector size is 1, V1 hasn't been thru the reduction process and it
  // kept its original type TYPE2.
  // Also there are two kinds of casting to be choosen from:
  // (1) Pure type casting;
  // (2) casting with saturation via llvm saturation intrinsics.

  // In below implementation, ToEltType has the work type for reduction with
  // saturation considertation; FromEltType is the original data type.
  // T is the return type and F matches V1's clang type.
  if (V1->getType()->getScalarType() !=
      CallInfo.CI->getType()->getScalarType()) {
    // Compute the cast kind.
    llvm::Instruction::CastOps CastOp;
    // For size-1 vector, didn't do the reduction. So remain as original type.
    auto *VTy = dyn_cast<llvm::FixedVectorType>(
        CallInfo.CI->getArgOperand(0)->getType());
    if (VTy && VTy->getNumElements() == 1)
      ToEltType = FromEltType;
    QualType F =
        ToEltType.isNull() ? CallInfo.CE->getArg(0)->getType() : ToEltType;
    if (F->isCMVectorMatrixType())
      F = F->getCMVectorMatrixElementType();
    QualType T = CallInfo.CE->getType();
    if (T->isCMVectorMatrixType())
      T = T->getCMVectorMatrixElementType();
    bool NeedsCast = CGCMRuntime::getCastOpKind(CastOp, CGF, T, F);
    (void)NeedsCast;
    assert(NeedsCast);
    unsigned ID = llvm::GenXIntrinsic::not_genx_intrinsic;
    if (!isSaturated() || T->isFloatingType()) {
      V1 = CGF.Builder.CreateCast(CastOp, V1, CallInfo.CI->getType());
      cast<llvm::Instruction>(V1)->setDebugLoc(CallInfo.CI->getDebugLoc());
    } else if (isSaturated() && getSatIntrinsicID(ID, T, F)) {
      // Add a saturated move via llvm saturate intrinsics
      llvm::Type *Tys[2] = {CallInfo.CI->getType(),
                            V1->getType()->getScalarType()};
      llvm::Function *Fn = CMRT.getGenXIntrinsic(ID, Tys);
      llvm::CallInst *NewCI =
          CGF.Builder.CreateCall(Fn, V1, CallInfo.CI->getName());
      NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
      V1 = NewCI;
    }
  }

  // Finally, delete the existing implementation intrinsic.
  CallInfo.CI->eraseFromParent();
  return V1;
}

void CMReductionEmitter::reduceIntra() {
  unsigned N1 = getV1Size();
  assert(N1 > 1 && "size too small");

  llvm::Value *LHS = readRegion(V1, N1 / 2);
  llvm::Value *RHS = readRegion(V1, N1 / 2, N1 / 2);
  V1 = reduce(LHS, RHS);
}

void CMReductionEmitter::reduceInter() {
  unsigned N1 = getV1Size();
  unsigned N2 = getV2Size();
  assert(N2 >= N1 && "invalid inter reduction step");

  // This is the last cross vector reduction.
  if (N2 == N1) {
    V1 = reduce(V1, V2);
    V2 = 0;
  } else {
    // Split V2 as V20, V21
    llvm::Value *V20 = readRegion(V2, N1);
    llvm::Value *V21 = readRegion(V2, N2 - N1, N1);
    // Reduce V1 and V20.
    V1 = reduce(V1, V20);
    // The remaining part is V21.
    V2 = V21;
  }
}

llvm::Value *CMReductionEmitter::reduce(llvm::Value *LHS, llvm::Value *RHS) {
  if (Kind == CMBK_cm_sum_impl || Kind == CMBK_cm_sum_sat_impl ||
      Kind == CMBK_cm_prod_impl || Kind == CMBK_cm_prod_sat_impl) {

    // Check if need to convert to the common execution type.
    if (!ToEltType.isNull()) {
      assert(!FromEltType.isNull());
      llvm::Type *ToEltTy = CGF.ConvertType(ToEltType);

      // Compute the cast kind.
      llvm::Instruction::CastOps CastOp;
      bool NeedsCast =
          CGCMRuntime::getCastOpKind(CastOp, CGF, ToEltType, FromEltType);
      (void)NeedsCast;
      assert(NeedsCast);

      llvm::Type *NewTy = ToEltTy;
      if (auto *VTy = dyn_cast<llvm::FixedVectorType>(LHS->getType()))
        NewTy = llvm::FixedVectorType::get(ToEltTy, VTy->getNumElements());

      if (LHS->getType()->getScalarType() != ToEltTy)
        LHS = CGF.Builder.CreateCast(CastOp, LHS, NewTy);
      if (RHS->getType()->getScalarType() != ToEltTy)
        RHS = CGF.Builder.CreateCast(CastOp, RHS, NewTy);
    }

    // Do the actual reduction.
    LHS = reduceAddMul(LHS, RHS);
  } else if (Kind == CMBK_cm_reduced_min_impl ||
             Kind == CMBK_cm_reduced_max_impl) {
    // Emit a call to genx intrinsic.
    unsigned ID = CMRT.GetGenxIntrinsicID(CallInfo, Kind);

    // Overload with its return type and src0's type, which are the same as the
    // type of LHS.
    assert(LHS->getType() == RHS->getType());
    assert(LHS->getType()->getScalarType() == CallInfo.CI->getType());

    llvm::Type *Tys[2] = {LHS->getType(), LHS->getType()};
    llvm::Value *Args[] = {LHS, RHS};
    llvm::Function *GenxFn = CMRT.getGenXIntrinsic(ID, Tys);
    LHS = CGF.Builder.CreateCall(GenxFn, Args);
    cast<llvm::Instruction>(LHS)->setDebugLoc(CallInfo.CI->getDebugLoc());
  }

  return LHS;
}

/// Emit code for a reduction step.
llvm::Value *CMReductionEmitter::reduceAddMul(llvm::Value *LHS,
                                              llvm::Value *RHS) {
  // Complication comes from the last reduction step. The intermediate
  // reductions are simple mul or add.
  if (!isLastReduction()) {
    assert(LHS->getType() == RHS->getType());
    if (LHS->getType()->isFPOrFPVectorTy())
      LHS = isAdd() ? CGF.Builder.CreateFAdd(LHS, RHS)
                    : CGF.Builder.CreateFMul(LHS, RHS);
    else
      LHS = isAdd() ? CGF.Builder.CreateAdd(LHS, RHS)
                    : CGF.Builder.CreateMul(LHS, RHS);
    cast<llvm::Instruction>(LHS)->setDebugLoc(CallInfo.CI->getDebugLoc());
    return LHS;
  }

  // This is the last reduction to be performed. We need to conside both
  // saturation and conversion. The code is the same as, but with the knowledge,
  // whether flag is sat or not, which simplies the codegen.
  //
  // template <typename T0, typename T1> cm_add(T1 LHS, T1 RHS, int flag);
  // template <typename T0, typename T1> cm_mul(T1 LHS, T1 RHS, int flag);
  QualType T0, T1;
  {
    const FunctionDecl *FD = CallInfo.CE->getDirectCallee();
    assert(FD && FD->isTemplateInstantiation());
    const TemplateArgumentList *TempArgs = FD->getTemplateSpecializationArgs();
    T0 = TempArgs->get(0).getAsType();

    // T1 may be the computation type we computed.
    if (ToEltType.isNull())
      T1 = TempArgs->get(1).getAsType();
    else
      T1 = ToEltType;
  }

  // Compute the cast kind from T1 to T0
  llvm::Instruction::CastOps CastOp;
  bool NeedsConv = CMRT.getCastOpKind(CastOp, CGF, T0, T1);
  llvm::Type *DstTy = CallInfo.CI->getType();
  if (LHS->getType()->isVectorTy())
    DstTy = llvm::FixedVectorType::get(CallInfo.CI->getType(), 1u);

  if (T1->isFloatingType()) {
    // No genx intrinsic needed. For example
    //
    // cm_add<float>(float, float, SAT)   => genx.sat(fadd(arg0, arg1))
    // cm_add<float>(double, double, SAT) => genx.sat(fptrunc fadd(arg0, arg1))
    // cm_add<double>(float, float, SAT)  => genx.sat(fpext fadd(arg0, arg1))
    // cm_add<int>(float, float, SAT)     => genx.fptosi.sat(fadd(arg0, arg1))
    LHS = isAdd() ? CGF.Builder.CreateFAdd(LHS, RHS)
                  : CGF.Builder.CreateFMul(LHS, RHS);
    cast<llvm::Instruction>(LHS)->setDebugLoc(CallInfo.CI->getDebugLoc());

    if (T0->isFloatingType()) {
      if (NeedsConv)
        LHS = CGF.Builder.CreateCast(CastOp, LHS, DstTy, "conv");
      if (isSaturated()) {
        llvm::Function *Fn =
            CMRT.getGenXIntrinsic(llvm::GenXIntrinsic::genx_sat, DstTy);
        LHS = CGF.Builder.CreateCall(Fn, LHS, "sat");
      }
      return LHS;
    } else {
      assert(T0->isIntegerType());
      assert(NeedsConv);
      if (!isSaturated())
        return CGF.Builder.CreateCast(CastOp, LHS, DstTy, "conv");

      unsigned ID = llvm::GenXIntrinsic::not_genx_intrinsic;
      getSatIntrinsicID(ID, T0, T1);
      assert(ID != llvm::GenXIntrinsic::not_genx_intrinsic);
      llvm::Type *Tys[] = {DstTy, LHS->getType()};
      llvm::Function *Fn = CMRT.getGenXIntrinsic(ID, Tys);
      return CGF.Builder.CreateCall(Fn, LHS, "sat");
    }
  }

  // Src type is integer.
  assert(T1->isIntegerType());
  unsigned IDs[] = {
      llvm::GenXIntrinsic::genx_ssadd_sat, llvm::GenXIntrinsic::genx_suadd_sat,
      llvm::GenXIntrinsic::genx_usadd_sat, llvm::GenXIntrinsic::genx_uuadd_sat,
      llvm::GenXIntrinsic::genx_ssmul_sat, llvm::GenXIntrinsic::genx_sumul_sat,
      llvm::GenXIntrinsic::genx_usmul_sat, llvm::GenXIntrinsic::genx_uumul_sat,
  };
  unsigned Offset = isAdd() ? 0 : 4;

  if (T0->isFloatingType()) {
    // cm_add<float>(int, int, SAT)
    // => genx.sat(sitofp(genx.ssadd.sat(int, int))
    Offset += T1->isSignedIntegerType() ? 0 : 3;

    llvm::Value *R0 = nullptr;
    if (!isSaturated()) {
      // The non-saturating case does not need an intrinsic. Just use a normal
      // add/mul.
      if (isAdd())
        R0 = CGF.Builder.CreateAdd(LHS, RHS, CallInfo.CI->getName());
      else
        R0 = CGF.Builder.CreateMul(LHS, RHS, CallInfo.CI->getName());
      cast<llvm::Instruction>(R0)->setDebugLoc(CallInfo.CI->getDebugLoc());
    } else {
      // Saturating case.
      llvm::Type *Tys[] = {LHS->getType(), LHS->getType()};
      llvm::Value *Args[] = {LHS, RHS};
      llvm::Function *Fn = CMRT.getGenXIntrinsic(IDs[Offset], Tys);
      R0 = CGF.Builder.CreateCall(Fn, Args);
      cast<llvm::Instruction>(R0)->setDebugLoc(CallInfo.CI->getDebugLoc());
    }

    // We definitely need an int-to-fp conversion here.
    assert(NeedsConv);
    LHS = CGF.Builder.CreateCast(CastOp, R0, DstTy, "conv");

    if (isSaturated()) {
      auto Fn = CMRT.getGenXIntrinsic(llvm::GenXIntrinsic::genx_sat, DstTy);
      LHS = CGF.Builder.CreateCall(Fn, LHS, "sat");
    }
  } else {
    // cm_add<int>(int, int, SAT) => genx.ssadd.sat(int, int)
    assert(T0->isIntegerType());

    llvm::Value *R0 = nullptr;
    if (!isSaturated()) {
      // The non-saturating case does not need an intrinsic. Just promote/demote
      // the args to the destination type and use a normal add/mul.
      auto ConvertedLHS = CGF.Builder.CreateCast(CastOp, LHS, DstTy, "conv");
      cast<llvm::Instruction>(ConvertedLHS)
          ->setDebugLoc(CallInfo.CI->getDebugLoc());
      auto ConvertedRHS = CGF.Builder.CreateCast(CastOp, RHS, DstTy, "conv");
      cast<llvm::Instruction>(ConvertedRHS)
          ->setDebugLoc(CallInfo.CI->getDebugLoc());
      if (isAdd())
        R0 = CGF.Builder.CreateAdd(ConvertedLHS, ConvertedRHS,
                                   CallInfo.CI->getName());
      else
        R0 = CGF.Builder.CreateMul(ConvertedLHS, ConvertedRHS,
                                   CallInfo.CI->getName());
      cast<llvm::Instruction>(R0)->setDebugLoc(CallInfo.CI->getDebugLoc());
      LHS = R0;
    } else {
      // Saturating case.
      Offset += T0->isSignedIntegerType() ? 0 : 2;
      Offset += T1->isSignedIntegerType() ? 0 : 1;

      llvm::Type *Tys[] = {DstTy, LHS->getType()};
      llvm::Value *Args[] = {LHS, RHS};
      llvm::Function *Fn = CMRT.getGenXIntrinsic(IDs[Offset], Tys);
      LHS = CGF.Builder.CreateCall(Fn, Args);
      cast<llvm::Instruction>(LHS)->setDebugLoc(CallInfo.CI->getDebugLoc());
    }
  }

  return LHS;
}

llvm::Value *CMReductionEmitter::readRegion(llvm::Value *V, unsigned Sz,
                                            unsigned Offset) {
  auto *Ty = dyn_cast<llvm::FixedVectorType>(V->getType());
  if (!Ty)
    return V;
  unsigned N = Ty->getNumElements();
  assert(Sz > 0 && "read at least one element?");

  // Single element access.
  if (N == 1) {
    llvm::Value *Index = llvm::ConstantInt::get(CGF.Int32Ty, 0);
    return CGF.Builder.CreateExtractElement(V, Index);
  }
  if (Sz == 1) {
    llvm::Value *Index = llvm::ConstantInt::get(CGF.Int32Ty, Offset);
    return CGF.Builder.CreateExtractElement(V, Index);
  }

  // Whole vector access.
  if (N == Sz)
    return V;

  assert(Offset + Sz <= N && "out of bound");
  return CMRT.EmitReadRegion(CGF, V, Sz, 1u, Offset);
}

/// \brief Postprocess builtin reduction implementations.
///
/// template <typename T, int SZ> T
/// __cm_intrinsic_impl_reduced_min(vector<T, SZ> src);
/// __cm_intrinsic_impl_reduced_max(vector<T, SZ> src);
///
/// template <typename T0, typename T1, int SZ> T0
/// __cm_intrinsic_impl_sum(vector<T1, SZ> src);
/// __cm_intrinsic_impl_prod(vector<T1, SZ> src);
/// __cm_intrinsic_impl_sum_sat(vector<T1, SZ> src);
/// __cm_intrinsic_impl_prod_sat(vector<T1, SZ> src);
///
llvm::Value *CGCMRuntime::HandleBuiltinReductionImpl(CMCallInfo &CallInfo,
                                                     CMBuiltinKind Kind) {
  CMReductionEmitter Emitter(*this, CallInfo, Kind);
  return Emitter.Emit();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Dataport interface
///
////////////////////////////////////////////////////////////////////////////////

/// Return SLM surfaceindex 254
llvm::Value *getSLMSurfaceIndex(CodeGenFunction &CGF) {
  return llvm::ConstantInt::get(CGF.Int32Ty, 254);
}

/// \brief Postprocess oword read implementations.
///
/// template <typename T, int SZ>
/// vector<T, SZ>
/// __cm_intrinsic_impl_oword_read(SurfaceIndex index, int offset);
/// __cm_intrinsic_impl_oword_read_dwaligned(SurfaceIndex index, int offset);
///
llvm::Value *CGCMRuntime::HandleBuiltinOWordReadImpl(CMCallInfo &Info,
                                                     CMBuiltinKind Kind) {
  unsigned ID = (Kind == CMBK_oword_read_impl)
                    ? llvm::GenXIntrinsic::genx_oword_ld
                    : llvm::GenXIntrinsic::genx_oword_ld_unaligned;
  auto *RetTy = cast<llvm::FixedVectorType>(Info.CI->getType());
  assert(llvm::isPowerOf2_32(RetTy->getNumElements()));

  llvm::Function *LoadFn = getGenXIntrinsic(ID, RetTy);
  llvm::FunctionType *LoadFnTy = LoadFn->getFunctionType();

  CGBuilderTy Builder(*Info.CGF, Info.CI);
  SmallVector<llvm::Value *, 3> Args;

  // Modifiers, always null value.
  Args.push_back(llvm::Constant::getNullValue(LoadFnTy->getParamType(0)));
  // SurfaceIndex.
  Args.push_back(Info.CI->getArgOperand(0));
  // Offset in owords for oword_ld but in bytes for oword_ld_unaligned.
  llvm::Value *Offset = Info.CI->getArgOperand(1);
  if (ID == llvm::GenXIntrinsic::genx_oword_ld) {
    llvm::Constant *V16 = llvm::ConstantInt::get(Offset->getType(), OWORD);
    Offset = Builder.CreateExactUDiv(Offset, V16);
  }
  Args.push_back(Offset);

  llvm::CallInst *NewCI = Builder.CreateCall(LoadFn, Args);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  NewCI->setName(Info.CI->getName());

  Info.CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess the oword write implementation.
/// template <typename T, int N>
/// void __cm_intrinsic_impl_oword_write(SurfaceIndex, int, vector<T, N>);
void CGCMRuntime::HandleBuiltinOWordWriteImpl(CMCallInfo &Info,
                                              CMBuiltinKind Kind) {
  // Overload this intrinsic with its input vector type which is the last
  // argument type.
  assert(Info.CI->getNumArgOperands() == 3);
  auto *VecTy = cast<llvm::FixedVectorType>(Info.CI->getArgOperand(2)->getType());
  unsigned ID = llvm::GenXIntrinsic::genx_oword_st;
  llvm::Function *StoreFn = getGenXIntrinsic(ID, VecTy);

  // The data size in OWords is in {1, 2, 4, 8, 16}.
  assert(llvm::isPowerOf2_32(VecTy->getNumElements()));
  assert(VecTy->getPrimitiveSizeInBits() / 8 >= OWORD);

  auto &TOpts = CGM.getTarget().getTargetOpts();
  unsigned MaxOBRWSize = TOpts.CMMaxOWordBlock;

  if (VecTy->getPrimitiveSizeInBits() / 8 > MaxOBRWSize * OWORD) {
    CGM.getDiags().Report(diag::warn_cm_oword_size) << MaxOBRWSize;
    assert(0 && "OWords assertion for debug build");
  }

  CGBuilderTy Builder(*Info.CGF, Info.CI);
  SmallVector<llvm::Value *, 4> Args;

  // SurfaceIndex.
  Args.push_back(Info.CI->getArgOperand(0));
  // Offset in owords.
  llvm::Constant *V16 =
      llvm::ConstantInt::get(Info.CI->getArgOperand(1)->getType(), OWORD);
  Args.push_back(Builder.CreateExactUDiv(Info.CI->getArgOperand(1), V16));
  // Data to write
  Args.push_back(Info.CI->getArgOperand(2));

  llvm::CallInst *NewCI = Builder.CreateCall(StoreFn, Args);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());

  Info.CI->eraseFromParent();
}

/// \brief Postprocess media read implementations.
/// template <typename T, int N, int M, int _M, CmBufferAttrib attr>
/// matrix<T, N, _M>
/// __cm_intrinsic_impl_media_read(SurfaceIndex index, int X, int Y);
llvm::Value *CGCMRuntime::HandleBuiltinMediaReadImpl(CMCallInfo &Info) {
  assert(Info.CI->getNumArgOperands() == 3);
  llvm::Type *RetTy = Info.CI->getType();
  assert(RetTy->isVectorTy());
  unsigned ID = llvm::GenXIntrinsic::genx_media_ld;
  llvm::Function *LoadFn = getGenXIntrinsic(ID, RetTy);
  llvm::FunctionType *LoadFnTy = LoadFn->getFunctionType();

  // Compute block width in bytes.
#if _DEBUG
  QualType CallTy = Info.CE->getType();
  assert(CallTy->isCMMatrixType());
#endif

  // Retrieve the orginal matrix width from the template argument M,
  // and the modifiers from the template argument attr.
  unsigned WidthInBytes = 0, ModifierValue = 0;
  {
    const FunctionDecl *FD = Info.CE->getDirectCallee();
    assert(FD && FD->isTemplateInstantiation());
    const TemplateArgumentList *TempArgs = FD->getTemplateSpecializationArgs();
    unsigned M = TempArgs->get(2).getAsIntegral().getZExtValue();
    WidthInBytes = M * RetTy->getScalarSizeInBits() / 8;
    ModifierValue = TempArgs->get(4).getAsIntegral().getZExtValue();
  }

  SmallVector<llvm::Value *, 8> Args;
  llvm::Type *ModifierType = LoadFn->getFunctionType()->getParamType(0);
  unsigned Modifier = MEDIA_LD_nomod;
  switch (ModifierValue) {
  default:
    break;
  case GENX_NONE:
    Modifier = MEDIA_LD_nomod;
    break;
  case GENX_TOP_FIELD:
    Modifier = MEDIA_LD_top;
    break;
  case GENX_MODIFIED:
    Modifier = MEDIA_LD_modified;
    break;
  case GENX_BOTTOM_FIELD:
    Modifier = MEDIA_LD_bottom;
    break;
  }
  // Modifier.
  Args.push_back(llvm::ConstantInt::get(ModifierType, Modifier));
  // SurfaceIndex.
  Args.push_back(Info.CI->getArgOperand(0));
  // Plane
  Args.push_back(llvm::Constant::getNullValue(LoadFnTy->getParamType(2)));
  // Block width in bytes.
  Args.push_back(
      llvm::ConstantInt::get(LoadFnTy->getParamType(3), WidthInBytes));
  // x byte offset, the input is already in bytes.
  Args.push_back(Info.CI->getArgOperand(1));
  // y byte offset, the input is already in bytes.
  Args.push_back(Info.CI->getArgOperand(2));

  CGBuilderTy Builder(*Info.CGF, Info.CI);
  llvm::CallInst *NewCI = Builder.CreateCall(LoadFn, Args);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  NewCI->setName(Info.CI->getName());

  Info.CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess media write implementations.
/// template <typename T, int N, int M, int _M, CmBufferAttrib attr>
/// void __cm_intrinsic_impl_media_write(SurfaceIndex, int, int, matrix<T, N,
/// _M>);
void CGCMRuntime::HandleBuiltinMediaWriteImpl(CMCallInfo &Info) {
  // Overload this intrinsic with its input vector type which is the last
  // argument type.
  assert(Info.CI->getNumArgOperands() == 4);
  llvm::Type *VecTy = Info.CI->getArgOperand(3)->getType();
  assert(VecTy->isVectorTy());
  unsigned ID = llvm::GenXIntrinsic::genx_media_st;
  llvm::Function *StoreFn = getGenXIntrinsic(ID, VecTy);
  llvm::FunctionType *StoreFnTy = StoreFn->getFunctionType();

  // Retrieve the orginal matrix width from the template argument M,
  // and the modifiers from the template argument attr.
  unsigned WidthInBytes = 0, ModifierValue = 0;
  {
    const FunctionDecl *FD = Info.CE->getDirectCallee();
    assert(FD && FD->isTemplateInstantiation());
    const TemplateArgumentList *TempArgs = FD->getTemplateSpecializationArgs();
    unsigned M = TempArgs->get(2).getAsIntegral().getZExtValue();
    WidthInBytes = M * VecTy->getScalarSizeInBits() / 8;
    ModifierValue = TempArgs->get(4).getAsIntegral().getZExtValue();
  }

  SmallVector<llvm::Value *, 8> Args;
  llvm::Type *ModifierType = StoreFn->getFunctionType()->getParamType(0);
  unsigned Modifier = MEDIA_ST_nomod;
  switch (ModifierValue) {
  default:
    break;
  case GENX_NONE:
    Modifier = MEDIA_ST_nomod;
    break;
  case GENX_TOP_FIELD:
    Modifier = MEDIA_ST_top;
    break;
  case GENX_BOTTOM_FIELD:
    Modifier = MEDIA_ST_bottom;
    break;
  }
  // modifiers
  Args.push_back(llvm::ConstantInt::get(ModifierType, Modifier));
  // surface index
  Args.push_back(Info.CI->getArgOperand(0));
  // plane
  Args.push_back(llvm::Constant::getNullValue(StoreFnTy->getParamType(2)));
  // block width in bytes
  Args.push_back(
      llvm::ConstantInt::get(StoreFnTy->getParamType(3), WidthInBytes));
  // x offset in bytes
  Args.push_back(Info.CI->getArgOperand(1));
  // y offset in rows
  Args.push_back(Info.CI->getArgOperand(2));
  // data to write.
  Args.push_back(Info.CI->getArgOperand(3));

  CGBuilderTy Builder(*Info.CGF, Info.CI);
  llvm::CallInst *NewCI = Builder.CreateCall(StoreFn, Args);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());

  Info.CI->eraseFromParent();
}

/// \brief Postprocess media read implementations.
/// template <typename T, int N, int M, CmBufferAttrib attr,
/// CmSurfacePlaneIndex plane> matrix<T, N, M>
/// __cm_intrinsic_impl_media_read_plane(SurfaceIndex index, int X, int Y);
llvm::Value *CGCMRuntime::HandleBuiltinMediaReadPlane(CMCallInfo &Info) {
  assert(Info.CI->getNumArgOperands() == 3);
  llvm::Type *RetTy = Info.CI->getType();
  assert(RetTy->isVectorTy());
  unsigned ID = llvm::GenXIntrinsic::genx_media_ld;
  llvm::Function *LoadFn = getGenXIntrinsic(ID, RetTy);
  llvm::FunctionType *LoadFnTy = LoadFn->getFunctionType();

  // Compute block width in bytes.
  QualType CallTy = Info.CE->getType();
  assert(CallTy->isCMMatrixType());
  const CMMatrixType *MT = CallTy->castAs<CMMatrixType>();
  unsigned NCols = MT->getNumColumns();
  unsigned BlockWidthInBytes = NCols * RetTy->getScalarSizeInBits() / 8;
  SmallVector<llvm::Value *, 8> Args;
  llvm::Type *ModifierType = LoadFn->getFunctionType()->getParamType(0);
  unsigned ModifierValue = getIntegralValue(Info.CE->getDirectCallee(), 3);
  unsigned Modifier = MEDIA_LD_nomod;
  switch (ModifierValue) {
  default:
    break;
  case GENX_NONE:
    Modifier = MEDIA_LD_nomod;
    break;
  case GENX_TOP_FIELD:
    Modifier = MEDIA_LD_top;
    break;
  case GENX_MODIFIED:
    Modifier = MEDIA_LD_modified;
    break;
  case GENX_BOTTOM_FIELD:
    Modifier = MEDIA_LD_bottom;
    break;
  }
  llvm::Type *PlaneType = LoadFn->getFunctionType()->getParamType(2);
  unsigned Plane = getIntegralValue(Info.CE->getDirectCallee(), 4);
  // Modifier.
  Args.push_back(llvm::ConstantInt::get(ModifierType, Modifier));
  // SurfaceIndex.
  Args.push_back(Info.CI->getArgOperand(0));
  // Plane
  Args.push_back(llvm::ConstantInt::get(PlaneType, Plane));
  // Block width in bytes.
  Args.push_back(
      llvm::ConstantInt::get(LoadFnTy->getParamType(3), BlockWidthInBytes));
  // x byte offset, the input is already in bytes.
  Args.push_back(Info.CI->getArgOperand(1));
  // y byte offset, the input is already in bytes.
  Args.push_back(Info.CI->getArgOperand(2));

  CGBuilderTy Builder(*Info.CGF, Info.CI);
  llvm::CallInst *NewCI = Builder.CreateCall(LoadFn, Args);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  NewCI->setName(Info.CI->getName());

  Info.CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess scattered read/write implementation.
///
/// template <typename T0, typename T1, int N>
/// vector<T1, N>
/// __cm_intrinsic_impl_scatter_read(SurfaceIndex index,
///                                  uint globalOffset,
///                                  vector<uint, N> elementOffset,
///                                  vector<T1, N> oldVal, T0 dummy);
///
/// template <typename T0, typename T1, int N>
/// void __cm_intrinsic_impl_scatter_write(SurfaceIndex index,
///                                        uint globalOffset,
///                                        vector<uint, N> elementOffset,
///                                        vector<T1, N> data, T0 dummy);
///
/// Both globalOffset and elementOffset are in bytes.
///
llvm::Value *CGCMRuntime::HandleBuiltinScatterReadWriteImpl(CMCallInfo &Info,
                                                            bool IsWrite) {
  // NBlocks equals # of bytes for this intrinsic.
  llvm::Type *T0 = Info.CI->getArgOperand(4)->getType();
  unsigned NBlocks = T0->getPrimitiveSizeInBits() / 8;
  unsigned NBlocksLog2 = 0;
  switch (NBlocks) {
  case 1:
    NBlocksLog2 = 0;
    break;
  case 2:
    NBlocksLog2 = 1;
    break;
  case 4:
    NBlocksLog2 = 2;
    break;
  default:
    Error(Info.CE->getExprLoc(), "element size not supported");
  }

  // Use scaled message for any platform since scale is 0.
  CodeGenFunction &CGF = *Info.CGF;
  llvm::CallInst *NewCI;
  if (IsWrite)
    NewCI =
        EmitScatterScaled(CGF, llvm::GenXIntrinsic::genx_scatter_scaled,
                          llvm::APInt(32, NBlocksLog2), // NBlocks
                          0,                            // scale
                          Info.CI->getArgOperand(0),    // surface index
                          Info.CI->getArgOperand(1), // global offset in bytes
                          Info.CI->getArgOperand(2), // element offset in bytes
                          Info.CI->getArgOperand(3)  // data to write
        );
  else
    NewCI =
        EmitGatherScaled(CGF, llvm::GenXIntrinsic::genx_gather_masked_scaled2,
                         llvm::APInt(32, NBlocksLog2), // NBlocks
                         0,                            // scale
                         Info.CI->getArgOperand(0),    // surface index
                         Info.CI->getArgOperand(1),    // global offset in bytes
                         Info.CI->getArgOperand(2), // element offset in bytes
                         Info.CI->getArgOperand(3)  // old value data
        );
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  Info.CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess media write implementations.
/// template <typename T, int N, int M, int _M, CmBufferAttrib attr,
/// CmSurfacePlaneIndex plane>
/// void __cm_intrinsic_impl_media_write_plane(SurfaceIndex index,
/// int X, int Y, matrix<T, N, _M> src);
void CGCMRuntime::HandleBuiltinMediaWritePlane(CMCallInfo &Info) {
  // Overload this intrinsic with its input vector type which is the last
  // argument type.
  assert(Info.CI->getNumArgOperands() == 4);
  llvm::Type *VecTy = Info.CI->getArgOperand(3)->getType();
  assert(VecTy->isVectorTy());
  unsigned ID = llvm::GenXIntrinsic::genx_media_st;
  llvm::Function *StoreFn = getGenXIntrinsic(ID, VecTy);
  llvm::FunctionType *StoreFnTy = StoreFn->getFunctionType();

  // Retrieve the orginal matrix width from the template argument M.
  unsigned WidthInBytes = 0;
  {
    const FunctionDecl *FD = Info.CE->getDirectCallee();
    assert(FD && FD->isTemplateInstantiation());
    const TemplateArgumentList *TempArgs = FD->getTemplateSpecializationArgs();
    unsigned M = TempArgs->get(2).getAsIntegral().getZExtValue();
    WidthInBytes = M * VecTy->getScalarSizeInBits() / 8;
  }

  SmallVector<llvm::Value *, 8> Args;
  llvm::Type *ModifierType = StoreFn->getFunctionType()->getParamType(0);
  unsigned ModifierValue = getIntegralValue(Info.CE->getDirectCallee(), 4);
  unsigned Modifier = MEDIA_ST_nomod;
  switch (ModifierValue) {
  default:
    break;
  case GENX_NONE:
    Modifier = MEDIA_ST_nomod;
    break;
  case GENX_TOP_FIELD:
    Modifier = MEDIA_ST_top;
    break;
  case GENX_BOTTOM_FIELD:
    Modifier = MEDIA_ST_bottom;
    break;
  }
  llvm::Type *PlaneType = StoreFn->getFunctionType()->getParamType(2);
  unsigned Plane = getIntegralValue(Info.CE->getDirectCallee(), 5);
  // modifiers
  Args.push_back(llvm::ConstantInt::get(ModifierType, Modifier));
  // surface index
  Args.push_back(Info.CI->getArgOperand(0));
  // plane
  Args.push_back(llvm::ConstantInt::get(PlaneType, Plane));
  // block width in bytes
  Args.push_back(
      llvm::ConstantInt::get(StoreFnTy->getParamType(3), WidthInBytes));
  // x offset in bytes
  Args.push_back(Info.CI->getArgOperand(1));
  // y offset in rows
  Args.push_back(Info.CI->getArgOperand(2));
  // data to write.
  Args.push_back(Info.CI->getArgOperand(3));

  CGBuilderTy Builder(*Info.CGF, Info.CI);
  llvm::CallInst *NewCI = Builder.CreateCall(StoreFn, Args);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());

  Info.CI->eraseFromParent();
}

/// \brief Postprocess builtin sample16.
/// template<typename T, int N, ChannelMaskType Mask>
/// matrix<T, N, 16>
/// __cm_intrinsic_impl_sample16(SamplerIndex sampIndex, SurfaceIndex surfIndex,
/// vector<float, 16> u, vector<float, 16> v, vector<float, 16> r);
///
llvm::Value *CGCMRuntime::HandleBuiltinSample16Impl(CMCallInfo &CallInfo,
                                                    CMBuiltinKind Kind) {
  assert(Kind == CMBK_sample16_impl);
  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind);

  // Overload with its return type and the vector offset type.
  llvm::Function *Fn = CallInfo.CI->getCalledFunction();
  llvm::Type *Tys[] = {Fn->getReturnType(),
                       Fn->getFunctionType()->getParamType(2)};
  llvm::Function *GenxFn = getGenXIntrinsic(ID, Tys);

  assert(CallInfo.CI->getNumArgOperands() == 5);
  llvm::CallInst *CI = CallInfo.CI;

  // Collect arguments.
  SmallVector<llvm::Value *, 8> Args;
  llvm::Type *MaskType = GenxFn->getFunctionType()->getParamType(0);
  unsigned MaskValue = getIntegralValue(CallInfo.CE->getDirectCallee(), 2);
  Args.push_back(llvm::ConstantInt::get(MaskType, MaskValue));
  for (unsigned I = 0, N = CI->getNumArgOperands(); I != N; ++I)
    Args.push_back(CI->getArgOperand(I));

  // Call genx intrinsic.
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(GenxFn, Args);
  NewCI->setName(CI->getName());
  NewCI->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess builtin sample32.
/// template<int N, ChannelMaskType Mask, OutputFormatControl Ofc>
/// matrix<ushort, N, 32>
/// __cm_intrinsic_impl_sample32(SamplerIndex sampIndex, SurfaceIndex surfIndex,
/// float u, float v, float deltaU, float deltaV);
///
llvm::Value *CGCMRuntime::HandleBuiltinSample32Impl(CMCallInfo &CallInfo,
                                                    CMBuiltinKind Kind) {
  assert(Kind == CMBK_sample32_impl);
  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind);

  // Overload with its return type.
  llvm::Function *Fn = CallInfo.CI->getCalledFunction();
  llvm::Function *GenxFn = getGenXIntrinsic(ID, Fn->getReturnType());

  assert(CallInfo.CI->getNumArgOperands() == 6);
  llvm::CallInst *CI = CallInfo.CI;

  // Collect arguments.
  SmallVector<llvm::Value *, 8> Args;
  unsigned ChannelValue = getIntegralValue(CallInfo.CE->getDirectCallee(), 1);
  unsigned OfcValue = getIntegralValue(CallInfo.CE->getDirectCallee(), 2);
  // bits 0-3 ChannelValue
  // bits 4-5 OutputFormatControl
  unsigned MaskValue = (ChannelValue & 0x0F) | ((OfcValue & 0x03) << 4);
  llvm::Type *MaskType = GenxFn->getFunctionType()->getParamType(0);
  Args.push_back(llvm::ConstantInt::get(MaskType, MaskValue));
  for (unsigned I = 0, N = CI->getNumArgOperands(); I != N; ++I)
    Args.push_back(CI->getArgOperand(I));

  // Call genx intrinsic.
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(GenxFn, Args);
  NewCI->setName(CI->getName());
  NewCI->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess builtin cm_3d_sample, cm_3d_load.
/// template <CM3DSampleOp Op, ChannelMaskType Ch, typename T, int N,
/// typename... Args> void cm_3d_sample(vector_ref<T, N> dst, ushort Aoffimmi,
/// SamplerIndex sampIndex, SurfaceIndex surfIndex, Args... args); template
/// <CM3DLoadOp Op, ChannelMaskType Ch, typename T, int N, typename... Args>
/// void cm_3d_load(vector_ref<T, N> dst, ushort Aofimmi, SurfaceIndex
/// surfIndex, Args... args);
///
void CGCMRuntime::HandleBuiltin3dOperationImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_3d_sample || Kind == CMBK_cm_3d_load);
  CodeGenFunction &CGF = *CallInfo.CGF;

  unsigned VariantArgStart = Kind == CMBK_cm_3d_sample ? 4 : 3;

  unsigned NumArgs = CallInfo.CI->getNumArgOperands();
  assert(NumArgs >= VariantArgStart + 1 && NumArgs <= VariantArgStart + 15);

  // Determine the overloaded intrinsic function and assemble arguments
  SmallVector<llvm::Type *, 16> Tys;
  SmallVector<llvm::Value *, 16> Args;
  const FunctionDecl *FD = CallInfo.CE->getDirectCallee();

  // Determine the SIMD width, based off the number of elements in the
  // first of the variant parameters.
  QualType VMT = CallInfo.CE->getArg(VariantArgStart)->getType();
  assert(VMT->isCMVectorMatrixType());
  unsigned SimdWidth;
  if (VMT->isCMMatrixType()) {
    auto MT = VMT->castAs<CMMatrixType>();
    SimdWidth = MT->getNumRows() * MT->getNumColumns();
  } else
    SimdWidth = VMT->castAs<CMVectorType>()->getNumElements();

  // The vISA spec says that the additional arguments only need to
  // have at least the SIMD Width elements in, rather than exactly.
  // So, if they appear to have more, do not fault it, just make
  // sure the closest valid SIMD width is chosen.
  if (SimdWidth < 8)
    CGF.CGM.Error(CallInfo.CE->getArg(3)->getExprLoc(),
                  "cm_3d_sample argument must have at least 8 elements");
  else if (SimdWidth < 16)
    SimdWidth = 8;
  else
    SimdWidth = 16;

  auto MaskType = getMaskType(CGF.getLLVMContext(), SimdWidth);

  llvm::Value *Dst = CallInfo.CI->getArgOperand(0);
  llvm::Type *DstTy = Dst->getType();
  assert(DstTy->isPointerTy() &&
         "pointer type expected for destination argument");
  Tys.push_back(DstTy->getPointerElementType());
  Tys.push_back(MaskType);

  Args.push_back(
      llvm::ConstantInt::get(CGF.Int32Ty, getIntegralValue(FD, 0))); // Opcode
  Args.push_back(llvm::Constant::getAllOnesValue(
      MaskType)); // Predicate, used to determine execution size
  Args.push_back(llvm::ConstantInt::get(
      CGF.Int32Ty, getIntegralValue(FD, 1))); // Channel mask
  for (unsigned SI = 1; SI < VariantArgStart; ++SI)
    Args.push_back(CallInfo.CI->getArgOperand(
        SI)); // Any required Aofimmi value, and sampler and surface indices
  Tys.push_back(CallInfo.CI->getArgOperand(VariantArgStart)
                    ->getType()); // First argument type
  Args.push_back(CallInfo.CI->getArgOperand(VariantArgStart)); // First argument

  // Remaining optional arguments
  unsigned I;
  for (I = VariantArgStart + 1; I < NumArgs; ++I) {
    // Validate that each of the variadic arguments is a vector or matrix with
    // at least the number of elements as required by the execution size.

    QualType AT = CallInfo.CE->getArg(I)->getType();
    unsigned NumElements = 0;
    if (AT->isCMMatrixType())
      NumElements = AT->castAs<CMMatrixType>()->getNumRows() *
                    AT->castAs<CMMatrixType>()->getNumColumns();
    else if (AT->isCMVectorType())
      NumElements = AT->castAs<CMVectorType>()->getNumElements();
    else
      CGF.CGM.Error(CallInfo.CE->getArg(I)->getExprLoc(),
                    "must be a matrix or vector type");
    if (NumElements < SimdWidth)
      CGF.CGM.Error(CallInfo.CE->getArg(I)->getExprLoc(),
                    "matrix or vector contains too few elements");

    Tys.push_back(CallInfo.CI->getArgOperand(I)->getType());
    Args.push_back(CallInfo.CI->getArgOperand(I));
  }

  // Pad out remaining intrinsic operands with zero
  llvm::Type *PadTy = llvm::FixedVectorType::get(CGF.FloatTy, SimdWidth);
  llvm::Value *PadVal = llvm::Constant::getNullValue(PadTy);
  for (; I < VariantArgStart + 15; ++I) {
    Tys.push_back(PadTy);
    Args.push_back(PadVal);
  }

  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind);
  llvm::Function *GenxFn = getGenXIntrinsic(ID, Tys);

  // Call the intrinsic
  llvm::CallInst *NewCI = CGF.Builder.CreateCall(GenxFn, Args);
  NewCI->setName(CallInfo.CI->getName());
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());

  // Store its result
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  CallInfo.CI->eraseFromParent();
}

/// \brief Postprocess builtin load16.
/// template<typename T, int N, ChannelMaskType Mask>
/// matrix<T, N, 16>
/// __cm_intrinsic_impl_load16(SurfaceIndex surfIndex, vector<float, 16> u,
/// vector<float, 16> v, vector<float, 16> r);
///
llvm::Value *CGCMRuntime::HandleBuiltinLoad16Impl(CMCallInfo &CallInfo,
                                                  CMBuiltinKind Kind) {
  assert(Kind == CMBK_load16_impl);
  unsigned ID = GetGenxIntrinsicID(CallInfo, Kind);

  // Overload with its return type and the vector offset type.
  llvm::Function *Fn = CallInfo.CI->getCalledFunction();
  llvm::Type *Tys[] = {Fn->getReturnType(),
                       Fn->getFunctionType()->getParamType(1)};
  llvm::Function *GenxFn = getGenXIntrinsic(ID, Tys);

  assert(CallInfo.CI->getNumArgOperands() == 4);
  llvm::CallInst *CI = CallInfo.CI;

  // Collect arguments.
  SmallVector<llvm::Value *, 8> Args;
  unsigned MaskValue = getIntegralValue(CallInfo.CE->getDirectCallee(), 2);
  llvm::Type *MaskType = GenxFn->getFunctionType()->getParamType(0);
  Args.push_back(llvm::ConstantInt::get(MaskType, MaskValue));
  for (unsigned I = 0, N = CI->getNumArgOperands(); I != N; ++I)
    Args.push_back(CI->getArgOperand(I));

  // Call genx intrinsic.
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(GenxFn, Args);
  NewCI->setName(CI->getName());
  NewCI->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

namespace {

// DO NOT MODIFY THE ENCODING.
typedef enum _CmAtomicOpType_ {
  ATOMIC_ADD = 0x0,
  ATOMIC_SUB = 0x1,
  ATOMIC_INC = 0x2,
  ATOMIC_DEC = 0x3,
  ATOMIC_MIN = 0x4,
  ATOMIC_MAX = 0x5,
  ATOMIC_XCHG = 0x6,
  ATOMIC_CMPXCHG = 0x7,
  ATOMIC_AND = 0x8,
  ATOMIC_OR = 0x9,
  ATOMIC_XOR = 0xa,
  ATOMIC_MINSINT = 0xb,
  ATOMIC_MAXSINT = 0xc,
  ATOMIC_FMAX = 0x10,
  ATOMIC_FMIN = 0x11,
  ATOMIC_FCMPWR = 0x12,
  ATOMIC_FADD = 0x13,
  ATOMIC_FSUB = 0x14
} CmAtomicOpType;

unsigned getAtomicIntrinsicID(CmAtomicOpType Op) {
  switch (Op) {
  case ATOMIC_ADD:
    return llvm::GenXIntrinsic::genx_dword_atomic2_add;
  case ATOMIC_SUB:
    return llvm::GenXIntrinsic::genx_dword_atomic2_sub;
  case ATOMIC_INC:
    return llvm::GenXIntrinsic::genx_dword_atomic2_inc;
  case ATOMIC_DEC:
    return llvm::GenXIntrinsic::genx_dword_atomic2_dec;
  case ATOMIC_MIN:
    return llvm::GenXIntrinsic::genx_dword_atomic2_min;
  case ATOMIC_MAX:
    return llvm::GenXIntrinsic::genx_dword_atomic2_max;
  case ATOMIC_XCHG:
    return llvm::GenXIntrinsic::genx_dword_atomic2_xchg;
  case ATOMIC_CMPXCHG:
    return llvm::GenXIntrinsic::genx_dword_atomic2_cmpxchg;
  case ATOMIC_AND:
    return llvm::GenXIntrinsic::genx_dword_atomic2_and;
  case ATOMIC_OR:
    return llvm::GenXIntrinsic::genx_dword_atomic2_or;
  case ATOMIC_XOR:
    return llvm::GenXIntrinsic::genx_dword_atomic2_xor;
  case ATOMIC_MINSINT:
    return llvm::GenXIntrinsic::genx_dword_atomic2_imin;
  case ATOMIC_MAXSINT:
    return llvm::GenXIntrinsic::genx_dword_atomic2_imax;
  case ATOMIC_FMAX:
    return llvm::GenXIntrinsic::genx_dword_atomic2_fmax;
  case ATOMIC_FMIN:
    return llvm::GenXIntrinsic::genx_dword_atomic2_fmin;
  case ATOMIC_FCMPWR:
    return llvm::GenXIntrinsic::genx_dword_atomic2_fcmpwr;
  case ATOMIC_FADD:
    return llvm::GenXIntrinsic::genx_dword_atomic2_fadd;
  case ATOMIC_FSUB:
    return llvm::GenXIntrinsic::genx_dword_atomic2_fsub;
  }

  llvm_unreachable("invalid atomic operation");
}

unsigned getAtomicTypedIntrinsicID(CmAtomicOpType Op) {
  switch (Op) {
  case ATOMIC_ADD:
    return llvm::GenXIntrinsic::genx_typed_atomic_add;
  case ATOMIC_SUB:
    return llvm::GenXIntrinsic::genx_typed_atomic_sub;
  case ATOMIC_INC:
    return llvm::GenXIntrinsic::genx_typed_atomic_inc;
  case ATOMIC_DEC:
    return llvm::GenXIntrinsic::genx_typed_atomic_dec;
  case ATOMIC_MIN:
    return llvm::GenXIntrinsic::genx_typed_atomic_min;
  case ATOMIC_MAX:
    return llvm::GenXIntrinsic::genx_typed_atomic_max;
  case ATOMIC_XCHG:
    return llvm::GenXIntrinsic::genx_typed_atomic_xchg;
  case ATOMIC_CMPXCHG:
    return llvm::GenXIntrinsic::genx_typed_atomic_cmpxchg;
  case ATOMIC_AND:
    return llvm::GenXIntrinsic::genx_typed_atomic_and;
  case ATOMIC_OR:
    return llvm::GenXIntrinsic::genx_typed_atomic_or;
  case ATOMIC_XOR:
    return llvm::GenXIntrinsic::genx_typed_atomic_xor;
  case ATOMIC_MINSINT:
    return llvm::GenXIntrinsic::genx_typed_atomic_imin;
  case ATOMIC_MAXSINT:
    return llvm::GenXIntrinsic::genx_typed_atomic_imax;
  case ATOMIC_FMAX:
    return llvm::GenXIntrinsic::genx_typed_atomic_fmax;
  case ATOMIC_FMIN:
    return llvm::GenXIntrinsic::genx_typed_atomic_fmin;
  case ATOMIC_FCMPWR:
    return llvm::GenXIntrinsic::genx_typed_atomic_fcmpwr;
  case ATOMIC_FADD:
    return llvm::GenXIntrinsic::genx_typed_atomic_fadd;
  case ATOMIC_FSUB:
    return llvm::GenXIntrinsic::genx_typed_atomic_fsub;
  }

  llvm_unreachable("invalid atomic operation");
}

} // namespace

///
/// template <CmAtomicOpType Op, int N, typename T>
/// vector<T, N>
/// __cm_intrinsic_impl_atomic_write(vector<ushort, N> mask,
///                                  SurfaceIndex index,
///                                  vector<uint, N> elementOffset,
///                                  vector<T, N> src0, vector<T, N> src1,
///                                  vector<T, N> oldVal);
///
llvm::Value *CGCMRuntime::HandleBuiltinWriteAtomicImpl(CMCallInfo &CallInfo,
                                                       CMBuiltinKind Kind) {
  llvm::LLVMContext &C = CallInfo.CGF->getLLVMContext();

  CmAtomicOpType Op = static_cast<CmAtomicOpType>(
      getIntegralValue(CallInfo.CE->getDirectCallee(), 0));
  unsigned ID = getAtomicIntrinsicID(Op);

  // Types for overloading
  SmallVector<llvm::Type *, 8> Tys;
  // Return type
  Tys.push_back(CallInfo.CI->getType());
  // Predicate type
  auto *VTy = cast<llvm::FixedVectorType>(CallInfo.CI->getType());
  auto *MaskTy = getMaskType(C, VTy->getNumElements());
  Tys.push_back(MaskTy);
  // Offset type (selectively mangled)
  if (ID != llvm::GenXIntrinsic::genx_dword_atomic_cmpxchg &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic_inc &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic_dec &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_cmpxchg &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_inc &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_dec)
    Tys.push_back(CallInfo.CI->getArgOperand(2)->getType());

  llvm::Function *GenxFn = getGenXIntrinsic(ID, Tys);

  assert(CallInfo.CI->getNumArgOperands() == 6);
  llvm::CallInst *CI = CallInfo.CI;
  CGBuilderTy &Builder = CallInfo.CGF->Builder;

  // Collect arguments.
  SmallVector<llvm::Value *, 8> Args;

  // Predicate
  llvm::Value *Pred = CI->getOperand(0);
  Pred = Builder.CreateTrunc(Pred, MaskTy);
  if (auto Inst = dyn_cast<llvm::Instruction>(Pred))
    Inst->setDebugLoc(CI->getDebugLoc());
  Args.push_back(Pred);

  // SurfaceIndex
  Args.push_back(CI->getArgOperand(1));

  // Convert offset in DWords to in bytes.
  llvm::Value *Offset = CI->getArgOperand(2);
  llvm::Type *OffsetTy = Offset->getType();
  Offset = Builder.CreateMul(Offset, llvm::ConstantInt::get(OffsetTy, 4));
  Args.push_back(Offset);

  // INC or DEC does not have any source.
  if (ID != llvm::GenXIntrinsic::genx_dword_atomic_inc &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic_dec &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_inc &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_dec)
    Args.push_back(CI->getArgOperand(3));

  // cmpxchg or fcmpwr takes one extra source.
  if (ID == llvm::GenXIntrinsic::genx_dword_atomic_cmpxchg ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic_fcmpwr ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic2_cmpxchg ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic2_fcmpwr)

    Args.push_back(CI->getArgOperand(4));

  // Call genx intrinsic.
  llvm::CallInst *NewCI = Builder.CreateCall(GenxFn, Args);
  NewCI->setName(CI->getName());
  NewCI->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

///
/// template <CmAtomicType Op, typename T, int N, typename... Args>
/// vector<T, N>
/// __cm_intrinsic_impl_atomic_write_typed(vector<ushort, N> mask,
///                                        SurfaceIndex surfIndex,
///                                        vector<T, N> src0, vector<T, N> src1,
///                                        vector<uint, N> u, Args... args);
///
llvm::Value *
CGCMRuntime::HandleBuiltinWriteAtomicTypedImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  llvm::LLVMContext &C = CallInfo.CGF->getLLVMContext();

  CmAtomicOpType Op = static_cast<CmAtomicOpType>(
      getIntegralValue(CallInfo.CE->getDirectCallee(), 0));
  unsigned ID = getAtomicTypedIntrinsicID(Op);

  // Types for overloading
  SmallVector<llvm::Type *, 8> Tys;
  // Return type
  Tys.push_back(CallInfo.CI->getType());
  // Predicate type
  auto *VTy = cast<llvm::FixedVectorType>(CallInfo.CI->getType());
  auto *MaskTy = getMaskType(C, VTy->getNumElements());
  Tys.push_back(MaskTy);
  // Coordinate type
  Tys.push_back(CallInfo.CI->getArgOperand(4)->getType());

  assert(CallInfo.CI->getNumArgOperands() >= 5);
  llvm::CallInst *CI = CallInfo.CI;
  CGBuilderTy &Builder = CallInfo.CGF->Builder;
  llvm::Function *GenxFn = getGenXIntrinsic(ID, Tys);

  // Collect arguments.
  SmallVector<llvm::Value *, 8> Args;

  // Predicate
  llvm::Value *Pred = CI->getOperand(0);
  Pred = Builder.CreateTrunc(Pred, MaskTy);
  if (auto Inst = dyn_cast<llvm::Instruction>(Pred))
    Inst->setDebugLoc(CI->getDebugLoc());
  Args.push_back(Pred);

  // SurfaceIndex
  Args.push_back(CI->getArgOperand(1));

  // INC or DEC does not have any source.
  if (ID != llvm::GenXIntrinsic::genx_typed_atomic_inc &&
      ID != llvm::GenXIntrinsic::genx_typed_atomic_dec)
    Args.push_back(CI->getArgOperand(2));

  // cmpxchg or fcmpwr takes one extra source.
  if (ID == llvm::GenXIntrinsic::genx_typed_atomic_cmpxchg ||
      ID == llvm::GenXIntrinsic::genx_typed_atomic_fcmpwr)
    Args.push_back(CI->getArgOperand(3));

  // u
  Args.push_back(CI->getArgOperand(4));

  // v, r and LOD
  for (unsigned i = 5; i < 8; i++) {
    if (CallInfo.CI->getNumArgOperands() > i) {
      Args.push_back(CI->getArgOperand(i));
    } else {
      unsigned parm_to_fill = Args.size();
      Args.push_back(llvm::UndefValue::get(
          GenxFn->getFunctionType()->getParamType(parm_to_fill)));
    }
  }

  // Call genx intrinsic.
  llvm::CallInst *NewCI = Builder.CreateCall(GenxFn, Args);
  NewCI->setName(CI->getName());
  NewCI->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

/// template <int N>
/// uint __cm_intrinsic_pack_mask(vector<ushort, N> src);
///
/// template <int N, int M>
/// uint __cm_intrinsic_pack_mask(matrix<ushort, N, M> src);
///
/// pack_mask (N) <dst> <src1>
/// Translates pack_mask pseudo-op into a couple of GenX instructions.
/// dst must be an int type scalar, src1 must be a vector/matrix of ushort
/// N can only be 8, 16 or 32. Below are the type conversions:
///     8 x i16 =>  8 x i1 =>  i8 => i32
///    16 x i16 => 16 x i1 => i16 => i32
///    32 x i16 => 32 x i1 => i32
///
llvm::Value *CGCMRuntime::HandleBuiltinPackMaskImpl(CMCallInfo &CallInfo) {
  assert(CallInfo.CI->getNumArgOperands() == 1);
  llvm::CallInst *CI = CallInfo.CI;
  CGBuilderTy &Builder = CallInfo.CGF->Builder;
  llvm::LLVMContext &Context = CallInfo.CGF->getLLVMContext();
  auto *FD = CallInfo.CE->getDirectCallee();

  int N = getIntegralValue(FD, 0);

  assert(FD && FD->isTemplateInstantiation());
  const TemplateArgumentList *TempArgs = FD->getTemplateSpecializationArgs();
  // we may have 2 integer template arguments if operand is matrix
  if (TempArgs->size() == 2)
    N *= getIntegralValue(FD, 1);

  llvm::Value *Trunc = Builder.CreateTrunc(
      CI->getArgOperand(0),
      llvm::FixedVectorType::get(llvm::Type::getInt1Ty(Context), N));
  llvm::Type *Ty = llvm::Type::getIntNTy(Context, N);

  llvm::Value *BitCast = Builder.CreateBitCast(Trunc, Ty);
  llvm::Value *Result = BitCast;
  if (N != 32) {
    Result = Builder.CreateCast(llvm::Instruction::ZExt, BitCast,
                                llvm::Type::getInt32Ty(Context));
  }

  Result->setName(CI->getName());
  cast<llvm::Instruction>(Result)->setDebugLoc(CI->getDebugLoc());
  CI->eraseFromParent();
  return Result;
}

/// template <typename T, int N>
/// vector<ushort, N> cm_unpack_mask(uint src1);
///
/// unpack_mask (N) <dst> <src1>
/// Translates unpack_mask pseudo-op into a couple of GenX instructions.
/// dst must be a vector of ushort, src1 must be an int type scalar
/// N can only be 8, 16 or 32.
/// Generating the following predicate and predicated move in vISA IR:
///   setp (M1, 16) P1 0x6e:ud
///   (P1) mov (M1, 16) V34(0,0)<1> 0x1:d
///
/// Below are the type conversions:
///   i32 =>        32 x i1 => 32 x i16
///   i32 => i16 => 16 x i1 => 16 x i16
///   i32 => i8  =>  8 x i1 =>  8 x i16
///
llvm::Value *CGCMRuntime::HandleBuiltinUnPackMaskImpl(CMCallInfo &CallInfo) {
  assert(CallInfo.CI->getNumArgOperands() == 1);
  llvm::CallInst *CI = CallInfo.CI;
  CGBuilderTy &Builder = CallInfo.CGF->Builder;
  llvm::LLVMContext &Context = CallInfo.CGF->getLLVMContext();
  unsigned N = getIntegralValue(CallInfo.CE->getDirectCallee(), 1);

  // get N x i1
  llvm::Value *Arg0 = CI->getArgOperand(0);
  unsigned Width = Arg0->getType()->getPrimitiveSizeInBits();
  if (Width > N) {
    llvm::Type *Ty = llvm::IntegerType::get(Context, N);
    Arg0 = Builder.CreateTrunc(Arg0, Ty);
    cast<llvm::Instruction>(Arg0)->setDebugLoc(CI->getDebugLoc());
  }
  assert(Arg0->getType()->getPrimitiveSizeInBits() == N);
  Arg0 = Builder.CreateBitCast(
      Arg0, llvm::FixedVectorType::get(llvm::Type::getInt1Ty(Context), N));

  // get N x i16
  llvm::Value *NewCI = Builder.CreateZExt(
      Arg0, llvm::FixedVectorType::get(llvm::Type::getInt16Ty(Context), N));
  NewCI->takeName(CI);
  cast<llvm::Instruction>(NewCI)->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return NewCI;
}

/// template <typename T = void>
/// vector<uint, 4> cm_rdtsc();
///
llvm::Value *CGCMRuntime::HandleBuiltinRDTSC(CMCallInfo &Info) {
  unsigned ID = llvm::GenXIntrinsic::genx_timestamp;
  llvm::Function *F = getGenXIntrinsic(ID, Info.CI->getType());
  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(F);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  Info.CI->eraseFromParent();
  return NewCI;
}

/// \brief Check if a value / expression is an integral constant or not. Return
/// true if it is an integer constant, false otherwise. If input llvm value is
/// already a constant, then do not check its corresponding expression AST. We
/// further check its corresponding expression just in case clang does not fold
/// this expression to a constant during CodeGen.
static bool getConstantValue(CGCMRuntime &CMRT, CodeGenFunction &CGF,
                             uint32_t &Result, llvm::Value *V, const Expr *E) {
  if (llvm::ConstantInt *CI = dyn_cast_or_null<llvm::ConstantInt>(V)) {
    Result = static_cast<uint32_t>(CI->getZExtValue());
    return true;
  }

  Expr::EvalResult SIResult;

  if (!E->EvaluateAsInt(SIResult, CGF.getContext())) {
    CMRT.Error(E->getExprLoc(), "compile time integer constant expected");
    return false;
  }
  llvm::APSInt SI = SIResult.Val.getInt();

  Result = static_cast<uint32_t>(SI.getZExtValue());
  return true;
}

/// template <int N1, int N2>
/// cm_send(matrix_ref<ushort, N1, 16> rspVar, matrix<ushort, N2, 16> msgVar,
///         uint exDesc, uint msgDesc, uint sendc);
///
/// template <int N1>
/// cm_send(int dummy, matrix<ushort, N2, 16> msgVar, uint exDesc, uint msgDesc,
///         uint sendc);
///
llvm::Value *CGCMRuntime::HandleBuiltinSendImpl(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  // Check whether exDesc and sendc are constant. If not, issue an
  // error and use 0.
  uint32_t ExDesc = 0;
  getConstantValue(*this, CGF, ExDesc, CallInfo.CI->getArgOperand(2),
                   CallInfo.CE->getArg(2));
  uint32_t IsSendc = 0;
  getConstantValue(*this, CGF, IsSendc, CallInfo.CI->getArgOperand(4),
                   CallInfo.CE->getArg(4));

  // Types for overloading.
  llvm::Value *Arg0 = CallInfo.CI->getArgOperand(0);
  llvm::Value *Arg1 = CallInfo.CI->getArgOperand(1);

  // A dummy parameter is not a pointer.
  bool HasDst = Arg0->getType()->isPointerTy();

  SmallVector<llvm::Type *, 8> Tys;
  if (HasDst)
    Tys.push_back(Arg0->getType()->getPointerElementType());  // rspVar
  Tys.push_back(llvm::Type::getInt1Ty(CGF.getLLVMContext())); // predicate
  Tys.push_back(Arg1->getType());                             // msgVar

  unsigned ID = HasDst ? llvm::GenXIntrinsic::genx_raw_send
                       : llvm::GenXIntrinsic::genx_raw_send_noresult;
  llvm::Function *Fn = getGenXIntrinsic(ID, Tys);
  llvm::FunctionType *FTy = Fn->getFunctionType();

  SmallVector<llvm::Value *, 8> Args;
  Args.push_back(
      llvm::ConstantInt::get(FTy->getParamType(0), IsSendc & 0x1)); // modifier
  Args.push_back(llvm::ConstantInt::getTrue(FTy->getParamType(1))); // predicate
  Args.push_back(llvm::ConstantInt::get(FTy->getParamType(2), ExDesc));
  Args.push_back(CallInfo.CI->getArgOperand(3)); // msgDesc
  Args.push_back(CallInfo.CI->getArgOperand(1)); // msgVar
  if (HasDst)
    Args.push_back(CGF.Builder.CreateDefaultAlignedLoad(Arg0)); // oldDst

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, Args);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  if (HasDst) {
    NewCI->takeName(CallInfo.CI);
    CGF.Builder.CreateDefaultAlignedStore(NewCI, Arg0);
  }

  CallInfo.CI->eraseFromParent();
  return NewCI;
}

/// template <int N1, int N2, int N3>
/// void cm_sends(matrix_ref<ushort, N1, 16> rspVar, matrix<ushort, N2, 16>
///               msgVar, matrix<ushort, N3, 16> msgVar2, uint exDesc,
///               uint msgDesc, uint sendc);
///
/// template <int N1, int N2>
/// void cm_sends(int dummy, matrix<ushort, N1, 16> msgVar,
//                matrix<ushort, N1, 16> msgVar2, uint exDesc,
///               uint msgDesc, uint sendc);
///
llvm::Value *CGCMRuntime::HandleBuiltinSendsImpl(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  // Check whether sendc is constant. If not, issue an error and use 0.
  uint32_t IsSendc = 0;
  getConstantValue(*this, CGF, IsSendc, CallInfo.CI->getArgOperand(5),
                   CallInfo.CE->getArg(5));

  uint32_t ExDescVal = 0;
  getConstantValue(*this, CGF, ExDescVal, CallInfo.CI->getArgOperand(3),
                   CallInfo.CE->getArg(3));
  uint8_t SFID = ExDescVal & 0xF;

  // Types for overloading.
  llvm::Value *Arg0 = CallInfo.CI->getArgOperand(0);
  llvm::Value *Arg1 = CallInfo.CI->getArgOperand(1);
  llvm::Value *Arg2 = CallInfo.CI->getArgOperand(2);

  // A dummy parameter is not a pointer.
  bool HasDst = Arg0->getType()->isPointerTy();

  SmallVector<llvm::Type *, 8> Tys;
  if (HasDst)
    Tys.push_back(Arg0->getType()->getPointerElementType());  // rspVar
  Tys.push_back(llvm::Type::getInt1Ty(CGF.getLLVMContext())); // predicate
  Tys.push_back(Arg1->getType());                             // msgVar1
  Tys.push_back(Arg2->getType());                             // msgVar2

  unsigned ID = HasDst ? llvm::GenXIntrinsic::genx_raw_sends
                       : llvm::GenXIntrinsic::genx_raw_sends_noresult;
  llvm::Function *Fn = getGenXIntrinsic(ID, Tys);
  llvm::FunctionType *FTy = Fn->getFunctionType();

  SmallVector<llvm::Value *, 8> Args;
  Args.push_back(
      llvm::ConstantInt::get(FTy->getParamType(0), IsSendc & 0x1)); // modifier
  Args.push_back(llvm::ConstantInt::getTrue(FTy->getParamType(1))); // predicate
  Args.push_back(llvm::ConstantInt::get(CGF.Int8Ty, SFID));         // SFID
  Args.push_back(CallInfo.CI->getArgOperand(3));                    // ExMsgDesc
  Args.push_back(CallInfo.CI->getArgOperand(4));                    // msgDesc
  Args.push_back(CallInfo.CI->getArgOperand(1));                    // msgVar
  Args.push_back(CallInfo.CI->getArgOperand(2));                    // msgVar
  if (HasDst)
    Args.push_back(CGF.Builder.CreateDefaultAlignedLoad(Arg0)); // oldDst

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, Args);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  if (HasDst) {
    NewCI->takeName(CallInfo.CI);
    CGF.Builder.CreateDefaultAlignedStore(NewCI, Arg0);
  }

  CallInfo.CI->eraseFromParent();
  return NewCI;
}

/// template <typename U1, int N1, typename U2, int N2,
///           typename U3, int N3, int N = 16>
/// void cm_raw_send(vector_ref<U1, N1> rspVar, vector<U2, N2> msgVar,
/// vector<U3, N3> msgVar2,
///                  uint exDesc, uint msgDesc, uchar execSize, uchar sfid,
///                  uchar numSrc0, uchar numSrc1, uchar numDst,
///                  uchar isEOT = 0, uchar isSendc = 0,
///                  vector<ushort, N> mask = 1);
llvm::Value *CGCMRuntime::HandleBuiltinRawSendImpl(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  uint32_t ExecSize = 0;
  getConstantValue(*this, CGF, ExecSize, CallInfo.CI->getArgOperand(5),
                   CallInfo.CE->getArg(5));

  uint32_t SFID = 0;
  getConstantValue(*this, CGF, SFID, CallInfo.CI->getArgOperand(6),
                   CallInfo.CE->getArg(6));

  uint32_t NumSrc0 = 0;
  getConstantValue(*this, CGF, NumSrc0, CallInfo.CI->getArgOperand(7),
                   CallInfo.CE->getArg(7));

  uint32_t NumSrc1 = 0;
  getConstantValue(*this, CGF, NumSrc1, CallInfo.CI->getArgOperand(8),
                   CallInfo.CE->getArg(8));

  uint32_t NumDst = 0;
  getConstantValue(*this, CGF, NumDst, CallInfo.CI->getArgOperand(9),
                   CallInfo.CE->getArg(9));

  uint32_t IsEOT = 0;
  getConstantValue(*this, CGF, IsEOT, CallInfo.CI->getArgOperand(10),
                   CallInfo.CE->getArg(10));

  uint32_t IsSendc = 0;
  getConstantValue(*this, CGF, IsSendc, CallInfo.CI->getArgOperand(11),
                   CallInfo.CE->getArg(11));

  // Types for overloading.
  llvm::Value *Dst = CallInfo.CI->getArgOperand(0);
  llvm::Value *Src0 = CallInfo.CI->getArgOperand(1);
  llvm::Value *Src1 = CallInfo.CI->getArgOperand(2);
  llvm::Value *Pred = CallInfo.CI->getArgOperand(12);

  // A dummy parameter is not a pointer.
  bool HasDst = Dst->getType()->isPointerTy();

  bool HasSrc1 = true;
  llvm::ConstantInt *Src1Const = dyn_cast_or_null<llvm::ConstantInt>(Src1);
  if (Src1Const && (Src1Const->getZExtValue() == 0)) {
    HasSrc1 = false;
    assert(NumSrc1 == 0);
  }

  llvm::Function *Fn = nullptr;
  llvm::FunctionType *FTy = nullptr;
  SmallVector<llvm::Type *, 8> Tys;
  SmallVector<llvm::Value *, 12> Args;
  auto *PredTy = cast<llvm::FixedVectorType>(Pred->getType());

  if (HasDst && HasSrc1) {
    Tys.push_back(Dst->getType()->getPointerElementType()); // rspVar
    auto *MaskTy =
        getMaskType(CallInfo.CGF->getLLVMContext(), PredTy->getNumElements());
    Tys.push_back(MaskTy);          // Predicate
    Tys.push_back(Src0->getType()); // msgVar1
    Tys.push_back(Src1->getType()); // msgVar2

    unsigned ID = llvm::GenXIntrinsic::genx_raw_sends2;
    Fn = getGenXIntrinsic(ID, Tys);
    FTy = Fn->getFunctionType();

    Args.push_back(llvm::ConstantInt::get(FTy->getParamType(0),
                                          ((IsEOT & 0x1) << 1) |
                                              (IsSendc & 0x1))); // modifier
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(1), ExecSize)); // execSize
    Pred = CallInfo.CGF->Builder.CreateTrunc(Pred, MaskTy);
    if (auto Inst = dyn_cast<llvm::Instruction>(Pred))
      Inst->setDebugLoc(CallInfo.CI->getDebugLoc());
    Args.push_back(Pred); // Predicate
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(3), NumSrc0)); // numSrc0
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(4), NumSrc1)); // numSrc1
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(5), NumDst)); // numDst
    Args.push_back(llvm::ConstantInt::get(FTy->getParamType(6), SFID)); // SFID
    Args.push_back(CallInfo.CI->getArgOperand(3));             // ExMsgDesc
    Args.push_back(CallInfo.CI->getArgOperand(4));             // msgDesc
    Args.push_back(CallInfo.CI->getArgOperand(1));             // msgVar
    Args.push_back(CallInfo.CI->getArgOperand(2));             // msgVar
    Args.push_back(CGF.Builder.CreateDefaultAlignedLoad(Dst)); // dst
  } else if (!HasDst && HasSrc1) {
    llvm::Type *MaskTy = getMaskType(CallInfo.CGF->getLLVMContext(),
                                     PredTy->getNumElements());
    Tys.push_back(MaskTy);          // Predicate
    Tys.push_back(Src0->getType()); // msgVar1
    Tys.push_back(Src1->getType()); // msgVar2

    unsigned ID = llvm::GenXIntrinsic::genx_raw_sends2_noresult;
    Fn = getGenXIntrinsic(ID, Tys);
    FTy = Fn->getFunctionType();

    Args.push_back(llvm::ConstantInt::get(FTy->getParamType(0),
                                          ((IsEOT & 0x1) << 1) |
                                              (IsSendc & 0x1))); // modifier
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(1), ExecSize)); // execSize
    Pred = CallInfo.CGF->Builder.CreateTrunc(Pred, MaskTy);
    if (auto Inst = dyn_cast<llvm::Instruction>(Pred))
      Inst->setDebugLoc(CallInfo.CI->getDebugLoc());
    Args.push_back(Pred); // Predicate
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(3), NumSrc0)); // numSrc0
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(4), NumSrc1)); // numSrc1
    Args.push_back(llvm::ConstantInt::get(FTy->getParamType(5), SFID)); // SFID
    Args.push_back(CallInfo.CI->getArgOperand(3)); // ExMsgDesc
    Args.push_back(CallInfo.CI->getArgOperand(4)); // msgDesc
    Args.push_back(CallInfo.CI->getArgOperand(1)); // msgVar
    Args.push_back(CallInfo.CI->getArgOperand(2)); // msgVar
  } else if (HasDst && !HasSrc1) {
    Tys.push_back(Dst->getType()->getPointerElementType()); // rspVar
    llvm::Type *MaskTy = getMaskType(CallInfo.CGF->getLLVMContext(),
                                     PredTy->getNumElements());
    Tys.push_back(MaskTy);          // Predicate
    Tys.push_back(Src0->getType()); // msgVar1

    unsigned ID = llvm::GenXIntrinsic::genx_raw_send2;
    Fn = getGenXIntrinsic(ID, Tys);
    FTy = Fn->getFunctionType();

    Args.push_back(llvm::ConstantInt::get(FTy->getParamType(0),
                                          ((IsEOT & 0x1) << 1) |
                                              (IsSendc & 0x1))); // modifier
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(1), ExecSize)); // execSize
    Pred = CallInfo.CGF->Builder.CreateTrunc(Pred, MaskTy);
    if (auto Inst = dyn_cast<llvm::Instruction>(Pred))
      Inst->setDebugLoc(CallInfo.CI->getDebugLoc());
    Args.push_back(Pred); // Predicate
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(3), NumSrc0)); // numSrc0
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(4), NumDst)); // numDst
    Args.push_back(llvm::ConstantInt::get(FTy->getParamType(5), SFID)); // SFID
    Args.push_back(CallInfo.CI->getArgOperand(3));             // ExMsgDesc
    Args.push_back(CallInfo.CI->getArgOperand(4));             // msgDesc
    Args.push_back(CallInfo.CI->getArgOperand(1));             // msgVar
    Args.push_back(CGF.Builder.CreateDefaultAlignedLoad(Dst)); // dst
  } else {
    llvm::Type *MaskTy = getMaskType(CallInfo.CGF->getLLVMContext(),
                                     PredTy->getNumElements());
    Tys.push_back(MaskTy);          // Predicate
    Tys.push_back(Src0->getType()); // msgVar1

    unsigned ID = llvm::GenXIntrinsic::genx_raw_send2_noresult;
    Fn = getGenXIntrinsic(ID, Tys);
    FTy = Fn->getFunctionType();

    Args.push_back(llvm::ConstantInt::get(FTy->getParamType(0),
                                          ((IsEOT & 0x1) << 1) |
                                              (IsSendc & 0x1))); // modifier
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(1), ExecSize)); // execSize
    Pred = CallInfo.CGF->Builder.CreateTrunc(Pred, MaskTy);
    if (auto Inst = dyn_cast<llvm::Instruction>(Pred))
      Inst->setDebugLoc(CallInfo.CI->getDebugLoc());
    Args.push_back(Pred); // Predicate
    Args.push_back(
        llvm::ConstantInt::get(FTy->getParamType(3), NumSrc0)); // numSrc0
    Args.push_back(llvm::ConstantInt::get(FTy->getParamType(4), SFID)); // SFID
    Args.push_back(CallInfo.CI->getArgOperand(3)); // ExMsgDesc
    Args.push_back(CallInfo.CI->getArgOperand(4)); // msgDesc
    Args.push_back(CallInfo.CI->getArgOperand(1)); // msgVar
  }

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, Args);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  if (HasDst) {
    NewCI->takeName(CallInfo.CI);
    CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);
  }

  CallInfo.CI->eraseFromParent();
  return NewCI;
}

/// template <typename T> vector<uint, 8> cm_get_r0();
///
llvm::Value *CGCMRuntime::HandleBuiltinGetR0Impl(CMCallInfo &CallInfo) {
  unsigned ID = llvm::GenXIntrinsic::genx_r0;
  llvm::Function *Fn = getGenXIntrinsic(ID, CallInfo.CI->getType());
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(Fn);
  NewCI->takeName(CallInfo.CI);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  assert(NewCI->getType() == CallInfo.CI->getType());

  CallInfo.CI->eraseFromParent();
  return NewCI;
}

/// template <typename T> vector<uint, 4> cm_get_sr0();
///
llvm::Value *CGCMRuntime::HandleBuiltinGetSR0Impl(CMCallInfo &CallInfo) {
  unsigned ID = llvm::GenXIntrinsic::genx_sr0;
  llvm::Function *Fn = getGenXIntrinsic(ID, CallInfo.CI->getType());
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(Fn);
  NewCI->takeName(CallInfo.CI);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  assert(NewCI->getType() == CallInfo.CI->getType());

  CallInfo.CI->eraseFromParent();
  return NewCI;
}

llvm::Value *CGCMRuntime::HandleBuiltinUnmaskBeginImpl(CMCallInfo &CallInfo) {
  unsigned ID = llvm::GenXIntrinsic::genx_unmask_begin;
  llvm::Function *Fn = getGenXIntrinsic(ID);
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(Fn);
  NewCI->takeName(CallInfo.CI);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  assert(NewCI->getType() == CallInfo.CI->getType());

  CallInfo.CI->eraseFromParent();
  return NewCI;
}

llvm::Value *CGCMRuntime::HandleBuiltinUnmaskEndImpl(CMCallInfo &CallInfo) {
  unsigned ID = llvm::GenXIntrinsic::genx_unmask_end;
  llvm::Function *Fn = getGenXIntrinsic(ID);
  llvm::Value *Mask = CallInfo.CI->getArgOperand(0);
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(Fn, Mask);
  NewCI->takeName(CallInfo.CI);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  assert(NewCI->getType() == CallInfo.CI->getType());
  CallInfo.CI->eraseFromParent();
  return NewCI;
}

/// template <typename T> T cm_get_value(T index);
///
llvm::Value *CGCMRuntime::HandleBuiltinGetValueImpl(CMCallInfo &CallInfo) {
  assert(CallInfo.CE->getArg(0)->getType()->isCMSurfaceIndexType() ||
         CallInfo.CE->getArg(0)->getType()->isCMSamplerIndexType() ||
         CallInfo.CE->getArg(0)->getType()->isCMVmeIndexType());

  // Nothing to emit. Just return its argument value.
  llvm::Value *Arg = CallInfo.CI->getArgOperand(0);
  CallInfo.CI->eraseFromParent();
  return Arg;
}

/// template <typename T, int N1, int N2>
/// typename std::enable_if<details::is_fp_or_dword_type<T>::value &&
///                         (N2 == 8 || N2 == 16)>::type
/// void read_untyped(SurfaceIndex surfIndex, ChannelMaskType channelMask,
///                   matrix_ref<T, N1, N2> m, vector<uint, N2> u);
///
/// template <typename T, int N1, int N2>
/// typename std::enable_if<details::is_fp_or_dword_type<T>::value &&
///                         (N2 == 8 || N2 == 16)>::type
/// write_untyped(SurfaceIndex surfIndex, ChannelMaskType channelMask,
///               matrix<T, N1, N2> m, vector<uint, N2> u);
///
void CGCMRuntime::HandleBuiltinReadWriteUntypedImpl(CMCallInfo &CallInfo,
                                                    CMBuiltinKind Kind) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  // Check whether the channel mask is a compile time constant.
  // If not issue an error and return.
  uint32_t Mask = 0;
  if (!getConstantValue(*this, CGF, Mask, CallInfo.CI->getArgOperand(1),
                        CallInfo.CE->getArg(1)))
    return;

  // Ensure that the channel mask is from 1 to 15.
  if (Mask < ChannelMaskType::CM_R_ENABLE ||
      Mask > ChannelMaskType::CM_ABGR_ENABLE)
    return Error(CallInfo.CE->getArg(1)->getExprLoc(),
                 "invalid channel mask kind");

  // Check whether matrix height is no less than the number of colors enabled.
  QualType MT = CallInfo.CE->getArg(2)->getType();
  unsigned N1 = MT->castAs<CMMatrixType>()->getNumRows();

  if (N1 < getNumberOfColors(static_cast<ChannelMaskType>(Mask))) {
    if (Kind == CMBK_read_untyped)
      return Error(CallInfo.CE->getArg(2)->getExprLoc(),
                   "untyped surface read destination size does not match "
                   "number of elements to be read");
    else
      return Error(CallInfo.CE->getArg(2)->getExprLoc(),
                   "untyped surface write source size does not match number of "
                   "elements to be written");
  }

  // Use scaled message for any platform since scale is 0.
  bool IsRead = Kind == CMBK_read_untyped;

  // Compute byte offsets
  llvm::Value *EltOffset = CallInfo.CI->getArgOperand(3);
  EltOffset = CGF.Builder.CreateShl(EltOffset, 2);
  llvm::Value *Arg2 = CallInfo.CI->getArgOperand(2);

  if (IsRead) {
    auto NewCI = EmitGatherScaled(
        CGF, llvm::GenXIntrinsic::genx_gather4_masked_scaled2,
        llvm::APInt(32, Mask),                     // channel mask
        0,                                         // scale
        CallInfo.CI->getArgOperand(0),             // surface index
        CGF.Builder.getInt32(0),                   // global offset in bytes
        EltOffset,                                 // element offset in bytes
        CGF.Builder.CreateDefaultAlignedLoad(Arg2) // old value data
    );
    NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
    CallInfo.CI->eraseFromParent();
    CGF.Builder.CreateDefaultAlignedStore(NewCI, Arg2);
  } else {
    auto NewCI =
        EmitScatterScaled(CGF, llvm::GenXIntrinsic::genx_scatter4_scaled,
                          llvm::APInt(32, Mask),         // channel mask
                          0,                             // scale
                          CallInfo.CI->getArgOperand(0), // surface index
                          CGF.Builder.getInt32(0), // global offset in bytes
                          EltOffset,               // element offset in bytes
                          Arg2                     // data to write
        );
    NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
    CallInfo.CI->eraseFromParent();
  }
}

/// template <typename T, int N1, int N2>
/// typename std::enable_if<details::is_fp_or_dword_type<T>::value &&
///                         (N2 == 8 || N2 == 16)>::type
/// read_typed(SurfaceIndex surfIndex, ChannelMaskType channelMask,
///            matrix_ref<T, N1, N2> m, vector<uint, N2> u,
///            vector<uint, N2> v = 0, vector<uint, N2> r = 0);
///
void CGCMRuntime::HandleBuiltinReadTypedImpl(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  // Check whether the channel mask is a compile time constant.
  // If not issue an error and return.
  uint32_t Mask = 0;
  if (!getConstantValue(*this, CGF, Mask, CallInfo.CI->getArgOperand(1),
                        CallInfo.CE->getArg(1)))
    return;

  // Ensure that the channel mask is from 1 to 15.
  if (Mask < ChannelMaskType::CM_R_ENABLE ||
      Mask > ChannelMaskType::CM_ABGR_ENABLE)
    return Error(CallInfo.CE->getArg(1)->getExprLoc(),
                 "invalid channel mask kind");

  // Check whether matrix height is no less than the number of colors enabled.
  QualType MT = CallInfo.CE->getArg(2)->getType();
  unsigned N1 = 0, N2 = 0;
  if (MT->isCMMatrixType()) {
    N1 = MT->castAs<CMMatrixType>()->getNumRows();
    N2 = MT->castAs<CMMatrixType>()->getNumColumns();
  } else if (MT->isCMVectorType()) {
    N1 = 1;
    N2 = MT->castAs<CMVectorType>()->getNumElements();
  }

  if (N1 < getNumberOfColors(static_cast<ChannelMaskType>(Mask)))
    return Error(CallInfo.CE->getArg(2)->getExprLoc(),
                 "typed surface read destination size does not match number of "
                 "elements to be read");

  llvm::Value *Arg2 = CallInfo.CI->getArgOperand(2);
  llvm::Type *Arg2Ty = Arg2->getType()->getPointerElementType();

  // Types for overloading.
  llvm::Type *Tys[] = {
      Arg2Ty,                                  // data type
      getMaskType(CGF.getLLVMContext(), N2),   // predicate
      CallInfo.CI->getArgOperand(3)->getType() // U pixel
  };

  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_gather4_typed, Tys);
  llvm::FunctionType *FTy = Fn->getFunctionType();
  llvm::Value *Args[] = {
      llvm::ConstantInt::get(FTy->getParamType(0),
                             Mask & 0xF), // channel mask, DO NOT INVERT.
      llvm::Constant::getAllOnesValue(FTy->getParamType(1)), // predicate
      CallInfo.CI->getArgOperand(0),                         // surface index
      CallInfo.CI->getArgOperand(3),                         // U pixel
      CallInfo.CI->getArgOperand(4),                         // V pixel
      CallInfo.CI->getArgOperand(5),                         // V pixel
      CGF.Builder.CreateDefaultAlignedLoad(Arg2)             // old value
  };

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, Args, "call");
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Arg2);

  CallInfo.CI->eraseFromParent();
}

/// template <typename T, int N1, int N2>
/// typename std::enable_if<details::is_fp_or_dword_type<T>::value &&
///                         (N2 == 8 || N2 == 16)>::type
/// write_typed(SurfaceIndex surfIndex, ChannelMaskType channelMask,
///             matrix<T, N1, N2> m, vector<uint, N2> u, vector<uint, N2> v = 0,
///             vector<uint, N2> r = 0);
///
void CGCMRuntime::HandleBuiltinWriteTypedImpl(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  // Check whether the channel mask is a compile time constant.
  uint32_t Mask = 0;
  if (!getConstantValue(*this, CGF, Mask, CallInfo.CI->getArgOperand(1),
                        CallInfo.CE->getArg(1)))
    return;
  // Ensure that the channel mask is from 1 to 15.
  if (Mask < ChannelMaskType::CM_R_ENABLE ||
      Mask > ChannelMaskType::CM_ABGR_ENABLE)
    return Error(CallInfo.CE->getArg(1)->getExprLoc(),
                 "invalid channel mask kind");

  // Check whether matrix height is no less than the number of colors enabled.
  QualType MT = CallInfo.CE->getArg(2)->getType();
  unsigned N1 = MT->castAs<CMMatrixType>()->getNumRows();
  unsigned N2 = MT->castAs<CMMatrixType>()->getNumColumns();

  if (N1 < getNumberOfColors(static_cast<ChannelMaskType>(Mask)))
    return Error(CallInfo.CE->getArg(2)->getExprLoc(),
                 "typed surface write source size does not match number of "
                 "elements to be written");

  // Types for overloading.
  llvm::Value *Arg2 = CallInfo.CI->getArgOperand(2);
  llvm::Type *Tys[] = {
      getMaskType(CGF.getLLVMContext(), N2),    // predicate
      CallInfo.CI->getArgOperand(3)->getType(), // U pixel
      Arg2->getType()                           // data type
  };
  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_scatter4_typed, Tys);
  llvm::FunctionType *FTy = Fn->getFunctionType();
  llvm::Value *Args[] = {
      llvm::ConstantInt::get(FTy->getParamType(0),
                             Mask & 0xF), // channel mask, DO NOT INVERT.
      llvm::Constant::getAllOnesValue(FTy->getParamType(1)), // predicate
      CallInfo.CI->getArgOperand(0),                         // surface index
      CallInfo.CI->getArgOperand(3),                         // U pixel
      CallInfo.CI->getArgOperand(4),                         // V pixel
      CallInfo.CI->getArgOperand(5),                         // V pixel
      Arg2                                                   // value to write
  };

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, Args);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());

  CallInfo.CI->eraseFromParent();
}

namespace {

// DO NOT MODIFY THE FOLLOWING ENCODING.
typedef enum _SLM_ChannelMaskType_ {
  SLM_R_ENABLE = 14,
  SLM_G_ENABLE = 13,
  SLM_GR_ENABLE = 12,
  SLM_B_ENABLE = 11,
  SLM_BR_ENABLE = 10,
  SLM_BG_ENABLE = 9,
  SLM_BGR_ENABLE = 8,
  SLM_A_ENABLE = 7,
  SLM_AR_ENABLE = 6,
  SLM_AG_ENABLE = 5,
  SLM_AGR_ENABLE = 4,
  SLM_AB_ENABLE = 3,
  SLM_ABR_ENABLE = 2,
  SLM_ABG_ENABLE = 1,
  SLM_ABGR_ENABLE = 0
} SLM_ChannelMaskType;

/// Compute the number of enabled SLM mask channels.
inline unsigned getNumOfChannels(uint64_t m) {
  return 4 -
         (((m >> 3) & 0x1) + ((m >> 2) & 0x1) + ((m >> 1) & 0x1) + (m & 0x1));
}

inline bool isValidSLMChannelMask(int64_t Val) {
  return Val >= SLM_ABGR_ENABLE && Val <= SLM_R_ENABLE;
}

} // namespace

/// template <typename T, int N, int M>
/// typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
/// cm_slm_read4(uint slmBuffer, vector<uint, N> vAddr, vector_ref<T, M> vDst,
///              SLM_ChannelMaskType mask);
///
/// cm_slm_read4(uint slmBuffer, vector<ushort, N> vAddr, vector_ref<T, M> vDst,
///              SLM_ChannelMaskType mask);
///
void CGCMRuntime::HandleBuiltinSLMRead4(CMCallInfo &CallInfo,
                                        bool IsDwordAddr) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  const Expr *Arg3 = CallInfo.CE->getArg(3);
  Expr::EvalResult MaskResult;
  if (!Arg3->EvaluateAsInt(MaskResult, CGF.getContext())) {
    Error(Arg3->getExprLoc(), "channel mask must be constant");
    return;
  } else if (!isValidSLMChannelMask(MaskResult.Val.getInt().getSExtValue())) {
    Error(Arg3->getExprLoc(), "channel mask is invalid");
    return;
  }

  llvm::APSInt Mask = MaskResult.Val.getInt();
  unsigned NumChannels = getNumOfChannels(Mask.getZExtValue());
  unsigned N = getIntegralValue(CallInfo.CE->getDirectCallee(), 1);
  unsigned M = getIntegralValue(CallInfo.CE->getDirectCallee(), 2);
  if (M != N * NumChannels) {
    Error(CallInfo.CE->getArg(2)->getExprLoc(),
          "destination size does not match number of elements to be read");
    return;
  }
  // Flip the bits 0, means off and 1 means on.
  // FIXME: Flip the mask encoding, need to update emulation code.
  Mask = (~Mask).trunc(4).zext(32);

  llvm::Value *Dst = CallInfo.CI->getArgOperand(2);
  assert(isa<llvm::PointerType>(Dst->getType()));
  llvm::Value *OldValue = CGF.Builder.CreateDefaultAlignedLoad(Dst);
  llvm::Value *Offset = CallInfo.CI->getArgOperand(1);

  // Convert the offest type to UD if it is not.
  auto *OffsetTy = cast<llvm::FixedVectorType>(Offset->getType());
  if (OffsetTy->getElementType() != CGF.Int32Ty) {
    llvm::Type *Ty = llvm::FixedVectorType::get(CGF.Int32Ty, N);
    Offset = CGF.Builder.CreateZExt(Offset, Ty);
  }

  if (IsDwordAddr) {
    // Convert element offset from by elements to by bytes.
    unsigned TySizeInBytes = OldValue->getType()->getScalarSizeInBits() / 8;
    Offset = CGF.Builder.CreateMul(
        Offset, llvm::ConstantInt::get(Offset->getType(), TySizeInBytes));
  }

  // Use scaled message for any platform since scale is 0.
  auto NewCI =
      EmitGatherScaled(CGF, llvm::GenXIntrinsic::genx_gather4_masked_scaled2,
                       Mask,                          // channel mask
                       0,                             // scale
                       getSLMSurfaceIndex(CGF),       // SLM surface index
                       CallInfo.CI->getArgOperand(0), // global offset in bytes
                       Offset,                        // element offset in bytes
                       OldValue                       // old value of data read
      );
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  CallInfo.CI->eraseFromParent();
}

/// template <typename T, int N, int M>
/// typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
/// cm_slm_write4(uint slmBuffer, vector<uint, N> vAddr, vector<T, M> vSrc,
///               SLM_ChannelMaskType mask);
///
/// cm_slm_write4(uint slmBuffer, vector<ushort, N> vAddr, vector<T, M> vSrc,
///               SLM_ChannelMaskType mask);
///
void CGCMRuntime::HandleBuiltinSLMWrite4(CMCallInfo &CallInfo,
                                         bool IsDwordAddr) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  const Expr *Arg3 = CallInfo.CE->getArg(3);
  Expr::EvalResult MaskResult;
  if (!Arg3->EvaluateAsInt(MaskResult, CGF.getContext())) {
    Error(Arg3->getExprLoc(), "channel mask must be constant");
    return;
  } else if (!isValidSLMChannelMask(MaskResult.Val.getInt().getSExtValue())) {
    Error(Arg3->getExprLoc(), "channel mask is invalid");
    return;
  }

  llvm::APSInt Mask = MaskResult.Val.getInt();
  unsigned NumChannels = getNumOfChannels(Mask.getZExtValue());
  unsigned N = getIntegralValue(CallInfo.CE->getDirectCallee(), 1);
  unsigned M = getIntegralValue(CallInfo.CE->getDirectCallee(), 2);
  if (M != N * NumChannels) {
    Error(CallInfo.CE->getArg(2)->getExprLoc(),
          "destination size does not match number of elements to write");
    return;
  }
  // Flip the bits 0, means off and 1 means on.
  // FIXME: change the mask encoding, need to update emulation code.
  Mask = (~Mask).trunc(4).zext(32);

  llvm::Value *Offset = CallInfo.CI->getArgOperand(1);
  llvm::Value *Src = CallInfo.CI->getArgOperand(2);

  // Convert the offest type to UD if it is not.
  auto *OffsetTy = cast<llvm::FixedVectorType>(Offset->getType());
  if (OffsetTy->getElementType() != CGF.Int32Ty) {
    llvm::Type *Ty = llvm::FixedVectorType::get(CGF.Int32Ty, N);
    Offset = CGF.Builder.CreateZExt(Offset, Ty);
  }

  if (IsDwordAddr) {
    // Convert element offset from by elements to by bytes.
    unsigned TySizeInBytes = Src->getType()->getScalarSizeInBits() / 8;
    Offset = CGF.Builder.CreateMul(
        Offset, llvm::ConstantInt::get(Offset->getType(), TySizeInBytes));
  }

  // Use scaled message for any platform since scale is 0.
  auto NewCI =
      EmitScatterScaled(CGF, llvm::GenXIntrinsic::genx_scatter4_scaled,
                        Mask,                          // channel mask
                        0,                             // scale
                        getSLMSurfaceIndex(CGF),       // SLM surface index
                        CallInfo.CI->getArgOperand(0), // global offset in bytes
                        Offset, // element offset in bytes
                        Src);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());

  CallInfo.CI->eraseFromParent();
}

namespace {

enum AtomicCheckResult {
  AR_Valid,      // valid
  AR_Invalid,    // invalid, for an unspecificed reason
  AR_NotZeroSrc, // invalid, expect no source operand
  AR_NotOneSrc,  // invalid, expect one source operand
  AR_NotTwoSrc,  // invalid, expect two source operands
  AR_NotD,       // invalid, expect D type
  AR_NotUD,      // invalid, expect UD type
  AR_NotFloat    // invalid, expect float type
};

AtomicCheckResult checkSLMAtomicOp(CmAtomicOpType Op, unsigned NumSrc) {
  switch (Op) {
  case ATOMIC_CMPXCHG:
  case ATOMIC_FCMPWR:
    return (NumSrc == 2) ? AR_Valid : AR_NotTwoSrc;
  case ATOMIC_INC:
  case ATOMIC_DEC:
    return (NumSrc == 0) ? AR_Valid : AR_NotZeroSrc;
  case ATOMIC_ADD:
  case ATOMIC_SUB:
  case ATOMIC_MIN:
  case ATOMIC_MAX:
  case ATOMIC_XCHG:
  case ATOMIC_AND:
  case ATOMIC_OR:
  case ATOMIC_XOR:
  case ATOMIC_MINSINT:
  case ATOMIC_MAXSINT:
  case ATOMIC_FMAX:
  case ATOMIC_FMIN:
  case ATOMIC_FADD:
  case ATOMIC_FSUB:
    return (NumSrc == 1) ? AR_Valid : AR_NotOneSrc;
  }

  return AR_Invalid;
}

AtomicCheckResult checkSLMAtomicOperands(CmAtomicOpType Op, QualType Ty) {
  switch (Op) {
  case ATOMIC_CMPXCHG:
  case ATOMIC_INC:
  case ATOMIC_DEC:
  case ATOMIC_ADD:
  case ATOMIC_SUB:
  case ATOMIC_MIN:
  case ATOMIC_MAX:
  case ATOMIC_XCHG:
  case ATOMIC_AND:
  case ATOMIC_OR:
  case ATOMIC_XOR:
    return Ty->isUnsignedIntegerType() ? AR_Valid : AR_NotUD;
  case ATOMIC_MINSINT:
  case ATOMIC_MAXSINT:
    return Ty->isSignedIntegerType() ? AR_Valid : AR_NotD;
  case ATOMIC_FMAX:
  case ATOMIC_FMIN:
  case ATOMIC_FCMPWR:
  case ATOMIC_FADD:
  case ATOMIC_FSUB:
    return Ty->isFloatingType() ? AR_Valid : AR_NotFloat;
  }

  return AR_Invalid;
}

} // namespace

/// template <typename T, int N>
/// typename std::enable_if<(N == 8 || N == 16) && sizeof(T) = 4, void>::type
/// cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
///               vector_ref<T, N> vDst, vector<T, N> vSrc0, vector<T, N>
///               vSrc1);
///
/// template <int N>
/// typename std::enable_if<(N == 8 || N == 16) && sizeof(T) = 4, void>::type
/// cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
///              int dummy, vector<T, N> vSrc0, vector<T, N> vSrc1);
///
/// template <typename T, int N>
/// typename std::enable_if<(N == 8 || N == 16) && sizeof(T) = 4, void>::type
/// cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
///               vector_ref<T, N> vDst);
///
/// template <int N>
/// typename std::enable_if<(N == 8 || N == 16) && sizeof(T) = 4, void>::type
/// cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
///               int dummy);
///
/// template <typename T, int N>
/// typename std::enable_if<(N == 8 || N == 16) && sizeof(T) = 4, void>::type
/// cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
///               vector_ref<T, N> vDst, vector<T, N> vSrc0);
///
/// template <int N>
/// typename std::enable_if<(N == 8 || N == 16) && sizeof(T) = 4, void>::type
/// cm_slm_atomic(uint slmBuffer, CmAtomicOpType op, vector<ushort, N> vAddr,
///               int dummy, vector<T, N> vSrc0);
void CGCMRuntime::HandleBuiltinSLMAtomic(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  // Check the atomic op kind.
  const Expr *Arg1 = CallInfo.CE->getArg(1);
  Expr::EvalResult OpValResult;
  CmAtomicOpType OpKind;
  if (!Arg1->EvaluateAsInt(OpValResult, CGF.getContext())) {
    Error(Arg1->getExprLoc(), "compile-time atomic op expected");
    return;
  } else {
    llvm::APSInt OpVal = OpValResult.Val.getInt();
    OpKind = static_cast<CmAtomicOpType>(OpVal.getZExtValue());
    switch (checkSLMAtomicOp(OpKind, CallInfo.CE->getNumArgs() - 4)) {
    case AR_NotZeroSrc:
      return Error(Arg1->getExprLoc(), "no source operand expected");
    case AR_NotOneSrc:
      return Error(Arg1->getExprLoc(), "no source operand expected");
    case AR_NotTwoSrc:
      return Error(Arg1->getExprLoc(), "two source operands expected");
    case AR_Invalid:
      return Error(Arg1->getExprLoc(), "invalid atomic op");
    default:
      break;
    }
  }

  // Check source and dst types.
  llvm::Value *Dst = CallInfo.CI->getArgOperand(3);
  QualType DstType = CallInfo.CE->getArg(3)->getType();
  AtomicCheckResult CheckResult = AR_Valid;

  // Is the result to be returned.
  bool NeedResult = true;
  SourceLocation Loc;
  if (DstType->isCMReferenceType()) {
    QualType T = DstType->getCMVectorMatrixElementType();
    CheckResult = checkSLMAtomicOperands(OpKind, T);
    Loc = CallInfo.CE->getArg(3)->getExprLoc();
  } else {
    NeedResult = false;
    if (OpKind != ATOMIC_INC && OpKind != ATOMIC_DEC) {
      QualType T = CallInfo.CE->getArg(4)->getType();
      T = T->getCMVectorMatrixElementType();
      CheckResult = checkSLMAtomicOperands(OpKind, T);
      Loc = CallInfo.CE->getArg(4)->getExprLoc();
    }
  }

  switch (CheckResult) {
  default:
    break;
  case AR_NotD:
    return Error(Loc, "expect signed int type");
  case AR_NotUD:
    return Error(Loc, "expect unsigned int type");
  case AR_NotFloat:
    return Error(Loc, "expect float type");
  }

  // At this point, the atomic call is valid. We emit the atomic send message.
  unsigned ID = getAtomicIntrinsicID(OpKind);

  llvm::Value *Arg2 = CallInfo.CI->getArgOperand(2);
  auto *Arg2Ty = cast<llvm::FixedVectorType>(Arg2->getType());
  unsigned N = Arg2Ty->getNumElements();

  // Types for overloading
  SmallVector<llvm::Type *, 8> Tys;
  llvm::LLVMContext &C = CGF.getLLVMContext();

  // Return type
  if (ID == llvm::GenXIntrinsic::genx_dword_atomic_fmin ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic_fmax ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic2_fmin ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic2_fmax ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic_fadd ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic_fsub ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic2_fadd ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic2_fsub ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic_fcmpwr ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic2_fcmpwr)
    Tys.push_back(llvm::FixedVectorType::get(CGF.FloatTy, N));
  else
    Tys.push_back(llvm::FixedVectorType::get(CGF.Int32Ty, N));

  // Predicate type
  Tys.push_back(getMaskType(C, N));

  // Offset type (selectively mangled)
  llvm::Type *OffsetTy = llvm::FixedVectorType::get(CGF.Int32Ty, N);
  if (ID != llvm::GenXIntrinsic::genx_dword_atomic_cmpxchg &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic_inc &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic_dec &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_cmpxchg &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_inc &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_dec)
    Tys.push_back(OffsetTy);

  // Collect arguments.
  llvm::Function *Fn = getGenXIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();
  CGBuilderTy &Builder = CallInfo.CGF->Builder;

  SmallVector<llvm::Value *, 8> Args;

  // Predicate
  Args.push_back(llvm::Constant::getAllOnesValue(FnTy->getParamType(0)));

  // SurfaceIndex
  Args.push_back(getSLMSurfaceIndex(CGF));

  // Convert the offset type to UD if it is not.
  llvm::Value *Offset = Arg2;
  if (Offset->getType() != OffsetTy)
    Offset = CGF.Builder.CreateZExt(Offset, OffsetTy);

  // Convert element offset from by elements to by bytes.
  unsigned TySizeInBytes = Tys[0]->getScalarSizeInBits() / 8;
  Offset = CGF.Builder.CreateMul(
      Offset, llvm::ConstantInt::get(Offset->getType(), TySizeInBytes));

  // Add slmBuffer into element offset.
  llvm::Value *Splat =
      CGF.Builder.CreateVectorSplat(N, CallInfo.CI->getArgOperand(0));
  Offset = CGF.Builder.CreateAdd(Offset, Splat);
  Args.push_back(Offset);

  // INC or DEC does not have any source.
  if (ID != llvm::GenXIntrinsic::genx_dword_atomic_inc &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic_dec &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_inc &&
      ID != llvm::GenXIntrinsic::genx_dword_atomic2_dec)
    Args.push_back(CallInfo.CI->getArgOperand(4));

  // cmpxchg or fcmpwr takes one extra source.
  if (ID == llvm::GenXIntrinsic::genx_dword_atomic_cmpxchg ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic_fcmpwr ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic2_cmpxchg ||
      ID == llvm::GenXIntrinsic::genx_dword_atomic2_fcmpwr)
    Args.push_back(CallInfo.CI->getArgOperand(5));

  // Call genx intrinsic.
  llvm::CallInst *NewCI = Builder.CreateCall(Fn, Args);
  NewCI->takeName(CallInfo.CI);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());

  if (NeedResult)
    CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  CallInfo.CI->eraseFromParent();
}

/// template <typename T, int N, int M>
/// typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
/// cm_ptr_read4(T* ptr, vector<ptrdiff_t, N> vOffset, vector_ref<T, M> vDst,
///              ChannelMaskType mask);
///
/// template <typename T, int N, int M>
/// typename std::enable_if<(N == 8 || N == 16 || N == 32) && (sizeof(T) ==
/// 4)>::type cm_svm_gather4_scaled(vector<uint, N> vOffset,
///                       vector_ref<T, M> vDst,
///                       ChannelMaskType mask)
void CGCMRuntime::HandleBuiltinSVMRead4Impl(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  const Expr *Arg3 = CallInfo.CE->getArg(3);
  Expr::EvalResult MaskResult;
  if (!Arg3->EvaluateAsInt(MaskResult, CGF.getContext())) {
    Error(Arg3->getExprLoc(), "channel mask must be constant");
    return;
  }

  llvm::APSInt Mask = MaskResult.Val.getInt();
  unsigned NumChannels =
      getNumberOfColors(static_cast<ChannelMaskType>(Mask.getZExtValue()));
  if (!NumChannels) {
    Error(Arg3->getExprLoc(), "channel mask is invalid");
    return;
  }
  unsigned N = getIntegralValue(CallInfo.CE->getDirectCallee(), 1);
  unsigned M = getIntegralValue(CallInfo.CE->getDirectCallee(), 2);
  if (M != N * NumChannels) {
    Error(CallInfo.CE->getArg(2)->getExprLoc(),
          "destination size does not match number of elements to be read");
    return;
  }

  llvm::Value *Dst = CallInfo.CI->getArgOperand(2);
  assert(isa<llvm::PointerType>(Dst->getType()));
  llvm::Value *OldValue = CGF.Builder.CreateDefaultAlignedLoad(Dst);
  llvm::Value *Offset = CallInfo.CI->getArgOperand(1);
  auto *OffsetTy = cast<llvm::FixedVectorType>(Offset->getType());

  // Convert the offest type to UD if it is not.
  if (OffsetTy->getElementType() != CGF.Int64Ty) {
    llvm::Type *Ty = llvm::FixedVectorType::get(CGF.Int64Ty, N);
    Offset = CGF.Builder.CreateZExt(Offset, Ty);
  }
  llvm::Value *BaseAddr = CallInfo.CI->getArgOperand(0);
  if (BaseAddr->getType() != CGF.Int64Ty) {
    if (BaseAddr->getType()->isPointerTy())
      BaseAddr = CGF.Builder.CreatePtrToInt(BaseAddr, CGF.Int64Ty);
    else
      BaseAddr = CGF.Builder.CreateZExt(BaseAddr, CGF.Int64Ty);
  }
  // workaround, hw does not really have a global-address, add
  // base-address and the offset
  // For cm_svm_gather4_scaled, BaseAddr = 0
  llvm::Value *Splat = CGF.Builder.CreateVectorSplat(N, BaseAddr);
  Offset = CGF.Builder.CreateAdd(Offset, Splat);

  // create intrinsic call: use scaled message for any platform since scale is
  // 0. Types for overloading.
  SmallVector<llvm::Type *, 3> Tys;
  Tys.push_back(OldValue->getType()); // return type for gather
  Tys.push_back(getMaskType(Offset->getContext(),
                            OffsetTy->getNumElements())); // predicate
  Tys.push_back(OffsetTy); // element offset
  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_svm_gather4_scaled, Tys);
  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.
  llvm::Value *Args[] = {
      llvm::Constant::getAllOnesValue(FTy->getParamType(0)), // predicate
      llvm::ConstantInt::get(CGF.Int32Ty, Mask),
      llvm::ConstantInt::get(CGF.Int16Ty, 0),
      llvm::ConstantInt::get(CGF.Int64Ty, 0),
      Offset,
      OldValue};
  auto NewCI = CGF.Builder.CreateCall(Fn, Args);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  CallInfo.CI->eraseFromParent();
}

/// template <typename T, int N, int M>
/// typename std::enable_if<(N == 8 || N == 16) && (sizeof(T) == 4)>::type
/// cm_ptr_write4(T* basePtr, vector<ptrdiff_t, N> vOffset, vector<T, M> vSrc,
///               ChannelMaskType mask);
///
/// template <typename T, int N, int M>
/// typename std::enable_if<(N == 8 || N == 16 || N == 32) && (sizeof(T) ==
/// 4)>::type cm_svm_scatter4_scaled(vector<uint, N> vOffset,
///                        vector<T, M> pSrc,
///                        ChannelMaskType mask)
void CGCMRuntime::HandleBuiltinSVMWrite4Impl(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  const Expr *Arg3 = CallInfo.CE->getArg(3);
  Expr::EvalResult MaskResult;
  if (!Arg3->EvaluateAsInt(MaskResult, CGF.getContext())) {
    Error(Arg3->getExprLoc(), "channel mask must be constant");
    return;
  }

  llvm::APSInt Mask = MaskResult.Val.getInt();
  unsigned NumChannels =
      getNumberOfColors(static_cast<ChannelMaskType>(Mask.getZExtValue()));
  if (!NumChannels) {
    Error(Arg3->getExprLoc(), "channel mask is invalid");
    return;
  }
  unsigned N = getIntegralValue(CallInfo.CE->getDirectCallee(), 1);
  unsigned M = getIntegralValue(CallInfo.CE->getDirectCallee(), 2);
  if (M != N * NumChannels) {
    Error(CallInfo.CE->getArg(2)->getExprLoc(),
          "destination size does not match number of elements to write");
    return;
  }
  llvm::Value *Offset = CallInfo.CI->getArgOperand(1);
  llvm::Value *Src = CallInfo.CI->getArgOperand(2);
  auto *OffsetTy = cast<llvm::FixedVectorType>(Offset->getType());

  // Convert the offest type to UD if it is not.
  if (OffsetTy->getElementType() != CGF.Int64Ty) {
    llvm::Type *Ty = llvm::FixedVectorType::get(CGF.Int64Ty, N);
    Offset = CGF.Builder.CreateZExt(Offset, Ty);
  }
  llvm::Value *BaseAddr = CallInfo.CI->getArgOperand(0);
  if (BaseAddr->getType() != CGF.Int64Ty) {
    if (BaseAddr->getType()->isPointerTy())
      BaseAddr = CGF.Builder.CreatePtrToInt(BaseAddr, CGF.Int64Ty);
    else
      BaseAddr = CGF.Builder.CreateZExt(BaseAddr, CGF.Int64Ty);
  }
  // workaround, hw does not really have a global-address, add
  // base-address and the offset
  // For cm_svm_scatter4_scaled, BaseAddr = 0
  llvm::Value *Splat = CGF.Builder.CreateVectorSplat(N, BaseAddr);
  Offset = CGF.Builder.CreateAdd(Offset, Splat);

  // Types for overloading.
  SmallVector<llvm::Type *, 3> Tys;
  Tys.push_back(getMaskType(Offset->getContext(),
                            OffsetTy->getNumElements())); // predicate
  Tys.push_back(OffsetTy);                                // element offset
  Tys.push_back(Src->getType()); // data type for scatter

  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_svm_scatter4_scaled, Tys);
  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.
  llvm::Value *Args[] = {
      llvm::Constant::getAllOnesValue(FTy->getParamType(0)), // predicate
      llvm::ConstantInt::get(CGF.Int32Ty, Mask),
      llvm::ConstantInt::get(CGF.Int16Ty, 0),
      llvm::ConstantInt::get(CGF.Int64Ty, 0), // hw BaseAddr,
      Offset,                                 // address vector
      Src};
  auto NewCI = CGF.Builder.CreateCall(Fn, Args);

  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());

  CallInfo.CI->eraseFromParent();
}

namespace {

unsigned getAtomicSVMIntrinsicID(CmAtomicOpType Op) {
  switch (Op) {
  case ATOMIC_ADD:
    return llvm::GenXIntrinsic::genx_svm_atomic_add;
  case ATOMIC_SUB:
    return llvm::GenXIntrinsic::genx_svm_atomic_sub;
  case ATOMIC_INC:
    return llvm::GenXIntrinsic::genx_svm_atomic_inc;
  case ATOMIC_DEC:
    return llvm::GenXIntrinsic::genx_svm_atomic_dec;
  case ATOMIC_MIN:
    return llvm::GenXIntrinsic::genx_svm_atomic_min;
  case ATOMIC_MAX:
    return llvm::GenXIntrinsic::genx_svm_atomic_max;
  case ATOMIC_XCHG:
    return llvm::GenXIntrinsic::genx_svm_atomic_xchg;
  case ATOMIC_CMPXCHG:
    return llvm::GenXIntrinsic::genx_svm_atomic_cmpxchg;
  case ATOMIC_AND:
    return llvm::GenXIntrinsic::genx_svm_atomic_and;
  case ATOMIC_OR:
    return llvm::GenXIntrinsic::genx_svm_atomic_or;
  case ATOMIC_XOR:
    return llvm::GenXIntrinsic::genx_svm_atomic_xor;
  case ATOMIC_MINSINT:
    return llvm::GenXIntrinsic::genx_svm_atomic_imin;
  case ATOMIC_MAXSINT:
    return llvm::GenXIntrinsic::genx_svm_atomic_imax;
  case ATOMIC_FMAX:
    return llvm::GenXIntrinsic::genx_svm_atomic_fmax;
  case ATOMIC_FMIN:
    return llvm::GenXIntrinsic::genx_svm_atomic_fmin;
  case ATOMIC_FCMPWR:
    return llvm::GenXIntrinsic::genx_svm_atomic_fcmpwr;
  case ATOMIC_FADD:
  case ATOMIC_FSUB:
    return llvm::GenXIntrinsic::not_genx_intrinsic;
  }

  llvm_unreachable("invalid atomic operation");
}

AtomicCheckResult checkSVMAtomicOp(CmAtomicOpType Op, unsigned NumSrc) {
  switch (Op) {
  case ATOMIC_CMPXCHG:
    return (NumSrc == 2) ? AR_Valid : AR_NotTwoSrc;
  case ATOMIC_INC:
  case ATOMIC_DEC:
    return (NumSrc == 0) ? AR_Valid : AR_NotZeroSrc;
  case ATOMIC_ADD:
  case ATOMIC_SUB:
  case ATOMIC_MIN:
  case ATOMIC_MAX:
  case ATOMIC_XCHG:
  case ATOMIC_AND:
  case ATOMIC_OR:
  case ATOMIC_XOR:
  case ATOMIC_MINSINT:
  case ATOMIC_MAXSINT:
    return (NumSrc == 1) ? AR_Valid : AR_NotOneSrc;
  case ATOMIC_FMAX:
  case ATOMIC_FMIN:
  case ATOMIC_FCMPWR:
  case ATOMIC_FADD:
  case ATOMIC_FSUB:
    return AR_Invalid;
  }

  return AR_Invalid;
}

/// Determine what type OrigElType needs to be converted to in order for it
// to be appropriate to use with the specified SVM atomic operation.
QualType checkSVMAtomicOperands(CmAtomicOpType Op, ASTContext &CTX,
                                QualType OrigElType) {
  bool NeedSigned = false;

  switch (Op) {
  case ATOMIC_CMPXCHG:
  case ATOMIC_INC:
  case ATOMIC_DEC:
  case ATOMIC_ADD:
  case ATOMIC_SUB:
  case ATOMIC_MIN:
  case ATOMIC_MAX:
  case ATOMIC_XCHG:
  case ATOMIC_AND:
  case ATOMIC_OR:
  case ATOMIC_XOR:
    NeedSigned = false;
    break;
  case ATOMIC_MINSINT:
  case ATOMIC_MAXSINT:
    NeedSigned = true;
    break;
  case ATOMIC_FMAX:
  case ATOMIC_FMIN:
  case ATOMIC_FCMPWR:
    return OrigElType;
  case ATOMIC_FADD:
  case ATOMIC_FSUB:
    assert(false &&
           "floating point operands not support for SVM atomic operation.");
    return OrigElType;
  }

  QualType NewElType;

  if (!OrigElType->isIntegerType())
    NewElType = NeedSigned ? CTX.IntTy : CTX.UnsignedIntTy;
  else {
    unsigned size = CTX.getTypeSize(OrigElType);

    if (size <= 32)
      NewElType = NeedSigned ? CTX.IntTy : CTX.UnsignedIntTy;
    else
      NewElType = NeedSigned ? CTX.LongLongTy : CTX.UnsignedLongLongTy;
  }

  return NewElType;
}

} // namespace

/// template<typename T0, int SZ>
/// void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, SZ> vAddr,
///                    vector_ref<T0, SZ> dst,
///                    vector<T0, SZ> src0, vector<T0, SZ> src1);
///
/// template<typename T0, int N, int M>
/// void cm_svm_atomic(CmAtomicOpType op, matrix<svmptr_t, N, M> vAddr,
///                    matrix_ref<T0, N, M> dst,
///                    matrix<T0, N, M> src0, matrix<T0, N, M> src1);
///
/// template<typename T0, int SZ>
/// void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, SZ> vAddr,
///                    vector_ref<T0, SZ> dst,
///                    vector<T0, SZ> src0);
///
/// template<typename T0, int N, int M>
/// void cm_svm_atomic(CmAtomicOpType op, matrix<svmptr_t, N, M> vAddr,
///                    matrix_ref<T0, N, M> dst,
///                    matrix<T0, N, M> src0);
///
/// template<typename T0, int SZ>
/// void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, SZ> vAddr,
///                    vector_ref<T0, SZ> dst);
///
/// template<typename T0, int N, int M>
/// void cm_svm_atomic(CmAtomicOpType op, matrix<svmptr_t, N, M> vAddr,
///                    matrix_ref<T0, N, M> dst);
void CGCMRuntime::HandleBuiltinSVMAtomicOpImpl(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  // Check the atomic op kind.
  const Expr *OpKindExpr = CallInfo.CE->getArg(0);
  Expr::EvalResult OpValResult;
  CmAtomicOpType OpKind;
  if (!OpKindExpr->EvaluateAsInt(OpValResult, CGF.getContext())) {
    Error(OpKindExpr->getExprLoc(), "compile-time atomic op expected");
    return;
  } else {
    llvm::APSInt OpVal = OpValResult.Val.getInt();
    OpKind = static_cast<CmAtomicOpType>(OpVal.getZExtValue());
    switch (checkSVMAtomicOp(OpKind, CallInfo.CE->getNumArgs() - 3)) {
    case AR_NotZeroSrc:
      return Error(OpKindExpr->getExprLoc(), "no source operand expected");
    case AR_NotOneSrc:
      return Error(OpKindExpr->getExprLoc(), "no source operand expected");
    case AR_NotTwoSrc:
      return Error(OpKindExpr->getExprLoc(), "two source operands expected");
    case AR_Invalid:
      return Error(OpKindExpr->getExprLoc(), "invalid atomic op");
    default:
      break;
    }
  }

  // Check source and dst types. The intrinsics have fairly strict constraints
  // on the types they accept (based on the constraints that the vISA
  // instruction they represent has), but the Cm language function is not
  // as constrained. So, validate that the arguments are vectors or
  // matrices of the appropriate dimensions, but introduce appropriate
  // casts where necessary.

  llvm::Value *Dst = CallInfo.CI->getArgOperand(2);
  const Expr *DstEx = CallInfo.CE->getArg(2);
  QualType DstType = DstEx->getType();

  uint32_t NumElems = 0;
  if (DstType->isCMMatrixType()) {
    const CMMatrixType *DstMTType = DstType->castAs<CMMatrixType>();
    NumElems = DstMTType->getNumColumns() * DstMTType->getNumRows();
  } else
    NumElems = DstType->castAs<CMVectorType>()->getNumElements();

  if (NumElems != 8)
    return Error(DstEx->getExprLoc(), "destination must have 8 elements");

  assert(DstType->isCMReferenceType());

  // At this point, the atomic call is valid. We emit the atomic SVM operation.

  // ConvType will be the most appropriate type to convert the destination
  // element type to, based on the operation being performed and the
  // incoming type, and may well be same as that type.
  QualType ConvType = checkSVMAtomicOperands(
      OpKind, CGF.getContext(), DstType->getCMVectorMatrixElementType());

  llvm::Type *ConvTy =
      llvm::FixedVectorType::get(CGF.ConvertType(ConvType), NumElems);
  llvm::Instruction::CastOps CastOp;
  bool NeedsCast = getCastOpKind(CastOp, CGF, ConvType,
                                 DstType->getCMVectorMatrixElementType());

  CGBuilderTy &Builder = CallInfo.CGF->Builder;

  // Determine the precise intrinsic needed.
  unsigned ID = getAtomicSVMIntrinsicID(OpKind);
  llvm::Value *AddrVec = CallInfo.CI->getArgOperand(1);
  auto *AddrVecTy = cast<llvm::FixedVectorType>(AddrVec->getType());
  unsigned N = AddrVecTy->getNumElements();

  // Types for overloading
  SmallVector<llvm::Type *, 8> Tys;
  llvm::LLVMContext &C = CGF.getLLVMContext();

  // Result type
  Tys.push_back(ConvTy);

  // Predicate type
  Tys.push_back(getMaskType(C, N));

  // Address vector type
  Tys.push_back(AddrVec->getType());

  llvm::Function *Fn = getGenXIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();

  // Collect arguments.
  SmallVector<llvm::Value *, 8> Args;

  // Predicate
  Args.push_back(llvm::Constant::getAllOnesValue(FnTy->getParamType(0)));

  // Address vector
  Args.push_back(AddrVec);

  // INC or DEC does not have any source. If a source is required,
  // make sure it has been cast to the appropriate type if necessary.
  if (ID != llvm::GenXIntrinsic::genx_svm_atomic_inc &&
      ID != llvm::GenXIntrinsic::genx_svm_atomic_dec) {
    llvm::Value *Arg3 = CallInfo.CI->getArgOperand(3);

    if (NeedsCast)
      Arg3 = Builder.CreateCast(CastOp, Arg3, ConvTy, "conv");
    Args.push_back(Arg3);
  }

  // cmpxchg takes one extra source. Again, make sure it has
  // be cast the the appropriate type if required.
  if (ID == llvm::GenXIntrinsic::genx_svm_atomic_cmpxchg) {
    llvm::Value *Arg4 = CallInfo.CI->getArgOperand(4);

    if (NeedsCast)
      Arg4 = Builder.CreateCast(CastOp, Arg4, ConvTy, "conv");
    Args.push_back(Arg4);
  }

  // The old value for the return value, cast if necessary.
  llvm::Value *DstDeref = Builder.CreateDefaultAlignedLoad(Dst);
  if (NeedsCast)
    DstDeref = Builder.CreateCast(CastOp, DstDeref, ConvTy, "conv");
  Args.push_back(DstDeref);

  // Call genx intrinsic.
  llvm::CallInst *NewCI = Builder.CreateCall(Fn, Args);
  NewCI->takeName(CallInfo.CI);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());

  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  CallInfo.CI->eraseFromParent();
}

/// template <typename T, int N, int M>
/// void cm_avs_sampler(matrix_ref<T, N, M> m, ChannelMaskType channelMask,
///                     SurfaceIndex surfIndex, SamplerIndex samplerIndex, float
///                     u, float v, float deltaU, float deltaV, float u2d, int
///                     groupID = -1, int verticalBlockNumber = -1,
///                     OutputFormatControl outControl = CM_16_FULL, float v2d =
///                     0, AVSExecMode execMode = CM_AVS_16x4, bool IEFBypass =
///                     false);
///
void CGCMRuntime::HandleBuiltinAVSSampler(CMCallInfo &Info) {
  CodeGenFunction &CGF = *Info.CGF;

  // Check whether the channel mask is a compile time constant. If not,
  // issue an error and use 0.
  uint32_t Mask = 0;
  getConstantValue(*this, CGF, Mask, Info.CI->getArgOperand(1),
                   Info.CE->getArg(1));

  // Check whether the output format control is a compile time constant.
  // If not, issue an error and use 0.
  uint32_t OutfmtCtrl = 0;
  getConstantValue(*this, CGF, OutfmtCtrl, Info.CI->getArgOperand(11),
                   Info.CE->getArg(11));

  // Check whether the execution mode is a compile time constant. If not,
  // issue an error and use 0.
  uint32_t ExecMode = 0;
  getConstantValue(*this, CGF, ExecMode, Info.CI->getArgOperand(13),
                   Info.CE->getArg(13));

  llvm::Value *Dst = Info.CI->getArgOperand(0);
  llvm::Type *DstTy = Dst->getType();
  assert(DstTy->isPointerTy() && "pointer type expected");

  // Declare the avs instrinsic.
  unsigned ID = llvm::GenXIntrinsic::genx_avs;
  llvm::Function *Fn = getGenXIntrinsic(ID, DstTy->getPointerElementType());
  llvm::FunctionType *FTy = Fn->getFunctionType();

  // The expected VerticalBlockNumber is of type UD.
  // The old compiler is doing sext here.
  llvm::Value *VerticalBlockNumber = Info.CGF->Builder.CreateSExtOrBitCast(
      Info.CI->getArgOperand(10), FTy->getParamType(9));

  auto &TOpts = CGM.getTarget().getTargetOpts();

  // IEFBypass is removed on some platforms (like TGLLP, etc) and should be
  // always set to 0
  llvm::Value *IEFBypass = llvm::Constant::getNullValue(FTy->getParamType(13));
  if (TOpts.CMIEFByPass)
    IEFBypass = Info.CGF->Builder.CreateZExtOrBitCast(
        Info.CI->getArgOperand(14), FTy->getParamType(13));

  // Arguments.
  llvm::Value *Args[] = {
      llvm::ConstantInt::get(FTy->getParamType(0), Mask), // channelMask
      Info.CI->getArgOperand(3),                          // samplerIndex
      Info.CI->getArgOperand(2),                          // surfaceIndex
      Info.CI->getArgOperand(4),                          // u
      Info.CI->getArgOperand(5),                          // v
      Info.CI->getArgOperand(6),                          // deltaU
      Info.CI->getArgOperand(7),                          // deltaV
      Info.CI->getArgOperand(8),                          // u2d
      Info.CI->getArgOperand(9),                          // groupID
      VerticalBlockNumber,                                // verticalBlockNumber
      llvm::ConstantInt::get(FTy->getParamType(10), OutfmtCtrl), // OutfmtCtrl
      Info.CI->getArgOperand(12),                                // v2d
      llvm::ConstantInt::get(FTy->getParamType(12), ExecMode),   // execMode
      IEFBypass,                                                 // IEFBypass
  };

  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(Fn, Args);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// template <int N>
/// void cm_va_2d_convolve(matrix_ref<short, N, 16> m, SurfaceIndex surfIndex,
///                        SamplerIndex sampIndex, float u, float v,
///                        CONVExecMode execMode = CM_CONV_16x4, bool big_kernel
///                        = false);
/// void cm_va_2d_convolve_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
///   float u, float v, bool big_kernel, CM_FORMAT_SIZE size, SurfaceIndex
///   destIndex, ushort x_offset, ushort y_offset);
void CGCMRuntime::HandleBuiltinVA2dConvolve(CMCallInfo &Info,
                                            CMBuiltinKind Kind) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Kind == CMBK_cm_va_2d_convolve || Kind == CMBK_cm_va_2d_convolve_hdc);

  assert(Info.CE->getNumArgs() == (Kind == CMBK_cm_va_2d_convolve ? 7 : 9) &&
         "incorrect number of arguments");

  uint32_t ExecMode = 0;
  if (!getConstantValue(*this, CGF, ExecMode, Info.CI->getArgOperand(5),
                        Info.CE->getArg(5)))
    return;

  uint32_t BigKernelArgNum = Kind == CMBK_cm_va_2d_convolve ? 6 : 4;
  uint32_t BigKernel = 0;
  if (!getConstantValue(*this, CGF, BigKernel,
                        Info.CI->getArgOperand(BigKernelArgNum),
                        Info.CE->getArg(BigKernelArgNum)))
    return;

  llvm::Function *Fn;
  llvm::Value *Dst = NULL;
  uint32_t ArgsBase;
  if (Kind == CMBK_cm_va_2d_convolve) {
    const Expr *DstEx = Info.CE->getArg(0);
    QualType DstType = DstEx->getType();
    assert(DstType->isCMMatrixType() &&
           "destination argument must be a matrix");

    assert(DstType->castAs<CMMatrixType>()->getNumColumns() == 16 &&
           "destination matrix must have 16 columns");

    uint32_t NumRows = DstType->castAs<CMMatrixType>()->getNumRows();
    if (!((ExecMode == 0 && NumRows == 4) || (ExecMode == 2 && NumRows == 1))) {
      Error(DstEx->getExprLoc(),
            "number of rows in cm_va_2d_convolve() destination "
            "matrix does not match the execution mode");
      return;
    }

    Dst = Info.CI->getArgOperand(0);
    llvm::Type *DstTy = Dst->getType();
    assert(DstTy->isPointerTy() &&
           "pointer type expected for destination argument");

    Fn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_convolve2d,
                          DstTy->getPointerElementType());
    ArgsBase = 1;
  } else {
    Fn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_hdc_convolve2d);
    ArgsBase = 0;
  }

  llvm::FunctionType *FTy = Fn->getFunctionType();

  // ExecMode means different things for the HDC and non-HDC variants,
  // but the way it and BigKernel are combined is the same for both.
  ExecMode = (ExecMode & 0xf) | ((BigKernel & 0x1) << 4);

  // Arguments.
  SmallVector<llvm::Value *, 8> ConvArgs;

  // The back-end intrinsic needs the input surface and sampler indices
  // in the opposite order to the Cm builtin.
  ConvArgs.push_back(Info.CI->getArgOperand(ArgsBase + 1)); // samplerIndex
  ConvArgs.push_back(Info.CI->getArgOperand(ArgsBase));     // surfaceIndex
  ConvArgs.push_back(Info.CI->getArgOperand(ArgsBase + 2)); // u
  ConvArgs.push_back(Info.CI->getArgOperand(ArgsBase + 3)); // v
  ConvArgs.push_back(llvm::ConstantInt::get(FTy->getParamType(4),
                                            ExecMode)); // ExecMode and size

  if (Kind == CMBK_cm_va_2d_convolve_hdc) {
    assert(ArgsBase == 0);
    ConvArgs.push_back(Info.CI->getArgOperand(6)); // destIndex
    ConvArgs.push_back(Info.CI->getArgOperand(7)); // x-offset
    ConvArgs.push_back(Info.CI->getArgOperand(8)); // y-offset
  }

  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(Fn, ConvArgs);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  if (Kind == CMBK_cm_va_2d_convolve)
    CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// template <int N>
/// void cm_va_erode(vector_ref<uint, N> m, SurfaceIndex surfIndex,
///                  SamplerIndex sampIndex, float u, float v,
///                  EDExecMode execMode = CM_ED_16x4);
/// template <int N>
/// void cm_va_dilate(vector_ref<uint, N> m, SurfaceIndex surfIndex,
///                   SamplerIndex sampIndex, float u, float v,
///                   EDExecMode execMode = CM_ED_16x4);
void CGCMRuntime::HandleBuiltinVAErodeDilate(CMCallInfo &Info,
                                             CMBuiltinKind Kind) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Kind == CMBK_cm_va_dilate || Kind == CMBK_cm_va_erode);

  assert(Info.CE->getNumArgs() == 6 && "incorrect number of arguments");

  uint32_t ExecMode = 0;
  if (!getConstantValue(*this, CGF, ExecMode, Info.CI->getArgOperand(5),
                        Info.CE->getArg(5)))
    return;

#if _DEBUG
  const Expr *DstEx = Info.CE->getArg(0);
  QualType DstType = DstEx->getType();
  assert(DstType->isCMVectorType() && "destination argument must be a vector");
#endif

  llvm::Value *Dst = Info.CI->getArgOperand(0);
  llvm::Type *DstTy = Dst->getType();
  assert(DstTy->isPointerTy() &&
         "pointer type expected for destination argument");

  unsigned ID = Kind == CMBK_cm_va_erode ? llvm::GenXIntrinsic::genx_va_erode
                                         : llvm::GenXIntrinsic::genx_va_dilate;
  llvm::Function *Fn = getGenXIntrinsic(ID, DstTy->getPointerElementType());
  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.

  llvm::Value *Args[] = {
      Info.CI->getArgOperand(2), // samplerIndex
      Info.CI->getArgOperand(1), // surfaceIndex
      Info.CI->getArgOperand(3), // u
      Info.CI->getArgOperand(4), // v
      llvm::ConstantInt::get(FTy->getParamType(4), ExecMode & 0x3), // ExecMode
  };

  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(Fn, Args);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// void cm_va_erode_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
///                      float u, float v,
///                      SurfaceIndex destIndex, ushort x_offset, ushort
///                      y_offset);
/// void cm_va_dilate_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
///                       float u, float v,
///                       SurfaceIndex destIndex, ushort x_offset, ushort
///                       y_offset);
void CGCMRuntime::HandleBuiltinVAErodeDilateHdc(CMCallInfo &Info,
                                                CMBuiltinKind Kind) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Kind == CMBK_cm_va_dilate_hdc || Kind == CMBK_cm_va_erode_hdc);

  assert(Info.CE->getNumArgs() == 7 && "incorrect number of arguments");

  unsigned ID = Kind == CMBK_cm_va_erode_hdc
                    ? llvm::GenXIntrinsic::genx_va_hdc_erode
                    : llvm::GenXIntrinsic::genx_va_hdc_dilate;
  llvm::Function *Fn = getGenXIntrinsic(ID);

  // Arguments.

  llvm::Value *Args[] = {
      // The low-level intrinsic requires its source sampler and surface
      // index parameter in the opposite order to the builtin's parameters.
      Info.CI->getArgOperand(1), // samplerIndex
      Info.CI->getArgOperand(0), // surfaceIndex
      Info.CI->getArgOperand(2), // u
      Info.CI->getArgOperand(3), // v
      Info.CI->getArgOperand(4), // destIndex
      Info.CI->getArgOperand(5), // x_offset
      Info.CI->getArgOperand(6)  // y_offset
  };

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, Args);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());

  Info.CI->eraseFromParent();
}

/// template <typename T, int N>
/// void cm_va_min_max(vector_ref_ref<T, N, M> m, SurfaceIndex surfIndex,
///                    float u, float v, MMFEnableMode mmfMode);
void CGCMRuntime::HandleBuiltinVAMinMax(CMCallInfo &Info) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Info.CE->getNumArgs() == 5 && "incorrect number of arguments");

  const Expr *DstEx = Info.CE->getArg(0);
  QualType DstType = DstEx->getType();
  assert(DstType->isCMVectorType() && "destination argument must be a vector");

  llvm::Value *Dst = Info.CI->getArgOperand(0);
  llvm::Type *DstTy = Dst->getType();
  assert(DstTy->isPointerTy() &&
         "pointer type expected for destination argument");

  DstTy = DstTy->getPointerElementType();

  uint32_t NumElementsNeeded = 0;
  uint32_t ElementSize = cast<llvm::FixedVectorType>(DstTy)
                             ->getElementType()
                             ->getPrimitiveSizeInBits();

  if (ElementSize == 8)
    NumElementsNeeded = 32;
  else if (ElementSize == 16)
    NumElementsNeeded = 16;
  else {
    Error(DstEx->getExprLoc(), "invalid vector element type");
    return;
  }

  if (DstType->castAs<CMVectorType>()->getNumElements() != NumElementsNeeded) {
    Error(DstEx->getExprLoc(),
          "invalid vector size for the vector element type");
    return;
  }

  unsigned ID = llvm::GenXIntrinsic::genx_va_minmax;
  llvm::Function *Fn = getGenXIntrinsic(ID, DstTy);

  // Arguments.

  llvm::Value *Args[] = {
      Info.CI->getArgOperand(1), // surfaceIndex
      Info.CI->getArgOperand(2), // u
      Info.CI->getArgOperand(3), // v
      Info.CI->getArgOperand(4)  // Min Max Enabled
  };

  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(Fn, Args);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// template <typename T, int N, int M>
/// void cm_va_min_max_filter(matrix_ref<T, N, M> m, SurfaceIndex surfIndex,
///                           SamplerIndex sampIndex, float u, float v,
///                           OutputFormatControl cntrl, MMFExecMode execMode,
///                           MMFEnableMode mmfMode);
void CGCMRuntime::HandleBuiltinVAMinMaxFilter(CMCallInfo &Info) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Info.CE->getNumArgs() == 8 && "incorrect number of arguments");

  uint32_t FormCntrl = 0;
  if (!getConstantValue(*this, CGF, FormCntrl, Info.CI->getArgOperand(5),
                        Info.CE->getArg(5)))
    return;

  uint32_t ExecMode = 0;
  if (!getConstantValue(*this, CGF, ExecMode, Info.CI->getArgOperand(6),
                        Info.CE->getArg(6)))
    return;

  const Expr *DstEx = Info.CE->getArg(0);
  QualType DstType = DstEx->getType();
  assert(DstType->isCMMatrixType() && "destination argument must be a matrix");

  llvm::Value *Dst = Info.CI->getArgOperand(0);
  llvm::Type *DstTy = Dst->getType();
  assert(DstTy->isPointerTy() &&
         "pointer type expected for destination argument");

  DstTy = DstTy->getPointerElementType();

  if (cast<llvm::FixedVectorType>(DstTy)
          ->getElementType()
          ->getPrimitiveSizeInBits() != (FormCntrl < 2 ? 16 : 8)) {
    Error(DstEx->getExprLoc(), "destination matrix element type not compatible "
                               "with specified output format");
    return;
  }

  FormCntrl &= 0x3;
  ExecMode &= 0x3;

  uint32_t NumRowsNeeded = 0;
  uint32_t NumColsNeeded = 0;

  if (ExecMode == 3) {
    NumRowsNeeded = 1;
    NumColsNeeded = FormCntrl < 2 ? 2 : 4;
  } else {
    NumColsNeeded = 32;
    if (ExecMode == 0)
      NumRowsNeeded = 4;
    else if (ExecMode == 2)
      NumRowsNeeded = 1;
    else {
      Error(Info.CE->getArg(6)->getExprLoc(), "invalid return data format");
      return;
    }
  }

  uint32_t NumRows = DstType->castAs<CMMatrixType>()->getNumRows();
  uint32_t NumCols = DstType->castAs<CMMatrixType>()->getNumColumns();

  if (NumRows != NumRowsNeeded || NumCols != NumColsNeeded) {
    Error(
        DstEx->getExprLoc(),
        "destination matrix dimenions not compatible with return data format");
    return;
  }

  unsigned ID = llvm::GenXIntrinsic::genx_va_minmax_filter;
  llvm::Function *Fn = getGenXIntrinsic(ID, DstTy);
  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.

  llvm::Value *Args[] = {
      Info.CI->getArgOperand(2),                               // samplerIndex
      Info.CI->getArgOperand(1),                               // surfaceIndex
      Info.CI->getArgOperand(3),                               // u
      Info.CI->getArgOperand(4),                               // v
      llvm::ConstantInt::get(FTy->getParamType(4), FormCntrl), // Output format
      llvm::ConstantInt::get(FTy->getParamType(5), ExecMode),  // ExecMode
      Info.CI->getArgOperand(7) // Min Max Enabled
  };

  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(Fn, Args);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// void cm_va_min_max_filter_hdc(SurfaceIndex surfIndex, SamplerIndex
/// sampIndex,
///                               float u, float v, MMFEnableMode mmfMode,
///                               CM_FORMAT_SIZE size, SurfaceIndex destIndex,
///                               ushort x_offset, ushort y_offset);
void CGCMRuntime::HandleBuiltinVAMinMaxFilterHdc(CMCallInfo &Info) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Info.CE->getNumArgs() == 9 && "incorrect number of arguments");

  uint32_t MmfMode = 0;
  if (!getConstantValue(*this, CGF, MmfMode, Info.CI->getArgOperand(4),
                        Info.CE->getArg(4)))
    return;

  uint32_t FormCntrl = 0;
  if (!getConstantValue(*this, CGF, FormCntrl, Info.CI->getArgOperand(5),
                        Info.CE->getArg(5)))
    return;

  FormCntrl &= 0x3;
  MmfMode &= 0x3;

  unsigned ID = llvm::GenXIntrinsic::genx_va_hdc_minmax_filter;
  llvm::Function *Fn = getGenXIntrinsic(ID);
  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.

  llvm::Value *Args[] = {
      Info.CI->getArgOperand(1),                               // samplerIndex
      Info.CI->getArgOperand(0),                               // surfaceIndex
      Info.CI->getArgOperand(2),                               // u
      Info.CI->getArgOperand(3),                               // v
      llvm::ConstantInt::get(FTy->getParamType(4), FormCntrl), // Output format
      llvm::ConstantInt::get(FTy->getParamType(5), MmfMode), // Min Max Enabled
      Info.CI->getArgOperand(6),                             // destIndex
      Info.CI->getArgOperand(7),                             // x_offset
      Info.CI->getArgOperand(8)                              // y_offset
  };

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, Args);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());

  Info.CI->eraseFromParent();
}

/// template <typename T, int N, int M>
/// void cm_va_boolean_centroid(matrix_ref<T, N, M> m, SurfaceIndex surfIndex,
///                             float u, float v, uchar vSize, uchar hSize);
/// template <typename T, int N, int M>
/// void cm_va_centroid(matrix_ref<T, N, M> m, SurfaceIndex surfIndex,
///                     float u, float v, uchar vSize);
void CGCMRuntime::HandleBuiltinVACentroid(CMCallInfo &Info,
                                          CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_va_centroid || Kind == CMBK_cm_va_boolean_centroid);

  assert(Info.CE->getNumArgs() == (Kind == CMBK_cm_va_centroid ? 5 : 6) &&
         "incorrect number of arguments");

  const Expr *DstEx = Info.CE->getArg(0);
  QualType DstType = DstEx->getType();
  assert(DstType->isCMMatrixType() && "destination argument must be a matrix");

  llvm::Value *Dst = Info.CI->getArgOperand(0);
  llvm::Type *DstTy = Dst->getType();
  assert(DstTy->isPointerTy() &&
         "pointer type expected for destination argument");
  DstTy = DstTy->getPointerElementType();

  if (cast<llvm::FixedVectorType>(DstTy)
          ->getElementType()
          ->getPrimitiveSizeInBits() != 32) {
    Error(DstEx->getExprLoc(),
          "destination matrix element type must be an int");
    return;
  }

  if (DstType->castAs<CMMatrixType>()->getNumRows() !=
          (Kind == CMBK_cm_va_centroid ? 4 : 2) ||
      DstType->castAs<CMMatrixType>()->getNumColumns() != 8) {
    Error(DstEx->getExprLoc(), "incorrect destination matrix dimenions");
    return;
  }

  unsigned ID = Kind == CMBK_cm_va_centroid
                    ? llvm::GenXIntrinsic::genx_va_centroid
                    : llvm::GenXIntrinsic::genx_va_bool_centroid;
  llvm::Function *Fn = getGenXIntrinsic(ID, DstTy);

  // Arguments.
  SmallVector<llvm::Value *, 8> CentroidArgs;

  CentroidArgs.push_back(Info.CI->getArgOperand(1)); // Surface index
  CentroidArgs.push_back(Info.CI->getArgOperand(2)); // u
  CentroidArgs.push_back(Info.CI->getArgOperand(3)); // v
  CentroidArgs.push_back(Info.CI->getArgOperand(4)); // vSize
  if (Kind == CMBK_cm_va_boolean_centroid)
    CentroidArgs.push_back(Info.CI->getArgOperand(5)); // hSize

  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(Fn, CentroidArgs);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  Info.CGF->Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// template <int N>
/// void cm_va_1d_convolution(matrix_ref<short, N, 16> m, SurfaceIndex
/// surfIndex,
///                           SamplerIndex sampIndex, bool isHdirection,
///                           float u, float v, CONVExecMode execMode =
///                           CM_CONV_16x4);
/// void cm_va_1d_convolution_hdc(SurfaceIndex surfIndex, SamplerIndex
/// sampIndex,
///    bool isHdirection, float u, float v, CM_FORMAT_SIZE size,
///    SurfaceIndex destIndex, ushort x_offset, ushort y_offset);
void CGCMRuntime::HandleBuiltinVA1dConvolution(CMCallInfo &Info,
                                               CMBuiltinKind Kind) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Kind == CMBK_cm_va_1d_convolution ||
         Kind == CMBK_cm_va_1d_convolution_hdc);

  assert(Info.CE->getNumArgs() == (Kind == CMBK_cm_va_1d_convolution ? 7 : 9) &&
         "incorrect number of arguments");

  uint32_t IsHArgNum = (Kind == CMBK_cm_va_1d_convolution ? 3 : 2);
  uint32_t IsHorizontal = 0;
  if (!getConstantValue(*this, CGF, IsHorizontal,
                        Info.CI->getArgOperand(IsHArgNum),
                        Info.CE->getArg(IsHArgNum)))
    return;

  // For the HDC mode the ExecMode is really the pixelSize argument
  uint32_t EMArgNum = (Kind == CMBK_cm_va_1d_convolution ? 6 : 5);
  uint32_t ExecMode = 0;
  if (!getConstantValue(*this, CGF, ExecMode, Info.CI->getArgOperand(EMArgNum),
                        Info.CE->getArg(EMArgNum)))
    return;

  llvm::Value *Dst = NULL;
  llvm::Function *Fn;
  uint32_t ArgsBase;

  if (Kind == CMBK_cm_va_1d_convolution) {
    const Expr *DstEx = Info.CE->getArg(0);
    QualType DstType = DstEx->getType();
    assert(DstType->isCMMatrixType() &&
           "destination argument must be a matrix");

    // The Cm language spec states that the destination must either be 1 or 4
    // rows by 16 columns. However, there are examples that are supplying 16
    // rows by 1 or 4 columns, and presumably expecting this to work. That could
    // be outlawed, but it is probably more helpful to permit it, as the number
    // elements in the result is the same. So, just validate that the number
    // of overall elements matches what the execution mode is expecting.

    uint32_t NumElements = DstType->castAs<CMMatrixType>()->getNumRows() *
                           DstType->castAs<CMMatrixType>()->getNumColumns();
    if (!((ExecMode == 0 && NumElements == 64) ||
          (ExecMode == 2 && NumElements == 16))) {
      Error(DstEx->getExprLoc(),
            "cm_va_1d_convolution() destination "
            "matrix's dimensions do not match the execution mode");
      return;
    }

    Dst = Info.CI->getArgOperand(0);
    llvm::Type *DstTy = Dst->getType();
    assert(DstTy->isPointerTy() &&
           "pointer type expected for destination argument");

    unsigned ID = IsHorizontal
                      ? llvm::GenXIntrinsic::genx_va_1d_convolve_horizontal
                      : llvm::GenXIntrinsic::genx_va_1d_convolve_vertical;
    Fn = getGenXIntrinsic(ID, DstTy->getPointerElementType());
    ArgsBase = 1;
  } else {
    unsigned ID = IsHorizontal
                      ? llvm::GenXIntrinsic::genx_va_hdc_1d_convolve_horizontal
                      : llvm::GenXIntrinsic::genx_va_hdc_1d_convolve_vertical;
    Fn = getGenXIntrinsic(ID);
    ArgsBase = 0;
  }

  llvm::FunctionType *FTy = Fn->getFunctionType();

  ExecMode &= 0x3;

  // Arguments.

  SmallVector<llvm::Value *, 8> ConvArgs;

  // The low-level intrinsic require the surface and sampler index parameter
  // order to be swapped.
  ConvArgs.push_back(Info.CI->getArgOperand(ArgsBase + 1)); // samplerIndex
  ConvArgs.push_back(Info.CI->getArgOperand(ArgsBase));     // surfaceIndex
  ConvArgs.push_back(Info.CI->getArgOperand(ArgsBase + 3)); // u
  ConvArgs.push_back(Info.CI->getArgOperand(ArgsBase + 4)); // v
  ConvArgs.push_back(
      llvm::ConstantInt::get(FTy->getParamType(4), ExecMode)); // ExecMode
  if (Kind == CMBK_cm_va_1d_convolution_hdc) {
    assert(ArgsBase == 0);
    ConvArgs.push_back(Info.CI->getArgOperand(6)); // destIndex
    ConvArgs.push_back(Info.CI->getArgOperand(7)); // x-offset
    ConvArgs.push_back(Info.CI->getArgOperand(8)); // y-offset
  }

  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(Fn, ConvArgs);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  if (Kind == CMBK_cm_va_1d_convolution)
    CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// template <int N, int M>
/// void cm_va_1pixel_convolve(matrix_ref<short, N, M> m, SurfaceIndex
/// surfIndex,
///   SamplerIndex sampIndex, float u, float v, CONVExecMode execMode,
///   matrix<short, 1, 32> offsets);
void CGCMRuntime::HandleBuiltinVA1PixelConvolve(CMCallInfo &Info) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Info.CE->getNumArgs() >= 6 && Info.CE->getNumArgs() <= 7 &&
         "incorrect number of arguments");

  uint32_t ExecMode = 0;
  if (!getConstantValue(*this, CGF, ExecMode, Info.CI->getArgOperand(5),
                        Info.CE->getArg(5)))
    return;

  ExecMode &= 3;
  if (ExecMode != 3 && Info.CE->getNumArgs() < 7) {
    Error(Info.CE->getExprLoc(),
          "specified execution mode requires an offsets parameter");
    return;
  }

  const Expr *DstEx = Info.CE->getArg(0);
  QualType DstType = DstEx->getType();
  assert(DstType->isCMMatrixType() && "destination argument must be a matrix");

  uint32_t NumRows = DstType->castAs<CMMatrixType>()->getNumRows();
  uint32_t NumCols = DstType->castAs<CMMatrixType>()->getNumColumns();

  if (ExecMode == 3) {
    if (NumRows != 1 || NumCols != 1) {
      Error(DstEx->getExprLoc(),
            "destination must be a 1x1 matrix for 1x1 mode");
      return;
    }
  } else {
    if (DstType->castAs<CMMatrixType>()->getNumColumns() != 16) {
      Error(DstEx->getExprLoc(), "destination matrix must have 16 columns");
      return;
    }
    if (!((ExecMode == 0 && NumRows == 4) || (ExecMode == 2 && NumRows == 1))) {
      Error(DstEx->getExprLoc(), "destination matrix has the wrong number of "
                                 "rows for the specified execution mode");
      return;
    }

    // The Cm language spec states that the offsets matrix, if supplied,
    // should be 1x32. However, there are examples that are supplying
    // a 2x16 matrix instead. So, just validate that the number of elements
    // overall is correct, rather than requiring a specific shape.

    const Expr *OffsEx = Info.CE->getArg(6);
    QualType OffsType = OffsEx->getType();
    assert(OffsType->isCMMatrixType() && "offsets parameter must be a matrix");

    uint32_t OffsNumRows = OffsType->castAs<CMMatrixType>()->getNumRows();
    uint32_t OffsNumCols = OffsType->castAs<CMMatrixType>()->getNumColumns();

    if (OffsNumRows * OffsNumCols != 32) {
      Error(OffsEx->getExprLoc(), "offsets matrix has incorrect dimensions");
      return;
    }
  }

  llvm::Value *Dst = Info.CI->getArgOperand(0);
  llvm::Type *DstTy = Dst->getType();
  assert(DstTy->isPointerTy() &&
         "pointer type expected for destination argument");

  llvm::Function *Fn;

  if (ExecMode != 3) {
    llvm::Value *Offs = Info.CI->getArgOperand(6);
    llvm::Type *Tys[] = {DstTy->getPointerElementType(), Offs->getType()};

    Fn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_1pixel_convolve, Tys);
  } else
    Fn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_1pixel_convolve_1x1mode,
                          DstTy->getPointerElementType());

  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.

  SmallVector<llvm::Value *, 8> ConvArgs;

  ConvArgs.push_back(Info.CI->getArgOperand(2)); // samplerIndex
  ConvArgs.push_back(Info.CI->getArgOperand(1)); // surfaceIndex
  ConvArgs.push_back(Info.CI->getArgOperand(3)); // u
  ConvArgs.push_back(Info.CI->getArgOperand(4)); // v

  if (ExecMode != 3) {
    ConvArgs.push_back(
        llvm::ConstantInt::get(FTy->getParamType(4), ExecMode)); // execMode
    ConvArgs.push_back(Info.CI->getArgOperand(6));               // offsets
  }

  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(Fn, ConvArgs);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// void cm_va_1pixel_convolve_hdc(SurfaceIndex surfIndex, SamplerIndex
/// sampIndex,
///   float u, float v, matrix<short, 1, 32> offsets, CM_FORMAT_SIZE size,
///   SurfaceIndex destIndex, ushort x_offset, ushort y_offset);
void CGCMRuntime::HandleBuiltinVA1PixelConvolveHdc(CMCallInfo &Info) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Info.CE->getNumArgs() == 9 && "incorrect number of arguments");

  uint32_t PixelSize = 0;
  if (!getConstantValue(*this, CGF, PixelSize, Info.CI->getArgOperand(5),
                        Info.CE->getArg(5)))
    return;

  PixelSize &= 3;

  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_hdc_1pixel_convolve,
                       Info.CI->getArgOperand(4)->getType());
  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.

  llvm::Value *Args[] = {
      // Low-level intrinsic requires sampler and surface index parameter order
      // to be swapped
      Info.CI->getArgOperand(1), // samplerIndex
      Info.CI->getArgOperand(0), // surfaceIndex
      Info.CI->getArgOperand(2), // u
      Info.CI->getArgOperand(3), // v
      // Low-level intrinsic requires pixelSize and offsets parameter order
      // to be swapped
      llvm::ConstantInt::get(FTy->getParamType(4), PixelSize), // pixelSize
      Info.CI->getArgOperand(4),                               // offsets
      Info.CI->getArgOperand(6),                               // destIndex
      Info.CI->getArgOperand(7),                               // x_offset
      Info.CI->getArgOperand(8)                                // y_offset
  };

  llvm::CallInst *NewCI = Info.CGF->Builder.CreateCall(Fn, Args);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());

  Info.CI->eraseFromParent();
}

/// template <int N>
/// void cm_va_lbp_creation(matrix_ref<uchar, N, 16> m, SurfaceIndex surfIndex,
///    float u, float v, LBPCreationExecMode execMode);
/// void cm_va_lbp_creation_hdc(SurfaceIndex surfIndex, float u, float v,
///    LBPCreationExecMode execMode, SurfaceIndex destIndex,
///    ushort x_offset, ushort y_offset);
void CGCMRuntime::HandleBuiltinVALbpCreation(CMCallInfo &Info,
                                             CMBuiltinKind Kind) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Kind == CMBK_cm_va_lbp_creation ||
         Kind == CMBK_cm_va_lbp_creation_hdc);

  assert(Info.CE->getNumArgs() == (Kind == CMBK_cm_va_lbp_creation ? 5 : 7) &&
         "incorrect number of arguments");

  uint32_t ExecModeArgNum = (Kind == CMBK_cm_va_lbp_creation ? 4 : 3);
  uint32_t ExecMode = 0;
  if (!getConstantValue(*this, CGF, ExecMode,
                        Info.CI->getArgOperand(ExecModeArgNum),
                        Info.CE->getArg(ExecModeArgNum)))
    return;

  ExecMode &= 0x3;

  llvm::Value *Dst = NULL;
  llvm::Function *Fn;
  uint32_t ArgsBase;

  if (Kind == CMBK_cm_va_lbp_creation) {
    const Expr *DstEx = Info.CE->getArg(0);
    QualType DstType = DstEx->getType();
    assert(DstType->isCMMatrixType() &&
           "destination argument must be a matrix");

    if (DstType->castAs<CMMatrixType>()->getNumColumns() != 16 ||
        DstType->castAs<CMMatrixType>()->getNumRows() !=
            (ExecMode == 0 ? 8 : 4)) {
      Error(DstEx->getExprLoc(), "destination matrix's dimensions are "
                                 "incorrect for the specified execution mode");
      return;
    }

    Dst = Info.CI->getArgOperand(0);
    llvm::Type *DstTy = Dst->getType();
    assert(DstTy->isPointerTy() &&
           "pointer type expected for destination argument");

    Fn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_lbp_creation,
                          DstTy->getPointerElementType());

    ArgsBase = 1;
  } else {
    Fn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_hdc_lbp_creation);
    ArgsBase = 0;
  }

  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.

  SmallVector<llvm::Value *, 8> CreateArgs;

  CreateArgs.push_back(Info.CI->getArgOperand(ArgsBase));     // surfaceIndex
  CreateArgs.push_back(Info.CI->getArgOperand(ArgsBase + 1)); // u
  CreateArgs.push_back(Info.CI->getArgOperand(ArgsBase + 2)); // v
  CreateArgs.push_back(
      llvm::ConstantInt::get(FTy->getParamType(3), ExecMode)); // ExecMode
  if (Kind == CMBK_cm_va_lbp_creation_hdc) {
    assert(ArgsBase == 0);
    CreateArgs.push_back(Info.CI->getArgOperand(4)); // destIndex
    CreateArgs.push_back(Info.CI->getArgOperand(5)); // x-offset
    CreateArgs.push_back(Info.CI->getArgOperand(6)); // y-offset
  }

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, CreateArgs);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  if (Kind == CMBK_cm_va_lbp_creation)
    CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// template <int N>
/// void cm_va_lbp_correlation(matrix_ref<uchar, N, 16> m, SurfaceIndex
/// surfIndex,
///   float u, float v, short x_offset_disparity);
/// void cm_va_lbp_correlation_hdc(SurfaceIndex surfIndex,
///   float u, float v, short x_offset_disparity,
///   SurfaceIndex destIndex, ushort x_offset, ushort y_offset);
void CGCMRuntime::HandleBuiltinVALbpCorrelation(CMCallInfo &Info,
                                                CMBuiltinKind Kind) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Kind == CMBK_cm_va_lbp_correlation ||
         Kind == CMBK_cm_va_lbp_correlation_hdc);

  assert(Info.CE->getNumArgs() ==
             (Kind == CMBK_cm_va_lbp_correlation ? 5 : 7) &&
         "incorrect number of arguments");

  llvm::Value *Dst = NULL;
  llvm::Function *Fn;
  uint32_t ArgsBase;
  if (Kind == CMBK_cm_va_lbp_correlation) {
    const Expr *DstEx = Info.CE->getArg(0);
    QualType DstType = DstEx->getType();
    assert(DstType->isCMMatrixType() &&
           "destination argument must be a matrix");

    if (DstType->castAs<CMMatrixType>()->getNumColumns() != 16 ||
        DstType->castAs<CMMatrixType>()->getNumRows() != 4) {
      Error(DstEx->getExprLoc(),
            "destination matrix's dimensions are incorrect");
      return;
    }

    Dst = Info.CI->getArgOperand(0);
    llvm::Type *DstTy = Dst->getType();
    assert(DstTy->isPointerTy() &&
           "pointer type expected for destination argument");

    Fn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_lbp_correlation,
                          DstTy->getPointerElementType());

    ArgsBase = 1;
  } else {
    Fn = getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_hdc_lbp_correlation);
    ArgsBase = 0;
  }

  // Arguments.

  SmallVector<llvm::Value *, 8> CorrArgs;

  CorrArgs.push_back(Info.CI->getArgOperand(ArgsBase));     // surfaceIndex
  CorrArgs.push_back(Info.CI->getArgOperand(ArgsBase + 1)); // u
  CorrArgs.push_back(Info.CI->getArgOperand(ArgsBase + 2)); // v
  CorrArgs.push_back(Info.CI->getArgOperand(ArgsBase + 3)); // disparity
  if (Kind == CMBK_cm_va_lbp_correlation_hdc) {
    assert(ArgsBase == 0);
    CorrArgs.push_back(Info.CI->getArgOperand(4)); // destIndex
    CorrArgs.push_back(Info.CI->getArgOperand(5)); // x-offset
    CorrArgs.push_back(Info.CI->getArgOperand(6)); // y-offset
  }

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, CorrArgs);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  if (Kind == CMBK_cm_va_lbp_correlation)
    CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// template <int N, int M>
/// void cm_va_correlation_search(matrix_ref<int, N, M> m, SurfaceIndex
/// surfIndex,
///   float u, float v, float verticalOrigin, float horizontalOrigin,
///   uchar xDirectionSize, uchar yDirectionSize, uchar xDirectionSearchSize,
///   uchar yDirectionSearchSize);
void CGCMRuntime::HandleBuiltinVACorrelationSearch(CMCallInfo &Info) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Info.CE->getNumArgs() == 10 && "incorrect number of arguments");

  const Expr *DstEx = Info.CE->getArg(0);
  QualType DstType = DstEx->getType();
  assert(DstType->isCMMatrixType() && "destination argument must be a matrix");

  // Validation opportunities are limited. The precise number of expected
  // results isn't known until runtime, which means the precise shape of
  // return matrix cannot be validated. All that can be validated is that
  // that number of columns is a value that is valid according to the spec.
  if (DstType->castAs<CMMatrixType>()->getNumColumns() != 16 &&
      DstType->castAs<CMMatrixType>()->getNumColumns() != 8) {
    Error(DstEx->getExprLoc(),
          "destination matrix must have either 8 or 16 columns");
    return;
  }

  llvm::Value *Dst = Info.CI->getArgOperand(0);
  llvm::Type *DstTy = Dst->getType();
  assert(DstTy->isPointerTy() &&
         "pointer type expected for destination argument");

  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_correlation_search,
                       DstTy->getPointerElementType());

  // Arguments.

  llvm::Value *Args[] = {
      Info.CI->getArgOperand(1), // surfaceIndex
      Info.CI->getArgOperand(2), // u
      Info.CI->getArgOperand(3), // v
      Info.CI->getArgOperand(4), // verticalOrigin
      Info.CI->getArgOperand(5), // horizontalOrigin
      Info.CI->getArgOperand(6), // xDirectionSize
      Info.CI->getArgOperand(7), // yDirectionSize
      Info.CI->getArgOperand(8), // xDirectionSearchSize
      Info.CI->getArgOperand(9), // yDirectionSearchSize
  };

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, Args);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

/// void cm_va_flood_fill(vector_ref<ushort, 8> v, bool Is8Connect,
///   vector<ushort, 10> pixelMaskHDirection, ushort pixelMaskVDirectionLeft,
///   ushort pixelMaskVDirectionRight, ushort loopCount);
void CGCMRuntime::HandleBuiltinVAFloodFill(CMCallInfo &Info) {
  CodeGenFunction &CGF = *Info.CGF;

  assert(Info.CE->getNumArgs() == 6 && "incorrect number of arguments");

  uint32_t Is8Connect = 0;
  if (!getConstantValue(*this, CGF, Is8Connect, Info.CI->getArgOperand(1),
                        Info.CE->getArg(1)))
    return;

#if _DEBUG
  const Expr *DstEx = Info.CE->getArg(0);
  QualType DstType = DstEx->getType();
  assert(DstType->isCMVectorType() && "destination argument must be a vector");
#endif

  llvm::Value *Dst = Info.CI->getArgOperand(0);
  llvm::Type *DstTy = Dst->getType();
  assert(DstTy->isPointerTy() &&
         "pointer type expected for destination argument");

  llvm::Value *PMHDir = Info.CI->getArgOperand(2);
  llvm::Type *Tys[] = {DstTy->getPointerElementType(), PMHDir->getType()};
  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_va_flood_fill, Tys);
  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.

  llvm::Value *Args[] = {
      llvm::ConstantInt::get(FTy->getParamType(0), Is8Connect), // Is8Connect
      Info.CI->getArgOperand(2), // pixelMaskHDirection
      Info.CI->getArgOperand(3), // pixelMaskVDirectionLeft
      Info.CI->getArgOperand(4), // pixelMaskVDirectionRight
      Info.CI->getArgOperand(5)  // loopCount
  };

  llvm::CallInst *NewCI = CGF.Builder.CreateCall(Fn, Args);
  NewCI->takeName(Info.CI);
  NewCI->setDebugLoc(Info.CI->getDebugLoc());
  CGF.Builder.CreateDefaultAlignedStore(NewCI, Dst);

  Info.CI->eraseFromParent();
}

llvm::Value *CGCMRuntime::HandleBuiltinSimdcfAnyImpl(CMCallInfo &CallInfo) {
  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);

  if (!Arg0->getType()->getScalarType()->isIntegerTy(1)) {
    if (Arg0->getType()->getScalarType()->isFloatingPointTy())
      Arg0 = CallInfo.CGF->Builder.CreateFCmpONE(
          Arg0, llvm::Constant::getNullValue(Arg0->getType()));
    else
      Arg0 = CallInfo.CGF->Builder.CreateICmpNE(
          Arg0, llvm::Constant::getNullValue(Arg0->getType()));
  }

  if (!Arg0->getType()->isVectorTy()) {
    CI->eraseFromParent();
    return Arg0;
  }

  unsigned ID = llvm::GenXIntrinsic::genx_simdcf_any;
  llvm::Function *Fn = getGenXIntrinsic(ID, Arg0->getType());
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(Fn, Arg0);

  NewCI->takeName(CI);
  NewCI->setDebugLoc(CI->getDebugLoc());
  CI->eraseFromParent();

  return NewCI;
}

// template<typename T0, int N>
// vector<T0, N> __cm_intrinsic_impl_simdcf_predgen(vector<T0, N> arg0)
llvm::Value *
CGCMRuntime::HandleBuiltinSimdcfGenericPredicationImpl(CMCallInfo &CallInfo) {
  const CallExpr *CE = CallInfo.CE;
  assert(CE->getNumArgs() == 1);
  assert(CE->getType()->isCMVectorMatrixType());
  assert(CE->getArg(0)->getType()->isCMVectorMatrixType());

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Type *Arg0Ty = Arg0->getType();
  llvm::Value *Arg1 = llvm::UndefValue::get(Arg0Ty);
  bool SrcSigned = CE->getArg(0)
                       ->getType()
                       ->getCMVectorMatrixElementType()
                       ->isSignedIntegerType();

  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_simdcf_predicate, Arg0Ty);
  llvm::Value *Args[] = {Arg0, Arg1};
  llvm::CallInst *NewCI =
      CallInfo.CGF->Builder.CreateCall(Fn, Args, "simdcfpref");
  NewCI->takeName(CI);
  NewCI->setDebugLoc(CI->getDebugLoc());
  CI->eraseFromParent();

  return NewCI;
}

// template<typename T0, int N>
// vector<T0, N> __cm_intrinsic_impl_simdcf_predmin(vector<T0, N> arg0)
// template<typename T0, int N>
// vector<T0, N> __cm_intrinsic_impl_simdcf_predmax(vector<T0, N> arg0)
llvm::Value *
CGCMRuntime::HandleBuiltinSimdcfMinMaxPredicationImpl(CMCallInfo &CallInfo,
                                                      CMBuiltinKind Kind) {
  assert(Kind == CMBK_simdcf_predmin_impl || Kind == CMBK_simdcf_predmax_impl);

  const CallExpr *CE = CallInfo.CE;
  assert(CE->getNumArgs() == 1);
  assert(CE->getType()->isCMVectorMatrixType());
  assert(CE->getArg(0)->getType()->isCMVectorMatrixType());

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Type *Arg0Ty = Arg0->getType();
  llvm::Value *Arg1;
  bool SrcSigned = CE->getArg(0)
                       ->getType()
                       ->getCMVectorMatrixElementType()
                       ->isSignedIntegerType();

  // Determine what the lowered intrinsic call's second parameter should be, the
  // one that represents the defaults to choose for SIMDCF channels which aren't
  // selected. This value will be a splat of either the maximum possible value
  // of whatever the first (vector) parameter's element type is (for the minimum
  // predication builtin) or the minimum possible value of that type (for the
  // maximum predication builtin).
  //
  // (The generic predication builtin could have been used, but it was just a
  // lot easier to get clang to work out what these default values should be
  // rather than trying to express this in Cm code for all possible valid
  // types.)

  if (CE->getArg(0)
          ->getType()
          ->getCMVectorMatrixElementType()
          ->isFloatingType()) {
    double Arg1Value = std::numeric_limits<float>::max();

    if (Kind == CMBK_simdcf_predmax_impl)
      Arg1Value = -Arg1Value;

    Arg1 = llvm::ConstantFP::get(Arg0Ty, Arg1Value);
  } else {
    uint64_t Arg1Value =
        cast<llvm::ConstantInt>(
            llvm::Constant::getAllOnesValue(Arg0Ty->getScalarType()))
            ->getZExtValue();

    if (SrcSigned) {
      Arg1Value >>= 1;
      if (Kind == CMBK_simdcf_predmax_impl)
        Arg1Value = ~Arg1Value;
    } else if (Kind == CMBK_simdcf_predmax_impl)
      Arg1Value = 0;

    Arg1 = llvm::ConstantInt::get(Arg0Ty, Arg1Value, SrcSigned);
  }

  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_simdcf_predicate, Arg0Ty);
  llvm::Value *Args[] = {Arg0, Arg1};
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(Fn, Args);
  NewCI->takeName(CI);
  NewCI->setDebugLoc(CI->getDebugLoc());
  CI->eraseFromParent();

  return NewCI;
}

/// SurfaceIndex predefined_surface(id);
///
llvm::Value *CGCMRuntime::HandlePredefinedSurface(CMCallInfo &CallInfo) {
  CodeGenFunction &CGF = *CallInfo.CGF;

  // Check whether the surface ID is a compile time constant.
  unsigned SID = 0;
  if (!getConstantValue(*this, CGF, SID, CallInfo.CI->getArgOperand(0),
                        CallInfo.CE->getArg(0)))
    return nullptr;
  if (!(SID < 4)) {
    Error(CallInfo.CE->getArg(0)->getExprLoc(),
          "integer constant is out of range expected");
    return nullptr;
  }

  llvm::Function *Fn =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_predefined_surface);
  llvm::Value *Arg =
      llvm::ConstantInt::get(Fn->getFunctionType()->getParamType(0), SID);
  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(Fn, Arg);
  NewCI->takeName(CallInfo.CI);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  assert(NewCI->getType() == CallInfo.CI->getType());

  CallInfo.CI->eraseFromParent();
  return NewCI;
}

// template <CmAtomicOpType Op, typename T, int N>
// vector<T, N> void __cm_intrinsic_impl_svm_atomic(vector<uint64_t, N> vAddr,
//                                                  vector<T, N> oldVal);
//
// template <CmAtomicOpType Op, typename T, int N>
// vector<T, N> void __cm_intrinsic_impl_svm_atomic(vector<uint64_t, N> vAddr,
//                                                  vector<T, N> src0,
//                                                  vector<T, N> oldVal);
//
// template <CmAtomicOpType Op, typename T, int N>
// vector<T, N> void __cm_intrinsic_impl_svm_atomic(vector<uint64_t, N> vAddr,
//                                                  vector<T, N> src0,
//                                                  vector<T, N> src1,
//                                                  vector<T, N> oldVal);
//
llvm::Value *CGCMRuntime::HandleBuiltinSVMAtomicImpl(CMCallInfo &CallInfo) {
  llvm::LLVMContext &C = CallInfo.CGF->getLLVMContext();
  const FunctionDecl *FD = CallInfo.CE->getDirectCallee();
  auto Op = static_cast<CmAtomicOpType>(getIntegralValue(FD, 0));
  unsigned ID = getAtomicSVMIntrinsicID(Op);

  auto *CITy = cast<llvm::FixedVectorType>(CallInfo.CI->getType());
  unsigned N = CITy->getNumElements();

  // Types for overloading
  llvm::Type *Tys[] = {
      CITy,                                    // result
      getMaskType(C, N),                       // predicate
      CallInfo.CI->getArgOperand(0)->getType() // address
  };

  llvm::Function *Fn = getGenXIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();

  SmallVector<llvm::Value *, 8> Args;
  // Predicate
  Args.push_back(llvm::Constant::getAllOnesValue(FnTy->getParamType(0)));
  // Address vector
  Args.push_back(CallInfo.CI->getArgOperand(0));
  // Remaining operands.
  for (unsigned i = 1, e = CallInfo.CI->getNumArgOperands(); i < e; ++i)
    Args.push_back(CallInfo.CI->getArgOperand(i));

  llvm::CallInst *NewCI = CallInfo.CGF->Builder.CreateCall(Fn, Args);
  NewCI->takeName(CallInfo.CI);
  NewCI->setDebugLoc(CallInfo.CI->getDebugLoc());
  CallInfo.CI->eraseFromParent();

  return NewCI;
}

// template <int width, int stride, typename T, int n>
// typename simd_type<T, width>::type
// __cm_intrinsic_impl_rdregion(typename simd_type<T, n>::type in, int offset);
//
llvm::Value *CGCMRuntime::HandleBuiltinRdregionImpl(CMCallInfo &CallInfo) {
  const CallExpr *CE = CallInfo.CE;
  auto CI = CallInfo.CI;
  unsigned Width = getIntegralValue(CE->getDirectCallee(), 0);
  unsigned Stride = getIntegralValue(CE->getDirectCallee(), 1);
  auto NewCI = EmitReadRegion1D(CallInfo.CGF->Builder, CI->getArgOperand(0),
                                Width, Stride, CI->getArgOperand(1));
  CI->replaceAllUsesWith(NewCI);
  CI->eraseFromParent();
  return NewCI;
}

// template <int width, int stride, typename T, int n, int m>
// typename simd_type<T, n>::type
// __cm_intrinsic_impl_wrregion(typename simd_type<T, n>::type oldVal,
//                              typename simd_type<T, m>::type newVal, int
//                              offset, typename mask_type<n>::type mask = 1);
//
llvm::Value *CGCMRuntime::HandleBuiltinWrregionImpl(CMCallInfo &CallInfo) {
  const CallExpr *CE = CallInfo.CE;
  auto CI = CallInfo.CI;
  unsigned Width = getIntegralValue(CE->getDirectCallee(), 0);
  unsigned Stride = getIntegralValue(CE->getDirectCallee(), 1);

  llvm::Value *Mask = CI->getArgOperand(3);
  auto *PredTy = cast<llvm::FixedVectorType>(Mask->getType());
  unsigned N = PredTy->getNumElements();
  llvm::Type *MaskTy = getMaskType(Mask->getContext(), N);
  Mask = CallInfo.CGF->Builder.CreateTrunc(Mask, MaskTy, ".trunc");
  auto NewCI = EmitWriteRegion1D(CallInfo.CGF->Builder, CI->getArgOperand(0),
                                 CI->getArgOperand(1), Width, Stride,
                                 CI->getArgOperand(2), Mask);
  CI->replaceAllUsesWith(NewCI);
  CI->eraseFromParent();
  return NewCI;
}

// template <typename T1, typename T2, typename T3, typename T4, int N>
// vector<T1, N> cm_dp4a(vector<T2, N> src0, vector<T3, N> src1, vector<T4, N>
// src2, int flag);
llvm::Value *CGCMRuntime::HandleBuiltinDP4AImpl(CMCallInfo &CallInfo,
                                                CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_dp4a_impl);
  auto CI = CallInfo.CI;
  llvm::Function *CalledFn = CI->getCalledFunction();
  llvm::Type *RetTy = CalledFn->getReturnType();

  llvm::Type *Tys[4] = {RetTy, CalledFn->getFunctionType()->getParamType(0),
                        CalledFn->getFunctionType()->getParamType(1),
                        CalledFn->getFunctionType()->getParamType(2)};

  llvm::Value *Flag = CI->getArgOperand(3);

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  // Check the saturation flag, compare it with 0.
  llvm::Value *CMP = Builder.CreateICmpEQ(
      Flag, llvm::Constant::getNullValue(Flag->getType()), "cmp");

  // non-saturated ID
  unsigned ID0 = GetGenxIntrinsicID(CallInfo, Kind, false);

  // saturated ID
  unsigned ID1 = GetGenxIntrinsicID(CallInfo, Kind, true);

  assert(CI->getNumArgOperands() == 4);
  llvm::Value *Args[3] = {CI->getArgOperand(0), CI->getArgOperand(1),
                          CI->getArgOperand(2)};

  llvm::Function *GenxFn0 = getGenXIntrinsic(ID0, Tys);
  llvm::CallInst *Result0 = Builder.CreateCall(GenxFn0, Args, CI->getName());
  Result0->setDebugLoc(CI->getDebugLoc());

  llvm::Function *GenxFn1 = getGenXIntrinsic(ID1, Tys);
  llvm::CallInst *Result1 = Builder.CreateCall(GenxFn1, Args, CI->getName());
  Result1->setDebugLoc(CI->getDebugLoc());

  // The final result depends on the saturation flag.
  llvm::Value *Result = Builder.CreateSelect(CMP, Result0, Result1);

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_bfn
///
/// template <typename T>
/// T __cm_intrinsic_impl_bfn(T s0, T s1, T s2, char bfval);
///
llvm::Value *CGCMRuntime::HandleBuiltinBFNImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_bfn_impl);

  const CallExpr *CE = CallInfo.CE;
  llvm::CallInst *CI = CallInfo.CI;
  CodeGenFunction &CGF = *CallInfo.CGF;
  CGBuilderTy Builder(CGF, CI);
  unsigned IntrinsicID = GetGenxIntrinsicID(CallInfo, Kind);

  assert(CE->getNumArgs() == 4);

  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);
  llvm::Value *Arg2 = CI->getArgOperand(2);
  llvm::Value *BFVAL = CI->getArgOperand(3);

  SmallVector<llvm::Type *, 8> Tys;
  Tys.push_back(CI->getType());
  Tys.push_back(Arg0->getType());

  SmallVector<llvm::Value *, 8> Args;

  Args.push_back(Arg0);
  Args.push_back(Arg1);
  Args.push_back(Arg2);
  Args.push_back(BFVAL);

  llvm::Function *F = getGenXIntrinsic(IntrinsicID, Tys);
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());
  CallInfo.CI->eraseFromParent();
  return Result;
}
// special case for dpas2
llvm::Value *CGCMRuntime::HandleBuiltinDPAS2Impl(CMCallInfo &CallInfo,
                                                 CMBuiltinKind Kind) {
  const CallExpr *CE = CallInfo.CE;
  CodeGenFunction &CGF = *CallInfo.CGF;

  assert(CE->getType()->isCMVectorType());
  assert(CE->getNumArgs() == 3);

  llvm::CallInst *CI = CallInfo.CI;

  llvm::SmallVector<llvm::Value *, 4> arguments;

  arguments.push_back(CI->getArgOperand(0));
  arguments.push_back(CI->getArgOperand(1));
  arguments.push_back(CI->getArgOperand(2));


  unsigned Src1Precision = getIntegralValue(CE->getDirectCallee(), 0);
  unsigned Src2Precision = getIntegralValue(CE->getDirectCallee(), 1);
  unsigned SystolicDepth = getIntegralValue(CE->getDirectCallee(), 2);
  unsigned RepeatCount = getIntegralValue(CE->getDirectCallee(), 3);

  // magic as in visa presisions are moved one upper...
  llvm::Value *ValSrc1Precision =
      llvm::ConstantInt::get(CGF.Int32Ty, Src1Precision + 1);
  llvm::Value *ValSrc2Precision =
      llvm::ConstantInt::get(CGF.Int32Ty, Src2Precision + 1);
  llvm::Value *ValSystolicDepth =
      llvm::ConstantInt::get(CGF.Int32Ty, SystolicDepth);
  llvm::Value *ValRepeatCount =
      llvm::ConstantInt::get(CGF.Int32Ty, RepeatCount);

  const clang::Type *dstTy = CE->getType().getTypePtr();
  const clang::Type *src0Ty = CE->getArg(0)->getType().getTypePtr();
  unsigned DstSign = dstTy->hasSignedIntegerRepresentation();
  unsigned Src0Sign = src0Ty->hasSignedIntegerRepresentation();
  llvm::Value *ValDstSign = llvm::ConstantInt::get(CGF.Int32Ty, DstSign);
  llvm::Value *ValSrc0Sign = llvm::ConstantInt::get(CGF.Int32Ty, Src0Sign);
  CGBuilderTy Builder(*CallInfo.CGF, CI);
  unsigned IntrinsicID = GetGenxIntrinsicID(CallInfo, Kind);
  assert(IntrinsicID == llvm::GenXIntrinsic::genx_dpas2); // error if return

  SmallVector<llvm::Type *, 8> Tys;
  Tys.push_back(CI->getType());
  // add as in dpas2 intrinsic type(dst) != type(src0)
  Tys.push_back(arguments[0]->getType());
  Tys.push_back(arguments[1]->getType());
  Tys.push_back(arguments[2]->getType());
  SmallVector<llvm::Value *, 8> Args;
  for (auto *arg : arguments)
    Args.push_back(arg);
  Args.push_back(ValSrc1Precision);
  Args.push_back(ValSrc2Precision);
  Args.push_back(ValSystolicDepth);
  Args.push_back(ValRepeatCount);
  Args.push_back(ValDstSign);
  Args.push_back(ValSrc0Sign);

  llvm::Function *F = getGenXIntrinsic(IntrinsicID, Tys);
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());
  CallInfo.CI->eraseFromParent();
  return Result;
}

// template <CmPrecisionType src1_precision, CmPrecisionType src2_precision, int
// systolic_depth,
//            typename T, typename T1, typename T2, int N, int N1, int N2,
//            typename T0>
// vector<T0, N> __cm_intrinsic_impl_dpas(vector<T, N> src0, vector<T1, N1>
// src1, vector<T2, N2> src2)
llvm::Value *CGCMRuntime::HandleBuiltinDPASImpl(CMCallInfo &CallInfo,
                                                CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_dpas_impl || Kind == CMBK_cm_dpasw_impl ||
         Kind == CMBK_cm_dpas2_impl || Kind == CMBK_cm_dpas_nosrc0_impl ||
         Kind == CMBK_cm_dpasw_nosrc0_impl);
  if (Kind == CMBK_cm_dpas2_impl)
    return HandleBuiltinDPAS2Impl(CallInfo, Kind);

  const CallExpr *CE = CallInfo.CE;
  CodeGenFunction &CGF = *CallInfo.CGF;

  assert(CE->getType()->isCMVectorType());
  assert(CE->getNumArgs() == 3);

  llvm::CallInst *CI = CallInfo.CI;

  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);
  llvm::Value *Arg2 = CI->getArgOperand(2);

  unsigned Src1Precision = getIntegralValue(CE->getDirectCallee(), 0);
  unsigned Src2Precision = getIntegralValue(CE->getDirectCallee(), 1);
  unsigned SystolicDepth = getIntegralValue(CE->getDirectCallee(), 2);
  unsigned RepeatCount = getIntegralValue(CE->getDirectCallee(), 3);

  auto fitsInByte = [&](StringRef Name, unsigned V) {
    if ((V & 0xff) == V)
      return true;

    Error(CallInfo.CE->getArg(0)->getExprLoc(),
          (Name + ": (" + std::to_string(V) + ") is out of the expected range")
              .str());
    return false;
  };

  if (!fitsInByte("Src1Precision", Src1Precision) ||
      !fitsInByte("Src2Precision", Src2Precision) ||
      !fitsInByte("SystolicDepth", SystolicDepth) ||
      !fitsInByte("RepeatCount", RepeatCount))
    return nullptr;

  uint32_t NewArgValue = (RepeatCount << 24) + (SystolicDepth << 16) +
                         ((++Src2Precision) << 8) + (++Src1Precision);
  llvm::Value *NewArg = llvm::ConstantInt::get(CGF.Int32Ty, NewArgValue);

  CGBuilderTy Builder(*CallInfo.CGF, CI);
  unsigned IntrinsicID = GetGenxIntrinsicID(CallInfo, Kind);

  SmallVector<llvm::Type *, 8> Tys;
  Tys.push_back(CI->getType());
  Tys.push_back(Arg1->getType());
  Tys.push_back(Arg2->getType());

  SmallVector<llvm::Value *, 8> Args;
  if (IntrinsicID == llvm::GenXIntrinsic::genx_dpas ||
      IntrinsicID == llvm::GenXIntrinsic::genx_dpasw) {
    Args.push_back(Arg0);
  }
  Args.push_back(Arg1);
  Args.push_back(Arg2);
  Args.push_back(NewArg);

  llvm::Function *F = getGenXIntrinsic(IntrinsicID, Tys);
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());
  CallInfo.CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_tf32_cvt.
///
/// template <typename T, typename T0, int N>
/// vector<T, N>
/// __cm_intrinsic_impl_tf32_cvt(vector<T0, N> src0)
///
llvm::Value *CGCMRuntime::HandleBuiltinTF32CVTImpl(CMCallInfo &CallInfo,
                                                   CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_tf32_cvt_impl);

  const CallExpr *CE = CallInfo.CE;
  assert(CE->getType()->isCMVectorMatrixType());
  assert(CE->getNumArgs() == 1);

  llvm::CallInst *CI = CallInfo.CI;
  CGBuilderTy Builder(*CallInfo.CGF, CI);

  llvm::Type *Tys[] = {CI->getType(), CI->getOperand(0)->getType()};
  llvm::Function *F = getGenXIntrinsic(llvm::GenXIntrinsic::genx_tf32_cvt, Tys);
  llvm::CallInst *Result =
      Builder.CreateCall(F, CI->getOperand(0), CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());

  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess builtin cm_srnd.
///
/// template <typename T0, typename T1, typename T2, int N>
/// vector<T0, N> __cm_intrinsic_impl_srnd(vector<T1, N> src0,
///                                        vector<T2, N> src1)
///
llvm::Value *CGCMRuntime::HandleBuiltinSRNDImpl(CMCallInfo &CallInfo,
                                                CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_srnd_impl);

  const CallExpr *CE = CallInfo.CE;

  assert(CE->getType()->isCMVectorMatrixType());
  assert(CE->getNumArgs() == 2);

  llvm::CallInst *CI = CallInfo.CI;
  llvm::Value *Arg0 = CI->getArgOperand(0);
  llvm::Value *Arg1 = CI->getArgOperand(1);

  SmallVector<llvm::Type *, 8> Tys;
  Tys.push_back(CI->getType());
  Tys.push_back(Arg0->getType());
  Tys.push_back(Arg1->getType());

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  SmallVector<llvm::Value *, 8> Args;
  Args.push_back(Arg0);
  Args.push_back(Arg1);

  llvm::Function *F = getGenXIntrinsic(llvm::GenXIntrinsic::genx_srnd, Tys);
  llvm::CallInst *Result = Builder.CreateCall(F, Args, CI->getName());
  Result->setDebugLoc(CI->getDebugLoc());
  CallInfo.CI->eraseFromParent();
  return Result;
}


#define LSCINC
#include "CMLSCDef.h"
#undef LSCINC

/// \brief Postprocess block 2d builtins load/store/prefetch.
///
/// template <typename T, int NBlocks, int Width, int Height, bool Transposed,
///           bool Transformed, CacheHint L1H, CacheHint L3H, int N>
/// vector<T, N>
/// __cm_intrinsic_impl_load2d(uint64_t Base, unsigned SurfaceWidth,
///                            unsigned SurfaceHeight, unsigned SurfacePitch,
///                            int X, int Y);
///
/// template <typename T, int NBlocks, int Width, int Height,
///           CacheHint L1H, CacheHint L3H, int N>
/// void __cm_intrinsic_impl_store2d(uint64_t Base, unsigned SurfaceWidth,
///                                  unsigned SurfaceHeight, unsigned
///                                  SurfacePitch, int X, int Y, vector<T, N>
///                                  Data);
///
/// template <typename T, int NBlocks, int Width, int Height, CacheHint L1H,
///           CacheHint L3H>
/// void __cm_intrinsic_impl_prefetch2d(uint64_t Base, unsigned SurfaceWidth,
///                                     unsigned SurfaceHeight, unsigned
///                                     SurfacePitch, int X, int Y);
///
llvm::Value *CGCMRuntime::HandleBuiltinLSC2dImpl(CMCallInfo &CallInfo,
                                                 CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_block_load2d_flat_impl ||
         Kind == CMBK_cm_block_prefetch2d_flat_impl ||
         Kind == CMBK_cm_block_store2d_flat_impl);

  auto &CGF = *CallInfo.CGF;
  const FunctionDecl *FD = CallInfo.CE->getDirectCallee();
  assert(FD && FD->isTemplateInstantiation());
  unsigned NBlocks = getIntegralValue(FD, 1);
  unsigned Width = getIntegralValue(FD, 2);
  unsigned Height = getIntegralValue(FD, 3);
  auto L1H = CacheHint::Default;
  auto L3H = CacheHint::Default;
  auto Tranposed = LSC_DATA_ORDER::LSC_DATA_ORDER_NONTRANSPOSE;
  uint8_t Transformed = 0;

  switch (Kind) {
  default:
    break;
  case CMBK_cm_block_load2d_flat_impl:
    Tranposed = getIntegralValue(FD, 4)
                    ? LSC_DATA_ORDER::LSC_DATA_ORDER_TRANSPOSE
                    : LSC_DATA_ORDER::LSC_DATA_ORDER_NONTRANSPOSE;
    Transformed = getIntegralValue<uint8_t>(FD, 5);
    L1H = static_cast<CacheHint>(getIntegralValue(FD, 6));
    L3H = static_cast<CacheHint>(getIntegralValue(FD, 7));
    break;
  case CMBK_cm_block_prefetch2d_flat_impl:
  case CMBK_cm_block_store2d_flat_impl:
    L1H = static_cast<CacheHint>(getIntegralValue(FD, 4));
    L3H = static_cast<CacheHint>(getIntegralValue(FD, 5));
    break;
  }

  DataSize DS = DataSize::U8;
  {
    const TemplateArgumentList *TempArgs = FD->getTemplateSpecializationArgs();
    QualType T = TempArgs->get(0).getAsType();
    size_t SizeInBits = CGM.getContext().getTypeSize(T);
    switch (SizeInBits) {
    default:
      CGM.ErrorUnsupported(FD, "unsupported template type");
      break;
    case 8:
      DS = DataSize::U8;
      break;
    case 16:
      DS = DataSize::U16;
      break;
    case 32:
      DS = DataSize::U32;
      break;
    case 64:
      DS = DataSize::U64;
      break;
    }
  }

  llvm::CallInst *CI = CallInfo.CI;
  CGBuilderTy Builder(*CallInfo.CGF, CI);

  llvm::Value *Pred = Builder.getTrue();
  llvm::Value *Addr = CI->getArgOperand(0);

  std::vector<llvm::Value *> Args = {
      Pred,
      Builder.getInt8(static_cast<uint8_t>(L1H)),
      Builder.getInt8(static_cast<uint8_t>(L3H)),
      Builder.getInt8(static_cast<uint8_t>(DS)),
      Builder.getInt8(static_cast<uint8_t>(Tranposed)),
      Builder.getInt8(static_cast<uint8_t>(NBlocks)),
      Builder.getInt16(static_cast<uint16_t>(Width)),
      Builder.getInt16(static_cast<uint16_t>(Height)),
      Builder.getInt8(static_cast<uint8_t>(Transformed)), // VNNI
      Addr,                                               // Surface base
      CI->getArgOperand(1),                               // Surface width
      CI->getArgOperand(2),                               // Surface height
      CI->getArgOperand(3),                               // Surface pitch
      CI->getArgOperand(4),                               // X
      CI->getArgOperand(5)                                // Y
  };

  assert((Addr->getType() == CGF.Int32Ty) || (Addr->getType() == CGF.Int64Ty));

  auto ID = llvm::GenXIntrinsic::genx_lsc_load2d_stateless;
  std::vector<llvm::Type *> Tys;
  if (Kind == CMBK_cm_block_load2d_flat_impl) {
    Tys.push_back(CI->getType());   // return type
    Tys.push_back(Pred->getType()); // predicate
    Tys.push_back(Addr->getType()); // address type (i32/i64)
  } else if (Kind == CMBK_cm_block_prefetch2d_flat_impl) {
    Tys.push_back(Pred->getType()); // predicate
    Tys.push_back(Addr->getType()); // address type (i32/i64)
    ID = llvm::GenXIntrinsic::genx_lsc_prefetch2d_stateless;
  } else {
    Tys.push_back(Pred->getType());                 // predicate
    Tys.push_back(Addr->getType());                 // address type (i32/i64)
    Tys.push_back(CI->getArgOperand(6)->getType()); // data type
    ID = llvm::GenXIntrinsic::genx_lsc_store2d_stateless;
    Args.push_back(CI->getArgOperand(6)); // Data to write
  }

  llvm::Function *F = getGenXIntrinsic(ID, Tys);
  auto NewCI = Builder.CreateCall(F, Args);
  NewCI->setDebugLoc(CI->getDebugLoc());
  if (!CI->getType()->isVoidTy())
    CI->replaceAllUsesWith(NewCI);
  CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess basic LSC messages
llvm::Value *CGCMRuntime::HandleBuiltinLSCImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind) {
  // main config from builtin kind
  const int *Config = getConfig(Kind);
  LDTYPE Ldt = getOpType(Kind);
  SFTYPE Sft = getSFType(Kind);
  bool IsBlock = getBlock(Kind);

  // params from function decl
  LSCParams Params{CallInfo, Config, Kind};
  CGBuilderTy Builder(*CallInfo.CGF, Params.CI_);

  // special logic for subopcode for atomics
  if (Ldt != ATOMIC) {
    LSC_SubOpcode Op;
    Op = getSubOp(Kind);
    Params.setOp(Op);
  }

  // special logic for SLM Idx
  if (Sft == SLM)
    Params.Idx_ = Builder.getInt32(0);

  // special logic for pred
  if (IsBlock)
    Params.Pred_ = Builder.getTrue();
  else {
    assert(Params.Pred_ && Params.Pred_->getType()->isVectorTy() &&
           "We expect vector Pred if exists");
    auto *PredTy = cast<llvm::FixedVectorType>(Params.Pred_->getType());
    auto *VTy = llvm::FixedVectorType::get(Builder.getInt1Ty(),
                                           PredTy->getNumElements());
    Params.Pred_ = Builder.CreateTruncOrBitCast(Params.Pred_, VTy);
  }

  // special logic for transpose
  // LSC_DATA_ORDER_TRANSPOSE requires ExecSize of 1
  if (!IsBlock)
    Params.setNonTranspose();

  // LSC_DATA_ELEMS_1 illegal on transposed operation
  if (Params.VS_ == static_cast<unsigned char>(VectorSize::N1))
    Params.setNonTranspose();

  // Special logic for flat addresses
  if (Sft == FLAT) {
    llvm::Value *BaseAddr = Params.Idx_;
    llvm::Value *Offsets = Params.Offset_;
    assert(BaseAddr && Offsets);
    // Base Addr shall be int64 in all cases
    auto Int64Ty = CallInfo.CGF->Int64Ty;
    if (BaseAddr->getType() != Int64Ty) {
      if (BaseAddr->getType()->isPointerTy())
        BaseAddr = Builder.CreatePtrToInt(BaseAddr, Int64Ty);
      else
        BaseAddr = Builder.CreateZExt(BaseAddr, Int64Ty);
    }

    // Creating add off = off + baseaddr with possible extend to vectors
    if (IsBlock) {
      if (Offsets->getType() != Int64Ty)
        Offsets = Builder.CreateZExt(Offsets, Int64Ty);
      Offsets = Builder.CreateAdd(Offsets, Params.Idx_);
    } else {
      auto *OffsetsTy = cast<llvm::FixedVectorType>(Offsets->getType());
      auto N = OffsetsTy->getNumElements();

      // Convert the offest type to UD if it is not.
      if (OffsetsTy->getElementType() != Int64Ty) {
        llvm::Type *Ty = llvm::FixedVectorType::get(Int64Ty, N);
        Offsets = Builder.CreateZExt(Offsets, Ty);
      }
      // workaround, hw does not really have a global-address, add
      // base-address and the offset
      llvm::Value *Splat = Builder.CreateVectorSplat(N, BaseAddr);
      Offsets = Builder.CreateAdd(Offsets, Splat);
    }

    // in flat base addr ignored after used
    Params.Idx_ = Builder.getInt32(0);
    Params.Offset_ = Offsets;
  }

  // undef value for atomic oldval
  llvm::Value *OldVal = llvm::UndefValue::get(Params.CI_->getType());

  // undef sources if null
  if (Ldt == ATOMIC) {
    if (Params.Data_ == nullptr)
      Params.Data_ = OldVal;
    if (Params.Data1_ == nullptr)
      Params.Data1_ = OldVal;
  }

  SmallVector<llvm::Type *, 3> Tys;

  if (Ldt == LOAD || Ldt == ATOMIC)
    Tys.push_back(Params.CI_->getType());

  assert(Params.Pred_ && Params.Offset_);
  Tys.push_back(Params.Pred_->getType());
  Tys.push_back(Params.Offset_->getType());

  if (Ldt == STORE) {
    assert(Params.Data_);
    Tys.push_back(Params.Data_->getType());
  }

  // if channel mask not specified, it is constant zero
  if (Params.ChM_ == nullptr)
    Params.ChM_ = Builder.getInt8(0);
  else {
    // making char from enum
    auto Int8Ty = CallInfo.CGF->Int8Ty;
    if (Params.ChM_->getType() != Int8Ty)
      Params.ChM_ = Builder.CreateTruncOrBitCast(Params.ChM_, Int8Ty);
  }

  llvm::Function *F = getGenXIntrinsic(getLSCIntrinsic(Kind), Tys);

  SmallVector<llvm::Value *, 12> Args;
  Args.push_back(Params.Pred_);                        // Pred
  Args.push_back(Builder.getInt8(Params.Op_));         // Subop
  Args.push_back(Builder.getInt8(Params.L1H_));        // L1H
  Args.push_back(Builder.getInt8(Params.L3H_));        // L3H
  Args.push_back(Builder.getInt16(1));                 // Addr scale
  Args.push_back(Builder.getInt32(Params.ImmOffset_)); // imm offset
  Args.push_back(Builder.getInt8(Params.DS_));         // Datum size
  Args.push_back(Builder.getInt8(Params.VS_));         // Vector size
  Args.push_back(Builder.getInt8(Params.Transposed_)); // transposed
  Args.push_back(Params.ChM_);                         // channel mask
  Args.push_back(Params.Offset_);                      // offsets
  if (Ldt == STORE)
    Args.push_back(Params.Data_); // data
  if (Ldt == ATOMIC) {
    Args.push_back(Params.Data_);  // src0 or undef
    Args.push_back(Params.Data1_); // src1 or undef
  }
  Args.push_back(Params.Idx_); // surface
  if (Ldt == ATOMIC)
    Args.push_back(OldVal); // oldval

  auto NewCI = Builder.CreateCall(F, Args);
  NewCI->setDebugLoc(Params.CI_->getDebugLoc());
  Params.CI_->replaceAllUsesWith(NewCI);
  Params.CI_->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess LSC Xe2+ typed implementation builtins.
///
/// template <ChannelMaskType Mask, CacheHint L1H, CacheHint L3H, int N,
///           typename RetTy>
/// RetTy __cm_intrinsic_impl_load4_typed_bti(
///     vector<ushort, N> Pred, SurfaceIndex Image, vector<unsigned, N> U,
///     vector<unsigned, N> V, vector<unsigned, N> R, vector<unsigned, N> LOD,
///     RetTy MergeData);
/// template <ChannelMaskType Mask, CacheHint L1H, CacheHint L3H, int N,
///           typename DataTy>
/// void __cm_intrinsic_impl_store4_typed_bti(
///     vector<ushort, N> Pred, SurfaceIndex Image, vector<unsigned, N> U,
///     vector<unsigned, N> V, vector<unsigned, N> R, vector<unsigned, N> LOD,
///     DataTy StoreData);
/// template <ChannelMaskType Mask, CacheHint L1H, CacheHint L3H, int N>
/// void __cm_intrinsic_impl_prefetch4_typed_bti(
///     vector<ushort, N> Pred, SurfaceIndex Image, vector<unsigned, N> U,
///     vector<unsigned, N> V, vector<unsigned, N> R, vector<unsigned, N> LOD);
///
llvm::Value *CGCMRuntime::HandleBuiltinLSCTypedImpl(CMCallInfo &CallInfo,
                                                    CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_load4_typed_impl ||
         Kind == CMBK_cm_prefetch4_typed_impl ||
         Kind == CMBK_cm_store4_typed_impl);

  const FunctionDecl *FD = CallInfo.CE->getDirectCallee();
  assert(FD && FD->isTemplateInstantiation());

  llvm::CallInst *CI = CallInfo.CI;
  CGBuilderTy Builder(*CallInfo.CGF, CI);

  unsigned ChMask = getIntegralValue(FD, 0);
  unsigned L1H = getIntegralValue(FD, 1);
  unsigned L3H = getIntegralValue(FD, 2);
  unsigned SimdWidth = getIntegralValue(FD, 3);

  llvm::Value *Pred = CI->getArgOperand(0);
  llvm::Value *BTI = CI->getArgOperand(1);
  llvm::Value *U = CI->getArgOperand(2);
  llvm::Value *V = CI->getArgOperand(3);
  llvm::Value *R = CI->getArgOperand(4);
  llvm::Value *LOD = CI->getArgOperand(5);

  llvm::Type *PredTy =
      llvm::FixedVectorType::get(Builder.getInt1Ty(), SimdWidth);
  llvm::Type *AddrTy = U->getType();
  llvm::Type *DataTy = nullptr;

  SmallVector<llvm::Value *, 10> Args = {
      Builder.CreateTrunc(Pred, PredTy),
      Builder.getInt8(L1H),
      Builder.getInt8(L3H),
      Builder.getInt8(ChMask),
      BTI,
      U,
      V,
      R,
      LOD,
  };

  if (Kind != CMBK_cm_prefetch4_typed_impl) {
    llvm::Value *Src = CI->getArgOperand(6);
    DataTy = Src->getType();
    Args.push_back(Src);
    assert(DataTy->isVectorTy());
  }

  llvm::Function *F = nullptr;
  switch (Kind) {
  default:
    llvm_unreachable("Unsupported function");
  case CMBK_cm_load4_typed_impl:
    F = getGenXIntrinsic(
        llvm::GenXIntrinsic::genx_lsc_load_merge_quad_typed_bti,
        {DataTy, PredTy, AddrTy});
    break;
  case CMBK_cm_store4_typed_impl:
    F = getGenXIntrinsic(llvm::GenXIntrinsic::genx_lsc_store_quad_typed_bti,
                         {PredTy, AddrTy, DataTy});
    break;
  case CMBK_cm_prefetch4_typed_impl:
    F = getGenXIntrinsic(llvm::GenXIntrinsic::genx_lsc_prefetch_quad_typed_bti,
                         {PredTy, AddrTy});
    break;
  }

  auto *NewCI = Builder.CreateCall(F, Args);
  NewCI->setDebugLoc(CI->getDebugLoc());
  if (!CI->getType()->isVoidTy())
    CI->replaceAllUsesWith(NewCI);
  CI->eraseFromParent();

  return NewCI;
}

/// \brief Postprocess LSC Xe2+ typed 2d implementation builtins.
///        It is separate handler because message is different and unfication
///        is rather hard. To rethink unification across messages
///
/// template <typename T, int Width, int Height, CacheHint L1H, CacheHint L3H>
/// matrix<T, Width, Height>
/// __cm_intrinsic_impl_load2d_bti(SurfaceIndex, int, int);
///
/// template <typename T, int Width, int Height, CacheHint L1H, CacheHint L3H>
/// void
/// __cm_intrinsic_impl_prefetch2d_bti(SurfaceIndex, int, int);
///
/// template <typename T, int Width, int Height, CacheHint L1H, CacheHint L3H>
/// void
/// __cm_intrinsic_impl_store2d_bti(SurfaceIndex, int, int, matrix<T, Width,
/// Height>);
///
llvm::Value *CGCMRuntime::HandleBuiltinLSCTyped2DImpl(CMCallInfo &CallInfo,
                                                      CMBuiltinKind Kind) {

  assert(Kind == CMBK_cm_load2d_bti_impl ||
         Kind == CMBK_cm_prefetch2d_bti_impl ||
         Kind == CMBK_cm_store2d_bti_impl);

  const FunctionDecl *FD = CallInfo.CE->getDirectCallee();
  assert(FD && FD->isTemplateInstantiation());

  llvm::CallInst *CI = CallInfo.CI;
  CGBuilderTy Builder(*CallInfo.CGF, CI);

  unsigned Height = getIntegralValue(FD, 1);
  unsigned Width = getIntegralValue(FD, 2);
  unsigned char L1H = getIntegralValue(FD, 3);
  unsigned char L3H = getIntegralValue(FD, 4);
  llvm::Value *Surface = CI->getArgOperand(0);
  llvm::Value *XOff = CI->getArgOperand(1);
  llvm::Value *YOff = CI->getArgOperand(2);
  llvm::Value *Data = nullptr;

  if (Kind == CMBK_cm_store2d_bti_impl)
    Data = CI->getArgOperand(3);

  auto ID = llvm::GenXIntrinsic::genx_lsc_load2d_typed_bti;
  if (Kind == CMBK_cm_store2d_bti_impl)
    ID = llvm::GenXIntrinsic::genx_lsc_store2d_typed_bti;

  SmallVector<llvm::Type *, 1> Tys;

  if (Kind == CMBK_cm_load2d_bti_impl)
    Tys.push_back(CI->getType());
  else if (Kind == CMBK_cm_store2d_bti_impl)
    Tys.push_back(Data->getType());

  llvm::Function *F = getGenXIntrinsic(ID, Tys);

  SmallVector<llvm::Value *, 8> Args;
  Args.push_back(Builder.getInt8(L1H));
  Args.push_back(Builder.getInt8(L3H));
  Args.push_back(Surface);
  Args.push_back(Builder.getInt32(Height));
  Args.push_back(Builder.getInt32(Width));
  Args.push_back(XOff);
  Args.push_back(YOff);

  if (Kind == CMBK_cm_store2d_bti_impl)
    Args.push_back(Data);

  auto NewCI = Builder.CreateCall(F, Args);
  NewCI->setDebugLoc(CI->getDebugLoc());
  if (!CI->getType()->isVoidTy())
    CI->replaceAllUsesWith(NewCI);
  CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess cm_fence implementation builtins.
//
// template <LSC_SFID Sfid,
//           LSC_FENCE_OP FenceOp,
//           LSC_SCOPE Scope>
// RetTy __cm_intrinsic_impl_lsc_fence(vector<ushort, 32> Pred);
//

llvm::Value *CGCMRuntime::HandleBuiltinLscFenceImpl(CMCallInfo &CallInfo,
                                                    CMBuiltinKind Kind) {
  assert(Kind == CMBK_cm_lsc_fence_impl);
  const FunctionDecl *FD = CallInfo.CE->getDirectCallee();
  auto Sfid = static_cast<LSC_SFID>(getIntegralValue(FD, 0));
  auto FenceOp = static_cast<LSC_FENCE_OP>(getIntegralValue(FD, 1));
  auto Scope = static_cast<LSC_SCOPE>(getIntegralValue(FD, 2));
  llvm::CallInst *CI = CallInfo.CI;
  CGBuilderTy Builder(*CallInfo.CGF, CI);
  // Convert predicate from N x i16 to N x i1
  llvm::Value *Pred = CI->getArgOperand(0);
  {
    auto *PredTy = cast<llvm::FixedVectorType>(Pred->getType());
    auto *VTy = llvm::FixedVectorType::get(Builder.getInt1Ty(),
                                           PredTy->getNumElements());
    Pred = Builder.CreateTruncOrBitCast(Pred, VTy);
  }
  llvm::Type *Tys[] = {
      Pred->getType() // predicate
  };

  llvm::Function *F =
      getGenXIntrinsic(llvm::GenXIntrinsic::genx_lsc_fence, Tys);
  SmallVector<llvm::Value *, 4> Args;
  Args.push_back(Pred);                                           // predicate
  Args.push_back(Builder.getInt8(static_cast<uint8_t>(Sfid)));    // SFID
  Args.push_back(Builder.getInt8(static_cast<uint8_t>(FenceOp))); // FenceOp
  Args.push_back(Builder.getInt8(static_cast<uint8_t>(Scope)));   // Scope

  auto NewCI = Builder.CreateCall(F, Args);
  NewCI->setDebugLoc(CI->getDebugLoc());
  llvm::Value *Result = nullptr;
  if (!CI->getType()->isVoidTy()) {
    Result = NewCI;
    CI->replaceAllUsesWith(NewCI);
  }
  CI->eraseFromParent();
  return Result;
}

/// \brief Postprocess scatter implementation.
//
// template <typename T, int N, int A>
// void __cm_builtin_impl_scatter(vector<T, N> data, vector<T*, N> ptrs,
//                                vector<ushort, N> mask);
//
void CGCMRuntime::HandleBuiltinScatterImpl(CMCallInfo &CallInfo,
                                           CMBuiltinKind Kind) {
  assert(Kind == CMBK_scatter_impl || Kind == CMBK_scatter_svm_impl);

  llvm::CallInst *CI = CallInfo.CI;

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  llvm::Value *Val = CI->getArgOperand(0);
  llvm::Value *Addr = CI->getArgOperand(1);
  llvm::Value *Mask = CI->getArgOperand(2);
  {
    auto *MaskTy = cast<llvm::FixedVectorType>(Mask->getType());
    auto *VTy = llvm::FixedVectorType::get(Builder.getInt1Ty(),
                                           MaskTy->getNumElements());
    Mask = Builder.CreateTruncOrBitCast(Mask, VTy);
  }

  uint32_t Alignment = getIntegralValue(CallInfo.CE->getDirectCallee(), 2);
  // We reserved zero for element size alignment.
  if (!Alignment)
    Alignment = cast<llvm::FixedVectorType>(Val->getType())
                    ->getElementType()
                    ->getPrimitiveSizeInBits() /
                8;

  if (Kind == CMBK_scatter_svm_impl) {
    llvm::Type *ElemTy =
        cast<llvm::FixedVectorType>(Val->getType())->getElementType();
    auto *AddrTy = cast<llvm::FixedVectorType>(Addr->getType());
    llvm::Type *VPtrTy = llvm::FixedVectorType::get(
        llvm::PointerType::get(ElemTy, 1), AddrTy->getNumElements());
    Addr = Builder.CreateIntToPtr(Addr, VPtrTy);
  }

  auto NewCI = Builder.CreateMaskedScatter(Val, Addr, Alignment, Mask);
  NewCI->setDebugLoc(CI->getDebugLoc());
  CI->eraseFromParent();
}

/// \brief Postprocess gather implementation.
//
// template <typename T, int N, int A>
// vector<T, N> __cm_builtin_impl_gather(vector<T*, N> ptrs, vector<ushort, N>
// mask,
//                                       vector<T, N> passthru);
//
llvm::Value *CGCMRuntime::HandleBuiltinGatherImpl(CMCallInfo &CallInfo,
                                                  CMBuiltinKind Kind) {
  assert(Kind == CMBK_gather_impl || Kind == CMBK_gather_svm_impl);

  llvm::CallInst *CI = CallInfo.CI;

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  llvm::Value *Addr = CI->getArgOperand(0);
  llvm::Value *Passthru = CI->getArgOperand(2);
  llvm::Value *Mask = CI->getArgOperand(1);
  {
    auto *MaskTy = cast<llvm::FixedVectorType>(Mask->getType());
    auto *VTy = llvm::FixedVectorType::get(Builder.getInt1Ty(),
                                           MaskTy->getNumElements());
    Mask = Builder.CreateTruncOrBitCast(Mask, VTy);
  }

  uint32_t Alignment = getIntegralValue(CallInfo.CE->getDirectCallee(), 2);
  // We reserved zero for element size alignment.
  if (!Alignment)
    Alignment = cast<llvm::FixedVectorType>(Passthru->getType())
                    ->getElementType()
                    ->getPrimitiveSizeInBits() /
                8;

  if (Kind == CMBK_gather_svm_impl) {
    llvm::Type *ElemTy =
        cast<llvm::FixedVectorType>(Passthru->getType())->getElementType();
    auto *AddrTy = cast<llvm::FixedVectorType>(Addr->getType());
    llvm::Type *VPtrTy = llvm::FixedVectorType::get(
        llvm::PointerType::get(ElemTy, 1), AddrTy->getNumElements());
    Addr = Builder.CreateIntToPtr(Addr, VPtrTy);
  }

  auto NewCI = Builder.CreateMaskedGather(Addr, Alignment, Mask, Passthru);
  NewCI->setDebugLoc(CI->getDebugLoc());
  CI->replaceAllUsesWith(NewCI);
  CI->eraseFromParent();
  return NewCI;
}

/// \brief Postprocess store implementation.
//
// template <typename T, int A>
// void __cm_builtin_impl_store(T val, T *ptr);
//
void CGCMRuntime::HandleBuiltinStoreImpl(CMCallInfo &CallInfo,
                                         CMBuiltinKind Kind) {
  assert(Kind == CMBK_store_impl);

  llvm::CallInst *CI = CallInfo.CI;

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  llvm::Value *Val = CI->getArgOperand(0);
  llvm::Value *Addr = CI->getArgOperand(1);

  uint32_t Alignment = getIntegralValue(CallInfo.CE->getDirectCallee(), 1);
  // We reserved zero for element size alignment.
  if (!Alignment) {
    llvm::Type *ValTy = Val->getType();
    llvm::Type *ElemTy =
        ValTy->isVectorTy()
            ? cast<llvm::FixedVectorType>(ValTy)->getElementType()
            : ValTy;
    Alignment = ElemTy->getPrimitiveSizeInBits() / 8;
  }

  auto NewCI = Builder.CreateAlignedStore(Val, Addr, Alignment);
  NewCI->setDebugLoc(CI->getDebugLoc());
  CI->eraseFromParent();
}

/// \brief Postprocess load implementation.
//
// template <typename T, int A>
// T __cm_builtin_impl_load(T *ptr);
//
llvm::Value *CGCMRuntime::HandleBuiltinLoadImpl(CMCallInfo &CallInfo,
                                                CMBuiltinKind Kind) {
  assert(Kind == CMBK_load_impl);

  llvm::CallInst *CI = CallInfo.CI;

  CGBuilderTy Builder(*CallInfo.CGF, CI);

  llvm::Value *Addr = CI->getArgOperand(0);

  uint32_t Alignment = getIntegralValue(CallInfo.CE->getDirectCallee(), 1);
  // We reserved zero for element size alignment.
  if (!Alignment) {
    llvm::Type *ValTy = CI->getType();
    llvm::Type *ElemTy =
        ValTy->isVectorTy()
            ? cast<llvm::FixedVectorType>(ValTy)->getElementType()
            : ValTy;
    Alignment = ElemTy->getPrimitiveSizeInBits() / 8;
  }

  auto NewCI = Builder.CreateAlignedLoad(CI->getType(), Addr, Alignment);
  NewCI->setDebugLoc(CI->getDebugLoc());
  CI->replaceAllUsesWith(NewCI);
  CI->eraseFromParent();
  return NewCI;
}

/// \brief Emit one of scatter_scaled, scatter4_scaled.
///
/// \param Selector log2 number of blocks for scatter_scaled
///                 or giving mask for scatter4_scaled.
///
llvm::CallInst *
CGCMRuntime::EmitScatterScaled(CodeGenFunction &CGF, unsigned IntrinsicID,
                               llvm::APInt Selector, unsigned Scale,
                               llvm::Value *Surface, llvm::Value *GlobalOffset,
                               llvm::Value *ElementOffset, llvm::Value *Data) {
  assert((IntrinsicID == llvm::GenXIntrinsic::genx_scatter_scaled ||
          IntrinsicID == llvm::GenXIntrinsic::genx_scatter4_scaled) &&
         "Expected gather intrinsics");
  // Types for overloading.
  SmallVector<llvm::Type *, 3> Tys;
  auto *ElementOffsetTy = cast<llvm::FixedVectorType>(ElementOffset->getType());
  Tys.push_back(getMaskType(ElementOffset->getContext(),
                            ElementOffsetTy->getNumElements())); // predicate
  Tys.push_back(ElementOffsetTy);          // element offset
  Tys.push_back(Data->getType());          // data type for scatter

  llvm::Function *Fn = getGenXIntrinsic(IntrinsicID, Tys);
  llvm::FunctionType *FTy = Fn->getFunctionType();

  // Arguments.
  llvm::Value *Args[] = {
      llvm::Constant::getAllOnesValue(FTy->getParamType(0)), // predicate
      llvm::ConstantInt::get(CGF.Int32Ty, Selector),
      llvm::ConstantInt::get(CGF.Int16Ty, Scale),
      Surface,
      GlobalOffset,
      ElementOffset,
      Data};
  return CGF.Builder.CreateCall(Fn, Args);
}

/// \brief Emit one of gather_scaled, scatter_scaled,
///    gather4_scaled, scatter4_scaled.
///
/// \param Selector log2 number of blocks for gather_scaled
///                 or giving mask for gather4_scaled.
///
llvm::CallInst *CGCMRuntime::EmitGatherScaled(
    CodeGenFunction &CGF, unsigned IntrinsicID, llvm::APInt Selector,
    unsigned Scale, llvm::Value *Surface, llvm::Value *GlobalOffset,
    llvm::Value *ElementOffset, llvm::Value *Data, llvm::Value *Mask) {
  assert((IntrinsicID == llvm::GenXIntrinsic::genx_gather_masked_scaled2 ||
          IntrinsicID == llvm::GenXIntrinsic::genx_gather4_masked_scaled2) &&
         "Expected gather intrinsics");

  // By default Mask is all ones value, unless specified.
  if (Mask == nullptr) {
    auto *ElementOffsetTy = cast<llvm::FixedVectorType>(ElementOffset->getType());
    llvm::Type *MaskTy =
        getMaskType(CGF.getLLVMContext(), ElementOffsetTy->getNumElements());
    Mask = llvm::Constant::getAllOnesValue(MaskTy);
  }

  // Types for overloading.
  SmallVector<llvm::Type *, 3> Tys;
  Tys.push_back(Data->getType());          // return type for gather
  Tys.push_back(ElementOffset->getType()); // element offset
  Tys.push_back(Mask->getType());

  llvm::Function *Fn = getGenXIntrinsic(IntrinsicID, Tys);

  // Arguments.
  llvm::Value *Args[] = {llvm::ConstantInt::get(CGF.Int32Ty, Selector),
                         llvm::ConstantInt::get(CGF.Int16Ty, Scale),
                         Surface,
                         GlobalOffset,
                         ElementOffset,
                         Mask};
  return CGF.Builder.CreateCall(Fn, Args);
}
