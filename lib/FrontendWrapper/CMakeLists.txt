#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
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
