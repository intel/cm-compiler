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
//  This file implements parser functions specific to MDF CM
//
//===----------------------------------------------------------------------===//

#include "clang/Parse/Parser.h"
#include "clang/Parse/RAIIObjectsForParser.h"
#include "clang/Basic/PrettyStackTrace.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/ParsedTemplate.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/TypoCorrection.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSwitch.h"
using namespace clang;

bool Parser::isCMMethodIdentifier(const IdentifierInfo &Id) {
  return llvm::StringSwitch<bool>(Id.getName())
      .Case("all", true)
      .Case("any", true)
      .Case("column", true)
      .Case("format", true)
      .Case("genx_select", true)
      .Case("iselect", true)
      .Case("merge", true)
      .Case("n_cols", true)
      .Case("n_elems", true)
      .Case("n_rows", true)
      .Case("replicate", true)
      .Case("row", true)
      .Case("select", true)
      .Case("select_all", true)
      .Default(false);
}

/// \brief For a postfix expression comprising a CM vector or matrix value
/// followed by '.' or '->' and an identifer that is a valid CM method name,
/// this function is used to parse the remainder of the expression as a
/// cm-method-expr.
///
/// \verbatim
///       postfix-expression:
///         ...
///         postfix-expression '.' cm-method-expr
///         postfix-expression '->' cm-method-expr
///         ...
///
///       cm-method-expr:
///         'all' '(' ')'
///         'any' '(' ')'
///         'column' '(' expression ')'
///         'format' '<' type-id '>' '(' ')'
///         'format' '<' type-id ',' expression ',' expression '>' '(' ')'
///         'genx_select' '<' constant-expression ',' expression ',' expression
///            '>' '(' expression ')'
///         'genx_select' '<' constant-expression ',' expression ',' expression
///            '>' '(' expression ',' expression ')'
///         'iselect' '(' cm-vector-expr ')'
///         'iselect' '(' cm-vector-expr ',' cm-vector-expr ')'
///         'merge' '(' expression ',' expression ')'
///         'merge' '(' expression ',' expression ',' expression ')'
///         'n_cols' '(' ')'
///         'n_elems' '(' ')'
///         'n_rows' '(' ')'
///         'replicate' '<' const-expr-list '>' '(' expr-list ')'
///         'row' '(' expression ')'
///         'select' '<' const-expr-list '>' '(' expr-list ')'
///         'select_all' '(' ')'
/// \endverbatim
ExprResult Parser::ParseCMMethodExpr(ExprResult LHS) {
  IdentifierInfo *Id = Tok.getIdentifierInfo();

  if (Id->isStr("all"))
    return ParseCMAll(LHS);
  if (Id->isStr("any"))
    return ParseCMAny(LHS);
  if (Id->isStr("column"))
    return ParseCMColumn(LHS);
  if (Id->isStr("format"))
    return ParseCMFormat(LHS);
  if (Id->isStr("genx_select"))
    return ParseCMGenxSelect(LHS);
  if (Id->isStr("iselect"))
    return ParseCMIselect(LHS);
  if (Id->isStr("merge"))
    return ParseCMMerge(LHS);
  if (Id->isStr("n_cols"))
    return ParseCMNCols(LHS);
  if (Id->isStr("n_elems"))
    return ParseCMNElems(LHS);
  if (Id->isStr("n_rows"))
    return ParseCMNRows(LHS);
  if (Id->isStr("replicate"))
    return ParseCMReplicate(LHS);
  if (Id->isStr("row"))
    return ParseCMRow(LHS);
  if (Id->isStr("select"))
    return ParseCMSelect(LHS);
  if (Id->isStr("select_all"))
    return ParseCMSelectAll(LHS);

  Diag(Tok.getLocation(), diag::err_cm_not_currently_supported)
      << Id->getName();
  return ExprError();
}

/// \brief Parse a vector or matrix all()
///
/// \verbatim
///     'all' '(' ')'
/// \endverbatim
ExprResult Parser::ParseCMAll(ExprResult LHS) {
  SourceLocation AllLoc = ConsumeToken();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMAll(AllLoc, LHS.get(), RParenLoc);
}

/// \brief Parse a vector or matrix any()
///
/// \verbatim
///     'any' '(' ')'
/// \endverbatim
ExprResult Parser::ParseCMAny(ExprResult LHS) {
  SourceLocation AnyLoc = ConsumeToken();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMAny(AnyLoc, LHS.get(), RParenLoc);
}

/// \brief Parse a matrix column()
///
/// \verbatim
///     'column' '(' integer-expression ')'
/// \endverbatim
ExprResult Parser::ParseCMColumn(ExprResult LHS) {
  SourceLocation ColumnLoc = ConsumeToken();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect an integer expression
  ExprResult ColExpr = ParseAssignmentExpression();
  if (ColExpr.isInvalid())
    return ExprError();

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMColumn(ColumnLoc, LHS.get(), ColExpr.get(),
                               RParenLoc);
}

/// \brief Parse a vector or matrix format()
///
/// \verbatim
///     'format' '<' type-id '>' '(' ')'
///     'format' '<' type-id ',' expression ',' expression '>' '(' ')'
/// \endverbatim
ExprResult Parser::ParseCMFormat(ExprResult LHS) {
  SourceLocation FormatLoc = ConsumeToken();
  CommaLocsTy ConstCommas;
  ExprVector FormatArgs;

  // expect '<'
  if (ExpectAndConsume(tok::less))
    return ExprError();

  TypeResult FormatType = ParseTypeName();
  if (FormatType.isInvalid()) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  // may have expression list for matrix variant
  if (Tok.is(tok::comma)) {
    ConsumeToken();

    if (ParseCMExpressionList(FormatArgs, ConstCommas)) {
      SkipUntil(tok::r_paren, StopAtSemi);
      return ExprError();
    }
  }

  // expect '>'
  SourceLocation GreaterLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::greater))
    return ExprError();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMFormat(FormatLoc, LHS.get(), FormatType.get(),
                               FormatArgs, GreaterLoc, RParenLoc);
}

/// \brief Parse a vector or matrix genx_select<>()
///
/// \verbatim
///     'genx_select' '<' constant-expression ',' expression ',' expression
///        '>' '(' expression ')'
///     'genx_select' '<' constant-expression ',' expression ',' expression
///        '>' '(' expression ',' expression ')'
/// \endverbatim
ExprResult Parser::ParseCMGenxSelect(ExprResult LHS) {
  SourceLocation GenxSelectLoc = ConsumeToken();
  CommaLocsTy GenxSelectCommas;
  ExprVector GenxSelectArgs;
  CommaLocsTy OffsetCommas;
  ExprVector OffsetArgs;

  // expect '<'
  if (ExpectAndConsume(tok::less))
    return ExprError();

  if (ParseCMExpressionList(GenxSelectArgs, GenxSelectCommas)) {
    SkipUntil(tok::greater, StopAtSemi);
    return ExprError();
  }

  // expect '>'
  SourceLocation GreaterLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::greater))
    return ExprError();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // optional expression list - offsets default to 0 for an empty list
  if (Tok.isNot(tok::r_paren)) {
    if (ParseSimpleExpressionList(OffsetArgs, OffsetCommas)) {
      SkipUntil(tok::r_paren, StopAtSemi);
      return ExprError();
    }
  }

  // expect ')'
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMGenxSelect(GenxSelectLoc, LHS.get(), GenxSelectArgs,
                                   GreaterLoc, OffsetArgs);
}

/// \brief Parse a vector or matrix iselect()
///
/// \verbatim
///     'iselect' '(' cm-vector-expr ')'
///     'iselect' '(' cm-vector-expr ',' cm-vector-expr ')'
/// \endverbatim
ExprResult Parser::ParseCMIselect(ExprResult LHS) {
  SourceLocation IselectLoc = ConsumeToken();
  CommaLocsTy IselectCommas;
  ExprVector IselectArgs;

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect one or two index vectors
  if (ParseSimpleExpressionList(IselectArgs, IselectCommas)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMISelect(IselectLoc, LHS.get(), IselectArgs, RParenLoc);
}

/// \brief Parse a vector or matrix merge()
///
/// \verbatim
///     'merge' '(' expression ',' expression ')'
///     'merge' '(' expression ',' expression ',' expression ')'
/// \endverbatim
ExprResult Parser::ParseCMMerge(ExprResult LHS) {
  SourceLocation MergeLoc = ConsumeToken();
  CommaLocsTy MergeCommas;
  ExprVector MergeArgs;

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect two or three parameters
  if (ParseSimpleExpressionList(MergeArgs, MergeCommas)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren))
    return ExprError();

  return Actions.ActOnCMMerge(MergeLoc, LHS.get(), MergeArgs, RParenLoc);
}

/// \brief Parse a matrix n_cols()
///
/// \verbatim
///     'n_cols' '(' ')'
/// \endverbatim
ExprResult Parser::ParseCMNCols(ExprResult LHS) {
  SourceLocation NColsLoc = ConsumeToken();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMNCols(NColsLoc, LHS.get(), RParenLoc);
}

/// \brief Parse a vector or matrix n_elems()
///
/// \verbatim
///     'n_elems' '(' ')'
/// \endverbatim
ExprResult Parser::ParseCMNElems(ExprResult LHS) {
  SourceLocation NElemsLoc = ConsumeToken();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMNElems(NElemsLoc, LHS.get(), RParenLoc);
}

/// \brief Parse a matrix n_rows()
///
/// \verbatim
///     'n_rows' '(' ')'
/// \endverbatim
ExprResult Parser::ParseCMNRows(ExprResult LHS) {
  SourceLocation NRowsLoc = ConsumeToken();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMNRows(NRowsLoc, LHS.get(), RParenLoc);
}

/// \brief Parse a vector or matrix replicate<>()
///
/// \verbatim
///     'replicate' '<' const-expr-list '>' '(' expr-list ')'
/// \endverbatim
ExprResult Parser::ParseCMReplicate(ExprResult LHS) {
  SourceLocation ReplicateLoc = ConsumeToken();
  CommaLocsTy ReplicateCommas;
  ExprVector ReplicateArgs;

  // expect '<'
  if (ExpectAndConsume(tok::less))
    return ExprError();

  if (ParseCMExpressionList(ReplicateArgs, ReplicateCommas)) {
    SkipUntil(tok::greater, StopAtSemi);
    return ExprError();
  }

  // expect '>'
  if (ExpectAndConsume(tok::greater))
    return ExprError();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // optional expression list - offsets default to 0 for an empty list
  CommaLocsTy Commas;
  ExprVector Offsets;
  if (Tok.isNot(tok::r_paren)) {
    if (ParseSimpleExpressionList(Offsets, Commas)) {
      SkipUntil(tok::r_paren, StopAtSemi);
      return ExprError();
    }
  }

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMReplicate(ReplicateLoc, LHS.get(), ReplicateArgs,
                                  Offsets, RParenLoc);
}

/// \brief Parse a matrix row()
///
/// \verbatim
///     'row' '(' integer-expression ')'
/// \endverbatim
ExprResult Parser::ParseCMRow(ExprResult LHS) {
  SourceLocation RowLoc = ConsumeToken();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect an integer expression
  ExprResult RowExpr = ParseAssignmentExpression();
  if (RowExpr.isInvalid())
    return ExprError();

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMRow(RowLoc, LHS.get(), RowExpr.get(), RParenLoc);
}

/// \brief Parse a vector or matrix select<>()
///
/// \verbatim
///     'select' '<' const-expr-list '>' '(' expr-list ')'
/// \endverbatim
/// We parse any number of elements in the expr-lists, and rely on semantics to
/// enforce semantic rules appropriate for vector or matrix selects.
ExprResult Parser::ParseCMSelect(ExprResult LHS) {
  SourceLocation SelectLoc = ConsumeToken();
  Expr *Base = LHS.get();
  CommaLocsTy ConstCommas;
  ExprVector ConstArgs;

  // expect '<'
  if (ExpectAndConsume(tok::less))
    return ExprError();

  if (ParseCMExpressionList(ConstArgs, ConstCommas)) {
    SkipUntil(tok::greater, StopAtSemi);
    return ExprError();
  }

  // expect '>'
  if (ExpectAndConsume(tok::greater))
    return ExprError();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // optional expression list - offsets default to 0 for an empty list
  CommaLocsTy Commas;
  ExprVector Args;

  // An empty expression list is allowed.
  if (Tok.isNot(tok::r_paren)) {
    if (ParseSimpleExpressionList(Args, Commas)) {
      SkipUntil(tok::r_paren, StopAtSemi);
      return ExprError();
    }
  }

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  // Select is syntactically valid, now deal with semantics.
  return Actions.ActOnCMSelect(SelectLoc, Base, ConstArgs, Args, RParenLoc);
}

/// \brief Parse a vector or matrix select_all()
///
/// \verbatim
///     'select_all' '(' ')'
/// \endverbatim
ExprResult Parser::ParseCMSelectAll(ExprResult LHS) {
  SourceLocation SelectAllLoc = ConsumeToken();

  // expect '('
  if (ExpectAndConsume(tok::l_paren))
    return ExprError();

  // expect ')'
  SourceLocation RParenLoc = Tok.getLocation();
  if (ExpectAndConsume(tok::r_paren)) {
    SkipUntil(tok::r_paren, StopAtSemi);
    return ExprError();
  }

  return Actions.ActOnCMSelectAll(SelectAllLoc, LHS.get(), RParenLoc);
}

/// ParseCMExpressionList - A simple comma-separated list of expressions in a
/// context where '>' is not an operator.
///
/// \verbatim
///       expr-list:
///         expression
///         expr-list , expression
/// \endverbatim
bool Parser::ParseCMExpressionList(SmallVectorImpl<Expr *> &Exprs,
                                   SmallVectorImpl<SourceLocation> &CommaLocs) {
  GreaterThanIsOperatorScope G(GreaterThanIsOperator, false);
  while (1) {
    ExprResult Expr = ParseConstantExpression();
    if (Expr.isInvalid())
      return true;

    Exprs.push_back(Expr.get());

    if (Tok.isNot(tok::comma))
      return false;

    // Move to the next argument, remember where the comma was.
    CommaLocs.push_back(ConsumeToken());
  }
}
