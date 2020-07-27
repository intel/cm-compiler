cmake_minimum_required(VERSION 3.12)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
  project(ClangPost)

  get_filename_component(LLVM_DIR ${LLVM_DIR} ABSOLUTE BASE_DIR ${CMAKE_BINARY_DIR})
  find_package(LLVM REQUIRED)
  message(STATUS "found LLVM: ${LLVM_DIR}")

  set(LLVM_TOOLS_BINARY_DIR ${TOOLS_BINARY_DIR} CACHE PATH "Path to llvm/bin")
  set(LLVM_LIBRARY_DIR ${LIBRARY_DIR} CACHE PATH "Path to llvm/lib")
  set(LLVM_MAIN_INCLUDE_DIR ${INCLUDE_DIR} CACHE PATH "Path to llvm/include")
  set(LLVM_BINARY_DIR ${LLVM_OBJ_ROOT} CACHE PATH "Path to LLVM build tree")
  set(LLVM_MAIN_SRC_DIR ${MAIN_SRC_DIR} CACHE PATH "Path to LLVM source tree")
  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${LLVM_LIBDIR_SUFFIX})

  # this required in order tools install to work
  set(CLANG_BUILD_TOOLS ON)

  get_filename_component(Clang_DIR ${Clang_DIR} ABSOLUTE BASE_DIR ${CMAKE_BINARY_DIR})
  find_package(Clang REQUIRED)
  message(STATUS "found clang: ${Clang_DIR}")

  include_directories("${Clang_DIR}/../../../include")
  include_directories("${LLVM_DIR}/../../../include")
  link_directories("${LLVM_LIBRARY_DIR}")
  
  # dependency on intrinsics and spirv
  get_filename_component(INTR_CMAKE_PATH ${LLVMGenXIntrinsics_DIR} ABSOLUTE BASE_DIR ${CMAKE_BINARY_DIR})

  # Add dependency of genx intrinsics
  find_package(LLVMGenXIntrinsics REQUIRED PATHS ${INTR_CMAKE_PATH} NO_DEFAULT_PATH)
  message(STATUS "found LLVM GenX Intrinsics: ${LLVMGenXIntrinsics_DIR}")
  include_directories("${LLVMGenXIntrinsics_DIR}/../../../include")
  link_directories("${LLVMGenXIntrinsics_DIR}/../../../lib")

  # temporary SPIRV path
  include_directories("${LLVMGenXIntrinsics_DIR}/../../../include/LLVMSPIRVLib")

  # dependency of codegen on visa libraries.
  if( NOT DEFINED VISA_INSTALL_DIR )
    message(FATAL_ERROR "Path to vISA libraries is not defined, use -DVISA_INSTALL_DIR to specify")
  endif()
  get_filename_component(VISA_INSTALL_DIR ${VISA_INSTALL_DIR} ABSOLUTE BASE_DIR ${CMAKE_BINARY_DIR})

  # Determine vISA modifier.
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(GENX_TARGET_MODIFIER "32")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(GENX_TARGET_MODIFIER "64")
  else()
    message(FATAL_ERROR "unexpected platform")
  endif()

  # Import target for every vISA library.
  set(FULL_VISA_LIBS igfxcmjit iga_s iga_enc)
  list(TRANSFORM FULL_VISA_LIBS APPEND ${GENX_TARGET_MODIFIER})

  # Account for non-prefixed igfxcmjit under linux.
  list(APPEND CMAKE_FIND_LIBRARY_PREFIXES "")
  foreach(VISA_LIB ${FULL_VISA_LIBS})
    unset(VISA_LIB_PATH CACHE)
    find_library(VISA_LIB_PATH ${VISA_LIB} PATHS ${VISA_INSTALL_DIR}/lib NO_DEFAULT_PATH)
    add_library(${VISA_LIB} STATIC IMPORTED)
    set_property(TARGET ${VISA_LIB} PROPERTY IMPORTED_LOCATION ${VISA_LIB_PATH})
  endforeach()

  # Introduce codegen as imported target which depends on vISA libraries.
  # This ensures that anyone linking with codegen also links with vISA.
  get_filename_component(VC_OPTS_LIBDIR ${VC_OPTS_LIBDIR} ABSOLUTE BASE_DIR ${CMAKE_BINARY_DIR})
  find_library(GENX_CODEGEN_PATH LLVMGenXCodeGen PATHS ${LLVM_LIBRARY_DIR} ${VC_OPTS_LIBDIR} NO_DEFAULT_PATH)
  add_library(LLVMGenXCodeGen STATIC IMPORTED)
  set_property(TARGET LLVMGenXCodeGen PROPERTY IMPORTED_LOCATION ${GENX_CODEGEN_PATH})
  set_property(TARGET LLVMGenXCodeGen PROPERTY INTERFACE_LINK_LIBRARIES LLVMGenXOpts ${FULL_VISA_LIBS})

  # Introduce GenXInfo as imported target just like CodeGen
  find_library(GENX_INFO_PATH LLVMGenXInfo PATHS ${LLVM_LIBRARY_DIR} ${VC_OPTS_LIBDIR} NO_DEFAULT_PATH)
  add_library(LLVMGenXInfo STATIC IMPORTED)
  set_property(TARGET LLVMGenXInfo PROPERTY IMPORTED_LOCATION ${GENX_INFO_PATH})

  if ( VC_OPTS_LIBDIR )      
    include_directories("${VC_OPTS_LIBDIR}/../include")
    link_directories("${VC_OPTS_LIBDIR}")
  else()
    message(FATAL_ERROR "VC_OPTS_LIBDIR required")
  endif()  

  set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${LLVM_CMAKE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules
  )

  include(AddLLVM)
  include(TableGen)
  include(HandleLLVMOptions)
  include(VersionFromVCS)
  include(CMakeParseArguments)
  include(AddClang)
  
  # fake target for clang-headers
  # this one is not exported from clang (headers are installed, though)  
  add_custom_target(clang-headers)  

  # We need version information for driver build
  if(NOT DEFINED CLANG_VERSION_MAJOR)
    set(CLANG_VERSION_MAJOR ${LLVM_VERSION_MAJOR})
  endif()
  if(NOT DEFINED CLANG_VERSION_MINOR)
    set(CLANG_VERSION_MINOR ${LLVM_VERSION_MINOR})
  endif()
  if(NOT DEFINED CLANG_VERSION_PATCHLEVEL)
    set(CLANG_VERSION_PATCHLEVEL ${LLVM_VERSION_PATCH})
  endif()
  set(CLANG_VERSION "${CLANG_VERSION_MAJOR}.${CLANG_VERSION_MINOR}.${CLANG_VERSION_PATCHLEVEL}")
  message(STATUS "Clang version: ${CLANG_VERSION}")

  # Clang version information
  set(CLANG_EXECUTABLE_VERSION
      "${CLANG_VERSION_MAJOR}" CACHE STRING
      "Major version number that will be appended to the clang executable name")

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tools/driver ${CMAKE_BINARY_DIR}/prebuilts/driver)
else()
  message(FATAL_ERROR "Only standalone with prebuilts is supported")
endif()
