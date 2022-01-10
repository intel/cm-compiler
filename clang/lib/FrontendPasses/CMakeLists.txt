#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================


# In a standard Clang+LLVM build, we need to generate intrinsics before
# building codegen. In a standalone build, LLVM is already built and we don't
# need this dependency. Furthermore, LLVM doesn't export it so we can't have
# this dependency.
set(frontendpasses_deps intrinsics_gen)
if (CLANG_BUILT_STANDALONE)
  set(frontendpasses_deps)
endif()


set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Core
  Support
  Instrumentation
  Passes
  TransformUtils
  )

add_clang_library(clangPasses
  OptFenceLowering.cpp

  DEPENDS
  ${frontendpasses_deps}
  )
