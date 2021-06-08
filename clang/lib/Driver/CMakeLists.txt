#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2018-2021 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT
#============================ end_copyright_notice =============================

set(LLVM_LINK_COMPONENTS
  BinaryFormat
  Option
  Support
  )

if(WIN32)
  # MSVCToolChain.cpp uses version.dll.
  set(system_libs version)
endif()

add_clang_library(clangDriver
  Action.cpp
  Compilation.cpp
  DarwinSDKInfo.cpp
  Distro.cpp
  Driver.cpp
  DriverOptions.cpp
  Job.cpp
  Multilib.cpp
  Phases.cpp
  SanitizerArgs.cpp
  Tool.cpp
  ToolChain.cpp
  ToolChains/Arch/AArch64.cpp
  ToolChains/Arch/ARM.cpp
  ToolChains/Arch/GenX.cpp
  ToolChains/Arch/Mips.cpp
  ToolChains/Arch/PPC.cpp
  ToolChains/Arch/RISCV.cpp
  ToolChains/Arch/Sparc.cpp
  ToolChains/Arch/SystemZ.cpp
  ToolChains/Arch/X86.cpp
  ToolChains/Ananas.cpp
  ToolChains/AMDGPU.cpp
  ToolChains/AVR.cpp
  ToolChains/BareMetal.cpp
  ToolChains/Clang.cpp
  ToolChains/CloudABI.cpp
  ToolChains/CommonArgs.cpp
  ToolChains/Contiki.cpp
  ToolChains/CrossWindows.cpp
  ToolChains/Cuda.cpp
  ToolChains/Darwin.cpp
  ToolChains/DragonFly.cpp
  ToolChains/FreeBSD.cpp
  ToolChains/Fuchsia.cpp
  ToolChains/GenX.cpp
  ToolChains/Gnu.cpp
  ToolChains/Haiku.cpp
  ToolChains/HIP.cpp
  ToolChains/Hexagon.cpp
  ToolChains/Hurd.cpp
  ToolChains/Linux.cpp
  ToolChains/MipsLinux.cpp
  ToolChains/MinGW.cpp
  ToolChains/Minix.cpp
  ToolChains/MSP430.cpp
  ToolChains/MSVC.cpp
  ToolChains/Myriad.cpp
  ToolChains/NaCl.cpp
  ToolChains/NetBSD.cpp
  ToolChains/OpenBSD.cpp
  ToolChains/PS4CPU.cpp
  ToolChains/RISCVToolchain.cpp
  ToolChains/Solaris.cpp
  ToolChains/TCE.cpp
  ToolChains/WebAssembly.cpp
  ToolChains/XCore.cpp
  Types.cpp
  XRayArgs.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  ${system_libs}
  )
