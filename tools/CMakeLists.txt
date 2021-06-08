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

create_subdirectory_options(CLANG TOOL)

add_clang_subdirectory(diagtool)

add_clang_subdirectory(driver)

add_clang_subdirectory(clang-diff)
add_clang_subdirectory(clang-format)
add_clang_subdirectory(clang-format-vs)
add_clang_subdirectory(clang-fuzzer)
add_clang_subdirectory(clang-import-test)
add_clang_subdirectory(clang-offload-bundler)

add_clang_subdirectory(c-index-test)

add_clang_subdirectory(clang-rename)
add_clang_subdirectory(clang-refactor)

if(CLANG_ENABLE_ARCMT)
  add_clang_subdirectory(arcmt-test)
  add_clang_subdirectory(c-arcmt-test)
endif()

if(CLANG_ENABLE_STATIC_ANALYZER)
  add_clang_subdirectory(clang-check)
  add_clang_subdirectory(clang-extdef-mapping)
  add_clang_subdirectory(scan-build)
  add_clang_subdirectory(scan-view)
endif()

# We support checking out the clang-tools-extra repository into the 'extra'
# subdirectory. It contains tools developed as part of the Clang/LLVM project
# on top of the Clang tooling platform. We keep them in a separate repository
# to keep the primary Clang repository small and focused.
# It also may be included by LLVM_EXTERNAL_CLANG_TOOLS_EXTRA_SOURCE_DIR.
add_llvm_external_project(clang-tools-extra extra)

# libclang may require clang-tidy in clang-tools-extra.
add_clang_subdirectory(libclang)

add_clang_subdirectory(cmoc)
