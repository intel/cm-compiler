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
/// CMKernelArgOffset
/// -----------------
///
/// This pass determines the offset of each CM kernel argument, and adds it to
/// the kernel metadata.
/// 
/// A CM kernel has metadata containing, amongst other things, an array of *kind*
/// bytes, one byte per kernel argument, that will be output in the vISA kernel input
/// table. This pass calculates the offset of each kernel argument, and adds an
/// array to the kernel metadata containing the calculated offsets.
/// 
/// Argument offsets start at 32, as r0 is reserved by the various thread dispatch
/// mechanisms.
/// 
/// The pass attempts to calculate the kernel argument offsets in a way that
/// minimizes space wasted by holes.
/// 
/// The arguments are processed in three sets, with each (non-empty) set starting
/// in a new GRF:
/// 
/// 1. explicit kernel arguments (i.e. ones that appeared in the CM source);
/// 
/// 2. implicit kernel (non-thread) arguments;
/// 
/// 3. implicit thread arguments.
/// 
/// These three sets need to be allocated as three separate chunks of whole GRF
/// registers in that order by the CM runtime. In theory, the CM runtime can cope
/// with the compiler creating a different ordering, but to do so it needs to
/// create its own ordering and insert mov instructions at the start of the kernel,
/// which is suboptimal. However, I am not clear whether that mechanism works, and
/// it has not been tested.
///
/// There is a compiler option that can be used to disable argument re-ordering. This is
/// for developers who are using the output asm files directly and want to control
/// the argument order explicitly. The option is -enable-kernel-arg-reordering
/// but is typically invoked as -mllvm -enable-kernel-arg-reordering=false (the
/// default is true)
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "cmkernelargoffset"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

static cl::opt<bool> EnableKernelArgReordering("enable-kernel-arg-reordering", cl::init(true), cl::Hidden,
                                               cl::desc("Enable kernel argument reordering"));

namespace {

enum {
  GENX_MAX_INPUT = 256,
  GENX_WIDTH_GENERAL_REG = 32,
  STARTOFFSET = 1 * GENX_WIDTH_GENERAL_REG,
  ENDOFFSET = 128 * GENX_WIDTH_GENERAL_REG
};

struct GrfParamZone {
  unsigned Start;
  unsigned End;
  GrfParamZone(unsigned s, unsigned e) : Start(s), End(e) {};
};

// Diagnostic information for error/warning from this pass.
class DiagnosticInfoCMKernelArgOffset : public DiagnosticInfoOptimizationBase {
private:
  static int KindID;
  static int getKindID() {
    if (KindID == 0)
      KindID = llvm::getNextAvailablePluginDiagnosticKind();
    return KindID;
  }
public:
  static void emit(Instruction *Inst, StringRef Msg,
                   DiagnosticSeverity Severity = DS_Error);
  DiagnosticInfoCMKernelArgOffset(DiagnosticSeverity Severity,
                                  const Function &Fn, const DebugLoc &DLoc,
                                  StringRef Msg)
      : DiagnosticInfoOptimizationBase((DiagnosticKind)getKindID(), Severity,
                                       /*PassName=*/nullptr, Msg, Fn, DLoc) {}
  // This kind of message is always enabled, and not affected by -rpass.
  virtual bool isEnabled() const override { return true; }
  static bool classof(const DiagnosticInfo *DI) {
    return DI->getKind() == getKindID();
  }
};
int DiagnosticInfoCMKernelArgOffset::KindID = 0;

// CMKernelArgOffset pass
class CMKernelArgOffset : public ModulePass {
  SmallVector<unsigned, 8> ArgKinds;
public:
  static char ID;
  CMKernelArgOffset() : ModulePass(ID) {
    initializeCMKernelArgOffsetPass(*PassRegistry::getPassRegistry());
  }
  virtual void getAnalysisUsage(AnalysisUsage &AU) const { }
  virtual StringRef getPassName() const { return "CM kernel arg offset"; }
  virtual bool runOnModule(Module &M);
private:
  void processKernel(MDNode *Node);
};

} // namespace

char CMKernelArgOffset::ID = 0;

INITIALIZE_PASS_BEGIN(CMKernelArgOffset, "cmkernelargoffset",
                      "CM kernel arg offset determination",
                      false, false)
INITIALIZE_PASS_END(CMKernelArgOffset, "cmkernelargoffset",
                    "CM kernel arg offset determination",
                    false, false)

Pass *llvm::createCMKernelArgOffsetPass() { return new CMKernelArgOffset(); }

/***********************************************************************
 * runOnModule : run the CM kernel arg offset pass
 */
bool CMKernelArgOffset::runOnModule(Module &M)
{
  NamedMDNode *Named = M.getNamedMetadata("genx.kernels");
  if (!Named)
    return 0;
  // Process each kernel in the CM kernel metadata.
  for (unsigned i = 0, e = Named->getNumOperands(); i != e; ++i) {
    MDNode *KernelNode = Named->getOperand(i);
    if (KernelNode)
      processKernel(KernelNode);
  }
  return true;
}

/***********************************************************************
 * processKernel : process one kernel
 *
 * Enter:   Node = metadata node for one kernel
 *
 * The genx.kernels named metadata node contains a metadata node
 * for each kernel, containing:
 *  0: reference to Function
 *  1: kernel name
 *  2: asm name
 *  3: kernel argument kinds
 *  4: slm size in bytes
 *  5: kernel argument offsets: initially placeholder; set to array in this pass
 *  6: kernel input/output kinds
 */
void CMKernelArgOffset::processKernel(MDNode *Node)
{
  if (Node->getNumOperands() < 7)
    return;

  auto getValue = [](Metadata *M) -> Value * {
    if (auto VM = dyn_cast<ValueAsMetadata>(M))
      return VM->getValue();
    return nullptr;
  };

  Function *F = dyn_cast_or_null<Function>(getValue(Node->getOperand(0)));
  if (!F)
    return;

  auto getTypeSizeInBytes = [=](Type *Ty) {
    const DataLayout &DL = F->getParent()->getDataLayout();
    if (auto PT = dyn_cast<PointerType>(Ty))
      return DL.getPointerTypeSize(Ty);
    return Ty->getPrimitiveSizeInBits() / 8;
  };

  // Get the arg kinds.
  MDNode *TypeNode = dyn_cast<MDNode>(Node->getOperand(3));
  assert(TypeNode);
  for (unsigned i = 0, e = TypeNode->getNumOperands(); i != e; ++i) {
    ConstantInt *AK = cast<ConstantInt>(getValue(TypeNode->getOperand(i)));
    ArgKinds.push_back((uint32_t)AK->getZExtValue());
  }

  // Check whether there is an input/output argument attribute.
  MDNode *IOKinds = dyn_cast<MDNode>(Node->getOperand(6));
  assert(IOKinds);
  for (unsigned i = 0, e = IOKinds->getNumOperands(); i != e; ++i) {
    ConstantInt *K = cast<ConstantInt>(getValue(IOKinds->getOperand(i)));
    // Value 0 means there is no input/output attribute; and compiler could
    // freely reorder arguments.
    if (K->getZExtValue() != 0) {
      EnableKernelArgReordering = false;
      break;
    }
  }

  // setup kernel inputs, optionally reordering the assigned offsets for
  // improved packing where appropriate. The reordering algorithm replicates
  // that used in the legacy Cm compiler, as certain media walker applications
  // seem sensitive to the way the kernel inputs are laid out.
  SmallDenseMap<Argument *, unsigned> PlacedArgs;
  unsigned Offset = 0;
  if (EnableKernelArgReordering /*DoReordering*/) {
    // Reorder kernel input arguments. Arguments are placed in size order,
    // largest first (then in natural argument order where arguments are the
    // same size). Each argument is placed at the lowest unused suitably
    // aligned offset. So, in general big arguments are placed first with the
    // smaller arguments being fit opportunistically into the gaps left
    // between arguments placed earlier.
    //
    // Arguments that are at least one GRF in size must be aligned to a GRF
    // boundary. Arguments smaller than a GRF must not cross a GRF boundary.
    //
    // FreeZones describes unallocated portions of the kernel input space,
    // and is list of non-overlapping start-end pairs, ordered lowest first.
    // Initially it consists of a single pair that describes the whole space

    SmallVector<GrfParamZone, 16> FreeZones;
    FreeZones.push_back(GrfParamZone(STARTOFFSET, ENDOFFSET));

    // Repeatedly iterate over the arguments list, each time looking for the
    // largest one that hasn't been processed yet.
    // But ignore implicit args for now as they want to go after all the others.

    do {
      Argument *BestArg = NULL;
      unsigned BestSize;
      unsigned BestElemSize;

      auto Kind = ArgKinds.begin();
      for (Function::arg_iterator i = F->arg_begin(), e = F->arg_end();
         i != e; ++i, ++Kind) {
        Argument *Arg = &*i;
        if (*Kind & 0xf8)
          continue; // implicit arg

        if (PlacedArgs.find(Arg) != PlacedArgs.end())
          // Already done this one.
          continue;

        Type *Ty = Arg->getType();
        unsigned Bytes = getTypeSizeInBytes(Ty);

        if (BestArg == NULL || BestSize < Bytes) {
          BestArg = Arg;
          BestSize = Bytes;
          BestElemSize = getTypeSizeInBytes(Ty->getScalarType());
        }
      }

      if (BestArg == NULL)
        // All done.
        break;

      // The best argument in this cycle has been found. Search FreeZones for
      // a suitably sized and aligned gap.

      unsigned Align;

      if (BestSize > GENX_WIDTH_GENERAL_REG)
        Align = GENX_WIDTH_GENERAL_REG;
      else
        Align = BestElemSize;

      auto zi = FreeZones.begin();
      auto ze = FreeZones.end();

      unsigned Start = 0, End = 0;

      for (; zi != ze; ++ zi) {
        GrfParamZone &Zone = *zi;

        Start = alignTo(Zone.Start, Align);
        End = Start + BestSize;

        if ((Start % GENX_WIDTH_GENERAL_REG) != 0 &&
            (Start / GENX_WIDTH_GENERAL_REG) != (End - 1) / GENX_WIDTH_GENERAL_REG) {
          Start = alignTo(Zone.Start, GENX_WIDTH_GENERAL_REG);
          End = Start + BestSize;
        }

        if (End <= Zone.End)
          // Found one. This should never fail unless we have too many
          // parameters to start with.
          break;
      }

      assert(zi != ze && "unable to allocate argument offset (too many arguments?)");

      // Exclude the found block from the free zones list. This may require
      // that the found zone be split in two if the start of the block is
      // not suitably aligned.

      GrfParamZone &Zone = *zi;

      if (Zone.Start == Start)
        Zone.Start = End;
      else {
        unsigned NewEnd = Zone.End;
        Zone.End = Start;
        ++ zi;
        FreeZones.insert(zi, GrfParamZone(End, NewEnd));
      }

      PlacedArgs[BestArg] = Start;
    } while (true);
    // Now process the implicit args. First get the offset at the start of the
    // last free zone. Process the implicit kernel args first, then the
    // implicit thread args.
    Offset = FreeZones.back().Start;
    for (int WantThreadImplicit = 0; WantThreadImplicit != 2;
        ++WantThreadImplicit) {
      bool FirstThreadImplicit = WantThreadImplicit;
      auto Kind = ArgKinds.begin();
      for (Function::arg_iterator i = F->arg_begin(), e = F->arg_end();
         i != e; ++i, ++Kind) {
        Argument *Arg = &*i;
        if (!(*Kind & 0xf8))
          continue; // not implicit arg
        int IsThreadImplicit = (*Kind >> 3) == 3; // local_id
        if (WantThreadImplicit != IsThreadImplicit)
          continue;
        Type *Ty = Arg->getType();
        unsigned Bytes = Ty->getPrimitiveSizeInBits() / 8U;
        unsigned Align = Ty->getScalarSizeInBits() / 8U;
        // If this is the first thread implicit arg, put it in a new GRF.
        if (FirstThreadImplicit)
          Align = GENX_WIDTH_GENERAL_REG;
        FirstThreadImplicit = false;
        Offset = alignTo(Offset, Align);
        if ((Offset & (GENX_WIDTH_GENERAL_REG - 1)) + Bytes
            > GENX_WIDTH_GENERAL_REG) {
          // GRF align if arg would cross GRF boundary
          Offset = alignTo(Offset, GENX_WIDTH_GENERAL_REG);
        }
        PlacedArgs[Arg] = Offset;
        Offset += Bytes;
      }
    }
  } else {
    // No argument reordering. Arguments are placed at increasing offsets
    // in their natural order, aligned according to their type.
    //
    // Again, arguments that are at least one GRF in size must be aligned to
    // a GRF boundary. Arguments smaller than a GRF must not cross a GRF
    // boundary.

    // kernel input start offset
    Offset = STARTOFFSET;

    for (Function::arg_iterator i = F->arg_begin(), e = F->arg_end();
        i != e; ++i) {
      Argument *Arg = &*i;

      Type *Ty = Arg->getType();
      unsigned Bytes = Ty->getScalarSizeInBits() / 8;

      Offset = alignTo(Offset, Bytes);

      if (isa<VectorType>(Ty)) {
        Bytes = Ty->getPrimitiveSizeInBits() / 8;

        if ((Offset & (GENX_WIDTH_GENERAL_REG - 1)) + Bytes > GENX_WIDTH_GENERAL_REG)
          // GRF align if arg would cross GRF boundary
          Offset = alignTo(Offset, GENX_WIDTH_GENERAL_REG);
      }

      PlacedArgs[Arg] = Offset;

      Offset += Bytes;
    }
  }

  // All arguments now have offsets. Update the metadata node containing the
  // offsets.
  assert(F->arg_size() == ArgKinds.size() && "Mismatch between metadata for kernel and number of args");
  SmallVector<Metadata *, 8> ArgOffsets;
  auto I32Ty = Type::getInt32Ty(F->getContext());
  for (auto ai = F->arg_begin(), ae = F->arg_end(); ai != ae; ++ai) {
    Argument *Arg = &*ai;
    ArgOffsets.push_back(ValueAsMetadata::getConstant(ConstantInt::get(I32Ty, PlacedArgs[Arg])));
  }
  MDNode *OffsetsNode = MDNode::get(F->getContext(), ArgOffsets);
  Node->replaceOperandWith(5, OffsetsNode);
  // Clear state.
  ArgKinds.clear();

  // Give an error on too many arguments.
  if (ArgOffsets.size() >= GENX_MAX_INPUT)
    DiagnosticInfoCMKernelArgOffset::emit(&F->front().front(),
                                          "Too many kernel arguments");
}

/***********************************************************************
 * DiagnosticInfoCMKernelArgOffset::emit : emit an error or warning
 */
void DiagnosticInfoCMKernelArgOffset::emit(Instruction *Inst, StringRef Msg,
                                           DiagnosticSeverity Severity) {
  DiagnosticInfoCMKernelArgOffset Err(Severity, *Inst->getParent()->getParent(),
      Inst->getDebugLoc(), Msg);
  Inst->getContext().diagnose(Err);
}

