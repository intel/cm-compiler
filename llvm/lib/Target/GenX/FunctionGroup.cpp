/*
 * Copyright (c) 2019, Intel Corporation
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
// This file implements FunctionGroup, FunctionGroupAnalysis and 
// FunctionGroupPass. See FunctionGroup.h for more details.
//
// The FunctionGroupPass part was adapted from CallGraphSCCPass.cpp.
//
// This file is currently in lib/Target/GenX, as that is the only place it
// is used. It could be moved somewhere more general.
//
//===----------------------------------------------------------------------===//

#include "FunctionGroup.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManagers.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "functiongroup-passmgr"

/***********************************************************************
 * FunctionGroupAnalysis implementation
 */
char FunctionGroupAnalysis::ID = 0;
INITIALIZE_PASS(FunctionGroupAnalysis, "FunctionGroupAnalysis", "FunctionGroupAnalysis", false, true/*analysis*/)

ModulePass *llvm::createFunctionGroupAnalysisPass()
{
  initializeFunctionGroupAnalysisPass(*PassRegistry::getPassRegistry());
  return new FunctionGroupAnalysis();
}

// clear : clear out the analysis
void FunctionGroupAnalysis::clear()
{
  GroupMap.clear();
  for (auto i = begin(), e = end(); i != e; ++i)
    delete *i;
  Groups.clear();
  M = nullptr;
}

// getGroup : get the FunctionGroup containing Function F, else 0
FunctionGroup *FunctionGroupAnalysis::getGroup(Function *F)
{
  auto i = GroupMap.find(F);
  if (i == GroupMap.end())
    return nullptr;
  return i->second;
}

// getGroupForHead : get the FunctionGroup for which Function F is the
// head, else 0
FunctionGroup *FunctionGroupAnalysis::getGroupForHead(Function *F)
{
  auto FG = getGroup(F);
  assert(FG->size());
  if (*FG->begin() == F)
    return FG;
  return nullptr;
}

// replaceFunction : replace a Function in a FunctionGroup
// An in-use iterator in the modified FunctionGroup remains valid.
void FunctionGroupAnalysis::replaceFunction(Function *OldF, Function *NewF)
{
  auto i = GroupMap.find(OldF);
  assert(i != GroupMap.end());
  FunctionGroup *FG = i->second;
  GroupMap.erase(i);
  GroupMap[NewF] = FG;
  for (auto i = FG->begin(); ; ++i) {
    assert(i != FG->end());
    if (*i == OldF) {
      *i = NewF;
      break;
    }
  }
}

// addToFunctionGroup : add Function F to FunctionGroup FG
// Using this (rather than calling push_back directly on the FunctionGroup)
// means that the mapping from F to FG will be created, and getGroup() will
// work for this Function.
void FunctionGroupAnalysis::addToFunctionGroup(FunctionGroup *FG, Function *F)
{
  assert(FG->getParent()->getModule() == M && "attaching to FunctionGroup from wrong Module");
  assert(!GroupMap[F] && "Function already attached to FunctionGroup");
  GroupMap[F] = FG;
  FG->push_back(F);
}

// createFunctionGroup : create new FunctionGroup for which F is the head
FunctionGroup *FunctionGroupAnalysis::createFunctionGroup(Function *F)
{
  auto FG = new FunctionGroup(this);
  Groups.push_back(FG);
  addToFunctionGroup(FG, F);
  return FG;
}

//===----------------------------------------------------------------------===//
// FGPassManager
//
/// FGPassManager manages FPPassManagers and FunctionGroupPasses.

namespace {

class FGPassManager : public ModulePass, public PMDataManager {
public:
  static char ID;
  explicit FGPassManager() 
    : ModulePass(ID), PMDataManager() { }

  /// run - Execute all of the passes scheduled for execution.  Keep track of
  /// whether any of the passes modifies the module, and if so, return true.
  bool runOnModule(Module &M) override;

  using ModulePass::doInitialization;
  using ModulePass::doFinalization;

  bool doInitialization(FunctionGroupAnalysis &FGA);
  bool doFinalization(FunctionGroupAnalysis &FGA);

  /// Pass Manager itself does not invalidate any analysis info.
  void getAnalysisUsage(AnalysisUsage &Info) const override {
    // FGPassManager needs FunctionGroupAnalysis.
    Info.addRequired<FunctionGroupAnalysis>();
    Info.setPreservesAll();
  }

  StringRef getPassName() const override {
    return "FunctionGroup Pass Manager";
  }

  PMDataManager *getAsPMDataManager() override { return this; }
  Pass *getAsPass() override { return this; }

  // Print passes managed by this manager
  void dumpPassStructure(unsigned Offset) override {
    errs().indent(Offset*2) << "FunctionGroup Pass Manager\n";
    for (unsigned Index = 0; Index < getNumContainedPasses(); ++Index) {
      Pass *P = getContainedPass(Index);
      P->dumpPassStructure(Offset + 1);
      dumpLastUses(P, Offset+1);
    }
  }

  Pass *getContainedPass(unsigned N) {
    assert(N < PassVector.size() && "Pass number out of range!");
    return static_cast<Pass *>(PassVector[N]);
  }

  PassManagerType getPassManagerType() const override {
    return PMT_FunctionGroupPassManager; 
  }

private:
  bool RunAllPassesOnFunctionGroup(FunctionGroup &FG);
  bool RunPassOnFunctionGroup(Pass *P, FunctionGroup &FG);
};

} // end anonymous namespace.

char FGPassManager::ID = 0;

bool FGPassManager::RunPassOnFunctionGroup(Pass *P, FunctionGroup &FG)
{
  bool Changed = false;
  PMDataManager *PM = P->getAsPMDataManager();

  if (!PM) {
    FunctionGroupPass *CGSP = (FunctionGroupPass*)P;
    {
      TimeRegion PassTimer(getPassTimer(CGSP));
      Changed = CGSP->runOnFunctionGroup(FG);
    }
    return Changed;
  }

  assert(PM->getPassManagerType() == PMT_FunctionPassManager &&
         "Invalid FGPassManager member");
  FPPassManager *FPP = (FPPassManager*)P;

  // Run pass P on all functions in the current FunctionGroup.
  for (auto I = FG.begin(), E = FG.end(); I != E; ++I) {
    Function *F = *I;
    dumpPassInfo(P, EXECUTION_MSG, ON_FUNCTION_MSG, F->getName());
    {
      TimeRegion PassTimer(getPassTimer(FPP));
      Changed |= FPP->runOnFunction(*F);
    }
    F->getContext().yield();
  }
  return Changed;
}


/// RunAllPassesOnFunctionGroup -  Execute the body of the entire pass manager
/// on the specified FunctionGroup
bool FGPassManager::RunAllPassesOnFunctionGroup(FunctionGroup &FG)
{
  bool Changed = false;
  // Run all passes on current FunctionGroup.
  for (unsigned PassNo = 0, e = getNumContainedPasses();
       PassNo != e; ++PassNo) {
    Pass *P = getContainedPass(PassNo);
    dumpRequiredSet(P);

    initializeAnalysisImpl(P);

    // Actually run this pass on the current FunctionGroup.
    Changed |= RunPassOnFunctionGroup(P, FG);
    if (Changed)
      dumpPassInfo(P, MODIFICATION_MSG, ON_FG_MSG, "");
    dumpPreservedSet(P);

    verifyPreservedAnalysis(P);
    removeNotPreservedAnalysis(P);
    recordAvailableAnalysis(P);
    removeDeadPasses(P, "", ON_FG_MSG);
  }
  return Changed;
}

/// run - Execute all of the passes scheduled for execution.  Keep track of
/// whether any of the passes modifies the module, and if so, return true.
bool FGPassManager::runOnModule(Module &M)
{
  FunctionGroupAnalysis &FGA = getAnalysis<FunctionGroupAnalysis>();
  bool Changed = doInitialization(FGA);
  // Run all passes on each FunctionGroup.
  for (auto i = FGA.begin(), e = FGA.end(); i != e; ++i)
    Changed |= RunAllPassesOnFunctionGroup(**i);
  Changed |= doFinalization(FGA);
  return Changed;
}


/// Initialize
bool FGPassManager::doInitialization(FunctionGroupAnalysis &FGA)
{
  bool Changed = false;
  for (unsigned i = 0, e = getNumContainedPasses(); i != e; ++i) {  
    if (PMDataManager *PM = getContainedPass(i)->getAsPMDataManager()) {
      assert(PM->getPassManagerType() == PMT_FunctionPassManager &&
             "Invalid FGPassManager member");
      Changed |= ((FPPassManager*)PM)->doInitialization(*FGA.getModule());
    } else {
      Changed |= ((FunctionGroupPass*)getContainedPass(i))->doInitialization(FGA);
    }
  }
  return Changed;
}

/// Finalize
bool FGPassManager::doFinalization(FunctionGroupAnalysis &FGA)
{
  bool Changed = false;
  for (unsigned i = 0, e = getNumContainedPasses(); i != e; ++i) {  
    if (PMDataManager *PM = getContainedPass(i)->getAsPMDataManager()) {
      assert(PM->getPassManagerType() == PMT_FunctionPassManager &&
             "Invalid FGPassManager member");
      Changed |= ((FPPassManager*)PM)->doFinalization(*FGA.getModule());
    } else {
      Changed |= ((FunctionGroupPass*)getContainedPass(i))->doFinalization(FGA);
    }
  }
  return Changed;
}

//===----------------------------------------------------------------------===//
// FunctionGroupPass Implementation
//===----------------------------------------------------------------------===//

/// Assign pass manager to manage this pass.
void FunctionGroupPass::assignPassManager(PMStack &PMS,
                                     PassManagerType PreferredType)
{
  // Find FGPassManager 
  while (!PMS.empty() &&
         PMS.top()->getPassManagerType() > PMT_FunctionGroupPassManager)
    PMS.pop();

  assert(!PMS.empty() && "Unable to handle FunctionGroup Pass");
  FGPassManager *GFP;
  
  if (PMS.top()->getPassManagerType() == PMT_FunctionGroupPassManager)
    GFP = (FGPassManager*)PMS.top();
  else {
    // Create new FunctionGroup Pass Manager if it does not exist. 
    assert(!PMS.empty() && "Unable to create FunctionGroup Pass Manager");
    PMDataManager *PMD = PMS.top();

    // [1] Create new FunctionGroup Pass Manager
    GFP = new FGPassManager();

    // [2] Set up new manager's top level manager
    PMTopLevelManager *TPM = PMD->getTopLevelManager();
    TPM->addIndirectPassManager(GFP);

    // [3] Assign manager to manage this new manager. This may create
    // and push new managers into PMS
    Pass *P = GFP;
    TPM->schedulePass(P);

    // [4] Push new manager into PMS
    PMS.push(GFP);
  }

  GFP->add(this);
}

/// getAnalysisUsage - For this class, we declare that we require and preserve
/// FunctionGroupAnalysis.  If the derived class implements this method, it should
/// always explicitly call the implementation here.
void FunctionGroupPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<FunctionGroupAnalysis>();
  AU.addPreserved<FunctionGroupAnalysis>();
}

//===----------------------------------------------------------------------===//
// PrintFunctionGroupPass Implementation
//===----------------------------------------------------------------------===//

namespace {
  /// PrintFunctionGroupPass - Print a FunctionGroup
  ///
  class PrintFunctionGroupPass : public FunctionGroupPass {
    std::string Banner;
    raw_ostream &Out;       // raw_ostream to print on.
  public:
    static char ID;
    PrintFunctionGroupPass(const std::string &B, raw_ostream &o)
      : FunctionGroupPass(ID), Banner(B), Out(o) {}

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

    bool runOnFunctionGroup(FunctionGroup &FG) override {
      Out << Banner;
      for (auto I = FG.begin(), E = FG.end(); I != E; ++I) {
        Function *F = *I;
        Out << Banner << static_cast<Value &>(*F);
      }
      return false;
    }
  };
} // end anonymous namespace.

char PrintFunctionGroupPass::ID = 0;

Pass *FunctionGroupPass::createPrinterPass(raw_ostream &O,
                                      const std::string &Banner) const
{
  return new PrintFunctionGroupPass(Banner, O);
}

//===----------------------------------------------------------------------===//
//  DominatorTreeGroupWrapperPass Implementation
//===----------------------------------------------------------------------===//
//
// The implementation details of the wrapper pass that holds a DominatorTree
// per Function in a FunctionGroup.
//
//===----------------------------------------------------------------------===//
char DominatorTreeGroupWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(DominatorTreeGroupWrapperPass, "groupdomtree", "Group Dominator Tree Construction", true, true)
INITIALIZE_PASS_END(DominatorTreeGroupWrapperPass, "groupdomtree", "Group Dominator Tree Construction", true, true)

void DominatorTreeGroupWrapperPass::releaseMemory() {
  for (auto i = DTs.begin(), e = DTs.end(); i != e; ++i)
    delete i->second;
  DTs.clear();
}

bool DominatorTreeGroupWrapperPass::runOnFunctionGroup(FunctionGroup &FG) {
  for (auto fgi = FG.begin(), fge = FG.end(); fgi != fge; ++fgi) {
    Function *F = *fgi;
    auto DT = new DominatorTree;
    DT->recalculate(*F);
    DTs[F] = DT;
  }
  return false;
}

void DominatorTreeGroupWrapperPass::verifyAnalysis() const {
  for (auto i = DTs.begin(), e = DTs.end(); i != e; ++i)
    i->second->verifyDomTree();
}

void DominatorTreeGroupWrapperPass::print(raw_ostream &OS, const Module *) const {
  for (auto i = DTs.begin(), e = DTs.end(); i != e; ++i)
    i->second->print(OS);
}


