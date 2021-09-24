#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(l0_example)

if(NOT DEFINED ENV{CSDK_DIR})
	message(FATAL_ERROR "CSDK_DIR environment variable is not set - did you run 'source setupenv.sh' ?")
endif()

set(APP "Sgemm")
set(KERNEL "Sgemm_genx")
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INSTALL_DIR "${CMAKE_BINARY_DIR}/bin" CACHE STRING "Test install path")

set(ALL_SOURCES ${SRC_DIR}/Sgemm.cpp)
set(KERNEL_SOURCE ${SRC_DIR}/${KERNEL}.cpp)
file(COPY
	${SRC_DIR}/${KERNEL}.cpp
	${SRC_DIR}/share.h
	DESTINATION ./
)

find_program(CMC cmc PATHS $ENV{CSDK_DIR}/usr/bin NO_DEFAULT_PATH)
if(NOT CMC)
    message(FATAL_ERROR "Couldn't find cmoc, make sure you setup environment with `source setupenv.sh`")
endif()

find_library(LIBOCLOC ocloc PATHS $ENV{CSDK_DIR}/usr/local/lib/ NO_DEFAULT_PATH REQUIRED)
find_library(ZE_LOADER ze_loader PATHS $ENV{CSDK_DIR}/usr/lib/x86_64-linux-gnu NO_DEFAULT_PATH REQUIRED)


add_custom_command(
	OUTPUT ${KERNEL}.bin
	COMMAND ${CMC} -fcmocl -mcpu=${CPU} -o ${KERNEL}.bin --  ${KERNEL_SOURCE}
	DEPENDS ${KERNEL_SOURCE}
	COMMENT "Generate ${KERNEL}.bin"
)

set(CPU "SKL" CACHE STRING "Platform name")

include_directories($ENV{CSDK_DIR}/usr/include $ENV{CSDK_DIR}/usr/local/include)
link_directories($ENV{CSDK_DIR}/usr/lib/x86_64-linux-gnu)

add_executable(${APP}_src ${ALL_SOURCES} )
target_compile_definitions(${APP}_src PRIVATE BINNAME="${KERNEL}.cpp" BINDIR="None" L0 CM_COMPILE_ONLINE)
target_link_libraries(${APP}_src ${LIBOCLOC} ${ZE_LOADER})
set_property(TARGET ${APP}_src PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(${APP}_bin ${ALL_SOURCES} "${KERNEL}.bin" )
target_compile_definitions(${APP}_bin PRIVATE BINNAME="${KERNEL}.bin" BINDIR="None" L0 CM_COMPILE_OFFLINE)
target_link_libraries(${APP}_bin ${ZE_LOADER})
set_property(TARGET ${APP}_bin PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

install(TARGETS ${APP}_bin ${APP}_src DESTINATION ${INSTALL_DIR})

install(FILES ${KERNEL_SOURCE} ${SRC_DIR}/share.h ${CMAKE_BINARY_DIR}/${KERNEL}.bin DESTINATION ${INSTALL_DIR})

