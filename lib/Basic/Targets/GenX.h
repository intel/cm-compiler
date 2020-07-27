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
  bool OCLRuntime = false;

public:
  GenXTargetInfo(const llvm::Triple &Triple, unsigned PointerWidth);

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
                        .Case("ICL", true)
                        .Case("ICLLP", true)
                        .Case("TGLLP", true)
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
    // OCL runtime specific headers support
    if (OCLRuntime)
      Builder.defineMacro("__CM_OCL_RUNTIME");
  }

  virtual void getDefaultFeatures(llvm::StringMap<bool> &Features) const {}

  virtual bool handleTargetFeatures(std::vector<std::string> &Features,
                                    DiagnosticsEngine &Diags);

  virtual bool hasFeature(StringRef Feature) const {
    bool has = llvm::StringSwitch<bool>(Feature)
      .Case("longlong", (CPU != "ICLLP") && (CPU != "TGLLP"))
      .Case("double", (CPU != "ICLLP") && (CPU != "TGLLP"))
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

  // Convert 'cr' and 'rw' into '^cr' and '^rw'.
  // '^' is a hint for two-letter constraint
  std::string convertConstraint(const char *&Constraint) const override {
    if ((Constraint[0] == 'r' && Constraint[1] == 'w') ||
        (Constraint[0] == 'c' && Constraint[1] == 'r'))
      return std::string("^") + std::string(Constraint++, 2);
    return std::string(1, *Constraint);
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    default:
      return false;
    case 'c':
      Name++;
      switch (*Name) {
      default:
        return false;
      case 'r':
        Info.setAllowsRegister();
        return true;
      }
    case 'a':
      Info.setAllowsRegister();
      return true;
    case 'w':
      Info.setAllowsRegister();
      return true;
    }
  }
};

} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_GENX_H
