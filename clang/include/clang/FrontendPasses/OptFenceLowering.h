/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "llvm/IR/PassManager.h"

namespace llvm {

class ModulePass;
class PassRegistry;

//-----------------------------------------------------------------------------
// New PM support
//-----------------------------------------------------------------------------
// Optimizations Fence Lowering pass for new PM.
class OptFenceLowering final : public PassInfoMixin<OptFenceLowering> {
public:
  OptFenceLowering() = default;

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static StringRef getArgString() { return "OptFenceLowering"; }
};
//-----------------------------------------------------------------------------
// Legacy PM support
//-----------------------------------------------------------------------------
void initializeOptFenceLoweringLegacyPass(PassRegistry &);

ModulePass *createOptFenceLoweringPass();
} // namespace llvm
