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
enable_testing()

project(ocl_example)

set(APP "BufferTest")
set(KERNEL "BufferTest_genx")
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INSTALL_DIR "${CMAKE_BINARY_DIR}/bin" CACHE STRING "Test install path")

set(ALL_SOURCES
	${SRC_DIR}/main.cpp
	${SRC_DIR}/utils.cpp
	${SRC_DIR}/basic.cpp
 	${SRC_DIR}/cmdparser.cpp
	${SRC_DIR}/oclobject.cpp
)

set(KERNEL_SOURCE
	${SRC_DIR}/${KERNEL}.cpp
)

find_package(OpenCL REQUIRED)
find_program(CMC cmc PATHS $ENV{CSDK_IGC}/usr/bin NO_DEFAULT_PATH)

if(NOT CMC)
    message(FATAL_ERROR "Couldn't find cmc, make sure you setup environment with `source setenv.sh`")
endif()

set(CPU "SKL" CACHE STRING "Platform name")

add_custom_command(
	OUTPUT ${KERNEL}.spv
	COMMAND ${CMC} -emit-spirv -fcmocl -mcpu=${CPU} -o ${KERNEL}.spv -- ${KERNEL_SOURCE}
	DEPENDS ${KERNEL_SOURCE}
	COMMENT "Generate ${KERNEL}.spv"
)

add_custom_command(
	OUTPUT ${KERNEL}.bin
	COMMAND ${CMC} -fcmocl -mcpu=${CPU} -o ${KERNEL}.bin -- ${KERNEL}.spv
	DEPENDS ${KERNEL}.spv
	COMMENT "Generate ${KERNEL}.bin"
)

add_compile_options(-Wno-write-strings -Wno-deprecated-declarations -Wno-narrowing -Wno-format -Wno-format-extra-args)
add_link_options(-Wl,--disable-new-dtag)
add_definitions(-D__linux__=1 -D_LINUX=1 -DLINUX=1 -DCL_TARGET_OPENCL_VERSION=220)
include_directories($ENV{CSDK_IGC}/usr/include $ENV{CSDK_IGC}/opt/intel/opencl/include)

add_executable(${APP}_bin ${ALL_SOURCES} "${KERNEL}.bin")
target_compile_definitions(${APP}_bin PRIVATE BIN=1)
target_link_libraries(${APP}_bin OpenCL)
set_property(TARGET ${APP}_bin PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)


add_executable(${APP}_spv ${ALL_SOURCES} "${KERNEL}.spv")
target_compile_definitions(${APP}_spv PRIVATE SPV=1)
target_link_libraries(${APP}_spv OpenCL)
set_property(TARGET ${APP}_spv PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(${APP}_src ${ALL_SOURCES} )
target_compile_definitions(${APP}_src PRIVATE SRC=1)
target_link_libraries(${APP}_src OpenCL)
set_property(TARGET ${APP}_src PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

install(TARGETS ${APP}_bin ${APP}_src ${APP}_spv DESTINATION ${INSTALL_DIR})

install(FILES ${KERNEL_SOURCE} ${CMAKE_BINARY_DIR}/${KERNEL}.bin ${CMAKE_BINARY_DIR}/${KERNEL}.spv DESTINATION ${INSTALL_DIR})
