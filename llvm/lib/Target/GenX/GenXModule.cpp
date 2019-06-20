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
// GenXModule is a module pass whose purpose is to store information
// about the GenX module being written, such as the built kernels and functions.
// See the comment in GenXModule.h.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "GENX_MODULE"

#include "FunctionGroup.h"
#include "GenXModule.h"
#include "GenX.h"
#include "GenXSubtarget.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <set>

using namespace llvm;

char GenXModule::ID = 0;
INITIALIZE_PASS_BEGIN(GenXModule, "GenXModule", "GenXModule", false, true/*analysis*/)
INITIALIZE_PASS_DEPENDENCY(FunctionGroupAnalysis)
INITIALIZE_PASS_END(GenXModule, "GenXModule", "GenXModule", false, true/*analysis*/)

ModulePass *llvm::createGenXModulePass()
{
  initializeGenXModulePass(*PassRegistry::getPassRegistry());
  return new GenXModule;
}

void GenXModule::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.addRequired<FunctionGroupAnalysis>();
  AU.setPreservesAll();
}

/***********************************************************************
 * runOnModule : run GenXModule analysis
 *
 * This populates FunctionGroupAnalysis such that each FunctionGroup
 * corresponds to a GenX kernel/function and its subroutines. If any
 * subroutine would be used in more than one FunctionGroup, it is
 * cloned.
 *
 * The FunctionGroup is populated in an order such that a function appears
 * after all its callers.
 */
bool GenXModule::runOnModule(Module &M)
{
  auto FGA = &getAnalysis<FunctionGroupAnalysis>();
  auto P = getAnalysisIfAvailable<GenXSubtargetPass>();
  ST = P ? P->getSubtarget() : nullptr;

  // Create a new FunctionGroup for each externally visible Function.
  for (Module::iterator mi = M.begin(), me = M.end(); mi != me; ++mi) {
    Function *F = &*mi;
    if (!F->empty() && F->getLinkage() != GlobalValue::InternalLinkage) {
      // Create a new FunctionGroup for this function.
      FGA->createFunctionGroup(F);
      DEBUG(dbgs() << "GenXModule: Function " << F->getName() << " is the head of its own FunctionGroup\n");
      // Check that it is not called internally and does not have its
      // address taken.
      if (!F->use_empty())
        report_fatal_error(Twine("Kernel/function ") + F->getName() + " cannot be called by itself or another function");
    }
  }
  // Iterate, processing each Function that is not yet assigned to a FunctionGroup.
  bool ModuleModified = false;
  for (;;) {
    bool Finished = true, Changed = false;
    for (Module::iterator mi = M.begin(), me = M.end(); mi != me; ++mi) {
      Function *F = &*mi;
      if (F->empty())
        continue;
      // See if this Function is already in a Func.
      if (FGA->getGroup(F))
        continue;
      // See what FunctionGroups this Function is called from.
      std::map<FunctionGroup *, Function *> CallerFGs;
      for (Value::use_iterator ui = F->use_begin(), ue = F->use_end();
          ui != ue; ++ui) {
        CallInst *user = dyn_cast<CallInst>(ui->getUser());
        if (!user || ui->getOperandNo() != user->getNumArgOperands())
          report_fatal_error(Twine("Kernel/function ") + F->getName() + " has its address taken");
        // We have a use in a call instruction.
        Function *Caller = user->getParent()->getParent();
        CallerFGs.insert(std::pair<FunctionGroup *, Function *>(
              FGA->getGroup(Caller), nullptr));
      }
      if (CallerFGs.find(nullptr) != CallerFGs.end()) {
        // At least one caller is not yet in a FunctionGroup.
        Finished = false;
      } else if (CallerFGs.size() == 1) {
        // All callers are in the same FunctionGroup. We can just add this
        // Function to that FunctionGroup.
        FGA->addToFunctionGroup(CallerFGs.begin()->first, F);
        Changed = true;
        DEBUG(dbgs() << "GenXModule: Function " << F->getName() << " is in FunctionGroup " << FGA->getGroup(F)->getName() << "\n");
      } else {
        // This Function is used by multiple Funcs. We need to clone it.
        // Scan the uses, changing to a cloned function if necessary.
        Changed = true;
        bool UsedOriginal = false;
        for (Value::use_iterator ui = F->use_begin(), ue = F->use_end();
            ui != ue; ) {
          // Increment iterator after setting U as we are going to change
          // the use.
          Use *U = &*ui++;
          Function *Caller = cast<CallInst>(U->getUser())->getParent()->getParent();
          auto i = CallerFGs.find(FGA->getGroup(Caller));
          if (!i->second) {
            // No function for a caller of this function group yet.
            if (!UsedOriginal) {
              i->second = F;
              UsedOriginal = true;
              DEBUG(dbgs() << "GenXModule: Function " << F->getName() << " is in FunctionGroup " << FGA->getGroup(Caller)->getName() << "\n");
            } else {
              ValueToValueMapTy VMap;
              Function *ClonedFunc = CloneFunction(F, VMap);
              i->second = ClonedFunc;
              DEBUG(dbgs() << "GenXModule: cloned Function " << ClonedFunc->getName() << " is in FunctionGroup " << FGA->getGroup(Caller)->getName() << "\n");
            }
            FGA->addToFunctionGroup(i->first, i->second);
          }
          // Change the use.
          *U = i->second;
        }
      }
    }
    if (Finished)
      break;
    if (Changed)
      continue;
    // We've got stuck. There must be some recursion.
    report_fatal_error("Recursion is illegal");
  }
  return ModuleModified;
}