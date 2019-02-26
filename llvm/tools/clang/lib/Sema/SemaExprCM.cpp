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
//
//  This file implements semantic functions specific to MDF CM
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCM.h"
#include "clang/AST/ExprCXX.h"
#include "llvm/ADT/SmallBitVector.h"
using namespace clang;
using namespace sema;

// \brief Semantic actions for CM vector/matrix all() member function
ExprResult Sema::ActOnCMAll(SourceLocation AllLoc, Expr *Base,
                            SourceLocation RParenLoc) {
  QualType BaseTy = Base->getType();
  if (BaseTy->isCMVectorMatrixType()) {
    QualType EltTy = Base->getType()->getCMVectorMatrixElementType();
    // TODO: we should maybe be stricter here regarding base element type?
    if (!EltTy->isDependentType() && !(EltTy->isIntegralOrEnumerationType() ||
                                       EltTy->isFloatingType())) {
      Diag(Base->getExprLoc(), diag::err_cm_inappropriate_mask_object) << 0;
      return ExprError();
    }
  }

  return BuildCMBoolReductionExpr(Base, CMBoolReductionExpr::RK_all, AllLoc,
                                  RParenLoc);
}

// \brief Semantic actions for CM vector/matrix any() member function
ExprResult Sema::ActOnCMAny(SourceLocation AnyLoc, Expr *Base,
                            SourceLocation RParenLoc) {
  QualType BaseTy = Base->getType();
  if (BaseTy->isCMVectorMatrixType()) {
    QualType EltTy = Base->getType()->getCMVectorMatrixElementType();
    // TODO: we should maybe be stricter here regarding base element type?
    if (!EltTy->isDependentType() && !(EltTy->isIntegralOrEnumerationType() ||
                                       EltTy->isFloatingType())) {
      Diag(Base->getExprLoc(), diag::err_cm_inappropriate_mask_object) << 1;
      return ExprError();
    }
  }
  return BuildCMBoolReductionExpr(Base, CMBoolReductionExpr::RK_any, AnyLoc,
                                  RParenLoc);
}

ExprResult Sema::BuildCMBoolReductionExpr(
    Expr *Base, CMBoolReductionExpr::CMBoolReductionKind RK,
    SourceLocation ReductionLoc, SourceLocation RParenLoc) {
  ExprResult Result = DefaultLvalueConversion(Base);
  if (!Result.isUsable())
    return ExprError();
  return new (Context)
      CMBoolReductionExpr(Context, RK, Result.get(), ReductionLoc, RParenLoc);
}

// \brief Semantic actions for CM matrix column() member function
ExprResult Sema::ActOnCMColumn(SourceLocation ColumnLoc, Expr *Base,
                               Expr *ColExpr, SourceLocation RParenLoc) {
  assert(Base && "null base expression");
  // defer the analysis if the base is dependent.
  if (Base->isTypeDependent()) {
    QualType BaseTy = Base->getType();
    QualType ExprTy = Context.DependentTy;
    if (BaseTy->isCMVectorMatrixType()) {
      Expr *CE = IntegerLiteral::Create(Context, llvm::APInt(32, 1),
                                        Context.IntTy, ColumnLoc);
      ExprTy = Context.getDependentCMMatrixType(
          /*IsRef*/ true, BaseTy->getCMVectorMatrixElementType(),
          /*RowExpr*/ nullptr, /*ColExpr*/ CE, ColumnLoc, ColumnLoc, ColumnLoc);
    }

    return new (Context)
        CMSelectExpr(Context, CMSelectExpr::SK_column, Base, ColumnLoc, ColExpr,
                     RParenLoc, ExprTy, VK_LValue);
  }

  if (!Base->getType()->isCMMatrixType()) {
    Diag(ColumnLoc, diag::err_cm_matrix_only_method) << 0
                                                     << Base->getSourceRange();
    return ExprError();
  }

  if (!ColExpr->getType()->isDependentType() &&
      !ColExpr->getType()->isIntegralOrEnumerationType()) {
    Diag(ColExpr->getExprLoc(), diag::err_cm_non_integer_index) << 1;
    return ExprError();
  }

  QualType BaseTy = Base->getType();
  const CMMatrixType *MT = BaseTy->getAs<CMMatrixType>();

  // Check whether index is out of bound.
  {
    unsigned NColumns = MT->getNumColumns();
    llvm::APSInt Val;
    SourceLocation Loc;
    if (!ColExpr->isValueDependent() &&
        ColExpr->isIntegerConstantExpr(Val, Context, &Loc)) {
      int64_t Index = Val.getSExtValue();
      if (Index < 0) {
        Diag(ColExpr->getExprLoc(), diag::err_cm_negative_index) << 1;
        return ExprError();
      } else if (Index >= NColumns) {
        Diag(ColExpr->getExprLoc(), diag::warn_cm_out_of_bound_index)
            << 1 << (unsigned)Index << NColumns;
      }
    }
  }

  // The column expr type is matrix_ref<T, N, 1>.
  QualType ExprTy = Context.getCMMatrixType(
      /*IsReference*/ true, MT->getElementType(), MT->getNumRows(), 1,
      MT->getVMLoc(), MT->getLessLoc(), MT->getGreaterLoc());

  ExprResult Res = DefaultLvalueConversion(ColExpr);
  Res = ImpCastExprToType(Res.get(), Context.UnsignedShortTy, CK_IntegralCast);
  if (!Res.isUsable())
    return ExprError();

  return new (Context)
      CMSelectExpr(Context, CMSelectExpr::SK_column, Base, ColumnLoc, Res.get(),
                   RParenLoc, ExprTy, VK_LValue);
}

// \brief Semantic actions for CM vector/matrix format() member function
ExprResult Sema::ActOnCMFormat(SourceLocation FormatLoc, Expr *Base,
                               ParsedType FormatElementType, MultiExprArg Args,
                               SourceLocation GreaterLoc,
                               SourceLocation RPLoc) {
  assert(Base && "null base");
  unsigned NumArgs = Args.size();
  if (NumArgs == 1) {
    Diag(GreaterLoc, diag::err_cm_format_too_few_dimensions);
    return ExprError();
  } else if (NumArgs > 2) {
    Diag(Args[2]->getExprLoc(), diag::err_cm_format_too_many_dimensions)
        << SourceRange(Args[2]->getSourceRange().getBegin(),
                       Args[NumArgs - 1]->getSourceRange().getEnd());
    return ExprError();
  }

  QualType ElementType = GetTypeFromParser(FormatElementType);
  return BuildCMFormat(Base, FormatLoc, ElementType, Args, RPLoc);
}

ExprResult Sema::BuildCMFormat(Expr *Base, SourceLocation FormatLoc,
                               QualType ElementType, MultiExprArg Args,
                               SourceLocation RPLoc) {
  // delay semantic analysis for dependent input.
  bool IsDependent = Base->isInstantiationDependent();
  IsDependent |= ElementType->isDependentType();
  for (unsigned I = 0, N = Args.size(); I < N; ++I)
    IsDependent |= Args[I]->isValueDependent() || Args[I]->isTypeDependent();

  QualType BaseTy = Base->getType();
  if (IsDependent) {
    QualType ExprTy = Context.DependentTy;
    if (Args.size() == 0)
      ExprTy = Context.getDependentCMVectorType(
          /*IsRef*/ true, ElementType,
          /*SizeExpr*/ 0, FormatLoc, FormatLoc, FormatLoc);
    else
      ExprTy = Context.getDependentCMMatrixType(
          /*IsRef*/ true, ElementType,
          /*NRowExpr*/ 0, /*NColExpr*/ 0, FormatLoc, FormatLoc, FormatLoc);

    return new (Context) CMFormatExpr(Context, Base, FormatLoc, ElementType,
                                      Args, RPLoc, ExprTy, VK_LValue);
  }

  if (!BaseTy->isCMVectorMatrixType()) {
    Diag(Base->getExprLoc(), diag::err_cm_vector_matrix_type_expected)
        << "format";
    return ExprError();
  }

  // Check the element type.
  if (ElementType.isNull() ||
      !ElementType->isCMElementType(/*AllowNonArithmetic*/ false)) {
    Diag(FormatLoc, diag::err_cm_format_invalid_type) << ElementType;
    return ExprError();
  }

  // getTypeSize rounds up to the next power of two, which seems required for
  // codegen. We compute the real size explicitly. Is this correct?
  QualType BaseEltType = BaseTy->getCMVectorMatrixElementType();
  unsigned BaseEltTySize = Context.getTypeSize(BaseEltType);
  unsigned BaseTySize = BaseEltTySize * BaseTy->getCMVectorMatrixSize();

  unsigned EltTySize = Context.getTypeSize(ElementType);
  QualType FormatType;

  SmallVector<Expr *, 2> ArgExprs;
  unsigned NumArgs = Args.size();
  if (NumArgs == 0) {
    // It is OK to format from <13 x i8> to <4 x i32> but it is KO to format
    // <3 x i8> to <0 x i32>.
    if (BaseTySize / EltTySize == 0) {
      Diag(Base->getExprLoc(), diag::err_cm_format_invalid_size)
          << BaseTy->isCMMatrixType() << (BaseTySize / 8) << 0 << (EltTySize / 8);
      return ExprError();
    }
    unsigned NumElts = BaseTySize / EltTySize;
    FormatType = Context.getCMVectorType(true, ElementType, NumElts, FormatLoc,
                                         FormatLoc, FormatLoc);
  } else {
    assert(NumArgs == 2 && "invalid number of arguments");

    llvm::APSInt ConstVal;
    SourceLocation Loc;
    // Determine the number of rows
    if (!Args[0]->isIntegerConstantExpr(ConstVal, Context, &Loc)) {
      Diag(Loc, diag::err_cm_format_non_const_value) << 0;
      return ExprError();
    }
    int NumRows = ConstVal.getSExtValue();
    if (NumRows < 1) {
      Diag(Args[0]->getExprLoc(), diag::err_cm_format_non_positive_value) << 0;
      return ExprError();
    }
    ExprResult Result = DefaultLvalueConversion(Args[0]);
    if (Result.isUsable())
      ArgExprs.push_back(Result.get());
    else
      return ExprError();

    // Determine the number of columns
    if (!Args[1]->isIntegerConstantExpr(ConstVal, Context, &Loc)) {
      Diag(Loc, diag::err_cm_format_non_const_value) << 1;
      return ExprError();
    }
    int NumCols = ConstVal.getSExtValue();
    if (NumCols < 1) {
      Diag(Args[1]->getExprLoc(), diag::err_cm_format_non_positive_value) << 1;
      return ExprError();
    }
    Result = DefaultLvalueConversion(Args[1]);
    if (Result.isUsable())
      ArgExprs.push_back(Result.get());
    else
      return ExprError();

    unsigned FormatTySize = NumRows * NumCols * EltTySize;

    // Check new matrix must be equal to the size of the source object.
    if (BaseTySize != FormatTySize) {
      Diag(Base->getExprLoc(), diag::err_cm_format_mismatches_source_size)
          << (NumRows * NumCols * EltTySize / 8) << (BaseTySize / 8);
      return ExprError();
    }

    FormatType = Context.getCMMatrixType(true, ElementType, NumRows, NumCols,
                                         FormatLoc, FormatLoc, FormatLoc);

    // If the new matrix is smaller than the size of the source object we need
    // to insert an implicit select so that any bitcast that is generated for
    // this format during CodeGen will be valid. First we Format it as a vector
    // with byte element type, then we select it to the required size. Using
    // byte elements works even when the result size is not divisible by the
    // base element size.
    // The original format can then be safely applied to the result.
    if (BaseTySize > FormatTySize) {
      // Format Base as a uchar vector
      QualType VecFormatType = Context.getCMVectorType(true,
          Context.UnsignedCharTy, BaseTySize / 8,
          FormatLoc, FormatLoc, FormatLoc);
      SmallVector<Expr *, 2> NullArgs;
      Base = new (Context) CMFormatExpr(Context, Base, FormatLoc,
                                        Context.UnsignedCharTy,
                                        NullArgs, RPLoc, VecFormatType,
                                        Base->getValueKind());
      // Select elements to make the result the desired size
      SmallVector<Expr *, 2> SelectArgs;
      llvm::APInt ValSize(16, (int)(FormatTySize / 8), false);
      IntegerLiteral *VecSize = IntegerLiteral::Create(
          Context, ValSize, Context.UnsignedShortTy, RPLoc);
      SelectArgs.push_back(VecSize);
      llvm::APInt ValOne(16, 1, /*isSigned*/ false);
      IntegerLiteral *One = IntegerLiteral::Create(
          Context, ValOne, Context.UnsignedShortTy, RPLoc);
      SelectArgs.push_back(One);
      llvm::APInt ValZero(16, 0, /*isSigned*/ false);
      IntegerLiteral *Zero = IntegerLiteral::Create(
          Context, ValZero, Context.UnsignedShortTy, RPLoc);
      SelectArgs.push_back(Zero);
      QualType VecSelectType = Context.getCMVectorType(true,
          Context.UnsignedCharTy, (int)(FormatTySize / 8),
          FormatLoc, FormatLoc, FormatLoc);
      Base = new (Context) CMSelectExpr(Context, CMSelectExpr::SK_select, Base,
          FormatLoc, SelectArgs, 2, RPLoc, VecSelectType, Base->getValueKind());
    }
  }

  // Everything is good.
  return new (Context) CMFormatExpr(Context, Base, FormatLoc, ElementType,
                                    ArgExprs, RPLoc, FormatType,
                                    Base->getValueKind());
}

// \brief Semantic actions for CM vector/matrix genx_select() member function
ExprResult Sema::ActOnCMGenxSelect(SourceLocation GenxSelectLoc, Expr *Base,
                                   MultiExprArg GenxSelectArgs,
                                   SourceLocation GreaterLoc,
                                   MultiExprArg OffsetArgs) {
  // genx_select is deprecated - simply issue an error.
  Diag(GenxSelectLoc, diag::err_cm_genx_select_deprecated);
  return ExprError();
}

// \brief Semantic actions for CM vector/matrix iselect() member function
ExprResult Sema::ActOnCMISelect(SourceLocation ISelectLoc, Expr *Base,
                                MultiExprArg Args, SourceLocation RPLoc) {
  if (!Base->getType()->isCMVectorMatrixType()) {
    assert(Base->getType()->isDependentType() && "already checked");
    // Defer the sema checking until the instantiation.
    return new (Context)
        CMSelectExpr(Context, CMSelectExpr::SK_iselect, Base, ISelectLoc, Args,
                     RPLoc, Context.DependentTy, VK_RValue);
  }

  // Check the number of arguments.
  bool IsVector = Base->getType()->isCMVectorType();
  // no arguments case has already been caught during Parse
  if (IsVector && Args.size() > 1) {
    Diag(Args[1]->getExprLoc(), diag::err_cm_iselect_invalid_index_num) << 0
      << 1 << SourceRange(Args[1]->getSourceRange().getBegin(),
                          Args.back()->getSourceRange().getEnd());
    return ExprError();
  }
  if (!IsVector && Args.size() < 2) {
    Diag(RPLoc, diag::err_cm_iselect_invalid_index_num) << 1 << 2;
    return ExprError();
  }
  if (!IsVector && Args.size() > 2) {
    Diag(Args[2]->getExprLoc(), diag::err_cm_iselect_invalid_index_num) << 1
      << 2 << SourceRange(Args[2]->getSourceRange().getBegin(),
                          Args.back()->getSourceRange().getEnd());
    return ExprError();
  }

  // Check the argument type, must be of type vector<T, N> and T is integral.
  for (unsigned i = 0, e = Args.size(); i < e; ++i) {
    QualType T = Args[i]->getType();
    if (T->isDependentType())
      continue;

    if (!T->isCMVectorType()) {
      Diag(Args[i]->getExprLoc(), diag::err_cm_iselect_invalid_index_type)
          << Args[i]->getType();
      return ExprError();
    }

    T = T->getCMVectorMatrixElementType();
    if (!T->isIntegralType(Context)) {
      Diag(Args[i]->getExprLoc(), diag::err_cm_iselect_invalid_index_type)
          << Args[i]->getType();
      return ExprError();
    }
  }

  SmallVector<Expr *, 2> ArgExprs;
  for (unsigned i = 0, e = Args.size(); i < e; ++i) {
    ExprResult Res = DefaultLvalueConversion(Args[i]);
    if (!Res.isUsable())
      return ExprError();

    if (!Res.get()->getType()->isDependentType()) {
      // Convert to unsigned short if necessary.
      const CMVectorType *VT = Res.get()->getType()->getAs<CMVectorType>();
      QualType EltTy = VT->getCMVectorMatrixElementType();
      EltTy = EltTy->getCanonicalTypeUnqualified();
      if (EltTy != Context.UnsignedShortTy) {
        QualType IndexTy = Context.getCMVectorType(
            /*isRef*/ false, Context.UnsignedShortTy, VT->getNumElements(),
            VT->getVMLoc(), VT->getVMLoc(), VT->getVMLoc());
        Res = ImpCastExprToType(Res.get(), IndexTy, CK_IntegralCast);
        if (!Res.isUsable())
          return ExprError();
      }
    }

    ArgExprs.push_back(Res.get());
  }

  // Extract the output vector size.
  unsigned NumElts = 0;
  QualType AT = Args[0]->getType();
  if (!AT->isDependentType()) {
    NumElts = AT->getCMVectorMatrixSize();
    // For matrix iselect, check vector type mismatch
    if (!IsVector && !Args[1]->isTypeDependent() &&
        (NumElts != Args[1]->getType()->getCMVectorMatrixSize())) {
      unsigned NumCols = Args[1]->getType()->getCMVectorMatrixSize();
      Diag(ISelectLoc, diag::err_cm_iselect_size_mismatch)
          << NumElts << NumCols << Args[0]->getExprLoc()
          << Args[1]->getExprLoc();
      return ExprError();
    }
  }

  QualType ExprTy;
  if (NumElts != 0)
    ExprTy = Context.getCMVectorType(
        /*isRef*/ false, Base->getType()->getCMVectorMatrixElementType(),
        NumElts, ISelectLoc, ISelectLoc, ISelectLoc);
  else
    ExprTy = Context.getDependentCMVectorType(
        /*isRef*/ false, Base->getType()->getCMVectorMatrixElementType(),
        /*SizeExpr*/ 0, ISelectLoc, ISelectLoc, ISelectLoc);

  return new (Context)
      CMSelectExpr(Context, CMSelectExpr::SK_iselect, Base, ISelectLoc,
                   ArgExprs, RPLoc, ExprTy, VK_RValue);
}

// \brief Semantic actions for CM vector/matrix merge() member function
ExprResult Sema::ActOnCMMerge(SourceLocation MergeLoc, Expr *Base,
                              MultiExprArg Args, SourceLocation RParenLoc) {
  assert(Base && "null base");
  unsigned NumArgs = Args.size();
  if (NumArgs < 2) {
    Diag(RParenLoc, diag::err_cm_merge_wrong_number_args) << 0;
    return ExprError();
  }
  if (NumArgs > 3) {
    Diag(Args[3]->getExprLoc(), diag::err_cm_merge_wrong_number_args)
        << 1 << SourceRange(Args[3]->getSourceRange().getBegin(),
                            Args[NumArgs - 1]->getSourceRange().getEnd());
    return ExprError();
  }

  return BuildCMMerge(Base, MergeLoc, Args, RParenLoc);
}

ExprResult Sema::BuildCMMerge(Expr *Base, SourceLocation MergeLoc,
                              MultiExprArg Args, SourceLocation RParenLoc) {
  if (Base->isTypeDependent())
    // the result is void
    return new (Context) CMMergeExpr(Context, Base, MergeLoc, Args, RParenLoc,
                                     Context.VoidTy, VK_RValue);

  QualType BaseTy = Base->getType();
  if (!BaseTy->isCMVectorMatrixType()) {
    Diag(Base->getExprLoc(), diag::err_cm_vector_matrix_type_expected)
        << "merge";
    return ExprError();
  }
  if (!CheckCMArithmeticType(Base->getExprLoc(), BaseTy))
    return ExprError();

  for (unsigned I = 0, N = Args.size(); I < N; ++I) {
    Expr *Arg = Args[I];
    QualType ArgTy = Arg->getType();
    if (ArgTy->isDependentType())
      continue;

    if (I != N - 1) {
      // Check the source argument.
      ExprResult Res = CheckCMMergeOperand(Base->getExprLoc(), BaseTy, Arg);
      if (Res.isUsable())
        Args[I] = Res.get();
      else
        return ExprError();
    } else {
      // Check the mask argument.
      if (ArgTy->isCMVectorMatrixType()) {
        // mask is VM, check its size and element type
        if (BaseTy->getCMVectorMatrixSize() != ArgTy->getCMVectorMatrixSize()) {
          Diag(Arg->getExprLoc(), diag::err_cm_merge_invalid_mask)
              << ArgTy << BaseTy << Base->getSourceRange();
          return ExprError();
        }
        QualType ArgEltTy = ArgTy->getCMVectorMatrixElementType();
        if (!ArgEltTy->isIntegerType()) {
          Diag(Arg->getExprLoc(), diag::err_cm_merge_invalid_mask_element)
              << ArgEltTy << Arg->getSourceRange();
          return ExprError();
        }
      } else {
        // mask is scalar, check its type.
        if (!ArgTy->isIntegralOrEnumerationType()) {
          Diag(Arg->getExprLoc(), diag::err_cm_merge_invalid_mask)
              << ArgTy << BaseTy << Arg->getSourceRange();
          return ExprError();
        }
        // warn if there are fewer bits than the number of elements
        unsigned numMaskBits = Context.getTypeSize(ArgTy);
        unsigned numElements = BaseTy->getCMVectorMatrixSize();
        if (numMaskBits < numElements) {
          Diag(Arg->getExprLoc(), diag::warn_cm_merge_mask_insufficient)
              << numMaskBits << numElements << Base->getSourceRange();
        }
        ExprResult Res = DefaultLvalueConversion(Arg);
        Res = ImpCastExprToType(Res.get(), Context.IntTy, CK_IntegralCast);
        if (!Res.isUsable())
          return ExprError();
        else
          Args[I] = Res.get();
      }
    }
  }

  // the result is void
  return new (Context) CMMergeExpr(Context, Base, MergeLoc, Args, RParenLoc,
                                   Context.VoidTy, VK_RValue);
}

bool Sema::CheckCMArithmeticType(SourceLocation Loc, QualType Ty) {
  if (Ty->isDependentType())
    return true;

  assert(Ty->isCMVectorMatrixType());
  QualType EltTy = Ty->getCMVectorMatrixElementType();
  if (!EltTy->isCMElementType(/* AllowNonArithmetic */ false)) {
    Diag(Loc, diag::err_cm_vector_matrix_invalid_element_type) << EltTy;
    return false;
  }

  return true;
}

/// \brief Check whether source expression is valid as the merge source and
/// perform necessary analysis.
ExprResult Sema::CheckCMMergeOperand(SourceLocation MergeLoc, QualType DstType,
                                     ExprResult Src) {
  assert(Src.isUsable());
  if (DstType->isDependentType() || Src.get()->isTypeDependent())
    return Src;

  Src = DefaultLvalueConversion(Src.get());
  if (!Src.isUsable())
    return Src;

  // For conversion purposes, we ignore any qualifiers.
  DstType = Context.getCanonicalType(DstType).getUnqualifiedType();
  QualType SrcType =
      Context.getCanonicalType(Src.get()->getType()).getUnqualifiedType();

  // If the types are identical, return.
  if (DstType == SrcType)
    return Src;

  QualType DstEltTy = DstType->getCMVectorMatrixElementType();
  QualType DstTypeNonRef = Context.getCMVectorMatrixBaseType(DstType);

  // Handle the case of a vector/matrix and scalar.
  if (SrcType->isScalarType()) {
    if (DstEltTy->isIntegralType(Context) &&
        SrcType->isIntegralOrEnumerationType()) {
      Src = ImpCastExprToType(Src.get(), DstEltTy, CK_IntegralCast);
      return ImpCastExprToType(Src.get(), DstTypeNonRef, CK_CMVectorMatrixSplat);
    }
    if (DstEltTy->isIntegralType(Context) && SrcType->isRealFloatingType()) {
      Src = ImpCastExprToType(Src.get(), DstEltTy, CK_FloatingToIntegral);
      return ImpCastExprToType(Src.get(), DstTypeNonRef, CK_CMVectorMatrixSplat);
    }
    if (DstEltTy->isRealFloatingType() &&
        SrcType->isIntegralOrEnumerationType()) {
      Src = ImpCastExprToType(Src.get(), DstEltTy, CK_IntegralToFloating);
      return ImpCastExprToType(Src.get(), DstTypeNonRef, CK_CMVectorMatrixSplat);
    }
    if (DstEltTy->isRealFloatingType() && SrcType->isRealFloatingType()) {
      Src = ImpCastExprToType(Src.get(), DstEltTy, CK_FloatingCast);
      return ImpCastExprToType(Src.get(), DstTypeNonRef, CK_CMVectorMatrixSplat);
    }

    // No conversion from src to dst.
    Diag(MergeLoc, diag::err_cm_merge_invalid_source)
        << DstType << SrcType << Src.get()->getSourceRange();
    return ExprError();
  }

  if (!SrcType->isCMVectorMatrixType()) {
    Diag(MergeLoc, diag::err_cm_merge_invalid_source)
        << DstType << SrcType << Src.get()->getSourceRange();
    return ExprError();
  }

  unsigned DstNumElts = DstType->getCMVectorMatrixSize();
  unsigned SrcNumElts = SrcType->getCMVectorMatrixSize();
  QualType SrcEltTy = SrcType->getCMVectorMatrixElementType();

  // The same number of elements.
  if (SrcNumElts != DstNumElts) {
    Diag(MergeLoc, diag::err_cm_merge_invalid_source)
        << DstType << SrcType << Src.get()->getSourceRange();
    return ExprError();
  }

  // Handle the cases that src is also vector / matrix.
  if (DstEltTy->isIntegralType(Context) && SrcEltTy->isIntegralType(Context)) {
    if (Context.getIntegerTypeOrder(DstEltTy, SrcEltTy) != 0)
      return ImpCastExprToType(Src.get(), DstTypeNonRef, CK_IntegralCast);
    else
      return Src;
  }

  if (DstEltTy->isIntegralType(Context) && SrcEltTy->isRealFloatingType())
    return ImpCastExprToType(Src.get(), DstTypeNonRef, CK_FloatingToIntegral);

  if (DstEltTy->isRealFloatingType() && SrcEltTy->isIntegralType(Context))
    return ImpCastExprToType(Src.get(), DstTypeNonRef, CK_IntegralToFloating);

  if (DstEltTy->isRealFloatingType() && SrcEltTy->isRealFloatingType()) {
    if (Context.getFloatingTypeOrder(DstEltTy, SrcEltTy) != 0)
      return ImpCastExprToType(Src.get(), DstTypeNonRef, CK_FloatingCast);
    else
      return Src;
  }

  Diag(MergeLoc, diag::err_cm_merge_invalid_source)
      << DstType << SrcType << Src.get()->getSourceRange();
  return ExprError();
}

// \brief Semantic actions for CM matrix n_cols() member function
ExprResult Sema::ActOnCMNCols(SourceLocation NColsLoc, Expr *Base,
                              SourceLocation RParenLoc) {
  assert(Base && "null base");
  // Delay sema if base is dependent.
  if (Base->isInstantiationDependent())
    return new (Context)
        CMSizeExpr(Context, CMSizeExpr::SK_n_cols, Base, NColsLoc, RParenLoc);

  if (!Base->getType()->isCMMatrixType()) {
    Diag(NColsLoc, diag::err_cm_matrix_only_method) << 1
                                                    << Base->getSourceRange();
    return ExprError();
  }

  const CMMatrixType *MT = Base->getType()->getAs<CMMatrixType>();
  unsigned Width = Context.getIntWidth(Context.UnsignedIntTy);
  llvm::APInt NumCols(Width, MT->getNumColumns(), true);
  // This is a bit lossy since the source information is lost. But this is
  // simpler since no codegen is needed any more.
  return IntegerLiteral::Create(Context, NumCols, Context.UnsignedIntTy, NColsLoc);
}

// \brief Semantic actions for CM matrix n_cols() member function
ExprResult Sema::ActOnCMNRows(SourceLocation NRowsLoc, Expr *Base,
  SourceLocation RParenLoc) {
  assert(Base && "null base");
  // Delay sema if base is dependent.
  if (Base->isInstantiationDependent())
    return new (Context)
        CMSizeExpr(Context, CMSizeExpr::SK_n_rows, Base, NRowsLoc, RParenLoc);

  if (!Base->getType()->isCMMatrixType()) {
    Diag(NRowsLoc, diag::err_cm_matrix_only_method) << 2
      << Base->getSourceRange();
    return ExprError();
  }

  const CMMatrixType *MT = Base->getType()->getAs<CMMatrixType>();
  unsigned Width = Context.getIntWidth(Context.UnsignedIntTy);
  llvm::APInt NumRows(Width, MT->getNumRows(), true);
  // This is a bit lossy since the source information is lost. But this is
  // simpler since no codegen is needed any more.
  return IntegerLiteral::Create(Context, NumRows, Context.UnsignedIntTy, NRowsLoc);
}

// \brief Semantic actions for CM vector/matrix n_elems() member function
ExprResult Sema::ActOnCMNElems(SourceLocation NElemsLoc, Expr *Base,
                               SourceLocation RParenLoc) {
  assert(Base && "null base");
  // Delay sema if base is dependent.
  if (Base->isInstantiationDependent())
    return new (Context)
        CMSizeExpr(Context, CMSizeExpr::SK_n_elems, Base, NElemsLoc, RParenLoc);

  unsigned Width = Context.getIntWidth(Context.UnsignedIntTy);
  unsigned NumElements = Base->getType()->getCMVectorMatrixSize();
  llvm::APInt NumElems(Width, NumElements, true);
  // This is a bit lossy since the source information is lost. But this is
  // simpler since no codegen is needed any more.
  return IntegerLiteral::Create(Context, NumElems, Context.UnsignedIntTy, NElemsLoc);
}

// Returns true if this expression can be evaluated as a constant integer.
static bool EvaluateAsConstantInt(int64_t &Result, Sema &S, Expr *E,
                                  unsigned DiagID, unsigned Index) {
  if (E->isValueDependent() || E->isTypeDependent())
    return false;

  llvm::APSInt ConstVal;
  SourceLocation Loc;
  if (!E->isIntegerConstantExpr(ConstVal, S.Context, &Loc)) {
    S.Diag(Loc, DiagID) << Index << E->getSourceRange();
    return false;
  }

  Result = ConstVal.getSExtValue();
  return true;
}

// \brief Semantic actions for CM replicate member function
//
ExprResult Sema::ActOnCMReplicate(SourceLocation ReplicateLoc, Expr *Base,
                                  MultiExprArg ReplicateArgs,
                                  MultiExprArg Offsets,
                                  SourceLocation RParenLoc) {
  assert(Base && "null base");
  QualType BaseTy = Base->getType();

  bool IsDependent = BaseTy->isDependentType();
  for (unsigned I = 0, N = ReplicateArgs.size(); I < N; ++I)
    IsDependent |= ReplicateArgs[I]->isTypeDependent() ||
                   ReplicateArgs[I]->isValueDependent();

  if (IsDependent) {
    QualType ExprTy = Context.DependentTy;
    if (BaseTy->isCMVectorMatrixType())
      ExprTy = Context.getDependentCMVectorType(
          /*IsRef*/ false, BaseTy->getCMVectorMatrixElementType(),
          /*SizeExpr*/ 0, ReplicateLoc, ReplicateLoc, ReplicateLoc);

    SmallVector<Expr *, 8> ArgExprs;
    ArgExprs.append(ReplicateArgs.begin(), ReplicateArgs.end());
    ArgExprs.append(Offsets.begin(), Offsets.end());

    return new (Context) CMSelectExpr(
        Context, CMSelectExpr::SK_replicate, Base, ReplicateLoc, ArgExprs,
        ReplicateArgs.size(), RParenLoc, ExprTy, VK_RValue);
  }

  unsigned NumArgs = ReplicateArgs.size();
  unsigned NumOffsets = Offsets.size();

  if (NumArgs > 4) {
    Diag(ReplicateArgs[4]->getExprLoc(), diag::err_cm_replicate_too_many_args)
        << SourceRange(ReplicateArgs[4]->getSourceRange().getBegin(),
                       ReplicateArgs[NumArgs - 1]->getSourceRange().getEnd());
    return ExprError();
  }

  SmallVector<Expr *, 8> ArgExprs;
  // Values REP, VS, W, HS
  //        REP, VS, W
  //        REP, W
  //        REP
  int64_t Vals[4] = {-1, -1, -1, -1};
  for (unsigned I = 0; I < NumArgs; ++I) {
    if (EvaluateAsConstantInt(Vals[I], *this, ReplicateArgs[I],
                              diag::err_cm_replicate_non_const_value, I)) {
      if (Vals[I] < 0) {
        Diag(ReplicateArgs[I]->getExprLoc(), diag::err_cm_replicate_arg_negative)
          << (((NumArgs == 2) && (I == 1)) ? 2 : I)
          << (int)Vals[I] << ReplicateArgs[I]->getSourceRange();
        return ExprError();
      }
      ExprResult Result = DefaultLvalueConversion(ReplicateArgs[I]);;
      ArgExprs.push_back(Result.get());
    } else
      return ExprError();
  }
  // Check REP value is not zero
  if (Vals[0] == 0) {
    Diag(ReplicateArgs[0]->getExprLoc(), diag::err_cm_replicate_arg_zero) << 0
      << ReplicateArgs[0]->getSourceRange();
    return ExprError();
  }
  // Check width value is not zero (if specified)
  int WidthArg = (NumArgs > 2) ? 2 : (NumArgs == 2) ? 1 : 0;
  if (WidthArg && (Vals[WidthArg] == 0)) {
    Diag(ReplicateArgs[WidthArg]->getExprLoc(), diag::err_cm_replicate_arg_zero) << 1
      << ReplicateArgs[WidthArg]->getSourceRange();
    return ExprError();
  }
  // We don't expect any offset if only REP is specified (in that case we
  // replicate the whole source object, so an offset is not meaningful).
  if ((NumArgs == 1) && NumOffsets) {
    Diag(Offsets[0]->getExprLoc(), diag::err_cm_replicate_unexpected_offset)
      << SourceRange(Offsets[0]->getSourceRange().getBegin(),
                     Offsets[NumOffsets-1]->getSourceRange().getEnd());
    return ExprError();
  }
  // Maximum of one offset for a vector base value
  if (BaseTy->isCMVectorType() && NumOffsets > 1) {
    Diag(Offsets[1]->getExprLoc(), diag::err_cm_replicate_too_many_offsets)
        << 0 << 1 << SourceRange(Offsets[1]->getSourceRange().getBegin(),
                            Offsets[NumOffsets - 1]->getSourceRange().getEnd());
    return ExprError();
  }
  // Maximum of two offsets for a matrix base value
  if (BaseTy->isCMMatrixType() && NumOffsets > 2) {
    Diag(Offsets[2]->getExprLoc(), diag::err_cm_replicate_too_many_offsets)
        << 1 << 2 << SourceRange(Offsets[2]->getSourceRange().getBegin(),
                            Offsets[NumOffsets - 1]->getSourceRange().getEnd());
    return ExprError();
  }

  // Check offsets.
  for (unsigned I = 0; I < NumOffsets; ++I) {
    if (!Offsets[I]->isTypeDependent()) {
      if (!Offsets[I]->getType()->isIntegralOrEnumerationType()) {
        Diag(Offsets[I]->getExprLoc(),
             diag::err_cm_replicate_non_integer_offset)
          << Offsets[I]->getSourceRange();
        return ExprError();
      }

      ExprResult Result = DefaultLvalueConversion(Offsets[I]);
      Result = ImpCastExprToType(Result.get(), Context.UnsignedShortTy,
                                 CK_IntegralCast);
      assert(Result.isUsable());
      ArgExprs.push_back(Result.get());
    } else
      ArgExprs.push_back(Offsets[I]);
  }

  // Fill default offsets, only when NumArgs > 1.
  if (NumArgs > 1) {
    unsigned NumExpectedOffsets = BaseTy->isCMVectorType() ? 1 : 2;
    for (unsigned I = NumOffsets; I < NumExpectedOffsets; ++I) {
      // If there is no argument, then fill 0 of type ushort.
      llvm::APInt Val(16, 0, /*isSigned*/ false);
      IntegerLiteral *Zero = IntegerLiteral::Create(
          Context, Val, Context.UnsignedShortTy, RParenLoc);
      ArgExprs.push_back(Zero);
    }
  }

  // Check for the most likely out-of-bounds conditions that are detectable at
  // compile time.
  if (WidthArg) {
    unsigned NumElts = BaseTy->getCMVectorMatrixSize();
    unsigned VS = 0;
    unsigned HS = 1;
    unsigned ElmtOffset = 0;
    unsigned RowSize = 0;
    if (const CMMatrixType *M = BaseTy->getAs<CMMatrixType>())
      RowSize = M->getNumColumns();
    bool BaseTypeIsMatrix = BaseTy->isCMMatrixType();
    // Get any constant offsets that may contribute to the maximum element index
    int64_t I = 0;
    int64_t J = 0;
    if (NumOffsets >= 1) {
      llvm::APSInt OffsetValue;
      if (Offsets[0]->EvaluateAsInt(OffsetValue, Context))
        I = OffsetValue.getSExtValue();
      if (I < 0) {
        Diag(Offsets[0]->getExprLoc(), diag::err_cm_replicate_offset_negative)
          << (int)I;
        return ExprError();
      }
      ElmtOffset = I;
    }
    if (NumOffsets == 2) {
      llvm::APSInt OffsetValue;
      if (Offsets[1]->EvaluateAsInt(OffsetValue, Context))
        J = OffsetValue.getSExtValue();
      if (J < 0) {
        Diag(Offsets[1]->getExprLoc(), diag::err_cm_replicate_offset_negative)
          << (int)J;
        return ExprError();
      }
      ElmtOffset = I * RowSize + J;
    }
    if (NumArgs > 2)
      VS = Vals[1];
    if (NumArgs > 3)
      HS = Vals[3];
    if (ElmtOffset + (VS * (Vals[0] - 1)) + (HS > 0? ((Vals[WidthArg]-1) * HS + 1) : 1) > NumElts) {
      // We highlight the components that contribute to the out of bounds value.
      // We use a single diagnostic where any fields that aren't present or are
      // zero have a default null value so aren't highlighted.
      SourceRange VSRange;
      SourceRange HSRange;
      SourceRange OffsetRange[2];
      if ((NumArgs > 2) && (VS > 0))
        VSRange = ReplicateArgs[1]->getSourceRange();
      if ((NumArgs > 3) && (HS > 0))
        HSRange = ReplicateArgs[3]->getSourceRange();
      if (I > 0)
        OffsetRange[0] = Offsets[0]->getSourceRange();
      if (J > 0)
        OffsetRange[1] = Offsets[0]->getSourceRange();
      Diag(ReplicateLoc, diag::warn_cm_replicate_out_of_bounds)
          << BaseTypeIsMatrix
          << ReplicateArgs[0]->getSourceRange()
          << ReplicateArgs[1]->getSourceRange()
          << ReplicateArgs[WidthArg]->getSourceRange()
          << HSRange
          << OffsetRange[0]
          << OffsetRange[1];
    }
  }

  // The result expression is a vector, and its size depends on replicate number
  // and other arguments.
  QualType EltTy = BaseTy->getCMVectorMatrixElementType();
  QualType ExprTy;

  if (NumArgs == 1) {
    // replicate<REP>()
    unsigned NumElts = BaseTy->getCMVectorMatrixSize();
    ExprTy = Context.getCMVectorType(false, EltTy, NumElts * Vals[0],
                                     ReplicateLoc, ReplicateLoc, ReplicateLoc);
  } else if (NumArgs == 2) {
    // replicate<REP, W>(ushort i = 0)
    // replicate<REP, W>(ushort i = 0, ushort j = 0)
    ExprTy = Context.getCMVectorType(false, EltTy, Vals[0] * Vals[1],
                                     ReplicateLoc, ReplicateLoc, ReplicateLoc);
  } else {
    // replicate<REP, VS, W>(ushort i = 0)
    // replicate<REP, VS, W>(ushort i = 0, ushort j = 0)
    // replicate<REP, VS, W, HS>(ushort i = 0)
    // replicate<REP, VS, W, HS>(ushort i = 0, ushort j = 0)
    ExprTy = Context.getCMVectorType(false, EltTy, Vals[0] * Vals[2],
                                     ReplicateLoc, ReplicateLoc, ReplicateLoc);
  }

  return new (Context) CMSelectExpr(
      Context, CMSelectExpr::SK_replicate, Base, ReplicateLoc, ArgExprs,
      ReplicateArgs.size(), RParenLoc, ExprTy, VK_RValue);
}

// \brief Semantic actions for CM matrix row() member function
ExprResult Sema::ActOnCMRow(SourceLocation RowLoc, Expr *Base, Expr *RowExpr,
                            SourceLocation RParenLoc) {
  assert(Base && "null base expression");
  QualType BaseTy = Base->getType();

  // defer the analysis if the base is dependent.
  if (Base->isTypeDependent()) {
    QualType ExprTy = Context.DependentTy;
    if (BaseTy->isCMVectorMatrixType())
      ExprTy = Context.getDependentCMVectorType(
          /*IsRef*/ true, BaseTy->getCMVectorMatrixElementType(),
          /*SizeExpr*/ 0, RowLoc, RowLoc, RowLoc);

    return new (Context)
        CMSelectExpr(Context, CMSelectExpr::SK_row, Base, RowLoc, RowExpr,
                     RParenLoc, ExprTy, VK_LValue);
  }

  if (!Base->getType()->isCMMatrixType()) {
    Diag(RowLoc, diag::err_cm_matrix_only_method) << 3
                                                  << Base->getSourceRange();
    return ExprError();
  }

  if (!RowExpr->getType()->isDependentType() &&
      !RowExpr->getType()->isIntegralOrEnumerationType()) {
    Diag(RowExpr->getExprLoc(), diag::err_cm_non_integer_index) << 0;
    return ExprError();
  }

  const CMMatrixType *MT = BaseTy->getAs<CMMatrixType>();

  // Check whether index is out of bound.
  {
    unsigned NRows = MT->getNumRows();
    llvm::APSInt Val;
    SourceLocation Loc;
    if (!RowExpr->isValueDependent() &&
        RowExpr->isIntegerConstantExpr(Val, Context, &Loc)) {
      int64_t Index = Val.getSExtValue();
      if (Index < 0) {
        Diag(RowExpr->getExprLoc(), diag::err_cm_negative_index) << 0;
        return ExprError();
      } else if (Index >= NRows) {
        Diag(RowExpr->getExprLoc(), diag::warn_cm_out_of_bound_index) << 0
          << (unsigned)Index << NRows;
      }
    }
  }

  // The row expr type is a vector reference type of its width.
  QualType ExprTy = Context.getCMVectorType(
      /*IsReference*/ true, MT->getElementType(), MT->getNumColumns(),
      MT->getVMLoc(), MT->getLessLoc(), MT->getGreaterLoc());

  ExprResult Res = DefaultLvalueConversion(RowExpr);
  Res = ImpCastExprToType(Res.get(), Context.UnsignedShortTy, CK_IntegralCast);
  if (!Res.isUsable())
    return ExprError();

  return new (Context) CMSelectExpr(Context, CMSelectExpr::SK_row, Base, RowLoc,
                                    Res.get(), RParenLoc, ExprTy, VK_LValue);
}

// \brief Semantic actions for CM vector select member function
//
// vector select should have two integer ConstArgs and one integer Arg
//   select<size,stride>(offset)
//   - size must not exceed the source size
//   - stride must not be 0
//
// matrix select should have four integer ConstArgs and two integer Args
//   select<v_size,v_stride,h_size,h_stride>(v_offset, h_offset)
//   - v_size must not exceed the source row-size
//   - v_stride must not be 0
//   - h_size must not exceed the source column-size
//   - h_stride must not be 0
//
ExprResult Sema::ActOnCMSelect(SourceLocation SelectLoc, Expr *Base,
                               MultiExprArg ConstArgs, MultiExprArg Args,
                               SourceLocation RParenLoc) {
  assert(Base && "null base");
  bool IsDependent = Base->isTypeDependent();
  for (unsigned I = 0, N = ConstArgs.size(); I < N; ++I)
    IsDependent |= ConstArgs[I]->isInstantiationDependent();

  QualType BaseTy = Base->getType();
  SmallVector<Expr *, 8> ArgExprs;
  if (IsDependent) {
    ArgExprs.append(ConstArgs.begin(), ConstArgs.end());
    ArgExprs.append(Args.begin(), Args.end());

    QualType ExprTy = Context.DependentTy;
    if (BaseTy->isCMVectorType())
      ExprTy = Context.getDependentCMVectorType(
          /*IsRef*/ true, BaseTy->getCMVectorMatrixElementType(),
          /*SizeExpr*/ 0, SelectLoc, SelectLoc, SelectLoc);
    else if (BaseTy->isCMMatrixType())
      ExprTy = Context.getDependentCMMatrixType(
          /*IsRef*/ true, BaseTy->getCMVectorMatrixElementType(),
          /*NRowExpr*/ 0, /*NColExpr*/ 0, SelectLoc, SelectLoc, SelectLoc);

    return new (Context) CMSelectExpr(
        Context, CMSelectExpr::SK_select, Base, SelectLoc, ArgExprs,
        ConstArgs.size(), RParenLoc, ExprTy, VK_LValue);
  }

  // Check the base type.
  bool IsMatrix = BaseTy->isCMMatrixType();
  if (!CheckCMArithmeticType(SelectLoc, BaseTy))
    return ExprError();

  // Check the number of constant arguments.
  unsigned ExpectedConstArgs = (IsMatrix ? 4u : 2u);
  unsigned NumConstArgs = ConstArgs.size();
  if (NumConstArgs < ExpectedConstArgs) {
    Diag(ConstArgs.back()->getSourceRange().getEnd(),
        diag::err_cm_select_wrong_number_values) << 0 << IsMatrix;
    return ExprError();
  }

  if (ConstArgs.size() > ExpectedConstArgs) {
    Diag(ConstArgs[ExpectedConstArgs]->getSourceRange().getBegin(),
         diag::err_cm_select_wrong_number_values)
        << 1 << IsMatrix
        << SourceRange(
               ConstArgs[ExpectedConstArgs]->getSourceRange().getBegin(),
               ConstArgs[NumConstArgs - 1]->getSourceRange().getEnd());
    return ExprError();
  }

  // Check the constant arguments.
  unsigned Size = 0; // VSize for matrix case.
  unsigned Stride = 0; // VStride for matrix case
  unsigned HSize = 0;
  unsigned HStride = 0;

  for (unsigned I = 0, N = ConstArgs.size(); I < N; ++I) {
    llvm::APSInt ConstVal;
    SourceLocation Loc;
    if (!ConstArgs[I]->isIntegerConstantExpr(ConstVal, Context, &Loc)) {
      Diag(Loc, diag::err_cm_select_non_const_value) << I;
      return ExprError();
    }

    // Unify the constant argument type.
    ExprResult Res = DefaultLvalueConversion(ConstArgs[I]);
    Res = ImpCastExprToType(Res.get(), Context.IntTy, CK_IntegralCast);
    if (Res.isUsable())
      ArgExprs.push_back(Res.get());
    else
      return ExprError();

    // Check static constraints.
    if (I == 0) {
      // size or v_size
      Size = ConstVal.getZExtValue();
      if (ConstVal.getSExtValue() < 1) {
        Diag(ConstArgs[I]->getExprLoc(), diag::err_cm_select_zero_size)
            << IsMatrix;
        return ExprError();
      }
    } else if (I == 1) {
      // stride or v_stride
      Stride = ConstVal.getZExtValue();
      if ((Size == 1) && (ConstVal.getZExtValue() != 1)) {
        Diag(ConstArgs[I]->getExprLoc(), diag::err_cm_select_stride_must_be_one)
            << IsMatrix << ConstArgs[0]->getSourceRange();
        return ExprError();
      }
      if (ConstVal.getSExtValue() < 1) {
        Diag(ConstArgs[I]->getExprLoc(), diag::err_cm_select_zero_stride)
            << IsMatrix;
        return ExprError();
      }
    } else if (I == 2) {
      // h_size
      HSize = ConstVal.getZExtValue();
      if (ConstVal.getSExtValue() < 1) {
        Diag(ConstArgs[I]->getExprLoc(), diag::err_cm_select_zero_size) << 2;
        return ExprError();
      }
    } else if (I == 3) {
      // h_stride
      HStride = ConstVal.getZExtValue();
      if ((HSize == 1) && (ConstVal.getZExtValue() != 1)) {
        Diag(ConstArgs[I]->getExprLoc(), diag::err_cm_select_stride_must_be_one)
            << 2 << ConstArgs[2]->getSourceRange();
        return ExprError();
      }
      if (ConstVal.getSExtValue() < 1) {
        Diag(ConstArgs[I]->getExprLoc(), diag::err_cm_select_zero_stride) << 2;
        return ExprError();
      }
    }
  }

  // Check offset arguments.
  for (unsigned I = 0, N = Args.size(); I < N; ++I) {
    if (Args[I]->isTypeDependent()) {
      ArgExprs.push_back(Args[I]);
      continue;
    }

    if (!Args[I]->getType()->isIntegralOrEnumerationType()) {
      Diag(Args[I]->getExprLoc(), diag::err_cm_select_non_integer_offset);
      return ExprError();
    }
    ExprResult Res = DefaultLvalueConversion(Args[I]);
    Res = ImpCastExprToType(Res.get(), Context.UnsignedShortTy, CK_IntegralCast);
    ArgExprs.push_back(Res.get());
  }

  // If there is less than expected number of arguments, fill 0 of type ushort.
  unsigned maxOffsets = (IsMatrix ? 2u : 1u);
  for (unsigned I = Args.size(); I < maxOffsets; ++I) {
    llvm::APInt Val(16, 0, /*isSigned*/ false);
    IntegerLiteral *Zero = IntegerLiteral::Create(
        Context, Val, Context.UnsignedShortTy, RParenLoc);
    ArgExprs.push_back(Zero);
  }

  // Generate error if too many offsets supplied
  if (Args.size() > maxOffsets) {
    Diag(Args[maxOffsets]->getExprLoc(), diag::err_cm_select_wrong_number_offsets)
      << 1 << IsMatrix
      << SourceRange(Args[maxOffsets]->getSourceRange().getBegin(),
                     Args.back()->getSourceRange().getEnd());
    return ExprError();
  }

  // Out-of-bound checks.
  if (IsMatrix) {
    assert(Size > 0 && Stride > 0 && HSize > 0 && HStride > 0);
    const CMMatrixType *MT = BaseTy->getAs<CMMatrixType>();
    unsigned NRows = MT->getNumRows();
    unsigned NCols = MT->getNumColumns();

    unsigned VOffset = 0;
    int VOffsetSigned = 0;
    llvm::APSInt Val;
    if (Args.size() >= 1 && Args[0]->isIntegerConstantExpr(Val, Context)) {
      VOffset = Val.getZExtValue();
      VOffsetSigned = Val.getSExtValue();
    }

    // negative VOffset
    if (VOffsetSigned < 0) {
      Diag(Args[0]->getExprLoc(), diag::err_cm_select_index_negative)
        << 1 << VOffsetSigned << Args[0]->getSourceRange();
      return ExprError();
    }

    // degenerate case of VOffset >= NRows
    if (VOffset >= NRows) {
      Diag(SelectLoc, diag::warn_cm_select_out_of_bounds_offset)
          << 1 << VOffset << (NRows - 1)
          << Args[0]->getSourceRange();
    }

    if ((Size - 1) * Stride + VOffset >= NRows) {
      // highlight VOffset if it is non-zero as it contributes to the out of
      // bounds value
      if (VOffset)
        Diag(SelectLoc, diag::warn_cm_select_out_of_bounds)
            << 1 << 1 << ConstArgs[0]->getSourceRange()
            << ConstArgs[1]->getSourceRange()
            << Args[0]->getSourceRange();
      else
        Diag(SelectLoc, diag::warn_cm_select_out_of_bounds)
            << 1 << 1 << ConstArgs[0]->getSourceRange()
            << ConstArgs[1]->getSourceRange();
    }

    unsigned HOffset = 0;
    int HOffsetSigned = 0;
    if (Args.size() >= 2 && Args[1]->isIntegerConstantExpr(Val, Context)) {
      HOffset = Val.getZExtValue();
      HOffsetSigned = Val.getSExtValue();
    }

    // negative HOffset
    if (HOffsetSigned < 0) {
      Diag(Args[1]->getExprLoc(), diag::err_cm_select_index_negative)
        << 2 << HOffsetSigned << Args[1]->getSourceRange();
      return ExprError();
    }

    // degenerate case of HOffset >= NCols
    if (HOffset >= NCols) {
      Diag(SelectLoc, diag::warn_cm_select_out_of_bounds_offset)
          << 2 << HOffset << (NCols - 1)
          << Args[1]->getSourceRange();
    }

    if ((HSize - 1) * HStride + HOffset >= NCols) {
      // highlight HOffset if it is non-zero as it contributes to the out of
      // bounds value
      if (HOffset)
        Diag(SelectLoc, diag::warn_cm_select_out_of_bounds)
            << 1 << 2 << ConstArgs[2]->getSourceRange()
            << ConstArgs[3]->getSourceRange()
            << Args[1]->getSourceRange();
      else
        Diag(SelectLoc, diag::warn_cm_select_out_of_bounds)
            << 1 << 2 << ConstArgs[2]->getSourceRange()
            << ConstArgs[3]->getSourceRange();
    }
  } else {
    assert(Size > 0 && Stride > 0);
    const CMVectorType *VT = BaseTy->getAs<CMVectorType>();
    unsigned NumElts = VT->getNumElements();

    unsigned Offset = 0;
    int OffsetSigned = 0;
    llvm::APSInt Val;
    if (Args.size() >= 1 && Args[0]->isIntegerConstantExpr(Val, Context)) {
      Offset = Val.getZExtValue();
      OffsetSigned = Val.getSExtValue();
    }

    // negative Offset
    if (OffsetSigned < 0) {
      Diag(Args[0]->getExprLoc(), diag::err_cm_select_index_negative)
        << 0 << OffsetSigned << Args[0]->getSourceRange();
      return ExprError();
    }

    // degenerate case of offset >= NumElts
    if (Offset >= NumElts) {
      Diag(SelectLoc, diag::warn_cm_select_out_of_bounds_offset)
          << 0 << Offset << (NumElts - 1)
          << Args[0]->getSourceRange();
    }

    if ((Size - 1) * Stride + Offset >= NumElts) {
      // highlight the offset if it is non-zero as it contributes to the out of
      // bounds value
      if (Offset)
        Diag(SelectLoc, diag::warn_cm_select_out_of_bounds)
            << 0 << 0 << ConstArgs[0]->getSourceRange()
            << ConstArgs[1]->getSourceRange()
            << Args[0]->getSourceRange();
      else
        Diag(SelectLoc, diag::warn_cm_select_out_of_bounds)
            << 0 << 0 << ConstArgs[0]->getSourceRange()
            << ConstArgs[1]->getSourceRange();
    }
  }

  QualType ExprTy = IsMatrix
          ? Context.getCMMatrixType(
                /*IsReference*/ true, BaseTy->getCMVectorMatrixElementType(),
                Size, HSize, SelectLoc, SelectLoc, SelectLoc)
          : Context.getCMVectorType(
                /*IsReference*/ true, BaseTy->getCMVectorMatrixElementType(),
                Size, SelectLoc, SelectLoc, SelectLoc);

  return new (Context)
      CMSelectExpr(Context, CMSelectExpr::SK_select, Base, SelectLoc, ArgExprs,
                   ConstArgs.size(), RParenLoc, ExprTy, VK_LValue);
}

// \brief Semantic actions for CM vector/matrix select_all() member function
ExprResult Sema::ActOnCMSelectAll(SourceLocation SelectAllLoc, Expr *Base,
                                  SourceLocation RParenLoc) {
  assert(Base && "null base");
  if (!Base->isTypeDependent() && !Base->getType()->isCMVectorMatrixType()) {
    Diag(Base->getExprLoc(), diag::err_cm_vector_matrix_type_expected)
        << "select_all";
    return ExprError();
  }

  return new (Context)
      CMSelectExpr(Context, CMSelectExpr::SK_select_all, Base, SelectAllLoc,
                   ArrayRef<Expr *>(), RParenLoc, Base->getType(), VK_LValue);
}

/// \brief Promote Cm vector or matrix type as a operand.
/// This is different from OpenCL where vector promotions are not allowed.
ExprResult Sema::DefaultCMUnaryConversion(Expr *E) {
  if (!E)
    return E;

  QualType Ty = E->getType();
  if (Ty->isCMVectorMatrixType()) {
    if (Ty->isDependentType())
      return E;
    QualType EltTy = Ty->getCMVectorMatrixElementType();
    QualType PromotedEltTy;
    // This is simplified version of function:
    // QualType ASTContext::getPromotedIntegerType
    // TODO: handle half type.
    if (const BuiltinType *BT = EltTy->getAs<BuiltinType>()) {
      switch (BT->getKind()) {
      default:
        break;
      case BuiltinType::Char_S:
      case BuiltinType::SChar:
      case BuiltinType::Short:
      case BuiltinType::Char_U:
      case BuiltinType::UChar:
      case BuiltinType::UShort:
        PromotedEltTy = Context.IntTy;
        break;
      }
    }

    // Nothing to promote
    if (PromotedEltTy.isNull())
      return E;

    SourceLocation Loc;
    if (const CMVectorType *VTy = Ty->getAs<CMVectorType>())
      Ty = Context.getCMVectorType(false, PromotedEltTy, VTy->getNumElements(),
                                   Loc, Loc, Loc);
    else {
      const CMMatrixType *MTy = Ty->getAs<CMMatrixType>();
      Ty = Context.getCMMatrixType(false, PromotedEltTy, MTy->getNumRows(),
                                   MTy->getNumColumns(), Loc, Loc, Loc);
    }

    return ImpCastExprToType(E, Ty, CK_IntegralCast);
  }

  // scalar operand performs default unary conversion.
  return UsualUnaryConversions(E);
}

ExprResult Sema::DefaultCMReferenceToBaseConversion(Expr *E) {
  // Only do reference to base conversion for CM reference types.
  if (!E->getType()->isCMReferenceType())
    return E;

  QualType T = E->getType();
  assert(!T.isNull() && "conversion on typeless expression");
  if (T->isDependentType())
    return E;

  if (T.hasQualifiers())
    T = T.getUnqualifiedType();

  // New type is the correspoding base type.
  T = Context.getCMVectorMatrixBaseType(T);

  return ImplicitCastExpr::Create(Context, T, CK_CMReferenceToBase, E, 0,
                                  E->getValueKind());
}

/// \brief Performance sema checks for binary operators. The following
/// CM specific conversions may be performed:
///
/// (1) CK_CMReferenceToBase:
///    vector op vector_ref or vector op vector_ref
///
/// (2) CK_BitCast:
///    vector op matrix of the same size
///
/// (3) Type promotion and scalar splat:
///
///    vector op scalar or scalar op vector
///
QualType Sema::CheckCMVectorMatrixOperands(ExprResult &LHS, ExprResult &RHS,
                                           SourceLocation Loc,
                                           bool IsCompAssign,
                                           bool IsShift) {
  LHS = DefaultCMReferenceToBaseConversion(LHS.get());
  if (!IsCompAssign) {
    LHS = DefaultFunctionArrayLvalueConversion(LHS.get());
    LHS = DefaultCMUnaryConversion(LHS.get());
    if (LHS.isInvalid())
      return QualType();
  }
  RHS = DefaultCMReferenceToBaseConversion(RHS.get());
  RHS = DefaultFunctionArrayLvalueConversion(RHS.get());
  RHS = DefaultCMUnaryConversion(RHS.get());
  if (RHS.isInvalid())
    return QualType();

  // For conversion purposes, we ignore any qualifiers.
  // For example, "const float" and "float" are equivalent.
  QualType LHSType =
      Context.getCanonicalType(LHS.get()->getType()).getUnqualifiedType();
  QualType RHSType =
      Context.getCanonicalType(RHS.get()->getType()).getUnqualifiedType();

  // If the types are identical, return.
  if (LHSType == RHSType)
    return LHSType;

  // Canonicalize the vector/matrix to the LHS, remember if we swapped so we can
  // swap back (so that we don't reverse the inputs to a subtract).
  bool swapped = false;
  if (RHSType->isCMVectorMatrixType() && !IsCompAssign) {
    swapped = true;
    std::swap(RHS, LHS);
    std::swap(RHSType, LHSType);
  }

  // Type mismatch. The following is illegal:
  // int x;
  // vector<int, 8> v;
  // x += v;
  //
  if (IsCompAssign && !LHSType->isCMVectorMatrixType()) {
    assert(RHSType->isCMVectorMatrixType());
    Diag(Loc, diag::err_cm_vector_not_convertable)
        << LHS.get()->getType() << RHS.get()->getType()
        << LHS.get()->getSourceRange() << RHS.get()->getSourceRange();
    return QualType();
  }

  const CMVectorType *LVT = LHSType->getAs<CMVectorType>();
  const CMMatrixType *LMT = LHSType->getAs<CMMatrixType>();
  QualType EltTy = LVT ? LVT->getElementType() : LMT->getElementType();

  // Handle the case of a vector/matrix and scalar.
  //
  // Tricky case: handle the following code
  //
  // vector<int, 4> v;
  // (1) v += 2.0f;
  // (2) v + 2.0f;
  //
  // The result type of (1) is the same as v. For (2), it is vector<float, 4>.
  //
  if (EltTy->isIntegralType(Context) && RHSType->isScalarType()) {
    if (RHSType->isIntegralType(Context)) {
      if (!IsShift) {
        int order = Context.getIntegerTypeOrder(EltTy, RHSType);
        if (order > 0)
          RHS = ImpCastExprToType(RHS.get(), EltTy, CK_IntegralCast);

        if (order < 0) {
          LHSType =
              Context.getCMVectorMatrixTypeWithElementType(LHSType, RHSType);
          if (!IsCompAssign)
            LHS = ImpCastExprToType(LHS.get(), LHSType, CK_IntegralCast);
        }
        RHS = ImpCastExprToType(RHS.get(), LHSType, CK_CMVectorMatrixSplat);
      } else {
        // A shift is treated like in C: after usual integer promotions
        // (already done above), the result element type is the lhs element type.
        // However if swapped is true, then the lhs is scalar and the rhs is
        // vector/matrix, so the result type needs to have the element type of
        // lhs and the vector/matrix size of rhs.
        if (swapped) {
          LHSType = Context.getCMVectorMatrixTypeWithElementType(LHSType, RHSType),
          LHS = ImpCastExprToType(LHS.get(), LHSType, CK_IntegralCast);
        } else {
          RHSType = EltTy;
          RHS = ImpCastExprToType(RHS.get(), RHSType, CK_IntegralCast);
        }
        RHSType =
              Context.getCMVectorMatrixTypeWithElementType(LHSType, RHSType);
        RHS = ImpCastExprToType(RHS.get(), RHSType, CK_CMVectorMatrixSplat);
      }

      if (swapped)
        std::swap(RHS, LHS);
      return LHSType;
    }

    if (RHSType->isRealFloatingType()) {
      LHSType = Context.getCMVectorMatrixTypeWithElementType(LHSType, RHSType);
      if (!IsCompAssign)
        LHS = ImpCastExprToType(LHS.get(), LHSType, CK_IntegralToFloating);

      RHS = ImpCastExprToType(RHS.get(), LHSType, CK_CMVectorMatrixSplat);
      if (swapped)
        std::swap(RHS, LHS);
      return LHSType;
    }
  }

  if (EltTy->isRealFloatingType() && RHSType->isScalarType()) {
    if (RHSType->isRealFloatingType()) {
      int order = Context.getFloatingTypeOrder(EltTy, RHSType);
      if (order > 0)
        RHS = ImpCastExprToType(RHS.get(), EltTy, CK_FloatingCast);

      if (order < 0) {
        LHSType =
            Context.getCMVectorMatrixTypeWithElementType(LHSType, RHSType);
        if (!IsCompAssign)
          LHS = ImpCastExprToType(LHS.get(), LHSType, CK_FloatingCast);
      }
      RHS = ImpCastExprToType(RHS.get(), LHSType, CK_CMVectorMatrixSplat);

      if (swapped)
        std::swap(RHS, LHS);
      return LHSType;
    }
    if (RHSType->isIntegralType(Context)) {
      RHS = ImpCastExprToType(RHS.get(), EltTy, CK_IntegralToFloating);
      RHS = ImpCastExprToType(RHS.get(), LHSType, CK_CMVectorMatrixSplat);
      if (swapped)
        std::swap(RHS, LHS);
      return LHSType;
    }
  }

  // Both LHS and RHS are vector/matrix types. Implemented rules are:
  //
  // (1) LHS is of integral type:
  //     vector<int, 4>  op  vector<char, 4>   (IntegralCast)
  //     vector<int, 4>  op= vector<char, 4>   (IntegralCast)
  //     vector<char, 4> op= vector<int, 4>    (no conversion)
  //     vector<int, 4>  op  vector<float, 4>  (FloatingCast)
  //     vector<int, 4>  op= vector<float, 4>  (no conversion)
  //     vector<int, 4>  op  matrix<int, 2, 2> (BitCast)
  //     vector<int, 4>  op= matrix<int, 2, 2> (BitCast)
  //     matrix<int, 4, 4> op matrix<int, 2, 8> (BitCast)
  //
  // (2) LHS is of floating type:
  //     vector<float, 4> op  vector<int, 4>      (IntegralToFloating)
  //     vector<float, 4> op= vector<int, 4>      (IntegralToFloating)
  //     vector<float, 4> op  vector<double, 4>   (FloatingCast)
  //     vector<float, 4> op= vector<double, 4>   (no conversion)
  //     vector<float, 4> op  matrix<float, 2, 2> (BitCast)
  //
  unsigned NumElts = LVT ? LVT->getNumElements() : LMT->getNumElements();
  unsigned RHSNumElts = RHSType->getCMVectorMatrixSize();
  QualType RHSEltTy = RHSType->getCMVectorMatrixElementType();

  assert(NumElts > 0 && "dependent CM vector/matrix type?");

  // The same number of elements.
  if (NumElts == RHSNumElts) {
    // Conversion between vector and matrix needed or between matrix types with
    // the same size but different dimensions.
    bool NeedVMConversion =
        (LHSType->isCMVectorType() && RHSType->isCMMatrixType()) ||
        (LHSType->isCMMatrixType() && RHSType->isCMVectorType());

    // Check whether matrix dimension differs.
    if (LHSType->isCMMatrixType() && RHSType->isCMMatrixType()) {
      unsigned LHSNRows = LHSType->getAs<CMMatrixType>()->getNumRows();
      unsigned RHSNRows = RHSType->getAs<CMMatrixType>()->getNumRows();
      NeedVMConversion |= (LHSNRows != RHSNRows);
    }

    // int op short etc.
    if (EltTy->isIntegralType(Context) && RHSEltTy->isIntegralType(Context)) {
      if (!IsShift) {
        int order = Context.getIntegerTypeOrder(EltTy, RHSEltTy);
        if (order > 0) {
          RHSType = Context.getCMVectorMatrixTypeWithElementType(RHSType, EltTy);
          RHS = ImpCastExprToType(RHS.get(), RHSType, CK_IntegralCast);
        }

        if (order < 0) {
          LHSType =
              Context.getCMVectorMatrixTypeWithElementType(LHSType, RHSEltTy);
          if (!IsCompAssign)
            LHS = ImpCastExprToType(LHS.get(), LHSType, CK_IntegralCast);
        }

      } else {
        // A shift is treated like in C: after usual integer promotions
        // (already done above), the result element type is the lhs element type.
        if (swapped) {
          LHSType = Context.getCMVectorMatrixTypeWithElementType(LHSType, RHSEltTy);
          LHS = ImpCastExprToType(LHS.get(), LHSType, CK_IntegralCast);
        } else {
          RHSType = Context.getCMVectorMatrixTypeWithElementType(RHSType, EltTy);
          RHS = ImpCastExprToType(RHS.get(), RHSType, CK_IntegralCast);
        }
      }
      if (NeedVMConversion)
        RHS = ImpCastExprToType(RHS.get(), LHSType, CK_BitCast);

      if (swapped)
        std::swap(RHS, LHS);
      return LHSType;
    }

    // int op float etc.
    if (EltTy->isIntegralType(Context) && RHSEltTy->isRealFloatingType()) {
      LHSType = Context.getCMVectorMatrixTypeWithElementType(LHSType, RHSEltTy);
      if (!IsCompAssign)
        LHS = ImpCastExprToType(LHS.get(), LHSType, CK_IntegralToFloating);
      if (NeedVMConversion)
        RHS = ImpCastExprToType(RHS.get(), LHSType, CK_BitCast);

      if (swapped)
        std::swap(RHS, LHS);
      return LHSType;
    }

    // float op int etc.
    if (EltTy->isRealFloatingType() && RHSEltTy->isIntegralType(Context)) {
      RHSType = Context.getCMVectorMatrixTypeWithElementType(RHSType, EltTy);
      RHS = ImpCastExprToType(RHS.get(), RHSType, CK_IntegralToFloating);

      if (NeedVMConversion)
        RHS = ImpCastExprToType(RHS.get(), LHSType, CK_BitCast);

      if (swapped)
        std::swap(RHS, LHS);
      return LHSType;
    }

    // float op double etc.
    if (EltTy->isRealFloatingType() && RHSEltTy->isRealFloatingType()) {
      int order = Context.getFloatingTypeOrder(EltTy, RHSEltTy);
      if (order > 0) {
        RHSType = Context.getCMVectorMatrixTypeWithElementType(RHSType, EltTy);
        RHS = ImpCastExprToType(RHS.get(), RHSType, CK_FloatingCast);
      }

      if (order < 0) {
        LHSType =
            Context.getCMVectorMatrixTypeWithElementType(LHSType, RHSEltTy);
        if (!IsCompAssign)
          LHS = ImpCastExprToType(LHS.get(), LHSType, CK_FloatingCast);
      }
      if (NeedVMConversion)
        RHS = ImpCastExprToType(RHS.get(), LHSType, CK_BitCast);

      if (swapped)
        std::swap(RHS, LHS);
      return LHSType;
    }
  }

  // Vectors/matrices of different size are errors.
  if (swapped)
    std::swap(RHS, LHS);

  Diag(Loc, diag::err_cm_vector_not_convertable)
      << LHS.get()->getType() << RHS.get()->getType()
      << LHS.get()->getSourceRange() << RHS.get()->getSourceRange();
  return QualType();
}

QualType Sema::CheckCMVectorMatrixCompareOpnds(ExprResult &LHS, ExprResult &RHS,
                                               SourceLocation Loc) {
  // Check to make sure we're operating on vectors of the same type and width,
  // Allowing one side to be a scalar of element type.
  QualType VType = CheckCMVectorMatrixOperands(LHS, RHS, Loc, false);
  if (VType.isNull())
    return VType;

  assert(LHS.get()->getType()->isCMVectorMatrixType() &&
         RHS.get()->getType()->isCMVectorMatrixType());

  unsigned NumElts = LHS.get()->getType()->getCMVectorMatrixSize();

  // vector<ushort, NumElts> is the return type.
  return Context.getCMVectorType(false, Context.UnsignedShortTy, NumElts, Loc,
                                 Loc, Loc);
}

static bool CheckValueInBound(Sema &S, Expr *E, unsigned Bound, bool IsMatrix) {
  // Bound value is dependent.
  if (Bound == 0)
    return true;

  llvm::APSInt Val;
  if (!E->isValueDependent() &&
      E->isIntegerConstantExpr(Val, S.Context)) {
    int64_t Index = Val.getSExtValue();
    if (Index < 0 || Index >= (int64_t)Bound) {
      S.Diag(E->getExprLoc(), diag::err_cm_element_access_invalid_index)
          << IsMatrix << 0 << Bound << E->getSourceRange();
      return false;
    }
  }

  // Cannot determine.
  return true;
}

ExprResult Sema::ActOnCMElementAccess(Expr *Base, SourceLocation LParenLoc,
                                      MultiExprArg ArgExprs,
                                      SourceLocation RParenLoc) {
  QualType Ty = Base->getType();
  assert(Ty->isCMVectorMatrixType());

  // Wrong number of indices.
  unsigned IndexNum = Ty->isCMVectorType() ? 1u : 2u;
  if (IndexNum != ArgExprs.size()) {
    Diag(Base->getExprLoc(), diag::err_cm_element_access_invalid_index_num)
        << Ty->isCMMatrixType() << SourceRange(LParenLoc, RParenLoc);
    return ExprError();
  }

  // Check the index type.
  for (unsigned i = 0, e = ArgExprs.size(); i < e; ++i) {
    Expr *Arg = ArgExprs[i];
    if (!Arg->isTypeDependent()) {
      if (!Arg->getType()->isIntegralOrEnumerationType()) {
        Diag(Arg->getExprLoc(), diag::err_cm_non_integer_index)
            << 2 << Arg->getSourceRange();
        return ExprError();
      }
    }
  }

  SmallVector<Expr *, 2> Args;
  for (unsigned i = 0, e = ArgExprs.size(); i < e; ++i) {
    ExprResult Result = DefaultLvalueConversion(ArgExprs[i]);
    if (!Result.get()->isTypeDependent())
      Result = ImpCastExprToType(Result.get(), Context.UnsignedShortTy,
                                 CK_IntegralCast);
    if (!Result.isUsable())
      return ExprError();
    else
      Args.push_back(Result.get());
  }

  QualType EltTy;
  if (Ty->isCMVectorType()) {
    const CMVectorType *VT = Ty->getAs<CMVectorType>();
    EltTy = VT->getElementType();

    // If the number of elements is known, check out-of-bound accesses.
    if (!CheckValueInBound(*this, Args[0], VT->getNumElements(), false))
      return ExprError();
  } else {
    const CMMatrixType *MT = Ty->getAs<CMMatrixType>();
    EltTy = MT->getElementType();
    if (!CheckValueInBound(*this, Args[0], MT->getNumRows(), true) ||
        !CheckValueInBound(*this, Args[1], MT->getNumColumns(), true))
      return ExprError();
  }

  return new (Context)
      CMSelectExpr(Context, CMSelectExpr::SK_element, Base, LParenLoc, Args,
                   RParenLoc, EltTy, VK_LValue);
}

ExprResult Sema::ActOnCMSubscriptAccess(Expr *Base, SourceLocation LLoc,
                                        Expr *Idx, SourceLocation RLoc) {
  assert(Base && "null base");
  // Build an unanalyzed expression if either operand is type-dependent.
  QualType BaseType = Base->getType();
  if (getLangOpts().CPlusPlus &&
      (Base->isTypeDependent() || Idx->isTypeDependent())) {
    QualType ExprTy = Context.DependentTy;
    if (BaseType->isCMVectorType())
      ExprTy = BaseType->getCMVectorMatrixElementType();
    else if (BaseType->isCMMatrixType())
      ExprTy = Context.getDependentCMVectorType(
          /*IsRef*/ true, BaseType->getCMVectorMatrixElementType(),
          /*SizeExpr*/ 0, LLoc, LLoc, LLoc);

    return new (Context)
                 CMSelectExpr(Context, CMSelectExpr::SK_subscript, Base, LLoc,
                              Idx, RLoc, ExprTy, VK_LValue);
  }

  if (!Idx->getType()->isIntegralOrEnumerationType()) {
    Diag(Idx->getExprLoc(), diag::err_cm_non_integer_index)
        << 3 << Idx->getSourceRange();
    return ExprError();
  }

  // Convert the index type to ushort.
  ExprResult Res = DefaultLvalueConversion(Idx);
  Res = ImpCastExprToType(Res.get(), Context.UnsignedShortTy, CK_IntegralCast);

  QualType ExprType;
  if (const CMVectorType *VT = BaseType->getAs<CMVectorType>()) {
    if (!CheckValueInBound(*this, Idx, VT->getNumElements(), false))
      return ExprError();
    ExprType = VT->getElementType();
  } else {
    const CMMatrixType *MT = BaseType->getAs<CMMatrixType>();
    assert(MT && "matrix type expected");
    if (!CheckValueInBound(*this, Idx, MT->getNumRows(), false))
      return ExprError();
    ExprType = Context.getCMVectorType(
        /*isReference*/ true, MT->getElementType(), MT->getNumColumns(),
        MT->getVMLoc(), MT->getVMLoc(), MT->getVMLoc());
  }

  return new (Context)
               CMSelectExpr(Context, CMSelectExpr::SK_subscript, Base, LLoc,
                            Res.get(), RLoc, ExprType, VK_LValue);
}

ExprResult Sema::BuildCMFunctionalCastExpr(TypeSourceInfo *TInfo,
                                           QualType Ty,
                                           SourceLocation LPLoc,
                                           Expr *CastExpr,
                                           SourceLocation RPLoc,
                                           Expr *Sat) {
  ExprResult Result =
      BuildCXXFunctionalCastExpr(TInfo, Ty, LPLoc, CastExpr, RPLoc);
  if (!Result.isUsable())
    return ExprError();

  if (Sat) {
    // Check if this is referencing SAT enum defined in the header file.
    DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(Sat);
    if (!DRE || !DRE->getDecl()->getName().equals("_GENX_SAT")) {
      Diag(Sat->getExprLoc(), diag::err_cm_invalid_saturation_argument);
      return Result;
    }

    CXXFunctionalCastExpr *CE = cast<CXXFunctionalCastExpr>(Result.get());
    // Ideally we should move this to the constructor. We set saturation
    // argument here to avoid excessive changes to clang mainline.
    CE->setSatExpr(Sat);

    Expr *SubE = CE->getSubExpr();
    if (ImplicitCastExpr *ICE = dyn_cast_or_null<ImplicitCastExpr>(SubE)) {
      switch (ICE->getCastKind()) {
      default:
        // Sliently ignore saturation for integer identity casts.
        // E.g. vector<int, 4> v1;  v2 = vector<int, 4>(v1, SAT);
        if (CE->getType()->getCMVectorMatrixElementType()->isRealFloatingType())
          ICE->setSaturated(true);
        break;
      case CK_IntegralToFloating:
      case CK_FloatingToIntegral:
      case CK_FloatingCast:
      case CK_IntegralCast:
      case CK_CMVectorMatrixSplat:
        ICE->setSaturated(true);
        break;
      }
    }
  }
  return Result;
}

// CheckCmPrintfCall
//
// - first param is the surface index BTI number
// - second param is expected to be the format string
// - string args can only use 8-bit wide characters (i.e. ASCII Or UTF-8)
// - string args must be less than 128 chars
// - format conversion specifiers p and n are not supported
// - matrix, vector, and sampler values are not supported
// - maximum of 64 arguments
//
bool Sema::CheckCmPrintfCall(CallExpr *TheCall) {
  bool ErrorFound = false;
  unsigned NumArgs = TheCall->getNumArgs();
  Expr **Args = TheCall->getArgs();
  // fix up the CallExpr type
  TheCall->setType(TheCall->getCallReturnType(getASTContext()));

  if (NumArgs < 2) {
    Diag(TheCall->getRParenLoc(), diag::err_cm_format_string_expected);
    return true;
  }
  if (StringLiteral *FS = dyn_cast<StringLiteral>(Args[1])) {
    // The format string literal can't use wide characters
    // or UTF-8)
    if (FS->getCharByteWidth() > 1) {
      Diag(Args[1]->getExprLoc(),
             diag::err_cm_printf_wide_chars_not_supported);
      ErrorFound = true;
    }
    // The format string must be less than 128 bytes
    if (FS->getByteLength() > 127) {
      Diag(Args[1]->getExprLoc(), diag::err_cm_printf_string_too_long)
          << 0 << FS->getByteLength();
      ErrorFound = true;
    }
#if 0
// FIXME TODO
    // Check the format string
    llvm::SmallBitVector CheckedVarArgs;
    CheckedVarArgs.resize(NumArgs);
    CheckFormatString(FS, Args[1],
                        llvm::makeArrayRef<const Expr *>(Args, NumArgs),
                        /*hasVAListArg*/ true,
                        /*format_idx*/ 0U,
                        /*data_idx*/ 1U,
                        /*formatType*/ Sema::FST_Printf,
                        /*InFunctionCall*/ true,
                        /*CallType*/ Sema::VariadicFunction, CheckedVarArgs);
#endif
  } else {
    Diag(Args[1]->getExprLoc(), diag::err_cm_format_string_expected)
        << Args[1]->getSourceRange();
    ErrorFound = true;
  }

  for (unsigned i = 2; i < NumArgs; ++i) {
    // check each argument is acceptable type
    if (StringLiteral *FS = dyn_cast<StringLiteral>(Args[i])) {
      // String literals can't use wide characters
      if (FS->getCharByteWidth() > 1) {
        Diag(Args[i]->getExprLoc(),
               diag::err_cm_printf_wide_chars_not_supported);
        ErrorFound = true;
      }
      // Strings must be less than 128 bytes
      if (FS->getByteLength() > 127) {
        Diag(Args[0]->getExprLoc(), diag::err_cm_printf_string_too_long)
            << 1 << FS->getByteLength();
        ErrorFound = true;
      }
    }
    if (Args[i]->getType()->isCMSamplerIndexType()) {
      Diag(Args[i]->getExprLoc(), diag::err_cm_printf_unsupported_type) << 0;
      ErrorFound = true;
    }
    if (Args[i]->getType()->isCMSurfaceIndexType()) {
      Diag(Args[i]->getExprLoc(), diag::err_cm_printf_unsupported_type) << 1;
      ErrorFound = true;
    }
    if (Args[i]->getType()->isCMVmeIndexType()) {
      Diag(Args[i]->getExprLoc(), diag::err_cm_printf_unsupported_type) << 2;
      ErrorFound = true;
    }
    if (Args[i]->getType()->isCMVectorType()) {
      Diag(Args[i]->getExprLoc(), diag::err_cm_printf_unsupported_type) << 3;
      ErrorFound = true;
    }
    if (Args[i]->getType()->isCMMatrixType()) {
      Diag(Args[i]->getExprLoc(), diag::err_cm_printf_unsupported_type) << 4;
      ErrorFound = true;
    }
  }

  if (NumArgs > 65) {
    // cm_printf accepts a maximum of 64 arguments
    Diag(Args[64]->getExprLoc(), diag::err_cm_printf_too_many_args)
        << NumArgs << SourceRange(Args[64]->getExprLoc(),
                                  Args[NumArgs - 1]->getSourceRange().getEnd());
    ErrorFound = true;
  }
  return ErrorFound;
}
