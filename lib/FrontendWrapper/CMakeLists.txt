#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
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

set(VERSION_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/exported_symbols.ld")

if (UNIX)
    # on Linux we run version script to export only the symbols we need
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--version-script=${VERSION_SCRIPT}")
endif()

set(LLVM_LINK_COMPONENTS
    Support
)
add_clang_library(clangFEWrapper
    FrontendWrapper.cpp
    HeaderStorage.c
  SHARED
  LINK_LIBS
    clangDriver
    clangFrontend
    clangFrontendTool
)

target_link_libraries(clangFEWrapper
  PRIVATE
  CmEmbeddedHeaders
  )
set_target_properties(clangFEWrapper PROPERTIES LINK_DEPENDS "${VERSION_SCRIPT}")
target_compile_definitions(clangFEWrapper PUBLIC INTEL_CM_CLANGFE_LIB)

# Helper interface target for interaction with created module.
add_library(CMFrontendWrapper INTERFACE)
target_include_directories(CMFrontendWrapper
  INTERFACE
  "${CLANG_SOURCE_DIR}/include/clang/FrontendWrapper"
  )
target_compile_definitions(CMFrontendWrapper
  INTERFACE
  "CMFE_WRAPPER_NAME=\"$<TARGET_FILE_NAME:clangFEWrapper>\""
  )
add_dependencies(CMFrontendWrapper clangFEWrapper)

if(LLVM_ON_UNIX)
  include("package.cmake")
endif()
