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
/// LowerCMSimdCF
/// -------------
///
/// This is the worker class to lowers CM SIMD control flow into a form where
/// the IR reflects the semantics. See CMSimdCFLowering.cpp for details.
///
//===----------------------------------------------------------------------===//

#include "llvm/ADT/MapVector.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include <algorithm>
#include <set>

namespace llvm {

// The worker class for lowering CM SIMD CF
class CMSimdCFLower {
  Function *F;
  // A map giving the basic blocks ending with a simd branch, and the simd
  // width of each one.
  MapVector<BasicBlock *, unsigned> SimdBranches;
  // A map giving the basic blocks to be predicated, and the simd width of
  // each one.
  MapVector<BasicBlock *, unsigned> PredicatedBlocks;
  // The join points, together with the simd width of each one.
  MapVector<BasicBlock *, unsigned> JoinPoints;
  // The JIP for each simd branch and join point.
  std::map<BasicBlock *, BasicBlock *> JIPs;
  // Subroutines that are predicated, mapping to the simd width.
  std::map<Function *, unsigned> PredicatedSubroutines;
  // Execution mask variable.
  GlobalVariable *EMVar;
  // Resume mask for each join point.
  std::map<BasicBlock *, AllocaInst *> RMAddrs;
  // Set of intrinsic calls (other than wrregion) that have been predicated.
  std::set<AssertingVH<Value>> AlreadyPredicated;
  // Mask for shufflevector to extract part of EM.
  SmallVector<Constant *, 32> ShuffleMask;
public:
  static const unsigned MAX_SIMD_CF_WIDTH = 32;

  CMSimdCFLower(GlobalVariable *EMask) : EMVar(EMask) {}

  static CallInst *isSimdCFAny(Value *V);
  static Use *getSimdConditionUse(Value *Cond);

  void processFunction(Function *F);

private:
  bool findSimdBranches(unsigned CMWidth);
  void determinePredicatedBlocks();
  void markPredicatedBranches();
  void fixSimdBranches();
  void findAndSplitJoinPoints();
  void determineJIPs();
  void determineJIP(BasicBlock *BB, std::map<BasicBlock *, unsigned> *Numbers, bool IsJoin);

  // Methods to add predication to the code
  void predicateCode(unsigned CMWidth);
  void predicateBlock(BasicBlock *BB, unsigned SimdWidth);
  void predicateInst(Instruction *Inst, unsigned SimdWidth);
  void rewritePredication(CallInst *CI, unsigned SimdWidth);
  void predicateStore(Instruction *SI, unsigned SimdWidth);
  CallInst *convertScatterGather(CallInst *CI, unsigned IID);
  void predicateSend(CallInst *CI, unsigned IntrinsicID, unsigned SimdWidth);
  void predicateScatterGather(CallInst *CI, unsigned SimdWidth, unsigned PredOperandNum);
  CallInst *predicateWrRegion(CallInst *WrR, unsigned SimdWidth);
  void predicateCall(CallInst *CI, unsigned SimdWidth);

  void lowerSimdCF();
  Instruction *loadExecutionMask(Instruction *InsertBefore, unsigned SimdWidth);
  Value *getRMAddr(BasicBlock *JP, unsigned SimdWidth);
};

} // namespace

