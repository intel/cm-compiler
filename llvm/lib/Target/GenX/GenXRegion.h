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
/// GenXRegion : region information
/// -------------------------------
/// 
/// An object of class GenXRegion describes the region parameters of a Gen region.
/// It is a transient object, in that a pass constructs it as needed and then
/// forgets it. It does not persist between passes, as the region parameters are
/// fully described by the arguments to the rdregion and wrregion intrinsics.
///
/// The region parameters in a GenXRegion are:
///
/// * ElementBytes : number of bytes per element
/// * ElementTy : Type of element
/// * NumElements : total number of elements in the region (number of rows is
///   thus NumElements/Width)
/// * VStride : vertical stride in elements
/// * Width : row width in elements
/// * Stride : horizontal stride in elements
/// * Offset : constant part of offset
/// * Indirect : variable index (nullptr for direct region, scalar value for
///   single indirect, vector value for multi indirect)
/// * IndirectIdx : start index in vector indirect. This is always 0 when
///   constructing a GenXRegion, but can be set to a non-zero value before
///   calling a method to create a new rdregion/wrregion intrinsic
/// * Mask : mask (predicate) for wrregion, nullptr if none
/// * ParentWidth : the parent width value (a statement that no row crosses a
///   boundary of a multiple of this number of elements)
///
/// There are the following constructors:
///
/// * Construct from a Type or Value, setting the GenXRegion to a region that
///   covers the whole value.
/// * Construct from a rdregion/wrregion intrinsic, setting the GenXRegion to the
///   region described by the intrinsic. This constructor also takes the
///   BaleInfo as an argument, allowing a variable index that is a baled in
///   constant add to be considered as a separate variable index and constant
///   offset.
/// * Construct from a bitmap of which elements need to be in the region. This
///   is used from GenXConstants when constructing a splat region when loading
///   a constant in multiple stages.
/// 
/// GenXLegalization uses GenXRegion to determine whether a region is legal, and
/// split it up if necessary. First it constructs a GenXRegion, then it has a loop
/// to split it into legal regions. Each loop iteration calls:
///
/// * the getLegalSize method (see below) to determine the split size; then
/// * getSubregion to modify the GenXRegion for the split size; then
/// * one of the methods to create a new rdregion or wrregion intrinsic.
///
/// GenXRegion is not used to represent the region parameters in predicate regions,
/// since they are much simpler. But GenXRegion does contain static methods to create
/// rdpredregion etc intrinsics given the predicate region parameters.
/// 
/// GenXRegion::getLegalSize
/// ^^^^^^^^^^^^^^^^^^^^^^^^
/// 
/// The ``getLegalSize`` method is used by GenXLegalization and some other passes
/// to determine whether a region is legal, and if not how small a split is
/// required to make it legal.
/// 
/// It takes the GenXSubtarget as an argument, because it needs to know
/// architecture-specific details, currently just whether a single GRF crossing is
/// allowed in an indirect region.
/// 
/// It also takes either an AlignmentInfo object, or the actual alignment of the
/// indirect index (if any). Knowing the alignment of the indirect index can help
/// allow a larger legal region, and avoid needing to split into simd1.
/// 
//===----------------------------------------------------------------------===//

#ifndef GENXREGION_H
#define GENXREGION_H

#include "GenXAlignmentInfo.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/IR/IntrinsicsGenX.h"

namespace llvm {
    class Constant;
    class Value;
    class Function;
    class GenXBaling;
    class GenXSubtarget;
    class Module;
    class Type;
    class Instruction;
    class raw_ostream;
    class Twine;
    class DebugLoc;
    class TargetLibraryInfo;

namespace genx {
    struct BaleInfo;

// Region : description of an operand's region
class Region {
public:
  unsigned ElementBytes;
  Type *ElementTy;
  unsigned NumElements;
  int VStride;
  unsigned Width;
  int Stride;
  int Offset;
  Value *Indirect;
  unsigned IndirectIdx; // start index in vector Indirect
  Value *Mask; // 0 else mask for wrregion
  unsigned ParentWidth; // 0 else parent width
  // Get a Region given a rdregion/wrregion, baling in constant add of offset
  static Region getWithOffset(Instruction *Inst, bool WantParentWith = false);
  // Default constructor: assume single element
  Region() : ElementBytes(0), ElementTy(0), NumElements(1), VStride(1),
        Width(1), Stride(1), Offset(0), Indirect(0), IndirectIdx(0), Mask(0),
        ParentWidth(0) {}
  // Construct from a type.
  Region(Type *Ty);
  // Construct from a value.
  Region(Value *V);
  // Construct from a rd/wr region/element and its BaleInfo
  Region(Instruction *Inst, const BaleInfo &BI, bool WantParentWidth = false);
  // Construct from a bitmap of which elements to set (legal 1D region)
  Region(unsigned Bits, unsigned ElementBytes);
  // Create rdregion intrinsic from this Region
  // Returns a scalar if the Region has one element and AllowScalar is true.
  // Otherwise returns a vector.
  Instruction *createRdRegion(Value *Input, const Twine &Name,
                              Instruction *InsertBefore, const DebugLoc &DL,
                              bool AllowScalar = false);
  // Modify Region object for a subregion
  void getSubregion(unsigned StartIdx, unsigned Size);
  // Create wrregion intrinsic from this Region
  Value *createWrRegion(Value *OldVal, Value *Input, const Twine &Name,
                        Instruction *InsertBefore, const DebugLoc &DL);
  // Create wrconstregion intrinsic from this Region
  Value *createWrConstRegion(Value *OldVal, Value *Input, const Twine &Name,
                             Instruction *InsertBefore, const DebugLoc &DL);
  // Create rdpredregion from given start index and size
  static Instruction *createRdPredRegion(Value *Input, unsigned Index,
                                         unsigned Size, const Twine &Name,
                                         Instruction *InsertBefore,
                                         const DebugLoc &DL);
  static Value *createRdPredRegionOrConst(Value *Input, unsigned Index,
                                          unsigned Size, const Twine &Name,
                                          Instruction *InsertBefore,
                                          const DebugLoc &DL);
  // Create wrpredregion from given start index
  static Instruction *createWrPredRegion(Value *OldVal, Value *Input,
                                         unsigned Index, const Twine &Name,
                                         Instruction *InsertBefore,
                                         const DebugLoc &DL);
  // Create wrpredpredregion from given start index
  static Instruction *createWrPredPredRegion(Value *OldVal, Value *Input,
                                             unsigned Index, Value *Pred,
                                             const Twine &Name,
                                             Instruction *InsertBefore,
                                             const DebugLoc &DL);
  // Set the called function in an intrinsic call
  static void setRegionCalledFunc(Instruction *Inst);
  // Compare two regions to see if they have the same region parameters other
  // than start offset (not allowing element type to be different).
  bool isStrictlySimilar(const Region &R2) const {
    return VStride == R2.VStride && Width == R2.Width && Stride == R2.Stride &&
           Mask == R2.Mask;
  }
  // Compare two regions to see if they have the same region parameters other
  // than start offset (also allowing element type to be different).
  bool isSimilar(const Region &R2) const;
  // Compare two regions to see if they have the same region parameters (also
  // allowing element type to be different).
  bool operator==(const Region &R2) const {
    return isSimilar(R2) && Offset == R2.Offset && Indirect == R2.Indirect
        && IndirectIdx == R2.IndirectIdx;
  }
  bool operator!=(const Region &R2) const { return !(*this == R2); }
  // Compare two regions to see if they overlaps each other.
  bool overlap(const Region &R2) const;
  // Test whether a region is scalar
  bool isScalar() const {
    return !Stride && (Width == NumElements || !VStride);
  }
  // Test whether a region is 2D
  bool is2D() const { return !isScalar() && Width != NumElements; }
  // Test whether a region is contiguous.
  bool isContiguous() const;
  // Test whether a region covers exactly the whole of the given type, allowing
  // for the element type being different.
  bool isWhole(Type *Ty) const;
  // Test whether the region has a whole number of rows. (append() can result
  // in a region with an incomplete final row, which is normally not allowed.)
  bool isWholeNumRows() const { return !(NumElements % Width); }
  // Evaluate rdregion with constant input.
  Constant *evaluateConstantRdRegion(Constant *Input, bool AllowScalar);
  // evaluateConstantWrRegion : evaluate wrregion with constant inputs
  Constant *evaluateConstantWrRegion(Constant *OldVal, Constant *NewVal);
  // getLegalSize : get the max legal size of a region
  unsigned getLegalSize(unsigned Idx, bool Allow2D, unsigned InputNumElements,
                        const GenXSubtarget *ST, AlignmentInfo *AI = nullptr);
  unsigned getLegalSize(unsigned Idx, bool Allow2D, unsigned InputNumElements,
                        const GenXSubtarget *ST, Alignment Align);
  // append : append region AR to this region
  bool append(Region AR);
  // changeElementType : change the element type of the region
  bool changeElementType(Type *NewElementType);
  // Debug dump/print
  void dump() const;
  void print(raw_ostream &OS) const;
private:
  // Create wrregion or wrconstregion intrinsic from this Region
  Value *createWrCommonRegion(unsigned IID, Value *OldVal, Value *Input,
                              const Twine &Name, Instruction *InsertBefore,
                              const DebugLoc &DL);
  // Get the function declaration for a region intrinsic
  static Function *getRegionDeclaration(Module *M, unsigned IID, Type *RetTy,
                                        ArrayRef<Value *> Args);
  // Get (or create instruction for) the start index of a region.
  Value *getStartIdx(const Twine &Name, Instruction *InsertBefore, const DebugLoc &DL);
};

inline raw_ostream &operator<<(raw_ostream &OS, const Region &R) {
  R.print(OS);
  return OS;
}

// RdWrRegionSequence : a sequence of rdregion-wrregion pairs probably
// created by legalization or coalescing, conforming to the following
// rules:
//
// 1. It is a sequence of wrregions, each one (other than the last)
//    having the next one's "old value" input as its only use.
//
// 2. Each wrregion's "new value" input is a single-use rdregion.
//
// 3. All the rdregions have the same "old value" input.
//
// 4. If the rdregions have a variable index, the index is the same for each
//    one, other than the constant offset from a baled in genx.add.addr.
//
// 5. The rdregion regions are such that they can be combined to give the
//    region parameters of the original unsplit rdregion. Those rdregion
//    parameters are stored in the RdR field.
//
// 6. If the wrregions have a variable index, the index is the same for each
//    one, other than the constant offset from a baled in genx.add.addr.
//
// 7. The wrregion regions are such that they can be combined to give the
//    region parameters of the original unsplit wrregion. Those wrregion
//    parameters are stored in the WrR field.
//
// Alternatively, a RdWrRegionSequence can represent a sequence of wrregion
// instructions with undef "old value" input to the first one and constant
// "new value" input to each one, forming a legalized constant load.
//
class RdWrRegionSequence {
  Instruction *WaitingFor;
public:
  Value *Input;
  Value *OldVal;
  Instruction *StartWr;
  Instruction *EndWr;
  Region RdR;
  Region WrR;
  // Default constructor
  RdWrRegionSequence() : Input(nullptr), EndWr(nullptr) {}
  // isNull : true if the RdWrRegionSequence has not been initialized
  bool isNull() const { return !EndWr && !Input; }
  // Scan for sequence from the start wrregion instruction.
  // Returns false if not even a single rdregion-wrregion pair found.
  bool buildFromStartWr(Instruction *Wr, GenXBaling *Baling);
  // Scan for sequence from any wrregion instruction in the sequence.
  // Returns false if not even a single rdregion-wrregion pair found.
  bool buildFromWr(Instruction *Wr, GenXBaling *Baling);
  // Scan for sequence from any rdregion instruction in the sequence.
  // Returns false if not even a single rdregion-wrregion pair found.
  bool buildFromRd(Instruction *Rd, GenXBaling *Baling);
  // Get number of rdregion-wrregion pairs in the sequence
  unsigned size() const;
  // Check whether the sequence is the only use of its input
  bool isOnlyUseOfInput() const;
  // Get the index of the legalized rdregion
  Value *getRdIndex() const;
  // Get the index of the legalized wrregion
  Value *getWrIndex() const;
  // Get some use of Input in the sequence
  Use *getInputUse() const;
  // Debug dump/print
  void dump() const;
  void print(raw_ostream &OS) const;
};

inline raw_ostream &operator<<(raw_ostream &OS, const RdWrRegionSequence &RWS) {
  RWS.print(OS);
  return OS;
}

Value *simplifyRegionInst(Instruction *Inst, const DataLayout *DL,
                          const TargetLibraryInfo *TLI);
bool simplifyRegionInsts(Function *F, const DataLayout *DL,
                         const TargetLibraryInfo *TLI);

} // end namespace genx
} // end namespace llvm

#endif /* GENXREGION_H */
