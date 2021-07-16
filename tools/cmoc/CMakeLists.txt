#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2020-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

set(CMC_TOOL_NAME cmc)

set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_tool(${CMC_TOOL_NAME}
  cmoc.cpp
  Backend.cpp
  )

find_path(OCLOC_API_HEADER ocloc_api.h)
if(OCLOC_API_HEADER)
  get_filename_component(OCLOC_API_INCLUDE_DIR
    "${OCLOC_API_HEADER}"
    DIRECTORY
    )

  target_include_directories(${CMC_TOOL_NAME}
    PRIVATE
    ${OCLOC_API_INCLUDE_DIR}
    )

  target_compile_definitions(${CMC_TOOL_NAME}
    PRIVATE
    "USE_OCLOC_API_HEADER"
    )
endif()

if(WIN32 OR CYGWIN)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(LIBOCLOC "ocloc64.dll")
  else()
    set(LIBOCLOC "ocloc32.dll")
  endif()
else()
  set(LIBOCLOC "libocloc.so")
endif()

target_compile_definitions(${CMC_TOOL_NAME}
  PRIVATE
  "LIBOCLOC_NAME=\"${LIBOCLOC}\""
  )

target_link_libraries(${CMC_TOOL_NAME}
  PRIVATE
  CMFrontendWrapper
  )
