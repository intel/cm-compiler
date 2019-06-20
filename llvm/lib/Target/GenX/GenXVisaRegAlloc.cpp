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
// GenXVisaRegAlloc is a function group pass that allocates vISA registers to
// LLVM IR values. See GenXVisaRegAlloc.h.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "GENX_REGALLOC"

#include "visa_igc_common_header.h"
#include "GenXVisaRegAlloc.h"
#include "GenX.h"
#include "GenXIntrinsics.h"
#include "GenXLiveness.h"
#include "GenXNumbering.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace genx;
using namespace visa;

static cl::opt<unsigned> LimitGenXExtraCoalescing("limit-genx-extra-coalescing", cl::init(UINT_MAX), cl::Hidden,
                                      cl::desc("Limit GenX extra coalescing."));


char GenXVisaRegAlloc::ID = 0;
INITIALIZE_PASS_BEGIN(GenXVisaRegAlloc, "GenXVisaRegAlloc", "GenXVisaRegAlloc", false, false)
INITIALIZE_PASS_DEPENDENCY(GenXLiveness)
INITIALIZE_PASS_DEPENDENCY(GenXNumbering)
INITIALIZE_PASS_END(GenXVisaRegAlloc, "GenXVisaRegAlloc", "GenXVisaRegAlloc", false, false)

FunctionGroupPass *llvm::createGenXVisaRegAllocPass()
{
  initializeGenXVisaRegAllocPass(*PassRegistry::getPassRegistry());
  return new GenXVisaRegAlloc();
}

void GenXVisaRegAlloc::getAnalysisUsage(AnalysisUsage &AU) const
{
  FunctionGroupPass::getAnalysisUsage(AU);
  AU.addRequired<GenXLiveness>();
  AU.addRequired<GenXNumbering>();
  AU.setPreservesAll();
}

/***********************************************************************
 * runOnFunctionGroup : run the register allocator for this FunctionGroup
 *
 * This is currently a trivial allocator that just gives a new vISA virtual
 * register to every single Value.
 */
bool GenXVisaRegAlloc::runOnFunctionGroup(FunctionGroup &FGArg)
{
  FG = &FGArg;
  Liveness = &getAnalysis<GenXLiveness>();
  Numbering = &getAnalysis<GenXNumbering>();
  BoolTy = Type::getInt1Ty(FG->getContext());
  // Empty out the analysis from the last function it was used on.
  RegMap.clear();
  for (unsigned i = 0; i != RegCategory::NUMREALCATEGORIES; ++i) {
    Regs[i].clear();
    NumReserved[i] = 0;
  }
  // Reserve the reserved registers.
  NumReserved[RegCategory::GENERAL] = VISA_NUM_RESERVED_REGS;
  NumReserved[RegCategory::PREDICATE] = VISA_NUM_RESERVED_PREDICATES;
  NumReserved[RegCategory::SURFACE] = VISA_NUM_RESERVED_SURFACES;
  for (unsigned Cat = 0; Cat != RegCategory::NUMREALCATEGORIES; ++Cat)
    for (unsigned R = 0; R != NumReserved[Cat]; ++R)
      Regs[Cat].push_back(Reg(0));
  // Do some extra coalescing.
  extraCoalescing();
  // Get the live ranges in a reproducible order.
  std::vector<LiveRange *> LRs;
  getLiveRanges(&LRs);
  // Allocate a register to each live range.
  for (auto i = LRs.begin(), e = LRs.end(); i != e; ++i)
    allocReg(*i);
  if (Regs[RegCategory::GENERAL].size() > VISA_MAX_GENERAL_REGS)
    report_fatal_error("Too many vISA general registers");
  if (Regs[RegCategory::ADDRESS].size() > VISA_MAX_ADDRESS_REGS)
    report_fatal_error("Too many vISA address registers");
  if (Regs[RegCategory::PREDICATE].size() > VISA_MAX_PREDICATE_REGS)
    report_fatal_error("Too many vISA predicate registers");
  if (Regs[RegCategory::SAMPLER].size() > VISA_MAX_SAMPLER_REGS)
    report_fatal_error("Too many vISA sampler registers");
  if (Regs[RegCategory::SURFACE].size() > VISA_MAX_SURFACE_REGS)
    report_fatal_error("Too many vISA surface registers");
  if (Regs[RegCategory::VME].size() > VISA_MAX_VME_REGS)
    report_fatal_error("Too many vISA VME registers");
  return false;
}

/***********************************************************************
 * getLiveRanges : get the live ranges in a reproducible order
 *
 * We scan the code to find the live ranges, rather than just walking the
 * GenXLiveness map, to ensure that registers are allocated in a consistent
 * order that does not depend on the layout of allocated memory.
 *
 * This ignores any live range with no category, so such a live range does not
 * get allocated a register. GenXArgIndirection uses that to stop an indirected
 * argument uselessly getting a register.
 */
void GenXVisaRegAlloc::getLiveRanges(std::vector<LiveRange *> *LRs) const
{
  // create LRs for global variables.
  for (auto &GV : FG->getModule()->globals())
    getLiveRangesForValue(&GV, LRs);
  for (auto fgi = FG->begin(), fge = FG->end(); fgi != fge; ++fgi) {
    Function *F = *fgi;
    for (auto ai = F->arg_begin(), ae = F->arg_end(); ai != ae; ++ai)
      getLiveRangesForValue(&*ai, LRs);
    if (fgi != FG->begin() && !F->getReturnType()->isVoidTy()) {
      // allocate reg for unified return value
      getLiveRangesForValue(Liveness->getUnifiedRet(F), LRs);
    }
    for (auto fi = F->begin(), fe = F->end(); fi != fe; ++fi) {
      BasicBlock *BB = &*fi;
      for (auto bi = BB->begin(), be = BB->end(); bi != be; ++bi)
        getLiveRangesForValue(&*bi, LRs);
    }
  }
}

void GenXVisaRegAlloc::getLiveRangesForValue(Value *V,
    std::vector<LiveRange *> *LRs) const
{
  auto Ty = V->getType();
  for (unsigned i = 0, e = IndexFlattener::getNumElements(Ty);
      i != e; ++i) {
    SimpleValue SV(V, i);
    LiveRange *LR = Liveness->getLiveRangeOrNull(SV);
    if (!LR || LR->getCategory() == RegCategory::NONE)
      continue;
    // Only process an LR if the map iterator is on the value that appears
    // first in the LR. That avoids processing the same LR multiple times.
    if (SV != *LR->value_begin())
      continue;
    LRs->push_back(LR);
  }
}

/***********************************************************************
 * extraCoalescing : do some extra coalescing over and above what
 *    GenXCoalescing does
 *
 * GenXCoalescing does coalescing where it saves a copy, for example for
 * a two address operand. This function does coalescing that does not save
 * a copy, but the two live ranges are related by being the operand (a
 * kill use) and result of the same instruction. This is in the hope that
 * the jitter's register allocator will be able to do a better job with it.
 *
 * A further case of extra coalescing is that multiple instances of a constant
 * load of a surface variable are coalesced together. This allows the CM code
 * to use lots of printfs without running out of surface variables.
 */
void GenXVisaRegAlloc::extraCoalescing()
{
  LiveRange *CommonSurface = nullptr;
  for (auto fgi = FG->begin(), fge = FG->end(); fgi != fge; ++fgi) {
    Function *F = *fgi;
    for (auto fi = F->begin(), fe = F->end(); fi != fe; ++fi) {
      BasicBlock *BB = &*fi;
      for (auto bi = BB->begin(), be = BB->end(); bi != be; ++bi) {
        auto Inst = &*bi;
        if (isa<StructType>(Inst->getType()))
          continue;
        if (isWrRegion(getIntrinsicID(Inst)))
          continue;
        auto LR = Liveness->getLiveRangeOrNull(Inst);
        if (!LR)
          continue;
        // Check for convert of constant ot surface.
        switch (getIntrinsicID(Inst)) {
          case Intrinsic::genx_convert:
          case Intrinsic::genx_constanti:
            if (LR->Category == RegCategory::SURFACE
                && isa<Constant>(Inst->getOperand(0))) {
              // See if we can coalesce it with CommonSurface.
              if (!CommonSurface)
                CommonSurface = LR;
              else if (!Liveness->interfere(CommonSurface, LR))
                CommonSurface = Liveness->coalesce(CommonSurface, LR, /*DisalowCASC=*/true);
            }
            break;
        }
        if (LR->Category != RegCategory::GENERAL)
          continue;
        // We have a non-struct non-wrregion instruction whose result has a
        // live range (it is not baled into anything else).
        // Check all uses to see if there is one in a non-alu intrinsic. We
        // don't want to coalesce that, because of the danger of the jitter
        // needing to add an extra move in the send.
        bool UseInNonAluIntrinsic = false;
        for (auto ui = Inst->use_begin(), ue = Inst->use_end();
            ui != ue && !UseInNonAluIntrinsic; ++ui) {
          auto user = dyn_cast<Instruction>(ui->getUser());
          assert(user);
          if (user->getType()->isVoidTy()) {
            UseInNonAluIntrinsic = true;
            break;
          }
          unsigned IID = getIntrinsicID(user);
          switch (IID) {
            case Intrinsic::not_intrinsic:
            case Intrinsic::genx_rdregioni:
            case Intrinsic::genx_rdregionf:
            case Intrinsic::genx_wrregioni:
            case Intrinsic::genx_wrregionf:
              break;
            default: {
                // It is an intrinsic. A non-alu intrinsic does not have a
                // return value that is general.
                GenXIntrinsicInfo II(IID);
                if (!II.getRetInfo().isGeneral())
                  UseInNonAluIntrinsic = true;
              }
              break;
          }
        }
        if (UseInNonAluIntrinsic)
          continue;

        // Do not coalesce when this is a two address instrinsic with undef
        // input. Otherwise logic is broken on lifetime marker in vISA emission.
        //
        auto skipTwoAddrCoalesce = [](Instruction *Inst) {
          unsigned IntrinsicID = getIntrinsicID(Inst);
          if (IntrinsicID == Intrinsic::not_intrinsic)
            return false;
          GenXIntrinsicInfo Info(IntrinsicID);
          const auto *descp = Info.getInstDesc();
          for (const auto *p = descp; *p; ++p) {
            GenXIntrinsicInfo::ArgInfo AI(*p);
            if (AI.getCategory() != GenXIntrinsicInfo::TWOADDR)
              continue;
            if (isa<UndefValue>(Inst->getOperand(AI.getArgIdx())))
              return true;
          }
          return false;
        };
        if (skipTwoAddrCoalesce(Inst))
          continue;

        // See if we can coalesce with any operand.
        for (unsigned oi = 0, oe = Inst->getNumOperands(); oi != oe; ++oi) {
          Value *Operand = Inst->getOperand(oi);
          if (isa<Constant>(Operand))
            continue;
          if (Operand->getType() != Inst->getType())
            continue;
          // Do not coalesce with kernel arguments as they are input variables.
          if (FG->getHead() == F && isa<Argument>(Operand))
            continue;
          auto OperandLR = Liveness->getLiveRangeOrNull(Operand);
          if (!OperandLR || OperandLR->Category != RegCategory::GENERAL)
            continue;
          if (Liveness->interfere(LR, OperandLR))
            continue;
          // The two live ranges do not interfere, so we can coalesce them.
          if (++CoalescingCount > LimitGenXExtraCoalescing)
            continue;
          if (LimitGenXExtraCoalescing != UINT_MAX)
            dbgs() << "genx extra coalescing " << CoalescingCount << "\n";
          Liveness->coalesce(LR, OperandLR, /*DisalowCASC=*/true);
          break;
        }
      }
    }
  }
}

/***********************************************************************
 * allocReg : allocate a register for a LiveRange
 */
void GenXVisaRegAlloc::allocReg(LiveRange *LR)
{
  if (LR->value_empty())
    return;
  if (LR->getCategory() >= RegCategory::NUMREALCATEGORIES)
    return; // don't allocate register to EM or RM value
  DEBUG(
    dbgs() << "Allocating ";
    LR->print(dbgs());
    dbgs() << "\n"
  );
  SimpleValue V = *LR->value_begin();
  Type *Ty = V.getType();
  if (auto GV = dyn_cast<GlobalVariable>(V.getValue()))
    if (GV->hasAttribute("genx_volatile"))
      Ty = Ty->getPointerElementType();
  assert(!Ty->isVoidTy());
  if (LR->Category == RegCategory::PREDICATE) {
    VectorType *VT = dyn_cast<VectorType>(Ty);
    assert((!VT || llvm::exactLog2(VT->getNumElements()) >= 0) && "invalid predicate width");
    (void)VT;
  }
  // Allocate the register, also setting the alignment.
  RegNum R = RegNum(LR->Category, Regs[LR->Category].size());
  Regs[LR->Category].push_back(Reg(Ty, DONTCARESIGNED, LR->getLogAlignment()));
  // Assign to the values. If any value is an input arg, ensure the register
  // gets its type, to avoid needing an alias for an input arg.
  for (LiveRange::value_iterator vi = LR->value_begin(), ve = LR->value_end();
      vi != ve; ++vi) {
    DEBUG(
      dbgs() << "Allocating reg " << R.Num << " to ";
      vi->print(dbgs());
      dbgs() << "\n"
    );
    assert(!RegMap[*vi].Num);
    RegMap[*vi] = R;
    if (isa<Argument>(vi->getValue()))
      Regs[LR->Category].back().Ty = vi->getType();
  }
}

/***********************************************************************
 * getRegNumForValueUntyped : get the vISA reg allocated to a particular
 *    value, ignoring signedness and type
 *
 * This is a const method so it can be called from print().
 */
GenXVisaRegAlloc::RegNum GenXVisaRegAlloc::getRegNumForValueUntyped(
    SimpleValue V) const
{
  RegMap_t::const_iterator i = RegMap.find(V);
  if (i == RegMap.end()) {
    // Check if it's predefined variables.
    unsigned IID = getIntrinsicID(V.getValue());
    if (IID != Intrinsic::genx_predefined_surface)
      return RegNum();
    auto CI = cast<CallInst>(V.getValue());
    unsigned Id = cast<ConstantInt>(CI->getArgOperand(0))->getZExtValue();
    assert(Id < 4 && "Invalid predefined surface ID!");
    RegNum RN(RegCategory::SURFACE, Id);
    return RN;
  }
  RegNum RN = i->second;
  return RN;
}

/***********************************************************************
 * getRegNumForValueOrNull : get the vISA reg allocated to a particular Value
 *
 * Enter:   V = value (Argument or Instruction) to get register for
 *          Signed = request for signed or unsigned
 *          OverrideType = 0 else override type of value (used for bitcast)
 *
 * Called from GenXVisaFunctionWriter to get the register for an
 * operand. The operand type might not match the register type (say a
 * bitcast has been coalesced, or the same integer value is used
 * unsigned in one place and signed in another), in which case we
 * find/create a vISA register alias.
 */
GenXVisaRegAlloc::RegNum GenXVisaRegAlloc::getRegNumForValueOrNull(
    SimpleValue V, Signedness Signed, Type *OverrideType)
{
  if (!OverrideType)
    OverrideType = V.getType();
  if (OverrideType->isPointerTy()) {
    auto GV = dyn_cast<GlobalVariable>(V.getValue());
    if (GV && GV->hasAttribute("genx_volatile"))
      OverrideType = OverrideType->getPointerElementType();
  }
  RegNum RN = getRegNumForValueUntyped(V);
  if (RN == RegNum())
    return RN; // no register allocated
  if (RN.Category == RegCategory::GENERAL) {
    // Check whether the type and signedness of the value matches the register.
    // If not we need to find or create an alias.
    unsigned OrigNum = RN.Num;
    Reg * R;
    for (;;) {
      R = &Regs[RegCategory::GENERAL][RN.Num];
      // Check whether this alias is the right type and signedness.
      // We treat a 1-vector the same as a scalar.
      Type *ExistingType = R->Ty;
      if (VectorType *VT = dyn_cast<VectorType>(ExistingType))
        if (VT->getNumElements() == 1)
          ExistingType = VT->getElementType();
      if (VectorType *VT = dyn_cast<VectorType>(OverrideType))
        if (VT->getNumElements() == 1)
          OverrideType = VT->getElementType();
      if (ExistingType == OverrideType) {
        if (R->Signed == DONTCARESIGNED) {
          // Match, use this alias. (Signedness is currently don't care, so
          // set it to the requested signedness.)
          R->Signed = Signed;
          break;
        }
        if (R->Signed == Signed || Signed == DONTCARESIGNED)
          break; // Match, use this alias.
      }
      // On to next alias.
      unsigned Next = R->NextAlias;
      if (Next) {
        RN.Num = R->NextAlias;
        continue;
      }
      // Run out of aliases. Add a new one.
      RegNum NewRN = RegNum(RegCategory::GENERAL, Regs[RegCategory::GENERAL].size());
      R->NextAlias = NewRN.Num;
      // Following push_back invalidates R if resize occurs
      Regs[RegCategory::GENERAL].push_back(Reg(OverrideType, Signed));
      R = &Regs[RegCategory::GENERAL][NewRN.Num];
      R->AliasTo = OrigNum;
      RN = NewRN;
      break;
    }
  }
  return RN;
}

/***********************************************************************
 * getSigned : get the signedness of a register
 *
 * If the register has byte type and is currently don't care signedness, this
 * arbitrarily picks unsigned. We do that because having a byte mov with
 * different signedness between source and destination can make the jitter
 * generate less efficient code.
 */
genx::Signedness GenXVisaRegAlloc::getSigned(RegNum RN)
{
  if (RN.Category != RegCategory::GENERAL)
    return DONTCARESIGNED;
  Reg *R = &Regs[RegCategory::GENERAL][RN.Num];
  if (R->Signed == DONTCARESIGNED && R->Ty->getScalarType()->isIntegerTy(8))
    R->Signed = UNSIGNED;
  return R->Signed;
}

/***********************************************************************
 * areAliased : check if two RegNum values returned by getRegNumForValue
 *              are aliased
 */
bool GenXVisaRegAlloc::areAliased(RegNum R1, RegNum R2)
{
  if (R1.Category != R2.Category)
    return false;
  if (R1.Num == R2.Num)
    return true;
  Reg Reg1 = Regs[R1.Category][R1.Num];
  Reg Reg2 = Regs[R2.Category][R2.Num];
  if (Reg1.AliasTo == R2.Num || Reg2.AliasTo == R1.Num
      || (Reg1.AliasTo && Reg1.AliasTo == Reg2.AliasTo))
    return true;
  return false;
}

/***********************************************************************
 * getLogAlignment : get required log2 alignment of a register
 */
unsigned GenXVisaRegAlloc::getLogAlignment(RegNum R)
{
  return Regs[R.Category][R.Num].Alignment;
}

// addRetIPArgument : Add the RetIP argument required for caller kernels and
// their caller.
void GenXVisaRegAlloc::addRetIPArgument() {
  auto Cat = RegCategory::GENERAL;
  RetIP = RegNum(Cat, Regs[Cat].size());
  Regs[Cat].push_back(Reg(Type::getInt64Ty(FG->getContext())));
}

/***********************************************************************
 * TypeDetails constructor
 *
 * Enter:   Ty = LLVM type
 *          Signedness = whether signed type required
 */
TypeDetails::TypeDetails(const DataLayout &DL, Type *Ty, Signedness Signed)
    : DL(DL) {
  Type *ElementTy = Ty;
  NumElements = 1;
  if (VectorType *VT = dyn_cast<VectorType>(ElementTy)) {
    ElementTy = VT->getElementType();
    NumElements = VT->getNumElements();
  }
  if (IntegerType *IT = dyn_cast<IntegerType>(ElementTy)) {
    BytesPerElement = IT->getBitWidth() / 8;
    if (Signed == UNSIGNED) {
      switch (BytesPerElement) {
        case 1: VisaType = ISA_TYPE_UB; break;
        case 2: VisaType = ISA_TYPE_UW; break;
        case 4: VisaType = ISA_TYPE_UD; break;
        default: VisaType = ISA_TYPE_UQ; break;
      }
    } else {
      switch (BytesPerElement) {
        case 1: VisaType = ISA_TYPE_B; break;
        case 2: VisaType = ISA_TYPE_W; break;
        case 4: VisaType = ISA_TYPE_D; break;
        default: VisaType = ISA_TYPE_Q; break;
      }
    }
  } else if (ElementTy->isHalfTy()) {
    VisaType = ISA_TYPE_HF;
    BytesPerElement = 2;
  } else if (ElementTy->isFloatTy()) {
    VisaType = ISA_TYPE_F;
    BytesPerElement = 4;
  } else if (auto PT = dyn_cast<PointerType>(ElementTy)) {
    BytesPerElement = DL.getPointerTypeSize(PT);
    if (BytesPerElement == 4)
      VisaType = ISA_TYPE_UD;
    else if (BytesPerElement == 8)
      VisaType = ISA_TYPE_UQ;
    else
      report_fatal_error("unsupported pointer type size");
  } else {
    assert(ElementTy->isDoubleTy());
    VisaType = ISA_TYPE_DF;
    BytesPerElement = 8;
  }
  if (NumElements > 8192 || NumElements * BytesPerElement > 8192 * 8)
    report_fatal_error("Variable too big");
}

/***********************************************************************
 * buildHeader1 : build the first part of the variable tables in the
 *                kernel/function header
 *
 * Enter: S = Stream to write the header fields into
 *
 * This outputs the following parts of the kernel/function header:
 *
 * uw variable_count;
 * var_info variables[variable_count];
 * uw address_count;
 * address_info addresses[address_count];
 * uw predicate_count;
 * predicate_info predicates[predicate_count];
 */
void GenXVisaRegAlloc::buildHeader1(Stream *S)
{
  auto &DL = FG->getModule()->getDataLayout();

  // Variables.
  S->push_back((uint32_t)(Regs[RegCategory::GENERAL].size() - VISA_NUM_RESERVED_REGS));
  for (unsigned Regnum = VISA_NUM_RESERVED_REGS, e = Regs[RegCategory::GENERAL].size();
      Regnum != e; ++Regnum) {
    // For each variable (not in the reserved range)...
    if (!Regs[RegCategory::GENERAL][Regnum].AliasTo) {
      // This is not an aliased register. Go through all the aliases and
      // determine the biggest alignment required. If the register is at least
      // as big as a GRF, make the alignment GRF.
      unsigned Alignment = 5; // GRF alignment
      Type *Ty = Regs[RegCategory::GENERAL][Regnum].Ty;
      unsigned NBits = Ty->isPointerTy() ? DL.getPointerSizeInBits()
                                         : Ty->getPrimitiveSizeInBits();
      if (NBits < 256 /* bits in GRF */) {
        Alignment = 0;
        for (unsigned AliasRegnum = Regnum; AliasRegnum; ) {
          Reg *R = &Regs[RegCategory::GENERAL][AliasRegnum];
          Type *AliasTy = R->Ty->getScalarType();
          unsigned ThisElementBytes =
              AliasTy->isPointerTy() ? DL.getPointerTypeSize(AliasTy)
                                     : AliasTy->getPrimitiveSizeInBits() / 8;
          unsigned LogThisElementBytes = llvm::log2(ThisElementBytes);
          if (LogThisElementBytes > Alignment)
            Alignment = LogThisElementBytes;
          if (R->Alignment > Alignment)
            Alignment = R->Alignment;
          AliasRegnum = R->NextAlias;
        }
      }
      for (unsigned AliasRegnum = Regnum; AliasRegnum; ) {
        Reg *R = &Regs[RegCategory::GENERAL][AliasRegnum];
        if (R->Alignment < Alignment)
          R->Alignment = Alignment;
        AliasRegnum = R->NextAlias;
      }
    }

    TypeDetails TD(DL, Regs[RegCategory::GENERAL][Regnum].Ty,
                   Regs[RegCategory::GENERAL][Regnum].Signed);
    unsigned Alignment = Regs[RegCategory::GENERAL][Regnum].Alignment;
    // Write a var_info for the variable.
    const Reg &R = Regs[RegCategory::GENERAL][Regnum];
    S->push_back((uint32_t)R.Name); // name_index
    S->push_back((uint8_t)(TD.VisaType | Alignment << 4)); // bit_properties
    S->push_back((uint16_t)TD.NumElements); // num_elements
    S->push_back((uint32_t)R.AliasTo); // alias_index
    S->push_back((uint16_t)0); // alias_offset
    S->push_back((uint8_t)0); // alias_scope_specifier
    R.Attributes.write<uint8_t>(S);
  }
  // Addresses.
  S->push_back((uint16_t)Regs[RegCategory::ADDRESS].size());
  for (unsigned Regnum = 0, e = Regs[RegCategory::ADDRESS].size();
      Regnum != e; ++Regnum) {
    const Reg &R = Regs[RegCategory::ADDRESS][Regnum];
    unsigned NumElements = 1;
    if (VectorType *VT = dyn_cast<VectorType>(R.Ty))
      NumElements = VT->getNumElements();
    S->push_back((uint32_t)R.Name); // name index
    S->push_back((uint16_t)NumElements); // num_elements
    R.Attributes.write<uint8_t>(S);
  }
  // Predicates.
  S->push_back((uint16_t)(Regs[RegCategory::PREDICATE].size() - VISA_NUM_RESERVED_PREDICATES));
  for (unsigned Regnum = VISA_NUM_RESERVED_PREDICATES, e = Regs[RegCategory::PREDICATE].size();
      Regnum != e; ++Regnum) {
    const Reg &R = Regs[RegCategory::PREDICATE][Regnum];
    unsigned NumElements = 1;
    if (VectorType *VT = dyn_cast<VectorType>(R.Ty))
      NumElements = VT->getNumElements();
    S->push_back((uint32_t)R.Name); // name index
    S->push_back((uint16_t)NumElements); // num_elements
    R.Attributes.write<uint8_t>(S);
  }
}

/***********************************************************************
 * buildHeader2 : build the second part of the variable tables in the
 *                kernel/function header
 *
 * Enter: S = Stream to write the header fields into
 *
 * This outputs the following parts of the kernel/function header:
 *
 * ub sampler_count;
 * var_info samplers[sampler_count];
 * ub surface_count;
 * var_info surfaces[surface_count];
 * ub vme_count;
 * var_info vmes[vme_count];
 */
void GenXVisaRegAlloc::buildHeader2(Stream *S)
{
  // Samplers, surfaces, vmes.
  for (unsigned Cat = RegCategory::SAMPLER; Cat <= RegCategory::VME; ++Cat) {
    if (Regs[Cat].size() >= NumReserved[Cat] + 256)
      report_fatal_error("Too many surface variables");
    S->push_back((uint8_t)(Regs[Cat].size() - NumReserved[Cat]));
    for (unsigned R = NumReserved[Cat], e = Regs[Cat].size();
        R != e; ++R) {
      assert(Regs[Cat][R].Ty->isIntegerTy(32) || "Wrong type for surface variable");
      // Write the state_var_info.
      TypeDetails TD(FG->getModule()->getDataLayout(), Regs[Cat][R].Ty,
                     Regs[Cat][R].Signed);
      S->push_back((uint32_t)Regs[Cat][R].Name); // name_index
      S->push_back((uint16_t)TD.NumElements); // num_elements (eg. Surface Array)
      S->push_back((uint8_t)0); // attribute_count
    }
  }
}

/***********************************************************************
 * print : dump the state of the pass. This is used by -genx-dump-regalloc
 */
void GenXVisaRegAlloc::print(raw_ostream &OS, const Module *M) const
{
  // Get the live ranges in a reproducible order, and sort them by "length"
  // (the total number of instructions that the live range covers).
  struct LiveRangeAndLength {
    LiveRange *LR;
    unsigned Length;
    LiveRangeAndLength(LiveRange *LR, unsigned Length) : LR(LR), Length(Length) {}
    bool operator<(const LiveRangeAndLength &Rhs) const { return Length > Rhs.Length; }
  };
  std::vector<LiveRange *> LRs;
  getLiveRanges(&LRs);
  std::vector<LiveRangeAndLength> LRLs;
  for (auto i = LRs.begin(), e = LRs.end(); i != e; ++i)
    LRLs.push_back(LiveRangeAndLength(*i, (*i)->getLength(/*WithWeak=*/ false)));
  LRs.clear();
  sort(LRLs.begin(), LRLs.end());
  // Dump them. Also keep count of the register pressure at each
  // instruction number.
  std::vector<unsigned> Pressure;
  std::vector<unsigned> FlagPressure;
  for (auto i = LRLs.begin(), e = LRLs.end(); i != e; ++i) {
    // Dump a single live range.
    LiveRange *LR = i->LR;
    SimpleValue SV = *LR->value_begin();
    RegNum RN = getRegNumForValueUntyped(SV);
    OS << "[";
    RN.print(OS);
    Type *ElTy = IndexFlattener::getElementType(SV.getValue()->getType(),
          SV.getIndex());
    unsigned Bytes = (ElTy->getPrimitiveSizeInBits() + 15U) / 8U & -2U;
    bool IsFlag = ElTy->getScalarType()->isIntegerTy(1);
    OS << "] (" << Bytes << " bytes, length " << i->Length <<") ";
    // Dump some indication of what the live range is. For a kernel argument,
    // show its name. For an instruction with debug info, show the location.
    // We try and find the earliest definition with debug info to show.
    unsigned BestNum = UINT_MAX;
    Instruction *BestInst = nullptr;
    Argument *KernelArg = nullptr;
    for (auto i = LR->value_begin(), e = LR->value_end(); i != e; ++i) {
      Value *V = i->getValue();
      if (auto Arg = dyn_cast<Argument>(V)) {
        if (Arg->getParent() == FG->getHead()) {
          KernelArg = Arg;
          break;
        }
      } else {
        auto Inst = cast<Instruction>(V);
        if (!isa<PHINode>(Inst)) {
          unsigned Num = Numbering->getNumber(Inst);
          if (Num < BestNum) {
            auto DL = Inst->getDebugLoc();
            if (!DL) {
              BestNum = Num;
              BestInst = Inst;
            }
          }
        }
      }
    }
    if (KernelArg)
      OS << KernelArg->getName();
    else if (BestInst) {
      DebugLoc DL = BestInst->getDebugLoc();
      OS << DL->getFilename() << ":" << DL.getLine();
    }
    // Dump the live range segments, and add each to the pressure score.
    OS << ":";
    LR->printSegments(OS);
    for (auto si = LR->begin(), se = LR->end(); si != se; ++si) {
      if (si->End >= Pressure.size()) {
        Pressure.resize(si->End + 1, 0);
        FlagPressure.resize(si->End + 1, 0);
      }
      for (unsigned n = si->Start; n != si->End; ++n) {
        Pressure[n] += Bytes;
        if (IsFlag)
          FlagPressure[n] += Bytes;
      }
    }
    OS << "\n";
  }
  OS << "\n";
  // Prepare to print register pressure info. First we need to compute a
  // mapping from instruction number to instruction. Only bother with
  // instructions with debug info.
  std::vector<Instruction *> Insts;
  for (auto fgi = FG->begin(), fge = FG->end(); fgi != fge; ++fgi) {
    Function *F = *fgi;
    for (auto fi = F->begin(), fe = F->end(); fi != fe; ++fi) {
      BasicBlock *BB = &*fi;
      for (auto bi = BB->begin(), be = BB->end(); bi != be; ++bi) {
        Instruction *Inst = &*bi;
        if (!Inst->getDebugLoc()) {
          unsigned Num = Numbering->getNumber(Inst);
          if (Num >= Insts.size())
            Insts.resize(Num + 1, nullptr);
          Insts[Num] = Inst;
        }
      }
    }
  }
  OS << "Register pressure (bytes):\n";
  unsigned Last = 0;
  bool HadInst = false;
  Function *LastFunc = nullptr;
  for (unsigned n = 0; n != Pressure.size(); ++n) {
    if (Pressure[n]) {
      Instruction *Inst = nullptr;
      if (n < Insts.size())
        Inst = Insts[n];
      if (Pressure[n] != Last)
        HadInst = false;
      if (Pressure[n] != Last || (!HadInst && Inst)) {
        if (Inst && Inst->getParent()->getParent() != LastFunc) {
          LastFunc = Inst->getParent()->getParent();
          OS << "In " << LastFunc->getName() << "\n";
        }
        Last = Pressure[n];
        OS << Pressure[n] << " at " << n;
        if (Inst) {
          HadInst = true;
          OS << " ";
          DebugLoc DL = Inst->getDebugLoc();
          DL.print(OS);
        }
        OS << "\n";
      }
    }
  }
  OS << "Flag pressure (bytes):\n";
  Last = 0;
  HadInst = false;
  for (unsigned n = 0; n != FlagPressure.size(); ++n) {
    Instruction *Inst = nullptr;
    if (n < Insts.size())
      Inst = Insts[n];
    if (FlagPressure[n] != Last)
      HadInst = false;
    if (FlagPressure[n] != Last || (!HadInst && Inst)) {
      Last = FlagPressure[n];
      OS << FlagPressure[n] << " at " << n;
      if (Inst) {
        HadInst = true;
        DebugLoc DL = Inst->getDebugLoc();
        OS << " " << DL->getFilename() << ":" << DL.getLine();
      }
      OS << "\n";
    }
  }
}

/***********************************************************************
 * RegNum::print : print a regnum
 */
void GenXVisaRegAlloc::RegNum::print(raw_ostream &OS)
{
  switch (Category) {
    case RegCategory::NONE: OS << "-"; return;
    case RegCategory::GENERAL: OS << "v"; break;
    case RegCategory::ADDRESS: OS << "a"; break;
    case RegCategory::PREDICATE: OS << "p"; break;
    case RegCategory::SAMPLER: OS << "s"; break;
    case RegCategory::SURFACE: OS << "t"; break;
    case RegCategory::VME: OS << "vme"; break;
    default: OS << "?"; break;
  }
  OS << Num;
}

