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
// CM specific code generation.
//===----------------------------------------------------------------------===//

#include "CGCM.h"
#include "CodeGenModule.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCM.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/Path.h"

using namespace clang;
using namespace CodeGen;

CGCMRegionInfo::CGCMRegionInfo(RegionKind Kind, LValue Base, unsigned vSize,
                               unsigned vStride, unsigned hSize,
                               unsigned hStride, llvm::Value *vOffset,
                               llvm::Value *hOffset)
    : Kind(Kind), Base(Base) {
  setVSize(vSize);
  setVStride(vStride);
  setHSize(hSize);
  setHStride(hStride);
  setVOffset(vOffset);
  setHOffset(hOffset);
}

CGCMRegionInfo::CGCMRegionInfo(RegionKind Kind, LValue Base, unsigned Size,
                               unsigned Stride, llvm::Value *Offset)
    : Kind(Kind), Base(Base) {
  setSize(Size);
  setStride(Stride);
  setVOffset(Offset);

  // fill zeros for other fields.
  Dims[2] = Dims[3] = 0;
  Indices[1] = 0;
}

CGCMRegionInfo::CGCMRegionInfo(RegionKind Kind, LValue Base)
    : Kind(Kind), Base(Base) {
  // fill zeros for other fields.
  Dims[0] = Dims[1] = Dims[2] = Dims[3] = 0;
  Indices[0] = Indices[1] = 0;
}

unsigned CGCMRegionInfo::getBaseWidth() const {
  assert(is2DRegion());

  QualType Ty = Base.getType();
  assert(Ty->isCMMatrixType() && "matrix type expected");
  return Ty->getAs<CMMatrixType>()->getNumColumns();
}

unsigned CGCMRegionInfo::getBaseHeight() const {
  assert(is2DRegion());

  QualType Ty = Base.getType();
  assert(Ty->isCMMatrixType() && "matrix type expected");
  return Ty->getAs<CMMatrixType>()->getNumRows();
}

unsigned CGCMRegionInfo::getBaseSize() const {
  assert(is1DRegion());

  QualType Ty = Base.getType();
  assert(Ty->isCMVectorType() && "vector type expected");
  return Ty->getAs<CMVectorType>()->getNumElements();
}

void CGCMRegionInfo::print(raw_ostream &OS) const {
  if (isFormat()) {
    OS << "{ format, " << Base.getType().getAsString() << " }";
    return;
  }

  OS << "{ ";
  if (Base.isSimple())
    OS << Base.getAddress().getPointer()  << "(base)";
  else if (Base.isCMRegion())
    OS << Base.getCMRegionAddr() << "(nested)";

  // Print all fields as it is.
  for (unsigned i = 0; i < 4; ++i)
    OS << ", " << Dims[i];

  OS << "; ";

  // Print all indices
  for (unsigned i = 0; i < 2; ++i)
    if (Indices[i] != 0) {
      Indices[i]->print(OS);
      OS << ", ";
    }

  OS << "}";
}

void CGCMRegionInfo::dump() const { print(llvm::errs()); }

////////////////////////////////////////////////////////////////////////////////
// CM specific expressions codegen.
////////////////////////////////////////////////////////////////////////////////

static unsigned EmitAsConstantInt(CodeGenFunction &CGF, const Expr *E) {
  llvm::Value *Val = CGF.EmitAnyExpr(E).getScalarVal();
  assert(isa<llvm::ConstantInt>(Val) && "not a constant integer");
  return static_cast<unsigned>(cast<llvm::ConstantInt>(Val)->getZExtValue());
}

static llvm::Value *getBaseAddr(LValue Base) {
  if (Base.isSimple())
    return Base.getAddress().getPointer();
  else if (Base.isCMRegion())
    return Base.getCMRegionAddr();

  llvm_unreachable("unexpected base lvalue");
}

LValue CGCMRuntime::EmitSelect(CodeGenFunction &CGF, const CMSelectExpr *E,
                               LValue Base) {
  assert(E->isSelect());
  if (E->is2D()) {
    // Emit vSize, vStride, hSize, hStride
    unsigned vSize = EmitAsConstantInt(CGF, E->getVSize());
    unsigned vStride = EmitAsConstantInt(CGF, E->getVStride());
    unsigned hSize = EmitAsConstantInt(CGF, E->getHSize());
    unsigned hStride = EmitAsConstantInt(CGF, E->getHStride());

    // Emit vOffset
    llvm::Value *vOffset = CGF.EmitAnyExpr(E->getVOffset()).getScalarVal();
    assert(vOffset && vOffset->getType()->isIntegerTy());

    // Emit hOffset
    llvm::Value *hOffset = CGF.EmitAnyExpr(E->getHOffset()).getScalarVal();
    assert(hOffset && hOffset->getType()->isIntegerTy());

    const CGCMRegionInfo &RI = addRegionInfo(
        new CGCMRegionInfo(CGCMRegionInfo::RK_select, Base, vSize, vStride,
                           hSize, hStride, vOffset, hOffset));

    return LValue::MakeCMRegion(getBaseAddr(Base), RI, E->getType(),
                                Base.getAlignment());
  }

  assert(E->is1D());
  // Emit size and stride.
  unsigned Size = EmitAsConstantInt(CGF, E->getSize());
  unsigned Stride = EmitAsConstantInt(CGF, E->getStride());

  // Emit offset.
  llvm::Value *Offset = CGF.EmitAnyExpr(E->getOffset()).getScalarVal();
  assert(Offset && Offset->getType()->isIntegerTy());

  const CGCMRegionInfo &RI = addRegionInfo(new CGCMRegionInfo(
      CGCMRegionInfo::RK_select, Base, Size, Stride, Offset));

  return LValue::MakeCMRegion(getBaseAddr(Base), RI, E->getType(),
                              Base.getAlignment());
}

LValue CGCMRuntime::EmitRowColumnSelect(CodeGenFunction &CGF,
                                        const CMSelectExpr *E, LValue Base) {
  assert(E->isColumnSelect() || E->isRowSelect());
  assert(E->is2D() && "not selecting matrix");

  const CMMatrixType *MT = E->getBase()->getType()->getAs<CMMatrixType>();
  unsigned Height = MT->getNumRows();
  unsigned Width = MT->getNumColumns();

  // Selecting a row is equivalent to 2D select with arguments:
  //
  // vSize = 1, vStride = 0, hSize = <matrix_width>, hStride = 1, and
  // vOffset = index, hOffset = 0
  //
  if (E->isRowSelect()) {
    llvm::Value *Index = CGF.EmitAnyExpr(E->getRowExpr()).getScalarVal();
    llvm::Value *Zero = llvm::Constant::getNullValue(Index->getType());
    const CGCMRegionInfo &RI = addRegionInfo(new CGCMRegionInfo(
        CGCMRegionInfo::RK_select, Base, 1, 0, Width, 1, Index, Zero));

    return LValue::MakeCMRegion(getBaseAddr(Base), RI, E->getType(),
                                Base.getAlignment());
  }

  // Selecting a column is equivalent to 2D select with arguments:
  //
  // vSize = <matrix_height>, vStride = 1, hSize = 1, hStride = 1,
  // and vOffset = 0, hOffset = index
  //
  llvm::Value *Index = CGF.EmitAnyExpr(E->getColumnExpr()).getScalarVal();
  llvm::Value *Zero = llvm::Constant::getNullValue(Index->getType());
  const CGCMRegionInfo &RI = addRegionInfo(new CGCMRegionInfo(
      CGCMRegionInfo::RK_select, Base, Height, 1, 1, 1, Zero, Index));

  return LValue::MakeCMRegion(getBaseAddr(Base), RI, E->getType(),
                              Base.getAlignment());
}

LValue CGCMRuntime::EmitElementSelect(CodeGenFunction &CGF,
                                      const CMSelectExpr *E, LValue Base) {
  assert(E->isElementSelect());

  // Selecting a single element from a vector is a regular select with:
  // Size = 1, Stride = 0, Offset = index
  if (E->is1D()) {
    llvm::Value *Index = CGF.EmitAnyExpr(E->getIndex()).getScalarVal();
    const CGCMRegionInfo &RI = addRegionInfo(
        new CGCMRegionInfo(CGCMRegionInfo::RK_select, Base, 1, 0, Index));

    return LValue::MakeCMRegion(getBaseAddr(Base), RI, E->getType(),
                                Base.getAlignment());
  }

  assert(E->is2D());
  // Selecting a single element from a matrix is a regular select with:
  // vSize = 1, vStride = 0, hSize = 1, hStride = 0, vOffset = <row_index>,
  // hOffset = <column_index>
  llvm::Value *RowIndex = CGF.EmitAnyExpr(E->getRowIndex()).getScalarVal();
  llvm::Value *ColIndex = CGF.EmitAnyExpr(E->getColumnIndex()).getScalarVal();
  const CGCMRegionInfo &RI = addRegionInfo(new CGCMRegionInfo(
      CGCMRegionInfo::RK_select, Base, 1, 0, 1, 0, RowIndex, ColIndex));

  return LValue::MakeCMRegion(getBaseAddr(Base), RI, E->getType(),
                              Base.getAlignment());
}

LValue CGCMRuntime::EmitSubscriptSelect(CodeGenFunction &CGF,
                                        const CMSelectExpr *E, LValue Base) {
  assert(E->isSubscriptSelect());

  // Selecting a single element from a vector is a regular select with:
  // Size = 1, Stride = 0, Offset = index
  if (E->is1D()) {
    llvm::Value *Index = CGF.EmitAnyExpr(E->getSubscriptIndex()).getScalarVal();
    const CGCMRegionInfo &RI = addRegionInfo(
        new CGCMRegionInfo(CGCMRegionInfo::RK_select, Base, 1, 0, Index));

    return LValue::MakeCMRegion(getBaseAddr(Base), RI, E->getType(),
                                Base.getAlignment());
  }

  assert(E->is2D());
  assert(E->getType()->isCMVectorType());
  unsigned Width = E->getType()->getCMVectorMatrixSize();
  // Selecting a row from a matrix is a regular select with:
  //
  // vSize = 1, vStride = 0, hSize = <matrix_width>, hStride = 1, and
  // vOffset = index, hOffset = 0
  //
  llvm::Value *Index = CGF.EmitAnyExpr(E->getSubscriptIndex()).getScalarVal();
  llvm::Value *Zero = llvm::Constant::getNullValue(Index->getType());
  const CGCMRegionInfo &RI = addRegionInfo(new CGCMRegionInfo(
      CGCMRegionInfo::RK_select, Base, 1, 0, Width, 1, Index, Zero));

  return LValue::MakeCMRegion(getBaseAddr(Base), RI, E->getType(),
                              Base.getAlignment());
}

LValue CGCMRuntime::EmitSelectAll(CodeGenFunction &CGF, const CMSelectExpr *E,
                                  LValue Base) {
  assert(E->isSelectAll());
  // Simply returns the base.
  return Base;
}

LValue CGCMRuntime::EmitCMSelectExprLValue(CodeGenFunction &CGF,
                                           const CMSelectExpr *E) {
  const Expr *Base = E->getBase();

  // This is a bug in the old compiler's implementation:
  //
  // CM provides a set of "select" functions for referencing a subset of the
  // elements of vector/matrix objects. All these operations (except "iselect",
  // "genx_select", and "replicate") return a reference to the elements of
  // matrix/vector objects, so they can be used as L-values in the statements.
  //
  // This means it is illegal to select a rvalue. On the other hand, C/C++ does
  // allow member accesses of rvalues, but the resulting expressions are also
  // rvalues. We have to somehow get over this defect, i.e. create a temporal
  // object internally and use it as the base. Approximately this is applying
  // some rvalue reference semantics to vector_ref or matrix_ref (!).
  //
  LValue BaseAddr;
  if (Base->isRValue()) {
    llvm::Value *Val = CGF.EmitAnyExpr(Base).getScalarVal();
    llvm::Value *Addr = CGF.CreateTempAlloca(Val->getType(), "rvalue.addr");
    CGF.Builder.CreateDefaultAlignedStore(Val, Addr);
    BaseAddr = CGF.MakeNaturalAlignAddrLValue(Addr, Base->getType());
  } else
    BaseAddr = CGF.EmitLValue(Base);

  switch (E->getSelectKind()) {
  default:
    break;
  case CMSelectExpr::SK_select:
    return EmitSelect(CGF, E, BaseAddr);
  case CMSelectExpr::SK_row:
  case CMSelectExpr::SK_column:
    return EmitRowColumnSelect(CGF, E, BaseAddr);
  case CMSelectExpr::SK_element:
    return EmitElementSelect(CGF, E, BaseAddr);
  case CMSelectExpr::SK_subscript:
    return EmitSubscriptSelect(CGF, E, BaseAddr);
  case CMSelectExpr::SK_select_all:
    return EmitSelectAll(CGF, E, BaseAddr);
  }
  llvm_unreachable("not implemented yet");
}

llvm::Value *CGCMRuntime::EmitCMSelectExpr(CodeGenFunction &CGF,
                                           const CMSelectExpr *E) {
  // A replicate select is always an RValue.
  if (E->isReplicate()) {
    assert(E->isRValue());
    return EmitReplicateSelect(CGF, E);
  }

  // An iselect is always an RValue.
  if (E->isISelect()) {
    assert(E->isRValue());
    return EmitISelect(CGF, E);
  }

  assert(E->isLValue());
  LValue LV = EmitCMSelectExprLValue(CGF, E);
  return CGF.EmitLoadOfLValue(LV, E->getExprLoc()).getScalarVal();
}

LValue CGCMRuntime::EmitCMFormatExprLValue(CodeGenFunction &CGF,
                                           const CMFormatExpr *E) {
  const Expr *Base = E->getBase();
  LValue BaseLV = CGF.EmitLValue(Base);

  const CGCMRegionInfo &RI =
      addRegionInfo(new CGCMRegionInfo(CGCMRegionInfo::RK_format, BaseLV));

  return LValue::MakeCMRegion(getBaseAddr(BaseLV), RI, E->getType(),
                              BaseLV.getAlignment());
}

llvm::Value *CGCMRuntime::EmitCMFormatExpr(CodeGenFunction &CGF,
                                           const CMFormatExpr *E) {
  if (E->isRValue()) {
    llvm::Value *Val = CGF.EmitAnyExpr(E->getBase()).getScalarVal();
    llvm::Type *Ty = CGF.ConvertType(E->getType());
    // In case the base is a reference.
    if (Ty->isPointerTy())
      Ty = Ty->getPointerElementType();
    Val = CGF.Builder.CreateBitCast(Val, Ty);
    return Val;
  }

  assert(E->isLValue());
  LValue LV = EmitCMFormatExprLValue(CGF, E);
  return CGF.EmitLoadOfLValue(LV, E->getExprLoc()).getScalarVal();
}

static llvm::Value *GetMergeMaskValue(CGCMRuntime &CMRT, CodeGenFunction &CGF,
                                      llvm::Value *Mask, unsigned NumElts) {
  llvm::VectorType *MaskTy = getMaskType(Mask->getContext(), NumElts);

  // Mask is a vector.
  if (isa<llvm::VectorType>(Mask->getType())) {
    assert(Mask->getType()->getVectorNumElements() == NumElts);
    // trunc to the mask type.
    return CGF.Builder.CreateTrunc(Mask, MaskTy);
  }

  // Mask is an integer scalar.
  assert(Mask->getType() == CGF.Int32Ty && "unexpected scalar mask type");
  unsigned BitWidth = Mask->getType()->getIntegerBitWidth();

  // Mask is a constant integer.
  if (llvm::ConstantInt *CI = dyn_cast<llvm::ConstantInt>(Mask)) {
    llvm::APInt MaskValue = CI->getValue();
    llvm::SmallVector<llvm::Constant*, 32> Bits;

    llvm::Type *MaskEltTy = MaskTy->getElementType();
    llvm::Constant *False = llvm::ConstantInt::getFalse(MaskEltTy);
    llvm::Constant *True = llvm::ConstantInt::getTrue(MaskEltTy);
    for (unsigned i = 0; i < NumElts; ++i) {
      // Replicate mask if NumElts exceeds BitWidth.
      unsigned Index = i % BitWidth;
      Bits.push_back(MaskValue[Index] ? True : False);
    }

    return llvm::ConstantVector::get(Bits);
  }

  // Mask is an integer variable.
  // If it is not big enough for the vector size, replicate it using a
  // splatting rdregion.
  auto I32Ty = llvm::Type::getInt32Ty(CGF.getLLVMContext());
  if (BitWidth < NumElts) {
    // First bitcast to a 1-vector.
    Mask = CGF.Builder.CreateBitCast(Mask,
        llvm::VectorType::get(Mask->getType(), 1), "replicatemask");
    // Then replicate.
    unsigned NumReplications = (NumElts + BitWidth - 1) / BitWidth;
    auto I16Ty = llvm::Type::getInt16Ty(CGF.getLLVMContext());
    auto Zero = llvm::ConstantInt::get(I32Ty, 0);
    auto Width = llvm::ConstantInt::get(I32Ty, NumReplications);
    auto I16Zero = llvm::ConstantInt::get(I16Ty, 0);
    llvm::Value *Args[] = { Mask,
        Zero/*vstride*/, Width, Zero/*stride*/, I16Zero/*index*/,
        llvm::UndefValue::get(I32Ty)/*parentwidth*/ };
    llvm::Type *Tys[] = {
        llvm::VectorType::get(Mask->getType(), NumReplications), // rdregion return type
        Mask->getType(), // input type
        I16Ty // index type
    };
    llvm::Function *Fn = CMRT.getIntrinsic(
        llvm::Intrinsic::genx_rdregioni, Tys);
    Mask = CGF.Builder.CreateCall(Fn, Args, "replicatemask");
    BitWidth *= NumReplications;
  }
  // Bitcast the (vector of) int to vector of i1.
  llvm::Type *VecTy = llvm::VectorType::get(
      llvm::Type::getInt1Ty(CGF.getLLVMContext()), BitWidth);
  auto Res = CGF.Builder.CreateBitCast(Mask, VecTy, "cast");
  // If that is now too wide, narrow it with a shufflevector.
  if (BitWidth > NumElts) {
    SmallVector<llvm::Constant *, 16> Indices;
    for (unsigned i = 0; i != NumElts; ++i)
      Indices.push_back(llvm::ConstantInt::get(I32Ty, i));
    auto SV = llvm::ConstantVector::get(Indices);
    Res = CGF.Builder.CreateShuffleVector(Res,
        llvm::UndefValue::get(Res->getType()), SV, "narrowmask");
  }
  return Res;
}

void CGCMRuntime::EmitCMMergeExpr(CodeGenFunction &CGF, const CMMergeExpr *E) {
  // Emit the Dst LValue first.
  LValue LV = CGF.EmitLValue(E->getBase());

  llvm::Value *Src1 = CGF.EmitAnyExpr(E->getSrc1()).getScalarVal();
  llvm::Value *Src2 = 0;
  if (E->isOneSourceMerge()) {
    // a.merge(b, mask);
    Src2 = CGF.EmitLoadOfLValue(LV, E->getExprLoc()).getScalarVal();
  } else {
    // a.merge(b, c, mask);
    assert(E->isTwoSourceMerge());
    Src2 = CGF.EmitAnyExpr(E->getSrc2()).getScalarVal();
  }
  assert(Src1->getType() == Src2->getType());

  // Emit mask, the final mask has type <NumElts x i1> or <32 x i1>.
  llvm::Value *Mask = CGF.EmitAnyExpr(E->getMask()).getScalarVal();
  unsigned NumElts = Src1->getType()->getVectorNumElements();
  Mask = GetMergeMaskValue(*this, CGF, Mask, NumElts);

  // Form the merged value.
  llvm::Value *MergedVal = 0;
  if (Mask->getType()->getVectorNumElements() == NumElts)
    MergedVal = CGF.Builder.CreateSelect(Mask, Src1, Src2, "merge");
  else
    MergedVal = EmitWriteRegion(CGF, Src2, Src1, NumElts, 1, 0, Mask);

  // Store the merged value to the base.
  CGF.EmitStoreThroughLValue(RValue::get(MergedVal), LV);
}

/// \brief CodeGen any() and all() expressions.
llvm::Value *
CGCMRuntime::EmitCMBoolReductionExpr(CodeGenFunction &CGF,
                                     const CMBoolReductionExpr *E) {
  llvm::Value *Base = CGF.EmitAnyExpr(E->getBase()).getScalarVal();
  assert(isa<llvm::VectorType>(Base->getType()));

  CGBuilderTy &Builder = CGF.Builder;
  llvm::Type *RetTy = CGF.ConvertType(E->getType());

  // Convert input to predicate by comparing not equal to all 0 value.
  llvm::Value *Cmp;
  if (Base->getType()->getScalarType()->isFloatingPointTy())
    Cmp = Builder.CreateFCmpONE(Base,
        llvm::Constant::getNullValue(Base->getType()), "cmp");
  else
    Cmp = Builder.CreateICmpNE(Base,
        llvm::Constant::getNullValue(Base->getType()), "cmp");

  // Add the SIMD CF predication of the result.
  llvm::Function *PredFn = getIntrinsic(llvm::Intrinsic::genx_simdcf_predicate,
      Cmp->getType());

  llvm::Value *Vals[] = {
      Cmp, E->isAny() ? llvm::Constant::getNullValue(Cmp->getType())
                      : llvm::Constant::getAllOnesValue(Cmp->getType())};
  llvm::Value *CmpPred = CGF.Builder.CreateCall(PredFn, Vals, "cmppred");

  // Build all/any intrinsic call.
  llvm::Function *Fn = getIntrinsic(
      E->isAny() ? llvm::Intrinsic::genx_any : llvm::Intrinsic::genx_all,
      Cmp->getType());
  llvm::Value *AllAny = CGF.Builder.CreateCall(Fn, CmpPred, "allany");

  // Zero extend to required type.
  return Builder.CreateZExt(AllAny, RetTy);
}

llvm::Value *CGCMRuntime::EmitReplicateSelect(CodeGenFunction &CGF,
                                              const CMSelectExpr *E) {
  assert(E->isReplicate());
  llvm::Value *Region = CGF.EmitAnyExpr(E->getBase()).getScalarVal();

  llvm::Type *Ty = Region->getType();
  assert(Ty->isVectorTy());

  // Types for overloading.
  llvm::LLVMContext &C = CGF.getLLVMContext();
  llvm::Type *Tys[] = {CGF.ConvertType(E->getType()), Ty,
      llvm::Type::getIntNTy(C, 16)};
  unsigned ID = Tys[0]->isFPOrFPVectorTy()
      ? llvm::Intrinsic::genx_rdregionf : llvm::Intrinsic::genx_rdregioni;
  llvm::Function *Fn = getIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();

  // Default values: <InRegion>, <VS>, <Width>, <HS>, <Offset>, <ParentWidth>.
  llvm::Value *Args[6] = {
      Region, llvm::ConstantInt::get(FnTy->getParamType(1), 0u),
      llvm::ConstantInt::get(FnTy->getParamType(2), Ty->getVectorNumElements()),
      llvm::ConstantInt::get(FnTy->getParamType(3), 1u),
      llvm::ConstantInt::get(FnTy->getParamType(4), 0u),
      llvm::UndefValue::get(FnTy->getParamType(5))};

  // According to the number of parameters, update the above default arguments.
  unsigned NumParams = E->getNumParameters();

  if (NumParams == 1) {
    // replicate<REP>()
    //
    // VStride = 0, HSize = <size of src>, HStride = 1, Offset = 0
    return CGF.Builder.CreateCall(Fn, Args, "rep");
  }

  // Width
  unsigned Width = EmitAsConstantInt(CGF, E->getRepWExpr());
  Args[2] = llvm::ConstantInt::get(FnTy->getParamType(2), Width);

  // Offset
  if (E->is1D()) {
    // Offset = Offset * sizeof(EltType)
    llvm::Value *Offset = CGF.EmitAnyExpr(E->getRepOffset()).getScalarVal();
    Args[4] = CGF.Builder.CreateMul(Offset,
        llvm::ConstantInt::get(Args[4]->getType(),
                               Ty->getScalarSizeInBits() / 8));
  } else {
    assert(E->is2D());
    // Offset = (VOffset * <BaseWidth> + HOffset ) * sizeof(EltType)
    const CMMatrixType *MT = E->getBase()->getType()->getAs<CMMatrixType>();
    unsigned BaseWidth = MT->getNumColumns();
    llvm::Value *VOffset = CGF.EmitAnyExpr(E->getRepVOffset()).getScalarVal();
    llvm::Value *HOffset = CGF.EmitAnyExpr(E->getRepHOffset()).getScalarVal();
    assert(VOffset->getType() == Args[4]->getType());
    assert(HOffset->getType() == Args[4]->getType());

    llvm::Value *Val = llvm::ConstantInt::get(Args[4]->getType(), BaseWidth);
    llvm::Value *Offset = CGF.Builder.CreateMul(VOffset, Val);
    Offset = CGF.Builder.CreateAdd(Offset, HOffset);
    Args[4] = CGF.Builder.CreateMul(
        Offset, llvm::ConstantInt::get(Offset->getType(),
                                       Ty->getScalarSizeInBits() / 8));
  }

  if (NumParams == 2) {
    // replicate<REP, W>(...)
    //
    // VStride = 0, HSize = W, HStride = 1
    return CGF.Builder.CreateCall(Fn, Args, "rep");
  }

  // VStride
  unsigned VS = EmitAsConstantInt(CGF, E->getRepVSExpr());
  Args[1] = llvm::ConstantInt::get(Args[1]->getType(), VS);

  if (NumParams == 3) {
    // replicate<REP, VS, W>(...)
    //
    // VStride = VS, HSize = W, HStride = 1
    return CGF.Builder.CreateCall(Fn, Args, "rep");
  }

  assert(NumParams == 4 && "unexpected number of parameters for replicate");
  // replicate<REP, VS, W, HS>(...)
  //
  // VStride = VS, HSize = W, HStride = HS
  unsigned HS = EmitAsConstantInt(CGF, E->getRepHSExpr());
  Args[3] = llvm::ConstantInt::get(Args[3]->getType(), HS);

  return CGF.Builder.CreateCall(Fn, Args, "rep");
}

llvm::Value *CGCMRuntime::EmitISelect(CodeGenFunction &CGF,
                                      const CMSelectExpr *E) {
  assert(E->isISelect());

  // Emit the base region.
  llvm::Value *Base = CGF.EmitAnyExpr(E->getBase()).getScalarVal();
  llvm::Type *BaseTy = Base->getType();
  assert(isa<llvm::VectorType>(BaseTy));

  // Emit the first vector argument.
  llvm::Value *A0 = CGF.EmitAnyExpr(E->getISelectIndexExpr(0)).getScalarVal();
  llvm::Type *OffsetTy = A0->getType();
  assert(isa<llvm::VectorType>(OffsetTy));
  assert(OffsetTy->getVectorElementType() == CGF.Int16Ty);

  // Types for overloading: return type, base region type, and offset type.
  llvm::Type *Tys[] = { CGF.ConvertType(E->getType()), BaseTy, OffsetTy };
  unsigned ID = Tys[0]->isFPOrFPVectorTy()
      ? llvm::Intrinsic::genx_rdregionf : llvm::Intrinsic::genx_rdregioni;
  llvm::Function *Fn = getIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();

  // Transform offset from in elements to in bytes.
  llvm::Value *Scale = llvm::ConstantVector::getSplat(
      OffsetTy->getVectorNumElements(),
      llvm::ConstantInt::get(OffsetTy->getVectorElementType(),
                             BaseTy->getScalarSizeInBits() / 8));

  // Compute the vector of offsets.
  llvm::Value *Offset = 0;
  llvm::Value *ParentWidth = 0;

  if (E->is1D()) {
    // 1D case. Reading a region with arguments:
    // vStride = 0, width = 1, hStride = 0, offsets.
    Offset = CGF.Builder.CreateMul(A0, Scale);

    // No parent width available for vectors.
    ParentWidth = llvm::UndefValue::get(FnTy->getParamType(5));
  } else {
    // 2d case. m.iselect(a0, a1) is emitted as m.iselect(c) where m is treated
    // as a vector, and c = a0 * <matrix_width> + a1.
    assert(E->is2D());

    llvm::Value *A1 = CGF.EmitAnyExpr(E->getISelectIndexExpr(1)).getScalarVal();
    // Arguments have the same type <N x i16>.
    assert(A1->getType() == OffsetTy);

    // Compute (a0 * <matrix_width> + a1) * sizeof(BaseEltTy)
    const CMMatrixType *MT = E->getBase()->getType()->getAs<CMMatrixType>();
    unsigned Width = MT->getNumColumns();

    llvm::Value *WidthVal = llvm::ConstantVector::getSplat(
        OffsetTy->getVectorNumElements(),
        llvm::ConstantInt::get(OffsetTy->getVectorElementType(), Width));

    Offset = CGF.Builder.CreateMul(A0, WidthVal);
    Offset = CGF.Builder.CreateAdd(Offset, A1);
    Offset = CGF.Builder.CreateMul(Offset, Scale);

    // Initialize the parent width. Needed?
    ParentWidth = llvm::ConstantInt::get(FnTy->getParamType(5), Width);
  }

  llvm::Value *Args[] = {
      Base,                                             // base
      llvm::ConstantInt::get(FnTy->getParamType(1), 0), // vStride
      llvm::ConstantInt::get(FnTy->getParamType(2), 1), // width
      llvm::ConstantInt::get(FnTy->getParamType(3), 0), // hStride
      Offset,                                           // offsets
      ParentWidth                                       // parent width
  };

  return CGF.Builder.CreateCall(Fn, Args, "iselect");
}

llvm::Value *CGCMRuntime::EmitReadRegionInRows(CGBuilderTy &Builder,
                                               llvm::Value *Region,
                                               unsigned BaseWidth,
                                               unsigned VSize, unsigned VStride,
                                               llvm::Value *VOffset) {
  llvm::Type *Ty = Region->getType();

  // Types for overloading.
  llvm::LLVMContext &C = Region->getContext();
  llvm::Type *Tys[] = {
      llvm::VectorType::get(Ty->getVectorElementType(), VSize * BaseWidth),
      Ty, llvm::Type::getIntNTy(C, 16)};

  unsigned ID = Tys[0]->isFPOrFPVectorTy()
      ? llvm::Intrinsic::genx_rdregionf : llvm::Intrinsic::genx_rdregioni;
  llvm::Function *Fn = getIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();

  // Emit offset in bytes.
  //
  // offset = VOffset * BaseWidth * sizeof(EltType)
  //
  llvm::Type *OffsetTy = VOffset->getType();
  assert(FnTy->getParamType(4) == OffsetTy && "wrong offset type");
  llvm::Value *OffsetVal = Builder.CreateMul(
      Builder.CreateMul(VOffset, llvm::ConstantInt::get(OffsetTy, BaseWidth)),
      llvm::ConstantInt::get(OffsetTy, Ty->getScalarSizeInBits() / 8));

  llvm::Value *Args[6] = {
      Region,
      llvm::ConstantInt::get(FnTy->getParamType(1), VStride * BaseWidth),
      llvm::ConstantInt::get(FnTy->getParamType(2), BaseWidth),
      llvm::ConstantInt::get(FnTy->getParamType(3), 1u), OffsetVal,
      llvm::ConstantInt::get(FnTy->getParamType(5), BaseWidth), // parent width
  };

  return Builder.CreateCall(Fn, Args, "rdr.rows");
}

llvm::Value *CGCMRuntime::EmitReadRegionInCols(CGBuilderTy &Builder,
                                               llvm::Value *Region,
                                               unsigned Height,
                                               unsigned HSize, unsigned HStride,
                                               llvm::Value *HOffset) {
  llvm::Type *Ty = Region->getType();

  // Types for overloading.
  llvm::LLVMContext &C = Region->getContext();
  llvm::Type *Tys[] = {
      llvm::VectorType::get(Ty->getVectorElementType(), Height * HSize),
      Ty, llvm::Type::getIntNTy(C, 16)};

  unsigned ID = Tys[0]->isFPOrFPVectorTy()
      ? llvm::Intrinsic::genx_rdregionf : llvm::Intrinsic::genx_rdregioni;
  llvm::Function *Fn = getIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();

  unsigned Width = Ty->getVectorNumElements() / Height;

  // Offset = HOffset * sizeof(EltTy)
  llvm::Type *OffsetTy = HOffset->getType();
  assert(FnTy->getParamType(4) == OffsetTy && "wrong offset type");

  llvm::Value *OffsetVal = Builder.CreateMul(
      HOffset, llvm::ConstantInt::get(OffsetTy, Ty->getScalarSizeInBits() / 8));

  llvm::Value *Args[6] = {
      Region, llvm::ConstantInt::get(FnTy->getParamType(1), Width),
      llvm::ConstantInt::get(FnTy->getParamType(2), HSize),
      llvm::ConstantInt::get(FnTy->getParamType(3), HStride), OffsetVal,
      llvm::ConstantInt::get(FnTy->getParamType(5), Width) // parent width
  };

  return Builder.CreateCall(Fn, Args, "rdr.cols");
}

llvm::Value *CGCMRuntime::EmitReadRegion1D(CGBuilderTy &Builder,
                                           llvm::Value *Region, unsigned Size,
                                           unsigned Stride,
                                           llvm::Value *Offset) {
  llvm::Type *Ty = Region->getType();

  // Types for overloading.
  llvm::LLVMContext &C = Region->getContext();
  llvm::Type *Tys[] = {llvm::VectorType::get(Ty->getVectorElementType(), Size),
                        Ty, llvm::Type::getIntNTy(C, 16)};

  unsigned ID = Tys[0]->isFPOrFPVectorTy()
      ? llvm::Intrinsic::genx_rdregionf : llvm::Intrinsic::genx_rdregioni;
  llvm::Function *Fn = getIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();

  // Offset = Offset * sizeof(EltTy)
  llvm::Type *OffsetTy = Offset->getType();
  llvm::Value *OffsetVal = Builder.CreateMul(
      Offset, llvm::ConstantInt::get(OffsetTy, Ty->getScalarSizeInBits() / 8));

  if (OffsetVal->getType() != FnTy->getParamType(4))
    OffsetVal = Builder.CreateZExtOrTrunc(OffsetVal, FnTy->getParamType(4));

  llvm::Value *Args[6] = {
      Region, llvm::ConstantInt::get(FnTy->getParamType(1), 0),
      llvm::ConstantInt::get(FnTy->getParamType(2), Size),
      llvm::ConstantInt::get(FnTy->getParamType(3), Stride), OffsetVal,
      llvm::UndefValue::get(FnTy->getParamType(5)) // parent width
  };

  return Builder.CreateCall(Fn, Args, "rdr");
}

// Given a scalar value V, turn this value into a single-element vector with an
// insertelement:
//
// insertelement <1 x Ty> undef, Ty V, i32 0
//
// When V is constant, IR builder will fold it right away.
llvm::Value *getSingleElementVector(CGBuilderTy &Builder, llvm::Value *V) {
  // noop for a vector.
  if (isa<llvm::VectorType>(V->getType()))
    return V;

  llvm::LLVMContext &C = V->getContext();
  llvm::Type *VTy = llvm::VectorType::get(V->getType(), 1);
  return Builder.CreateInsertElement(
      llvm::UndefValue::get(VTy), V,
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(C), 0));
}

llvm::Value *CGCMRuntime::EmitWriteRegion1D(CGBuilderTy &Builder,
                                            llvm::Value *Dst, llvm::Value *Src,
                                            unsigned Size, unsigned Stride,
                                            llvm::Value *Offset,
                                            llvm::Value *Mask) {
  // If src is a scalar, then turn it into a vector of single element.
  Src = getSingleElementVector(Builder, Src);

  // By default Mask = 1, unless specified.
  if (Mask == 0) {
    llvm::Type *MaskTy = llvm::Type::getInt1Ty(Dst->getContext());
    Mask = llvm::ConstantInt::get(MaskTy, 1u);
  }

  // Types for overloading.
  llvm::LLVMContext &C = Src->getContext();
  llvm::Type *Tys[] = {Dst->getType(), Src->getType(),
                       llvm::Type::getIntNTy(C, 16), Mask->getType()};

  unsigned ID = Tys[1]->isFPOrFPVectorTy()
      ? llvm::Intrinsic::genx_wrregionf : llvm::Intrinsic::genx_wrregioni;
  llvm::Function *Fn = getIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();

  // Offset = Offset * sizeof(EltTy)
  llvm::Type *OffsetTy = Offset->getType();
  Offset = Builder.CreateMul(
      Offset,
      llvm::ConstantInt::get(OffsetTy, Tys[0]->getScalarSizeInBits() / 8));
  if (OffsetTy != FnTy->getParamType(5))
    Offset = Builder.CreateZExtOrTrunc(Offset, FnTy->getParamType(5));

  llvm::Value *Args[] = {Dst,
                         Src,
                         llvm::ConstantInt::get(FnTy->getParamType(2), 0),
                         llvm::ConstantInt::get(FnTy->getParamType(3), Size),
                         llvm::ConstantInt::get(FnTy->getParamType(4), Stride),
                         Offset,
                         llvm::UndefValue::get(FnTy->getParamType(6)), // parent width
                         Mask};

  return Builder.CreateCall(Fn, Args, "wrregion");
}

RValue CGCMRuntime::EmitCMReadRegion(CodeGenFunction &CGF, LValue LV) {
  assert(LV.isCMRegion() && "not a region");

  const CGCMRegionInfo &RI = LV.getCMRegionInfo();
  LValue Base = RI.getBase();

  // format lvalue read.
  if (RI.isFormat()) {
    assert(LV.getType()->isCMReferenceType());
    llvm::Type *Ty = CGF.ConvertType(LV.getType());
    Ty = cast<llvm::PointerType>(Ty)->getElementType();

    llvm::Value *Result =
        CGF.EmitLoadOfLValue(Base, SourceLocation()).getScalarVal();

    llvm::Type *BaseTy = Result->getType();
    assert(Ty->isVectorTy() && BaseTy->isVectorTy());

    if (Ty != BaseTy) {
      assert(BaseTy->getPrimitiveSizeInBits() == Ty->getPrimitiveSizeInBits() &&
             "invalid format");
      Result = CGF.Builder.CreateBitCast(Result, Ty, "cast");
    }
    // format is no-op if type matches.
    return RValue::get(Result);
  }

  // select lvalue.
  assert(RI.isSelect());
  llvm::Value *Region = 0;
  if (Base.isSimple()) {
    // load the base object value.
    Region = CGF.EmitLoadOfLValue(Base, SourceLocation()).getScalarVal();
  } else {
    assert(Base.isCMRegion() && "not a CM region");
    // recursively read its base.
    Region = EmitCMReadRegion(CGF, Base).getScalarVal();
  }

  if (RI.is1DRegion()) {
    Region = EmitReadRegion1D(CGF.Builder, Region, RI.getSize(), RI.getStride(),
                              RI.getOffset());

    // If this is an element access then returns a scalar not a vector.
    if (!LV.getType()->isCMVectorMatrixType())
      Region = CGF.Builder.CreateExtractElement(
          Region, llvm::ConstantInt::get(CGF.Int32Ty, 0));

    return RValue::get(Region);
  }

  assert(RI.is2DRegion());
  unsigned BaseWidth = RI.getBaseWidth();

  // read region in rows.
  if ((RI.getBaseHeight() != RI.getVSize()) || (RI.getVOffset() != 0))
    Region = EmitReadRegionInRows(CGF.Builder, Region, BaseWidth, RI.getVSize(),
                                  RI.getVStride(), RI.getVOffset());

  unsigned Height = RI.getVSize();

  // read region in columns.
  if ((BaseWidth != RI.getHSize()) || (RI.getHOffset() != 0))
    Region = EmitReadRegionInCols(CGF.Builder, Region, Height, RI.getHSize(),
                                  RI.getHStride(), RI.getHOffset());

  // If this is an element access then returns a scalar not a vector.
  if (!LV.getType()->isCMVectorMatrixType())
    Region = CGF.Builder.CreateExtractElement(
        Region, llvm::ConstantInt::get(CGF.Int32Ty, 0));

  // The final region value.
  return RValue::get(Region);
}

llvm::Value *CGCMRuntime::EmitWriteRegion2D(
    CGBuilderTy &Builder, llvm::Value *Dst, unsigned BaseWidth,
    llvm::Value *Src, unsigned VSize, unsigned VStride, unsigned HSize,
    unsigned HStride, llvm::Value *VOffset, llvm::Value *HOffset) {

  // If src is a scalar, then turn it into a vector of single element.
  Src = getSingleElementVector(Builder, Src);

  // Types for overloading.
  llvm::Type *MaskTy = llvm::Type::getInt1Ty(Dst->getContext());
  llvm::LLVMContext &C = Src->getContext();
  llvm::Type *Tys[] = {Dst->getType(), Src->getType(),
      llvm::Type::getIntNTy(C, 16), MaskTy};

  unsigned ID = Tys[1]->isFPOrFPVectorTy()
      ? llvm::Intrinsic::genx_wrregionf : llvm::Intrinsic::genx_wrregioni;
  llvm::Function *Fn = getIntrinsic(ID, Tys);
  llvm::FunctionType *FnTy = Fn->getFunctionType();

  llvm::Type *OffsetTy = VOffset->getType();
  assert(FnTy->getParamType(5) == OffsetTy && "wrong offset type");

  // Linearlized offset: Offset = VOffset * BaseWidth + HOffset
  llvm::Value *Offset = Builder.CreateAdd(
      Builder.CreateMul(VOffset, llvm::ConstantInt::get(OffsetTy, BaseWidth)),
      HOffset);

  // Offset = Offset * sizeof(EltTy)
  Offset = Builder.CreateMul(
      Offset,
      llvm::ConstantInt::get(OffsetTy, Tys[0]->getScalarSizeInBits() / 8));

  llvm::Value *Args[] = {
      Dst,
      Src,
      llvm::ConstantInt::get(FnTy->getParamType(2), BaseWidth * VStride),
      llvm::ConstantInt::get(FnTy->getParamType(3), HSize),
      llvm::ConstantInt::get(FnTy->getParamType(4), HStride),
      Offset,
      llvm::ConstantInt::get(FnTy->getParamType(6), BaseWidth), // parent width
      llvm::ConstantInt::get(MaskTy, 1u)};

  return Builder.CreateCall(Fn, Args, "wrregion");
}

void CGCMRuntime::EmitCMWriteRegion(CodeGenFunction &CGF, RValue Src,
                                    LValue LV) {
  assert(LV.isCMRegion() && "not a region");
  const CGCMRegionInfo &RI = LV.getCMRegionInfo();

  // For a format, convert Src to the expected base type and write to base.
  if (RI.isFormat()) {
    llvm::Value *SrcVal = Src.getScalarVal();
    assert(isa<llvm::VectorType>(SrcVal->getType()));
    assert(LV.getType()->isCMReferenceType());
    llvm::Type *Ty = CGF.ConvertType(LV.getType());
    Ty = cast<llvm::PointerType>(Ty)->getElementType();

    llvm::Type *BaseTy = CGF.ConvertType(RI.getBase().getType());
    if (llvm::PointerType *PtrTy = dyn_cast<llvm::PointerType>(BaseTy))
      BaseTy = PtrTy->getElementType();

    assert(Ty->isVectorTy() && BaseTy->isVectorTy());
    if (Ty != BaseTy) {
      // FIXME: the following assertion is not true when src size is smaller
      // then the base size.
      assert(BaseTy->getPrimitiveSizeInBits() == Ty->getPrimitiveSizeInBits() &&
             "invalid format");
      llvm::Value *Cast = CGF.Builder.CreateBitCast(SrcVal, BaseTy, "cast");
      return CGF.EmitStoreThroughLValue(RValue::get(Cast), RI.getBase());
    } else {
      // format is a noop.
      return CGF.EmitStoreThroughLValue(Src, RI.getBase());
    }
  }

  assert(RI.isSelect());
  // Load the current value.
  RValue OldVal = CGF.EmitLoadOfLValue(RI.getBase(), SourceLocation());
  llvm::Value *NewVal = 0;

  if (RI.is1DRegion()) {
    // Merge the old value with the new value.
    NewVal = EmitWriteRegion1D(CGF.Builder, OldVal.getScalarVal(),
                               Src.getScalarVal(), RI.getSize(), RI.getStride(),
                               RI.getOffset());
  } else {
    assert(RI.is2DRegion());
    NewVal = EmitWriteRegion2D(
        CGF.Builder, OldVal.getScalarVal(), RI.getBaseWidth(),
        Src.getScalarVal(), RI.getVSize(), RI.getVStride(), RI.getHSize(),
        RI.getHStride(), RI.getVOffset(), RI.getHOffset());
  }

  // Store the merged region value back.
  CGF.EmitStoreThroughLValue(RValue::get(NewVal), RI.getBase());
}

// Emit lvalue for referencing a CM vector_ref/matrix_ref variable.
//
// [1] vector<float, 8> v1 = 0;
// [2] vector_ref<float, 4> v2 = v1.select<4, 2>(0);
// [3] read(idx, 0, v2);
//
// Emit [1] normally as
//
// %v1 = alloca <8 x float>
// store <8 x float> zeroinitializer, %v1
//
// [2] generates no code, but the (region) lvalue from the RHS is cached.
//
// Emit [3]'s DeclRefExpr as the lvalue cached.
LValue CGCMRuntime::EmitCMDeclRefLValue(CodeGenFunction &CGF,
                                        const DeclRefExpr *E) {
  assert(E->getType()->isCMReferenceType());
  assert(ReferenceDecls.count(E->getDecl()) && "not emitted yet");
  return ReferenceDecls[E->getDecl()];
}

void CGCMRuntime::EmitCMRefDeclInit(CodeGenFunction &CGF, const ValueDecl *VD,
                                    LValue LV) {
  assert(!ReferenceDecls.count(VD) && "already emitted");
  ReferenceDecls.insert(std::make_pair(VD, LV));
}

// Emit initializer row by row. Zero will be filled if entries are not
// initialized. E.g
//
// const int A[2][4] = {{1, 2},{3}};
//
// produces {1, 2, 0, 0, 3, 0, 0, 0}.
//
static void EmitConstantInRows(std::vector<llvm::Constant *> &Vals,
                               const Expr *E, CodeGenFunction &CGF) {
  assert(E->getType()->isConstantArrayType());
  const ConstantArrayType *AT = cast<ConstantArrayType>(E->getType());

  // base case.
  if (AT->getElementType()->isCMElementType()) {
    unsigned Count = 0;
    for (Stmt::const_child_iterator CI = E->child_begin(), CE = E->child_end();
         CI != CE; ++CI, ++Count) {
      const Expr *Item = static_cast<const Expr *>(*CI);
      llvm::Constant *IVal = 0;
      Expr::EvalResult Result;
      if (Item->EvaluateAsRValue(Result, CGF.getContext())) {
        APValue APV = Result.Val;
        if (APV.isInt())
          IVal = llvm::ConstantInt::get(CGF.getLLVMContext(), APV.getInt());
        else if (APV.isFloat())
          IVal = llvm::ConstantFP::get(CGF.getLLVMContext(), APV.getFloat());
      }

      if (!IVal)
        CGF.CGM.Error(Item->getExprLoc(), "not a constant initializer");

      Vals.push_back(IVal);
    }

    // The remaining entries in this row is initialized to zero.
    uint64_t ArraySize = AT->getSize().getZExtValue();
    if (Count < ArraySize) {
      llvm::Type *EltTy = CGF.ConvertType(AT->getElementType());
      llvm::Constant *NullVal = llvm::Constant::getNullValue(EltTy);
      Vals.insert(Vals.end(), ArraySize - Count, NullVal);
    }

    // row finished.
    return;
  }

  // recurse for each subexpr.
  for (Stmt::const_child_iterator CI = E->child_begin(), CE = E->child_end();
       CI != CE; ++CI)
    EmitConstantInRows(Vals, static_cast<const Expr *>(*CI), CGF);
}

// Compute the llvm cast op, given dst element type and initilizer element type.
bool CGCMRuntime::getCastOpKind(llvm::Instruction::CastOps &CastOp,
                                CodeGenFunction &CGF, QualType DstType,
                                QualType SrcType) {
  DstType = DstType->getCanonicalTypeUnqualified();
  SrcType = SrcType->getCanonicalTypeUnqualified();

  llvm::Type *DstTy = CGF.ConvertType(DstType);
  llvm::Type *SrcTy = CGF.ConvertType(SrcType);

  if (DstType == SrcType)
    return false;

  // float->double or double->float.
  if (DstTy->isFloatingPointTy() && SrcTy->isFloatingPointTy()) {
    CastOp = (DstTy->getTypeID() < SrcTy->getTypeID())
                 ? llvm::Instruction::FPTrunc
                 : llvm::Instruction::FPExt;
    return true;
  }
  // int->short etc.
  if (DstTy->isIntegerTy() && SrcTy->isIntegerTy()) {
    if (SrcTy->getScalarSizeInBits() > DstTy->getScalarSizeInBits())
      CastOp = llvm::Instruction::Trunc;
    else
      CastOp = SrcType->isUnsignedIntegerType() ? llvm::Instruction::ZExt
                                                : llvm::Instruction::SExt;
    return true;
  }
  // float->int etc.
  if (DstTy->isIntegerTy() && SrcTy->isFloatingPointTy()) {
    CastOp = DstType->isUnsignedIntegerType() ? llvm::Instruction::FPToUI
                                              : llvm::Instruction::FPToSI;
    return true;
  }
  // int->float etc.
  if (DstTy->isFloatingPointTy() && SrcTy->isIntegerTy()) {
    CastOp = SrcType->isUnsignedIntegerType() ? llvm::Instruction::UIToFP
                                              : llvm::Instruction::SIToFP;
    return true;
  }

  llvm_unreachable("invalid conversion");
}

static QualType getArrayInitElementType(QualType T) {
  if (const ArrayType *AT = dyn_cast<ArrayType>(T))
    return getArrayInitElementType(AT->getElementType());

  return T;
}

// The initial values of the global array or the initializer-array are
// sequentially copied to the vector or matrix. Initializer array could be
// bigger or smaller than the initialized vector/matrix. Initialization would be
// done up to the minimum of their sizes. Initializer array can have more than
// one dimension - this gives more flexibility to users allowing them to skip
// initialization of some segments of the matrix or vector.
//
// const short init[5] = {0, 1, 2, 3, 4};
// vector<int, 8> v(init);
//
void CGCMRuntime::EmitCMConstantInitializer(
    CodeGenFunction &CGF, const CodeGenFunction::AutoVarEmission &E) {

  const VarDecl *VD = E.Variable;
  QualType VarType = VD->getType();
  assert(VarType->isCMBaseType());
  llvm::Type *DstTy = CGF.ConvertType(VarType);

  const Expr *Init = VD->getInit()->IgnoreParenImpCasts();
  assert(isa<DeclRefExpr>(Init));
  const VarDecl *InitVar = cast<VarDecl>(cast<DeclRefExpr>(Init)->getDecl());

  // Get all the constant values in rows.
  std::vector<llvm::Constant *> ValInRows;
  EmitConstantInRows(ValInRows, InitVar->getInit(), CGF);

  // Generate constant initializer.
  unsigned DstNumElts = VarType->getCMVectorMatrixSize();
  unsigned SrcNumElts = ValInRows.size();

  ArrayRef<llvm::Constant *> Vals(ValInRows.data(),
                                  std::min(DstNumElts, SrcNumElts));
  llvm::Value *Src = llvm::ConstantVector::get(Vals);

  // Perform necessary cast.
  llvm::Instruction::CastOps CastOp;
  if (getCastOpKind(CastOp, CGF, VarType->getCMVectorMatrixElementType(),
                    getArrayInitElementType(Init->getType()))) {
    llvm::Type *NewSrcTy = llvm::VectorType::get(
        DstTy->getVectorElementType(), std::min(DstNumElts, SrcNumElts));
    if ((CastOp == llvm::Instruction::FPToUI
         && NewSrcTy->getScalarType()->getPrimitiveSizeInBits() <= 32)
        || (CastOp == llvm::Instruction::FPToSI
         && NewSrcTy->getScalarType()->getPrimitiveSizeInBits() < 32)) {
      // CM: float->any int goes via signed int unless destination type
      // is signed int.
      auto InterTy = llvm::VectorType::get(llvm::Type::getInt32Ty(
            DstTy->getContext()), NewSrcTy->getVectorNumElements());
      Src = CGF.Builder.CreateFPToSI(Src, InterTy);
      CastOp = llvm::Instruction::Trunc;
    }
    Src = CGF.Builder.CreateCast(CastOp, Src, NewSrcTy);
  }

  // If initializer has less values, then use write region to insert values.
  // Otherwise, emit a simple store.
  LValue LV = CGF.MakeAddrLValue(E.Addr, VarType);
  if (DstNumElts > SrcNumElts)
    Src = EmitWriteRegion1D(CGF.Builder, llvm::UndefValue::get(DstTy), Src,
                            SrcNumElts, /*Stride*/ 1,
                            llvm::ConstantInt::get(CGF.Int16Ty, 0));

  return CGF.EmitStoreThroughLValue(RValue::get(Src), LV, true);
}

// CM kernel metadata is organized as follows:
//
// !genx.kernels = !{ !0, !1 }
// !0 = metadata !{ @func0, <kernel-name>, <asm-name>, <arg-kinds>, <slm-size>, ... }
// !1 = metadata !{ @func1, <kernel-name>, <asm-name>, <arg-kinds>, <slm-size>, ... }
//
// where:
//  * <kernel-name> is an MDString metadata, describing the kernel name;
//  * <asm-name> is an MDString metadata, describing the filename that
//    contains the kernel, with path excluded;
//  * <arg-kinds> is a metadata node containing N i32s, where N is
//    the number of kernel arguments, and each i32 is the kind of argument,
//    one of: 0 = general, 1 = sampler, 2 = surface, 3 = vme (the same values
//    as in the "kind" field of an "input_info" record in a vISA kernel).
//  * <slm-size> is the slm size in bytes for this kernel.
//  * <arg-offsets> is a dummy entry, a placeholder for the kernel arg offsets
//    calculated in the CMKernelArgOffset pass.
//  * <arg-inout-kind> is a metadata node containing N i32s where N is the
//    number of kernel arguments, and each i32 is one of
//      0 = normal
//      1 = input
//      2 = output
//      3 = input and output
//  * <argtype_desc> is a metadata node describing N strings where N is the
//    number of kernel arguments, each string describing argument type in OpenCL.
//
void CGCMRuntime::EmitCMKernelMetadata(const FunctionDecl *FD,
                                       llvm::Function *Fn) {
  if (!FD->hasAttr<CMGenxMainAttr>())
    return;

  // Starting with the non-mangled kernel name.
  SmallString<32> NameStr(FD->getName());
  llvm::raw_svector_ostream KernelName(NameStr);

  // If this is a template then append template argument types or values.
  if (FD->isTemplateInstantiation()) {
    // <kernel-name> := <kernel-name> + '<' + <template-arg-seq> + '>'
    // <template-arg-seq> := <template-type-name> + ", " + <template-arg-seq>
    //                    := <template-arg-value> + ", " + <template-arg-seq>
    // <template-arg-value> := <signed-int-val>    for signed or bool parameters
    //                      := <unsigned-int-val> + 'U'  for unsigned parameters
    const TemplateArgumentList *Args = FD->getTemplateSpecializationArgs();

    KernelName << '<';
    for (unsigned I = 0, N = Args->size(); I < N; ++I) {
      FunctionTemplateDecl *FTD = FD->getPrimaryTemplate();
      NamedDecl *ND = FTD->getTemplateParameters()->getParam(I);
      const TemplateArgument &TA = Args->get(I);

      switch (TA.getKind()) {
      case TemplateArgument::Type:
        KernelName << TA.getAsType().getCanonicalType().getAsString();
        break;
      case TemplateArgument::Integral: {
        NonTypeTemplateParmDecl *NTT = cast<NonTypeTemplateParmDecl>(ND);

        // Follow the old compiler's mangling, although it is buggy here.
        if (NTT->getType()->isEnumeralType()) {
          // the old compiler mangles this to the enum text name.
          // But this name has been replaced by its integeral value and
          // we cannot retrieve it from AST. Just give up.
          Error(ND->getLocation(), "cannot mangle enum!");
          return;
        }

        // Append 'U' if this parameter is unsigned. Note that bool type is
        // treated as unsigned in clang but the old compiler treats as signed!
        if (NTT->getType()->isBooleanType() || NTT->getType()->isSignedIntegerType())
          KernelName << TA.getAsIntegral().getSExtValue();
        else
          KernelName << TA.getAsIntegral().getZExtValue() << 'U';
      } break;
      default:
        Error(ND->getLocation(), "cannot mangle this template argument!");
        return;
      }
      if (I != N - 1)
        KernelName << ", ";
    }
    KernelName << '>';
  }

  // Retrieve the root named MD node.
  llvm::LLVMContext &Context = Fn->getContext();
  llvm::Module *M = Fn->getParent();
  llvm::NamedMDNode *Kernels = M->getOrInsertNamedMetadata("genx.kernels");

  // AsmName := <base-filename> + '_' + <index> + ".asm"
  SourceManager &SM = FD->getASTContext().getSourceManager();
  // we'll use the expansion location file just in case the kernel was created
  // by a macro in a header file (which would otherwise give a null filename)
  SourceLocation KernelLoc = SM.getExpansionLoc(FD->getLocation());
  StringRef BaseName = llvm::sys::path::stem(SM.getFilename(KernelLoc));
  // Asm file BaseName value may be specified explicitly - we don't use any
  // path or extension that may have been provided, and force the extension to
  // ".asm". If the stem of the supplied name is null, we use the stem of the
  // source file name.
  if (!CGM.getCodeGenOpts().GenXAsmName.empty()) {
    StringRef AsmStem = llvm::sys::path::stem(CGM.getCodeGenOpts().GenXAsmName);
    if (!AsmStem.empty() && (AsmStem.find_first_not_of('.') != StringRef::npos))
      BaseName = AsmStem;
  }
  std::string AsmName = (BaseName + llvm::Twine('_') +
                        llvm::Twine(Kernels->getNumOperands()) +
                        llvm::Twine(".asm")).str();

  // Kernel arg kinds
  llvm::Type *I32Ty = llvm::Type::getInt32Ty(Context);
  llvm::SmallVector<llvm::Metadata *, 8> ArgKinds;
  llvm::SmallVector<llvm::Metadata *, 8> ArgInOutKinds;
  llvm::SmallVector<llvm::Metadata *, 8> ArgTypeDescs;
  enum { AK_NORMAL, AK_SAMPLER, AK_SURFACE, AK_VME };
  enum { IK_NORMAL, IK_INPUT, IK_OUTPUT, IK_INPUT_OUTPUT };
  for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end();
      i != e; ++i) {
    const ParmVarDecl *PVD = *i;

    // Generate argument type descriptor if any.
    StringRef ArgDesc = "";
    if (auto AT = PVD->getAttr<CMOpenCLTypeAttr>())
      ArgDesc = AT->getType_desc();
    ArgTypeDescs.push_back(llvm::MDString::get(Context, ArgDesc));

    const Type *T = PVD->getTypeSourceInfo()->getType().getTypePtr();
    int Kind = AK_NORMAL;
    if (T->isCMSamplerIndexType())
      Kind = AK_SAMPLER;
    else if (T->isCMSurfaceIndexType())
      Kind = AK_SURFACE;
    else if (T->isCMVectorType()) {
      // If this is a vector of SurfaceIndex then we need to set Kind
      // appropriately (to get correct metadata)
      const CMVectorType *VT = cast<CMVectorType>(T->getCanonicalTypeInternal());
      if (VT->getElementType()->isCMSurfaceIndexType())
        Kind = AK_SURFACE;
      else if (VT->getElementType()->isCMSamplerIndexType())
        Kind = AK_SAMPLER;
    } else if (T->isCMVmeIndexType())
      Kind = AK_VME;
    ArgKinds.push_back(getMD(llvm::ConstantInt::get(I32Ty, Kind)));

    // IN + OUT = IN_OUT
    // IN + IN_OUT = IN_OUT
    // OUT + IN_OUT = IN_OUT
    int IKind = IK_NORMAL;
    if (PVD->hasAttr<CMInputOutputAttr>())
      IKind = IK_INPUT_OUTPUT;
    else {
      if (PVD->hasAttr<CMInputAttr>())
        IKind |= IK_INPUT;
      if (PVD->hasAttr<CMOutputAttr>())
        IKind |= IK_OUTPUT;
    }
    ArgInOutKinds.push_back(getMD(llvm::ConstantInt::get(I32Ty, IKind)));
  }

  llvm::MDNode *Kinds = llvm::MDNode::get(Context, ArgKinds);
  llvm::MDNode *IOKinds = llvm::MDNode::get(Context, ArgInOutKinds);
  llvm::MDNode *ArgDescs = llvm::MDNode::get(Context, ArgTypeDescs);
  llvm::Metadata *MDArgs[] = {
      getMD(Fn),
      llvm::MDString::get(Context, KernelName.str()),
      llvm::MDString::get(Context, AsmName),
      Kinds,
      getMD(llvm::ConstantInt::getNullValue(I32Ty)),
      getMD(llvm::ConstantInt::getNullValue(I32Ty)), // placeholder for arg offsets
      IOKinds,
      ArgDescs
  };

  // Add this kernel to the root.
  Kernels->addOperand(llvm::MDNode::get(Context, MDArgs));
}

void CGCMRuntime::finalize() {
  auto Kernels = CGM.getModule().getOrInsertNamedMetadata("genx.kernels");

  // Mark this kernel to emit code compatible with OpenCL runtime.
  StringRef Attr = CGM.getCodeGenOpts().EmitCmOCL ? "true" : "false";
  for (auto K : Kernels->operands()) {
    llvm::Metadata *M = K->getOperand(0).get();
    if (auto F = dyn_cast_or_null<llvm::Function>(getVal(M)))
      F->addFnAttr("oclrt", Attr);
  }

  if (CGM.getCodeGenOpts().EmitCMGlobalsAsVolatile)
    for (auto &GV : CGM.getModule().getGlobalList()) {
      llvm::Type *EltTy = GV.getType()->getElementType();
      // At least 64 floats, or 8 GRFs
      const unsigned THRESHOLD = sizeof(float) * 8 * 64;
      if (EltTy->isVectorTy() && EltTy->getPrimitiveSizeInBits() >= THRESHOLD)
        GV.addAttribute("genx_volatile");
    }

  // Reverse kernel ASM names during codegen.
  // This provides an option to match the old compiler's output.
  if (CGM.getCodeGenOpts().ReverseCMKernelList) {
    auto I = Kernels->op_begin();
    auto E = Kernels->op_end();
    // Reverse the ASM name nodes.
    while (I != E && I != --E) {
      auto LHS = *I;
      auto RHS = *E;
      llvm::Metadata *AsmName = LHS->getOperand(2).get();
      LHS->replaceOperandWith(2, RHS->getOperand(2).get());
      RHS->replaceOperandWith(2, AsmName);
      ++I;
    }
  }
}

/// \brief Emit kernel output marker calls.
void CGCMRuntime::EmitCMOutput(CodeGenFunction &CGF) {
  auto FD = dyn_cast_or_null<FunctionDecl>(CGF.CurFuncDecl);
  if (!FD || !FD->hasAttr<CMGenxMainAttr>())
    return;

  SmallVector<llvm::Value *, 8> OutArgs;
  auto &Builder = CGF.Builder;
  for (auto VD : FD->parameters()) {
    if (VD->hasAttr<CMOutputAttr>() || VD->hasAttr<CMInputOutputAttr>()) {
      Address Addr = CGF.GetAddrOfLocalVar(VD);
      OutArgs.push_back(Builder.CreateLoad(Addr));
    }
  }

  if (!OutArgs.empty()) {
    auto OutFn = getIntrinsic(llvm::Intrinsic::genx_output);
    Builder.CreateCall(OutFn, OutArgs);
  }
}
namespace {

/// \brief Cleanup to ensure that by ref arguments get updated properly.
struct ReferenceArgWriteback : public EHScopeStack::Cleanup {
  /// \brief The address of variable being used for reference argument passing.
  llvm::Value *Address;
  /// \brief The target lvalue for this reference argument.
  LValue Target;

public:
  ReferenceArgWriteback(llvm::Value *Addr, LValue LV)
      : Address(Addr), Target(LV) {}

  void Emit(CodeGenFunction &CGF, Flags F) {
    // Load the returned value in the temporary variable.
    llvm::Value *Result = CGCMRuntime::EmitCMRefLoad(CGF, Address);

    // Update the target lvalue with the returned value.
    CGF.CGM.getCMRuntime().EmitCMWriteRegion(CGF, RValue::get(Result), Target);
  }
};

} // namespace

llvm::Value *CGCMRuntime::EmitCMReferenceArg(CodeGenFunction &CGF, LValue LV) {
  assert(LV.isCMRegion());
  QualType VarType = CGF.getContext().getCMVectorMatrixBaseType(LV.getType());
  llvm::Value *Address = CGF.CreateMemTemp(VarType, "argref").getPointer();
  llvm::Value *Region = EmitCMReadRegion(CGF, LV).getScalarVal();
  CGCMRuntime::EmitCMRefStore(CGF, Region, Address);

  // Reference argument writeback is implemented as a cleanup, using the
  // existing C++ cleanup mechanism.
  CGF.EHStack.pushCleanup<ReferenceArgWriteback>(NormalAndEHCleanup, Address, LV);

  return Address;
}

static bool isScalar(llvm::Value *Addr) {
  auto EltTy = Addr->getType()->getPointerElementType();
  if (EltTy->isVectorTy())
    return EltTy->getVectorNumElements() == 1;
  return true;
}

llvm::Value *CGCMRuntime::EmitCMRefLoad(CodeGenFunction &CGF, llvm::Value *Addr) {

  if (!CGF.CGM.getCodeGenOpts().EmitVLoadStore || isScalar(Addr))
    return CGF.Builder.CreateDefaultAlignedLoad(Addr);

  unsigned ID = llvm::Intrinsic::genx_vload;
  llvm::Type *Tys[] = {Addr->getType()->getPointerElementType(),
                       Addr->getType()};
  llvm::Function *Fn = CGF.CGM.getIntrinsic(ID, Tys);
  llvm::Value *Result = CGF.Builder.CreateCall(Fn, Addr, "ref.load");
  return Result;
}

void CGCMRuntime::EmitCMRefStore(CodeGenFunction &CGF, llvm::Value *Val,
                                 llvm::Value *Addr) {

  if (!CGF.CGM.getCodeGenOpts().EmitVLoadStore || isScalar(Addr)) {
    CGF.Builder.CreateDefaultAlignedStore(Val, Addr);
    return;
  }

  unsigned ID = llvm::Intrinsic::genx_vstore;
  llvm::Type *Tys[] = {Val->getType(), Addr->getType()};
  llvm::Function *Fn = CGF.CGM.getIntrinsic(ID, Tys);
  llvm::Value *Vals[] = {Val, Addr};
  CGF.Builder.CreateCall(Fn, Vals);
}
