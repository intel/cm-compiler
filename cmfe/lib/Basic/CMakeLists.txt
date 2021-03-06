set(LLVM_LINK_COMPONENTS
  Core
  MC
  Support
  GenXIntrinsics
  )

find_first_existing_vc_file(llvm_vc "${LLVM_MAIN_SRC_DIR}")
find_first_existing_vc_file(clang_vc "${CLANG_SOURCE_DIR}")

# The VC revision include that we want to generate.
set(version_inc "${CMAKE_CURRENT_BINARY_DIR}/SVNVersion.inc")

set(get_svn_script "${LLVM_CMAKE_PATH}/GetSVN.cmake")

if(DEFINED clang_vc)
  set(SRC_DIRS ${CLANG_SOURCE_DIR})
  set(SRC_NAMES SVN)
endif()

if(DEFINED llvm_vc)
  set(SRC_DIRS ${SRC_DIRS} ${LLVM_MAIN_SRC_DIR})
  set(SRC_NAMES ${SRC_NAMES} LLVM)
endIf()

if(DEFINED SRC_DIRS)
  # Create custom target to generate the VC revision include.
  add_custom_command(OUTPUT "${version_inc}"
    DEPENDS "${llvm_vc}" "${clang_vc}" "${get_svn_script}"
    COMMAND
    ${CMAKE_COMMAND} "-DSOURCE_DIRS=\"${SRC_DIRS}\""
                     "-DNAMES=\"${SRC_NAMES}\""
                     "-DHEADER_FILE=${version_inc}"
                     -P "${get_svn_script}")

  # Mark the generated header as being generated.
  set_source_files_properties("${version_inc}"
    PROPERTIES GENERATED TRUE
               HEADER_FILE_ONLY TRUE)

  # Tell Version.cpp that it needs to build with -DHAVE_SVN_VERSION_INC.
  set_source_files_properties(Version.cpp
    PROPERTIES COMPILE_DEFINITIONS "HAVE_SVN_VERSION_INC")
else()
  # Not producing a VC revision include.
  set(version_inc)

  # Being able to force-set the SVN revision in cases where it isn't available
  # is useful for performance tracking, and matches compatibility from autoconf.
  if(SVN_REVISION)
    set_source_files_properties(Version.cpp
      PROPERTIES COMPILE_DEFINITIONS "SVN_REVISION=\"${SVN_REVISION}\"")
  endif()
endif()

add_clang_library(clangBasic
  Attributes.cpp
  Builtins.cpp
  CharInfo.cpp
  CodeGenOptions.cpp
  Cuda.cpp
  Diagnostic.cpp
  DiagnosticIDs.cpp
  DiagnosticOptions.cpp
  FileManager.cpp
  FileSystemStatCache.cpp
  FixedPoint.cpp
  IdentifierTable.cpp
  LangOptions.cpp
  MemoryBufferCache.cpp
  Module.cpp
  ObjCRuntime.cpp
  OpenMPKinds.cpp
  OperatorPrecedence.cpp
  SanitizerBlacklist.cpp
  SanitizerSpecialCaseList.cpp
  Sanitizers.cpp
  SourceLocation.cpp
  SourceManager.cpp
  TargetInfo.cpp
  Targets.cpp
  Targets/AArch64.cpp
  Targets/AMDGPU.cpp
  Targets/ARC.cpp
  Targets/ARM.cpp
  Targets/AVR.cpp
  Targets/BPF.cpp
  Targets/GenX.cpp
  Targets/Hexagon.cpp
  Targets/Lanai.cpp
  Targets/Le64.cpp
  Targets/MSP430.cpp
  Targets/Mips.cpp
  Targets/NVPTX.cpp
  Targets/OSTargets.cpp
  Targets/PNaCl.cpp
  Targets/PPC.cpp
  Targets/RISCV.cpp
  Targets/SPIR.cpp
  Targets/Sparc.cpp
  Targets/SystemZ.cpp
  Targets/TCE.cpp
  Targets/WebAssembly.cpp
  Targets/X86.cpp
  Targets/XCore.cpp
  TokenKinds.cpp
  Version.cpp
  Warnings.cpp
  XRayInstr.cpp
  XRayLists.cpp
  ${version_inc}
  )

