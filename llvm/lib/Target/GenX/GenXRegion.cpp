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
// Implementation of methods for Region class
//
//===----------------------------------------------------------------------===//

#include "GenXRegion.h"
#include "GenXAlignmentInfo.h"
#include "GenXBaling.h"
#include "GenXSubtarget.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace genx;

/***********************************************************************
 * getWithOffset : get a Region given a rdregion/wrregion, baling in
 *      constant add of offset
 *
 * This constructs the Region with a variable index that is a constant add
 * baled in (i.e. Region::Indirect and Region::Offset both set to the
 * operands of the add). It is for use when baling information is not
 * available, but the caller wants the constant offset separated out like
 * that.
 */
Region Region::getWithOffset(Instruction *Inst, bool WantParentWidth)
{
  unsigned OperandNum = 0;
  switch (getIntrinsicID(Inst)) {
    case Intrinsic::genx_rdregioni:
    case Intrinsic::genx_rdregionf:
      OperandNum = Intrinsic::GenXRegion::RdIndexOperandNum;
      break;
    case Intrinsic::genx_wrregioni:
    case Intrinsic::genx_wrregionf:
    case Intrinsic::genx_wrconstregion:
      OperandNum = Intrinsic::GenXRegion::WrIndexOperandNum;
      break;
    default:
      llvm_unreachable("not rdregion or wrregion");
      break;
  }
  BaleInfo BI;
  if (GenXBaling::isBalableIndexAdd(Inst->getOperand(OperandNum)))
    BI.setOperandBaled(OperandNum);
  return Region(Inst, BI, WantParentWidth);
}

/***********************************************************************
 * Region constructor from a type
 */
Region::Region(Type *Ty)
    : ElementBytes(0), NumElements(1), VStride(0), Width(1),
      Stride(1), Offset(0), Indirect(0), IndirectIdx(0), Mask(0),
      ParentWidth(0)
{
  ElementTy = Ty;
  if (VectorType *VT = dyn_cast<VectorType>(ElementTy)) {
    ElementTy = VT->getElementType();
    NumElements = VT->getNumElements();
    Width = NumElements;
  }
  ElementBytes = ElementTy->getPrimitiveSizeInBits() / 8;
}

/***********************************************************************
 * Region constructor from a value
 *
 * I think C++11 is supposed to let me have a delegated constructor. But
 * VS2012 objects to it. So we have to have a copy of the constructor from
 * Type above.
 */
Region::Region(Value *V)
    : ElementBytes(0), NumElements(1), VStride(0), Width(1),
      Stride(1), Offset(0), Indirect(0), IndirectIdx(0), Mask(0),
      ParentWidth(0)
{
  ElementTy = V->getType();
  if (VectorType *VT = dyn_cast<VectorType>(ElementTy)) {
    ElementTy = VT->getElementType();
    NumElements = VT->getNumElements();
    Width = NumElements;
  }
  ElementBytes = ElementTy->getPrimitiveSizeInBits() / 8;
}

/***********************************************************************
 * Region constructor from a rd/wr region and its BaleInfo
 * This also works with rdpredregion and wrpredregion, with Offset in
 * bits rather than bytes, and with ElementBytes set to 1.
 */
Region::Region(Instruction *Inst, const BaleInfo &BI, bool WantParentWidth)
    : ElementBytes(0), NumElements(1), VStride(1), Width(1),
      Stride(1), Offset(0), Indirect(0), IndirectIdx(0), Mask(0),
      ParentWidth(0)
{
  // Determine where to get the subregion value from and which arg index
  // the region parameters start at.
  unsigned ArgIdx = 0;
  Value *Subregion = 0;
  assert(isa<CallInst>(Inst));
  switch (cast<CallInst>(Inst)->getCalledFunction()->getIntrinsicID()) {
    case Intrinsic::genx_rdpredregion:
      NumElements = Inst->getType()->getVectorNumElements();
      Width = NumElements;
      Offset = cast<ConstantInt>(Inst->getOperand(1))->getZExtValue();
      ElementBytes = 1;
      return;
    case Intrinsic::genx_wrpredregion:
      NumElements = Inst->getOperand(1)->getType()->getVectorNumElements();
      Width = NumElements;
      Offset = cast<ConstantInt>(Inst->getOperand(2))->getZExtValue();
      ElementBytes = 1;
      return;
    case Intrinsic::genx_rdregioni:
    case Intrinsic::genx_rdregionf:
      ArgIdx = 1;
      // The size/type of the region is given by the return value:
      Subregion = Inst;
      break;
    case Intrinsic::genx_wrregioni:
    case Intrinsic::genx_wrregionf:
    case Intrinsic::genx_wrconstregion:
      ArgIdx = 2;
      // The size/type of the region is given by the "subregion value to
      // write" operand:
      Subregion = Inst->getOperand(1);
      // For wrregion, while we're here, also get the mask. We set mask to NULL
      // if the mask operand is constant 1 (i.e. not predicated).
      Mask = Inst->getOperand(Intrinsic::GenXRegion::PredicateOperandNum);
      if (auto C = dyn_cast<Constant>(Mask))
        if (C->isAllOnesValue())
          Mask = 0;
      break;
    default:
      assert(0);
  }
  // Get the region parameters.
  assert(Subregion);
  ElementTy = Subregion->getType();
  if (VectorType *VT = dyn_cast<VectorType>(ElementTy)) {
    ElementTy = VT->getElementType();
    NumElements = VT->getNumElements();
  }
  ElementBytes = ElementTy->getPrimitiveSizeInBits() / 8;
  VStride = cast<ConstantInt>(Inst->getOperand(ArgIdx))->getSExtValue();
  Width = cast<ConstantInt>(Inst->getOperand(ArgIdx + 1))->getSExtValue();
  Stride = cast<ConstantInt>(Inst->getOperand(ArgIdx + 2))->getSExtValue();
  ArgIdx += 3;
  // Get the start index.
  Value *V = Inst->getOperand(ArgIdx);
  assert(V->getType()->getScalarType()->isIntegerTy(16) &&
         "region index must be i16 or vXi16 type");
#if _DEBUG
  if (VectorType *VT = dyn_cast<VectorType>(V->getType()))
    assert(VT->getNumElements() * Width == NumElements &&
           "vector region index size mismatch");
#endif
  if (ConstantInt *CI = dyn_cast<ConstantInt>(V))
    Offset = CI->getSExtValue(); // Constant index.
  else {
    Indirect = V; // Index is variable; assume no baled in add.
    if (BI.isOperandBaled(ArgIdx)) {
      // The index is variable and has something baled in. We want to process
      // a baled in add or add_addr, and ignore a baled in rdregion.
      Instruction *Add = cast<Instruction>(V);
      if (!isRdRegion(getIntrinsicID(Add))) {
        // The index is variable and has a baled in add/sub/add_addr.
        Constant *C = cast<Constant>(Add->getOperand(1));
        ConstantInt *CI = dyn_cast<ConstantInt>(C);
        if (!CI)
          CI = cast<ConstantInt>(C->getSplatValue());
        Offset = CI->getSExtValue();
        if (Add->getOpcode() == Instruction::Sub)
          Offset = -Offset;
        else
          assert(Add->getOpcode() == Instruction::Add
              || getIntrinsicID(Add) == Intrinsic::genx_add_addr);
        Indirect = Add->getOperand(0);
      }
    }
    // For a variable index, get the parent width arg.
    ConstantInt *PW = dyn_cast<ConstantInt>(Inst->getOperand(ArgIdx + 1));
    if (PW)
      ParentWidth = PW->getZExtValue();
  }
  // We do some trivial legalization here. The legalization pass does not
  // make these changes; instead we do them here so they are not permanently
  // written back into the IR but are made on the fly each time some other
  // pass uses this code to get the region info.
  if (NumElements == 1) {
    Width = Stride = 1;
    VStride = 0;
  } else {
    if (NumElements <= Width) {
      Width = NumElements;
      VStride = 0;
    } else if ((unsigned)VStride == Width * Stride) {
      // VStride == Width * Stride, so we can canonicalize to a 1D region,
      // but only if not indirect or not asked to preserve parentwidth,
      // and never if multi-indirect.
      if (!Indirect
          || (!isa<VectorType>(Indirect->getType()) && !WantParentWidth)) {
        Width = NumElements;
        VStride = 0;
        ParentWidth = 0;
      }
    } else if (Width == 1) {
      // We can turn a 2D width 1 region into a 1D region, but if it is
      // indirect it invalidates ParentWidth. So only do it if not asked
      // to keep ParentWidth. Also we cannot do it if it is multi-indirect.
      if (!Indirect
          || (!isa<VectorType>(Indirect->getType()) && !WantParentWidth)) {
        Width = NumElements;
        Stride = VStride;
        VStride = 0;
        ParentWidth = 0;
      }
    }
    if (Stride == 0 && Width == NumElements) {
      // Canonical scalar region.
      Width = 1;
      VStride = 0;
    }
  }
}

/***********************************************************************
 * Region constructor from bitmap of which elements to set
 *
 * Enter:   Bits = bitmap of which elements to set
 *          ElementBytes = bytes per element
 *
 * It is assumed that Bits represents a legal 1D region.
 */
Region::Region(unsigned Bits, unsigned ElementBytes)
    : ElementBytes(ElementBytes), ElementTy(0), NumElements(1), VStride(1),
      Width(1), Stride(1), Offset(0), Indirect(0), IndirectIdx(0), Mask(0)
{
  assert(Bits);
  Offset = countTrailingZeros(Bits, ZB_Undefined);
  Bits >>= Offset;
  Offset *= ElementBytes;
  if (Bits != 1) {
    Stride = countTrailingZeros(Bits & ~1, ZB_Undefined);
    NumElements = Width = countPopulation(Bits);
  }
}

/***********************************************************************
 * Region::getSubregion : modify Region struct for a subregion
 *
 * Enter:   StartIdx = start index of subregion (in elements)
 *          Size = size of subregion (in elements)
 *
 * This does not modify the Mask; the caller needs to do that separately.
 */
void Region::getSubregion(unsigned StartIdx, unsigned Size)
{
  if (Indirect && isa<VectorType>(Indirect->getType())) {
    // Vector indirect (multi indirect). Set IndirectIdx to the index of
    // the start element in the vector indirect.
    IndirectIdx = StartIdx / Width;
    StartIdx %= Width;
  }
  int AddOffset = StartIdx / Width * VStride;
  AddOffset += StartIdx % Width * Stride;
  AddOffset *= ElementBytes;
  Offset += AddOffset;
  if (!(StartIdx % Width) && !(Size % Width)) {
    // StartIdx is at the start of a row and Size is a whole number of
    // rows.
  } else if (StartIdx % Width + Size > Width) {
    // The subregion goes over a row boundary. This can only happen if there
    // is only one row split and it is exactly in the middle.
    VStride += (Size / 2 - Width) * Stride;
    Width = Size / 2;
  } else {
    // Within a single row.
    Width = Size;
    VStride = Size * Stride;
  }
  NumElements = Size;
}

/***********************************************************************
 * Region::createRdRegion : create rdregion intrinsic from "this" Region
 *
 * Enter:   Input = vector value to extract subregion from
 *          Name = name for new instruction
 *          InsertBefore = insert new inst before this point
 *          DL = DebugLoc to give the new instruction
 *          AllowScalar = true to return scalar if region is size 1
 *
 * Return:  newly created instruction
 */
Instruction *Region::createRdRegion(Value *Input, const Twine &Name,
    Instruction *InsertBefore, const DebugLoc &DL, bool AllowScalar)
{
  assert(ElementBytes && "not expecting i1 element type");
  Value *StartIdx = getStartIdx(Name, InsertBefore, DL);
  IntegerType *I32Ty = Type::getInt32Ty(Input->getContext());
  Value *ParentWidthArg = UndefValue::get(I32Ty);
  if (Indirect)
    ParentWidthArg = ConstantInt::get(I32Ty, ParentWidth);
  Value *Args[] = {   // Args to new rdregion:
      Input, // input to original rdregion
      ConstantInt::get(I32Ty, VStride), // vstride
      ConstantInt::get(I32Ty, Width), // width
      ConstantInt::get(I32Ty, Stride), // stride
      StartIdx, // start index (in bytes)
      ParentWidthArg // parent width (if variable start index)
  };
  Type *ElTy = cast<VectorType>(Args[0]->getType())->getElementType();
  Type *RegionTy;
  if (NumElements != 1 || !AllowScalar)
    RegionTy = VectorType::get(ElTy, NumElements);
  else
    RegionTy = ElTy;
  Module *M = InsertBefore->getParent()->getParent()->getParent();
  unsigned IID = ElTy->isFloatingPointTy()
      ? Intrinsic::genx_rdregionf : Intrinsic::genx_rdregioni;
  Function *Decl = getRegionDeclaration(M, IID, RegionTy, Args);
  Instruction *NewInst = CallInst::Create(Decl, Args, Name, InsertBefore);
  NewInst->setDebugLoc(DL);
  return NewInst;
}

/***********************************************************************
 * Region::createWrRegion : create wrregion instruction for subregion
 * Region::createWrConstRegion : create wrconstregion instruction for subregion
 *
 * Enter:   OldVal = vector value to insert subregion into (can be undef)
 *          Input = subregion value to insert (can be scalar, as long as
 *                  region size is 1)
 *          Name = name for new instruction
 *          InsertBefore = insert new inst before this point
 *          DL = DebugLoc to give any new instruction
 *
 * Return:  The new wrregion instruction. However, if it would have had a
 *          predication mask of all 0s, it is omitted and OldVal is returned
 *          instead.
 */
Value *Region::createWrRegion(Value *OldVal, Value *Input,
    const Twine &Name, Instruction *InsertBefore, const DebugLoc &DL)
{
  return createWrCommonRegion(OldVal->getType()->isFPOrFPVectorTy()
        ? Intrinsic::genx_wrregionf : Intrinsic::genx_wrregioni,
      OldVal, Input,
      Name, InsertBefore, DL);
}

Value *Region::createWrConstRegion(Value *OldVal, Value *Input,
    const Twine &Name, Instruction *InsertBefore, const DebugLoc &DL)
{
  assert(!Indirect);
  assert(!Mask);
  assert(isa<Constant>(Input));
  return createWrCommonRegion(Intrinsic::genx_wrconstregion, OldVal, Input,
      Name, InsertBefore, DL);
}

Value *Region::createWrCommonRegion(unsigned IID, Value *OldVal, Value *Input,
    const Twine &Name, Instruction *InsertBefore, const DebugLoc &DL)
{
  assert(ElementBytes && "not expecting i1 element type");
  assert(isa<VectorType>(Input->getType()) || NumElements == 1);
  assert(OldVal->getType()->getScalarType() ==
             Input->getType()->getScalarType() &&
         "scalar type mismatch");
  Value *StartIdx = getStartIdx(Name, InsertBefore, DL);
  IntegerType *I32Ty = Type::getInt32Ty(Input->getContext());
  Value *ParentWidthArg = UndefValue::get(I32Ty);
  if (Indirect)
    ParentWidthArg = ConstantInt::get(I32Ty, ParentWidth);
  // Get the mask value. If R.Mask is 0, then the wrregion is unpredicated
  // and we just use constant 1.
  Value *MaskArg = Mask;
  if (!MaskArg)
    MaskArg = ConstantInt::get(Type::getInt1Ty(Input->getContext()), 1);
  // Build the wrregion.
  Value *Args[] = {   // Args to new wrregion:
      OldVal, // original vector
      Input, // value to write into subregion
      ConstantInt::get(I32Ty, VStride), // vstride
      ConstantInt::get(I32Ty, Width), // width
      ConstantInt::get(I32Ty, Stride), // stride
      StartIdx, // start index (in bytes)
      ParentWidthArg, // parent width (if variable start index)
      MaskArg // mask
  };
  Module *M = InsertBefore->getParent()->getParent()->getParent();
  Function *Decl = getRegionDeclaration(M, IID, nullptr, Args);
  Instruction *NewInst = CallInst::Create(Decl, Args, Name, InsertBefore);
  NewInst->setDebugLoc(DL);
  return NewInst;
}

/***********************************************************************
 * Region::createRdPredRegion : create rdpredregion instruction
 * Region::createRdPredRegionOrConst : create rdpredregion instruction, or
 *      simplify to constant
 *
 * Enter:   Input = vector value to extract subregion from
 *          Index = start index of subregion
 *          Size = size of subregion
 *          Name = name for new instruction
 *          InsertBefore = insert new inst before this point
 *          DL = DebugLoc to give any new instruction
 *
 * Return:  The new rdpredregion instruction
 *
 * Unlike createRdRegion, this is a static method in Region, because you pass
 * the region parameters (the start index and size) directly into this method.
 */
Instruction *Region::createRdPredRegion(Value *Input, unsigned Index,
    unsigned Size, const Twine &Name, Instruction *InsertBefore,
    const DebugLoc &DL)
{
  Type *I32Ty = Type::getInt32Ty(InsertBefore->getContext());
  Value *Args[] = { // Args to new rdpredregion call:
    Input, // input predicate
    ConstantInt::get(I32Ty, Index) // start offset
  };
  auto RetTy = VectorType::get(Args[0]->getType()->getScalarType(), Size);
  Module *M = InsertBefore->getParent()->getParent()->getParent();
  Function *Decl = getRegionDeclaration(M, Intrinsic::genx_rdpredregion,
      RetTy, Args);
  Instruction *NewInst = CallInst::Create(Decl, Args, Name, InsertBefore);
  NewInst->setDebugLoc(DL);
  if (NewInst->getName() == "phitmp18.i.i.split0")
    dbgs() << "wobble\n";
  return NewInst;
}

Value *Region::createRdPredRegionOrConst(Value *Input, unsigned Index,
    unsigned Size, const Twine &Name, Instruction *InsertBefore,
    const DebugLoc &DL)
{
  if (auto C = dyn_cast<Constant>(Input))
    return getConstantSubvector(C, Index, Size);
  return createRdPredRegion(Input, Index, Size, Name, InsertBefore, DL);
}

/***********************************************************************
 * Region::createWrPredRegion : create wrpredregion instruction
 *
 * Enter:   OldVal = vector value to insert subregion into (can be undef)
 *          Input = subregion value to insert
 *          Index = start index of subregion
 *          Name = name for new instruction
 *          InsertBefore = insert new inst before this point
 *          DL = DebugLoc to give any new instruction
 *
 * Return:  The new wrpredregion instruction
 *
 * Unlike createWrRegion, this is a static method in Region, because you pass
 * the only region parameter (the start index) directly into this method.
 */
Instruction *Region::createWrPredRegion(Value *OldVal, Value *Input,
    unsigned Index, const Twine &Name, Instruction *InsertBefore,
    const DebugLoc &DL)
{
  IntegerType *I32Ty = Type::getInt32Ty(Input->getContext());
  Value *Args[] = {   // Args to new wrpredregion:
      OldVal, // original vector
      Input, // value to write into subregion
      ConstantInt::get(I32Ty, Index), // start index
  };
  Module *M = InsertBefore->getParent()->getParent()->getParent();
  Function *Decl = getRegionDeclaration(M, Intrinsic::genx_wrpredregion,
      nullptr, Args);
  Instruction *NewInst = CallInst::Create(Decl, Args, Name, InsertBefore);
  NewInst->setDebugLoc(DL);
  return NewInst;
}

/***********************************************************************
 * Region::createWrPredPredRegion : create wrpredpredregion instruction
 *
 * Enter:   OldVal = vector value to insert subregion into (can be undef)
 *          Input = subregion value to insert
 *          Index = start index of subregion
 *          Pred = predicate for the write region
 *          Name = name for new instruction
 *          InsertBefore = insert new inst before this point
 *          DL = DebugLoc to give any new instruction
 *
 * Return:  The new wrpredpredregion instruction
 *
 * Unlike createWrRegion, this is a static method in Region, because you pass
 * the only region parameter (the start index) directly into this method.
 */
Instruction *Region::createWrPredPredRegion(Value *OldVal, Value *Input,
    unsigned Index, Value *Pred, const Twine &Name, Instruction *InsertBefore,
    const DebugLoc &DL)
{
  Type *Tys[] = { OldVal->getType(), Input->getType() };
  Function *CalledFunc = Intrinsic::getDeclaration(
      InsertBefore->getParent()->getParent()->getParent(),
      Intrinsic::genx_wrpredpredregion, Tys);
  Value *Args[] = { OldVal, Input, 
      ConstantInt::get(Type::getInt32Ty(InsertBefore->getContext()), Index),
      Pred };
  auto NewInst = CallInst::Create(CalledFunc, Args, "", InsertBefore);
  NewInst->setDebugLoc(DL);
  return NewInst;
}

/***********************************************************************
 * setRegionCalledFunc : for an existing rdregion/wrregion call, modify
 *      its called function to match its operand types
 *
 * This is used in GenXLegalization after modifying a wrregion operand
 * such that its type changes. The called function then needs to change
 * because it is decorated with overloaded types.
 */
void Region::setRegionCalledFunc(Instruction *Inst)
{
  auto CI = cast<CallInst>(Inst);
  SmallVector<Value *, 8> Opnds;
  for (unsigned i = 0, e = CI->getNumArgOperands(); i != e; ++i)
    Opnds.push_back(CI->getOperand(i));
  Function *Decl = getRegionDeclaration(
      Inst->getParent()->getParent()->getParent(), getIntrinsicID(Inst),
      Inst->getType(), Opnds);
  CI->setOperand(CI->getNumArgOperands(), Decl);
}

/***********************************************************************
 * getRegionDeclaration : get the function declaration for a region intrinsic
 *
 * Enter:   M = Module
 *          IID = intrinsic ID
 *          RetTy = return type (can be 0 if return type not overloaded)
 *          Args = array of operands so we can determine overloaded types
 *
 * Return:  the Function
 */
Function *Region::getRegionDeclaration(Module *M,
    unsigned IID, Type *RetTy, ArrayRef<Value *> Args)
{
  switch (IID) {
    case Intrinsic::genx_rdregioni:
    case Intrinsic::genx_rdregionf: {
      Type *Tys[] = { RetTy, Args[0]->getType(), Args[4]->getType() };
      return Intrinsic::getDeclaration(M, (Intrinsic::ID)IID, Tys);
    }
    case Intrinsic::genx_wrregioni:
    case Intrinsic::genx_wrregionf:
    case Intrinsic::genx_wrconstregion: {
      Type *Tys[] = { Args[0]->getType(), Args[1]->getType(),
          Args[5]->getType(), Args[7]->getType() };
      return Intrinsic::getDeclaration(M, (Intrinsic::ID)IID, Tys);
    }
    case Intrinsic::genx_rdpredregion: {
      Type *Tys[] = { RetTy, Args[0]->getType() };
      return Intrinsic::getDeclaration(M, (Intrinsic::ID)IID, Tys);
    }
    case Intrinsic::genx_wrpredregion: {
      Type *Tys[] = { Args[0]->getType(), Args[1]->getType() };
      return Intrinsic::getDeclaration(M, (Intrinsic::ID)IID, Tys);
    }
    default:
      llvm_unreachable("unrecognized region intrinsic ID");
  }
  return nullptr;
}

/***********************************************************************
 * getStartIdx : get the LLVM IR Value for the start index of a region
 *
 * This is common code used by both createRdRegion and createWrRegion.
 */
Value *Region::getStartIdx(const Twine &Name, Instruction *InsertBefore,
    const DebugLoc &DL)
{
  IntegerType *I16Ty = Type::getInt16Ty(InsertBefore->getContext());
  if (!Indirect)
    return ConstantInt::get(I16Ty, Offset);
  // Deal with indirect (variable index) region.
  if (auto VT = dyn_cast<VectorType>(Indirect->getType())) {
    if (VT->getNumElements() != NumElements) {
      // We have a vector indirect and we need to take a subregion of it.
      Region IdxRegion(Indirect);
      IdxRegion.getSubregion(IndirectIdx, NumElements / Width);
      Indirect = IdxRegion.createRdRegion(Indirect,
          Name + ".multiindirect_idx_subregion", InsertBefore, DL);
      IndirectIdx = 0;
    }
  }
  Value *Index = Indirect;
  if (Offset) {
    Constant *OffsetVal = ConstantInt::get(I16Ty, Offset);
    if (auto VT = dyn_cast<VectorType>(Indirect->getType()))
      OffsetVal = ConstantVector::getSplat(VT->getNumElements(), OffsetVal);
    auto BO = BinaryOperator::Create(Instruction::Add, Index, OffsetVal,
        Name + ".indirect_idx_add", InsertBefore);
    BO->setDebugLoc(DL);
    Index = BO;
  }
  return Index;
}

/***********************************************************************
 * isSimilar : compare two regions to see if they have the same region
 *      parameters other than start offset, also allowing element type to
 *      be different
 */
bool Region::isSimilar(const Region &R2) const
{
  if (ElementBytes == R2.ElementBytes)
    return isStrictlySimilar(R2);
  // Change the element type to match, so we can compare the regions.
  Region R = R2;
  if (!R.changeElementType(ElementTy))
    return false;
  return isStrictlySimilar(R);
}

class ByteBitmapGenerator {
  const Region &Rg;
  int Offset, UpBound;
  SmallVector<uint64_t, 16> BitMap;
public:
  ByteBitmapGenerator(const Region &_R, int Off) : Rg(_R), Offset(Off) {
    assert(Rg.ElementBytes == 1 || Rg.ElementBytes == 2 ||
           Rg.ElementBytes == 4 || Rg.ElementBytes == 8);
    uint64_t BitMask = (Rg.ElementBytes == 1) ? 0x01ULL :
                       (Rg.ElementBytes == 2) ? 0x03ULL :
                       (Rg.ElementBytes == 4) ? 0x0FULL :
                       (Rg.ElementBytes == 8) ? 0xFFULL : 0;
    UpBound = 0;
    unsigned Row = 0;
    unsigned Col = 0;
    for (unsigned i = 0, e = Rg.NumElements; i != e; ++i) {
      int L = Rg.ElementBytes * ((Row * Rg.VStride) + (Col * Rg.Stride));
      int R = L + Rg.ElementBytes - 1;
      UpBound = std::max(UpBound, R);
      unsigned L64 = L / 64;
      unsigned Lsh = L % 64;
      unsigned R64 = R / 64;
      unsigned Rsh = R % 64;
      while (R64 >= BitMap.size())
        BitMap.push_back(0ULL);
      BitMap[L64] |= BitMask << Lsh;
      if (Rsh < 63) BitMap[R64] |= BitMask >> (63 - Rsh);
      ++Col;
      if (Col >= Rg.Width) {
        ++Row;
        Col = 0;
      }
    }
    UpBound += Offset;
  }
  std::tuple<uint64_t, bool> getNext() {
    if (Offset > UpBound)
      return std::make_tuple(0, true);
    uint64_t M = 0;
    if (Offset >= Rg.Offset) {
      int L = Offset - Rg.Offset;
      unsigned L64 = L / 64;
      unsigned Lsh = L % 64;
      if (L64 < BitMap.size())
        M |= BitMap[L64] >> Lsh;
    }
    int N = Offset + 63;
    if (N >= Rg.Offset) {
      int R = N - Rg.Offset;
      unsigned R64 = R / 64;
      unsigned Rsh = R % 64;
      if (R64 < BitMap.size())
        if (Rsh < 63) M |= BitMap[R64] << (63 - Rsh);
    }
    Offset = N + 1;
    return std::make_tuple(M, false);
  }
};

// overlap: Compare two regions to see whether they overlaps each other.
//
bool Region::overlap(const Region &R2) const {
  // To be conservative, if any of them is indirect, they overlaps.
  if (Indirect || R2.Indirect)
    return true;
  // To be conservative, if different masks are used, they overlaps.
  if (Mask != R2.Mask)
    return true;
  int MinOff = std::min(Offset, R2.Offset);
  ByteBitmapGenerator G1(*this, MinOff);
  ByteBitmapGenerator G2(R2, MinOff);
  bool Overlap = false;
  bool EOM1 = false, EOM2 = false;
  do {
    uint64_t M1 = 0, M2 = 0;
    std::tie(M1, EOM1) = G1.getNext();
    std::tie(M2, EOM2) = G2.getNext();
    Overlap = (M1 & M2) != 0;
  } while (!Overlap && !EOM1 && !EOM2);
  return Overlap;
}

/***********************************************************************
 * Region::isContiguous : test whether a region is contiguous
 */
bool Region::isContiguous() const
{
  if (Width != 1 && Stride != 1)
    return false;
  if (Width != NumElements && VStride != (int)Width)
    return false;
  return true;
}

/***********************************************************************
 * Region::isWhole : test whether a region covers exactly the whole of the
 *      given type, allowing for the element type being different
 */
bool Region::isWhole(Type *Ty) const
{
  return isContiguous() && NumElements * ElementBytes * 8
      == Ty->getPrimitiveSizeInBits();
}

/***********************************************************************
 * evaluateConstantRdRegion : evaluate rdregion with constant input
 */
Constant *Region::evaluateConstantRdRegion(Constant *Input, bool AllowScalar)
{
  assert(!Indirect);
  if (NumElements != 1)
    AllowScalar = false;
  if (Constant *SV = Input->getSplatValue()) {
    if (AllowScalar)
      return SV;
    return ConstantVector::getSplat(NumElements, SV);
  }
  auto VT = cast<VectorType>(Input->getType());
  SmallVector<Constant *, 8> Values;
  Constant *Undef = UndefValue::get(AllowScalar
      ? ElementTy : VectorType::get(ElementTy, NumElements));
  if (isa<UndefValue>(Input))
    return Undef;
  unsigned RowIdx = Offset / ElementBytes;
  unsigned Idx = RowIdx;
  unsigned NextRow = Width;
  for (unsigned i = 0; i != NumElements; ++i) {
    if (i == NextRow) {
      RowIdx += VStride;
      Idx = RowIdx;
    }
    if (Idx >= VT->getNumElements())
      return Undef; // out of range index
    // Get the element value and push it into Values.
    if (ConstantDataVector *CDV = dyn_cast<ConstantDataVector>(Input))
      Values.push_back(CDV->getElementAsConstant(Idx));
    else {
      auto CV = cast<ConstantVector>(Input);
      Values.push_back(CV->getOperand(Idx));
    }
    Idx += Stride;
  }
  if (AllowScalar)
    return Values[0];
  return ConstantVector::get(Values);
}

/***********************************************************************
 * evaluateConstantWrRegion : evaluate wrregion with constant inputs
 */
Constant *Region::evaluateConstantWrRegion(Constant *OldVal, Constant *NewVal)
{
  assert(!Indirect);
  SmallVector<Constant *, 8> Vec;
  for (unsigned i = 0, e = OldVal->getType()->getVectorNumElements();
      i != e; ++i)
    Vec.push_back(OldVal->getAggregateElement(i));
  unsigned Off = Offset / ElementBytes, Row = Off;
  auto NewVT = dyn_cast<VectorType>(NewVal->getType());
  unsigned NewNumEls = !NewVT ? 1 : NewVT->getNumElements();
  for (unsigned i = 0;;) {
    if (Off >= Vec.size())
      return UndefValue::get(OldVal->getType()); // out of range
    Vec[Off] = !NewVT ? NewVal : NewVal->getAggregateElement(i);
    if (++i == NewNumEls)
      break;
    if (i % Width) {
      Off += Stride;
      continue;
    }
    Row += VStride;
    Off = Row;
  }
  return ConstantVector::get(Vec);
}

/***********************************************************************
 * Region::getLegalSize : get the max legal size of a region
 *
 * Enter:   Idx = start index into the subregion
 *          Allow2D = whether to allow 2D region
 *          InputNumElements = number of elements in whole input vector (so
 *                we can tell if it is small enough that it cannot possibly
 *                cross a GRF boundary)
 *          ST = GenXSubtarget (so we can get gen specific crossing rules)
 *          AI = 0 else AlignmentInfo (to determine alignment of indirect index)
 */
unsigned Region::getLegalSize(unsigned Idx, bool Allow2D,
    unsigned InputNumElements, const GenXSubtarget *ST, AlignmentInfo *AI)
{
  Alignment Align;
  if (Indirect) {
    Align = Alignment::getUnknown();
    if (AI)
      Align = AI->get(Indirect);
  }
  return getLegalSize(Idx, Allow2D, InputNumElements, ST, Align);
}

/***********************************************************************
 * Region::getLegalSize : get the max legal size of a region
 *
 * Enter:   Idx = start index into the subregion
 *          Allow2D = whether to allow 2D region
 *          InputNumElements = number of elements in whole input vector (so
 *                we can tell if it is small enough that it cannot possibly
 *                cross a GRF boundary)
 *          ST = GenXSubtarget (so we can get gen specific crossing rules)
 *          Align = alignment of indirect index if any
 *
 * The setting of Indirect is used as follows:
 *
 * 0: not indirect
 * anything of scalar type: single indirect
 * anything of vector type: multi indirect
 */
unsigned Region::getLegalSize(unsigned Idx, bool Allow2D,
    unsigned InputNumElements, const GenXSubtarget *ST, Alignment Align)
{
  // Determine the max valid width.
  unsigned ValidWidth = 1;
  if ((!Stride || exactLog2(Stride) >= 0) && (Allow2D || Stride <= 4)) {
    // The stride is legal, so we can potentially do more than one element at a
    // time.
    // Disallow 2D if the stride is too large for a real Gen region. For a
    // source operand (Allow2D is true), we allow a 1D region with stride too
    // large, because the vISA writer turns it into a 2D region with width 1.
    bool StrideValid = Stride <= 4;

    if (Indirect && isa<VectorType>(Indirect->getType())) {
      // Multi indirect.
      if (!Allow2D) {
        // Multi indirect not allowed in wrregion.
        if (!Stride)
          ValidWidth = 1 << llvm::log2(Width);
      } else if (Width == 1 || !Stride) {
        // Multi indirect with width 1 or stride 0.
        // Return the max power of two number of elements that:
        // 1. fit in 2 GRFs; and
        // 2. fit in the whole region; and
        // 3. fit in a row if the width is not legal
        // 4. no more than 8 elements in multi indirect (because there
        //    are only 8 elements in an address register).
        unsigned LogWidth = llvm::log2(Width);
        if (1U << LogWidth == Width)
          LogWidth = llvm::log2(NumElements); // legal width
        unsigned LogElementBytes = llvm::log2(ElementBytes);
        if (LogWidth + LogElementBytes > 6 /*log(64)*/)
          LogWidth = 6 - LogElementBytes;
        ValidWidth = 1 << LogWidth;
        if (ValidWidth > 8)
          ValidWidth = 8;
      }
      // Other multi indirect can only do one element.
    } else {
      // Calculate number of elements up to the boundary imposed by GRF
      // crossing rules.
      unsigned ElementsPerGRF = 32 / ElementBytes;
      unsigned OffsetElements = Offset / ElementBytes;
      unsigned ElementsToBoundary = 1;
      unsigned RealIdx = Idx / Width * VStride + Idx % Width * Stride;
      if (!Indirect) {
        // For a direct operand, just use the constant offset of the
        // region and the index so far to calculate how far into a GRF this
        // subregion starts, and set the boundary at the next-but-one GRF
        // boundary.
        ElementsToBoundary = (2 * ElementsPerGRF)
            - ((RealIdx + OffsetElements) % ElementsPerGRF);
      } else if (InputNumElements <= ElementsPerGRF) {
        // Indirect region but the whole vector is no bigger than a GRF, so
        // there is no limit imposed by GRF crossing.
        ElementsToBoundary = ElementsPerGRF;
      } else {
        // For an indirect region, calculate the min and max index (including
        // offset) from the region parameters, and add on the current start
        // index to both.
        // For <= BDW:
        //   1. If the min and max then are in the same GRF, then the distance
        //      from max to the next GRF boundary is the allowed size.
        // For >= SKL:
        //   1. If the min and max then are in the same GRF, then the distance
        //      from max to the next-but-one GRF boundary is the allowed size.
        //   2. If the max is in the next GRF after min, then the distance
        //      from max to the next GRF boundary is the allowed size.
        // However vISA imposes the restriction that, in a source indirect
        // region, a row cannot cross a GRF, unless the region is contiguous.
        // Pending a proper fix, we have a temporary fix here that we disallow
        // GRF crossing completely unless the original region is a destination
        // operand or is a 1D source operand (so GenXVisaFuncWriter can turn it
        // into Nx1 instead of 1xN).  We use Allow2D as a proxy for "is source
        // operand".
        unsigned GRFsPerIndirect = 1;
        if (ST->hasIndirectGRFCrossing()) {
          // SKL+. See if we can allow GRF crossing.
          if (Allow2D || !is2D())
            GRFsPerIndirect = 2;
        }
        unsigned Last = (NumElements / Width - 1) * VStride + (Width - 1) * Stride;
        unsigned Max = InputNumElements - Last - 1 + RealIdx;
        unsigned Min = RealIdx;
        unsigned MinMaxGRFDiff = (Max & -ElementsPerGRF) - (Min & -ElementsPerGRF);
        if (!MinMaxGRFDiff) // min and max in same GRF
          ElementsToBoundary = ElementsPerGRF * GRFsPerIndirect
              - (Max & (ElementsPerGRF - 1));
        else if (MinMaxGRFDiff == 1 && GRFsPerIndirect > 1)
          ElementsToBoundary = ElementsPerGRF - (Max & (ElementsPerGRF - 1));
        // We may be able to refine an indirect region legal width further...
        if (exactLog2(ParentWidth) >= 0
            && ParentWidth <= ElementsPerGRF) {
          // ParentWidth tells us that a row of our region cannot cross a GRF
          // boundary. Say that the boundary is at the next multiple of
          // ParentWidth.
          ElementsToBoundary = std::max(ParentWidth - RealIdx % ParentWidth,
                ElementsToBoundary);
        } else if (!isa<VectorType>(Indirect->getType())) {
          // Use the alignment+offset of the single indirect index, with alignment
          // limited to 32 bytes (one GRF).
          if (!Align.isUnknown()) {
            unsigned LogAlign = Align.getLogAlign();
            unsigned ExtraBits = Align.getExtraBits();
            ExtraBits += (Offset + RealIdx * ElementBytes);
            ExtraBits &= ((1 << LogAlign) - 1);
            if (LogAlign >= 5 && !ExtraBits) {
              // Start is GRF aligned, so legal width is 1 GRF for <=BDW or
              // 2 GRFs for >=SKL.
              ElementsToBoundary = ElementsPerGRF * GRFsPerIndirect;
            } else if (LogAlign > (unsigned)llvm::log2(ElementBytes) ||
                       (LogAlign == (unsigned)llvm::log2(ElementBytes) &&
                        ExtraBits == 0)) {
              LogAlign = std::min(5U, LogAlign) - llvm::log2(ElementBytes);
              ExtraBits = (ExtraBits & 31) >> llvm::log2(ElementBytes);
              // We have some alignment, so we can say that the next GRF boundary
              // is (at least) that many elements away, minus the offset from that
              // alignment.
              // For SKL+, we can cross one GRF boundary, so add on one GRF's
              // worth.
              unsigned ElementsToBoundaryFromAlign = (1U << LogAlign) - ExtraBits;
              ElementsToBoundaryFromAlign += (GRFsPerIndirect - 1) * ElementsPerGRF;
              ElementsToBoundary = std::max(ElementsToBoundaryFromAlign,
                  ElementsToBoundary);
            }
          }
        }
      }

      // Now calculate what subregion we can fit in before the boundary
      // calculated above.
      if (Allow2D && StrideValid) {
        if ((!VStride || exactLog2(VStride) >= 0) && exactLog2(Width) >= 0
            && Width <= 16 && !(Idx % Width)
            && ElementsToBoundary >= (Width - 1) * Stride + 1) {
          // The vstride and width are legal, and we're at the start of a
          // row, and ElementsToBoundary is big enough for at least one
          // whole row, so we can potentially do more than one whole row at a
          // time. See how many we can fit, without including the "slack"
          // at the end of the last row.
          unsigned NumRows = 0;
          if (VStride == 0) // Avoid divide by 0
            NumRows = (NumElements - Idx) / Width;
          else {
            unsigned LastElementOfRow = (Width - 1) * Stride;
            unsigned Slack = VStride - (LastElementOfRow + 1);
            NumRows = (ElementsToBoundary + Slack) / VStride;
            if (NumRows) {
              if (NumRows * Width + Idx > NumElements)
                NumRows = (NumElements - Idx) / Width;
            }
          }
          ValidWidth = (1 << llvm::log2(NumRows)) * Width;
        }
        if (ValidWidth == 1 && Idx % Width) {
          // That failed. See if we can legally get to the end of the row then
          // the same number of elements again at the start of the next row.
          unsigned ToEndOfRow = Width - Idx % Width;
          if (exactLog2(ToEndOfRow) >= 0 && ToEndOfRow <= 16) {
            unsigned NewVStride = VStride + (ToEndOfRow - Width) * Stride;
            if (exactLog2(NewVStride) >= 0
                && NewVStride + (ToEndOfRow - 1) * Stride < ElementsToBoundary) {
              // Yes, we can do the end of one row and the same size start of
              // the next row.
              ValidWidth = 2 * ToEndOfRow;
            }
          }
        }
      }
      if (ValidWidth == 1) {
        // That failed. See how many elements we can get, no further than the
        // next end of row.
        ValidWidth = Width - Idx % Width;
        if (ValidWidth * Stride - (Stride - 1) > ElementsToBoundary)
          ValidWidth = (ElementsToBoundary + Stride - 1) / Stride;
        ValidWidth = 1 << llvm::log2(ValidWidth);
      }
      // If the RStride is 0 (which is seen in splat operations) then the
      // above logic tends to determine that all of the elements can fit,
      // irrespective of vector size and type. This is usually incorrect
      // in the wider context, so clamp it here to whatever fits in 2GRF if
      // necessary
      if (ValidWidth > (2 * ElementsPerGRF))
        ValidWidth = 2 * ElementsPerGRF;

    }
  }
  return ValidWidth;
}

/***********************************************************************
 * Region::changeElementType : change element type of the region
 *
 * Return:  true if succeeded, false if failed (nothing altered)
 */
bool Region::changeElementType(Type *NewElementType)
{
  unsigned NewElementBytes = NewElementType->getPrimitiveSizeInBits() / 8U;
  int LogRatio = llvm::log2(NewElementBytes) - llvm::log2(ElementBytes);
  if (!LogRatio) {
    // No change in element size
    ElementTy = NewElementType;
    return true;
  }
  if (LogRatio >= 0) {
    // Trying to make the element size bigger.
    if (Width & ((1 << LogRatio) - 1))
      return false; // width misaligned
    if (Stride != 1)
      return false; // rows not contiguous
    NumElements >>= LogRatio;
    Width >>= LogRatio;
    VStride >>= LogRatio;
    if (Width == 1) {
      // Width is now 1, so turn it into a 1D region.
      Stride = VStride;
      VStride = 0;
      Width = NumElements;
    }
    ElementTy = NewElementType;
    ElementBytes = NewElementBytes;
    return true;
  }
  // Trying to make the element size smaller.
  LogRatio = -LogRatio;
  if (Stride == 1 || Width == 1) {
    // Row contiguous.
    Stride = 1;
    NumElements <<= LogRatio;
    Width <<= LogRatio;
    VStride <<= LogRatio;
    ElementTy = NewElementType;
    ElementBytes = NewElementBytes;
    return true;
  }
  if (!is2D()) {
    // 1D and not contiguous. Turn it into a 2D region.
    VStride = Stride << LogRatio;
    Stride = 1;
    Width = 1 << LogRatio;
    NumElements <<= LogRatio;
    ElementTy = NewElementType;
    ElementBytes = NewElementBytes;
    return true;
  }
  return false;
}

/***********************************************************************
 * Region::append : append region AR to this region
 *
 * Return:  true if succeeded (this region modified)
 *          false if not possible to append (this region in indeterminate state)
 *
 * This succeeds even if it leaves this region in an illegal state where
 * it has a non-integral number of rows. After doing a sequence of appends,
 * the caller needs to check that the resulting region is legal by calling
 * isWholeNumRows().
 */
bool Region::append(Region AR)
{
  assert(AR.isWholeNumRows());
  if (Indirect != AR.Indirect)
    return false;
  unsigned ARNumRows = AR.NumElements / AR.Width;
  // Consider each row of AR separately.
  for (unsigned ARRow = 0; ARRow != ARNumRows;
      ++ARRow, AR.Offset += AR.VStride * AR.ElementBytes) {
    if (NumElements == Width) {
      // This region is currently 1D.
      if (NumElements == 1)
        Stride = (AR.Offset - Offset) / ElementBytes;
      else if (AR.Width != 1 && Stride != AR.Stride)
        return false; // Mismatched stride.
      int NextOffset = Offset + Width * Stride * ElementBytes;
      if (AR.Offset == NextOffset) {
        // AR is a continuation of the same single row.
        Width += AR.Width;
        NumElements = Width;
        continue;
      }
      // AR is the start (or whole) of a second row.
      if (AR.Width > Width)
        return false; // AR row is bigger than this row.
      VStride = (AR.Offset - Offset) / ElementBytes;
      NumElements += AR.Width;
      continue;
    }
    // This region is already 2D.
    unsigned ExtraBit = NumElements % Width;
    int NextOffset = Offset + ((VStride * (NumElements / Width))
        + ExtraBit) * ElementBytes;
    if (NextOffset != AR.Offset)
      return false; // Mismatched next offset.
    if (AR.Width > Width - ExtraBit)
      return false; // Too much to fill whole row, or remainder of row after
                    //   existing extra bit.
    if (AR.Width != 1 && AR.Stride != Stride)
      return false; // Mismatched stride.
    NumElements += AR.Width;
  }
  return true;
}

/***********************************************************************
 * Region debug dump/print
 */
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void Region::dump() const
{
  errs() << *this << "\n";
}
#endif

void Region::print(raw_ostream &OS) const
{
  OS << *VectorType::get(ElementTy, NumElements) << " <"
      << VStride << ";" << Width << "," << Stride << ">(";
  if (Indirect) {
    OS << Indirect->getName();
    if (auto VT = dyn_cast<VectorType>(Indirect->getType()))
      OS << "<" << VT->getNumElements() << ">(" << IndirectIdx << ")";
    OS << " + ";
  }
  OS << Offset << ")";
  if (Indirect && ParentWidth)
    OS << " {parentwidth=" << ParentWidth << "}";
  if (Mask)
    OS << " {mask=" << *Mask << "}";
}

/***********************************************************************
 * RdWrRegionSequence::buildFromStartWr:  detect a split (legalized)
 *    sequence rdregion-wrregion from the start, and populate the
 *    RdWrRegionSequence object with its details
 *
 * This fails if there is any predication. It succeeds with a sequence length
 * of one (i.e. a single rdregion-wrregion pair).
 *
 * On success, if the WaitingFor field matches one of the wrregions in the
 * sequence, then WaitingFor is reset to 0. This is used by buildFromWr to
 * check that the sequence includes the wrregion originally passed to it.
 *
 * On failure, EndWr is left as is, which means that isNull() continues to
 * be true.
 */
bool RdWrRegionSequence::buildFromStartWr(Instruction *ArgStartWr,
    GenXBaling *Baling)
{
  StartWr = ArgStartWr;
  auto Wr = StartWr;
  assert(isWrRegion(getIntrinsicID(Wr)));
  Region TotalWrR(Wr, Baling->getBaleInfo(Wr));
  WrR = TotalWrR;
  if (TotalWrR.Mask)
    return false;
  OldVal = Wr->getOperand(Intrinsic::GenXRegion::OldValueOperandNum);
  auto RdVal = Wr->getOperand(Intrinsic::GenXRegion::NewValueOperandNum);
  if (auto Rd = dyn_cast<Instruction>(RdVal)) {
    // Handle the case that the start wrregion has a rdregion, so we look for
    // a sequence of rd-wr pairs.
    if (!isRdRegion(getIntrinsicID(Rd)))
      return false;
    Region TotalRdR(Rd, Baling->getBaleInfo(Rd));
    RdR = TotalRdR;
    Input = Rd->getOperand(Intrinsic::GenXRegion::OldValueOperandNum);
    EndWr = Wr;
    if (Wr == WaitingFor)
      WaitingFor = nullptr;
    bool SeenWaitingFor = false;
    for (;;) {
      if (!Wr->hasOneUse() || Wr->use_begin()->getOperandNo()
          != Intrinsic::GenXRegion::OldValueOperandNum)
        break;
      Wr = cast<Instruction>(Wr->use_begin()->getUser());
      if (!isWrRegion(getIntrinsicID(Wr)))
        break;
      Value *In = Wr->getOperand(Intrinsic::GenXRegion::NewValueOperandNum);
      if (!isRdRegion(getIntrinsicID(In)))
        break;
      auto Rd = cast<Instruction>(In);
      if (Rd->getOperand(Intrinsic::GenXRegion::OldValueOperandNum) != Input)
        break;
      // Append to the regions. Give up if either fails.
      if (!TotalRdR.append(Region(Rd, Baling->getBaleInfo(Rd)))
          || !TotalWrR.append(Region(Wr, Baling->getBaleInfo(Wr))))
        break;
      SeenWaitingFor |= Wr == WaitingFor;
      // If both regions are now legal (have a whole number of rows), then
      // save the current position.
      if (TotalRdR.isWholeNumRows() && TotalWrR.isWholeNumRows()) {
        RdR = TotalRdR;
        WrR = TotalWrR;
        EndWr = Wr;
        if (SeenWaitingFor)
          WaitingFor = nullptr;
      }
    }
    return true;
  }
  if (!isa<UndefValue>(Wr->getOperand(Intrinsic::GenXRegion::OldValueOperandNum)))
    return false;
  auto TotalC = dyn_cast<Constant>(RdVal);
  if (!TotalC)
    return false;
  // Handle the case that the start wrregion has a constant "new value" operand
  // and an undef "old value" operand.
  // We look for a sequence of wrregions where the "new value" operands are all
  // constant and we derive the overall constant.
  Region TotalRdR(TotalC);
  RdR = TotalRdR;
  Input = TotalC;
  EndWr = Wr;
  if (Wr == WaitingFor)
    WaitingFor = nullptr;
  bool SeenWaitingFor = false;
  for (;;) {
    if (!Wr->hasOneUse() || Wr->use_begin()->getOperandNo()
        != Intrinsic::GenXRegion::OldValueOperandNum)
      break;
    Wr = cast<Instruction>(Wr->use_begin()->getUser());
    if (!isWrRegion(getIntrinsicID(Wr)))
      break;
    auto In = dyn_cast<Constant>(Wr->getOperand(Intrinsic::GenXRegion::NewValueOperandNum));
    if (!In)
      break;
    // Append to the regions. Give up if either fails.
    Region InR(In);
    InR.Offset = TotalRdR.NumElements * TotalRdR.ElementBytes;
    if (!TotalRdR.append(InR)
        || !TotalWrR.append(Region(Wr, Baling->getBaleInfo(Wr))))
      break;
    SeenWaitingFor |= Wr == WaitingFor;
    // Append the constant.
    TotalC = concatConstants(TotalC, In);
    // If both regions are now legal (have a whole number of rows), then save
    // the current position.
    if (TotalRdR.isWholeNumRows() && TotalWrR.isWholeNumRows()) {
      RdR = TotalRdR;
      WrR = TotalWrR;
      EndWr = Wr;
      Input = TotalC;
      if (SeenWaitingFor)
        WaitingFor = nullptr;
    }
  }
  return true;
}

/***********************************************************************
 * RdWrRegionSequence::buildFromWr:  detect a split (legalized) rdregion-wrregion
 *    sequence starting from any wrregion within it, and populate the
 *    RdWrRegionSequence object with its details
 *
 * This fails if there is any predication. It succeeds with a sequence length
 * of one (i.e. a single rdregion-wrregion pair).
 *
 * On failure, EndWr is left as is, which means that isNull() continues to
 * be true.
 */
bool RdWrRegionSequence::buildFromWr(Instruction *Wr, GenXBaling *Baling)
{
  // Remember that our sequence needs to contain Wr.
  WaitingFor = Wr;
  // Scan back to what looks like the start of the sequence.
  assert(isWrRegion(getIntrinsicID(Wr)));
  StartWr = Wr;
  auto RdVal = Wr->getOperand(Intrinsic::GenXRegion::NewValueOperandNum);
  auto Rd = dyn_cast<Instruction>(RdVal);
  bool ConstSequence = false;
  if (!Rd) {
    if (!isa<Constant>(RdVal))
      return 0;
    ConstSequence = true;
  } else
    Input = Rd->getOperand(Intrinsic::GenXRegion::OldValueOperandNum);
  for (;;) {
    Wr = dyn_cast<Instruction>(
        Wr->getOperand(Intrinsic::GenXRegion::OldValueOperandNum));
    if (!isWrRegion(getIntrinsicID(Wr)))
      break;
    if (!Wr->hasOneUse())
      break;
    RdVal = Wr->getOperand(Intrinsic::GenXRegion::NewValueOperandNum);
    if (ConstSequence) {
      if (!isa<Constant>(RdVal))
        break;
    } else {
      Rd = dyn_cast<Instruction>(
          Wr->getOperand(Intrinsic::GenXRegion::NewValueOperandNum));
      if (!Rd)
        break;
      if (Input != Rd->getOperand(Intrinsic::GenXRegion::OldValueOperandNum))
        break;
    }
    StartWr = Wr;
  }
  // Try detecting a split rdregion-wrregion starting at StartWr.
  for (;;) {
    if (!buildFromStartWr(StartWr, Baling)) {
      EndWr = nullptr;
      return false;
    }
    if (!WaitingFor)
      return true; // success
    // The detected sequence did not include the wrregion this function
    // started with. Retry with the following sequence.
    StartWr = cast<Instruction>(EndWr->use_begin()->getUser());
    if (isWrRegion(getIntrinsicID(StartWr)))
      return false;
  }
}

/***********************************************************************
 * RdWrRegionSequence::buildFromRd:  detect a split (legalized) rdregion-wrregion
 *    sequence starting from any rdregion within it, and populate the
 *    RdWrRegionSequence object with its details
 *
 * This fails if there is any predication. It succeeds with a sequence length
 * of one (i.e. a single rdregion-wrregion pair).
 */
bool RdWrRegionSequence::buildFromRd(Instruction *Rd, GenXBaling *Baling)
{
  assert(isRdRegion(getIntrinsicID(Rd)));
  if (!Rd->hasOneUse())
    return false;
  if (Rd->use_begin()->getOperandNo() != Intrinsic::GenXRegion::NewValueOperandNum)
    return false;
  auto Wr = cast<Instruction>(Rd->use_begin()->getUser());
  if (!isWrRegion(getIntrinsicID(Wr)))
    return false;
  return buildFromWr(Wr, Baling);
}

/***********************************************************************
 * RdWrRegionSequence::size : get number of rdregion-wrregion pairs in the
 *    sequence
 */
unsigned RdWrRegionSequence::size() const
{
  unsigned Size = 1;
  Instruction *Wr = EndWr;
  for ( ; Wr != StartWr; ++Size)
    Wr = cast<Instruction>(
        Wr->getOperand(Intrinsic::GenXRegion::OldValueOperandNum));
  return Size;
}

/***********************************************************************
 * RdWrRegionSequence::isOnlyUseOfInput : check whether the sequence is the
 *    only use of its input
 */
bool RdWrRegionSequence::isOnlyUseOfInput() const
{
  unsigned Count = 0;
  for (auto ui = Input->use_begin(), ue = Input->use_end();
      ui != ue; ++ui)
    ++Count;
  return Count == size();
}

/***********************************************************************
 * RdWrRegionSequence::getRdIndex : get the index of the legalized rdregion
 */
Value *RdWrRegionSequence::getRdIndex() const
{
  if (isa<Constant>(Input))
    return ConstantInt::get(Type::getInt16Ty(StartWr->getContext()), 0);
  auto Rd = cast<Instruction>(
      StartWr->getOperand(Intrinsic::GenXRegion::NewValueOperandNum));
  assert(isRdRegion(getIntrinsicID(Rd)));
  return Rd->getOperand(Intrinsic::GenXRegion::RdIndexOperandNum);
}

/***********************************************************************
 * RdWrRegionSequence::getWrIndex : get the index of the legalized wrregion
 */
Value *RdWrRegionSequence::getWrIndex() const
{
  return StartWr->getOperand(Intrinsic::GenXRegion::WrIndexOperandNum);
}

/***********************************************************************
 * RdWrRegionSequence::getInputUse : get some use of Input in the sequence
 *
 * This only works if the RdWrRegionSequence is a sequence of rd-wr pairs,
 * rather than a sequence of wrregions with constant input. In the latter
 * case, this returns 0.
 */
Use *RdWrRegionSequence::getInputUse() const
{
  auto Rd = dyn_cast<Instruction>(
      StartWr->getOperand(Intrinsic::GenXRegion::NewValueOperandNum));
  if (!isRdRegion(getIntrinsicID(Rd)))
    return nullptr;
  assert(Rd->getOperand(Intrinsic::GenXRegion::OldValueOperandNum) == Input);
  return &Rd->getOperandUse(Intrinsic::GenXRegion::OldValueOperandNum);
}

/***********************************************************************
 * RdWrRegionSequence::print : debug dump/print
 */
void RdWrRegionSequence::print(raw_ostream &OS) const
{
  if (isNull())
    OS << "null";
  else {
    OS << "sequence";
    if (OldVal)
      dbgs() << " OldVal=" << OldVal->getName();
    dbgs() << " Input=" << Input->getName()
      << " StartWr=" << StartWr->getName()
      << " EndWr=" << EndWr->getName()
      << " RdR=" << RdR
      << " WrR=" << WrR;
  }
}

static Value *simplifyRegionWrite(Instruction *Inst) {
  assert(isWrRegion(Inst));
  Value *NewVal = Inst->getOperand(Intrinsic::GenXRegion::NewValueOperandNum);

  // Replace C with A
  // C = wrregion(A, undef, R)
  if (isa<UndefValue>(NewVal))
    return Inst->getOperand(Intrinsic::GenXRegion::OldValueOperandNum);

  // When A and undef have the same type, replace C with A
  // B = rdregion(A, R)
  // C = wrregion(undef, B, R)
  //
  // or replace C by A
  //
  // B = rdregion(A, R)
  // C = wrregion(A, B, R)
  //
  if (isRdRegion(NewVal)) {
    Instruction *B = cast<Instruction>(NewVal);
    Region InnerR(B, BaleInfo());
    Region OuterR(Inst, BaleInfo());
    if (OuterR != InnerR)
      return nullptr;

    auto OldValB = B->getOperand(Intrinsic::GenXRegion::OldValueOperandNum);
    auto OldValC = Inst->getOperand(Intrinsic::GenXRegion::OldValueOperandNum);
    if ((isa<UndefValue>(OldValC) &&
         OldValB->getType() == OldValC->getType()) ||
        OldValB == OldValC)
      return OldValB;
  }

  return nullptr;
}

static Value *simplifyRegionRead(Instruction *Inst) {
  assert(isRdRegion(Inst));
  Value *Input = Inst->getOperand(Intrinsic::GenXRegion::OldValueOperandNum);
  if (isa<UndefValue>(Input))
    return UndefValue::get(Inst->getType());
  else if (auto C = dyn_cast<Constant>(Input)) {
    if (auto Splat = C->getSplatValue()) {
      Type *Ty = Inst->getType();
      if (Ty->isVectorTy())
        Splat = ConstantVector::getSplat(Ty->getVectorNumElements(), Splat);
      return Splat;
    }
  } else if (isWrRegion(Input) && Input->hasOneUse()) {
    // W = wrr(A, B, R)
    // C = rdr(W, R)
    // =>
    // replace C by B
    Instruction *WI = cast<Instruction>(Input);
    Region R1(WI, BaleInfo());
    Region R2(Inst, BaleInfo());
    if (R1 == R2) {
      Value *B = WI->getOperand(Intrinsic::GenXRegion::NewValueOperandNum);
      if (B->getType() == Inst->getType())
        return B;
    }
  }
  return nullptr;
}

// Simplify a region read or write.
Value *llvm::genx::simplifyRegionInst(Instruction *Inst, const DataLayout *DL,
                                      const TargetLibraryInfo *TLI) {
  if (Inst->use_empty())
    return nullptr;

  if (Constant *C = ConstantFoldInstruction(Inst, *DL, TLI))
    return C;

  unsigned ID = getIntrinsicID(Inst);
  switch (ID) {
  case Intrinsic::genx_wrregionf:
  case Intrinsic::genx_wrregioni:
    return simplifyRegionWrite(Inst);
  case Intrinsic::genx_rdregionf:
  case Intrinsic::genx_rdregioni:
    return simplifyRegionRead(Inst);
  default:
    break;
  }
  return nullptr;
}

bool llvm::genx::simplifyRegionInsts(Function *F, const DataLayout *DL,
                                     const TargetLibraryInfo *TLI) {
  bool Changed = false;
  for (auto &BB : F->getBasicBlockList()) {
    for (auto I = BB.begin(); I != BB.end();) {
      Instruction *Inst = &*I++;
      if (auto V = simplifyRegionInst(Inst, DL, TLI)) {
        Inst->replaceAllUsesWith(V);
        Inst->eraseFromParent();
        Changed = true;
      }
    }
  }
  return Changed;
}
