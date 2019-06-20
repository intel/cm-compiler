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
// This file defines the GenX specific subclass of TargetMachine.
//
/// Non-pass classes
/// ================
/// 
/// This section documents some GenX backend classes and abstractions that are not
/// in themselves passes, but are used by the passes.
///
/// .. include:: GenXAlignmentInfo.h
///
/// .. include:: GenXRegion.h
///
/// .. include:: GenXSubtarget.h
///
/// Pass documentation
/// ==================
/// 
/// The GenX backend runs the following passes on LLVM IR:
/// 
/// .. contents::
///    :local:
///    :depth: 1
/// 
//
//===----------------------------------------------------------------------===//

#include "FunctionGroup.h"
#include "GenX.h"
#include "GenXTargetMachine.h"
#include "GenXModule.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

static cl::opt<bool> DumpRegAlloc("genx-dump-regalloc", cl::init(false), cl::Hidden,
                  cl::desc("Enable dumping of GenX liveness and register allocation to a file."));

// There's another copy of DL string in clang/lib/Basic/Targets.cpp
static std::string getDL(bool Is64Bit) {
  return Is64Bit ? "e-p:64:64-i64:64-n8:16:32" : "e-p:32:32-i64:64-n8:16:32";
}

GenXTargetMachine::GenXTargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     Optional<Reloc::Model> RM,
                                     Optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool Is64Bit)
    : TargetMachine(T, getDL(Is64Bit), TT, CPU, FS, Options), Is64Bit(Is64Bit),
      Subtarget(TT, CPU, FS) {}

GenXTargetMachine::~GenXTargetMachine() = default;

void GenXTargetMachine32::anchor() {}

GenXTargetMachine32::GenXTargetMachine32(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         Optional<Reloc::Model> RM,
                                         Optional<CodeModel::Model> CM,
                                         CodeGenOpt::Level OL, bool JIT)
    : GenXTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, false) {}

void GenXTargetMachine64::anchor() {}

GenXTargetMachine64::GenXTargetMachine64(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         Optional<Reloc::Model> RM,
                                         Optional<CodeModel::Model> CM,
                                         CodeGenOpt::Level OL, bool JIT)
    : GenXTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, true) {}

//===----------------------------------------------------------------------===//
//                       External Interface declaration
//===----------------------------------------------------------------------===//
extern "C" void LLVMInitializeGenXTarget() {
  // Register the target.
  RegisterTargetMachine<GenXTargetMachine32> X(getTheGenXTarget32());
  RegisterTargetMachine<GenXTargetMachine64> Y(getTheGenXTarget64());
}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//

bool GenXTargetMachine::addPassesToEmitFile(PassManagerBase &PM,
                                            raw_pwrite_stream &o,
                                            CodeGenFileType FileType,
                                            bool DisableVerify,
                                            MachineModuleInfo *) {
  // We can consider the .isa file to be an object file, or an assembly file
  // which may later be converted to GenX code by the Finalizer. If we're
  // asked to produce any other type of file return true to indicate an error.
  if ((FileType != TargetMachine::CGFT_ObjectFile) &&
      (FileType != TargetMachine::CGFT_AssemblyFile))
    return true;
  // GenXSubtargetPass is a wrapper pass to query features or options.
  // This adds it explicitly to allow passes access the subtarget object using
  // method getAnalysisIfAvailable.
  PM.add(createGenXSubtargetPass(Subtarget));

  PM.add(createTransformPrivMemPass());
  PM.add(createPromoteMemoryToRegisterPass());
    // All passes which modify the LLVM IR are now complete; run the verifier
  // to ensure that the IR is valid.
  if (!DisableVerify)
    PM.add(createVerifierPass());
  // Run passes to generate vISA.

  /// BasicAliasAnalysis
  /// ------------------
  /// This is a standard LLVM analysis pass to provide basic AliasAnalysis
  /// support.
  PM.add(createBasicAAWrapperPass());
  /// SROA
  /// ----
  /// This is a standard LLVM pass, used at this point in the GenX backend.
  /// Normally all alloca variables have been
  /// removed by now by earlier LLVM passes, unless ``-O0`` was specified.
  /// We run this pass here to cover that case.
  ///
  /// **IR restriction**: alloca, load, store not supported after this pass.
  ///
  PM.add(createSROAPass());
  /// LowerSwitch
  /// -----------
  /// This is a standard LLVM pass to lower a switch instruction to a chain of
  /// conditional branches.
  ///
  /// **IR restriction**: switch not supported after this pass.
  ///
  // TODO: keep some switch instructions and lower them to JMPSWITCH vISA ops.
  PM.add(createLowerSwitchPass());
  /// .. include:: GenXCFSimplification.cpp
  PM.add(createGenXCFSimplificationPass());
  /// CFGSimplification
  /// -----------------
  /// This is a standard LLVM pass, used at this point in the GenX backend.
  ///
  PM.add(createCFGSimplificationPass());
  /// .. include:: GenXGEPLowering.cpp
  PM.add(createGenXGEPLoweringPass());
  /// .. include:: GenXReduceIntSize.cpp
  PM.add(createGenXReduceIntSizePass());
  /// InstructionCombining
  /// --------------------
  /// This is a standard LLVM pass, used at this point in the GenX backend.
  ///
  PM.add(createInstructionCombiningPass());
  // Run integer reduction again to revert some trunc/ext patterns transformed
  // by instcombine.
  PM.add(createGenXReduceIntSizePass());
  /// .. include:: GenXSimdCFConformance.cpp
  PM.add(createGenXEarlySimdCFConformancePass());
  /// .. include:: GenXPromotePredicate.cpp
  PM.add(createGenXPromotePredicatePass());
  // Run GEP lowering again to remove possible GEPs after instcombine.
  PM.add(createGenXGEPLoweringPass());
  /// .. include:: GenXLowering.cpp
  PM.add(createGenXLoweringPass());
  if (!DisableVerify) PM.add(createVerifierPass());
  /// .. include:: GenXRegionCollapsing.cpp
  PM.add(createGenXRegionCollapsingPass());
  /// EarlyCSE
  /// --------
  /// This is a standard LLVM pass, run at this point in the GenX backend.
  /// It commons up common subexpressions, but only in the case that two common
  /// subexpressions are related by one dominating the other.
  ///
  PM.add(createEarlyCSEPass());
  /// .. include:: GenXPatternMatch.cpp
  PM.add(createGenXPatternMatchPass(&Options));
  if (!DisableVerify) PM.add(createVerifierPass());
  /// .. include:: GenXExtractVectorizer.cpp
  PM.add(createGenXExtractVectorizerPass());
  /// .. include:: GenXRawSendRipper.cpp
  PM.add(createGenXRawSendRipperPass());
  /// DeadCodeElimination
  /// -------------------
  /// This is a standard LLVM pass, run at this point in the GenX backend. It
  /// removes code that has been made dead by other passes.
  ///
  PM.add(createDeadCodeEliminationPass());
  /// .. include:: GenXBaling.h
  PM.add(createGenXFuncBalingPass(BalingKind::BK_Legalization, &Subtarget));
  /// .. include:: GenXLegalization.cpp
  PM.add(createGenXLegalizationPass());
  /// .. include:: GenXEmulate.cpp
  PM.add(createGenXEmulatePass());
  /// .. include:: GenXDeadVectorRemoval.cpp
  PM.add(createGenXDeadVectorRemovalPass());
  /// DeadCodeElimination
  /// -------------------
  /// This is a standard LLVM pass, run at this point in the GenX backend. It
  /// removes code that has been made dead by other passes.
  ///
  PM.add(createDeadCodeEliminationPass());
  /// .. include:: GenXPostLegalization.cpp
  /// .. include:: GenXConstants.cpp
  /// .. include:: GenXVectorDecomposer.h
  PM.add(createGenXPostLegalizationPass());
  if (!DisableVerify) PM.add(createVerifierPass());
  /// EarlyCSE
  /// --------
  /// This is a standard LLVM pass, run at this point in the GenX backend.
  /// It commons up common subexpressions, but only in the case that two common
  /// subexpressions are related by one dominating the other.
  ///
  PM.add(createEarlyCSEPass());
  /// LICM
  /// ----
  /// This is a standard LLVM pass to hoist/sink the loop invariant code after
  /// legalization.
  PM.add(createLICMPass());
  /// DeadCodeElimination
  /// -------------------
  /// This is a standard LLVM pass, run at this point in the GenX backend. It
  /// removes code that has been made dead by other passes.
  ///
  PM.add(createDeadCodeEliminationPass());
  /// BreakCriticalEdges
  /// ------------------
  /// In the control flow graph, a critical edge is one from a basic block with
  /// multiple successors (a conditional branch) to a basic block with multiple
  /// predecessors.
  ///
  /// We use this standard LLVM pass to split such edges, to ensure that
  /// GenXCoalescing has somewhere to insert a phi copy if needed.
  ///
  PM.add(createBreakCriticalEdgesPass());
  ///
  PM.add(createGenXIMadPostLegalizationPass());
  /// .. include:: FunctionGroup.h
  /// .. include:: GenXModule.h
  PM.add(createGenXModulePass());
  /// .. include:: GenXLiveness.h
  PM.add(createGenXLivenessPass());
  PM.add(createGenXGroupBalingPass(BalingKind::BK_Analysis, &Subtarget));
  PM.add(createGenXNumberingPass());
  PM.add(createGenXLiveRangesPass());
  /// .. include:: GenXRematerialization.cpp
  PM.add(createGenXRematerializationPass());
  /// .. include:: GenXCategory.cpp
  PM.add(createGenXCategoryPass());
  /// Late SIMD CF conformance pass
  /// -----------------------------
  /// This is the same pass as GenXSimdCFConformance above, but run in a
  /// slightly different way. See above.
  ///
  /// **IR restriction**: After this pass, the EM values must have EM register
  /// category. The RM values must have RM register category. The !any result of
  /// a goto/join must have NONE register category.
  ///
  PM.add(createGenXLateSimdCFConformancePass());
  /// CodeGen baling pass
  /// -------------------
  /// This is the same pass as GenXBaling above, but run in a slightly different
  /// way. See above.
  ///
  /// **IR restriction**: Any pass after this needs to be careful when modifying
  /// code, as it also needs to update baling info.
  ///
  PM.add(createGenXGroupBalingPass(BalingKind::BK_CodeGen, &Subtarget));
  /// .. include:: GenXUnbaling.cpp
  PM.add(createGenXUnbalingPass());
  /// .. include:: GenXDepressurizer.cpp
  PM.add(createGenXDepressurizerPass());
  /// .. include:: GenXNumbering.h
  PM.add(createGenXNumberingPass());
  /// .. include:: GenXLiveRanges.cpp
  PM.add(createGenXLiveRangesPass());
  /// .. include:: GenXCoalescing.cpp
  PM.add(createGenXCoalescingPass());
  /// .. include:: GenXAddressCommoning.cpp
  PM.add(createGenXAddressCommoningPass());
  /// .. include:: GenXArgIndirection.cpp
  PM.add(createGenXArgIndirectionPass());
  /// .. include:: GenXTidyControlFlow.cpp
  //initializeLoopInfoPass(*PassRegistry::getPassRegistry());
  PM.add(createGenXTidyControlFlowPass());
  /// .. include:: GenXVisaRegAlloc.h
  auto RegAlloc = createGenXVisaRegAllocPass();
  PM.add(RegAlloc);
  if (DumpRegAlloc || Subtarget.dumpRegAlloc())
    PM.add(createGenXGroupAnalysisDumperPass(RegAlloc, ".regalloc"));
  /// .. include:: GenXVisaFuncWriter.cpp
  PM.add(createGenXVisaFuncWriterPass());
  if (!DisableVerify) PM.add(createVerifierPass());
  /// .. include:: GenXVisaWriter.cpp
  PM.add(createGenXVisaWriterPass(o));
  return false;
}
