/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//
/// OptFenceLowering
/// ----------------
/// Converts instructions marked as an optimization barrier to the optimization
/// barrier itself.
///
/// The barrier looks like this:
/// ...
/// CMOptimizationsFence fence;
/// ...
///
///
/// The main idea of lovering is that a function break is made at the place of
/// the barrier instruction and as far as possible the code below the
/// instruction is transferred to a new function. The barrier is replaced by
/// calling a new function.
///===---------------------------------------------------------------------===//
//

#include "clang/FrontendPasses/OptFenceLowering.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <unordered_set>

#define DEBUG_TYPE "optfencelowering"

using namespace llvm;

constexpr char SBeginIdentifier[] = "__cm_optfence_begin__";
constexpr char SEndIdentifier[] = "__cm_optfence_end__";
constexpr char SNewFunctionIdentifier[] = "__cm_optfence__";

// Implements the logic of the optimization barrier.
class OptFenceLower final : public InstVisitor<OptFenceLower> {
  template <typename T> using WorkListTy = std::vector<std::pair<T, T>>;
  using WorkListCITy = WorkListTy<CallInst *>;
  using WorkListBBTy = WorkListTy<BasicBlock *>;

  WorkListCITy WorkList;

public:
  OptFenceLower() = default;

  bool run(Module &M);

public:
  void visitCallInst(CallInst &CI);

private:
  WorkListBBTy getSplitedBBs(const WorkListCITy &WL);
  void clearIR(Module &M, const WorkListCITy &WL);
  std::vector<Function *> getExtractedFunctions(const WorkListBBTy &WL);
  void setNoInlineForFenceRegion(const std::vector<Function *> &Fs);
}; // class OptFenceLower

std::vector<BasicBlock *> getAreaToExtract(BasicBlock *BBBegin,
                                           BasicBlock *BBEnd);

bool OptFenceLower::run(Module &M) {
  LLVM_DEBUG(dbgs() << "[OFL]: Start pass in module: " << M.getName() << "\n");

  visit(M);
  LLVM_DEBUG(dbgs() << "[OFL]: found " << WorkList.size()
                    << " fences in module.\n");
  if (WorkList.empty())
    return false;

  auto&& BBEnds = getSplitedBBs(WorkList);
  clearIR(M, WorkList);
  auto&& ExtractedFunctions = getExtractedFunctions(BBEnds);
  setNoInlineForFenceRegion(ExtractedFunctions);

  LLVM_DEBUG(dbgs() << "[OFL]: pass completed\n");
  WorkList.clear();
  return true;
}

void OptFenceLower::visitCallInst(CallInst &CI) {
  LLVM_DEBUG(dbgs() << "[OFL]: visit CallInst: " << CI << "\n");

  // Search for users always starts with the constructor.
  // Otherwise, one barrier will be processed twice.
  Function *F = CI.getCalledFunction();
  if (!F || !F->getName().contains(SBeginIdentifier)) {
    return;
  }

  // Constructor is expected to accept only one argument
  if (CI.arg_size() != 1)
    report_fatal_error("unexpected number(" + Twine(CI.arg_size()) +
                       ") of arguments in constructor of OFptFenceLowering");

  CallInst *CIBegin = nullptr;
  CallInst *CIEnd = nullptr;
  Value *Arg = CI.getArgOperand(0);
  if (!isa<AllocaInst>(Arg))
    report_fatal_error("OptFenceLowering cannot be global or not stack object");

  for (auto *User : Arg->users()) {
    LLVM_DEBUG(dbgs() << "Check user: " << *User << "\n");

    CallInst *UserCI = dyn_cast<CallInst>(User);
    Function *UserF = UserCI ? UserCI->getCalledFunction() : nullptr;
    StringRef UserFName = UserF ? UserF->getName() : "";

    // Interested only in the name of the function to which
    // the fence object is passed. All other cases are treated as an error.
    if (UserF && UserFName.contains(SEndIdentifier)) {
      LLVM_DEBUG(dbgs() << "Detected end\n");
      if (CIEnd)
        report_fatal_error("several constructors for the CMOptimizationsFence");
      CIEnd = UserCI;
    } else if (UserF && UserFName.contains(SBeginIdentifier)) {
      LLVM_DEBUG(dbgs() << "Detected begin\n");
      if (CIBegin)
        report_fatal_error("several destructors for the CMOptimizationsFence");
      CIBegin = UserCI;
    } else {
      report_fatal_error("unexpected use of CMOptimizationsFence object");
    }
  }

  if (!CIBegin || !CIEnd)
    report_fatal_error("[CMOptimizationsFence] Сomplete constructor-destructor "
                       "pair was not detected");
  WorkList.emplace_back(CIBegin, CIEnd);
}

OptFenceLower::WorkListBBTy
OptFenceLower::getSplitedBBs(const WorkListCITy &WL) {
  WorkListBBTy Result;
  Result.reserve(WL.size());
  std::transform(
      WL.begin(), WL.end(), std::back_inserter(Result),
      [](WorkListCITy::const_reference Work) -> WorkListBBTy::value_type {
        CallInst *CIBegin = Work.first;
        CallInst *CIEnd = Work.second;
        // we cut the BasicBlock into two parts at the location
        // of the barrier
        BasicBlock *NewBBBegin = SplitBlock(CIBegin->getParent(), CIBegin);
        BasicBlock *NewBBEnd = SplitBlock(CIEnd->getParent(), CIEnd);

        return {NewBBBegin, NewBBEnd};
      });
  return Result;
}

void OptFenceLower::clearIR(Module &M, const WorkListCITy &WL) {
  // Alloc call that creates a barrier object and calls to the constructor
  // and destructor for this object are not needed in the future.
  // Clearing the IR of all this.
  std::for_each(WL.begin(), WL.end(), [](WorkListCITy::const_reference Work) {
    AllocaInst *AI = cast<AllocaInst>(Work.first->getArgOperand(0));

    AI->dropAllReferences();
    Work.first->dropAllReferences();
    Work.second->dropAllReferences();

    Work.first->eraseFromParent();
    Work.second->eraseFromParent();
    AI->eraseFromParent();
  });

  // Erase dead function prototypes.
  for (Function &F : llvm::make_early_inc_range(M)) {
    if (F.getName().contains(SEndIdentifier) || F.getName().contains(SBeginIdentifier)) {
      assert(F.isDeclaration() && "Only declaration expected at this point");
      F.eraseFromParent();
    }
  }
}

std::vector<Function *>
OptFenceLower::getExtractedFunctions(const WorkListBBTy &WL) {
  std::vector<Function *> Result;
  Result.reserve(WL.size());
  std::transform(WL.begin(), WL.end(), std::back_inserter(Result),
                 [](WorkListBBTy::const_reference Work) {
                   std::vector<BasicBlock *> ExtractArea =
                       getAreaToExtract(Work.first, Work.second);
                   return CodeExtractor(ExtractArea, nullptr, false, nullptr,
                                        nullptr, nullptr, false, false,
                                        SNewFunctionIdentifier)
                       .extractCodeRegion();
                 });

  if (llvm::any_of(Result, [](Function *F) { return F == nullptr; }))
    report_fatal_error("[CMOptimizationsFence] Couldn't split the function");

  return Result;
}

void OptFenceLower::setNoInlineForFenceRegion(
    const std::vector<Function *> &Fs) {
  // rename all new functions and add NoInline attr
  for (auto &&F : Fs) {
    F->removeFnAttr(llvm::Attribute::AlwaysInline);
    F->removeFnAttr(llvm::Attribute::InlineHint);
    F->addFnAttr(llvm::Attribute::NoInline);
  }
}

std::vector<BasicBlock *> getAreaToExtract(BasicBlock *BBBegin,
                                           BasicBlock *BBEnd) {
  std::vector<BasicBlock *> Result;

  // Pass linearly from BBBegin to BBEnd.
  std::vector<BasicBlock *> RecursiveStack;
  RecursiveStack.reserve(BBBegin->getParent()->size());
  // 'set' is for remember visited BasicBlocks and checking this fact.
  std::unordered_set<BasicBlock *> BBVisited;
  // Exit from the algorithm: we have reached the BasicBlock
  // of the end of the barrier (we consider it colored)
  BBVisited.emplace(BBEnd);
  RecursiveStack.push_back(BBBegin);
  while (!RecursiveStack.empty()) {
    BasicBlock *CurBB = RecursiveStack.back();
    RecursiveStack.pop_back();
    if (BBVisited.count(CurBB))
      continue;

    auto &&Succs = llvm::successors(CurBB);
    if (Succs.begin() == Succs.end())
      report_fatal_error("[CMOptimizationsFence] The constructor does not dominate the destructor!");
    std::copy(Succs.begin(), Succs.end(), std::back_inserter(RecursiveStack));

    Result.push_back(CurBB);
    BBVisited.emplace(CurBB);
  }

  return Result;
}

//-----------------------------------------------------------------------------
// New PM support
//-----------------------------------------------------------------------------
PreservedAnalyses llvm::OptFenceLowering::run(Module &M,
                                              ModuleAnalysisManager &) {
  OptFenceLower Impl;
  Impl.run(M);
  return PreservedAnalyses::none();
}

//-----------------------------------------------------------------------------
// Legacy PM support
//-----------------------------------------------------------------------------
namespace {
class OptFenceLoweringLegacy : public ModulePass {
  OptFenceLower Impl;

public:
  static char ID;

  OptFenceLoweringLegacy() : ModulePass(ID), Impl() {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    ModulePass::getAnalysisUsage(AU);
  }

  bool runOnModule(Module &M) override { return Impl.run(M); }
};
} // namespace

ModulePass *llvm::createOptFenceLoweringPass() {
  return new OptFenceLoweringLegacy();
}

char OptFenceLoweringLegacy::ID = 0;
INITIALIZE_PASS(OptFenceLoweringLegacy, OptFenceLowering::getArgString(),
                OptFenceLowering::getArgString(), false, false)
