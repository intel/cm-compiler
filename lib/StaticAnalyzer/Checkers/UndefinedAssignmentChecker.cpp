/*========================== begin_copyright_notice ============================

Copyright (C) 2016-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

SPDX-License-Identifier: MIT
============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

// This defines UndefinedAssignmentChecker, a builtin check in ExprEngine that
// checks for assigning undefined values.

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {
class UndefinedAssignmentChecker
  : public Checker<check::Bind> {
  mutable std::unique_ptr<BugType> BT;

public:
  void checkBind(SVal location, SVal val, const Stmt *S,
                 CheckerContext &C) const;
};
}

void UndefinedAssignmentChecker::checkBind(SVal location, SVal val,
                                           const Stmt *StoreE,
                                           CheckerContext &C) const {
  if (!val.isUndef())
    return;

  // Do not report assignments of uninitialized values inside swap functions.
  // This should allow to swap partially uninitialized structs
  // (radar://14129997)
  if (const FunctionDecl *EnclosingFunctionDecl =
      dyn_cast<FunctionDecl>(C.getStackFrame()->getDecl()))
    if (C.getCalleeName(EnclosingFunctionDecl) == "swap")
      return;

  ExplodedNode *N = C.generateErrorNode();

  if (!N)
    return;

  static const char *const DefaultMsg =
      "Assigned value is garbage or undefined";
  if (!BT)
    BT.reset(new BuiltinBug(this, DefaultMsg));

  // Generate a report for this bug.
  llvm::SmallString<128> Str;
  llvm::raw_svector_ostream OS(Str);

  const Expr *ex = nullptr;

  while (StoreE) {
    if (const UnaryOperator *U = dyn_cast<UnaryOperator>(StoreE)) {
      OS << "The expression is an uninitialized value. "
            "The computed value will also be garbage";

      ex = U->getSubExpr();
      break;
    }

    if (const BinaryOperator *B = dyn_cast<BinaryOperator>(StoreE)) {
      if (B->isCompoundAssignmentOp()) {
        if (C.getSVal(B->getLHS()).isUndef()) {
          OS << "The left expression of the compound assignment is an "
                "uninitialized value. The computed value will also be garbage";
          ex = B->getLHS();
          break;
        }
      }

      ex = B->getRHS();
      break;
    }

    if (const DeclStmt *DS = dyn_cast<DeclStmt>(StoreE)) {
      const VarDecl *VD = dyn_cast<VarDecl>(DS->getSingleDecl());
      ex = VD->getInit();
    }

    if (const auto *CD =
            dyn_cast<CXXConstructorDecl>(C.getStackFrame()->getDecl())) {
      if (CD->isImplicit()) {
        for (auto I : CD->inits()) {
          if (I->getInit()->IgnoreImpCasts() == StoreE) {
            OS << "Value assigned to field '" << I->getMember()->getName()
               << "' in implicit constructor is garbage or undefined";
            break;
          }
        }
      }
    }

    break;
  }

  // CM vector/matrix values may be aliased, so we can't currently be certain
  // whether one is actually undefined or not. Ignore them for now.
  if (ex->getType()->isCMVectorMatrixType())
    return;

  if (OS.str().empty())
    OS << DefaultMsg;

  auto R = llvm::make_unique<BugReport>(*BT, OS.str(), N);
  if (ex) {
    R->addRange(ex->getSourceRange());
    bugreporter::trackExpressionValue(N, ex, *R);
  }
  C.emitReport(std::move(R));
}

void ento::registerUndefinedAssignmentChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefinedAssignmentChecker>();
}
