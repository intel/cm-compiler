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
