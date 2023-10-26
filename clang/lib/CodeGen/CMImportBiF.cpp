/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This pass import Builtin Function library compiled into bitcode
// - analysis functions called by the main module
// - import used function, and remove unused functions

#define DEBUG_TYPE "cmimportbif"

#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/Error.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/ValueMapper.h>

#include <vector>

#include "llvm/GenXIntrinsics/GenXIntrinsicInst.h"

using namespace llvm;

class BIConvert {
  // builtins that maps to one intrinsic
  std::map<StringRef, unsigned> OneMap;
  // builtins that maps to two intrinsics
  std::map<StringRef, std::pair<unsigned, unsigned>> TwoMap;

public:
  BIConvert();
  void runOnModule(Module &M);
};

BIConvert::BIConvert() {
  // float-to-float
  OneMap["__builtin_IB_frnd_ne"] = GenXIntrinsic::genx_rnde;
  OneMap["__builtin_IB_ftoh_rtn"] = GenXIntrinsic::genx_rndd;
  OneMap["__builtin_IB_ftoh_rtp"] = GenXIntrinsic::genx_rndu;
  OneMap["__builtin_IB_ftoh_rtz"] = GenXIntrinsic::genx_rndz;
  OneMap["__builtin_IB_dtoh_rtn"] = GenXIntrinsic::genx_rnde;
  OneMap["__builtin_IB_dtoh_rtp"] = GenXIntrinsic::genx_rndu;
  OneMap["__builtin_IB_dtoh_rtz"] = GenXIntrinsic::genx_rndz;
  OneMap["__builtin_IB_dtof_rtn"] = GenXIntrinsic::genx_rnde;
  OneMap["__builtin_IB_dtof_rtp"] = GenXIntrinsic::genx_rndu;
  OneMap["__builtin_IB_dtof_rtz"] = GenXIntrinsic::genx_rndz;
  // math
  OneMap["__builtin_IB_frnd_pi"] = GenXIntrinsic::genx_rndu;
  OneMap["__builtin_IB_frnd_ni"] = GenXIntrinsic::genx_rndd;
  OneMap["__builtin_IB_frnd_zi"] = GenXIntrinsic::genx_rndz;
  OneMap["__builtin_IB_native_cosf"] = GenXIntrinsic::genx_cos;
  OneMap["__builtin_IB_native_cosh"] = GenXIntrinsic::genx_cos;
  OneMap["__builtin_IB_native_sinf"] = GenXIntrinsic::genx_sin;
  OneMap["__builtin_IB_native_sinh"] = GenXIntrinsic::genx_sin;
  OneMap["__builtin_IB_native_exp2f"] = GenXIntrinsic::genx_exp;
  OneMap["__builtin_IB_native_exp2h"] = GenXIntrinsic::genx_exp;
  OneMap["__builtin_IB_native_log2f"] = GenXIntrinsic::genx_log;
  OneMap["__builtin_IB_native_log2h"] = GenXIntrinsic::genx_log;
  OneMap["__builtin_IB_native_sqrtf"] = GenXIntrinsic::genx_sqrt;
  OneMap["__builtin_IB_native_sqrth"] = GenXIntrinsic::genx_sqrt;
  OneMap["__builtin_IB_native_sqrtd"] = GenXIntrinsic::genx_sqrt;
  OneMap["__builtin_IB_popcount_1u32"] = GenXIntrinsic::genx_cbit;
  OneMap["__builtin_IB_popcount_1u16"] = GenXIntrinsic::genx_cbit;
  OneMap["__builtin_IB_popcount_1u8"] = GenXIntrinsic::genx_cbit;
  OneMap["__builtin_IB_native_powrf"] = GenXIntrinsic::genx_pow;
  OneMap["__builtin_IB_fma"] = Intrinsic::fma;
  OneMap["__builtin_IB_fmah"] = Intrinsic::fma;
  OneMap["__builtin_IB_bfrev"] = GenXIntrinsic::genx_bfrev;
  OneMap["__builtin_IB_fmax"] = GenXIntrinsic::genx_fmax;
  OneMap["__builtin_IB_fmin"] = GenXIntrinsic::genx_fmin;
  OneMap["__builtin_IB_HMAX"] = GenXIntrinsic::genx_fmax;
  OneMap["__builtin_IB_HMIN"] = GenXIntrinsic::genx_fmin;
  OneMap["__builtin_IB_dmin"] = GenXIntrinsic::genx_fmin;
  OneMap["__builtin_IB_dmax"] = GenXIntrinsic::genx_fmax;
  // ieee
  OneMap["__builtin_IB_ieee_sqrt"] = GenXIntrinsic::genx_ieee_sqrt;
  OneMap["__builtin_IB_ieee_divide"] = GenXIntrinsic::genx_ieee_div;
  OneMap["__builtin_IB_ieee_divide_f64"] = GenXIntrinsic::genx_ieee_div;

  TwoMap["__builtin_IB_dtoi8_rtn"] =
      std::make_pair(GenXIntrinsic::genx_rndd, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi8_rtp"] =
      std::make_pair(GenXIntrinsic::genx_rndu, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi8_rte"] =
      std::make_pair(GenXIntrinsic::genx_rnde, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi16_rtn"] =
      std::make_pair(GenXIntrinsic::genx_rndd, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi16_rtp"] =
      std::make_pair(GenXIntrinsic::genx_rndu, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi16_rte"] =
      std::make_pair(GenXIntrinsic::genx_rnde, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi32_rtn"] =
      std::make_pair(GenXIntrinsic::genx_rndd, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi32_rtp"] =
      std::make_pair(GenXIntrinsic::genx_rndu, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi32_rte"] =
      std::make_pair(GenXIntrinsic::genx_rnde, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi64_rtn"] =
      std::make_pair(GenXIntrinsic::genx_rndd, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi64_rtp"] =
      std::make_pair(GenXIntrinsic::genx_rndu, GenXIntrinsic::genx_fptosi_sat);
  TwoMap["__builtin_IB_dtoi64_rte"] =
      std::make_pair(GenXIntrinsic::genx_rnde, GenXIntrinsic::genx_fptosi_sat);

  TwoMap["__builtin_IB_dtoui8_rtn"] =
      std::make_pair(GenXIntrinsic::genx_rndd, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui8_rtp"] =
      std::make_pair(GenXIntrinsic::genx_rndu, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui8_rte"] =
      std::make_pair(GenXIntrinsic::genx_rnde, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui16_rtn"] =
      std::make_pair(GenXIntrinsic::genx_rndd, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui16_rtp"] =
      std::make_pair(GenXIntrinsic::genx_rndu, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui16_rte"] =
      std::make_pair(GenXIntrinsic::genx_rnde, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui32_rtn"] =
      std::make_pair(GenXIntrinsic::genx_rndd, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui32_rtp"] =
      std::make_pair(GenXIntrinsic::genx_rndu, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui32_rte"] =
      std::make_pair(GenXIntrinsic::genx_rnde, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui64_rtn"] =
      std::make_pair(GenXIntrinsic::genx_rndd, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui64_rtp"] =
      std::make_pair(GenXIntrinsic::genx_rndu, GenXIntrinsic::genx_fptoui_sat);
  TwoMap["__builtin_IB_dtoui64_rte"] =
      std::make_pair(GenXIntrinsic::genx_rnde, GenXIntrinsic::genx_fptoui_sat);

  TwoMap["__builtin_IB_fma_rtz_f64"] =
      std::make_pair(Intrinsic::fma, GenXIntrinsic::genx_rndz);
  TwoMap["__builtin_IB_fma_rtz_f32"] =
      std::make_pair(Intrinsic::fma, GenXIntrinsic::genx_rndz);
}

void BIConvert::runOnModule(Module &M) {
  std::vector<Instruction *> ListDelete;
  for (Function &func : M) {
    // a hack to avoid extractvalue in backend
    if (func.getName() == "__clc_exp_helper")
      func.addFnAttr(Attribute::AlwaysInline);
    for (auto &BB : func) {
      for (auto I = BB.begin(), E = BB.end(); I != E; I++) {
        CallInst *InstCall = dyn_cast<CallInst>(I);
        if (!InstCall)
          continue;
        Function *callee = InstCall->getCalledFunction();
        if (!callee)
          continue;
        // get rid of lifetime marker, avoid dealing with it in packetizer
        Intrinsic::ID id = (Intrinsic::ID)callee->getIntrinsicID();
        if (id == Intrinsic::lifetime_start || id == Intrinsic::lifetime_end) {
          ListDelete.push_back(InstCall);
          continue;
        }
        else if (id == Intrinsic::ctlz) {
          // convert this to genx_ldz, but genx_lzd only support 32-bit input
          auto Src = InstCall->getOperand(0);
          auto SrcTy = Src->getType();
          assert(SrcTy->isIntegerTy());
          assert(SrcTy->getPrimitiveSizeInBits() == 32);
          Type *tys[1];
          SmallVector<llvm::Value *, 1> args;
          // build type-list for the 1st intrinsic
          tys[0] = SrcTy;
          // build argument list for the 1st intrinsic
          args.push_back(Src);
          Function *IntrinFunc = GenXIntrinsic::getAnyDeclaration(&M, GenXIntrinsic::genx_lzd, tys);
          Instruction *IntrinCall = CallInst::Create(
            IntrinFunc, args, InstCall->getName(), InstCall);
          IntrinCall->setDebugLoc(InstCall->getDebugLoc());
          InstCall->replaceAllUsesWith(IntrinCall);
          ListDelete.push_back(InstCall);
          continue;
        }

        StringRef CalleeName = callee->getName();
        // Check if it exists in the one-intrinsic map.
        if (OneMap.count(CalleeName)) {
          Type *tys[1];
          SmallVector<llvm::Value *, 3> args;
          unsigned IID = OneMap[CalleeName];
          // build type-list
          tys[0] = callee->getReturnType();
          // tys[1] = InstCall->getArgOperand(0)->getType();
          // build argument list
          args.append(InstCall->op_begin(),
                      InstCall->op_begin() + InstCall->getNumArgOperands());
          Function *IntrinFunc = GenXIntrinsic::getAnyDeclaration(&M, IID, tys);
          Instruction *IntrinCall =
              CallInst::Create(IntrinFunc, args, InstCall->getName(), InstCall);
          IntrinCall->setDebugLoc(InstCall->getDebugLoc());
          InstCall->replaceAllUsesWith(IntrinCall);
          ListDelete.push_back(InstCall);
        }
        // check if the builtin maps to two intrinsics
        else if (TwoMap.count(CalleeName)) {
          auto pair = TwoMap[CalleeName];
          // create the 1st intrinsic
          Type *tys0[1];
          SmallVector<llvm::Value *, 3> args0;
          // build type-list for the 1st intrinsic
          tys0[0] = InstCall->getArgOperand(0)->getType();
          // build argument list for the 1st intrinsic
          args0.append(InstCall->op_begin(),
                       InstCall->op_begin() + InstCall->getNumArgOperands());
          Function *IntrinFunc0 =
              GenXIntrinsic::getAnyDeclaration(&M, pair.first, tys0);
          Instruction *IntrinCall0 = CallInst::Create(
              IntrinFunc0, args0, InstCall->getName(), InstCall);
          IntrinCall0->setDebugLoc(InstCall->getDebugLoc());
          // create the 2nd intrinsic
          Type *tys1[2];
          SmallVector<llvm::Value *, 3> args1;
          // build type-list for the 1st intrinsic
          tys1[0] = callee->getReturnType();
          tys1[1] = IntrinCall0->getType();
          // build argument list for the 1st intrinsic
          args1.push_back(IntrinCall0);
          Function *IntrinFunc1 =
              GenXIntrinsic::getAnyDeclaration(&M, pair.second, tys1);
          Instruction *IntrinCall1 = CallInst::Create(
              IntrinFunc1, args1, InstCall->getName(), InstCall);
          IntrinCall1->setDebugLoc(InstCall->getDebugLoc());
          InstCall->replaceAllUsesWith(IntrinCall1);
          ListDelete.push_back(InstCall);
        }
        // other cases
        else if (CalleeName.startswith("__builtin_IB_itof")) {
          Instruction *Replace = SIToFPInst::Create(
              Instruction::SIToFP, InstCall->getArgOperand(0),
              callee->getReturnType(), InstCall->getName(), InstCall);
          Replace->setDebugLoc(InstCall->getDebugLoc());
          InstCall->replaceAllUsesWith(Replace);
          ListDelete.push_back(InstCall);
        } else if (CalleeName.startswith("__builtin_IB_uitof")) {
          Instruction *Replace = UIToFPInst::Create(
              Instruction::UIToFP, InstCall->getArgOperand(0),
              callee->getReturnType(), InstCall->getName(), InstCall);
          Replace->setDebugLoc(InstCall->getDebugLoc());
          InstCall->replaceAllUsesWith(Replace);
          ListDelete.push_back(InstCall);
        } else if (CalleeName.startswith("__builtin_IB_mul_rtz")) {
          Instruction *Mul = BinaryOperator::Create(
              Instruction::FMul, InstCall->getArgOperand(0),
              InstCall->getArgOperand(1), InstCall->getName(), InstCall);
          Mul->setDebugLoc(InstCall->getDebugLoc());
          Type *tys[1];
          SmallVector<llvm::Value *, 3> args;
          // build type-list for the 1st intrinsic
          tys[0] = InstCall->getArgOperand(0)->getType();
          // build argument list for the 1st intrinsic
          args.push_back(Mul);
          Function *IntrinFunc =
              GenXIntrinsic::getAnyDeclaration(&M, GenXIntrinsic::genx_rndz, tys);
          Instruction *IntrinCall =
              CallInst::Create(IntrinFunc, args, InstCall->getName(), InstCall);
          IntrinCall->setDebugLoc(InstCall->getDebugLoc());
          InstCall->replaceAllUsesWith(IntrinCall);
          ListDelete.push_back(InstCall);
        } else if (CalleeName.startswith("__builtin_IB_add_rtz")) {
          Instruction *Add = BinaryOperator::Create(
              Instruction::FAdd, InstCall->getArgOperand(0),
              InstCall->getArgOperand(1), InstCall->getName(), InstCall);
          Add->setDebugLoc(InstCall->getDebugLoc());
          Type *tys[1];
          SmallVector<llvm::Value *, 3> args;
          // build type-list for the 1st intrinsic
          tys[0] = InstCall->getArgOperand(0)->getType();
          // build argument list for the 1st intrinsic
          args.push_back(Add);
          Function *IntrinFunc =
              GenXIntrinsic::getAnyDeclaration(&M, GenXIntrinsic::genx_rndz, tys);
          Instruction *IntrinCall =
              CallInst::Create(IntrinFunc, args, InstCall->getName(), InstCall);
          IntrinCall->setDebugLoc(InstCall->getDebugLoc());
          InstCall->replaceAllUsesWith(IntrinCall);
          ListDelete.push_back(InstCall);
        }
      }
    }
  }
  // clean up the dead calls
  for (auto I : ListDelete) {
    I->eraseFromParent();
  }

  for (auto &Global : M.getGlobalList()) {
    if (!Global.isDeclaration())
      Global.setLinkage(GlobalValue::InternalLinkage);
  }
  for (auto &F : M.getFunctionList()) {
    if (F.getIntrinsicID() == Intrinsic::not_intrinsic && !F.isDeclaration() &&
        !F.hasDLLExportStorageClass())
      F.setLinkage(GlobalValue::InternalLinkage);
  }
}

typedef std::vector<llvm::Function *> TFunctionsVec;

static Function *GetBuiltinFunction(llvm::StringRef funcName,
                                    llvm::Module *BiFModule) {
  Function *pFunc = nullptr;
  if ((pFunc = BiFModule->getFunction(funcName)) && !pFunc->isDeclaration())
    return pFunc;
  return nullptr;
}

static bool materialized_use_empty(const Value *v) {
  return v->materialized_use_begin() == v->use_end();
}

static void GetCalledFunctions(Function *pFunc, TFunctionsVec &calledFuncs) {
  SmallPtrSet<Function *, 8> visitedSet;
  // Iterate over function instructions and look for call instructions
  for (auto it = inst_begin(pFunc), e = inst_end(pFunc); it != e; ++it) {
    CallInst *pInstCall = dyn_cast<CallInst>(&*it);
    if (!pInstCall)
      continue;
    pInstCall->setCallingConv(pFunc->getCallingConv());
    Function *pCalledFunc = pInstCall->getCalledFunction();
    if (!pCalledFunc) {
      // This case can occur only if CallInst is calling something other than
      // LLVM function. Thus, no need to handle this case - function casting is
      // not allowed (and not expected!)
      continue;
    }
    if (visitedSet.count(pCalledFunc))
      continue;
    visitedSet.insert(pCalledFunc);
    calledFuncs.push_back(pCalledFunc);
  }
}

static void removeFunctionBitcasts(llvm::Module &M) {
  std::vector<Instruction *> list_delete;
  DenseMap<Function *, std::vector<Function *>> bitcastFunctionMap;

  for (Function &func : M) {
    for (auto &BB : func) {
      for (auto I = BB.begin(), E = BB.end(); I != E; I++) {
        CallInst *pInstCall = dyn_cast<CallInst>(I);
        if (!pInstCall || pInstCall->getCalledFunction())
          continue;
        if (auto constExpr =
                dyn_cast<llvm::ConstantExpr>(pInstCall->getCalledOperand())) {
          if (auto funcTobeChanged =
                  dyn_cast<llvm::Function>(constExpr->stripPointerCasts())) {
            if (funcTobeChanged->isDeclaration())
              continue;
            // Map between values (functions) in source of bitcast
            // to their counterpart values in destination
            llvm::ValueToValueMapTy operandMap;
            Function *pDstFunc = nullptr;
            auto BCFMI = bitcastFunctionMap.find(funcTobeChanged);
            bool notExists = BCFMI == bitcastFunctionMap.end();
            if (!notExists) {
              auto funcVec = bitcastFunctionMap[funcTobeChanged];
              notExists = true;
              for (Function *F : funcVec) {
                if (pInstCall->getFunctionType() == F->getFunctionType()) {
                  notExists = false;
                  pDstFunc = F;
                  break;
                }
              }
            }

            if (notExists) {
              pDstFunc = Function::Create(pInstCall->getFunctionType(),
                                          funcTobeChanged->getLinkage(),
                                          funcTobeChanged->getName(), &M);
              if (pDstFunc->arg_size() != funcTobeChanged->arg_size())
                continue;
              // Need to copy the attributes over too.
              auto FuncAttrs = funcTobeChanged->getAttributes();
              pDstFunc->setAttributes(FuncAttrs);

              // Go through and convert function arguments over, remembering the
              // mapping.
              Function::arg_iterator itSrcFunc = funcTobeChanged->arg_begin();
              Function::arg_iterator eSrcFunc = funcTobeChanged->arg_end();
              llvm::Function::arg_iterator itDest = pDstFunc->arg_begin();

              for (; itSrcFunc != eSrcFunc; ++itSrcFunc, ++itDest) {
                itDest->setName(itSrcFunc->getName());
                operandMap[&(*itSrcFunc)] = &(*itDest);
              }

              // Clone the body of the function into the dest function.
              SmallVector<ReturnInst *, 8> Returns; // Ignore returns.
              CloneFunctionInto(pDstFunc, funcTobeChanged, operandMap, false,
                                Returns, "");

              pDstFunc->setCallingConv(funcTobeChanged->getCallingConv());
              bitcastFunctionMap[funcTobeChanged].push_back(pDstFunc);
            }

            std::vector<Value *> Args;
            for (unsigned I = 0, E = pInstCall->getNumArgOperands(); I != E;
                 ++I) {
              Args.push_back(pInstCall->getArgOperand(I));
            }
            auto newCI = CallInst::Create(pDstFunc, Args, "", pInstCall);
            newCI->takeName(pInstCall);
            newCI->setCallingConv(pInstCall->getCallingConv());
            pInstCall->replaceAllUsesWith(newCI);
            pInstCall->dropAllReferences();
            if (constExpr->use_empty())
              constExpr->dropAllReferences();
            if (funcTobeChanged->use_empty())
              funcTobeChanged->eraseFromParent();

            list_delete.push_back(pInstCall);
          }
        }
      }
    }
  }

  for (auto i : list_delete) {
    i->eraseFromParent();
  }
}

static void InitializeBIFlags(llvm::Module &M) {
  /// @brief Adds initialization to a global-var according to given value.
  ///        If the given global-var does not exist, does nothing.
  auto initializeVarWithValue = [&M](StringRef varName, uint32_t value) {
    GlobalVariable *gv = M.getGlobalVariable(varName);
    if (gv == nullptr)
      return;
    gv->setInitializer(
        ConstantInt::get(Type::getInt32Ty(M.getContext()), value));
  };

  initializeVarWithValue("__FlushDenormals", 1);
  initializeVarWithValue("__DashGSpecified", 0);
  initializeVarWithValue("__FastRelaxedMath", 0);
  initializeVarWithValue("__UseNative64BitSubgroupBuiltin", 1);
  initializeVarWithValue("__CRMacros", 1);

  initializeVarWithValue("__IsSPIRV", 0);

  initializeVarWithValue("__EnableSWSrgbWrites", 0);

  float profilingTimerResolution = 0.0;
  initializeVarWithValue("__ProfilingTimerResolution",
                         *reinterpret_cast<int *>(&profilingTimerResolution));
}

bool CMImportBiF(llvm::Module *MainModule,
                 std::unique_ptr<llvm::Module> BiFModule) {
  std::function<void(Function *)> Explore = [&](Function *pRoot) -> void {
    TFunctionsVec calledFuncs;
    GetCalledFunctions(pRoot, calledFuncs);

    for (auto *pCallee : calledFuncs) {
      Function *pFunc = nullptr;
      if (pCallee->isDeclaration()) {
        auto funcName = pCallee->getName();
        Function *pSrcFunc = GetBuiltinFunction(funcName, BiFModule.get());
        if (!pSrcFunc)
          continue;
        pFunc = pSrcFunc;
      } else {
        pFunc = pCallee;
      }

      if (pFunc->isMaterializable()) {
        if (Error Err = pFunc->materialize()) {
          std::string Msg;
          handleAllErrors(std::move(Err), [&](ErrorInfoBase &EIB) {
            errs() << "===> Materialize Failure: " << EIB.message().c_str()
                   << '\n';
          });
          assert(0 && "Failed to materialize Global Variables");
        } else {
          pFunc->setCallingConv(pRoot->getCallingConv());
          Explore(pFunc);
        }
      }
    }
  };

  for (auto &func : *MainModule) {
    Explore(&func);
  }

  // nuke the unused functions so we can materializeAll() quickly
  auto CleanUnused = [](llvm::Module *Module) {
    for (auto I = Module->begin(), E = Module->end(); I != E;) {
      auto *F = &(*I++);
      if (F->isDeclaration() || F->isMaterializable()) {
        if (materialized_use_empty(F)) {
          F->eraseFromParent();
        }
      }
    }
  };

  CleanUnused(BiFModule.get());
  Linker ld(*MainModule);

  if (Error err = BiFModule->materializeAll()) {
    assert(0 && "materializeAll failed for generic builtin module");
  }

  if (ld.linkInModule(std::move(BiFModule))) {
    assert(0 && "Error linking generic builtin module");
  }

  InitializeBIFlags(*MainModule);
  removeFunctionBitcasts(*MainModule);

  std::vector<Instruction *> InstToRemove;

  for (auto I : InstToRemove) {
    I->eraseFromParent();
  }

  // create converter
  BIConvert CVT;
  CVT.runOnModule(*MainModule);
  return true;
}
