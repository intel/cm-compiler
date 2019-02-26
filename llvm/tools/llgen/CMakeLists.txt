set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Analysis
  CodeGen
  Core
  IRReader
  MC
  ScalarOpts
  SPIRVLib
  Support
  Target
  TransformUtils
  )

add_llvm_tool(llgen
  llgen.cpp

  DEPENDS
  intrinsics_gen
  )
export_executable_symbols(llgen)
