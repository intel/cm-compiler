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

#ifndef TARGET_GENX_H
#define TARGET_GENX_H
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include <string>

namespace llvm {

class BasicBlock;
class CallInst;
class Constant;
class DebugLoc;
class DominatorTree;
class formatted_raw_ostream;
class Function;
class FunctionGroupPass;
class FunctionPass;
class GenXSubtarget;
class Instruction;
class MDNode;
class ModulePass;
class ShuffleVectorInst;
class TargetOptions;
class Twine;
class Value;
class raw_ostream;
class raw_pwrite_stream;

enum BalingKind {
  BK_Legalization, // build baling info for legalization
  BK_CodeGen,      // build baling info for the final vISA emission
  BK_Analysis,     // build baling info for analysis (register pressure)
};

FunctionPass *createGenXPrinterPass(raw_ostream &O, const std::string &Banner);
FunctionGroupPass *createGenXGroupPrinterPass(raw_ostream &O, const std::string &Banner);
FunctionPass *createGenXAnalysisDumperPass(FunctionPass *Analysis, const char *Suffix);
FunctionGroupPass *createGenXGroupAnalysisDumperPass(FunctionGroupPass *Analysis, const char *Suffix);

FunctionPass *createGenXCFSimplificationPass();
ModulePass *createGenXEarlySimdCFConformancePass();
FunctionPass *createGenXReduceIntSizePass();
FunctionPass *createGenXLoweringPass();
FunctionPass *createGenXGEPLoweringPass();
FunctionPass *createGenXRegionCollapsingPass();
FunctionPass *createGenXExtractVectorizerPass();
FunctionPass *createGenXRawSendRipperPass();
FunctionPass *createGenXFuncBalingPass(BalingKind Kind);
FunctionPass *createGenXLegalizationPass();
ModulePass *createGenXEmulatePass();
FunctionPass *createGenXDeadVectorRemovalPass();
FunctionPass *createGenXPatternMatchPass(const TargetOptions *Options);
FunctionPass *createGenXPostLegalizationPass();
FunctionPass *createGenXPromotePredicatePass();
FunctionPass *createGenXIMadPostLegalizationPass();
ModulePass *createGenXModulePass();
FunctionGroupPass *createGenXLateSimdCFConformancePass();
FunctionGroupPass *createGenXLivenessPass();
FunctionGroupPass *createGenXCategoryPass();
FunctionGroupPass *createGenXGroupBalingPass(BalingKind Kind);
FunctionGroupPass *createGenXUnbalingPass();
FunctionGroupPass *createGenXDepressurizerPass();
FunctionGroupPass *createGenXLateLegalizationPass();
FunctionGroupPass *createGenXNumberingPass();
FunctionGroupPass *createGenXLiveRangesPass();
FunctionGroupPass *createGenXRematerializationPass();
FunctionGroupPass *createGenXCoalescingPass();
FunctionGroupPass *createGenXAddressCommoningPass();
FunctionGroupPass *createGenXArgIndirectionPass();
FunctionPass *createGenXTidyControlFlowPass();
FunctionGroupPass *createGenXVisaRegAllocPass();
FunctionGroupPass *createGenXVisaFuncWriterPass();
ModulePass *createGenXVisaWriterPass(raw_pwrite_stream &o);

// Utility function to get the integral log base 2 of an integer, or -1 if
// the input is not a power of 2.
inline int exactLog2(unsigned Val)
{
  unsigned CLZ = countLeadingZeros(Val, ZB_Width);
  if (CLZ != 32 && 1U << (31 - CLZ) == Val)
    return 31 - CLZ;
  return -1;
}

// Utility function to get the log base 2 of an integer, truncated to an
// integer, or -1 if the number is 0 or negative.
template<typename T>
inline int log2(T Val)
{
  if (Val <= 0)
    return -1;
  unsigned CLZ = countLeadingZeros((uint32_t)Val, ZB_Width);
  return 31 - CLZ;
}

namespace genx {

// The encoding for register category, used in GenXCategory,
// GenXLiveness and GenXVisaRegAlloc.  It is an anonymous enum inside a class
// rather than a named enum so you don't need to cast to/from int.
struct RegCategory {
  enum { NONE, GENERAL, ADDRESS, PREDICATE,
      SAMPLER, SURFACE, VME, NUMREALCATEGORIES,
      EM, RM, NUMCATEGORIES };
};

// A local encoding (not part of vISA or GenX) of whether an operand should be signed.
enum Signedness {
  DONTCARESIGNED = 3, SIGNED = 1, UNSIGNED = 2
};

// ConstantLoader : class to insert instruction(s) to load a constant
class ConstantLoader {
  Constant *C;
  Instruction *User;
  // AddedInstructions: a vector that the caller has requested any added
  // instructions to be pushed in to.
  SmallVectorImpl<Instruction *> *AddedInstructions;
  // Info from analyzing for possible packed vector constant.
  int PackedIntScale;       // amount to scale packed int vector by
  int64_t PackedIntAdjust;  // amount to adjust by, special casing 0 or -8
                            //  when PackedIntScale is 1
  unsigned PackedIntMax;    // max value in packed vector, used when scale is
                            //  1 and adjust is 0 to tell whether it would fit
                            //  in 0..7
  bool PackedFloat;

public:
  // Constructor
  // User = the instruction that uses the constant. If this is genx.constanti,
  //        then a packed vector constant can be an isSimple() constant even
  //        when the element type is not i16. Also used to disallow a packed
  //        vector constant in a logic op. If User==0 then it is assumed that
  //        a packed vector constant with an element type other than i16 is OK.
  // AddedInstructions = vector to add new instructions to when loading a
  //        non simple constant, so the caller can see all the newly added
  //        instructions.
  ConstantLoader(Constant *C, Instruction *User = nullptr,
      SmallVectorImpl<Instruction *> *AddedInstructions = nullptr)
      : C(C), User(User), AddedInstructions(AddedInstructions),
        PackedIntScale(0), PackedFloat(false) {
    analyze();
  }
  Instruction *load(Instruction *InsertBefore);
  Instruction *loadBig(Instruction *InsertBefore);
  Instruction *loadNonSimple(Instruction *InsertBefore);
  bool isBigSimple();
  bool isSimple();
  bool isLegalSize();
private:
  bool isPackedIntVector();
  bool isPackedFloatVector();
  void analyze();
  Constant *getConsolidatedConstant(Constant *C);
  unsigned getRegionBits(unsigned NeededBits, unsigned OptionalBits, unsigned VecWidth);
  void analyzeForPackedInt(unsigned NumElements);
  void analyzeForPackedFloat(unsigned NumElements);
  Instruction *loadSplatConstant(Instruction *InsertPos);
};

// KernelMetadata : class to parse kernel metadata
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
  // constructor from Function
  KernelMetadata(Function *F);
  // Accessors
  bool isKernel() const { return IsKernel; }
  StringRef getName() const { return Name; }
  StringRef getAsmName() const { return AsmName; }
  unsigned getSLMSize() const { return SLMSize; }
  ArrayRef<unsigned> getArgKinds() { return ArgKinds; }
  unsigned getNumArgs() const { return ArgKinds.size(); }
  unsigned getArgKind(unsigned Idx) const { return ArgKinds[Idx]; }
  unsigned getArgCategory(unsigned Idx) const;
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

// Utility function to tell whether a Function is a vISA kernel.
bool isKernel(Function *F);

// Load a constant using the llvm.genx.constant intrinsic.
inline Instruction *loadConstant(Constant *C, Instruction *InsertBefore,
      SmallVectorImpl<Instruction *> *AddedInstructions = nullptr) {
  return ConstantLoader(C, nullptr, AddedInstructions).load(InsertBefore);
}

// Load non-simple constants used in an instruction.
bool loadNonSimpleConstants(Instruction *Inst, SmallVectorImpl<Instruction *> *AddedInstructions = nullptr);

// Load constants used in an instruction.
bool loadConstants(Instruction *Inst);

// Load constants used in phi nodes in a function.
bool loadPhiConstants(Function *F, DominatorTree *DT, bool ExcludePredicate = false);

// Utility function to get the intrinsic ID if V is an intrinsic call.
// V is allowed to be 0.
unsigned getIntrinsicID(Value *V);

// createConvert : create a genx_convert intrinsic call
CallInst *createConvert(Value *In, const Twine &Name, Instruction *InsertBefore, Module *M = nullptr);

// createConvertAddr : create a genx_convert_addr intrinsic call
CallInst *createConvertAddr(Value *In, int Offset, const Twine &Name, Instruction *InsertBefore, Module *M = nullptr);

// createAddAddr : create a genx_add_addr intrinsic call
CallInst *createAddAddr(Value *Lhs, Value *Rhs, const Twine &Name, Instruction *InsertBefore, Module *M = nullptr);

// getPredicateConstantAsInt : get a vXi1 constant's value as a single integer
unsigned getPredicateConstantAsInt(Constant *C);

// getConstantSubvector : get a contiguous region from a vector constant
Constant *getConstantSubvector(Constant *V, unsigned StartIdx, unsigned Size);

// concatConstants : concatenate two possibly vector constants, giving a vector constant
Constant *concatConstants(Constant *C1, Constant *C2);

// findClosestCommonDominator : find latest common dominator of some instructions
Instruction *findClosestCommonDominator(DominatorTree *DT, ArrayRef<Instruction *> Insts);

// convertShlShr : convert Shl followed by AShr/LShr by the same amount into trunc+sext/zext
Instruction *convertShlShr(Instruction *Inst);

// splitStructPhis : split all struct phis in a function
bool splitStructPhis(Function *F);

// breakConstantExprs : break constant expressions in a function.
bool breakConstantExprs(Function *F);

// isRdRegion : test whether the intrinsic id is rdregion
static inline bool isRdRegion(unsigned IntrinID) {
  switch (IntrinID) {
    case Intrinsic::genx_rdregioni:
    case Intrinsic::genx_rdregionf:
      return true;
    default:
      return false;
  }
}

// isWrRegion : test whether the intrinsic id is wrregion
static inline bool isWrRegion(unsigned IntrinID) {
  switch (IntrinID) {
    case Intrinsic::genx_wrregioni:
    case Intrinsic::genx_wrregionf:
    case Intrinsic::genx_wrconstregion:
      return true;
    default:
      return false;
  }
}

// isIntegerSat : test whether the intrinsic id is integer saturation.
static inline bool isIntegerSat(unsigned IID) {
  switch (IID) {
  case Intrinsic::genx_sstrunc_sat:
  case Intrinsic::genx_sutrunc_sat:
  case Intrinsic::genx_ustrunc_sat:
  case Intrinsic::genx_uutrunc_sat:
    return true;
  default:
    return false;
  }
}

// isRdRegion: test whether the value is a rdregion.
static inline bool isRdRegion(Value *V) {
  return isRdRegion(getIntrinsicID(V));
}

// isWrRegion: test whether the value is a wrregion.
static inline bool isWrRegion(Value *V) {
  return isWrRegion(getIntrinsicID(V));
}

// isIntegerSat: test whether the value is a saturation on integer.
static inline bool isIntegerSat(Value *V) {
  return isIntegerSat(getIntrinsicID(V));
}

// getTwoAddressOperandNum : get operand number of two address operand
int getTwoAddressOperandNum(CallInst *CI);

// isNot : test whether an instruction is a "not" instruction (an xor with
//    constant all ones)
bool isNot(Instruction *Inst);

// isPredNot : test whether an instruction is a "not" instruction (an xor
//    with constant all ones) with predicate (i1 or vector of i1) type
bool isPredNot(Instruction *Inst);

// isIntNot : test whether an instruction is a "not" instruction (an xor
//    with constant all ones) with non-predicate type
bool isIntNot(Instruction *Inst);

// ShuffleVectorAnalyzer : class to analyze a shufflevector
class ShuffleVectorAnalyzer {
  ShuffleVectorInst *SI;
public:
  ShuffleVectorAnalyzer(ShuffleVectorInst *SI) : SI(SI) {}
  // getAsSlice : return start index of slice, or -1 if shufflevector is not
  //  slice
  int getAsSlice();
  // getAsUnslice : see if the shufflevector is an
  //     unslice where the "old value" is operand 0 and operand 1 is another
  //     shufflevector and operand 0 of that is the "new value" Returns start
  //     index, or -1 if it is not an unslice
  int getAsUnslice();
  // getAsSplat : if shufflevector is a splat, get the splatted input, with the
  //  element's vector index if the input is a vector
  struct SplatInfo {
    Value *Input;
    unsigned Index;
    SplatInfo(Value *Input, unsigned Index) : Input(Input), Index(Index) {}
  };
  SplatInfo getAsSplat();

  // Serialize this shuffulevector instruction.
  Value *serialize();

  // Compute the cost in terms of number of insertelement instructions needed.
  unsigned getSerializeCost(unsigned i);
};

// adjustPhiNodesForBlockRemoval : adjust phi nodes when removing a block
void adjustPhiNodesForBlockRemoval(BasicBlock *Succ, BasicBlock *BB);

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
Value *sinkAdd(Value *V);

// Check if this is a mask packing operation, i.e. a bitcast from Vxi1 to
// integer i8, i16 or i32.
static inline bool isMaskPacking(const Value *V) {
  if (auto BC = dyn_cast<BitCastInst>(V)) {
    auto SrcTy = dyn_cast<VectorType>(BC->getSrcTy());
    if (!SrcTy || !SrcTy->getScalarType()->isIntegerTy(1))
      return false;
    unsigned NElts = SrcTy->getVectorNumElements();
    if (NElts != 8 && NElts != 16 && NElts != 32)
      return false;
    return V->getType()->getScalarType()->isIntegerTy(NElts);
  }
  return false;
}

// Turn a MDNode into llvm::value or its subclass.
// Return nullptr if the underlying value has type mismatch.
template <typename Ty = llvm::Value>
Ty *getValueAsMetadata(Metadata *M) {
  if (auto VM = dyn_cast<ValueAsMetadata>(M))
    if (auto V = dyn_cast<Ty>(VM->getValue()))
      return V;
  return nullptr;
}

void LayoutBlocks(Function &func, LoopInfo &LI);
void LayoutBlocks(Function &func);

} // End genx namespace
} // End llvm namespace

#endif
