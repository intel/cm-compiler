#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2014-2021 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT
#============================ end_copyright_notice =============================

set(LLVM_LINK_COMPONENTS
  Support
  )

if (MSVC)
  set_source_files_properties(SemaDeclAttr.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(SemaExpr.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(SemaExprCXX.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(SemaTemplate.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()

add_clang_library(clangSema
  AnalysisBasedWarnings.cpp
  CodeCompleteConsumer.cpp
  DeclSpec.cpp
  DelayedDiagnostic.cpp
  IdentifierResolver.cpp
  JumpDiagnostics.cpp
  MultiplexExternalSemaSource.cpp
  ParsedAttr.cpp
  Scope.cpp
  ScopeInfo.cpp
  Sema.cpp
  SemaAccess.cpp
  SemaAttr.cpp
  SemaCXXScopeSpec.cpp
  SemaCast.cpp
  SemaChecking.cpp
  SemaCodeComplete.cpp
  SemaConsumer.cpp
  SemaCoroutine.cpp
  SemaCUDA.cpp
  SemaDecl.cpp
  SemaDeclAttr.cpp
  SemaDeclCXX.cpp
  SemaDeclObjC.cpp
  SemaExceptionSpec.cpp
  SemaExpr.cpp
  SemaExprCM.cpp
  SemaExprCXX.cpp
  SemaExprMember.cpp
  SemaExprObjC.cpp
  SemaFixItUtils.cpp
  SemaInit.cpp
  SemaInitCM.cpp
  SemaLambda.cpp
  SemaLookup.cpp
  SemaObjCProperty.cpp
  SemaOpenMP.cpp
  SemaOverload.cpp
  SemaPseudoObject.cpp
  SemaStmt.cpp
  SemaStmtAsm.cpp
  SemaStmtAttr.cpp
  SemaTemplate.cpp
  SemaTemplateDeduction.cpp
  SemaTemplateInstantiate.cpp
  SemaTemplateInstantiateDecl.cpp
  SemaTemplateVariadic.cpp
  SemaType.cpp
  SemaTypeCM.cpp
  TypeLocBuilder.cpp

  LINK_LIBS
  clangAST
  clangAnalysis
  clangBasic
  clangEdit
  clangLex
  )
