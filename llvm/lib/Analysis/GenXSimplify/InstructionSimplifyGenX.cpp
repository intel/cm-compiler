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
// This file defines a routine for simplifying a GenX intrinsic call to a
// constant or one of the operands. This is for cases where not all operands
// are constant; the constant operand cases are handled in ConstantFoldGenX.cpp.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsGenX.h"
#include "llvm/Pass.h"
#include "llvm/PassSupport.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

/***********************************************************************
 * SimplifyGenXIntrinsic : given a GenX intrinsic and a set of arguments,
 * see if we can fold the result.
 *
 * ConstantFoldingGenX.cpp handles pure constant folding cases. This code
 * only handles cases where not all operands are constant, but we can do
 * some folding anyway.
 *
 * If this call could not be simplified, returns null.
 */
template<typename IterTy>
static Value *SimplifyGenXIntrinsic2(unsigned IID, Type *RetTy,
    IterTy ArgBegin, IterTy ArgEnd)
{
  switch (IID) {
    case Intrinsic::genx_rdregioni:
    case Intrinsic::genx_rdregionf:
      // Identity rdregion can be simplified to its "old value" input.
      if (RetTy
          == ArgBegin[Intrinsic::GenXRegion::OldValueOperandNum]->getType()) {
        unsigned NumElements = RetTy->getVectorNumElements();
        unsigned Width = cast<ConstantInt>(
              ArgBegin[Intrinsic::GenXRegion::RdWidthOperandNum])
            ->getZExtValue();
        auto IndexV = dyn_cast<Constant>(
          ArgBegin[Intrinsic::GenXRegion::RdIndexOperandNum]);
        if (!IndexV)
          return nullptr;
        unsigned Index = 0;
        if (!isa<VectorType>(IndexV->getType()))
          Index = dyn_cast<ConstantInt>(IndexV)->getZExtValue()
          / (RetTy->getScalarType()->getPrimitiveSizeInBits() / 8);
        else
          return nullptr;
        if ((Index == 0 || Index >= NumElements) &&
            (Width == NumElements || Width == cast<ConstantInt>(ArgBegin[
             Intrinsic::GenXRegion::RdVStrideOperandNum])->getSExtValue()))
          if (NumElements == 1 || cast<ConstantInt>(ArgBegin[
                Intrinsic::GenXRegion::RdStrideOperandNum])->getSExtValue())
            return ArgBegin[Intrinsic::GenXRegion::OldValueOperandNum];
      }
      // rdregion with splatted constant input can be simplified to a constant of
      // the appropriate type, ignoring the possibly variable index.
      if (auto C = dyn_cast<Constant>(
            ArgBegin[Intrinsic::GenXRegion::OldValueOperandNum]))
        if (auto Splat = C->getSplatValue()) {
          if (auto VT = dyn_cast<VectorType>(RetTy))
            return ConstantVector::getSplat(VT->getNumElements(), Splat);
          return Splat;
        }
      break;
    case Intrinsic::genx_wrregioni:
    case Intrinsic::genx_wrregionf:
      // The wrregion case specifically excludes genx_wrconstregion.
      // Identity wrregion can be simplified to its "new value" input.
      if (RetTy
          == ArgBegin[Intrinsic::GenXRegion::NewValueOperandNum]->getType()) {
        if (auto CMask = dyn_cast<Constant>(ArgBegin[
              Intrinsic::GenXRegion::PredicateOperandNum])) {
          if (CMask->isAllOnesValue()) {
            unsigned NumElements = RetTy->getVectorNumElements();
            unsigned Width = cast<ConstantInt>(
                  ArgBegin[Intrinsic::GenXRegion::WrWidthOperandNum])
                ->getZExtValue();
            auto IndexV = dyn_cast<Constant>(
              ArgBegin[Intrinsic::GenXRegion::WrIndexOperandNum]);
            if (!IndexV)
              return nullptr;
            unsigned Index = 0;
            if (!isa<VectorType>(IndexV->getType()))
              Index = dyn_cast<ConstantInt>(IndexV)->getZExtValue()
              / (RetTy->getScalarType()->getPrimitiveSizeInBits() / 8);
            else
              return nullptr;
            if ((Index == 0 || Index >= NumElements) &&
                (Width == NumElements || Width == cast<ConstantInt>(ArgBegin[
                 Intrinsic::GenXRegion::WrVStrideOperandNum])->getSExtValue()))
              if (NumElements == 1 || cast<ConstantInt>(ArgBegin[
                    Intrinsic::GenXRegion::WrStrideOperandNum])->getSExtValue())
                return ArgBegin[Intrinsic::GenXRegion::NewValueOperandNum];
          }
        }
      }
      // Wrregion with constant 0 predicate can be simplified to its "old value"
      // input.
      if (auto CMask = dyn_cast<Constant>(ArgBegin[
            Intrinsic::GenXRegion::PredicateOperandNum]))
        if (CMask->isNullValue())
          return ArgBegin[Intrinsic::GenXRegion::OldValueOperandNum];
      // Wrregion writing a value that has just been read out of the same
      // region in the same vector can be simplified to its "old value" input.
      // This works even if the predicate is not all true.
      if (auto RdR = dyn_cast<CallInst>(ArgBegin[
            Intrinsic::GenXRegion::NewValueOperandNum])) {
        if (auto RdRFunc = RdR->getCalledFunction()) {
          Value *OldVal = ArgBegin[Intrinsic::GenXRegion::OldValueOperandNum];
          if ((RdRFunc->getIntrinsicID() == Intrinsic::genx_rdregioni
               || RdRFunc->getIntrinsicID() == Intrinsic::genx_rdregionf)
              && RdR->getArgOperand(Intrinsic::GenXRegion::OldValueOperandNum)
                == OldVal) {
            // Check the region parameters match between the rdregion and
            // wrregion. There are 4 region parameters: vstride, width, stride,
            // index.
            bool CanSimplify = true;
            for (unsigned i = 0; i != 4; ++i) {
              if (ArgBegin[Intrinsic::GenXRegion::WrVStrideOperandNum + i]
                  != RdR->getArgOperand(
                    Intrinsic::GenXRegion::RdVStrideOperandNum + i)) {
                CanSimplify = false;
                break;
              }
            }
            if (CanSimplify)
              return OldVal;
          }
        }
      }
      break;
    case Intrinsic::genx_wrpredregion:
      // wrpredregion with undef "new value" input is simplified to the "old
      // value" input.
      if (isa<UndefValue>(ArgBegin[1]))
        return ArgBegin[0];
      break;
  }
  return nullptr;
}

// Instantiations for the two ways in which SimplifyGenXIntrinsic is
// used from InstructionSimplify.cpp.
Value *llvm::SimplifyGenXIntrinsic(unsigned IID, Type *RetTy,
    Use *ArgBegin, Use *ArgEnd)
{
  return SimplifyGenXIntrinsic2(IID, RetTy, ArgBegin, ArgEnd);
}

Value *llvm::SimplifyGenXIntrinsic(unsigned IID, Type *RetTy,
    Value *const *ArgBegin, Value *const *ArgEnd)
{
  return SimplifyGenXIntrinsic2(IID, RetTy, ArgBegin, ArgEnd);
}

namespace {
class GenXSimplify : public FunctionPass {
public:
  static char ID;

  GenXSimplify() : FunctionPass(ID) {
    initializeGenXSimplifyPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

private:
  template <typename IterTy>
  Value *simplifyCall(Value *V, IterTy ArgBegin, IterTy ArgEnd);
};
} // namespace

bool GenXSimplify::runOnFunction(Function &F) {
  bool Changed = false;
  for (BasicBlock &BB : F) {
    for (auto I = BB.begin(); I != BB.end();) {
      Instruction *Inst = &(*I++);
      if (auto CI = dyn_cast<CallInst>(Inst)) {
        CallSite CS(CI);
        if (Value *V = simplifyCall(CS.getCalledValue(), CS.arg_begin(),
                                    CS.arg_end())) {
          Inst->replaceAllUsesWith(V);
          Inst->eraseFromParent();
          Changed = true;
        }
      }
    }
  }
  return Changed;
}

template <typename IterTy>
Value *GenXSimplify::simplifyCall(Value *V, IterTy ArgBegin, IterTy ArgEnd) {
  Type *Ty = V->getType();
  if (PointerType *PTy = dyn_cast<PointerType>(Ty))
    Ty = PTy->getElementType();
  FunctionType *FTy = cast<FunctionType>(Ty);
  Function *F = dyn_cast<Function>(V);
  if (!F)
    return nullptr;

  if (unsigned IID = F->getIntrinsicID()) {
    // Handle GenX intrinsics. This comparison uses the start and end markers of
    // the genx intrinsic enum values, relying on tablegen outputting the
    // intrinsics in sorted by name order.
    if ((unsigned)(IID - Intrinsic::genx_aaaabegin) <=
        (Intrinsic::genx_zzzzend - Intrinsic::genx_aaaabegin))
      if (Value *Ret = SimplifyGenXIntrinsic(IID, FTy->getReturnType(),
                                             ArgBegin, ArgEnd))
        return Ret;
  }
  return nullptr;
}

char GenXSimplify::ID = 0;
INITIALIZE_PASS(GenXSimplify, "genxsimplify",
                "simplify genx specific instructions", false, false)

FunctionPass *llvm::createGenXSimplifyPass() { return new GenXSimplify; }
