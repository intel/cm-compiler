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

//===-- llvm/GenX.h - defines for GenX backends------------- ----*- C++ -*-===//
//===----------------------------------------------------------------------===//
//
// This file defines common GenX.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_GENX_COMMON_H
#define LLVM_IR_GENX_COMMON_H

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

namespace llvm {
namespace Intrinsic {
namespace GenXRegion {

enum {
  // Operands in both rdregion and wrregion:
  OldValueOperandNum = 0,
  // Operands in rdregion:
  RdVStrideOperandNum = 1,
  RdWidthOperandNum = 2,
  RdStrideOperandNum = 3,
  RdIndexOperandNum = 4,
  // Operands in wrregion:
  NewValueOperandNum = 1,
  WrVStrideOperandNum = 2,
  WrWidthOperandNum = 3,
  WrStrideOperandNum = 4,
  WrIndexOperandNum = 5,
  PredicateOperandNum = 7
};

} // End GenXRegion namespace
} // End Intrinsic namespace

namespace genx {

enum { VISA_MAJOR_VERSION = 3, VISA_MINOR_VERSION = 6 };

// The encoding for register category, used in GenXCategory,
// GenXLiveness and GenXVisaRegAlloc.  It is an anonymous enum inside a class
// rather than a named enum so you don't need to cast to/from int.
struct RegCategory {
  enum {
    NONE,
    GENERAL,
    ADDRESS,
    PREDICATE,
    SAMPLER,
    SURFACE,
    VME,
    NUMREALCATEGORIES,
    EM,
    RM,
    NUMCATEGORIES
  };
};

// Utility function to tell whether a Function is a vISA kernel.
inline bool isKernel(const Function *F) {
  // We use DLLExport to represent a kernel in LLVM IR.
  return F->hasDLLExportStorageClass();
}

// Turn a MDNode into llvm::value or its subclass.
// Return nullptr if the underlying value has type mismatch.
template <typename Ty = llvm::Value> Ty *getValueAsMetadata(Metadata *M) {
  if (auto VM = dyn_cast<ValueAsMetadata>(M))
    if (auto V = dyn_cast<Ty>(VM->getValue()))
      return V;
  return nullptr;
}

/// KernelMetadata : class to parse kernel metadata
class KernelMetadata {
  bool IsKernel;
  StringRef Name;
  StringRef AsmName;
  unsigned SLMSize;
  SmallVector<unsigned, 4> ArgKinds;
  SmallVector<unsigned, 4> ArgOffsets;
  SmallVector<unsigned, 4> ArgIOKinds;

public:
  // default constructor
  KernelMetadata() : IsKernel(false), SLMSize(0) {}
  /*
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
   *  7: kernel argument type descriptors (optional).
   */
  KernelMetadata(Function *F) : IsKernel(false), SLMSize(0) {
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
      ArgKinds.push_back(
          getValueAsMetadata<ConstantInt>(KindsNode->getOperand(i))
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
  // Accessors
  bool isKernel() const { return IsKernel; }
  StringRef getName() const { return Name; }
  StringRef getAsmName() const { return AsmName; }
  unsigned getSLMSize() const { return SLMSize; }
  ArrayRef<unsigned> getArgKinds() { return ArgKinds; }
  unsigned getNumArgs() const { return ArgKinds.size(); }
  unsigned getArgKind(unsigned Idx) const { return ArgKinds[Idx]; }

  enum { AK_NORMAL, AK_SAMPLER, AK_SURFACE, AK_VME };
  unsigned getArgCategory(unsigned Idx) const {
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

  // All the Kinds defined
  // These correspond to the values used in vISA
  // Bits 0-2 represent category (see enum)
  // Bits 7..3 represent the value needed for the runtime to determine what
  //           the implicit argument should be
  //
  // IMP_OCL_LOCAL_ID{X, Y, Z} and IMP_OCL_GLOBAL_OR_LOCAL_SIZE apply to OCL
  // runtime only.
  //
  enum ImpValue {
    IMP_NONE                    = 0x0,
    IMP_LOCAL_SIZE              = 0x1 << 3,
    IMP_GROUP_COUNT             = 0x2 << 3,
    IMP_LOCAL_ID                = 0x3 << 3,
    IMP_SB_DELTAS               = 0x4 << 3,
    IMP_SB_BTI                  = 0x5 << 3,
    IMP_SB_DEPCNT               = 0x6 << 3,
    IMP_OCL_LOCAL_ID_X          = 0x7 << 3,
    IMP_OCL_LOCAL_ID_Y          = 0x8 << 3,
    IMP_OCL_LOCAL_ID_Z          = 0x9 << 3,
    IMP_OCL_GROUP_OR_LOCAL_SIZE = 0xA << 3,
    IMP_PSEUDO_INPUT            = 0x10 << 3
  };

  enum { SKIP_OFFSET_VAL = -1 };
  // Check if this argument should be omitted as a kernel input.
  bool shouldSkipArg(unsigned Idx) const {
    return ArgOffsets[Idx] == SKIP_OFFSET_VAL;
  }
  unsigned getNumNonSKippingInputs() const {
    unsigned K = 0;
    for (unsigned Val : ArgOffsets)
      K += (Val != SKIP_OFFSET_VAL);
    return K;
  }
  unsigned getArgOffset(unsigned Idx) const { return ArgOffsets[Idx]; }

  enum ArgIOKind {
    IO_Normal = 0,
    IO_INPUT = 1,
    IO_OUTPUT = 2,
    IO_INPUT_OUTPUT = 3
  };
  ArgIOKind getArgInputOutputKind(unsigned Idx) const {
    if (Idx < ArgIOKinds.size())
      return static_cast<ArgIOKind>(ArgIOKinds[Idx] & 0x3);
    return IO_Normal;
  }
  bool isOutputArg(unsigned Idx) const {
    auto Kind = getArgInputOutputKind(Idx);
    return Kind == ArgIOKind::IO_OUTPUT || Kind == ArgIOKind::IO_INPUT_OUTPUT;
  }
};

} // namespace genx
} // namespace llvm

#endif
