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
//  This file defines the CM specific Expr interface and subclasses.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_EXPRCM_H
#define LLVM_CLANG_AST_EXPRCM_H

#include "clang/AST/Expr.h"
#include "clang/AST/ASTContext.h"

namespace clang {
class IdentifierInfo;
class ASTContext;

/// \brief Represent CM Boolean Reduction member functions.
///
/// \code
/// matrix<float, 4, 4> m;
/// ushort a = m.all();
/// ushort a = m.any();
/// \code
///
class CMBoolReductionExpr : public CMMemberExpr {
public:
  enum CMBoolReductionKind {
    RK_all,
    RK_any
  };

private:
  /// \brief The kind of boolean reduction.
  CMBoolReductionKind BRKind;

  /// \brief The closing ')' location.
  SourceLocation RParenLoc;

public:
  CMBoolReductionExpr(const ASTContext &C, CMBoolReductionKind RK, Expr *Base,
                      SourceLocation ReductionLoc, SourceLocation RPLoc)
      : CMMemberExpr(C, CMBoolReductionExprClass, Base, ReductionLoc,
                     SmallVector<Expr *, 4>(), C.UnsignedShortTy, VK_RValue),
        BRKind(RK), RParenLoc(RPLoc) {};

  CMBoolReductionKind getBoolReductionKind() const { return BRKind; }

  bool isAll() const { return BRKind == RK_all; }
  bool isAny() const { return BRKind == RK_any; }

  const char *getBoolReductionKindName() const {
    switch (BRKind) {
    case RK_all:
      return "all";
    case RK_any:
      return "any";
    }
    llvm_unreachable("invalid boolean reduction kind");
  }

  SourceLocation getRParenLoc() const { return RParenLoc; }
  SourceLocation getLocStart() const { return getBase()->getLocStart(); }
  SourceLocation getLocEnd() const { return getRParenLoc(); }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CMBoolReductionExprClass;
  }
};

/// \brief Represent CM merge member function.
///
/// \code
/// matrix<int, 4, 4> m;
/// matrix<int, 4, 4> src1;
/// matrix<int, 4, 4> src2;
/// matrix<ushort, 4, 4> mask;
/// m.merge(src1, mask);
/// m.merge(src1, src2, mask);
/// \code
///
/// The SubExprs store Base and the 2 or 3 arguments.
/// The access method getSrc2() will return the SubExpr[2] if there are 3
/// arguments, and SubExpr[0] (i.e. Base) if there are only 2 arguments.
/// The access method getMask() will always return the last SubExpr.
class CMMergeExpr : public CMMemberExpr {
private:
  /// \brief The closing ')' location.
  SourceLocation RParenLoc;

public:
  CMMergeExpr(const ASTContext &C, Expr *Base, SourceLocation MergeLoc,
              ArrayRef<Expr *> Args, SourceLocation RPLoc, QualType ExprTy,
              ExprValueKind VK)
      : CMMemberExpr(C, CMMergeExprClass, Base, MergeLoc, Args, ExprTy, VK),
        RParenLoc(RPLoc) {}

  Expr *getSrc1() const {
    assert(NumSubExprs >= 2);
    return static_cast<Expr *>(SubExprs[1]);
  }
  Expr *getSrc2() const {
    assert(isTwoSourceMerge());
    return static_cast<Expr *>(SubExprs[2]);
  }
  Expr *getMask() const {
    assert(NumSubExprs >= 3);
    // Mask is always the last argument
    return static_cast<Expr *>(SubExprs[NumSubExprs - 1]);
  }

  bool isOneSourceMerge() const { return !isTwoSourceMerge(); }
  bool isTwoSourceMerge() const { return NumSubExprs == 4; }

  SourceLocation getRParenLoc() const { return RParenLoc; }
  SourceLocation getLocStart() const { return getBase()->getLocStart(); }
  SourceLocation getLocEnd() const { return getRParenLoc(); }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CMMergeExprClass;
  }
};

class CMSizeExpr : public CMMemberExpr {
public:
  enum CMSizeExprKind { SK_n_elems, SK_n_rows, SK_n_cols };

private:
  /// \brief The size expression kind.
  CMSizeExprKind Kind;

  /// \brief The closing ')' location.
  SourceLocation RParenLoc;

public:
  CMSizeExpr(const ASTContext &C, CMSizeExprKind SK, Expr *Base,
             SourceLocation MemLoc, SourceLocation RPLoc)
      : CMMemberExpr(C, CMSizeExprClass, Base, MemLoc, ArrayRef<Expr *>(),
                     C.IntTy, VK_RValue),
        Kind(SK), RParenLoc(RPLoc) {}

  bool isNElems() const { return Kind == SK_n_elems; }
  bool isNRows() const { return Kind == SK_n_rows; }
  bool isNCols() const { return Kind == SK_n_cols; }

  const char *getSizeExprKindName() const {
    switch (Kind) {
    case SK_n_elems:
      return "n_elems";
    case SK_n_rows:
      return "n_rows";
    case SK_n_cols:
      return "n_cols";
    }
    llvm_unreachable("invalid size kind");
  }

  SourceLocation getRParenLoc() const { return RParenLoc; }
  SourceLocation getLocStart() const { return getBase()->getLocStart(); }
  SourceLocation getLocEnd() const { return getRParenLoc(); }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CMSizeExprClass;
  }
};

/// \brief Represent CM format member function.
///
/// \code
/// matrix<int, 4, 4> m1;
/// matrix_ref<char, 4, 16> m2 = m1.format<char, 4, 16>( );
/// vector_ref<int, 16> v = m1.format<int>( );
/// \code
///
class CMFormatExpr : public CMMemberExpr {
private:
  /// \brief The format element type
  QualType ElementType;

  /// \brief The closing ')' location.
  SourceLocation RParenLoc;

public:
  CMFormatExpr(const ASTContext &C, Expr *Base,
               SourceLocation FormatLoc, QualType ElType,
               ArrayRef<Expr *> Args, SourceLocation RPLoc,
               QualType ExprTy, ExprValueKind VK)
    : CMMemberExpr(C, CMFormatExprClass, Base, FormatLoc, Args, ExprTy, VK),
      ElementType(ElType), RParenLoc(RPLoc) {}

  bool isMatrixFormat() const { return NumSubExprs == 3; }

  QualType getElementType() const {
    return ElementType;
  }

  Expr *getRows() const {
    assert(NumSubExprs >= 2);
    return static_cast<Expr *>(SubExprs[1]);
  }

  Expr *getColumns() const {
    assert(NumSubExprs >= 3);
    return static_cast<Expr *>(SubExprs[2]);
  }

  SourceLocation getRParenLoc() const { return RParenLoc; }
  SourceLocation getLocStart() const { return getBase()->getLocStart(); }
  SourceLocation getLocEnd() const { return getRParenLoc(); }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CMFormatExprClass;
  }
};

} // end namespace clang

#endif
