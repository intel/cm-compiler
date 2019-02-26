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

#include "PacketBuilder.h"
#include "llvm/Pass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Transforms/Scalar/CMRegion.h"
#include "llvm/Transforms/Scalar/LowerCMSimdCF.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <algorithm>
#include <set>
#include <stack>
#include <unordered_set>

using namespace pktz;

namespace llvm {

////////////////////////////////////////////////////////////////////////////
/// GenXPacketize - brutal-force-vectorization Pass for CM SIMT functions
/// @brief Packetize the function, which includes:
///   - Replace all genx scalar intrinsics with vector versions
///   - Replace all scalar function calls with their vector equivalent
///   - Replace generic control-flow with SIMD control-flow
class GenXPacketize : public ModulePass {
public:
  static char ID;
  explicit GenXPacketize() : ModulePass(ID) {}
  ~GenXPacketize() { releaseMemory();  }
  virtual StringRef getPassName() const { return "GenX Packetize"; }
  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredID(BreakCriticalEdgesID);
  };
  bool runOnModule(Module &M);
  void releaseMemory() override {
    ReplaceMap.clear();
    UniformSet.clear();
    FuncOrder.clear();
    FuncVectors.clear();
    FuncMap.clear();
  }
private:
  void findFunctionVectorizationOrder(Module *M);

  Value *getPacketizeValue(Value *ScalarValue);
  Function *getVectorIntrinsic(Module *M, Intrinsic::ID id, std::vector<Type *> &ArgTy);
  Value *packetizeConstant(Constant *pConstant);
  Value *packetizeGenXIntrinsic(Instruction *pInst);
  Value *packetizeLLVMIntrinsic(Instruction *pInst);
  Value *packetizeLLVMInstruction(Instruction *pInst);
  Value *packetizeInstruction(Instruction *pInst);

  void replaceAllUsesNoTypeCheck(Value *pInst, Value *pNewInst);
  void removeDeadInstructions(Function &F);
  void fixupLLVMIntrinsics(Function &F);

  Function *vectorizeSIMTFunction(Function *F, unsigned Width);
  bool      vectorizeSIMTEntry(Function &F);

  void findUniforms(Function &F);

  void lowerControlFlowAfter(std::vector<Function *> &SIMTFuncs);
  GlobalVariable *findGlobalExecMask();

private:
  Module *M;
  PacketBuilder *B;

  // track already packetized values
  ValueToValueMapTy ReplaceMap;

  /// uniform set for alloca, load, store, and GEP
  std::set<const Instruction*> UniformSet;
  /// sort function in caller-first order
  std::vector<Function *> FuncOrder;
  /// map: function ==> a set of vectorization width 
  std::map<Function*, std::set<unsigned>> FuncVectors;
  /// Map: original function and vectorization width ==> vectorized version
  std::map<std::pair<Function*, unsigned>, Function*> FuncMap;

  const DataLayout *DL;
};

bool GenXPacketize::runOnModule(Module &Module)
{
  M = &Module;

  // find all the SIMT enntry-functions
  // also find the set of uniform allocas, loads, and stores
  UniformSet.clear();
  std::vector<Function*> ForkFuncs;
  for (auto &F : M->getFunctionList()) {
    if (F.hasFnAttribute("CMGenxSIMT")) {
      uint32_t Width = 0;
      F.getFnAttribute("CMGenxSIMT").getValueAsString()
        .getAsInteger(0, Width);
      if (Width > 1) {
        assert(Width == 8 || Width == 16 || Width == 32);
        ForkFuncs.push_back(&F);
      }
#if 0
      if (F.hasFnAttribute("CMGenxInline")) {
        F.removeFnAttr("CMGenxInline");
        F.removeFnAttr(Attribute::NoInline);
        F.addFnAttr(Attribute::AlwaysInline);
      }
#endif
    }
  }
  if (ForkFuncs.empty())
    return false;
  auto DemotePass = createDemoteRegisterToMemoryPass();
  for (auto &F : M->getFunctionList()) {
    DemotePass->runOnFunction(F);
  }
  for (auto F : ForkFuncs)
    findUniforms(*F);
  // sort functions in order, also find those functions that are used in 
  // the SIMT mode, therefore need whole-function vectorization.
  findFunctionVectorizationOrder(M);

  DL = &(M->getDataLayout());
  B = new PacketBuilder(M);
  std::vector<Function*> SIMTFuncs;
  // Process those functions called in the SIMT mode
  unsigned NumFunc = FuncOrder.size();
  for (int i = NumFunc - 1;  i >= 0; --i) {
    auto F = FuncOrder[i];
    auto iter = FuncVectors.find(F);
    if (iter != FuncVectors.end()) {
      fixupLLVMIntrinsics(*F);
      auto WV = iter->second;
      for (auto W : WV) {
        auto VF = vectorizeSIMTFunction(F, W);
        auto Key = std::pair<Function*, unsigned>(F, W);
        FuncMap.insert(std::pair<std::pair<Function*, unsigned>, Function*>(Key, VF));
        SIMTFuncs.push_back(VF);
      }
    }
  }

  // vectorize SIMT entry-functions
  bool Modified = false;
  for (auto F : ForkFuncs) {
    Modified |= vectorizeSIMTEntry(*F);
    SIMTFuncs.push_back(&(*F));
  }

  delete B;

  // lower the SIMD control-flow
  lowerControlFlowAfter(SIMTFuncs);

  return Modified;
}

/***************************************************************************
* vectorize a functions that is used in the fork-region
*/
Function *GenXPacketize::vectorizeSIMTFunction(Function *F, unsigned Width)
{
  assert(!F->hasFnAttribute("CMGenxSIMT"));
  B->SetTargetWidth(Width);

  // vectorize the argument and return types
  std::vector<Type*> ArgTypes;
  for (const Argument &I : F->args())
      ArgTypes.push_back(B->GetVectorType(I.getType()));
  Type* RetTy = B->GetVectorType(F->getReturnType());
  // Create a new function type...
  assert(!F->isVarArg());
  FunctionType* FTy = FunctionType::get(RetTy, ArgTypes, false);

  // Create the vector function prototype
  StringRef VecFName = F->getName();
  const char* Suffix[] = {".vec00", ".vec08", ".vec16", ".vec24", ".vec32" };
  Function *ClonedFunc =
    Function::Create(FTy, F->getLinkage(), VecFName + Suffix[Width/8], F->getParent());
  ClonedFunc->setCallingConv(F->getCallingConv());
  ClonedFunc->setAttributes(F->getAttributes());
  ClonedFunc->setAlignment(F->getAlignment());

  // then use CloneFunctionInto
  ValueToValueMapTy ArgMap;
  Function::arg_iterator ArgI = ClonedFunc->arg_begin();
  for (Function::const_arg_iterator I = F->arg_begin(),
       E = F->arg_end(); I != E; ++I) {
    ArgI->setName(I->getName()); // Copy the name over...
    ArgMap[I] = ArgI++;        // Add mapping to ValueMap
  }
  SmallVector<ReturnInst*, 10> returns;
  ClonedCodeInfo CloneInfo;
  CloneFunctionInto(ClonedFunc, F, ArgMap, false, returns, Suffix[Width/8], &CloneInfo);

  ReplaceMap.clear();
  // vectorize instructions in the fork-regions
  for (auto I = ClonedFunc->begin(), E = ClonedFunc->end(); I != E; ++I) {
    BasicBlock *BB = &*I;
    for (auto &I : BB->getInstList()) {
      Value *pPacketizedInst = packetizeInstruction(&I);
      ReplaceMap[&I] = pPacketizedInst;
    }
  }

  removeDeadInstructions(*ClonedFunc);

  return ClonedFunc;
}

/***************************************************************************
 * vectorize a SIMT-entry function
 */
bool GenXPacketize::vectorizeSIMTEntry(Function &F)
{
  assert(F.hasFnAttribute("CMGenxSIMT"));
  uint32_t Width = 0;
  F.getFnAttribute("CMGenxSIMT").getValueAsString()
    .getAsInteger(0, Width);

  B->SetTargetWidth(Width);

  ReplaceMap.clear();

  B->IRB()->SetInsertPoint(&F.getEntryBlock(), F.getEntryBlock().begin());

  fixupLLVMIntrinsics(F);

  // vectorize instructions in the fork-regions
  for (auto I = F.begin(), E = F.end(); I != E; ++I) {
    BasicBlock *BB = &*I;
    for (auto &I : BB->getInstList()) {
      if (!UniformSet.count(&I)) {
        Value *pPacketizedInst = packetizeInstruction(&I);
        ReplaceMap[&I] = pPacketizedInst;
      }
      else {
        for (int i = 0, n = I.getNumOperands(); i < n; ++i) {
          Value *OrigValue = I.getOperand(i);
          auto iter = ReplaceMap.find(OrigValue);
          if (iter != ReplaceMap.end() && iter->second != OrigValue) {
            I.setOperand(i, iter->second);
          }
        }
      }
    }
  }

  removeDeadInstructions(F);

  return true;
}

/************************************************************************
 * findFunctionVectorizationOrder : calculate the order we want to visit
 * functions, such that a function is not visited until all its callees 
 * have been visited. Also if a function is called directly or indirectly
 * in the SIMT mode, add it to the list that need vectorization
 */
 // Call graph node
struct CGNode {
  Function *F;
  std::set<CGNode *> UnvisitedCallers;
  std::set<CGNode *> Callees;
};

void GenXPacketize::findFunctionVectorizationOrder(Module *M)
{
  // First build the call graph.
  // We roll our own call graph here, because it is simpler than the general
  // case supported by LLVM's call graph analysis (CM does not support
  // recursion or function pointers), and we want to modify it (using the
  // UnvisitedCallers set) when we traverse it.
  std::map<Function *, CGNode> CallGraph;
  for (auto mi = M->begin(), me = M->end(); mi != me; ++mi) {
    Function *F = &*mi;
    if (F->empty())
      continue;
    // For each defined function: for each use (a call), add it to our
    // UnvisitedCallers set, and add us to its Callees set.
    // We are ignoring an illegal non-call use of a function; someone
    // else can spot and diagnose that later.
    // If the function has no callers, then add it straight in to FuncOrder.
    CGNode *CGN = &CallGraph[F];
    CGN->F = F;
    if (F->use_empty()) {
      FuncOrder.push_back(F);
      continue;
    }
    for (auto ui = F->use_begin(), ue = F->use_end(); ui != ue; ++ui) {
      if (auto CI = dyn_cast<CallInst>(ui->getUser())) {
        BasicBlock *Blk = CI->getParent();
        Function *Caller = Blk->getParent();
        CGNode *CallerNode = &CallGraph[Caller];
        CallerNode->F = Caller;
        CGN->UnvisitedCallers.insert(CallerNode);
        CallerNode->Callees.insert(CGN);
        // find the vectorization width of callee
        auto CallerVectorIter = FuncVectors.find(Caller);
        if (CallerVectorIter != FuncVectors.end()) {
          auto CalleeVectorIter = FuncVectors.find(F);
          if (CalleeVectorIter != FuncVectors.end())
            CalleeVectorIter->second.insert(CallerVectorIter->second.begin(),
                                            CallerVectorIter->second.end());
          else
            FuncVectors.insert(std::pair<Function*, std::set<unsigned>>(F, CallerVectorIter->second));
        }
        else if (Caller->hasFnAttribute("CMGenxSIMT")){
          uint32_t width = 0;
          Caller->getFnAttribute("CMGenxSIMT").getValueAsString()
            .getAsInteger(0, width);
          if (width > 1) {
            auto CalleeVectorIter = FuncVectors.find(F);
            if (CalleeVectorIter != FuncVectors.end())
              CalleeVectorIter->second.insert(width);
            else {
              std::set<unsigned> WidthSet;
              WidthSet.insert(width);
              FuncVectors.insert(std::pair<Function*, std::set<unsigned>>(F, WidthSet));
            }
          }
        }
      }
    }
  }
  // Run through the visit order. For each function, remove it from each
  // callee's UnvisitedCallers set, and, if now empty, add the callee to
  // the end of the visit order.
  for (unsigned i = 0; i != FuncOrder.size(); ++i) {
    CGNode *CGN = &CallGraph[FuncOrder[i]];
    for (auto ci = CGN->Callees.begin(), ce = CGN->Callees.end();
      ci != ce; ++ci) {
      CGNode *Callee = *ci;
      Callee->UnvisitedCallers.erase(CGN);
      if (Callee->UnvisitedCallers.empty())
        FuncOrder.push_back(Callee->F);
      // find the vectorization width of callee
      auto CallerVectorIter = FuncVectors.find(CGN->F);
      if (CallerVectorIter != FuncVectors.end()) {
        auto CalleeVectorIter = FuncVectors.find(Callee->F);
        if (CalleeVectorIter != FuncVectors.end())
          CalleeVectorIter->second.insert(CallerVectorIter->second.begin(),
                                          CallerVectorIter->second.end());
        else
          FuncVectors.insert(std::make_pair(Callee->F, CallerVectorIter->second));
      }
    }
  }
}

void GenXPacketize::findUniforms(Function &F)
{
  // first find out all the uniform alloca
  std::stack<Value*> uvset;
  for (const Argument &I : F.args()) {
    for (auto UI = I.user_begin(), E = I.user_end(); UI != E; ++UI) {
      const Value *use = (*UI);
      if (auto LI = dyn_cast<LoadInst>(use)) {
        UniformSet.insert(LI);
      }
      else if (auto GEP = dyn_cast<GetElementPtrInst>(use)) {
        if (GEP->getPointerOperand() == &I) {
          UniformSet.insert(GEP);
          uvset.push((Value *)GEP);
        }
      }
      else if (auto SI = dyn_cast<StoreInst>(use)) {
        if (SI->getPointerOperand() == &I)
          UniformSet.insert(SI);
        else {
          auto PI = SI->getPointerOperand();
          if (auto AI = dyn_cast<AllocaInst>(PI)) {
            UniformSet.insert(AI);
            uvset.push((Value *)AI);
          }
        }
      }
      else if (auto CI = dyn_cast<CallInst>(use)) {
        if (Function *Callee = CI->getCalledFunction()) {
          Intrinsic::ID IID = (Intrinsic::ID)Callee->getIntrinsicID();
          if (IID == Intrinsic::genx_vload ||
              IID == Intrinsic::genx_vstore) {
            UniformSet.insert(CI);
          }
        }
      }
    }
  }

  // then find the uniform loads and stores in fork-region
  while (!uvset.empty()) {
    Value *Def = uvset.top();
    uvset.pop();
    for (auto UI = Def->user_begin(), E = Def->user_end(); UI != E; ++UI) {
      Value *use = (*UI);
      if (auto UseI = dyn_cast<Instruction>(use)) {
        if (isa<LoadInst>(UseI) || isa<StoreInst>(UseI)) {
          UniformSet.insert(UseI);
        }
        else if (isa<GetElementPtrInst>(UseI)) {
          uvset.push(UseI);
          UniformSet.insert(UseI);
        }
      }
    }
  }
  return;
}

Value *GenXPacketize::getPacketizeValue(Value *ScalarValue)
{
  auto iter = ReplaceMap.find(ScalarValue);
  if (iter != ReplaceMap.end()) {
    return iter->second;
  }
  else if (auto C = dyn_cast<Constant>(ScalarValue)) {
    return packetizeConstant(C);
  }
  else if (auto A = dyn_cast<Argument>(ScalarValue)) {
    auto V = B->VBROADCAST(ScalarValue, ScalarValue->getName());
    ReplaceMap[ScalarValue] = V;
    return V;
  }
  else if (auto Inst = dyn_cast<Instruction>(ScalarValue)) {
    // need special handling for alloca
    if (auto AI = dyn_cast<AllocaInst>(ScalarValue)) {
      // this is not a uniform alloca
      if (!UniformSet.count(Inst)) {
        Type *VecType = B->GetVectorType(AI->getAllocatedType());
        auto V = B->ALLOCA(VecType, nullptr, AI->getName());
        V->removeFromParent();
        V->insertBefore(Inst);
        ReplaceMap[ScalarValue] = V;
        return V;
      }
      ReplaceMap[ScalarValue] = ScalarValue;
      return ScalarValue;
    }
    else if (UniformSet.count(Inst)) {
      auto V = B->VBROADCAST(ScalarValue);
      return V;
    }
  }

  report_fatal_error("Could not find packetized value!");

  return nullptr;
}

//////////////////////////////////////////////////////////////////////////
/// @brief Returns the equivalent vector intrinsic for the input scalar
/// intrinsic
Function *GenXPacketize::getVectorIntrinsic(Module *M, Intrinsic::ID id,
  std::vector<Type *> &ArgTy) 
{
  if (id == Intrinsic::fma) {
    return Intrinsic::getDeclaration(M, id, B->mSimdFP32Ty);
  } else if (id == Intrinsic::pow) {
    // for some reason, passing the 2 vector input args to the pow declaration
    // results in a malformed vectored pow intrinsic. Forcing the expected
    // vector input here.
    return Intrinsic::getDeclaration(M, id, B->mSimdFP32Ty);
  } else if ((id == Intrinsic::maxnum) || (id == Intrinsic::minnum)) {
    return Intrinsic::getDeclaration(M, id, ArgTy[0]);
  } else {
    return Intrinsic::getDeclaration(M, id, ArgTy);
  }
}

//////////////////////////////////////////////////////////////////////////
/// @brief Determines if instruction is an llvm intrinsic (which may include
///        x86 intrinsics
static bool IsLLVMIntrinsic(Instruction *pInst)
{
  if (isa<CallInst>(pInst)) {
    CallInst *call = cast<CallInst>(pInst);
    Function *f = call->getCalledFunction();
    return f->isIntrinsic();
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////
/// @brief Packetize a scalar constant
Value *GenXPacketize::packetizeConstant(Constant *pConstant) 
{
  if (isa<UndefValue>(pConstant)) {
    return UndefValue::get(B->GetVectorType(pConstant->getType()));
  } else {
    return B->VBROADCAST(pConstant);
  }
}

//////////////////////////////////////////////////////////////////////////
/// @brief Packetize an LLVM intrinsic.  Generally this means replacing
///        a scalar intrinsic function call with a vectored equivalent.
Value *GenXPacketize::packetizeLLVMIntrinsic(Instruction *pInst)
{
  Module *M = B->mpModule;

  B->IRB()->SetInsertPoint(pInst);
  CallInst *pCall = cast<CallInst>(pInst);
  Function *f = pCall->getCalledFunction();
  assert(f && f->isIntrinsic());
  Intrinsic::ID id = (Intrinsic::ID)f->getIntrinsicID();

  // packetize intrinsic operands
  std::vector<Type *> vectorArgTys;
  std::vector<Value *> packetizedArgs;
  for (auto &operand : pCall->arg_operands()) {
    vectorArgTys.push_back(B->GetVectorType(operand.get()->getType()));
    packetizedArgs.push_back(getPacketizeValue(operand.get()));
  }

  // override certain intrinsics
  Value *pNewCall;
  switch (id) {
  case Intrinsic::log2:
    pNewCall = B->VLOG2PS(packetizedArgs[0]);
    break;
  case Intrinsic::exp2:
    pNewCall = B->VEXP2PS(packetizedArgs[0]);
    break;
  default: {
    Function *newF = getVectorIntrinsic(M, id, vectorArgTys);
    pNewCall = CallInst::Create(newF, packetizedArgs, "", pCall);
  }
  }
  return pNewCall;
}

Value *GenXPacketize::packetizeLLVMInstruction(Instruction *pInst) 
{
  Value *pReplacedInst = nullptr;
  B->IRB()->SetInsertPoint(pInst);
  // packetize a call 
  if (auto CI = dyn_cast<CallInst>(pInst)) {
    auto F = CI->getCalledFunction();
    auto FMI = FuncMap.find(std::pair<Function*, unsigned>(F, B->mVWidth));
    if (FMI != FuncMap.end()) {
      std::vector<Value*> ArgOps;
      for (unsigned i = 0, e = CI->getNumArgOperands(); i < e; ++i) {
        ArgOps.push_back(getPacketizeValue(CI->getArgOperand(i)));
      }
      pReplacedInst = CallInst::Create(FMI->second, ArgOps, CI->getName(), CI);
      return pReplacedInst;
    }
    else
      assert(false);
  }
  uint32_t opcode = pInst->getOpcode();

  switch (opcode) {
  case Instruction::AddrSpaceCast:
  case Instruction::BitCast: {
    // packetize the bitcast source
    Value *pPacketizedSrc = getPacketizeValue(pInst->getOperand(0));
    Type *pPacketizedSrcTy = pPacketizedSrc->getType();

    // packetize dst type
    Type *pReturnTy;
    if (pInst->getType()->isPointerTy()) {
      // two types of pointers, <N x Ty>* or <N x Ty*>
      Type *pDstScalarTy = pInst->getType()->getPointerElementType();

      if (pPacketizedSrc->getType()->isVectorTy()) {
        // <N x Ty*>
        Type *pDstPtrTy = PointerType::get(
            pDstScalarTy, pInst->getType()->getPointerAddressSpace());
        uint32_t numElems = pPacketizedSrcTy->getVectorNumElements();
        pReturnTy = VectorType::get(pDstPtrTy, numElems);
      } else {
        // <N x Ty>*
        pReturnTy =
            PointerType::get(B->GetVectorType(pDstScalarTy),
                              pInst->getType()->getPointerAddressSpace());
      }
    } else {
      pReturnTy = B->GetVectorType(pInst->getType());
    }

    pReplacedInst =
        B->CAST((Instruction::CastOps)opcode, pPacketizedSrc, pReturnTy);
    break;
  }

  case Instruction::GetElementPtr: {
    GetElementPtrInst *pGepInst = cast<GetElementPtrInst>(pInst);
    Value *pVecSrc = getPacketizeValue(pGepInst->getOperand(0));

    if (pVecSrc->getType()->isVectorTy()) {
      // AOS GEP with vector source, just packetize the GEP to a vector GEP.
      // Ex. gep <8 x float*>
      // Result will be <N x Ty*>
      assert(pVecSrc->getType()->getVectorElementType()->isPointerTy());
      SmallVector<Value *, 8> vecIndices;
      for (uint32_t i = 0; i < pGepInst->getNumIndices(); ++i) {
        vecIndices.push_back(getPacketizeValue(pGepInst->getOperand(1 + i)));
      }
      pReplacedInst = B->GEPA(pVecSrc, vecIndices);
    } else {
      if (pGepInst->hasAllConstantIndices()) {
        // SOA GEP with scalar src and constant indices, result will be <N x
        // Ty>* Ex. gep [4 x <8 x float>]*, 0, 0 --> <8 x float>*
        SmallVector<Value *, 8> vecIndices;
        for (uint32_t i = 0; i < pGepInst->getNumIndices(); ++i) {
          vecIndices.push_back(pGepInst->getOperand(1 + i));
        }
        pReplacedInst = B->GEPA(pVecSrc, vecIndices);
      } else {
        //// SOA GEP with non-uniform indices. Need to vector GEP to each SIMD
        ///lane.
        /// Result will be <N x Ty*>
        SmallVector<Value *, 8> vecIndices;
        for (uint32_t i = 0; i < pGepInst->getNumIndices(); ++i) {
          vecIndices.push_back(
              getPacketizeValue(pGepInst->getOperand(1 + i)));
        }

        // Step to the SIMD lane
        if (B->mVWidth == 8) {
          vecIndices.push_back(B->C({0, 1, 2, 3, 4, 5, 6, 7}));
        } else if (B->mVWidth == 16) {
          vecIndices.push_back(
              B->C({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
        } else {
          report_fatal_error("Unsupported SIMD width.");
        }

        pReplacedInst = B->GEPA(pVecSrc, vecIndices);
      }
    }
    break;
  }

  case Instruction::Load: {
    LoadInst *pLoadInst = cast<LoadInst>(pInst);
    Value *pSrc = pLoadInst->getPointerOperand();
    Value *pVecSrc = getPacketizeValue(pSrc);
    if (pVecSrc == pSrc)
      pReplacedInst = pInst;
    else
      pReplacedInst = B->LOAD(pVecSrc);
    break;
  }

  case Instruction::Store: {
    StoreInst *pStoreInst = cast<StoreInst>(pInst);
    Value *pVecDstPtrs = getPacketizeValue(pStoreInst->getPointerOperand());
    Value *pVecSrc = getPacketizeValue(pStoreInst->getOperand(0));
    pReplacedInst = B->STORE(pVecSrc, pVecDstPtrs);
    break;
  }

  case Instruction::ExtractElement: {
    auto Vec = getPacketizeValue(pInst->getOperand(0));
    auto Idx = pInst->getOperand(1);
    if (isa<Constant>(Idx) && Vec->getType()->getVectorNumElements() == B->mVWidth )
      pReplacedInst = B->BITCAST(Vec, Vec->getType());
    else
      report_fatal_error(
        "ExtractElement instructions should've been replaced by Scalarizer.");
    break;
  }

  case Instruction::InsertElement: {
    auto Vec = pInst->getOperand(0);
    auto Elm = getPacketizeValue(pInst->getOperand(1));
    auto Idx = pInst->getOperand(2);
    if (isa<UndefValue>(Vec) && isa<Constant>(Idx))
      pReplacedInst = B->BITCAST(Elm, Elm->getType());
    else
      report_fatal_error(
        "InsertElement instructions should've been replaced by Scalarizer.");
    break;
  }

  case Instruction::Br: {
    // any conditional branches with vectored conditions need to preceded with
    // a genx_simdcf_any to ensure we branch iff all lanes are set
    BranchInst *pBranch = cast<BranchInst>(pInst);
    if (pBranch->isConditional()) {
      Value *vCondition = getPacketizeValue(pBranch->getCondition());
      unsigned IID = Intrinsic::genx_simdcf_any;
      llvm::Function *NewFn = Intrinsic::getDeclaration(
        B->mpModule,
        (llvm::Intrinsic::ID)IID,
        vCondition->getType());
      llvm::CallInst *NewTest = CallInst::Create(NewFn, vCondition, "", pInst);
      NewTest->setName("exit.cond.mask.test");
      pBranch->setCondition(NewTest);
    }
    pReplacedInst = pBranch;
    break;
  }

  case Instruction::PHI: {
    Type *vecType = B->GetVectorType(pInst->getType());
    pInst->mutateType(vecType);
    pReplacedInst = pInst;
    break;
  }

  case Instruction::Alloca: {
    AllocaInst *pAllocaInst = cast<AllocaInst>(pInst);
    Type *pVecType = B->GetVectorType(pAllocaInst->getAllocatedType());
    Value *pReturn = B->ALLOCA(pVecType, nullptr, pInst->getName());
    pReplacedInst = pReturn;
    break;
  }

  case Instruction::FPExt: {
    // convert fpext of half type to CVTPH2PS intrinsic
    // llvm seems to have issues with codegen of fpext for x86
    Value *pSrc =
        B->BITCAST(getPacketizeValue(pInst->getOperand(0)), B->mSimdInt16Ty);
    Value *pReturn = B->VCVTPH2PS(pSrc);
    pReplacedInst = pReturn;
    break;
  }

  case Instruction::ShuffleVector: {
    auto Src1 = pInst->getOperand(0);
    auto Src2 = pInst->getOperand(1);
    auto Mask = pInst->getOperand(2);
    if (Src1->getType()->getVectorNumElements() == 1 &&
        Mask->getType()->getVectorNumElements() == 1) {
      if (cast<Constant>(Mask)->isAllOnesValue())
        pReplacedInst = getPacketizeValue(Src2);
      else
        pReplacedInst = getPacketizeValue(Src1);
    }
    else
      report_fatal_error(
        "ShuffleVector should've been replaced by Scalarizer.");
    break;
  }

  case Instruction::IntToPtr: {
    IntToPtrInst *pIntToPtrInst = cast<IntToPtrInst>(pInst);
    Value *pVecSrc = getPacketizeValue(pInst->getOperand(0));
    Type *pVecDestTy =
        VectorType::get(pIntToPtrInst->getDestTy(), B->mVWidth);
    pReplacedInst = B->INT_TO_PTR(pVecSrc, pVecDestTy);
    break;
  }

  case Instruction::Select: {
    Value *pVecCond = getPacketizeValue(pInst->getOperand(0));
    Value *pTrueSrc = getPacketizeValue(pInst->getOperand(1));
    Value *pFalseSrc = getPacketizeValue(pInst->getOperand(2));

    if (!pTrueSrc->getType()->isPointerTy()) {
      // simple select packetization
      pReplacedInst = B->SELECT(pVecCond, pTrueSrc, pFalseSrc);
    } else {
      // vector struct input, need to loop over components and build up new
      // struct allocation
      Value *pAlloca = B->ALLOCA(
          B->GetVectorType(pInst->getType()->getPointerElementType()));
      uint32_t numElems =
          pInst->getType()->getPointerElementType()->getArrayNumElements();

      for (uint32_t i = 0; i < numElems; ++i) {
        Value *pTrueSrcElem = B->LOAD(pTrueSrc, {0, i});
        Value *pFalseSrcElem = B->LOAD(pFalseSrc, {0, i});

        // mask store true components
        Value *pGep = B->GEP(pAlloca, {0, i});
        B->MASKED_STORE(pTrueSrcElem, pGep, 4, pVecCond);

        // store false components to inverted mask
        B->MASKED_STORE(pFalseSrcElem, pGep, 4, B->NOT(pVecCond));
      }
      pReplacedInst = pAlloca;
    }
    break;
  }

  case Instruction::Ret: {
    ReturnInst *pRet = cast<ReturnInst>(pInst);
    if (pRet->getReturnValue() != nullptr) {
      Value *pReturn = getPacketizeValue(pRet->getReturnValue());
      ReturnInst *pNewRet = B->RET(pReturn);
      pReplacedInst = pNewRet;
    } else {
      pReplacedInst = pInst;
    }

    break;
  }

  default: {
    // for the rest of the instructions, vectorize the instruction type as
    // well as its args
    Type *vecType = B->GetVectorType(pInst->getType());
    pInst->mutateType(vecType);

    for (Use &op : pInst->operands()) {
      op.set(getPacketizeValue(op.get()));
    }
    pReplacedInst = pInst;
  }
  }

  return pReplacedInst;
}


Value* GenXPacketize::packetizeGenXIntrinsic(Instruction* inst)
{
  B->IRB()->SetInsertPoint(inst);

  if (auto CI = dyn_cast_or_null<CallInst>(inst)) {
    if (Function *Callee = CI->getCalledFunction()) {
      Intrinsic::ID IID = (Intrinsic::ID)Callee->getIntrinsicID();
      Value*    replacement = nullptr;
      switch (IID) {
        case Intrinsic::genx_gather_scaled: {
          Value *Predicate = getPacketizeValue(CI->getOperand(0));
          Value *NBlk = CI->getOperand(1);
          assert(isa<Constant>(NBlk));
          Value *Scale = CI->getOperand(2);
          assert(isa<Constant>(Scale));
          Value *BTIV = getPacketizeValue(CI->getOperand(3));
          auto BTI = B->VEXTRACT(BTIV, (uint64_t)0, CI->getOperand(3)->getName());
          Value *GOff = CI->getOperand(4);
          if (!isa<Constant>(GOff)) {
            Value *GOffV = getPacketizeValue(CI->getOperand(4));
            GOff = B->VEXTRACT(GOffV, (uint64_t)0, CI->getOperand(4)->getName());
          }
          Value *ElemOffsets = getPacketizeValue(CI->getOperand(5));
          Value *InData = getPacketizeValue(CI->getOperand(6));
          Value *Args[] = {
            Predicate, NBlk, Scale, BTI, GOff, ElemOffsets, InData
          };
          auto RetTy = B->GetVectorType(CI->getType());
          Type *Tys[] = { RetTy, Args[0]->getType(), Args[5]->getType() };
          auto Decl = Intrinsic::getDeclaration(M, (Intrinsic::ID)IID, Tys);
          replacement = CallInst::Create(Decl, Args, CI->getName(), CI);
          cast<CallInst>(replacement)->setDebugLoc(CI->getDebugLoc());
          return replacement;
        }
        break;
        case Intrinsic::genx_lane_id: {
          assert((CI->getType()->getIntegerBitWidth() == 32) &&
            "Expected to return 32-bit integer.");
          if (B->mVWidth == 8) {
            std::initializer_list<uint32_t> l = { 0, 1, 2, 3, 4, 5, 6, 7 };
            replacement = B->C(l);
          }
          else if (B->mVWidth == 16) {
            std::initializer_list<uint32_t> l = { 0, 1, 2, 3, 4, 5, 6, 7,
                                               8, 9, 10, 11, 12, 13, 14, 15 };
            replacement = B->C(l);
          }
          else if (B->mVWidth == 32) {
            std::initializer_list<uint32_t> l = { 0, 1, 2, 3, 4, 5, 6, 7,
                                            8, 9, 10, 11, 12, 13, 14, 15,
                                            16, 17, 18, 19, 20, 21, 22, 23,
                                            24, 25, 26, 27, 28, 29, 30, 31 };
            replacement = B->C(l);
          }
          else
            assert(false);
          return replacement;
        }
        break;
        case Intrinsic::genx_rdregionf:
        case Intrinsic::genx_rdregioni: {
          // packetize intrinsic operands
          DebugLoc DL = CI->getDebugLoc();
          auto OrigV0 = CI->getOperand(0);
          CMRegion R(CI);
          assert(R.Width == 1);
          if (OrigV0->getType()->getVectorNumElements() == 1) {
            replacement = getPacketizeValue(OrigV0);
          }
          else {
            R.NumElements = B->mVWidth;
            if (R.Indirect) {
              R.Indirect = getPacketizeValue(R.Indirect);
            }
            replacement = R.createRdRegion(getPacketizeValue(OrigV0), CI->getName(), CI, DL);
          }
          return replacement;
        }
        break;
        case Intrinsic::genx_wrregionf:
        case Intrinsic::genx_wrregioni: {
          auto NewV0 = CI->getOperand(1);
          DebugLoc DL = CI->getDebugLoc();
          CMRegion R(CI);
          assert(isa<VectorType>(NewV0->getType()));
          assert(NewV0->getType()->getVectorNumElements() == 1);
          auto NewV1 = getPacketizeValue(NewV0);
          R.NumElements = B->mVWidth;
          if (R.Indirect) {
            R.Indirect = getPacketizeValue(R.Indirect);
          }
          replacement = R.createWrRegion(CI->getOperand(0), NewV1, CI->getName(), CI, DL);
          return replacement;
        }
        break;
        default:
          break;
      }
    }
  }
  return nullptr;
}

/// - map old instruction to new in case we revisit the old instruction
Value *GenXPacketize::packetizeInstruction(Instruction *pInst) 
{
  // determine instruction type and call its packetizer
  Value *pResult = packetizeGenXIntrinsic(pInst);
  if (!pResult) {
    if (IsLLVMIntrinsic(pInst))
      pResult = packetizeLLVMIntrinsic(pInst);
    else
      pResult = packetizeLLVMInstruction(pInst);
  }

  if (pResult) {
    if (pInst->getName() != "") {
      pResult->setName(pInst->getName());
    }

    // Copy any metadata to new instruction
    if (pResult != pInst && isa<Instruction>(pResult)) {
      cast<Instruction>(pResult)->copyMetadata(*pInst);
    }
  }

  return pResult;
}

//////////////////////////////////////////////////////////////////////////
/// @brief Replace all uses but avoid any type checking as instructions
///        maybe in a partial bad state.
/// @param pInst - old instruction we're replacing.
/// @param pNewInst - new instruction
void GenXPacketize::replaceAllUsesNoTypeCheck(Value *pInst, Value *pNewInst)
{
  SmallVector<User *, 8> users;
  SmallVector<uint32_t, 8> opNum;

  for (auto &U : pInst->uses()) {
    users.push_back(U.getUser());
    opNum.push_back(U.getOperandNo());
  }
  for (uint32_t i = 0; i < users.size(); ++i) {
    users[i]->setOperand(opNum[i], pNewInst);
  }
}

//////////////////////////////////////////////////////////////////////////
/// @brief Remove replaced instructions. DCE will not remove calls, etc.
///        So we have to remove these manually.
void GenXPacketize::removeDeadInstructions(Function &F) 
{
  for (auto RMI : ReplaceMap) {
    if (RMI.first != RMI.second) {
      if (Instruction* UnusedInst = (Instruction*)dyn_cast<Instruction>(RMI.first)) {
        UnusedInst->replaceAllUsesWith(UndefValue::get(UnusedInst->getType()));
        UnusedInst->eraseFromParent();
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////
/// @brief LLVM optimizes certain operations and replaces with general C
/// functions instead
///        of llvm intrinsics (sqrtf() instead of llvm.sqrt() for example). We
///        convert these back to known llvm intrinsics before packetization,
///        which are handled natively
/// @param F - function to analyze
void GenXPacketize::fixupLLVMIntrinsics(Function &F)
{
  std::unordered_set<Instruction *> removeSet;

  for (auto &BB : F.getBasicBlockList()) {
    for (auto &I : BB.getInstList()) {
      if (isa<CallInst>(I)) {
        CallInst *pCallInst = cast<CallInst>(&I);
        Function *pFunc = pCallInst->getCalledFunction();
        if (pFunc) {
          if (pFunc->getName().startswith("sqrt")) {
            B->IRB()->SetInsertPoint(&I);
            Value *pSqrt = B->VSQRTPS(pCallInst->getOperand(0));
            pCallInst->replaceAllUsesWith(pSqrt);
            removeSet.insert(pCallInst);
          } else if (pFunc->getName().startswith("fabs")) {
            B->IRB()->SetInsertPoint(&I);
            Value *pFabs = B->FABS(pCallInst->getOperand(0));
            pCallInst->replaceAllUsesWith(pFabs);
            removeSet.insert(pCallInst);
          } else if (pFunc->getName().startswith("exp2")) {
            B->IRB()->SetInsertPoint(&I);
            Value *pExp2 = B->EXP2(pCallInst->getOperand(0));
            pCallInst->replaceAllUsesWith(pExp2);
            removeSet.insert(pCallInst);
          } else if (pFunc->getName().equals("ldexpf")) {
            B->IRB()->SetInsertPoint(&I);
            Value *pArg = pCallInst->getOperand(0);
            Value *pExp = pCallInst->getOperand(1);

            // replace ldexp with arg * 2^exp = arg * (2 << arg)
            Value *pShift = B->SHL(B->C(1), pExp);
            pShift = B->UI_TO_FP(pShift, B->mFP32Ty);
            Value *pResult = B->FMUL(pArg, pShift);
            pCallInst->replaceAllUsesWith(pResult);
            removeSet.insert(pCallInst);
          }
        }
      }
    }
  }

  for (auto *pInst : removeSet) {
    pInst->eraseFromParent();
  }
}

//////////////////////////////////////////////////////////////////////////
/// @brief find the global ExecMask variable if exists in order to lower
/// CM SIMD control-flow representation after packetization
GlobalVariable *GenXPacketize::findGlobalExecMask() {
  // look for the global EMask variable if exists
  for (auto &Global : M->getGlobalList()) {
    auto Ty = Global.getType()->getElementType();
    if (Ty->isVectorTy() && Ty->getVectorNumElements() == CMSimdCFLower::MAX_SIMD_CF_WIDTH) {
      auto ElemTy = Ty->getVectorElementType();
      if (ElemTy->isIntegerTy() && ElemTy->getIntegerBitWidth() == 1) {
        // so far the type is right, then check the use
        for (auto EMUI = Global.use_begin(), EMUE = Global.use_end(); EMUI != EMUE; ++EMUI) {
          if (auto LD = dyn_cast<LoadInst>(EMUI->getUser())) {
            for (auto UI = LD->user_begin(), E = LD->user_end(); UI != E; ++UI) {
              const Value *LocalUse = (*UI);
              if (auto CI = dyn_cast_or_null<CallInst>(LocalUse)) {
                if (Function *Callee = CI->getCalledFunction()) {
                  if (Callee->getIntrinsicID() == Intrinsic::genx_simdcf_goto)
                    return &Global;
                }
              }
            }
          }
        }
      }
    }
  }
  return nullptr;
}
//////////////////////////////////////////////////////////////////////////
/// @brief lower CM SIMD control-flow representation after packetization
///
void GenXPacketize::lowerControlFlowAfter(std::vector<Function*> &SIMTFuncs) {
  auto EMVar = findGlobalExecMask();
  // create one if we cannot find one.
  if (!EMVar) {
    auto EMTy = VectorType::get(Type::getInt1Ty(M->getContext()),
      CMSimdCFLower::MAX_SIMD_CF_WIDTH);
    EMVar = new GlobalVariable(*M, EMTy, false/*isConstant*/,
      GlobalValue::InternalLinkage, Constant::getAllOnesValue(EMTy), "EM");
  }
  CMSimdCFLower CFL(EMVar);
  // Derive an order to process functions such that a function is visited
  // after anything that calls it.
  int n = SIMTFuncs.size();
  for (int i = n-1;  i >= 0; --i)
    CFL.processFunction(SIMTFuncs[i]);
}

// foward declare the initializer
void initializeGenXPacketizePass(PassRegistry &);

} // namespace llvm

using namespace llvm;

char GenXPacketize::ID = 0;
INITIALIZE_PASS_BEGIN(GenXPacketize, "GenXPacketize", "GenXPacketize", false, false)
INITIALIZE_PASS_DEPENDENCY(BreakCriticalEdges)
INITIALIZE_PASS_END(GenXPacketize, "GenXPacketize", "GenXPacketize", false, false)

namespace llvm {
  ModulePass *createGenXPacketizePass()
  {
    initializeGenXPacketizePass(*PassRegistry::getPassRegistry());
    return new GenXPacketize();
  }
}