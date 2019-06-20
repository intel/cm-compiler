set(LLVM_LINK_COMPONENTS
  Analysis
  CodeGen
  Core
  GenXCodeGen
  GenXInfo
  IRReader
  MC
  ScalarOpts
  SPIRVLib
  Support
  Target
  TransformUtils
  )

add_llvm_library(igcmc SHARED
  igcmc.cpp
  igcmc.h
  )


if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(TARGET_MODIFIER "32")
else()
  set(TARGET_MODIFIER "64")
endif()

set_target_properties(igcmc PROPERTIES OUTPUT_NAME "igcmc${TARGET_MODIFIER}")
