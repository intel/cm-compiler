/*
 * Copyright (c) 2018, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

//===----------------------------------------------------------------------===//
//
/// GenXIMadLegalization
/// --------------------
///
/// This pass performs the legalization on integer mad to ensure additive
/// operand is alway single-used so that it could be mapped to accumulator
/// register.
///
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "GENX_IMAD_POST_LEGALIZATION"

#include "GenX.h"
#include "GenXBaling.h"
#include "GenXModule.h"
#include "GenXRegion.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace genx;

namespace {

class GenXIMadPostLegalization : public FunctionPass {
  DominatorTree *DT;
public:
  static char ID;

  explicit GenXIMadPostLegalization() : FunctionPass(ID), DT(nullptr) {}

  const char *getPassName() const override {
    return "GenX IMAD post-legalization pass";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<GenXModule>();
  }

  bool runOnFunction(Function &F) override;
};

} // end anonymous namespace

char GenXIMadPostLegalization::ID = 0;

namespace llvm {
void initializeGenXIMadPostLegalizationPass(PassRegistry &);
}

INITIALIZE_PASS_BEGIN(GenXIMadPostLegalization, "GenXIMadLegalization", "GenXIMadLegalization", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeGroupWrapperPass)
INITIALIZE_PASS_END(GenXIMadPostLegalization, "GenXIMadLegalization", "GenXIMadLegalization", false, false)

FunctionPass *llvm::createGenXIMadPostLegalizationPass() {
  initializeGenXIMadPostLegalizationPass(*PassRegistry::getPassRegistry());
  return new GenXIMadPostLegalization();
}

static bool isIntegerMadIntrinsic(Value *V) {
  switch (getIntrinsicID(V)) {
  default: break;
  case Intrinsic::genx_ssmad:
  case Intrinsic::genx_sumad:
  case Intrinsic::genx_usmad:
  case Intrinsic::genx_uumad:
  case Intrinsic::genx_ssmad_sat:
  case Intrinsic::genx_sumad_sat:
  case Intrinsic::genx_usmad_sat:
  case Intrinsic::genx_uumad_sat:
    return true;
  }
  return false;
}

static bool isIntegerMulIntrinsic(Value *V) {
  switch (getIntrinsicID(V)) {
  default: break;
  case Intrinsic::genx_ssmul:
  case Intrinsic::genx_sumul:
  case Intrinsic::genx_usmul:
  case Intrinsic::genx_uumul:
    return true;
  }
  return false;
}

static std::tuple<BasicBlock *, Instruction *>
findNearestInsertPt(DominatorTree *DT, ArrayRef<Instruction *> Users) {
  DenseMap<BasicBlock *, Instruction *> BBs;
  for (auto U : Users) {
    auto UseBB = U->getParent();
    auto MI = BBs.end();
    bool New = false;
    std::tie(MI, New) = BBs.insert(std::make_pair(UseBB, U));
    if (New)
      continue;
    // Find the earliest user if more than one users are in the same block.
    auto BI = UseBB->begin();
    for (; &*BI != U && &*BI != MI->second; ++BI)
      /* EMPTY */;
    MI->second = &*BI;
  }

  assert(BBs.size() != 0 && "At least one BB should be found!");

  auto MI = BBs.begin();
  if (BBs.size() == 1)
    return std::make_tuple(MI->first, MI->second);

  auto BB = MI->first;
  auto ME = BBs.end();
  for (++MI; MI != ME; ++MI)
    BB = DT->findNearestCommonDominator(BB, MI->first);

  MI = BBs.find(BB);
  if (MI != BBs.end())
    return std::make_tuple(MI->first, MI->second);

  return std::make_tuple(BB, nullptr);
}

bool GenXIMadPostLegalization::runOnFunction(Function &F) {
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  bool Changed = false;

  // After this point, we should not do constant folding.
  Changed |= breakConstantExprs(&F);

  SmallVector<Instruction *, 16> Deads;
  for (auto &BB : F) {
    for (auto BI = BB.begin(), BE = BB.end(); BI != BE; /* EMPTY */) {
      Instruction *I = &*BI++;
      if (!isIntegerMadIntrinsic(I))
        continue;
      auto II = cast<IntrinsicInst>(I);
      // Check src2 and duplicate if necessary.
      Value *S2 = II->getOperand(2);
      if (S2->hasOneUse()) {
        // Sink S2 closer to user to shorten acc live ranges.
        // This is particular important when 32 bit integer multiplications
        // are not native and acc registers will be used to emulate them.
        auto I2 = dyn_cast<Instruction>(S2);
        if (I2 == nullptr || I2->getParent() != I->getParent())
          continue;
        if (I2->mayHaveSideEffects() || isa<PHINode>(I2) ||
            I2->getNextNode() == I)
          continue;
        I2->moveBefore(I);
        Changed = true;
        continue;
      }
      // Only duplicate on selective instructions.
      if (!isRdRegion(S2) && !isIntegerMulIntrinsic(S2))
        continue;
      Instruction *RII = cast<Instruction>(S2);
      SmallVector<Instruction *, 16> Others;
      for (auto UI = S2->use_begin(),
                UE = S2->use_end(); UI != UE; /* EMPTY */) {
        Use &U = *UI++;
        auto InsertPt = cast<Instruction>(U.getUser());
        if (!isIntegerMadIntrinsic(InsertPt) || U.getOperandNo() != 2) {
          Others.push_back(InsertPt);
          continue;
        }
        auto NewInst = RII->clone();
        NewInst->setName(RII->getName() + ".postimad");
        NewInst->insertBefore(InsertPt);
        U.set(NewInst);
      }
      if (!Others.empty()) {
        // Find a new place for RII.
        BasicBlock *NBB = nullptr;
        Instruction *Pt = nullptr;
        std::tie(NBB, Pt) = findNearestInsertPt(DT, Others);
        Pt = Pt ? Pt : NBB->getTerminator();
        RII->moveBefore(Pt);
      } else
        Deads.push_back(RII);
      Changed = true;
    }
  }
  for (auto I : Deads)
    I->eraseFromParent();

  return Changed;
}