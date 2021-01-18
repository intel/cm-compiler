cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(sample)

if(NOT DEFINED ENV{CSDK_IGC})
  message(FATAL_ERROR "CSDK_IGC environment variable is not set - did you run 'setenv.bat' ?")
endif()

# set paths
set(CSDK_IGC $ENV{CSDK_IGC})
set(CMEMU_PATH ${CSDK_IGC}/cmemu)

# locate all relevant packages
foreach(program CMOC OCLOC)
  string(TOLOWER ${program} binary)
  find_program(${program} ${binary} REQUIRED)
  if(NOT ${program})
    message(FATAL_ERROR "Unable to locate ${binary} executable - did you run 'setenv.bat' ?")
  else()
    message(INFO " using ${binary} from ${${program}}")
  endif()
endforeach(program)
find_library(LIB_CM_64 NAMES cm libcm PATHS ${CMEMU_PATH}/lib/x64)
find_library(LIB_SHIM_64 NAMES shim PATHS ${CMEMU_PATH}/lib/x64)
find_library(LIB_SHIM_L0_64 NAMES shim_l0 PATHS ${CMEMU_PATH}/lib/x64)
find_library(LIB_CMEMU_64 NAMES igfxcmrt_emu igfxcmrt64_emu PATHS ${CMEMU_PATH}/lib/x64)
find_library(LIB_OPENCL NAMES Intel_OpenCL_ICD64 PATHS ${CSDK_IGC}/runtime/opencl/lib)
find_library(LIB_LEVEL0 NAMES ze_loader PATHS ${CSDK_IGC}/runtime/level_zero/lib)

# our sources
set(KERNEL ${CMAKE_SOURCE_DIR}/kernel.cpp)
set(HOST_OCL ${CMAKE_SOURCE_DIR}/host.cpp)
set(HOST_L0  ${CMAKE_SOURCE_DIR}/host_l0.cpp)

# os-specific
if (CMAKE_HOST_SYSTEM_NAME MATCHES Windows)
  set(dll ${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
  set(dll)
endif()
set(INSTALL_DIR ${CMAKE_BINARY_DIR}/bin)


#######
# SHIM
add_library(kernel.shim MODULE ${KERNEL})
set_target_properties(kernel.shim PROPERTIES COMPILE_FLAGS "-DCMRT_EMU -DSHIM")
target_include_directories(kernel.shim PUBLIC ${CMEMU_PATH}/include/libcm ${CMEMU_PATH}/include/shim)
target_link_libraries(kernel.shim ${LIB_CM_64})
install(TARGETS kernel.shim DESTINATION ${INSTALL_DIR})

add_executable(vector.shim ${HOST_OCL})
set_target_properties(vector.shim PROPERTIES COMPILE_FLAGS -DKERNEL=\\\"kernel.shim${dll}\\\")
target_include_directories(vector.shim PUBLIC ${CMEMU_PATH}/include/libcm ${CSDK_IGC}/runtime/opencl/include)
target_link_libraries(vector.shim ${LIB_SHIM_64} ${LIB_CMEMU_64} ${LIB_CM_64})
add_dependencies(vector.shim kernel.shim)
install(TARGETS vector.shim DESTINATION ${INSTALL_DIR})


#######
# GPU
set(out ${CMAKE_BINARY_DIR}/kernel.spv.skl ${CMAKE_BINARY_DIR}/kernel.skl)
list(GET out 0 spirv)
list(GET out 1 binary)
string(REPLACE ${CMAKE_BINARY_DIR}/ "" kernel ${binary})
add_custom_command(OUTPUT ${out}
  COMMAND ${CMOC} -emit-spirv -fcmocl -mcpu=SKL -m64 ${KERNEL} -o ${spirv}
  COMMAND ${OCLOC} -device skl -output_no_suffix -options "-cmc" -spirv_input -file ${spirv} -output ${binary}
)
add_custom_target(kernel_gpu DEPENDS ${out})
install(FILES ${out} DESTINATION ${INSTALL_DIR})

add_executable(vector.skl ${HOST_OCL})
set_target_properties(vector.skl PROPERTIES COMPILE_FLAGS -DKERNEL=\\\"${kernel}\\\")
target_include_directories(vector.skl PUBLIC ${CSDK_IGC}/runtime/opencl/include)
target_link_libraries(vector.skl ${LIB_OPENCL})
add_dependencies(vector.skl kernel_gpu)
install(TARGETS vector.skl DESTINATION ${INSTALL_DIR})





#######
# SHIM / L0
add_library(kernel.l0.shim MODULE ${KERNEL})
set_target_properties(kernel.l0.shim PROPERTIES COMPILE_FLAGS "-DCMRT_EMU -DSHIM")
target_include_directories(kernel.l0.shim PUBLIC ${CMEMU_PATH}/include/shim ${CMEMU_PATH}/include/libcm)
target_link_libraries(kernel.l0.shim ${LIB_CM_64})
install(TARGETS kernel.l0.shim DESTINATION ${INSTALL_DIR})

add_executable(vector.l0.shim ${HOST_L0})
set_target_properties(vector.l0.shim PROPERTIES COMPILE_FLAGS -DKERNEL=\\\"kernel.l0.shim${dll}\\\")
target_include_directories(vector.l0.shim PUBLIC ${CMEMU_PATH}/include/shim ${CSDK_IGC}/runtime/level_zero/include)
target_link_libraries(vector.l0.shim ${LIB_SHIM_L0_64} ${LIB_CMEMU_64} ${LIB_CM_64})
add_dependencies(vector.l0.shim kernel.l0.shim)
install(TARGETS vector.l0.shim DESTINATION ${INSTALL_DIR})


#######
# GPU / L0
set(kernel kernel.spv.skl)
add_executable(vector.l0.skl ${HOST_L0})
set_target_properties(vector.l0.skl PROPERTIES COMPILE_FLAGS -DKERNEL=\\\"${kernel}\\\")
target_include_directories(vector.l0.skl PUBLIC ${CSDK_IGC}/runtime/level_zero/include)
target_link_libraries(vector.l0.skl ${LIB_LEVEL0})
add_dependencies(vector.l0.skl kernel_gpu)
install(TARGETS vector.l0.skl DESTINATION ${INSTALL_DIR})





# all targets to build
add_custom_target(build ALL)
add_dependencies(build vector.skl vector.l0.skl)
