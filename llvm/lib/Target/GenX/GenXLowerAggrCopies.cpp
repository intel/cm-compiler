//===- GenXLowerAggrCopies.cpp - ------------------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// \file
// Lower aggregate copies, memset, memcpy, memmov intrinsics into loops when
// the size is large or is not a compile-time constant.
//
//===----------------------------------------------------------------------===//

#include "GenXLowerAggrCopies.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LowerMemIntrinsics.h"

#define DEBUG_TYPE "GENX_LOWERAGGRCOPIES"

using namespace llvm;

namespace {

// actual analysis class, which is a functionpass
struct GenXLowerAggrCopies : public FunctionPass {
  static char ID;

  GenXLowerAggrCopies() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<StackProtector>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }

  bool runOnFunction(Function &F) override;

  static const unsigned MaxAggrCopySize = 1; //128;

  StringRef getPassName() const override {
    return "Lower aggregate copies/intrinsics into loops";
  }
};

char GenXLowerAggrCopies::ID = 0;

bool GenXLowerAggrCopies::runOnFunction(Function &F) {
  SmallVector<MemIntrinsic *, 4> MemCalls;

  const TargetTransformInfo &TTI =
      getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

  // Collect all aggregate loads and mem* calls.
  for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
    for (BasicBlock::iterator II = BI->begin(), IE = BI->end(); II != IE;
         ++II) {
      if (MemIntrinsic *IntrCall = dyn_cast<MemIntrinsic>(II)) {
        // Convert intrinsic calls with variable size or with constant size
        // larger than the MaxAggrCopySize threshold.
        if (ConstantInt *LenCI = dyn_cast<ConstantInt>(IntrCall->getLength())) {
          if (LenCI->getZExtValue() >= MaxAggrCopySize) {
            MemCalls.push_back(IntrCall);
          }
        } else {
          MemCalls.push_back(IntrCall);
        }
      }
    }
  }

  if (MemCalls.size() == 0) {
    return false;
  }

  // Transform mem* intrinsic calls.
  for (MemIntrinsic *MemCall : MemCalls) {
    if (MemCpyInst *Memcpy = dyn_cast<MemCpyInst>(MemCall)) {
      expandMemCpyAsLoop(Memcpy, TTI);
    } else if (MemMoveInst *Memmove = dyn_cast<MemMoveInst>(MemCall)) {
      expandMemMoveAsLoop(Memmove);
    } else if (MemSetInst *Memset = dyn_cast<MemSetInst>(MemCall)) {
      expandMemSetAsLoop(Memset);
    }
    MemCall->eraseFromParent();
  }

  return true;
}

} // namespace

namespace llvm {
void initializeGenXLowerAggrCopiesPass(PassRegistry &);
}

INITIALIZE_PASS(GenXLowerAggrCopies, "genx-lower-aggr-copies",
                "Lower aggregate copies, and llvm.mem* intrinsics into loops",
                false, false)

FunctionPass *llvm::createGenXLowerAggrCopiesPass() {
  return new GenXLowerAggrCopies();
}
