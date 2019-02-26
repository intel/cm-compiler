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
// This file implements semantic analysis for CM related initializers.
//
//===----------------------------------------------------------------------===//
#include "clang/Sema/SemaInternal.h"

using namespace clang;

//
// The following forms of initializations are to be supported:
//
// (1) vector/matrix splat conversion. I.e. scalar values can be implicitly
//     casted to CM vector/matrix values as initializers.
//
//     vector<int, 4> v1 = 1;
//     matrix<int, 4, 4> m2 = 1;
//     vector<int, 4> v3 = 1.0f;  // allowed.
//
// (2) vector/matrix conversion. I.e. CM vector/matrix values can be implicitly
//     casted to vector/matrix with the same size.
//
//     matrix<int, 2, 2> m = v1;
//
//     Similar to standard C++, constructors can be also used in CM to set the
//     data elements in vector/matrix objects. One restriction is that
//     vector_ref constructor can only take vector/vector_ref object with the
//     same data type/size, which must be contiguous. Similarly, the matrix_ref
//     constructor can only take matrix/matrix_ref object with the same data
//     type/size, which must be contiguous.
//
// (3) vector/matrix array initializers.
//
//     const ushort init[] = {0, 1, 2, 3};
//     vector<ushort, 8> v(init);
//
//     Initializer array could be bigger or smaller than the initialized
//     array–initialization would be done up to the minimum of their sizes.
//     Initializer array can have more than one dimension – this gives more
//     flexibility to users allowing them to skip initialization of some
//     segments of the matrix or vector.
//
//     const short init_tab[5][16] = {{4,6,8},{0,1,1,0,0},{8,9,7,6},{23},{45}};
//     matrix<uint, 5, 16> table(init_tab);
//

static bool isConvertibleVMType(Sema &S, QualType FromType, QualType ToType,
                                bool AllowCMVMRefConversion) {
  assert(FromType->isCMVectorMatrixType() && ToType->isCMVectorMatrixType());

  unsigned ToSize = ToType->getCMVectorMatrixSize();
  unsigned FromSize = FromType->getCMVectorMatrixSize();

  // When the target type is a reference, then the source type must have the
  // same data type and size. Type vector_ref only takes vector or vector_ref
  // types; type matrix_ref only takes matrix or matrix_ref types.
  //
  // Note: Is this allowed? We currently do not support.
  //
  //   matrix<float, 8, 2> v = 0;
  //   mtrix_ref<float, 4, 4> mref = v;
  //
  if (ToType->isCMReferenceType()) {
     // For assignments, the following is allowed:
     // matrix<float, 8, 2> v = 0;
     // mtrix_ref<float, 4, 4> mref = ...;
     // mref = v;
     if (AllowCMVMRefConversion) {
       // Any element type is allowed, conversion will be performed.
       return ToSize == FromSize;
     }

     if (FromType->isCMVectorType() && ToType->isCMVectorType()) {
       // Do not just compare the type, since FromType may not be a reference.
       const CMVectorType *FT = FromType->getAs<CMVectorType>();
       const CMVectorType *TT = ToType->getAs<CMVectorType>();
       return S.Context.hasSameType(FT->getElementType(),
                                    TT->getElementType()) &&
              FT->getNumElements() == TT->getNumElements();
     }

     if (FromType->isCMMatrixType() && ToType->isCMMatrixType()) {
       // Do not just compare the type, since FromType may not be a reference.
       const CMMatrixType *FT = FromType->getAs<CMMatrixType>();
       const CMMatrixType *TT = ToType->getAs<CMMatrixType>();
       return S.Context.hasSameType(FT->getElementType(),
                                    TT->getElementType()) &&
              FT->getNumRows() == TT->getNumRows() &&
              FT->getNumColumns() == TT->getNumColumns();
     }

     // FromType and ToType have different shape.
     return false;
  }

  // ToType is not a reference. It is allowed to have different shape as long as
  // they have the same size.
  return ToSize == FromSize;
}

// FIXME: Revisit this function. We may have missed some implicit conversions.
bool Sema::IsCMVectorConversion(QualType FromType, QualType ToType,
                                ImplicitConversionKind &ICK,
                                bool AllowCMVMRefConversion) {
  // No conversion if ToType is not a vector/matrix type.
  if (!ToType->isCMVectorMatrixType())
    return false;

  // Identical types require no conversions.
  if (Context.hasSameUnqualifiedType(FromType, ToType))
    return false;

  // Check if this is convertible when FromType is a vector/matrix type.
  if (FromType->isCMVectorMatrixType() &&
      isConvertibleVMType(*this, FromType, ToType, AllowCMVMRefConversion)) {

    // Check if this is elmentalwise implicit conversion.
    QualType FromEltType = FromType->getCMVectorMatrixElementType();
    QualType ToEltType = ToType->getCMVectorMatrixElementType();

    if (Context.hasSameUnqualifiedType(FromEltType, ToEltType)) {
      if (ToType->isCMReferenceType() && FromType->isCMBaseType())
        ICK = ICK_CMBaseToReference;
      else if (ToType->isCMBaseType() && FromType->isCMReferenceType())
        ICK = ICK_CMReferenceToBase;
      else if (ToType->isCMBaseType() && FromType->isCMBaseType())
        ICK = ICK_CMVectorMatrix_Conversion;
      else
        ICK = ICK_Identity;
      return true;
    } else if (IsFloatingPointPromotion(FromEltType, ToEltType)) {
      ICK = ICK_Floating_Promotion;
      return true;
    } else if (FromEltType->isRealFloatingType() &&
               ToEltType->isRealFloatingType()) {
      ICK = ICK_Floating_Conversion;
      return true;
    } else if ((FromEltType->isRealFloatingType() &&
                ToEltType->isIntegralType(Context)) ||
               (FromEltType->isIntegralType(Context) &&
                ToEltType->isRealFloatingType())) {
      ICK = ICK_Floating_Integral;
      return true;
    } else if (FromEltType->isIntegralType(Context) &&
               ToEltType->isIntegralType(Context)) {
      ICK = ICK_Integral_Conversion;
      return true;
    }

    // Otherwise, this is a vector-to-matrix or matrix-to-vector conversions.
    ICK = ICK_CMVectorMatrix_Conversion;
    return true;
  }

  // Vector splat from any CM element type to a vector.
  // If assigning to a ref expression, then it is allowed to splat conversion.
  // Scalar comparison produces bool type which is not a valid CM element type,
  // but it is allowed to initialize a CM vector/matrix.
  // Splat from an enum type is also allowed.
  if ((ToType->isCMBaseType() || AllowCMVMRefConversion) &&
      (FromType->isCMElementType() || FromType->isBooleanType() ||
       FromType->isEnumeralType())) {
    ICK = ICK_CMVectorMatrix_Splat;
    return true;
  }

  // Anything else is not supported.
  return false;
}

void Sema::DiagnoseCMVectorMatrixInitializer(QualType VarType, Expr *E) {
  DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E);
  if (!DRE) {
    Diag(E->getExprLoc(), diag::err_cm_invalid_vector_matrix_init)
        << VarType->isCMMatrixType();
    return;
  }

  VarDecl *VD = dyn_cast_or_null<VarDecl>(DRE->getDecl());
  if (!VD) {
    Diag(E->getExprLoc(), diag::err_cm_invalid_vector_matrix_init)
        << VarType->isCMMatrixType();
    return;
  }

  // Only allow global constant arrays.
  if (!VD->hasGlobalStorage() || VD->isLocalVarDecl()) {
    Diag(E->getExprLoc(), diag::err_cm_vector_matrix_init_not_global)
        << VarType->isCMMatrixType() << E->getSourceRange();
    Diag(VD->getLocation(), diag::note_cm_var_initialization);
    return;
  }

  if (!VD->getInit()) {
    Diag(E->getExprLoc(), diag::err_cm_invalid_vector_matrix_init)
        << E->getSourceRange();
    Diag(VD->getLocation(), diag::note_cm_var_initialization);
    return;
  }

#if 0
  // It is not clear that array must be constant. Disable this check for now.
  // Anyway, C++ initializer list is a better syntax.
  if (!VD->getType().getQualifiers().hasConst()) {
    Diag(E->getExprLoc(), diag::err_cm_vector_matrix_init_not_constant)
        << VarType->isCMMatrixType() << E->getSourceRange();
    Diag(VD->getLocation(), diag::note_cm_var_initialization);
    return;
  }
#endif

  // Check the element type, which needs to be a valid CM element type,
  // or recursively a valid CM initializer type.
  const ConstantArrayType *AT = cast<ConstantArrayType>(E->getType());
  while (true) {
    QualType EltTy = AT->getElementType();
    if (EltTy->isCMElementType())
      break;
    else if ((AT = dyn_cast<ConstantArrayType>(EltTy)))
      continue;
    else {
      Diag(E->getExprLoc(), diag::err_cm_invalid_vector_matrix_init)
          << VarType->isCMMatrixType();
      return;
    }
  }

  // Any missing checks?
}
