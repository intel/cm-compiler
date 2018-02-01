/*
 * Copyright (c) 2018, Intel Corporation
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
//  This file implements semantic analysis specific to MDF CM
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExprObjC.h"
#include "clang/AST/StmtCXX.h"
#include "clang/Analysis/Analyses/FormatString.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/ScopeInfo.h"
#include "clang/Sema/Sema.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <limits>
using namespace clang;
using namespace sema;

// Check_cm_slm_atomic
//
// - check op (2nd arg) is within expected range
// - check no src given for INC or DEC
// - check two src given for CMPXCHG
// - check one src given for all other ops
//
static bool Check_cm_slm_atomic(Sema &S, Expr **Args, CallExpr *TheCall) {
  llvm::APSInt Value;
  if (Args[1]->EvaluateAsInt(Value, S.Context)) {
    unsigned NumArgs = TheCall->getNumArgs();
    int op = Value.getLimitedValue();
    if (op < 0 || op > 0xc) {
      S.Diag(Args[1]->getExprLoc(), diag::err_cm_atomic_unknown_op) << op;
      return true;
    } else if (op == 0x2 /*INC*/ || op == 0x3 /*DEC*/) {
      if (NumArgs > 4) {
        S.Diag(Args[4]->getExprLoc(), diag::err_cm_atomic_no_src_expected) << op;
        return true;
      }
    } else if (op == 0x7 /*CMPXCHG*/ && NumArgs < 6) {
      S.Diag(TheCall->getRParenLoc(), diag::err_cm_atomic_two_src_expected);
      return true;
    } else if (NumArgs == 6) {
      S.Diag(Args[5]->getExprLoc(), diag::err_cm_atomic_one_src_expected_two_given) << op;
      return true;
    } else if (NumArgs < 5) {
      S.Diag(TheCall->getRParenLoc(), diag::err_cm_atomic_one_src_expected) << op;
      return true;
    }
  }
  return false;
}

// CheckCmFunctionCall - Check calls to CM functions for properties
// not otherwise enforced by Clang.
//
bool Sema::CheckCmFunctionCall(FunctionDecl *FDecl, CallExpr *TheCall) {

  IdentifierInfo *FnInfo = FDecl->getIdentifier();
  Expr** Args = TheCall->getArgs();
  unsigned NumArgs = TheCall->getNumArgs();

  if (FnInfo->isStr("cm_slm_atomic") && NumArgs >= 2) {
    return Check_cm_slm_atomic(*this, Args, TheCall);
  }

  // We ignore all the following function calls if they appear in a system header file
  if (Context.getSourceManager().isInSystemHeader(TheCall->getExprLoc()))
    return false;

  // No functions are currently ignored if in a system header file.

  return false;
}

ExprResult Sema::CheckGenxBuiltinFunctionCall(unsigned BuiltinID,
                                              CallExpr *TheCall) {
  switch (BuiltinID) {
  default:
    return TheCall;
  }
  return TheCall;
}
