#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2020-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

add_subdirectory(Headers)
add_subdirectory(Basic)
add_subdirectory(Lex)
add_subdirectory(Parse)
add_subdirectory(AST)
add_subdirectory(ASTMatchers)
add_subdirectory(CrossTU)
add_subdirectory(Sema)
add_subdirectory(CodeGen)
add_subdirectory(Analysis)
add_subdirectory(Edit)
add_subdirectory(Rewrite)
if(CLANG_ENABLE_ARCMT)
  add_subdirectory(ARCMigrate)
endif()
add_subdirectory(Driver)
add_subdirectory(Serialization)
add_subdirectory(Frontend)
add_subdirectory(FrontendTool)
add_subdirectory(Tooling)
add_subdirectory(DirectoryWatcher)
add_subdirectory(Index)
if(CLANG_ENABLE_STATIC_ANALYZER)
  add_subdirectory(StaticAnalyzer)
endif()
add_subdirectory(Format)
add_subdirectory(FrontendWrapper)
add_subdirectory(FrontendPasses)
add_subdirectory(Testing)
