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
#include "llvm/IR/IntrinsicsGenX.h"
#include "llvm/IR/IRBuilder.h"
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

static cl::opt<bool>
    EnableOCLCodeGen("enable-opencl-codegen", cl::init(false), cl::Hidden,
                     cl::desc("Enable codegen for OpenCL runtime"));

namespace {

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

  // Emit code for OCL runtime.
  bool OCLCodeGen;

public:
  static char ID;
  CMKernelArgOffset(unsigned GrfByteSize = 32, bool OCLCodeGen = false)
      : ModulePass(ID), GrfByteSize(GrfByteSize), OCLCodeGen(OCLCodeGen) {
    initializeCMKernelArgOffsetPass(*PassRegistry::getPassRegistry());
    GrfMaxCount = 256;
    GrfStartOffset = GrfByteSize;
    GrfEndOffset = 128 * GrfByteSize;
  }
  virtual void getAnalysisUsage(AnalysisUsage &AU) const { }
  virtual StringRef getPassName() const { return "CM kernel arg offset"; }
  virtual bool runOnModule(Module &M);
private:
  void processKernel(MDNode *Node);
  void processKernelOnOCLRT(MDNode *Node, Function *F);

  static Value *getValue(Metadata *M) {
    if (auto VM = dyn_cast<ValueAsMetadata>(M))
      return VM->getValue();
    return nullptr;
  }

  // Get the arg kinds and check whether there is an input/output argument
  // attribute.
  void checkArgKinds(MDNode *KernelMD) {
    MDNode *TypeNode = dyn_cast<MDNode>(KernelMD->getOperand(3));
    assert(TypeNode);
    for (unsigned i = 0, e = TypeNode->getNumOperands(); i != e; ++i) {
      ConstantInt *AK = cast<ConstantInt>(getValue(TypeNode->getOperand(i)));
      ArgKinds.push_back((uint32_t)AK->getZExtValue());
    }

    MDNode *IOKinds = dyn_cast<MDNode>(KernelMD->getOperand(6));
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
  }

  // Relayout thread paylod for OpenCL runtime.
  bool enableOCLCodeGen() const { return OCLCodeGen || EnableOCLCodeGen; }

  // Update ArgKind metadata so that it aligns with OpenCL style.
  // This is a cosmetic cleanup.
  void clearArgKinds(MDNode *KernelMD) {
    MDNode *TypeNode = dyn_cast<MDNode>(KernelMD->getOperand(3));
    assert(TypeNode);

    LLVMContext &C = KernelMD->getContext();
    SmallVector<Metadata *, 8> Kinds;
    for (unsigned i = 0; i < TypeNode->getNumOperands(); ++i) {
      ConstantInt *AK = cast<ConstantInt>(getValue(TypeNode->getOperand(i)));
      // Mask out the implicit kind for OpenCL runtime.
      uint32_t Val = (uint32_t)AK->getZExtValue() & 0x7;
      Kinds.push_back(ValueAsMetadata::getConstant(
          ConstantInt::get(Type::getInt32Ty(C), Val)));
    }
    KernelMD->replaceOperandWith(3, MDNode::get(C, Kinds));
  }

  // Update offset MD node
  void updateOffsetMD(MDNode *KernelMD,
                      SmallDenseMap<Argument *, unsigned> &PlacedArgs) {
    Function *F = dyn_cast_or_null<Function>(getValue(KernelMD->getOperand(0)));
    assert(F && "null kernel");

    // All arguments now have offsets. Update the metadata node containing the
    // offsets.
    assert(F->arg_size() == ArgKinds.size() &&
           "Mismatch between metadata for kernel and number of args");
    SmallVector<Metadata *, 8> ArgOffsets;
    auto I32Ty = Type::getInt32Ty(F->getContext());
    for (auto ai = F->arg_begin(), ae = F->arg_end(); ai != ae; ++ai) {
      Argument *Arg = &*ai;
      ArgOffsets.push_back(ValueAsMetadata::getConstant(
          ConstantInt::get(I32Ty, PlacedArgs[Arg])));
    }
    MDNode *OffsetsNode = MDNode::get(F->getContext(), ArgOffsets);
    KernelMD->replaceOperandWith(5, OffsetsNode);
    ArgKinds.clear();

    // Give an error on too many arguments.
    if (ArgOffsets.size() >= GrfMaxCount)
      DiagnosticInfoCMKernelArgOffset::emit(&F->front().front(),
                                            "Too many kernel arguments");
  }

  unsigned GrfByteSize;
  unsigned GrfMaxCount;
  unsigned GrfStartOffset;
  unsigned GrfEndOffset;
};

} // namespace

char CMKernelArgOffset::ID = 0;

INITIALIZE_PASS_BEGIN(CMKernelArgOffset, "cmkernelargoffset",
                      "CM kernel arg offset determination",
                      false, false)
INITIALIZE_PASS_END(CMKernelArgOffset, "cmkernelargoffset",
                    "CM kernel arg offset determination",
                    false, false)

Pass *llvm::createCMKernelArgOffsetPass(unsigned GrfByteSize, bool OCLCodeGen) {
  return new CMKernelArgOffset(GrfByteSize, OCLCodeGen);
}

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
 *  7: kernel argument type descriptors (optional).
 */
void CMKernelArgOffset::processKernel(MDNode *Node)
{
  Function *F = dyn_cast_or_null<Function>(getValue(Node->getOperand(0)));
  if (!F)
    return;

  checkArgKinds(Node);

  // Layout kernel arguments differently if to run on OpenCL runtime.
  if (enableOCLCodeGen() && F->hasFnAttribute("oclrt")) {
    auto Attr = F->getFnAttribute("oclrt");
    if (Attr.getValueAsString() == "true")
      return processKernelOnOCLRT(Node, F);
  }

  auto getTypeSizeInBytes = [=](Type *Ty) {
    const DataLayout &DL = F->getParent()->getDataLayout();
    if (auto PT = dyn_cast<PointerType>(Ty))
      return DL.getPointerTypeSize(Ty);
    return Ty->getPrimitiveSizeInBits() / 8;
  };

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
    FreeZones.push_back(GrfParamZone(GrfStartOffset, GrfEndOffset));

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

      if (BestSize > GrfByteSize)
        Align = GrfByteSize;
      else
        Align = BestElemSize;

      auto zi = FreeZones.begin();
      auto ze = FreeZones.end();

      unsigned Start = 0, End = 0;

      for (; zi != ze; ++ zi) {
        GrfParamZone &Zone = *zi;

        Start = alignTo(Zone.Start, Align);
        End = Start + BestSize;

        if ((Start % GrfByteSize) != 0 &&
            (Start / GrfByteSize) != (End - 1) / GrfByteSize) {
          Start = alignTo(Zone.Start, GrfByteSize);
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
          Align = GrfByteSize;
        FirstThreadImplicit = false;
        Offset = alignTo(Offset, Align);
        if ((Offset & (GrfByteSize - 1)) + Bytes > GrfByteSize) {
          // GRF align if arg would cross GRF boundary
          Offset = alignTo(Offset, GrfByteSize);
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
    Offset = GrfStartOffset;

    for (auto &Arg : F->args()) {
      Type *Ty = Arg.getType();
      unsigned Bytes = Ty->getScalarSizeInBits() / 8;
      Offset = alignTo(Offset, Bytes);

      if (isa<VectorType>(Ty)) {
        Bytes = Ty->getPrimitiveSizeInBits() / 8;
        if ((Offset & (GrfByteSize - 1)) + Bytes > GrfByteSize)
          // GRF align if arg would cross GRF boundary
          Offset = alignTo(Offset, GrfByteSize);
      }

      PlacedArgs[&Arg] = Offset;
      Offset += Bytes;
    }
  }

  // Update the offset MD node.
  updateOffsetMD(Node, PlacedArgs);
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

namespace {

struct KernelArgInfo {
  uint32_t Kind;
  explicit KernelArgInfo(uint32_t Kind) : Kind(Kind) {}
  bool isNormalCategory() const {
    return (Kind & 0x7) == genx::KernelMetadata::AK_NORMAL;
  }
  bool isLocalIDX() const {
    uint32_t Val = Kind & 0xFFF8;
    return Val == genx::KernelMetadata::IMP_OCL_LOCAL_ID_X;
  }
  bool isLocalIDY() const {
    uint32_t Val = Kind & 0xFFF8;
    return Val == genx::KernelMetadata::IMP_OCL_LOCAL_ID_Y;
  }
  bool isLocalIDZ() const {
    uint32_t Val = Kind & 0xFFF8;
    return Val == genx::KernelMetadata::IMP_OCL_LOCAL_ID_Z;
  }
  bool isGroupOrLocalSize() const {
    uint32_t Val = Kind & 0xFFF8;
    return Val == genx::KernelMetadata::IMP_OCL_GROUP_OR_LOCAL_SIZE;
  }
};

} // namespace

void CMKernelArgOffset::processKernelOnOCLRT(MDNode *Node, Function *F) {
  BasicBlock &Entry = F->getEntryBlock();

  // Update sr0 to enable all channels.
  // mov(1) sr0.2<1>:ud 0xffffffff:ud
  {
    Instruction *InsertPt = &*Entry.getFirstInsertionPt();
    IRBuilder<> Builder(InsertPt);
    auto ID = Intrinsic::genx_set_sr0_2;
    Function *Fn = Intrinsic::getDeclaration(F->getParent(), ID);
    FunctionType *Ty = Fn->getFunctionType();
    Value *Arg = ConstantInt::get(Ty->getFunctionParamType(0), 0xFFFFFFFF);
    Builder.CreateCall(Fn, Arg);
  }

  // Assign BTI values.
  {
    unsigned SurfaceID = 0;
    unsigned SamplerID = 0;
    unsigned VMEID = 0;
    auto Kind = ArgKinds.begin();
    for (auto &Arg : F->args()) {
      if (*Kind == genx::KernelMetadata::AK_SAMPLER)
        Arg.replaceAllUsesWith(ConstantInt::get(Arg.getType(), SamplerID++));
      if (*Kind == genx::KernelMetadata::AK_SURFACE)
        Arg.replaceAllUsesWith(ConstantInt::get(Arg.getType(), SurfaceID++));
      if (*Kind == genx::KernelMetadata::AK_VME)
        Arg.replaceAllUsesWith(ConstantInt::get(Arg.getType(), VMEID++));
      ++Kind;
    }
  }

  SmallDenseMap<Argument *, unsigned> PlacedArgs;
  {
    // OpenCL SIMD8 thread payloads are organized as follows:
    //
    //     0        1        2        3        4        5        6        7
    // R0:          GX                                           GY       GZ
    // R1: LX0,LX1  LX2,LX3  LX4,LX5  LX6,LX7
    // R2: LY0,LY1  LY2,LY3  LY4,LY5  LY6,LY7
    // R3: LZ0,LZ1  LZ2,LZ3  LZ4,LZ5  LZ6,LZ7
    // R4: GO_X     GO_Y     GO_Z     LWS_X    LWS_Y    LWS_Z
    // R5: other cross-thread constant data (1-3 GRFs)
    //
    unsigned Offset = GrfStartOffset;

    // Always place R1, R2, R3, and R4
    unsigned ThreadPayloads[] = {
        Offset,                   // R1, local_id_x
        Offset + GrfByteSize,     // R2, local_id_y
        Offset + 2 * GrfByteSize, // R3, local_id_z
        Offset + 3 * GrfByteSize  // R4, group or local size
    };
    auto getImpOffset = [&](KernelArgInfo AI) -> int {
      if (AI.isLocalIDX())
        return ThreadPayloads[0];
      if (AI.isLocalIDY())
        return ThreadPayloads[1];
      if (AI.isLocalIDZ())
        return ThreadPayloads[2];
      if (AI.isGroupOrLocalSize())
        return ThreadPayloads[3];
      return -1;
    };

    // Starting offsets for non-implicit arguments.
    Offset += 4 * GrfByteSize;

    auto Kind = ArgKinds.begin();
    for (auto &Arg : F->args()) {
      // Skip arguments that are processed already.
      KernelArgInfo AI(*Kind++);
      if (!AI.isNormalCategory()) {
        PlacedArgs[&Arg] = genx::KernelMetadata::SKIP_OFFSET_VAL;
        continue;
      }

      // Implicit argument offsets.
      int ImpOffset = getImpOffset(AI);
      if (ImpOffset > 0) {
        PlacedArgs[&Arg] = ImpOffset;
        continue;
      }

      // Normal arguments.
      //
      // Arguments larger than a GRF must be at least GRF-aligned. Arguments
      // smaller than a GRF may not cross GRF boundaries. This means that
      // arguments cross a GRF boundary must be GRF aligned.
      //
      Type *Ty = Arg.getType();
      unsigned Alignment = Ty->getScalarSizeInBits() / 8;
      Offset = alignTo(Offset, Alignment);
      unsigned Bytes = Ty->getPrimitiveSizeInBits() / 8;
      unsigned StartGRF = Offset / GrfByteSize;
      unsigned EndGRF = (Offset + Bytes - 1) / GrfByteSize;
      if (StartGRF != EndGRF)
        Offset = alignTo(Offset, GrfByteSize);

      PlacedArgs[&Arg] = Offset;
      Offset += Bytes;
    }
  }

  clearArgKinds(Node);
  updateOffsetMD(Node, PlacedArgs);
}
