#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2020-2021 Intel Corporation
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
add_subdirectory(Index)
if(CLANG_ENABLE_STATIC_ANALYZER)
  add_subdirectory(StaticAnalyzer)
endif()
add_subdirectory(Format)
add_subdirectory(FrontendWrapper)
