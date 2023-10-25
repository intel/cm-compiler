#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2014-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  MC
  MCParser
  Support
  )

add_clang_library(clangParse
  ParseAST.cpp
  ParseCXXInlineMethods.cpp
  ParseDecl.cpp
  ParseDeclCXX.cpp
  ParseExpr.cpp
  ParseExprCM.cpp
  ParseExprCXX.cpp
  ParseInit.cpp
  ParseObjc.cpp
  ParseOpenMP.cpp
  ParsePragma.cpp
  ParseStmt.cpp
  ParseStmtAsm.cpp
  ParseTemplate.cpp
  ParseTentative.cpp
  Parser.cpp

  LINK_LIBS
  clangAST
  clangBasic
  clangLex
  clangSema
  )
