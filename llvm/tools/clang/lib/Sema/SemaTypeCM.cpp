//===--- SemaTypeCM.cpp - Semantic Analysis for Types ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements type-related semantic analysis for CM.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/Expr.h"

using namespace clang;

namespace {

enum CMTypeKind { TK_Vector = 0, TK_Matrix, TK_VectorRef, TK_MatrixRef };

} // namespace

static bool checkCMElementType(Sema &S, CMTypeKind Kind, QualType EltTy,
                               SourceLocation VMLoc) {
  if (EltTy.isNull())
    return false;

  if (EltTy->isReferenceType()) {
    S.Diag(VMLoc, diag::err_cm_element_type_no_ref) << Kind << EltTy;
    return false;
  }

  if (EltTy->isPointerType()) {
    S.Diag(VMLoc, diag::err_cm_element_type_no_ptr) << Kind << EltTy;
    return false;
  }

  if (EltTy->isIncompleteType()) {
    S.Diag(VMLoc, diag::err_cm_element_type_incomplete) << Kind << EltTy;
    return false;
  }

  // No qualifiers allowed.
  if (EltTy.hasQualifiers()) {
    S.Diag(VMLoc, diag::err_cm_element_type_no_qualifier) << Kind << EltTy;
    return false;
  }

  // Defer other type checking if it is dependent.
  if (EltTy->isDependentType())
    return true;

  // Finally, check if the element type is a supported type and emit a general
  // error if necessary.
  if (!EltTy->isCMElementType()) {
    S.Diag(VMLoc, diag::err_cm_invalid_element_type) << Kind << EltTy;
    return false;
  }

  return true;
}

QualType Sema::BuildCMVectorType(bool IsReference, QualType T, Expr *SizeExpr,
                                 SourceLocation VLoc, SourceLocation LessLoc,
                                 SourceLocation GreaterLoc) {
  CMTypeKind Kind = IsReference ? TK_VectorRef : TK_Vector;

  // Check the element type.
  if (!checkCMElementType(*this, Kind, T, VLoc))
    return QualType();

  // Check the size expression and its type.
  if (!SizeExpr)
    return QualType();

  if (!SizeExpr->isTypeDependent() && !SizeExpr->isValueDependent()) {
    llvm::APSInt Size;
    if (!SizeExpr->isIntegerConstantExpr(Size, Context)) {
      Diag(SizeExpr->getLocStart(), diag::err_cm_dim_expr_type)
          << Kind << 0 << SizeExpr->getSourceRange();
      return QualType();
    }

    int VS = static_cast<int>(Size.getSExtValue());
    if (VS <= 0) {
      Diag(SizeExpr->getLocStart(), diag::err_cm_positive_dim_expected)
          << Kind << 0 << SizeExpr->getSourceRange();
      return QualType();
    }

    // Vector size must be less than 8kb
    if (!T->isDependentType()) {
      int VectorSizeInBytes = (VS * Context.getTypeSize(T)) / 8;
      if (VectorSizeInBytes >= 8192) {
        Diag(VLoc, diag::err_cm_max_data_size_exceeded)
          << 0 << VectorSizeInBytes << SourceRange(VLoc, GreaterLoc);
        return QualType();
      }
    }

    // No error found.
    return Context.getCMVectorType(IsReference, T, VS, VLoc, LessLoc,
                                   GreaterLoc);
  }

  return Context.getDependentCMVectorType(IsReference, T, SizeExpr, VLoc,
                                          LessLoc, GreaterLoc);
}

QualType Sema::BuildCMMatrixType(bool IsReference, QualType T, Expr *NRowExpr,
                                 Expr *NColExpr, SourceLocation MLoc,
                                 SourceLocation LessLoc,
                                 SourceLocation GreaterLoc) {
  CMTypeKind Kind = IsReference ? TK_MatrixRef : TK_Matrix;

  // Check the element type.
  if (!checkCMElementType(*this, Kind, T, MLoc))
    return QualType();

  // Check the size expression and its type.
  if (!NRowExpr || !NColExpr)
    return QualType();

  if (!NRowExpr->isTypeDependent() && !NRowExpr->isValueDependent() &&
      !NColExpr->isTypeDependent() && !NColExpr->isValueDependent()) {
    llvm::APSInt NRow;
    if (!NRowExpr->isIntegerConstantExpr(NRow, Context)) {
      Diag(NRowExpr->getLocStart(), diag::err_cm_dim_expr_type)
          << Kind << 1 << NRowExpr->getSourceRange();
      return QualType();
    }
    int NumRows = static_cast<int>(NRow.getSExtValue());
    if (NumRows <= 0) {
      Diag(NRowExpr->getLocStart(), diag::err_cm_positive_dim_expected)
          << 1 << NRowExpr->getSourceRange();
      return QualType();
    }

    llvm::APSInt NCol;
    if (!NColExpr->isIntegerConstantExpr(NCol, Context)) {
      Diag(NColExpr->getLocStart(), diag::err_cm_dim_expr_type)
          << Kind << 2 << NColExpr->getSourceRange();
      return QualType();
    }
    int NumCols = static_cast<int>(NCol.getSExtValue());
    if (NumCols <= 0) {
      Diag(NColExpr->getLocStart(), diag::err_cm_positive_dim_expected)
          << 2 << NColExpr->getSourceRange();
      return QualType();
    }

    // Matrix size must be less than 8kb
    if (!T->isDependentType()) {
      int MatrixSizeInBytes = (NumRows * NumCols * Context.getTypeSize(T)) / 8;
      if (MatrixSizeInBytes >= 8192) {
        Diag(MLoc, diag::err_cm_max_data_size_exceeded)
          << 1 << MatrixSizeInBytes << SourceRange(MLoc, GreaterLoc);
        return QualType();
      }
    }

    // No error found.
    return Context.getCMMatrixType(IsReference, T, NumRows, NumCols, MLoc,
                                   LessLoc, GreaterLoc);
  }

  return Context.getDependentCMMatrixType(IsReference, T, NRowExpr, NColExpr,
                                          MLoc, LessLoc, GreaterLoc);
}
