#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

# Variables to manipulate packages:
# FEWRAPPER_MAJOR, FEWRAPPER_MINOR, FEWRAPPER_PATCH - version provided in 3 separate values
# FEWRAPPER_VERSION - version provided in form 1.2.34

if(NOT (DEFINED FEWRAPPER_MAJOR AND DEFINED FEWRAPPER_MINOR AND DEFINED FEWRAPPER_PATCH) )
  if(DEFINED FEWRAPPER_VERSION)
    set(GIT_TAG "cmclang-${FEWRAPPER_VERSION}-0-0000000")
  else()
    find_package(Git)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} describe --long --tags
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_TAG
      RESULT_VARIABLE EXIT_STATUS
      ERROR_VARIABLE GET_TAG_ERROR
    )
    if(NOT EXIT_STATUS EQUAL 0)
      message(WARNING "[CMFE] Could not get package revision: ${GET_TAG_ERROR}")
      set(GIT_TAG "cmclang-1.0.1-0-00000000")
    endif()
  endif()

  string(REGEX MATCH "^cmclang-([0-9]+)\.([0-9]+)\.([0-9]+)-([0-9]+)-" FEVERSION "${GIT_TAG}")
  if(CMAKE_MATCH_COUNT EQUAL 4)
    set(FEWRAPPER_MAJOR ${CMAKE_MATCH_1})
    set(FEWRAPPER_MINOR ${CMAKE_MATCH_2})
    set(BASE_PATCH ${CMAKE_MATCH_3})
    set(ADD_PATCH ${CMAKE_MATCH_4})
    math(EXPR FEWRAPPER_PATCH "${BASE_PATCH} + ${ADD_PATCH}")
  else()
    message(WARNING "[CMFE] Could not understand package version: ${GIT_TAG}")
    set(FEWRAPPER_MAJOR 1)
    set(FEWRAPPER_MINOR 0)
    set(FEWRAPPER_PATCH 1)
  endif()
endif()
message("[CMFE] Detected CMFE version ${FEWRAPPER_MAJOR}.${FEWRAPPER_MINOR}.${FEWRAPPER_PATCH}")
