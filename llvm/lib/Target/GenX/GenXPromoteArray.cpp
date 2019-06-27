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
/// GenXPromoteArray
/// --------------------
///
/// GenXPromotePredicate is an optimization pass that converts load/store
/// from an allocated private array into vector loads/stores followed by 
/// read-region and write-region.  Then we can apply standard llvm optimization
/// to promote the entire array into virtual registers, and remove those
/// loads and stores
//===----------------------------------------------------------------------===//

#include "GenX.h"
#include "GenXModule.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/Local.h"
#include <llvm/ADT/SmallVector.h>

#define MAX_ALLOCA_PROMOTE_GRF_NUM      48

using namespace llvm;
using namespace genx;

namespace {

  class TransposeHelper
  {
  public:
    TransposeHelper(bool vectorIndex) : m_vectorIndex(vectorIndex) {}
    void HandleAllocaSources(
      llvm::Instruction* v,
      llvm::Value* idx);
    void handleGEPInst(
      llvm::GetElementPtrInst *pGEP,
      llvm::Value* idx);
    virtual void handleLoadInst(
      llvm::LoadInst *pLoad,
      llvm::Value *pScalarizedIdx) = 0;
    virtual void handleStoreInst(
      llvm::StoreInst *pStore,
      llvm::Value *pScalarizedIdx) = 0;
    void handleLifetimeMark(llvm::IntrinsicInst *inst);
    void EraseDeadCode();
  private:
    bool m_vectorIndex;
    std::vector<llvm::Instruction*> m_toBeRemovedGEP;
  };

  /// @brief  TransformPrivMem pass is used for lowering the allocas identified while visiting the alloca instructions
  ///         and then inserting insert/extract elements instead of load stores. This allows us
  ///         to store the data in registers instead of propagating it to scratch space.
  class TransformPrivMem : public llvm::FunctionPass, public llvm::InstVisitor<TransformPrivMem>
  {
  public:
    TransformPrivMem();

    ~TransformPrivMem() {}

    virtual llvm::StringRef getPassName() const override
    {
      return "TransformPrivMem";
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override
    {
      AU.setPreservesCFG();
    }

    virtual bool runOnFunction(llvm::Function &F) override;

    void visitAllocaInst(llvm::AllocaInst &I);

    unsigned int extractAllocaSize(llvm::AllocaInst* pAlloca);

  private:
    llvm::AllocaInst* createVectorForAlloca(
      llvm::AllocaInst *pAlloca,
      llvm::Type *pBaseType);
    void handleAllocaInst(llvm::AllocaInst *pAlloca);

    bool CheckIfAllocaPromotable(llvm::AllocaInst* pAlloca);

  public:
    static char ID;

  private:
    const llvm::DataLayout                              *m_pDL;
    LLVMContext                                         *m_ctx;
    std::vector<llvm::AllocaInst*>                       m_allocasToPrivMem;
    llvm::Function                                      *m_pFunc;
  };


}

// Register pass to igc-opt
namespace llvm { void initializeTransformPrivMemPass(PassRegistry&); }
#define PASS_FLAG "transform-priv-mem"
#define PASS_DESCRIPTION "transform private arrays for promoting them to registers"
#define PASS_CFG_ONLY false
#define PASS_ANALYSIS false
INITIALIZE_PASS_BEGIN(TransformPrivMem, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)
INITIALIZE_PASS_END(TransformPrivMem, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)

char TransformPrivMem::ID = 0;

FunctionPass *llvm::createTransformPrivMemPass()
{
  return new TransformPrivMem();
}

TransformPrivMem::TransformPrivMem() : FunctionPass(ID), m_pFunc(nullptr)
{
  initializeTransformPrivMemPass(*PassRegistry::getPassRegistry());
}

llvm::AllocaInst* TransformPrivMem::createVectorForAlloca(
  llvm::AllocaInst* pAlloca,
  llvm::Type* pBaseType)
{
  IRBuilder<> IRB(pAlloca);

  unsigned int totalSize = extractAllocaSize(pAlloca) / (unsigned int)(m_pDL->getTypeAllocSize(pBaseType));

  llvm::VectorType* pVecType = llvm::VectorType::get(pBaseType, totalSize);

  AllocaInst *pAllocaValue = IRB.CreateAlloca(pVecType);
  return pAllocaValue;
}

bool TransformPrivMem::runOnFunction(llvm::Function &F)
{
  m_pFunc = &F;
  m_ctx = &(m_pFunc->getContext());

  m_pDL = &F.getParent()->getDataLayout();
  m_allocasToPrivMem.clear();

  visit(F);

  std::vector<llvm::AllocaInst*> &allocaToHande = m_allocasToPrivMem;
  for (auto pAlloca : allocaToHande)
  {
    handleAllocaInst(pAlloca);
  }

  // Last remove alloca instructions
  for (auto pInst : allocaToHande)
  {
    if (pInst->use_empty())
    {
      pInst->eraseFromParent();
    }
  }
  // IR changed only if we had alloca instruction to optimize
  return !allocaToHande.empty();
}

void TransposeHelper::EraseDeadCode()
{
  for (auto pInst = m_toBeRemovedGEP.rbegin(); pInst != m_toBeRemovedGEP.rend(); ++pInst)
  {
    assert((*pInst)->use_empty() && "Instruction still has usage");
    (*pInst)->eraseFromParent();
  }
}

unsigned int TransformPrivMem::extractAllocaSize(llvm::AllocaInst* pAlloca)
{
  unsigned int arraySize = (unsigned int)(cast<ConstantInt>(pAlloca->getArraySize())->getZExtValue());
  unsigned int totalArrayStructureSize = (unsigned int)(m_pDL->getTypeAllocSize(pAlloca->getAllocatedType()) * arraySize);

  return totalArrayStructureSize;
}

bool TransformPrivMem::CheckIfAllocaPromotable(llvm::AllocaInst* pAlloca)
{
  unsigned int allocaSize = extractAllocaSize(pAlloca);
  unsigned int allowedAllocaSizeInBytes = MAX_ALLOCA_PROMOTE_GRF_NUM * 32;

  // if alloca size exceeds alloc size threshold, return false
  if (allocaSize > allowedAllocaSizeInBytes)
  {
    return false;
  }
  return true;
}

static Type* GetBaseType(Type* pType)
{
  if (pType->isStructTy())
  {
    int num_elements = pType->getStructNumElements();
    if (num_elements > 1)
      return nullptr;

    pType = pType->getStructElementType(0);
  }

  while (pType->isArrayTy())
  {
    pType = pType->getArrayElementType();
  }

  if (pType->isStructTy())
  {
    int num_elements = pType->getStructNumElements();
    if (num_elements > 1)
      return nullptr;

    pType = pType->getStructElementType(0);
  }
  return pType;
}

void TransformPrivMem::visitAllocaInst(AllocaInst &I)
{
  // Alloca should always be private memory
  if (!CheckIfAllocaPromotable(&I))
  {
    // alloca size extends remain per-lane-reg space
    return;
  }
  m_allocasToPrivMem.push_back(&I);
}

void TransposeHelper::HandleAllocaSources(Instruction* v, Value* idx)
{
  SmallVector<Value*, 10> instructions;
  for (Value::user_iterator it = v->user_begin(), e = v->user_end(); it != e; ++it)
  {
    Value* inst = cast<Value>(*it);
    instructions.push_back(inst);
  }

  for (auto instruction : instructions)
  {
    if (GetElementPtrInst *pGEP = dyn_cast<GetElementPtrInst>(instruction))
    {
      handleGEPInst(pGEP, idx);
    }
    else if (BitCastInst* bitcast = dyn_cast<BitCastInst>(instruction))
    {
      m_toBeRemovedGEP.push_back(bitcast);
      HandleAllocaSources(bitcast, idx);
    }
    else if (StoreInst *pStore = llvm::dyn_cast<StoreInst>(instruction))
    {
      handleStoreInst(pStore, idx);
    }
    else if (LoadInst *pLoad = llvm::dyn_cast<LoadInst>(instruction))
    {
      handleLoadInst(pLoad, idx);
    }
    else if (IntrinsicInst* inst = dyn_cast<IntrinsicInst>(instruction))
    {
      handleLifetimeMark(inst);
    }
  }
}

class TransposeHelperPromote : public TransposeHelper
{
public:
  void handleLoadInst(
    LoadInst *pLoad,
    Value *pScalarizedIdx);
  void handleStoreInst(
    StoreInst *pStore,
    Value *pScalarizedIdx);
  AllocaInst *pVecAlloca;
  TransposeHelperPromote(AllocaInst* pAI) : TransposeHelper(false) { pVecAlloca = pAI; }
};

void TransformPrivMem::handleAllocaInst(llvm::AllocaInst* pAlloca)
{
  // Extract the Alloca size and the base Type
  Type* pType = pAlloca->getType()->getPointerElementType();
  assert(!pType->isStructTy() && "CM does not support struct-type");
  Type *pBaseType = GetBaseType(pType);
  if (!pBaseType)
    return;
  pBaseType = pBaseType->getScalarType();
  llvm::AllocaInst* pVecAlloca = createVectorForAlloca(pAlloca, pBaseType);
  if (!pVecAlloca)
    return;

  IRBuilder<> IRB(pVecAlloca);
  Value* idx = IRB.getInt32(0);
  TransposeHelperPromote helper(pVecAlloca);
  helper.HandleAllocaSources(pAlloca, idx);
  helper.EraseDeadCode();
}

void TransposeHelper::handleLifetimeMark(IntrinsicInst *inst)
{
  assert(inst->getIntrinsicID() == llvm::Intrinsic::lifetime_start ||
    inst->getIntrinsicID() == llvm::Intrinsic::lifetime_end);
  inst->eraseFromParent();
}

void TransposeHelper::handleGEPInst(
  llvm::GetElementPtrInst *pGEP,
  llvm::Value* idx)
{
  // Add GEP instruction to remove list
  m_toBeRemovedGEP.push_back(pGEP);
  if (pGEP->use_empty())
  {
    // GEP has no users, do nothing.
    return;
  }

  // Given %p = getelementptr [4 x [3 x <2 x float>]]* %v, i64 0, i64 %1, i64 %2
  // compute the scalarized index with an auxiliary array [4, 3, 2]:
  //
  // Formula: index = (%1 x 3 + %2) x 2
  //
  IRBuilder<> IRB(pGEP);
  Value *pScalarizedIdx = IRB.getInt32(0);
  Type* T = pGEP->getPointerOperandType()->getPointerElementType();
  for (unsigned i = 0, e = pGEP->getNumIndices(); i < e; ++i)
  {
    auto GepOpnd = IRB.CreateZExtOrTrunc(pGEP->getOperand(i + 1), IRB.getInt32Ty());
    unsigned int arr_sz = 1;
    if (T->isStructTy())
    {
      arr_sz = 1;
      T = T->getStructElementType(0);
    }
    else if (T->isArrayTy())
    {
      arr_sz = (unsigned int)(T->getArrayNumElements());
      T = T->getArrayElementType();
    }
    else if (T->isVectorTy())
    {
      // based on whether we want the index in number of element or number of vector
      if (m_vectorIndex)
      {
        arr_sz = 1;
      }
      else
      {
        arr_sz = T->getVectorNumElements();
      }
      T = T->getVectorElementType();
    }

    pScalarizedIdx = IRB.CreateNUWAdd(pScalarizedIdx, GepOpnd);
    pScalarizedIdx = IRB.CreateNUWMul(pScalarizedIdx, IRB.getInt32(arr_sz));
  }
  pScalarizedIdx = IRB.CreateNUWAdd(pScalarizedIdx, idx);
  HandleAllocaSources(pGEP, pScalarizedIdx);
}

// Load N elements from a vector alloca, Idx, ... Idx + N - 1. Return a scalar
// or a vector value depending on N.
static Value *loadEltsFromVecAlloca(
  unsigned N, AllocaInst *pVecAlloca,
  Value *pScalarizedIdx,
  IRBuilder<> &IRB,
  Type* scalarType)
{
  Value *pLoadVecAlloca = IRB.CreateLoad(pVecAlloca);
  if (N == 1)
  {
    return IRB.CreateBitCast(
      IRB.CreateExtractElement(pLoadVecAlloca, pScalarizedIdx),
      scalarType);
  }

  // A vector load
  // %v = load <2 x float>* %ptr
  // becomes
  // %w = load <32 x float>* %ptr1
  // %v0 = extractelement <32 x float> %w, i32 %idx
  // %v1 = extractelement <32 x float> %w, i32 %idx+1
  // replace all uses of %v with <%v0, %v1>
  assert(N > 1 && "out of sync");
  Type* Ty = VectorType::get(scalarType, N);
  Value *Result = UndefValue::get(Ty);

  for (unsigned i = 0; i < N; ++i)
  {
    Value *VectorIdx = ConstantInt::get(pScalarizedIdx->getType(), i);
    auto Idx = IRB.CreateAdd(pScalarizedIdx, VectorIdx);
    auto Val = IRB.CreateExtractElement(pLoadVecAlloca, Idx);
    Val = IRB.CreateBitCast(Val, scalarType);
    Result = IRB.CreateInsertElement(Result, Val, VectorIdx);
  }
  return Result;
}

void TransposeHelperPromote::handleLoadInst(
  LoadInst *pLoad,
  Value *pScalarizedIdx)
{
  assert(pLoad->isSimple());
  IRBuilder<> IRB(pLoad);
  unsigned N = pLoad->getType()->isVectorTy()
    ? pLoad->getType()->getVectorNumElements()
    : 1;
  Value *Val = loadEltsFromVecAlloca(N, pVecAlloca, pScalarizedIdx, IRB, pLoad->getType()->getScalarType());
  pLoad->replaceAllUsesWith(Val);
  pLoad->eraseFromParent();
}

void TransposeHelperPromote::handleStoreInst(
  llvm::StoreInst *pStore,
  llvm::Value *pScalarizedIdx)
{
  // Add Store instruction to remove list
  assert(pStore->isSimple());

  IRBuilder<> IRB(pStore);
  llvm::Value* pStoreVal = pStore->getValueOperand();
  llvm::Value* pLoadVecAlloca = IRB.CreateLoad(pVecAlloca);
  llvm::Value* pIns = pLoadVecAlloca;
  if (pStoreVal->getType()->isVectorTy())
  {
    // A vector store
    // store <2 x float> %v, <2 x float>* %ptr
    // becomes
    // %w = load <32 x float> *%ptr1
    // %v0 = extractelement <2 x float> %v, i32 0
    // %w0 = insertelement <32 x float> %w, float %v0, i32 %idx
    // %v1 = extractelement <2 x float> %v, i32 1
    // %w1 = insertelement <32 x float> %w0, float %v1, i32 %idx+1
    // store <32 x float> %w1, <32 x float>* %ptr1
    for (unsigned i = 0, e = pStoreVal->getType()->getVectorNumElements(); i < e; ++i)
    {
      Value *VectorIdx = ConstantInt::get(pScalarizedIdx->getType(), i);
      auto Val = IRB.CreateExtractElement(pStoreVal, VectorIdx);
      Val = IRB.CreateBitCast(Val, pLoadVecAlloca->getType()->getScalarType());
      auto Idx = IRB.CreateAdd(pScalarizedIdx, VectorIdx);
      pIns = IRB.CreateInsertElement(pIns, Val, Idx);
    }
  }
  else
  {
    pStoreVal = IRB.CreateBitCast(pStoreVal, pLoadVecAlloca->getType()->getScalarType());
    pIns = IRB.CreateInsertElement(pLoadVecAlloca, pStoreVal, pScalarizedIdx);
  }
  IRB.CreateStore(pIns, pVecAlloca);
  pStore->eraseFromParent();
}
