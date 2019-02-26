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
// This provides an interface class for CM code generation.
//===----------------------------------------------------------------------===//

#ifndef CLANG_CODEGEN_CM_RUNTIME_H
#define CLANG_CODEGEN_CM_RUNTIME_H

#include "CGBuilder.h"
#include "CGCall.h"
#include "CGCMBuiltin.h"
#include "CodeGenFunction.h"
#include "clang/AST/Type.h"
#include "llvm/IR/Type.h"

namespace clang {
class CallExpr;

namespace CodeGen {

class CodeGenFunction;
class CodeGenModule;

/// \brief Class with information about how a region should be accessed.
///
/// This class describes a region for CM specific expressions, e.g. select.
/// Since CM expressions can be nested, this will create a stack-like data
/// structure. For example,
///
/// \code
/// matrix<float, 8, 8> m = 0;
/// m.select<4, 1, 8, 1>(0, 0).select<4, 1, 4, 1>(0, 0) = 1;
/// \code
///
/// two region info objects will be created, each for a select expression:
///
/// R1 : {m, 4, 1, 8, 4; 0, 0}
/// R2 : {R1, 4, 1, 4, 1; 0, 0}
///
/// These regions could be collapsed when indices are constants, but we leave
/// this to an llvm optimization pass. Each index will be evaluated once,
/// results will be cached, and evaluation order will be kept as if they are
/// arguments in function calls. Note that argument evaluation order is
/// unspecified.
///
class CGCMRegionInfo {
public:
  enum RegionKind {
    RK_select, // region constructed out of a select like op
    RK_format  // region constructed out of a format
  };

private:
  RegionKind Kind;

  /// \brief The base object. This base object could be another CM region
  /// LValue.
  LValue Base;

  /// vSize, vStride, hSize, hStride.
  /// for vectors, vSize and vStride are zero.
  unsigned Dims[4];

  /// vOffset, hOffset;
  llvm::Value *Indices[2];

  CGCMRegionInfo(CGCMRegionInfo &) = delete;

public:
  CGCMRegionInfo(RegionKind Kind, LValue Base, unsigned vSize, unsigned vStride,
                 unsigned hSize, unsigned hStride, llvm::Value *vOffset,
                 llvm::Value *hOffset);

  CGCMRegionInfo(RegionKind Kind, LValue Base, unsigned Size, unsigned Stride,
                 llvm::Value *Offset);

  CGCMRegionInfo(RegionKind Kind, LValue Base);

  RegionKind getRegionKind() const { return Kind; }
  bool isFormat() const { return getRegionKind() == RK_format; }
  bool isSelect() const { return getRegionKind() == RK_select; }

  LValue getBase() const { return Base; }

  /// \brief Returns the base width in number of elements. Only for 2D base.
  unsigned getBaseWidth() const;

  /// \brief Returns the base height in number of elements. Only for 2D base.
  unsigned getBaseHeight() const;

  /// \brief Returns the base size in number of elements. Only for 1D base.
  unsigned getBaseSize() const;

  bool is2DRegion() const { return getHSize() != 0; }
  bool is1DRegion() const { return !is2DRegion(); }

  unsigned getVSize() const { return Dims[0]; }
  unsigned getVStride() const { return Dims[1]; }
  unsigned getHSize() const { return Dims[2]; }
  unsigned getHStride() const { return Dims[3]; }

  void setVSize(unsigned Val) { Dims[0] = Val; }
  void setVStride(unsigned Val) { Dims[1] = Val; }
  void setHSize(unsigned Val) { Dims[2] = Val; }
  void setHStride(unsigned Val) { Dims[3] = Val; }

  llvm::Value *getVOffset() const { return Indices[0]; }
  llvm::Value *getHOffset() const { return Indices[1]; }

  void setVOffset(llvm::Value *Val) { Indices[0] = Val; }
  void setHOffset(llvm::Value *Val) { Indices[1] = Val; }

  // 1D getter/setter
  unsigned getSize() const { return Dims[0]; }
  unsigned getStride() const { return Dims[1]; }
  llvm::Value *getOffset() const { return Indices[0]; }

  void setSize(unsigned Val) { Dims[0] = Val; }
  void setStride(unsigned Val) { Dims[1] = Val; }
  void setOffset(llvm::Value *Val) { Indices[0] = Val; }

  void print(raw_ostream &OS) const;
  void dump() const;
};

class CGCMRuntime {
public:
  CodeGenModule &CGM;

  /// \brief Class for emitting a CM call expression.
  struct CMCallInfo {
    // The codegen functon being processed.
    CodeGenFunction *CGF;
    // The call expression being processed.
    const CallExpr *CE;
    // The call instruction emitted.
    llvm::CallInst *CI;

    CMCallInfo() : CGF(0), CE(0), CI(0) {}
    CMCallInfo(CodeGenFunction *CGF, const CallExpr *CE)
        : CGF(CGF), CE(CE), CI(0) {}

    operator bool() { return CGF != 0; }

    void setCallInst(llvm::CallInst *CI) {
      assert(!this->CI && "already set");
      this->CI = CI;
    }
  };

  SmallVector<CMCallInfo, 4> CallInfoStack;

  /// \brief The pool of CGCMRegionInfo objects, that will be created and
  /// released for each function.
  std::vector<CGCMRegionInfo *> RegionInfoPool;

  /// \brief The cached lvalue initialzier for CM vector_ref/matrix_ref
  /// declarations.
  llvm::DenseMap<const ValueDecl *, LValue> ReferenceDecls;

  // \brief Each kernel function has an optional but unique SLM alloca, which
  // will be used to store the global offset in bytes for SLM.
  llvm::DenseMap<const llvm::Function *, llvm::AllocaInst *> SLMAllocas;

public:
  explicit CGCMRuntime(CodeGenModule &CGM) : CGM(CGM) {
    // Push a dummy one as the bottom.
    CallInfoStack.push_back(CMCallInfo());
  }

  ~CGCMRuntime() {
    assert(CallInfoStack.size() == 1);
    clear();
  }

  llvm::AllocaInst *getOrCreateSLMIndexVar(CodeGenFunction &CGF);

  /// \brief Return the CM builtin kind.
  CMBuiltinKind getCMBuiltinKind(const FunctionDecl *F) const;

  RValue EmitCMCallExpr(CodeGenFunction &CGF, const CallExpr *E,
                        ReturnValueSlot ReturnValue);

  RValue EmitCMBuiltin(CodeGenFunction &CGF, unsigned ID, const CallExpr *E);

  /// \brief Emit CM kernel metadata.
  void EmitCMKernelMetadata(const FunctionDecl *FD, llvm::Function *Fn);

  /// \brief Emit kernel output marker calls.
  void EmitCMOutput(CodeGenFunction &CGF);

  /// \brief Returns the value loaded from a CM region.
  RValue EmitCMReadRegion(CodeGenFunction &CGF, LValue LV);

  /// \brief Write into a CM region LValue.
  void EmitCMWriteRegion(CodeGenFunction &CGF, RValue Src, LValue LV);

  /// \brief Returns a LValue that describes how to access a CM base object.
  LValue EmitCMSelectExprLValue(CodeGenFunction &CGF, const CMSelectExpr *E);

  /// \brief Returns the value for a select expression.
  llvm::Value *EmitCMSelectExpr(CodeGenFunction &CGF, const CMSelectExpr *E);

  /// \brief Returns the LValue for a CM format expression.
  LValue EmitCMFormatExprLValue(CodeGenFunction &CGF, const CMFormatExpr *E);

  /// \brief Returns the value for a CM format expression.
  llvm::Value *EmitCMFormatExpr(CodeGenFunction &CGF, const CMFormatExpr *E);

  /// Merge expressions return nothing.
  void EmitCMMergeExpr(CodeGenFunction &CGF, const CMMergeExpr *E);

  /// \brief CodeGen any() and all() expressions.
  llvm::Value *EmitCMBoolReductionExpr(CodeGenFunction &CGF,
                                       const CMBoolReductionExpr *E);

  /// CM reference variable initialization does not emit code but cache the
  /// lvalue for future uses.
  void EmitCMRefDeclInit(CodeGenFunction &CGF, const ValueDecl *VD, LValue LV);

  /// Emit LValue for referencing a vector_ref/matrix_ref variable.
  LValue EmitCMDeclRefLValue(CodeGenFunction &CGF, const DeclRefExpr *E);

  /// Emit CM vector / matrix constant intialization.
  void EmitCMConstantInitializer(CodeGenFunction &CGF,
                                 const CodeGenFunction::AutoVarEmission &E);

  /// \brief Emit a temporary for by reference argument passing.
  ///
  /// This is necessary since format or select regions cannot be represented
  /// by address without a temporary variable. E.g.
  ///
  /// \code
  /// void foo(vector_ref<int, 8> v);
  /// void bar() {
  ///   vector<int, 16> v;
  ///   foo(v.select<8, 2>());
  /// }
  /// \code
  /// will be emitted as
  ///
  /// vector<int, 8> t = v.select<8, 2>();
  /// foo(t);
  /// v.select<8, 2>() = t;
  ///
  /// This function emits the initialization of t and returns the address of t.
  /// The writeback region write will be inserted after the call.
  llvm::Value *EmitCMReferenceArg(CodeGenFunction &CGF, LValue LV);

  // helper functions for emitting vector load/store intrinsic calls.
  static llvm::Value *EmitCMRefLoad(CodeGenFunction &CGF, llvm::Value *Addr);
  static void EmitCMRefStore(CodeGenFunction &CGF, llvm::Value *Val,
                             llvm::Value *Addr);

  CMCallInfo &getCurCMCallInfo() {
    assert(CallInfoStack.back() && "not initialized");
    return CallInfoStack.back();
  }

  void pushCMCallInfo(CodeGenFunction &CGF, const CallExpr *E) {
    CallInfoStack.push_back(CMCallInfo(&CGF, E));
  }

  void popCMCallInfo() {
    assert(CallInfoStack.back() && "not initialized");
    CallInfoStack.pop_back();
  }

  class EnterCMCallRAII {
    CGCMRuntime &CMRT;
  public:
    EnterCMCallRAII(CGCMRuntime &RT, CodeGenFunction &CGF, const CallExpr *E)
        : CMRT(RT) {
      CMRT.pushCMCallInfo(CGF, E);
    }
    ~EnterCMCallRAII() {
      CMRT.popCMCallInfo();
    }
  };

  void Error(SourceLocation Loc, StringRef Msg);
  llvm::Function *getIntrinsic(unsigned ID, ArrayRef<llvm::Type *> Tys = None);

  /// \brief Clear per function objects.
  void clear() {
    for (std::vector<CGCMRegionInfo *>::iterator I = RegionInfoPool.begin(),
                                                 E = RegionInfoPool.end();
         I != E; ++I)
      delete *I;
    RegionInfoPool.clear();
    ReferenceDecls.clear();
  }

  /// \brief Add a new region info object to pool, which owns this object.
  const CGCMRegionInfo &addRegionInfo(CGCMRegionInfo *Info) {
    assert(Info);
    RegionInfoPool.push_back(Info);
    return *Info;
  }

  /// \brief Determine the cast kind from SrcType to DestType. Returns false if
  /// no cast is necessary. Otherwise, returns true and the cast kind is returned
  /// in the first argument.
  static bool getCastOpKind(llvm::Instruction::CastOps &CastOp,
                            CodeGenFunction &CGF, QualType DstType,
                            QualType SrcType);

  void finalize();

private:
  /// \brief Postprocess builtin interface.
  void HandleBuiltinInterface(CMCallInfo &CallInfo);

  /// \brief Postprocess cm_slm_read.
  llvm::Value *HandleBuiltinSLMReadImpl(CMCallInfo &Info);

  /// \brief Postprocess cm_slm_write.
  void HandleBuiltinSLMWriteImpl(CMCallInfo &Info);

  /// \brief Postprocess cm_slm_read4.
  void HandleBuiltinSLMRead4(CMCallInfo &Info, bool IsDwordAddr);

  /// \brief Postprocess cm_slm_write4.
  void HandleBuiltinSLMWrite4(CMCallInfo &Info, bool IsDwordAddr);

  /// \brief Postprocess cm_slm_atomic variants.
  void HandleBuiltinSLMAtomic(CMCallInfo &Info);

  /// \brief Postprocess cm_avs_sampler.
  void HandleBuiltinAVSSampler(CMCallInfo &Info);

  /// \brief Postprocess cm_va_2d_convolve.
  void HandleBuiltinVA2dConvolve(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess cm_va_dilate and cm_va_erode.
  void HandleBuiltinVAErodeDilate(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess cm_va_dilate_hdc and cm_va_erode_hdc.
  void HandleBuiltinVAErodeDilateHdc(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess cm_va_min_max.
  void HandleBuiltinVAMinMax(CMCallInfo &Info);

  /// \brief Postprocess cm_va_min_max_filter.
  void HandleBuiltinVAMinMaxFilter(CMCallInfo &Info);

  /// \brief Postprocess cm_va_min_max_filter_hdc.
  void HandleBuiltinVAMinMaxFilterHdc(CMCallInfo &Info);

  /// \brief Postprocess cm_va_centroid and cm_va_boolean_centroid.
  void HandleBuiltinVACentroid(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess cm_va_1d_convolution and cm_va_1d_convolution_hdc.
  void HandleBuiltinVA1dConvolution(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess cm_va_1pixel_convolve.
  void HandleBuiltinVA1PixelConvolve(CMCallInfo &Info);

  /// \brief Postprocess cm_va_lbp_creation and cm_va_lbp_creation_hdc.
  void HandleBuiltinVALbpCreation(CMCallInfo &Info, CMBuiltinKind);

  /// \brief Postprocess cm_va_1pixel_convolve_hdc.
  void HandleBuiltinVA1PixelConvolveHdc(CMCallInfo &Info);

  /// \brief Postprocess cm_va_lbp_correlation and cm_va_lbp_correlation_hdc.
  void HandleBuiltinVALbpCorrelation(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess cm_va_correlation_search.
  void HandleBuiltinVACorrelationSearch(CMCallInfo &Info);

  /// \brief Postprocess cm_va_flood_fill.
  void HandleBuiltinVAFloodFill(CMCallInfo &Info);

  /// \brief Postprocess oword read implementations.
  llvm::Value *HandleBuiltinOWordReadImpl(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess the oword write implementation.
  void HandleBuiltinOWordWriteImpl(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess media read implementations.
  llvm::Value *HandleBuiltinMediaReadImpl(CMCallInfo &Info);

  /// \brief Postprocess media write implementations.
  void HandleBuiltinMediaWriteImpl(CMCallInfo &Info);

  /// \brief Postprocess media read_plane implementations.
  llvm::Value *HandleBuiltinMediaReadPlane(CMCallInfo &Info);

  /// \brief Postprocess media write_plane implementations.
  void HandleBuiltinMediaWritePlane(CMCallInfo &Info);

  /// \brief Postprocess scattered read and write implementations.
  llvm::Value *HandleBuiltinScatterReadWriteImpl(CMCallInfo &Info,
                                                 bool IsWrite = false);

  /// \brief Postprocess read_untyped and write_untyped implementations.
  void HandleBuiltinReadWriteUntypedImpl(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess read_typed implementation.
  void HandleBuiltinReadTypedImpl(CMCallInfo &Info);

  /// \brief Postprocess write_typed implementation.
  void HandleBuiltinWriteTypedImpl(CMCallInfo &Info);

  /// \brief Postprocess svm block read implementations.
  llvm::Value *HandleBuiltinSVMBlockReadImpl(CMCallInfo &Info, CMBuiltinKind Kind);

  /// \brief Postprocess svm block write implementation.
  void HandleBuiltinSVMBlockWriteImpl(CMCallInfo &Info);

  /// \brief Postprocess svm scatter read implementation.
  llvm::Value *HandleBuiltinSVMScatterReadImpl(CMCallInfo &Info);

  /// \brief Postprocess svm scatter write implementation.
  void HandleBuiltinSVMScatterWriteImpl(CMCallInfo &Info);

  /// \brief Postprocess svm scatter write implementation.
  void HandleBuiltinSVMAtomicOpImpl(CMCallInfo &CallInfo);

  /// \brief Postprocess builtin cm_3d_sample, cm_3d_load.
  void HandleBuiltin3dOperationImpl(CMCallInfo &CallInfo, CMBuiltinKind Kind);

  /// \brief Postprocess builtin saturate implementation.
  llvm::Value *HandleBuiltinSaturateImpl(CMCallInfo &CallInfo,
                                         CMBuiltinKind Kind);

  /// \brief Postprocess builtin min/max implementation.
  llvm::Value *HandleBuiltinMinMaxImpl(CMCallInfo &CallInfo,
                                       CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_dp2, cm_dp3, cm_dp4 and cm_dph.
  llvm::Value *HandleBuiltinDotProductImpl(CMCallInfo &CallInfo,
                                           CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_line
  llvm::Value *HandleBuiltinLineImpl(CMCallInfo &CallInfo,
                                     CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_intrinsic_impl_frc.
  llvm::Value *HandleBuiltinFblFbhImpl(CMCallInfo &CallInfo,
                                       CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_intrinsic_impl_frc.
  llvm::Value *HandleBuiltinFrcImpl(CMCallInfo &CallInfo,
                                    CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_intrinsic_impl_lzd.
  llvm::Value *HandleBuiltinLzdImpl(CMCallInfo &CallInfo,
                                    CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_inv, cm_log, etc.
  llvm::Value *HandleBuiltinExtendedMathImpl(CMCallInfo &CallInfo,
                                             CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_abs.
  llvm::Value *HandleBuiltinAbsImpl(CMCallInfo &CallInfo, CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_mul and cm_add.
  llvm::Value *HandleBuiltinMulAddImpl(CMCallInfo &CallInfo,
                                       CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_avg.
  llvm::Value *HandleBuiltinAvgImpl(CMCallInfo &CallInfo,
                                    CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_imul.
  llvm::Value *HandleBuiltinIMulImpl(CMCallInfo &CallInfo);

  /// \brief Postprocess builtin cm_shl.
  llvm::Value *HandleBuiltinShlImpl(CMCallInfo &CallInfo,
                                    CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_sad2.
  llvm::Value *HandleBuiltinSad2Impl(CMCallInfo &CallInfo,
                                     CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_sada2.
  llvm::Value *HandleBuiltinSad2AddImpl(CMCallInfo &CallInfo,
                                        CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_lrp.
  llvm::Value *HandleBuiltinLrpImpl(CMCallInfo &CallInfo,
                                    CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_pln.
  llvm::Value *HandleBuiltinPlnImpl(CMCallInfo &CallInfo,
                                    CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_bfrev.
  llvm::Value *HandleBuiltinCountBitsImpl(CMCallInfo &CallInfo,
                                          CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_bfrev.
  llvm::Value *HandleBuiltinBitFieldReverseImpl(CMCallInfo &CallInfo,
                                                CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_bf_insert.
  llvm::Value *HandleBuiltinBitFieldInsertImpl(CMCallInfo &CallInfo,
                                               CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_bf_extract.
  llvm::Value *HandleBuiltinBitFieldExtractImpl(CMCallInfo &CallInfo,
                                                CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_rndd/rndu/rude/rudz.
  llvm::Value *HandleBuiltinRoundingImpl(CMCallInfo &CallInfo,
                                         CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_sum/prod/reduced_min/reduced_max.
  llvm::Value *HandleBuiltinReductionImpl(CMCallInfo &CallInfo,
                                          CMBuiltinKind Kind);

  /// \brief Postprocess builtin sample16.
  llvm::Value *HandleBuiltinSample16Impl(CMCallInfo &CallInfo,
                                         CMBuiltinKind Kind);

  /// \brief Postprocess builtin sample32.
  llvm::Value *HandleBuiltinSample32Impl(CMCallInfo &CallInfo,
                                         CMBuiltinKind Kind);

  /// \brief Postprocess builtin load16.
  llvm::Value *HandleBuiltinLoad16Impl(CMCallInfo &CallInfo,
                                       CMBuiltinKind Kind);

  /// \brief Postprocess builtin atomic write.
  llvm::Value *HandleBuiltinWriteAtomicImpl(CMCallInfo &CallInfo,
                                            CMBuiltinKind Kind);

  /// \brief Postprocess builtin atomic typed write.
  llvm::Value *HandleBuiltinWriteAtomicTypedImpl(CMCallInfo &CallInfo,
                                                 CMBuiltinKind Kind);

  /// \brief Postprocess builtin cm_pack_mask.
  llvm::Value *HandleBuiltinPackMaskImpl(CMCallInfo &CallInfo);

  /// \brief Postprocess builtin cm_unpack_mask.
  llvm::Value *HandleBuiltinUnPackMaskImpl(CMCallInfo &CallInfo);

  /// \brief Postprocess builtin cm_rdtsc.
  llvm::Value *HandleBuiltinRDTSC(CMCallInfo &CallInfo);

  /// \brief Postprocess builtin cm_send.
  llvm::Value *HandleBuiltinSendImpl(CMCallInfo &CallInfo);

  /// \brief Postprocess builtin cm_sends.
  llvm::Value *HandleBuiltinSendsImpl(CMCallInfo &CallInfo);

  /// \brief Postprocess builtin cm_get_r0.
  llvm::Value *HandleBuiltinGetR0Impl(CMCallInfo &CallInfo);

  /// \brief Postprocess builtin cm_get_sr0.
  llvm::Value *HandleBuiltinGetSR0Impl(CMCallInfo &CallInfo);
  
  /// \brief Postprocess builtin cm_get_value.
  llvm::Value *HandleBuiltinGetValueImpl(CMCallInfo &CallInfo);

  /// \brief Postprocess simdcf_any implementation builtin.
  llvm::Value *HandleBuiltinSimdcfAnyImpl(CMCallInfo &CallInfo);

  /// \brief Postprocess simdcf_predgen implementation builtin.
  llvm::Value *HandleBuiltinSimdcfGenericPredicationImpl(CMCallInfo &CallInfo);

  /// \brief Postprocess simdcf_predmin, simdcf_prefmax implementation builtins.
  llvm::Value *HandleBuiltinSimdcfMinMaxPredicationImpl(CMCallInfo &CallInfo,
                                                        CMBuiltinKind Kind);

  /// \brief Postprocess predefined_surface implementation builtin.
  llvm::Value *HandlePredefinedSurface(CMCallInfo &CallInfo);

  /// \brief Postprocess cm_svm_atomic implementation builtins.
  llvm::Value *HandleBuiltinSVMAtomicImpl(CMCallInfo &CallInfo);


  /// \brief Emit 1D/2D select expression.
  LValue EmitSelect(CodeGenFunction &CGF, const CMSelectExpr *E, LValue Base);

  /// \brief Emit 2D row/column select expression.
  LValue EmitRowColumnSelect(CodeGenFunction &CGF, const CMSelectExpr *E,
                             LValue Base);

  /// \brief Emit 1D/2D element access.
  LValue EmitElementSelect(CodeGenFunction &CGF, const CMSelectExpr *E,
                           LValue Base);

  /// \brief Emit subscription access.
  LValue EmitSubscriptSelect(CodeGenFunction &CGF, const CMSelectExpr *E,
                             LValue Base);

  /// \brief Emit select_all expression.
  LValue EmitSelectAll(CodeGenFunction &CGF, const CMSelectExpr *E,
                       LValue Base);

  /// \brief Emit replicate expression.
  llvm::Value *EmitReplicateSelect(CodeGenFunction &CGF, const CMSelectExpr *E);

  /// \brief Emit iselect expression
  llvm::Value *EmitISelect(CodeGenFunction &CGF, const CMSelectExpr *E);

  /// \brief Emit a region read in rows. E.g.
  ///
  /// matrix<float, 8, 32> m;
  /// m.select<6, 1, 32, 1>(idx, 0);
  ///
  /// where HSize equals 32, HStride equals 1 and HOffset equals 0.
  ///
  llvm::Value *EmitReadRegionInRows(CGBuilderTy &Builder, llvm::Value *Region,
                                    unsigned BaseWidth, unsigned VSize,
                                    unsigned VStride, llvm::Value *VOffset);

  /// \brief Emit a region read in columns. E.g.
  ///
  /// matrix<float, 8, 32> m;
  /// m.select<8, 1, 24, 1>(0, idy);
  ///
  /// where VSize equals 8, VStride equals 1 and VOffset equals 0.
  ///
  llvm::Value *EmitReadRegionInCols(CGBuilderTy &Builder, llvm::Value *Region,
                                    unsigned Height, unsigned HSize,
                                    unsigned HStride, llvm::Value *HOffset);

  llvm::Value *EmitReadRegion1D(CGBuilderTy &Builder, llvm::Value *Region,
                                unsigned Size, unsigned Stride,
                                llvm::Value *Offset);

  llvm::Value *EmitWriteRegion1D(CGBuilderTy &Builder, llvm::Value *Dst,
                                 llvm::Value *Src, unsigned Size,
                                 unsigned Stride, llvm::Value *Offset,
                                 llvm::Value *Mask = 0);

  llvm::Value *EmitWriteRegion2D(CGBuilderTy &Builder, llvm::Value *Dst,
                                 unsigned BaseWidth, llvm::Value *Src,
                                 unsigned VSize, unsigned VStride,
                                 unsigned HSize, unsigned HStride,
                                 llvm::Value *VOffset, llvm::Value *HOffset);

  /// Emit cm_slm_init builtin call.
  void EmitBuiltinSLMInit(CodeGenFunction &CGF, const CallExpr *E);

  /// Emit cm_slm_alloc builtin call.
  llvm::Value *EmitBuiltinSLMAlloc(CodeGenFunction &CGF, const CallExpr *E);

  /// Emit cm_slm_free builtin call.
  llvm::Value *EmitBuiltinSLMFree(CodeGenFunction &CGF, const CallExpr *E);

  /// \brief Emit one of gather_scaled, scatter_scaled,
  ///    gather4_scaled, scatter4_scaled.
  llvm::CallInst *EmitGatherScatterScaled(CodeGenFunction &CGF,
      unsigned IntrinsicID, llvm::APInt Selector, unsigned Scale,
      llvm::Value *Surface, llvm::Value *GlobalOffset, llvm::Value *ElementOffset,
      llvm::Value *Data);

public:
  /// \brief Returns the corresponding genx intrinsic ID for this call.
  unsigned GetGenxIntrinsicID(CMCallInfo &CallInfo, CMBuiltinKind Kind,
                              bool IsSaturated = false);

  /// \brief Read a region with static offset in number of elements.
  llvm::Value *EmitReadRegion(CodeGenFunction &CGF, llvm::Value *Region,
                              unsigned Size, unsigned Stride, unsigned Offset) {
    return EmitReadRegion1D(CGF.Builder, Region, Size, Stride,
                            llvm::ConstantInt::get(CGF.Int16Ty, Offset));
  }

  /// \brief Write a region with static offset in number of elements.
  llvm::Value *EmitWriteRegion(CodeGenFunction &CGF, llvm::Value *Dst,
                               llvm::Value *Src, unsigned Size, unsigned Stride,
                               unsigned Offset, llvm::Value *Mask = 0) {
    return EmitWriteRegion1D(CGF.Builder, Dst, Src, Size, Stride,
                             llvm::ConstantInt::get(CGF.Int16Ty, Offset), Mask);
  }
};

// Returns the llvm type for masks (<32 x i1> etc.).
static inline llvm::VectorType *getMaskType(llvm::LLVMContext &Context,
                                            unsigned NumElts = 32u) {
  return llvm::VectorType::get(llvm::Type::getInt1Ty(Context), NumElts);
}

// Turn a MDNode into llvm::value or its subclass.
// Return nullptr if the underlying value has type mismatch.
template <typename Ty = llvm::Value> Ty *getVal(llvm::Metadata *M) {
  if (auto VM = dyn_cast<llvm::ValueAsMetadata>(M))
    if (auto V = dyn_cast<Ty>(VM->getValue()))
      return V;
  return nullptr;
}

static inline llvm::Metadata *getMD(llvm::Value *V) {
  return llvm::ValueAsMetadata::get(V);
}

} // namespace CodeGen
} // namespace clang

#endif
