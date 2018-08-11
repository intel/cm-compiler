//===--- GenX.h - Declare GenX target feature support ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares Intel GenX TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_GENX_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_GENX_H

#include "OSTargets.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace clang {
namespace targets {

// GenX
// Not all features of a target are required for GenX.
// We are primarily interested in being able to specify a particular gpu
// variant.
//
class LLVM_LIBRARY_VISIBILITY GenXTargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];
  std::string CPU;

public:
  GenXTargetInfo(const llvm::Triple &Triple) : TargetInfo(Triple) {
    // There's another copy of this in llvm/lib/Target/GenX/GenXTargetMachine.h
    resetDataLayout("e-p:32:32-i64:64-n8:16:32");
    // GenX target always uses Itanium C++ ABI.
    TheCXXABI.set(TargetCXXABI::GenericItanium);
  }

  /// \brief Flags for architecture specific defines.
  typedef enum {
    ArchDefineNone = 0,
    ArchDefineName = 1 << 0 // <name> is substituted for arch name.
  } ArchDefineTypes;

  virtual bool setCPU(const std::string &Name) {
    bool CPUKnown = llvm::StringSwitch<bool>(Name)
                        .Case("HSW", true)
                        .Case("BDW", true)
                        .Case("CHV", true)
                        .Case("SKL", true)
                        .Case("BXT", true)
                        .Case("GLK", true)
                        .Case("KBL", true)
                        .Case("CNL", true)
                        .Case("ICL", true)
                        .Case("ICLLP", true)
                        .Default(false);

    if (CPUKnown)
      CPU = Name;

    return CPUKnown;
  }

  virtual const std::string &getCPU() const { return CPU; }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override {
    return ArrayRef<Builtin::Info>();
  }

  virtual BuiltinVaListKind getBuiltinVaListKind() const {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  virtual void getTargetDefines(const LangOptions &Opts,
                                MacroBuilder &Builder) const {
    Builder.defineMacro("_WIN32");
    Builder.defineMacro("_WINNT");
    Builder.defineMacro("_X86_");
    Builder.defineMacro("__MSVCRT__");
    Builder.defineMacro("_HAS_EXCEPTIONS", "0");
  }

  virtual void getDefaultFeatures(llvm::StringMap<bool> &Features) const {}

  virtual bool handleTargetFeatures(std::vector<std::string> &Features,
                                    DiagnosticsEngine &Diags);

  virtual bool hasFeature(StringRef Feature) const {
    bool has = llvm::StringSwitch<bool>(Feature)
      .Case("longlong", (CPU != "ICLLP"))
      .Case("double", (CPU != "ICLLP"))
      .Default(true);
    return has;
  }

  virtual const char *getClobbers() const { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override {
    return ArrayRef<const char *>();
  }

  ArrayRef<GCCRegAlias> getGCCRegAliases() const override {
    return ArrayRef<GCCRegAlias>();
  }

  virtual bool validateAsmConstraint(const char *&Name,
                                     TargetInfo::ConstraintInfo &Info) const {
    return false;
  }
};

} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_GENX_H
