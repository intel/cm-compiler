//===--- GenX.h - GenX ToolChain Implementations ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_GENX_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_GENX_H

#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {

namespace tools {
namespace GenX {
// For GenX, we do not need to instantiate tools for PreProcess, PreCompile and
// Compile. We simply use "clang -cc1" for those actions.
class LLVM_LIBRARY_VISIBILITY Assemble : public Tool {
public:
  Assemble(const ToolChain &TC) : Tool("GenX::Assemble", "Gen Finalizer", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
  mutable std::string FinalizerExecutable;
};

} // end namespace GenX
} // end namespace tools

namespace toolchains {
class LLVM_LIBRARY_VISIBILITY GenX : public ToolChain {
public:
  GenX(const Driver &D, const llvm::Triple &Triple,
       const llvm::opt::ArgList &Args);

protected:
  Tool *buildAssembler() const override;

public:
  void
  AddClangSystemIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                            llvm::opt::ArgStringList &CC1Args) const override;
  bool isPICDefault() const override;
  bool isPIEDefault() const override;
  bool isPICDefaultForced() const override;
};
} // end namespace toolchains

} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_GENX_H
