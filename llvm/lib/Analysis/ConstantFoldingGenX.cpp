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
// This file defines a routine for folding a GenX intrinsic call into a constant.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsGenX.h"

using namespace llvm;

/***********************************************************************
 * canConstantFoldGenXIntrinsic : Return true if it is even possible to fold
 *     a call to the specified GenX intrinsic
 */
bool llvm::canConstantFoldGenXIntrinsic(unsigned IID)
{
  switch (IID) {
    case Intrinsic::genx_rdregioni:
    case Intrinsic::genx_rdregionf:
    // The wrregion case specifically excludes genx_wrconstregion
    case Intrinsic::genx_wrregioni:
    case Intrinsic::genx_wrregionf:
    case Intrinsic::genx_all:
    case Intrinsic::genx_any:
      return true;
  }
  return false;
}

/***********************************************************************
 * constantFoldRdRegion : attempt to constant fold rdregion
 */
static Constant *constantFoldRdRegion(Type *RetTy, ArrayRef<Constant *> Operands)
{
  Constant *Input = Operands[Intrinsic::GenXRegion::OldValueOperandNum];
  // The input can be a ConstantExpr if we are being called from
  // CallAnalyzer.
  if (isa<ConstantExpr>(Input))
    return nullptr;
  // If the input value is undef, just return undef.
  if (isa<UndefValue>(Input))
    return UndefValue::get(RetTy);
  // Parse the region parameters.
  unsigned WholeNumElements = Input->getType()->getVectorNumElements();
  unsigned NumElements = 1;
  if (auto VT = dyn_cast<VectorType>(RetTy))
    NumElements = VT->getNumElements();
  int VStride = cast<ConstantInt>(
      Operands[Intrinsic::GenXRegion::RdVStrideOperandNum])->getSExtValue();
  unsigned Width = cast<ConstantInt>(
      Operands[Intrinsic::GenXRegion::RdWidthOperandNum])->getSExtValue();
  int Stride = cast<ConstantInt>(
      Operands[Intrinsic::GenXRegion::RdStrideOperandNum])->getSExtValue();
  auto OffsetC = dyn_cast<Constant>(
      Operands[Intrinsic::GenXRegion::RdIndexOperandNum]);
  if (!OffsetC)
    return nullptr;
  unsigned Offset = 0;
  if (!isa<VectorType>(OffsetC->getType()))
    Offset = dyn_cast<ConstantInt>(OffsetC)->getZExtValue()
      / (RetTy->getScalarType()->getPrimitiveSizeInBits() / 8);
  else
    assert(OffsetC->getType()->getVectorNumElements() == NumElements);
  if (Offset >= WholeNumElements)
    return UndefValue::get(RetTy); // out of range index
  if (!isa<VectorType>(RetTy))
    return Input->getAggregateElement(Offset);
  // Gather the elements of the region being read.
  SmallVector<Constant *, 8> Values;
  unsigned RowIdx = Offset;
  unsigned Idx = RowIdx;
  unsigned NextRow = Width;
  for (unsigned i = 0; i != NumElements; ++i) {
    if (i == NextRow) {
      NextRow += Width;
      RowIdx += VStride;
      Idx = RowIdx;
    }
    if (isa<VectorType>(OffsetC->getType())) {
      auto EltOffset = 
        dyn_cast<ConstantInt>(OffsetC->getAggregateElement(i))->getZExtValue();
      EltOffset = EltOffset / 
        (RetTy->getScalarType()->getPrimitiveSizeInBits() / 8);
      Idx += EltOffset;
    }
    if (Idx >= WholeNumElements)
      return UndefValue::get(RetTy); // out of range index
    // Get the element value and push it into Values.
    Values.push_back(Input->getAggregateElement(Idx));
    Idx += Stride;
  }
  return ConstantVector::get(Values);
}

/***********************************************************************
 * constantFoldWrRegion : attempt to constant fold Wrregion
 */
static Constant *constantFoldWrRegion(Type *RetTy, ArrayRef<Constant *> Operands)
{
  Constant *OldValue = Operands[Intrinsic::GenXRegion::OldValueOperandNum];
  Constant *NewValue = Operands[Intrinsic::GenXRegion::NewValueOperandNum];
  // The inputs can be ConstantExpr if we are being called from
  // CallAnalyzer.
  if (isa<ConstantExpr>(OldValue) || isa<ConstantExpr>(NewValue))
    return nullptr;
  assert(RetTy == OldValue->getType());
  if (isa<UndefValue>(OldValue)) {
    // If old value is undef and new value is splat, and the result vector
    // is no bigger than 2 GRFs, then just return a splat of the right type.
    Constant *Splat = NewValue;
    if (isa<VectorType>(NewValue->getType()))
      Splat = NewValue->getSplatValue();
    if (Splat)
      if (RetTy->getPrimitiveSizeInBits() <= 2 * 32 * 8)
        return ConstantVector::getSplat(RetTy->getVectorNumElements(), Splat);
    // If new value fills the whole vector, just return the new value.
    if (NewValue->getType() == RetTy)
      return NewValue;
  }
  // Parse the region parameters.
  unsigned WholeNumElements = RetTy->getVectorNumElements();
  unsigned NumElements = 1;
  if (auto VT = dyn_cast<VectorType>(NewValue->getType()))
    NumElements = VT->getNumElements();
  int VStride = cast<ConstantInt>(
      Operands[Intrinsic::GenXRegion::WrVStrideOperandNum])->getSExtValue();
  unsigned Width = cast<ConstantInt>(
      Operands[Intrinsic::GenXRegion::WrWidthOperandNum])->getSExtValue();
  int Stride = cast<ConstantInt>(
      Operands[Intrinsic::GenXRegion::WrStrideOperandNum])->getSExtValue();
  auto OffsetC = dyn_cast<ConstantInt>(
      Operands[Intrinsic::GenXRegion::WrIndexOperandNum]);
  if (!OffsetC)
    return nullptr; // allow for but do not const fold when index is vector
  unsigned Offset = OffsetC->getSExtValue()
      / (RetTy->getScalarType()->getPrimitiveSizeInBits() / 8);
  // Gather the elements of the old value.
  SmallVector<Constant *, 8> Values;
  for (unsigned i = 0; i != WholeNumElements; ++i)
    Values.push_back(OldValue->getAggregateElement(i));
  // Insert the elements of the new value.
  if (Offset >= Values.size())
    return UndefValue::get(RetTy); // out of range index
  if (!isa<VectorType>(NewValue->getType()))
    Values[Offset] = NewValue;
  else {
    unsigned RowIdx = Offset;
    unsigned Idx = RowIdx;
    unsigned NextRow = Width;
    for (unsigned i = 0; i != NumElements; ++i) {
      if (i == NextRow) {
        NextRow += Width;
        RowIdx += VStride;
        Idx = RowIdx;
      }
      if (Idx >= WholeNumElements)
        return UndefValue::get(RetTy); // out of range index
      Values[Idx] = NewValue->getAggregateElement(i);
      Idx += Stride;
    }
  }
  return ConstantVector::get(Values);
}

/***********************************************************************
 * constantFoldAll : constant fold llvm.genx.all
 * constantFoldAny : constant fold llvm.genx.any
 */
static Constant *constantFoldAll(Type *RetTy, Constant *In)
{
  if (In->isAllOnesValue())
    return Constant::getAllOnesValue(RetTy);
  return Constant::getNullValue(RetTy);
}
static Constant *constantFoldAny(Type *RetTy, Constant *In)
{
  if (!In->isNullValue())
    return Constant::getAllOnesValue(RetTy);
  return Constant::getNullValue(RetTy);
}

/***********************************************************************
 * ConstantFoldGenXIntrinsic : attempt to constant fold a call to the
 *    specified GenX intrinsic with the specified arguments, returning null if
 *    unsuccessful
 */
Constant *llvm::ConstantFoldGenXIntrinsic(unsigned IID, Type *RetTy,
    ArrayRef<Constant *> Operands)
{
  switch (IID) {
    case Intrinsic::genx_rdregioni:
    case Intrinsic::genx_rdregionf:
      return constantFoldRdRegion(RetTy, Operands);
    // The wrregion case specifically excludes genx_wrconstregion
    case Intrinsic::genx_wrregioni:
    case Intrinsic::genx_wrregionf:
      return constantFoldWrRegion(RetTy, Operands);
    case Intrinsic::genx_all:
      return constantFoldAll(RetTy, Operands[0]);
    case Intrinsic::genx_any:
      return constantFoldAny(RetTy, Operands[0]);
  }
  return nullptr;
}

