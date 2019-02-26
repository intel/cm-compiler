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
// Utility functions for the GenX backend.
//
//===----------------------------------------------------------------------===//
#include "GenX.h"
#include "GenXIntrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace genx;

/***********************************************************************
 * getIntrinsicID : utility function to get the intrinsic ID if the
 *                  value is an intrinsic call, Intrinsic::not_intrinsic
 *                  otherwise
 *
 * Enter:   V = value that could be an intrinsic call
 *
 * V is allowed to be 0.
 */
unsigned genx::getIntrinsicID(Value *V)
{
  if (V)
    if (CallInst *CI = dyn_cast<CallInst>(V))
      if (Function *Callee = CI->getCalledFunction())
        return Callee->getIntrinsicID();
  return Intrinsic::not_intrinsic;
}

/***********************************************************************
 * KernelMetadata constructor
 *
 * Enter:   F = Function that purports to be a CM kernel
 *
 * The metadata node has the following operands:
 *  0: reference to Function
 *  1: kernel name
 *  2: asm name
 *  3: reference to metadata node containing kernel arg kinds
 *  4: slm-size in bytes
 *  5: kernel argument offsets
 *  6: reference to metadata node containing kernel argument input/output kinds
 */
KernelMetadata::KernelMetadata(Function *F) : IsKernel(false), SLMSize(0)
{
  if (!genx::isKernel(F))
    return;
  NamedMDNode *Named = F->getParent()->getNamedMetadata("genx.kernels");
  if (!Named)
    return;

  MDNode *Node = nullptr;
  for (unsigned i = 0, e = Named->getNumOperands(); i != e; ++i) {
    if (i == e)
      return;
    Node = Named->getOperand(i);
    if (Node->getNumOperands() >= 7 &&
        getValueAsMetadata(Node->getOperand(0)) == F)
      break;
  }
  if (!Node)
    return;
  // Node is the metadata node for F, and it has the required 6 operands.
  IsKernel = true;
  if (MDString *MDS = dyn_cast<MDString>(Node->getOperand(1)))
    Name = MDS->getString();
  if (MDString *MDS = dyn_cast<MDString>(Node->getOperand(2)))
    AsmName = MDS->getString();
  if (ConstantInt *Sz = getValueAsMetadata<ConstantInt>(Node->getOperand(4)))
    SLMSize = Sz->getZExtValue();
  // Build the argument kinds and offsets arrays that should correspond to the
  // function arguments (both explicit and implicit)
  MDNode *KindsNode = dyn_cast<MDNode>(Node->getOperand(3));
  MDNode *OffsetsNode = dyn_cast<MDNode>(Node->getOperand(5));
  MDNode *InputOutputKinds = dyn_cast<MDNode>(Node->getOperand(6));
  assert(KindsNode && OffsetsNode &&
         KindsNode->getNumOperands() == OffsetsNode->getNumOperands());

  for (unsigned i = 0, e = KindsNode->getNumOperands(); i != e; ++i) {
    ArgKinds.push_back(getValueAsMetadata<ConstantInt>(KindsNode->getOperand(i))
                           ->getZExtValue());
    ArgOffsets.push_back(
        getValueAsMetadata<ConstantInt>(OffsetsNode->getOperand(i))
            ->getZExtValue());
  }
  assert(InputOutputKinds &&
         KindsNode->getNumOperands() >= InputOutputKinds->getNumOperands());
  for (unsigned i = 0, e = InputOutputKinds->getNumOperands(); i != e; ++i)
    ArgIOKinds.push_back(
        getValueAsMetadata<ConstantInt>(InputOutputKinds->getOperand(i))
            ->getZExtValue());
}

/***********************************************************************
 * KernelMetadata::getArgCategory : get category of kernel arg
 */
unsigned KernelMetadata::getArgCategory(unsigned Idx) const
{
  enum { AK_NORMAL, AK_SAMPLER, AK_SURFACE, AK_VME };
  switch (getArgKind(Idx) & 7) {
    case AK_SAMPLER:
      return RegCategory::SAMPLER;
    case AK_SURFACE:
      return RegCategory::SURFACE;
    case AK_VME:
      return RegCategory::VME;
    default:
      return RegCategory::GENERAL;
  }
}

/***********************************************************************
 * createConvert : create a genx_convert intrinsic call
 *
 * Enter:   In = value to convert
 *          Name = name to give convert instruction
 *          InsertBefore = instruction to insert before else 0
 *          M = Module (can be 0 as long as InsertBefore is not 0)
 */
CallInst *genx::createConvert(Value *In, const Twine &Name,
    Instruction *InsertBefore, Module *M)
{
  if (!M)
    M = InsertBefore->getParent()->getParent()->getParent();
  Function *Decl = Intrinsic::getDeclaration(M, Intrinsic::genx_convert,
      In->getType());
  return CallInst::Create(Decl, In, Name, InsertBefore);
}

/***********************************************************************
 * createConvertAddr : create a genx_convert_addr intrinsic call
 *
 * Enter:   In = value to convert
 *          Offset = constant offset
 *          Name = name to give convert instruction
 *          InsertBefore = instruction to insert before else 0
 *          M = Module (can be 0 as long as InsertBefore is not 0)
 */
CallInst *genx::createConvertAddr(Value *In, int Offset, const Twine &Name,
    Instruction *InsertBefore, Module *M)
{
  if (!M)
    M = InsertBefore->getParent()->getParent()->getParent();
  auto OffsetVal = ConstantInt::get(In->getType()->getScalarType(), Offset);
  Function *Decl = Intrinsic::getDeclaration(M, Intrinsic::genx_convert_addr,
      In->getType());
  Value *Args[] = { In, OffsetVal };
  return CallInst::Create(Decl, Args, Name, InsertBefore);
}

/***********************************************************************
 * createAddAddr : create a genx_add_addr intrinsic call
 *
 * InsertBefore can be 0 so the new instruction is not inserted anywhere,
 * but in that case M must be non-0 and set to the Module.
 */
CallInst *genx::createAddAddr(Value *Lhs, Value *Rhs, const Twine &Name,
    Instruction *InsertBefore, Module *M)
{
  if (!M)
    M = InsertBefore->getParent()->getParent()->getParent();
  Value *Args[] = {Lhs, Rhs};
  Type *Tys[] = {Rhs->getType(), Lhs->getType()};
  Function *Decl = Intrinsic::getDeclaration(M, Intrinsic::genx_add_addr, Tys);
  return CallInst::Create(Decl, Args, Name, InsertBefore);
}

/***********************************************************************
 * getPredicateConstantAsInt : get an i1 or vXi1 constant's value as a single integer
 */
unsigned genx::getPredicateConstantAsInt(Constant *C)
{
  if (auto CI = dyn_cast<ConstantInt>(C))
    return CI->getZExtValue(); // scalar
  unsigned Bits = 0;
  unsigned NumElements = cast<VectorType>(C->getType())->getNumElements();
  for (unsigned i = 0; i != NumElements; ++i) {
    auto El = C->getAggregateElement(i);
    if (!isa<UndefValue>(El))
      Bits |= (cast<ConstantInt>(El)->getZExtValue() & 1) << i;
  }
  return Bits;
}

/***********************************************************************
 * getConstantSubvector : get a contiguous region from a vector constant
 */
Constant *genx::getConstantSubvector(Constant *V,
    unsigned StartIdx, unsigned Size)
{
  Type *ElTy = cast<VectorType>(V->getType())->getElementType();
  Type *RegionTy = VectorType::get(ElTy, Size);
  if (isa<UndefValue>(V))
    V = UndefValue::get(RegionTy);
  else if (isa<ConstantAggregateZero>(V))
    V = ConstantAggregateZero::get(RegionTy);
  else {
    SmallVector<Constant *, 32> Val;
    for (unsigned i = 0; i != Size; ++i)
      Val.push_back(V->getAggregateElement(i + StartIdx));
    V = ConstantVector::get(Val);
  }
  return V;
}

/***********************************************************************
 * concatConstants : concatenate two possibly vector constants, giving a
 *      vector constant
 */
Constant *genx::concatConstants(Constant *C1, Constant *C2)
{
  assert(C1->getType()->getScalarType() == C2->getType()->getScalarType());
  Constant *CC[] = { C1, C2 };
  SmallVector<Constant *, 8> Vec;
  bool AllUndef = true;
  for (unsigned Idx = 0; Idx != 2; ++Idx) {
    Constant *C = CC[Idx];
    if (auto VT = dyn_cast<VectorType>(C->getType())) {
      for (unsigned i = 0, e = VT->getNumElements(); i != e; ++i) {
        Constant *El = C->getAggregateElement(i);
        Vec.push_back(El);
        AllUndef &= isa<UndefValue>(El);
      }
    } else {
      Vec.push_back(C);
      AllUndef &= isa<UndefValue>(C);
    }
  }
  auto Res = ConstantVector::get(Vec);
  if (AllUndef)
    Res = UndefValue::get(Res->getType());
  return Res;
}

/***********************************************************************
 * findClosestCommonDominator : find closest common dominator of some instructions
 *
 * Enter:   DT = dominator tree
 *          Insts = the instructions
 *
 * Return:  The one instruction that dominates all the others, if any.
 *          Otherwise the terminator of the closest common dominating basic
 *          block.
 */
namespace {
  struct InstScanner {
    Instruction *Original;
    Instruction *Current;
    InstScanner(Instruction *Inst) : Original(Inst), Current(Inst) {}
  };
}

Instruction *genx::findClosestCommonDominator(DominatorTree *DT,
    ArrayRef<Instruction *> Insts)
{
  assert(!Insts.empty());
  SmallVector<InstScanner, 8> InstScanners;
  // Find the closest common dominating basic block.
  Instruction *Inst0 = Insts[0];
  BasicBlock *NCD = Inst0->getParent();
  InstScanners.push_back(InstScanner(Inst0));
  for (unsigned ii = 1, ie = Insts.size(); ii != ie; ++ii) {
    Instruction *Inst = Insts[ii];
    if (Inst->getParent() != NCD) {
      auto NewNCD = DT->findNearestCommonDominator(NCD, Inst->getParent());
      if (NewNCD != NCD)
        InstScanners.clear();
      NCD = NewNCD;
    }
    if (NCD == Inst->getParent())
      InstScanners.push_back(Inst);
  }
  // Now we have NCD = the closest common dominating basic block, and
  // InstScanners populated with the instructions from Insts that are
  // in that block.
  if (InstScanners.empty()) {
    // No instructions in that block. Return the block's terminator.
    return NCD->getTerminator();
  }
  if (InstScanners.size() == 1) {
    // Only one instruction in that block. Return it.
    return InstScanners[0].Original;
  }
  // Create a set of the original instructions.
  std::set<Instruction *> OrigInsts;
  for (auto i = InstScanners.begin(), e = InstScanners.end(); i != e; ++i)
    OrigInsts.insert(i->Original);
  // Scan back one instruction at a time for each scanner. If a scanner reaches
  // another original instruction, the scanner can be removed, and when we are
  // left with one scanner, that must be the earliest of the original
  // instructions.  If a scanner reaches the start of the basic block, that was
  // the earliest of the original instructions.
  //
  // In the worst case, this algorithm could scan all the instructions in a
  // basic block, but it is designed to be better than that in the common case
  // that the original instructions are close to each other.
  for (;;) {
    for (auto i = InstScanners.begin(), e = InstScanners.end(); i != e; ++i) {
      if (i->Current == &i->Current->getParent()->front())
        return i->Original; // reached start of basic block
      i->Current = i->Current->getPrevNode();
      if (OrigInsts.find(i->Current) != OrigInsts.end()) {
        // Scanned back to another instruction in our original set. Remove
        // this scanner.
        *i = InstScanners.back();
        InstScanners.pop_back();
        if (InstScanners.size() == 1)
          return InstScanners[0].Original; // only one scanner left
        break; // restart loop so as not to confuse the iterator
      }
    }
  }
}

/***********************************************************************
 * getTwoAddressOperandNum : get operand number of two address operand
 *
 * If an intrinsic has a "two address operand", then that operand must be
 * in the same register as the result. This function returns the operand number
 * of the two address operand if any, or -1 if not.
 */
int genx::getTwoAddressOperandNum(CallInst *CI)
{
  unsigned IntrinsicID = getIntrinsicID(CI);
  if (IntrinsicID == Intrinsic::not_intrinsic)
    return -1; // not intrinsic
  if (isWrRegion(IntrinsicID)
      || IntrinsicID == Intrinsic::genx_wrpredregion
      || IntrinsicID == Intrinsic::genx_wrpredpredregion)
    return 0; // wr(pred(pred))region has operand 0 as two address operand
  if (CI->getType()->isVoidTy())
    return -1; // no return value
  GenXIntrinsicInfo II(IntrinsicID);
  unsigned Num = CI->getNumArgOperands();
  if (!Num)
    return -1; // no args
  --Num; // Num = last arg number, could be two address operand
  if (isa<UndefValue>(CI->getOperand(Num)))
    return -1; // operand is undef, must be RAW_NULLALLOWED
  if (II.getArgInfo(Num).getCategory() != GenXIntrinsicInfo::TWOADDR)
    return -1; // not two addr operand
  if (CI->use_empty() && II.getRetInfo().rawNullAllowed())
    return -1; // unused result will be V0
  return Num; // it is two addr
}

/***********************************************************************
 * isNot : test whether an instruction is a "not" instruction (an xor with
 *    constant all ones)
 */
bool genx::isNot(Instruction *Inst)
{
  if (Inst->getOpcode() == Instruction::Xor)
    if (auto C = dyn_cast<Constant>(Inst->getOperand(1)))
      if (C->isAllOnesValue())
        return true;
  return false;
}

/***********************************************************************
 * isPredNot : test whether an instruction is a "not" instruction (an xor
 *    with constant all ones) with predicate (i1 or vector of i1) type
 */
bool genx::isPredNot(Instruction *Inst)
{
  if (Inst->getOpcode() == Instruction::Xor)
    if (auto C = dyn_cast<Constant>(Inst->getOperand(1)))
      if (C->isAllOnesValue() && C->getType()->getScalarType()->isIntegerTy(1))
        return true;
  return false;
}

/***********************************************************************
 * isIntNot : test whether an instruction is a "not" instruction (an xor
 *    with constant all ones) with non-predicate type
 */
bool genx::isIntNot(Instruction *Inst)
{
  if (Inst->getOpcode() == Instruction::Xor)
    if (auto C = dyn_cast<Constant>(Inst->getOperand(1)))
      if (C->isAllOnesValue() && !C->getType()->getScalarType()->isIntegerTy(1))
        return true;
  return false;
}

/***********************************************************************
 * ShuffleVectorAnalyzer::getAsSlice : see if the shufflevector is a slice on
 *    operand 0, and if so return the start index, or -1 if it is not a slice
 */
int ShuffleVectorAnalyzer::getAsSlice()
{
  unsigned WholeWidth = SI->getOperand(0)->getType()->getVectorNumElements();
  Constant *Selector = cast<Constant>(SI->getOperand(2));
  unsigned Width = SI->getType()->getVectorNumElements();
  unsigned StartIdx = cast<ConstantInt>(
      Selector->getAggregateElement((unsigned)0))->getZExtValue();
  if (StartIdx >= WholeWidth)
    return -1; // start index beyond operand 0
  unsigned SliceWidth;
  for (SliceWidth = 1; SliceWidth != Width; ++SliceWidth) {
    auto CI = dyn_cast<ConstantInt>(Selector->getAggregateElement(SliceWidth));
    if (!CI)
      break;
    if (CI->getZExtValue() != StartIdx + SliceWidth)
      return -1; // not slice
  }
  return StartIdx;
}

/***********************************************************************
 * ShuffleVectorAnalyzer::getAsUnslice : see if the shufflevector is an
 *    unslice where the "old value" is operand 0 and operand 1 is another
 *    shufflevector and operand 0 of that is the "new value"
 *
 * Return:  start index, or -1 if it is not an unslice
 */
int ShuffleVectorAnalyzer::getAsUnslice()
{
  auto SI2 = dyn_cast<ShuffleVectorInst>(SI->getOperand(1));
  if (!SI2)
    return -1;
  Constant *MaskVec = cast<Constant>(SI->getOperand(2));
  // Find prefix of undef or elements from operand 0.
  unsigned OldWidth = SI2->getType()->getVectorNumElements(); 
  unsigned NewWidth = SI2->getOperand(0)->getType()->getVectorNumElements(); 
  unsigned Prefix = 0;
  for (;; ++Prefix) {
    if (Prefix == OldWidth - NewWidth)
      break;
    Constant *IdxC = MaskVec->getAggregateElement(Prefix);
    if (isa<UndefValue>(IdxC))
      continue;
    unsigned Idx = cast<ConstantInt>(IdxC)->getZExtValue();
    if (Idx == OldWidth)
      break; // found end of prefix
    if (Idx != Prefix)
      return -1; // not part of prefix
  }
  // Check that the whole of SI2 operand 0 follows
  for (unsigned i = 1; i != NewWidth; ++i) {
    Constant *IdxC = MaskVec->getAggregateElement(Prefix + i);
    if (isa<UndefValue>(IdxC))
      continue;
    if (cast<ConstantInt>(IdxC)->getZExtValue() != i + OldWidth)
      return -1; // not got whole of SI2 operand 0
  }
  // Check that the remainder is undef or elements from operand 0.
  for (unsigned i = Prefix + NewWidth; i != OldWidth; ++i) {
    Constant *IdxC = MaskVec->getAggregateElement(i);
    if (isa<UndefValue>(IdxC))
      continue;
    if (cast<ConstantInt>(IdxC)->getZExtValue() != i)
      return -1;
  }
  // Check that the first Prefix elements of SI2 come from its operand 1.
  Constant *MaskVec2 = cast<Constant>(SI2->getOperand(2));
  for (unsigned i = 0; i != Prefix; ++i) {
    Constant *IdxC = MaskVec2->getAggregateElement(Prefix + i);
    if (isa<UndefValue>(IdxC))
      continue;
    if (cast<ConstantInt>(IdxC)->getZExtValue() != i)
      return -1;
  }
  // Success.
  return Prefix;
}

/***********************************************************************
 * ShuffleVectorAnalyzer::getAsSplat : if shufflevector is a splat, get the
 *      splatted input, with its vector index if the input is a vector
 */
ShuffleVectorAnalyzer::SplatInfo ShuffleVectorAnalyzer::getAsSplat()
{
  Value *InVec1 = SI->getOperand(0);
  Value *InVec2 = SI->getOperand(1);
  Constant *MaskVec = cast<Constant>(SI->getOperand(2));
  ConstantInt *IdxVal = dyn_cast_or_null<ConstantInt>(MaskVec->getSplatValue());
  if (!IdxVal)
    return SplatInfo(0, 0);
  // The mask is a splat. Work out which element of which input vector
  // it refers to.
  unsigned ShuffleIdx = IdxVal->getSExtValue();
  unsigned InVec1NumElements = InVec1->getType()->getVectorNumElements();
  if (ShuffleIdx >= InVec1NumElements) {
    ShuffleIdx -= InVec1NumElements;
    InVec1 = InVec2;
  }
  if (auto IE = dyn_cast<InsertElementInst>(InVec1)) {
    if (InVec1NumElements == 1 || isa<UndefValue>(IE->getOperand(0)))
      return SplatInfo(IE->getOperand(1), 0);
    // Even though this is a splat, the input vector has more than one
    // element. IRBuilder::CreateVectorSplat does this. See if the input
    // vector is the result of an insertelement at the right place, and
    // if so return that. Otherwise we end up allocating
    // an unnecessarily large register.
    if (auto ConstIdx = dyn_cast<ConstantInt>(IE->getOperand(2)))
      if (ConstIdx->getSExtValue() == ShuffleIdx)
        return SplatInfo(IE->getOperand(1), 0);
  }
  return SplatInfo(InVec1, ShuffleIdx);
}

Value *ShuffleVectorAnalyzer::serialize() {
  unsigned Cost0 = getSerializeCost(0);
  unsigned Cost1 = getSerializeCost(1);

  Value *Op0 = SI->getOperand(0);
  Value *Op1 = SI->getOperand(1);
  Value *V = Op0;
  bool UseOp0AsBase = Cost0 <= Cost1;
  if (!UseOp0AsBase)
    V = Op1;

  // Expand or shink the initial value if sizes mismatch.
  unsigned NElts = SI->getType()->getVectorNumElements();
  unsigned M = V->getType()->getVectorNumElements();
  bool SkipBase = true;
  if (M != NElts) {
    if (auto C = dyn_cast<Constant>(V)) {
      SmallVector<Constant *, 16> Vals;
      for (unsigned i = 0; i < NElts; ++i) {
        Type *Ty = C->getType()->getVectorElementType();
        Constant *Elt =
            (i < M) ? C->getAggregateElement(i) : UndefValue::get(Ty);
        Vals.push_back(Elt);
      }
      V = ConstantVector::get(Vals);
    } else {
      // Need to insert individual elements.
      V = UndefValue::get(SI->getType());
      SkipBase = false;
    }
  }

  IRBuilder<> Builder(SI);
  for (unsigned i = 0; i < NElts; ++i) {
    // Undef index returns -1.
    int idx = SI->getMaskValue(i);
    if (idx < 0)
      continue;
    if (SkipBase) {
      if (UseOp0AsBase && idx == i)
        continue;
      if (!UseOp0AsBase && idx == i + M)
        continue;
    }

    Value *Vi = nullptr;
    if (idx < M)
      Vi = Builder.CreateExtractElement(Op0, idx, "");
    else
      Vi = Builder.CreateExtractElement(Op1, idx - M, "");
    if (!isa<UndefValue>(Vi))
      V = Builder.CreateInsertElement(V, Vi, i, "");
  }

  return V;
}

unsigned ShuffleVectorAnalyzer::getSerializeCost(unsigned i) {
  unsigned Cost = 0;
  Value *Op = SI->getOperand(i);
  if (!isa<Constant>(Op) && Op->getType() != SI->getType())
    Cost += Op->getType()->getVectorNumElements();

  unsigned NElts = SI->getType()->getVectorNumElements();
  for (unsigned j = 0; j < NElts; ++j) {
    // Undef index returns -1.
    int idx = SI->getMaskValue(j);
    if (idx < 0)
      continue;
    // Count the number of elements out of place.
    unsigned M = Op->getType()->getVectorNumElements();
    if ((i == 0 && idx != j) || (i == 1 && idx != j + M))
      Cost++;
  }

  return Cost;
}

/***********************************************************************
 * adjustPhiNodesForBlockRemoval : adjust phi nodes when removing a block
 *
 * Enter:   Succ = the successor block to adjust phi nodes in
 *          BB = the block being removed
 *
 * This modifies each phi node in Succ as follows: the incoming for BB is
 * replaced by an incoming for each of BB's predecessors.
 */
void genx::adjustPhiNodesForBlockRemoval(BasicBlock *Succ, BasicBlock *BB)
{
  for (auto i = Succ->begin(), e = Succ->end(); i != e; ++i) {
    auto Phi = dyn_cast<PHINode>(&*i);
    if (!Phi)
      break;
    // For this phi node, get the incoming for BB.
    int Idx = Phi->getBasicBlockIndex(BB);
    assert(Idx >= 0);
    Value *Incoming = Phi->getIncomingValue(Idx);
    // Iterate through BB's predecessors. For the first one, replace the
    // incoming block with the predecessor. For subsequent ones, we need
    // to add new phi incomings.
    auto pi = pred_begin(BB), pe = pred_end(BB);
    assert(pi != pe);
    Phi->setIncomingBlock(Idx, *pi);
    for (++pi; pi != pe; ++pi)
      Phi->addIncoming(Incoming, *pi);
  }
}

/***********************************************************************
 * sinkAdd : sink add(s) in address calculation
 *
 * Enter:   IdxVal = the original index value
 *
 * Return:  the new calculation for the index value
 *
 * This detects the case when a variable index in a region or element access
 * is one or more constant add/subs then some mul/shl/truncs. It sinks
 * the add/subs into a single add after the mul/shl/truncs, so the add
 * stands a chance of being baled in as a constant offset in the region.
 *
 * If add sinking is successfully applied, it may leave now unused
 * instructions behind, which need tidying by a later dead code removal
 * pass.
 */
Value *genx::sinkAdd(Value *V) {
  Instruction *IdxVal = dyn_cast<Instruction>(V);
  if (!IdxVal)
    return V;
  // Collect the scale/trunc/add/sub instructions.
  int Offset = 0;
  SmallVector<Instruction *, 8> ScaleInsts;
  Instruction *Inst = IdxVal;
  int Scale = 1;
  bool NeedChange = false;
  for (;;) {
    if (isa<TruncInst>(Inst))
      ScaleInsts.push_back(Inst);
    else {
      if (!isa<BinaryOperator>(Inst))
        break;
      if (ConstantInt *CI = dyn_cast<ConstantInt>(Inst->getOperand(1))) {
        if (Inst->getOpcode() == Instruction::Mul) {
          Scale *= CI->getSExtValue();
          ScaleInsts.push_back(Inst);
        } else if (Inst->getOpcode() == Instruction::Shl) {
          Scale <<= CI->getSExtValue();
          ScaleInsts.push_back(Inst);
        } else if (Inst->getOpcode() == Instruction::Add) {
          Offset += CI->getSExtValue() * Scale;
          if (V != Inst)
            NeedChange = true;
        } else if (Inst->getOpcode() == Instruction::Sub) {
          Offset -= CI->getSExtValue() * Scale;
          if (IdxVal != Inst)
            NeedChange = true;
        } else
          break;
      } else
        break;
    }
    Inst = dyn_cast<Instruction>(Inst->getOperand(0));
    if (!Inst)
      return V;
  }
  if (!NeedChange)
    return V;
  // Clone the scale and trunc instructions, starting with the value that
  // was input to the add(s).
  for (SmallVectorImpl<Instruction *>::reverse_iterator i = ScaleInsts.rbegin(),
                                                        e = ScaleInsts.rend();
       i != e; ++i) {
    Instruction *Clone = (*i)->clone();
    Clone->insertBefore(IdxVal);
    Clone->setName((*i)->getName());
    Clone->setOperand(0, Inst);
    Inst = Clone;
  }
  // Create a new add instruction.
  Inst = BinaryOperator::Create(
      Instruction::Add, Inst,
      ConstantInt::get(Inst->getType(), (int64_t)Offset, true /*isSigned*/),
      Twine("addr_add"), IdxVal);
  Inst->setDebugLoc(IdxVal->getDebugLoc());
  return Inst;
}

/***********************************************************************
* reorderBlocks : reorder blocks to increase fallthrough, and specifically
*    to satisfy the requirements of SIMD control flow
*/
#define SUCCSZANY     (true)
#define SUCCHASINST   (succ->size() > 1)
#define SUCCNOINST    (succ->size() <= 1)
#define SUCCANYLOOP   (true)

#define PUSHSUCC(BLK, C1, C2) \
        for(succ_iterator succIter = succ_begin(BLK), succEnd = succ_end(BLK); \
          succIter!=succEnd; ++succIter) {                                   \
          llvm::BasicBlock *succ = *succIter;                                \
          if (!visitSet.count(succ) && C1 && C2) {                           \
            visitVec.push_back(succ);                                        \
            visitSet.insert(succ);                                           \
            break;                                                           \
          }                                                                  \
        }

void genx::LayoutBlocks(Function &func, LoopInfo &LI)
{
  std::vector<llvm::BasicBlock*> visitVec;
  std::set<llvm::BasicBlock*> visitSet;
  // Insertion Position per loop header
  std::map<llvm::BasicBlock*, llvm::BasicBlock*> InsPos;

  llvm::BasicBlock* entry = &(func.getEntryBlock());
  visitVec.push_back(entry);
  visitSet.insert(entry);
  InsPos[entry] = entry;

  while (!visitVec.empty()) {
    llvm::BasicBlock* blk = visitVec.back();
    llvm::Loop *curLoop = LI.getLoopFor(blk);
    if (curLoop) {
      auto hd = curLoop->getHeader();
      if (blk == hd && InsPos.find(hd) == InsPos.end()) {
        InsPos[blk] = blk;
      }
    }
    // push: time for DFS visit
    PUSHSUCC(blk, SUCCANYLOOP, SUCCNOINST);
    if (blk != visitVec.back())
      continue;
    // push: time for DFS visit
    PUSHSUCC(blk, SUCCANYLOOP, SUCCHASINST);
    // pop: time to move the block to the right location
    if (blk == visitVec.back()) {
      visitVec.pop_back();
      if (curLoop) {
        auto hd = curLoop->getHeader();
        if (blk != hd) {
          // move the block to the beginning of the loop 
          auto insp = InsPos[hd];
          assert(insp);
          if (blk != insp) {
            blk->moveBefore(insp);
            InsPos[hd] = blk;
          }
        }
        else {
          // move the entire loop to the beginning of
          // the parent loop
          auto LoopStart = InsPos[hd];
          assert(LoopStart);
          auto PaLoop = curLoop->getParentLoop();
          auto PaHd = PaLoop ? PaLoop->getHeader() : entry;
          auto insp = InsPos[PaHd];
          if (LoopStart == hd) {
            // single block loop
            hd->moveBefore(insp);
          }
          else {
            // loop-header is not moved yet, so should be at the end
            // use splice
            llvm::Function::BasicBlockListType& BBList = func.getBasicBlockList();
            BBList.splice(insp->getIterator(), BBList, LoopStart->getIterator(),
              hd->getIterator());
            hd->moveBefore(LoopStart);
          }
          InsPos[PaHd] = hd;
        }
      }
      else {
        auto insp = InsPos[entry];
        if (blk != insp) {
          blk->moveBefore(insp);
          InsPos[entry] = blk;
        }
      }
    }
  }
}

void genx::LayoutBlocks(Function &func)
{
  std::vector<llvm::BasicBlock*> visitVec;
  std::set<llvm::BasicBlock*> visitSet;
  // Reorder basic block to allow more fall-through 
  llvm::BasicBlock* entry = &(func.getEntryBlock());
  visitVec.push_back(entry);
  visitSet.insert(entry);

  while (!visitVec.empty()) {
    llvm::BasicBlock* blk = visitVec.back();
    // push in the empty successor 
    PUSHSUCC(blk, SUCCANYLOOP, SUCCNOINST);
    if (blk != visitVec.back())
      continue;
    // push in the other successor 
    PUSHSUCC(blk, SUCCANYLOOP, SUCCHASINST);
    // pop
    if (blk == visitVec.back()) {
      visitVec.pop_back();
      if (blk != entry) {
        blk->moveBefore(entry);
        entry = blk;
      }
    }
  }
}