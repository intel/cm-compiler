#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  InstCombine
  Option
  Support
  )

add_clang_executable(clang-check
  ClangCheck.cpp
  )

clang_target_link_libraries(clang-check
  PRIVATE
  clangAST
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangSerialization
  clangStaticAnalyzerFrontend
  clangTooling
  )

install(TARGETS clang-check
  RUNTIME DESTINATION bin)
