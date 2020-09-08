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
  SHARED
  LINK_LIBS
    clangDriver
    clangFrontend
    clangFrontendTool
)

set_target_properties(clangFEWrapper PROPERTIES LINK_DEPENDS "${VERSION_SCRIPT}")
target_compile_definitions(clangFEWrapper PUBLIC INTEL_CM_CLANGFE_LIB)

if(LLVM_ON_UNIX)
  include("package.cmake")
endif()