/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

// This file declares Intel GenX TargetInfo objects.

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_GENX_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_GENX_H

#include "OSTargets.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

#include <unordered_set>

namespace clang {
namespace targets {

// GenX
// Not all features of a target are required for GenX.
// We are primarily interested in being able to specify a particular gpu
// variant.
//
class LLVM_LIBRARY_VISIBILITY GenXTargetInfo : public TargetInfo {
public:
  GenXTargetInfo(const llvm::Triple &Triple, unsigned PointerWidth);

  /// \brief Flags for architecture specific defines.
  typedef enum {
    ArchDefineNone = 0,
    ArchDefineName = 1 << 0 // <name> is substituted for arch name.
  } ArchDefineTypes;

  bool setCPU(const std::string &Name) override;

  const std::string &getCPU() const override { return CPU; }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override {
    return ArrayRef<Builtin::Info>();
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  void adjustTargetOptions(const CodeGenOptions &CGOpts,
                           TargetOptions &TargetOpts) const override;
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;

  bool hasFeature(StringRef Feature) const override;

  const char *getClobbers() const override { return ""; }

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

  const char *getBFloat16Mangling() const override { return "u6__bf16"; };

private:
  void setCPUProperties();

  static const Builtin::Info BuiltinInfo[];

  uint32_t Major;
  uint32_t Minor;
  uint32_t Revision;

  std::string CPU;

  bool HasFP64 = false;
  bool HasSLMCasInt64 = false;

  bool HasDpas;
  bool HasDpasw;
  bool HasDpasFp16;
  bool HasDpasBf16;
  bool HasDpasTf32;

  unsigned GrfWidth = 256;
  std::unordered_set<unsigned> SupportedGrfNums;

  unsigned MaxSLMSize = 64;

  // FIXME: get rid of this stuff
  bool HasIEFByPass = false;
  unsigned MaxOWordBlock = 8;
};

} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_GENX_H
