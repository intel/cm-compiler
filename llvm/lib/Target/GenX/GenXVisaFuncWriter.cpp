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
/// GenXVisaFuncWriter
/// ------------------
///
/// This writes a vISA kernel or function. It is a FunctionGroupPass, thus it
/// runs once for each kernel, writing the kernel and its subroutines.
///
/// The bulk of the work is done in the constructor of the VisaFuncWriter class,
/// which is an implementation of the abstract FuncWriter class in GenXModule.h.
///
/// VisaFuncWriter has three members that are all instances of the Stream class
/// (also from GenXModule.h), providing an interface for writing byte data to
/// a stream. The three members are:
///
/// * Header: the kernel's header in the vISA file
///
/// * Body: the kernel's body in the vISA file
///
/// * Code: the kernel's code in the vISA file.
///
/// Once the VisaFuncWriter has been constructed (and thus the vISA for the
/// kernel written into the three streams), it is pushed into GenXModule,
/// where it is picked up by the subsequent GenXVisaWriter pass.
///
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include "visa_igc_common_header.h"
#include "FunctionGroup.h"
#include "GenX.h"
#include "GenXAlignmentInfo.h"
#include "GenXBaling.h"
#include "GenXGotoJoin.h"
#include "GenXIntrinsics.h"
#include "GenXLiveness.h"
#include "GenXModule.h"
#include "GenXRegion.h"
#include "GenXPressureTracker.h"
#include "GenXSubtarget.h"
#include "GenXVisa.h"
#include "GenXVisaRegAlloc.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Path.h"
#include <vector>
using namespace llvm;
using namespace visa;
using namespace genx;

/***********************************************************************
 * GenXVisaFuncWriter pass declaration
 */
namespace {
  class GenXVisaFuncWriter : public FunctionGroupPass {
    const GenXSubtarget *ST;
  public:
    static char ID;
    explicit GenXVisaFuncWriter() : FunctionGroupPass(ID) {}
    virtual StringRef getPassName() const {
      return "GenX vISA function writer";
    }

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DominatorTreeGroupWrapperPass>();
      AU.addRequired<GenXGroupBaling>();
      AU.addRequired<GenXLiveness>();
      AU.addRequired<GenXVisaRegAlloc>();
      AU.addRequired<GenXModule>();
      AU.setPreservesAll();
    }

    bool runOnFunctionGroup(FunctionGroup &FG);
    // createPrinterPass : get a pass to print the IR, together with the GenX
    // specific analyses
    virtual Pass *createPrinterPass(raw_ostream &O, const std::string &Banner) const
    { return createGenXGroupPrinterPass(O, Banner); }
  };
} // end anonymous namespace.

char GenXVisaFuncWriter::ID = 0;
namespace llvm { void initializeGenXVisaFuncWriterPass(PassRegistry &); }
INITIALIZE_PASS_BEGIN(GenXVisaFuncWriter, "GenXVisaFuncWriter", "GenXVisaFuncWriter", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeGroupWrapperPass)
INITIALIZE_PASS_DEPENDENCY(GenXGroupBaling)
INITIALIZE_PASS_DEPENDENCY(GenXLiveness)
INITIALIZE_PASS_DEPENDENCY(GenXVisaRegAlloc)
INITIALIZE_PASS_DEPENDENCY(GenXModule)
INITIALIZE_PASS_END(GenXVisaFuncWriter, "GenXVisaFuncWriter", "GenXVisaFuncWriter", false, false)

FunctionGroupPass *llvm::createGenXVisaFuncWriterPass() {
  initializeGenXVisaFuncWriterPass(*PassRegistry::getPassRegistry());
  return new GenXVisaFuncWriter;
}

namespace {

// Label : a vISA label
struct Label {
  enum { BLOCK, SUBROUTINE, FASTCOMPOSITE };
  unsigned NameIndex;
  unsigned Kind;
  Label(unsigned NameIndex, int Kind) : NameIndex(NameIndex), Kind(Kind) {}
};

/***********************************************************************
 * class VisaFuncWriter : represents a vISA kernel or function
 * This is the implementation class that writes the vISA code for the function.
 */
class VisaFuncWriter : public FuncWriter {
  typedef std::map<std::string, unsigned> Strings_t;
  Strings_t Strings;
  FunctionGroup *FG;
  GenXVisaRegAlloc *RegAlloc; // Lifetime only during constructor
  GenXBaling *Baling; // ditto
  DominatorTreeGroupWrapperPass *DTs; // ditto
  GenXLiveness *Liveness; // ditto
  AlignmentInfo AI;
  Function *Func; // function currently being written during constructor
  std::map<Function *, LoopInfoBase<BasicBlock, Loop> *> Loops; // loop info for each function
  const GenXSubtarget *ST;
  Attrs Attributes;
  std::vector<Label> Labels;
  std::map<Value *, unsigned> LabelMap;
  std::map<Value *, SmallVector<unsigned, 4>> BBRefs;
  Stream Header;
  Stream Body;
  Stream Code;
  unsigned InputOffset; // offset within kernel object of inputs (kernel only)
  unsigned OffsetOffset; // offset in HS to write "offset" field to
  unsigned LabelSeq;
  int LastLabel;
  unsigned LastLine;
  unsigned PendingLine;
  StringRef LastFilename;
  StringRef PendingFilename;
  StringRef LastDirectory;
  StringRef PendingDirectory;
  KernelMetadata KM;
  bool HasBarrier;
  bool HasCallable;
  ValueMap<Function *, bool> IsInLoopCache;

  // normally 0, set to 0x80 if there is any SIMD CF in the func or this is
  // (indirectly) called inside any SIMD CF.
  unsigned NoMask;

  // The default float control from kernel attribute. Each subroutine may
  // overrride this control mask, but it should revert back to the default float
  // control mask before exiting from the subroutine.
  uint32_t DefaultFloatControl = 0;

  // The effective bits in CR register.
  static const uint32_t CR_Mask = 0x1 << 10 | 0x3 << 6 | 0x3 << 4 | 0x1;

  // GRF width in unit of byte
  unsigned GrfByteSize;

  // getStringIdx : add/find string in string table and return index
  unsigned getStringIdx(std::string Str, bool Limit64 = true);
  // getBBRef : get reference to BB's label, adding to list of forward
  // references that need patching if necessary
  unsigned getBBRef(Value *BB);
  // getLabel : get the label number for a Function or BasicBlock or join
  int getLabel(Value *V);
  // setLabel : set the label number for a Function or BasicBlock or join
  void setLabel(Value *V, unsigned Num);
  // getOrCreateLabel : get/create label for Function or BasicBlock or join
  unsigned getOrCreateLabel(Value *V, int Kind);
public:
  VisaFuncWriter(FunctionGroup *FG, GenXVisaRegAlloc *RegAlloc, GenXBaling *Baling,
      DominatorTreeGroupWrapperPass *DTs, GenXLiveness *Liveness,
      const GenXSubtarget *ST);
  ~VisaFuncWriter() { clearLoops(); }
  void clearLoops() {
    for (auto i = Loops.begin(), e = Loops.end(); i != e; ++i) {
      delete i->second;
      i->second = nullptr;
    }
    Loops.clear();
  }
  // isKernel : true if the Func is a vISA kernel
  bool isKernel() { return genx::isKernel(FG->getHead()); }
  // setOffset : set the offset field in the header
  // For a kernel, it also sets the input_offset field in the header
  void setOffset(uint32_t O) {
    Header.setData(OffsetOffset, &O, sizeof(O));
    if (isKernel()) {
      O += InputOffset;
      Header.setData(OffsetOffset + 8, &O, sizeof(O));
    }
  }
  // get header/body size
  unsigned getHeaderSize() { return Header.size(); }
  unsigned getBodySize() { return Body.size() + Code.size(); }
  // write header/body
  void writeHeader(raw_pwrite_stream &Out) { Header.write(Out); }
  void writeBody(raw_pwrite_stream &Out) { Body.write(Out); Code.write(Out); }
private:
  void getKernelAttrsFromMetadata();
  void buildInputs(Function *F, GenXVisaRegAlloc *RA, bool NeedRetIP);
  // addAttribute : add an attribute
  void addAttribute(const char *Name, const Twine &Value) {
    Attributes.push_back(getStringIdx(Name), Value);
  }
  // Build the code for this function and any subroutine (in the vISA sense)
  // that it calls.
  void buildCode(FunctionGroup *FG);
  void writeLabel(Value *BB);
  bool buildBasicBlock(BasicBlock *BB);
  bool buildInstruction(Instruction *I);

  // A utility class to capture the destination operand.
  // If a bale ends with a g_store baling, then the destination is actually
  // provided by two parts:
  // * write region instruction for region
  // * g_store instuction for the actual variable and overrided type.
  //
  struct DstOpndDesc {
    Instruction *WrRegion = nullptr;
    BaleInfo WrRegionBI;
    Instruction *GStore = nullptr;

    DstOpndDesc() {}
    DstOpndDesc(Instruction *WrRegion, BaleInfo BI,
                Instruction *GStore = nullptr)
        : WrRegion(WrRegion), WrRegionBI(BI), GStore(GStore) {
      assert(!GStore || isa<StoreInst>(GStore));
    }

    GlobalVariable *getGlobalVariable() const {
      return GStore ? getUnderlyingGlobalVariable(GStore->getOperand(1))
                    : nullptr;
    }
  };

  // Builders for individual instructions
  void buildLoneWrRegion(const DstOpndDesc &Desc);
  void buildLoneWrPredRegion(Instruction *Inst, genx::BaleInfo BI);
  void buildLoneOperand(Instruction *Inst, genx::BaleInfo BI, unsigned Mod,
                        const DstOpndDesc &DstDesc);
  bool buildMainInst(Instruction *Inst, genx::BaleInfo BI, unsigned Mod,
                     const DstOpndDesc &DstDesc);
  void buildPhiNode(PHINode *Phi);
  void buildRet(ReturnInst *RI);
  void buildCall(CallInst *CI);
  bool buildBranch(BranchInst *BI);
  void buildGoto(CallInst *Goto, BranchInst *Branch);
  void buildJoin(CallInst *Join, BranchInst *Branch);
  void buildCmp(CmpInst *Cmp, genx::BaleInfo BI, const DstOpndDesc &DstDesc);
  void buildBinaryOperator(BinaryOperator *BO, genx::BaleInfo BI, unsigned Mod,
                           const DstOpndDesc &DstDesc);
  void buildSelectInst(SelectInst *SI, BaleInfo BI, unsigned Mod,
                       const DstOpndDesc &DstDesc);
  void buildBoolBinaryOperator(BinaryOperator *BO);
  void buildCastInst(CastInst *CI, genx::BaleInfo BI, unsigned Mod,
                     const DstOpndDesc &DstDesc);
  void buildBitCast(CastInst *CI, genx::BaleInfo BI, unsigned Mod,
                    const DstOpndDesc &DstDesc);
  void buildConvert(CallInst *CI, BaleInfo BI, unsigned Mod,
                    const DstOpndDesc &DstDesc);
  void buildConvertAddr(CallInst *CI, BaleInfo BI, unsigned Mod,
                        const DstOpndDesc &DstDesc);
  void buildIntrinsic(CallInst *CI, unsigned IntrinID, genx::BaleInfo BI,
                      unsigned Mod, const DstOpndDesc &DstDesc);
  void buildControlRegUpdate(unsigned Mask, bool Clear);
  // Methods to write instruction operands etc.
  void writeWrLifetimeStart(Instruction *WrRegion);
  void writeSendLifetimeStart(Instruction *Inst);
  bool isInLoop(BasicBlock *BB);
  void writeLifetimeStart(Instruction *Inst);
  void writeExecSizeFromWrPredRegion(unsigned ExecSize,
                                     Instruction *WrPredRegion, bool IsNoMask);
  void writeExecSizeFromWrRegion(unsigned ExecSize, const DstOpndDesc &DstDesc,
                                 bool IsNoMask = false);
  void writeExecSizeFromSelect(unsigned ExecSize, Instruction *SI, BaleInfo BI);
  void writePredFromWrRegion(const DstOpndDesc &DstDesc);
  void writePredFromSelect(Instruction *SI, BaleInfo BI);
  Value *getPredicateOperand(Instruction *Inst, unsigned OperandNum,
                             BaleInfo BI, unsigned *PredField,
                             unsigned *MaskCtrl);
  void writePred(Instruction *Inst, BaleInfo BI, unsigned OperandNum);
  void writeRawDestination(Value *V, const DstOpndDesc &DstDesc,
                           genx::Signedness Signed = DONTCARESIGNED);
  void writeRawSourceOperand(Instruction *Inst, unsigned OperandNum,
                             genx::BaleInfo BI,
                             genx::Signedness Signed = DONTCARESIGNED);
  genx::Signedness writeDestination(Value *Dest, genx::Signedness Signed,
                                    unsigned Mod, const DstOpndDesc &DstDesc);
  genx::Signedness writeSourceOperand(Instruction *Inst,
                                      genx::Signedness Signed,
                                      unsigned OperandNum, genx::BaleInfo BI,
                                      unsigned Mod = 0, unsigned MaxWidth = 16);
  genx::Signedness writeSource(Value *V, genx::Signedness Signed, bool Baled,
                               unsigned Mod, unsigned MaxWidth = 16);
  void writeState(Region *R, unsigned RegNum, unsigned Kind);
  void writeRegion(Region *R, unsigned RegNum, Signedness Signed, unsigned Mod,
                   bool IsDest, unsigned MaxWidth = 16);
  void writeImmediateOperand(Constant *V, genx::Signedness Signed);
  void writeAddressOperand(Value *V);
  void writePredicateOperand(Value *V);
  void writePredicateOperand(GenXVisaRegAlloc::RegNum Reg);
  void writeOpcode(int Val);
  void writeByte(int Val) { Code.push_back((uint8_t)Val); }
  void writeShort(int Val) { Code.push_back((uint16_t)Val); }
  void writeInt(int Val) { Code.push_back((uint32_t)Val); }
  void emitOptimizationHints();
  // Auxiliary methods
  Instruction *getOriginalInstructionForSource(Instruction *CI, BaleInfo BI);
  LoopInfoBase<BasicBlock, Loop> *getLoops(Function *F);
};

} // end anonymous namespace

static Signedness getISatSrcSign(unsigned IID) {
  switch (IID) {
  case Intrinsic::genx_sstrunc_sat:
  case Intrinsic::genx_ustrunc_sat:
    return SIGNED;
  case Intrinsic::genx_sutrunc_sat:
  case Intrinsic::genx_uutrunc_sat:
    return UNSIGNED;
  default:
    return DONTCARESIGNED;
  }
}

static Signedness getISatDstSign(unsigned IID) {
  switch (IID) {
  case Intrinsic::genx_sstrunc_sat:
  case Intrinsic::genx_sutrunc_sat:
    return SIGNED;
  case Intrinsic::genx_ustrunc_sat:
  case Intrinsic::genx_uutrunc_sat:
    return UNSIGNED;
  default:
    return DONTCARESIGNED;
  }
}

static Signedness getISatSrcSign(Value *V) {
  return getISatSrcSign(getIntrinsicID(V));
}

static Signedness getISatDstSign(Value *V) {
  return getISatDstSign(getIntrinsicID(V));
}

/***********************************************************************
 * GenXVisaFuncWriter::runOnFunctionGroup : process a FunctionGroup
 */
bool GenXVisaFuncWriter::runOnFunctionGroup(FunctionGroup &FG)
{
  // Create the vISA Func and add it to GenXVisaFuncWriter.
  GenXModule *GM = getAnalysisIfAvailable<GenXModule>();
  GenXVisaRegAlloc *VRA = getAnalysisIfAvailable<GenXVisaRegAlloc>();
  GenXBaling *Baling = &getAnalysis<GenXGroupBaling>();
  auto DTs = &getAnalysis<DominatorTreeGroupWrapperPass>();
  auto Liveness = &getAnalysis<GenXLiveness>();
  auto P = getAnalysisIfAvailable<GenXSubtargetPass>();
  ST = P ? P->getSubtarget() : nullptr;
  GM->push_back(new VisaFuncWriter(&FG, VRA, Baling, DTs, Liveness, ST));
  return false;
}

/***********************************************************************
 * VisaFuncWriter constructor
 *
 * This builds the vISA function header, body and code into the respective
 * Streams in the VisaFuncWriter object.
 */
VisaFuncWriter::VisaFuncWriter(FunctionGroup *FG, GenXVisaRegAlloc *RA,
    GenXBaling *Baling, DominatorTreeGroupWrapperPass *DTs,
    GenXLiveness *Liveness, const GenXSubtarget *ST)
    : FuncWriter(), FG(FG), RegAlloc(RA), Baling(Baling), DTs(DTs),
      Liveness(Liveness), ST(ST), LabelSeq(0), LastLine(0), PendingLine(0),
      KM(FG->getHead()), HasBarrier(false), HasCallable(false)
{
  GrfByteSize = ST ? ST->getGRFWidth() : 32;
  // The string table always starts with an empty string.
  getStringIdx("");
  // Extract kernel name, asm name and slm size from genx.kernels metadata.
  getKernelAttrsFromMetadata();
  // Build the code for this function and any subroutine (in the vISA sense)
  // that it calls.
  buildCode(FG);
  // Add the function name and get its string index. Do not limit to 64 byes otherwise long kernel
  // names (typically templated functions) will fail for sim mode (fix for VMCL-816)
  StringRef Name = KM.getName();
  if (!Name.size()) {
    // If it is not a kernel, or no metadata was found, then set the
    // name to the IR name.
    Name = FG->getHead()->getName();
  }
  bool NeedRetIP = false; // Need special return IP variable for FC.
  unsigned NameIdx = getStringIdx(Name, false);
  if (KM.isKernel()) {
    // For a kernel, add an attribute for asm filename for the jitter.
    if (!KM.getAsmName().empty())
      addAttribute("AsmName", KM.getAsmName());
    // For a kernel with no barrier instruction, add a NoBarrier attribute.
    if (!HasBarrier)
      addAttribute("NoBarrier", "");

    // Populate variable attributes if any.
    unsigned Idx = 0;
    bool IsComposable = false;
    for (auto &Arg : FG->getHead()->args()) {
      const char *Kind = nullptr;
      switch (KM.getArgInputOutputKind(Idx++)) {
      default:
        break;
      case KernelMetadata::IO_INPUT:
        Kind = "Input";
        break;
      case KernelMetadata::IO_OUTPUT:
        Kind = "Output";
        break;
      case KernelMetadata::IO_INPUT_OUTPUT:
        Kind = "Input_Output";
        break;
      }
      if (Kind != nullptr) {
        auto R = RegAlloc->getRegNumForValueUntyped(&Arg);
        assert(R.Category == RegCategory::GENERAL);
        RegAlloc->addAttr(R, getStringIdx(Kind), "");
        IsComposable = true;
      }
    }
    if (IsComposable)
      addAttribute("Composable", "");
    if (HasCallable) {
      addAttribute("Caller", "");
      NeedRetIP = true;
    }
    if (FG->getHead()->hasFnAttribute("CMCallable")) {
      addAttribute("Callable", "");
      NeedRetIP = true;
    }
    if (FG->getHead()->hasFnAttribute("CMEntry")) {
      addAttribute("Entry", "");
    }
  }

  if (NeedRetIP) {
    // Ask RegAlloc to add a special variable RetIP.
    RegAlloc->addRetIPArgument();
    auto R = RegAlloc->getRetIPArgument();
    RegAlloc->setName(R, getStringIdx("RetIP"));
    RegAlloc->addAttr(R, getStringIdx("Input_Output"), "");
  }

  // For a kernel, give each kernel input a name so as not to confuse sim mode.
  if (false && isKernel()) {
    for (auto ai = FG->getHead()->arg_begin(), ae = FG->getHead()->arg_end();
        ai != ae; ++ai) {
      auto Arg = &*ai;
      auto R = RegAlloc->getRegNumForValueUntyped(Arg);
      RegAlloc->setName(R,
          getStringIdx((Twine("Input") + Twine(Arg->getArgNo())).str()));
    }
  }

  // Emit optimization hints if any.
  emitOptimizationHints();

  // Write the strings.
  {
    unsigned NumStrings = Strings.size();
    Body.push_back((uint32_t)NumStrings);
    const std::string **StringArray = new const std::string *[NumStrings];
    for (Strings_t::iterator I = Strings.begin(), E = Strings.end(); I != E; I++)
      StringArray[I->second] = &I->first;
    for (unsigned I = 0; I != NumStrings; I++) {
      if (StringArray[I]->size())
        Body.push_back(StringArray[I]->data(), StringArray[I]->size());
      Body.push_back((char)0);
    }
    delete[] StringArray;
    Strings.clear();
  }
  // Name index.
  Body.push_back((uint32_t)NameIdx);

  // Variables, addresses, predicates.
  RegAlloc->buildHeader1(&Body);
  // Labels.
  Body.push_back((uint16_t)Labels.size());
  for (auto i = Labels.begin(), e = Labels.end(); i != e; ++i) {
    Body.push_back((uint32_t)i->NameIndex);
    Body.push_back((uint8_t)i->Kind);
    Body.push_back((uint8_t)0); // attr count
  }
  // Samplers, surfaces, vmes.
  RegAlloc->buildHeader2(&Body);
  // Inputs (if this is a kernel).
  if (isKernel()) {
    InputOffset = Body.size();
    buildInputs(FG->getHead(), RA, NeedRetIP);
  }
  // Code size
  Body.push_back((uint32_t)Code.size());
  // Entry (modify to correct value below).
  unsigned EntryOffset = Body.size();
  Body.push_back((uint32_t)0);
  if (!isKernel()) {
    // input_size (non-kernel func only)
    Body.push_back((uint8_t)0);
    // return_value_size (non-kernel func only)
    Body.push_back((uint8_t)0);
  }
  // Attributes
  Attributes.write<uint16_t>(&Body);
  // Modify entry field.
  uint32_t Entry = Body.size();
  Body.setData(EntryOffset, &Entry, sizeof(Entry));
  //--------------------------------------------------------------------
  // Build the function/kernel header.
  // Linkage (func only)
  if (!isKernel())
    Header.push_back((uint8_t)(FG->getHead()->getLinkage()
          == GlobalValue::ExternalLinkage ? 2 : 1));
  // Write the length-prefixed kernel/func name.
  Header.push_back((uint8_t)Name.size());
  Header.push_back(Name.data(), Name.size());
  // Offset field, unknown until later on
  OffsetOffset = Header.size();
  Header.push_back((uint32_t)0);
  // Size
  Header.push_back((uint32_t)(Body.size() + Code.size()));
  // Input offset, unknown until later on (kernel only)
  if (isKernel())
    Header.push_back((uint32_t)0);
  // variable_reloc_symtab
  Header.push_back((uint16_t)0);
  // function_reloc_symtab
  Header.push_back((uint16_t)0);
  // gen_binaries (kernel only)
  if (isKernel())
    Header.push_back((uint8_t)0);
  // RegAlloc lifetime ends here.
  RegAlloc = 0;
  AI.clear();
}

/***********************************************************************
 * VisaFuncWriter::buildInputs : build vISA kernel input table
 */

void VisaFuncWriter::buildInputs(Function *F, GenXVisaRegAlloc *RA, bool NeedRetIP)
{
  // All arguments now have offsets, generate the vISA parameter block
  assert(F->arg_size() == KM.getNumArgs() && "Mismatch between metadata for kernel and number of args");
  // Num of non-skipping inputs.
  auto Size = KM.getNumNonSKippingInputs();
  if (NeedRetIP) ++Size;

  // Number of globals to be binded statically.
  std::vector<std::pair<GlobalVariable *, int32_t>> Bindings;
  Module *M = F->getParent();
  for (auto &GV : M->getGlobalList()) {
    int32_t Offset = 0;
    GV.getAttribute("genx_byte_offset")
        .getValueAsString()
        .getAsInteger(0, Offset);
    if (Offset > 0)
      Bindings.emplace_back(&GV, Offset);
  }
  Size += Bindings.size();

  Body.push_back(uint32_t(Size));
  // Each argument.
  unsigned Idx = 0;
  bool PatchImpArgOff = false;
  for (auto i = F->arg_begin(), e = F->arg_end(); i != e; ++i, ++Idx) {
    if (KM.shouldSkipArg(Idx))
      continue;
    Argument *Arg = &*i;
    GenXVisaRegAlloc::RegNum Reg = RA->getRegNumForValueUntyped(Arg);
    assert(Reg.Category != RegCategory::NONE);
    uint8_t Kind = (uint8_t)KM.getArgKind(Idx);
    Body.push_back(Kind); // kind
    Body.push_back((uint32_t)Reg.Num); // id
    if (!PatchImpArgOff) {
      Body.push_back((int16_t)KM.getArgOffset(Idx));
    }
    // Argument size in bytes.
    auto &DL = F->getParent()->getDataLayout();
    Type *Ty = Arg->getType();
    uint16_t NumBytes = Ty->isPointerTy() ? DL.getPointerTypeSize(Ty)
                                          : Ty->getPrimitiveSizeInBits() / 8U;
    Body.push_back(NumBytes);
  }
  // Add the special RetIP argument.
  if (NeedRetIP) {
    GenXVisaRegAlloc::RegNum Reg = RA->getRetIPArgument();
    Body.push_back(uint8_t(0));
    Body.push_back(uint32_t(Reg.Num)); // id
    Body.push_back(uint16_t(127 * GrfByteSize + 6 * 4)); // r127.6
    Body.push_back(uint16_t(64/8));
  }
  // Add pseudo-input for global variables with offset attribute.
  for (auto &Item : Bindings) {
    // TODO: sanity check. No overlap with other inputs.
    GlobalVariable *GV = Item.first;
    int32_t Offset = Item.second;
    assert(Offset > 0);
    GenXVisaRegAlloc::RegNum Reg = RA->getRegNumForValueUntyped(GV);
    unsigned ByteSize = GV->getValueType()->getPrimitiveSizeInBits() / 8U;
    Body.push_back(uint8_t(KernelMetadata::IMP_PSEUDO_INPUT)); // kind
    Body.push_back(uint32_t(Reg.Num));  // id
    Body.push_back(uint16_t(Offset));   // offset
    Body.push_back(uint16_t(ByteSize)); // size
  }
}

// FIXME: We should use NM by default once code quality issues are addressed in
// vISA compiler.
static bool setNoMaskByDefault(Function *F) {
  for (auto &BB : F->getBasicBlockList())
    if (GotoJoin::isGotoBlock(&BB))
      return true;

  // Check if this is subroutine call.
  for (auto U : F->users()) {
    if (auto CI = dyn_cast<CallInst>(U)) {
       Function *G = CI->getParent()->getParent();
       if (setNoMaskByDefault(G))
         return true;
    }
  }

  return false;
}

/***********************************************************************
 * VisaFuncWriter::buildCode : build the code for this function and any subroutine
 *      (in the vISA sense) that it calls
 */
void VisaFuncWriter::buildCode(FunctionGroup *FG)
{
  for (auto fgi = FG->begin(), fge = FG->end(); fgi != fge; ++fgi) {
    Func = *fgi;
    // Set NoMask to 0x80 if there is SIMD CF in the function, 0 otherwise.
    // We could just set nomask on all non-SIMD CF masked instructions, but
    // that seems to make code worse in a few cases, possibly because the
    // finalizer register allocator treats nomask instructions differently.
    NoMask = setNoMaskByDefault(Func) ? 0x80 : 0;

    // Initial function label.
    Code.push_back((uint8_t)ISA_LABEL);
    Code.push_back((uint16_t)getOrCreateLabel(Func, Label::SUBROUTINE));

    // If a float control is specified, emit code to make that happen.
    // Float control contains rounding mode, denorm behaviour and single precision float mode (ALT
    // or IEEE)
    // Relevant bits are already set as defined for VISA control reg in header definition on enums
    if (Func->hasFnAttribute("CMFloatControl")) {
      uint32_t FloatControl = 0;
      Func->getFnAttribute("CMFloatControl").getValueAsString()
          .getAsInteger(0, FloatControl);

      // Clear current float control bits to known zero state
      buildControlRegUpdate(CR_Mask, true);

      // Set rounding mode to required state if that isn't zero
      FloatControl &= CR_Mask;
      if (FloatControl) {
        if (FG->getHead() == Func)
          DefaultFloatControl = FloatControl;
        buildControlRegUpdate(FloatControl, false);
      }
    }

    LastLabel = -1;
    // Only output a label for the initial basic block if it is used from
    // somewhere else.
    bool NeedsLabel = !Func->front().use_empty();
    for (Function::iterator fi = Func->begin(), fe = Func->end(); fi != fe; ++fi) {
      BasicBlock *BB = &*fi;
      if (!NeedsLabel && BB != &Func->front()) {
        NeedsLabel = !BB->getSinglePredecessor();
        if (!NeedsLabel)
          NeedsLabel = GotoJoin::isJoinLabel(BB);
      }
      if (NeedsLabel)
        writeLabel(BB);
      NeedsLabel = !buildBasicBlock(BB);
    }
    if (!BBRefs.empty())
      dbgs() << "In " << Func->getName() << "\n";
    for (auto i = BBRefs.begin(), e = BBRefs.end(); i != e; ++i)
      dbgs() << "unpatched forward reference: " << i->first->getName() << "\n";
    assert(BBRefs.empty() && "leftover unpatched forward references");
    clearLoops();
  }
}

/***********************************************************************
 * writeLabel : write a label for a basic block or join
 */
void VisaFuncWriter::writeLabel(Value *BB)
{
  if (LastLabel >= 0) {
    // There has been no code since the last label, so use the same label
    // for this basic block.
    setLabel(BB, LastLabel);
  } else {
    // Need a new label.
    LastLabel = getOrCreateLabel(BB, Label::BLOCK);
    Code.push_back((uint8_t)ISA_LABEL);
    Code.push_back((uint16_t)LastLabel);
  }
  // Patch forward references to the label.
  auto i = BBRefs.find(BB);
  if (i != BBRefs.end()) {
    uint16_t Num = LastLabel;
    auto Refs = &i->second;
    for (auto j = Refs->begin(), je = Refs->end(); j != je; ++j)
      Code.setData(*j, &Num, sizeof(Num));
    BBRefs.erase(i);
  }
}

/***********************************************************************
 * VisaFuncWriter::getKernelAttrsFromMetadata : set slm size from kernel
 *      metadata
 */
void VisaFuncWriter::getKernelAttrsFromMetadata()
{
  unsigned Val = KM.getSLMSize();
  if (Val) {
    // Compute the slm size in KB and roundup to power of 2.
    Val = alignTo(Val, 1024) / 1024;
    if (!isPowerOf2_64(Val))
      Val = NextPowerOf2(Val);
    if (Val > 64)
      report_fatal_error("slm size must not exceed 64KB");
    else {
      // For pre-SKL, valid values are {0, 4, 8, 16, 32, 64}.
      // For SKL+, valid values are {0, 1, 2, 4, 8, 16, 32, 64}.
      // FIXME: remove the following line for SKL+.
      Val = (Val < 4) ? 4 : Val;
      uint8_t SLMSize = static_cast<uint8_t>(Val);
      addAttribute("SLMSize", Twine(SLMSize));
    }
  }
}

/***********************************************************************
 * getBBRef : get reference to basic block for branch instruction
 *
 * If this is a forward reference, so the basic block does not yet have a
 * label, then this stores the about-to-be-written location in the BBRef map,
 * so it can be patched later when we reach the referenced basic block, and
 * just returns 0 for the caller to write into the code.
 *
 * We patch basic block forward references, rather than allocating a label
 * number to a basic block when we first see a branch to it, so that we can
 * merge adjacent labels as we go.
 */
unsigned VisaFuncWriter::getBBRef(Value *BB)
{
  int Num = getLabel(BB);
  if (Num >= 0) {
    // This is a backward reference, so the BB already has a label number.
    return Num;
  }
  // This is a forward reference, so push the location of the reference for
  // patching up later.
  BBRefs[BB].push_back(Code.size());
  return 0;
}

/***********************************************************************
 * getLabel : get label number for a Function or BasicBlock
 *
 * Return:  label number, -1 if none found
 */
int VisaFuncWriter::getLabel(Value *V)
{
  std::map<Value *, unsigned>::iterator i = LabelMap.find(V);
  if (i != LabelMap.end())
    return i->second;
  return -1;
}

/***********************************************************************
 * setLabel : set the label number for a Function or BasicBlock
 */
void VisaFuncWriter::setLabel(Value *V, unsigned Num)
{
  LabelMap[V] = Num;
}

/***********************************************************************
 * getOrCreateLabel : get/create label number for a Function or BasicBlock
 */
unsigned VisaFuncWriter::getOrCreateLabel(Value *V, int Kind)
{
  int Num = getLabel(V);
  if (Num >= 0)
    return Num;
  Num = Labels.size();
  setLabel(V, Num);

  // Replicate the functionality of the old compiler and make the first label for a function
  // contain the name (makes sure the function label is unique)
  // It's not clear this is strictly necessary any more (but doesn't do any harm and may even make
  // reading the intermediate forms easier)
  if (Kind == Label::SUBROUTINE) {
    StringRef N = KM.getName();
    std::string NameBuf;
    if (V != FG->getHead()) {
      // This is a subroutine, not the kernel/function at the head of the
      // FunctionGroup. Use the name of the subroutine.
      N = V->getName();
    } else {
      // For a kernel/function name, fix illegal characters. The jitter uses
      // the same name for the label in the .asm file, and aubload does not
      // like the illegal characters.
      NameBuf = N.str();
      for (unsigned i = 0, e = NameBuf.size(); i != e; ++i) {
        int ch = NameBuf[i];
        if (!isalnum(ch) && ch != '_')
          NameBuf[i] = '_';
      }
      N = NameBuf;
    }
    Labels.push_back(Label(getStringIdx((Twine(N)
                                       + Twine("_BB_")
                                       + Twine(LabelSeq++)).str()), Kind));
  } else if (Kind == Label::BLOCK) {
    Labels.push_back(Label(getStringIdx((Twine("BB_")
                                       + Twine(LabelSeq++)).str()), Kind));
  } else if (Kind == Label::FASTCOMPOSITE) {
    assert(isa<Function>(V));
    auto F = cast<Function>(V);
    Attribute A = F->getFnAttribute("CMCallable");
    StringRef N = A.getValueAsString();
    Labels.push_back(Label(getStringIdx(Twine(N).str()), Kind));
  } else {
    StringRef N = V->getName();
    Labels.push_back(Label(getStringIdx((Twine("_") + Twine(N) + Twine("_") 
                                        + Twine(LabelSeq++)).str()), Kind));
  }
  return Num;
}

/***********************************************************************
 * buildBasicBlock : build code for one basic block
 *
 * Return:  true if fell through to successor
 */
bool VisaFuncWriter::buildBasicBlock(BasicBlock *BB)
{
  for (BasicBlock::iterator bi = BB->begin(), be = BB->end(); bi != be; ++bi) {
    Instruction *Inst = &*bi;
    if (auto TI = dyn_cast<TerminatorInst>(Inst)) {
      // Before the terminator inst of a basic block, if there is a single
      // successor and it is the header of a loop, for any vector of at least
      // four GRFs with a phi node where our incoming value is undef, insert a
      // lifetime.start here.
      if (TI->getNumSuccessors() == 1) {
        auto Succ = TI->getSuccessor(0);
        if (getLoops(Succ->getParent())->isLoopHeader(Succ)) {
          for (auto si = Succ->begin(); ; ++si) {
            auto Phi = dyn_cast<PHINode>(&*si);
            if (!Phi)
              break;
            if (Phi->getType()->getPrimitiveSizeInBits() >= 256 * 4
                && isa<UndefValue>(Phi->getIncomingValue(
                      Phi->getBasicBlockIndex(BB))))
              writeLifetimeStart(Phi);
          }
        }
      }
    }
    // Build the instruction.
    if (!Baling->isBaled(Inst))
      if (buildInstruction(Inst))
        return true;
  }
  return false;
}

/***********************************************************************
 * buildInstruction : build code for one Instruction
 *
 * Return:  true if it was a terminator that fell through to successor
 */
bool VisaFuncWriter::buildInstruction(Instruction *Inst)
{
  // Make the source location pending, so it is output as vISA FILE and LOC
  // instructions next time an opcode is written.
  const DebugLoc &DL = Inst->getDebugLoc();
  if (DL) {
    StringRef Filename = DL->getFilename();
    if (Filename != "") {
      PendingFilename = Filename;
      PendingDirectory = DL->getDirectory();
    }
    PendingLine = DL.getLine();
  }
  // Process the bale that this is the head instruction of.
  BaleInfo BI = Baling->getBaleInfo(Inst);

  DstOpndDesc DstDesc;
  if (BI.Type == BaleInfo::GSTORE) {
    // Inst is a global variable store. It should be baled into a wrr
    // instruction.
    Bale B;
    Baling->buildBale(Inst, &B);
    // This is an identity bale; no code will be emitted.
    if (isIdentityBale(B))
      return false;

    assert(BI.isOperandBaled(0));
    DstDesc.GStore = Inst;
    Inst = cast<Instruction>(Inst->getOperand(0));
    BI = Baling->getBaleInfo(Inst);
  }

  if (BI.Type == BaleInfo::WRREGION || BI.Type == BaleInfo::WRPREDREGION
      || BI.Type == BaleInfo::WRPREDPREDREGION) {
    // Inst is a wrregion or wrpredregion or wrpredpredregion.
    DstDesc.WrRegion = Inst;
    DstDesc.WrRegionBI = BI;
    if (isa<UndefValue>(Inst->getOperand(0)) && !DstDesc.GStore) {
      // This is a wrregion, probably a partial write, to an undef value.
      // Write a lifetime start if appropriate to help the jitter's register
      // allocator.
      writeWrLifetimeStart(DstDesc.WrRegion);
    }
    // See if it bales in the instruction
    // that generates the subregion/element.  That is always operand 1.
    enum { OperandNum = 1 };
    if (!BI.isOperandBaled(OperandNum)) {
      if (BI.Type == BaleInfo::WRPREDREGION)
        buildLoneWrPredRegion(DstDesc.WrRegion, DstDesc.WrRegionBI);
      else
        buildLoneWrRegion(DstDesc);
      return false;
    }
    // Yes, source of wrregion is baled in.
    Inst = cast<Instruction>(DstDesc.WrRegion->getOperand(OperandNum));
    BI = Baling->getBaleInfo(Inst);
  }
  unsigned Mod = 0;
  if (BI.Type == BaleInfo::SATURATE) {
    // Inst is a fp saturate. See if it bales in the instruction that
    // generates the value to saturate. That is always operand 0. If
    // not, just treat the saturate as a normal intrinsic.
    if (BI.isOperandBaled(0)) {
      Mod = MOD_SAT;
      Inst = cast<Instruction>(Inst->getOperand(0));
      BI = Baling->getBaleInfo(Inst);
    } else
      BI.Type = BaleInfo::MAININST;
  }
  if (BI.Type == BaleInfo::CMPDST) {
    // Dst of sel instruction is baled in.
    Inst = cast<Instruction>(Inst->getOperand(0));
    assert(isa<CmpInst>(Inst) && "Only bale sel into a cmp instruction");
    BI = Baling->getBaleInfo(Inst);
  }
  switch (BI.Type) {
    case BaleInfo::RDREGION:
    case BaleInfo::ABSMOD:
    case BaleInfo::NEGMOD:
    case BaleInfo::NOTMOD:
      // This is a rdregion or modifier not baled in to a main instruction
      // (but possibly baled in to a wrregion or sat modifier).
      buildLoneOperand(Inst, BI, Mod, DstDesc);
      return false;
  }
  assert(BI.Type == BaleInfo::MAININST || BI.Type == BaleInfo::NOTP
      || BI.Type == BaleInfo::ZEXT || BI.Type == BaleInfo::SEXT);
  return buildMainInst(Inst, BI, Mod, DstDesc);
}

/***********************************************************************
 * buildLoneWrRegion : build a lone wrregion
 */
void VisaFuncWriter::buildLoneWrRegion(const DstOpndDesc &DstDesc) {
  enum { OperandNum = 1 };
  Value *Input = DstDesc.WrRegion->getOperand(OperandNum);
  if (isa<UndefValue>(Input))
    return; // No code if input is undef
  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(Input->getType()))
    ExecSize = VT->getNumElements();
  writeOpcode(ISA_MOV); // opcode
  writeExecSizeFromWrRegion(ExecSize, DstDesc); // execution size
  writePredFromWrRegion(DstDesc); // predication
  // Give dest and source the same signedness for byte mov.
  auto Signed = writeDestination(Input, DONTCARESIGNED, 0, DstDesc); // destination
  writeSource(Input, Signed, false, 0); // source 0
}

/***********************************************************************
* buildLoneWrPredRegion : build a lone wrpredregion
*/
void VisaFuncWriter::buildLoneWrPredRegion(Instruction *Inst, BaleInfo BI)
{
  enum { OperandNum = 1 };
  Value *Input = Inst->getOperand(OperandNum);
  assert(isa<Constant>(Input));
  auto C = dyn_cast<Constant>(Input);
  unsigned Size = C->getType()->getPrimitiveSizeInBits();
  writeOpcode(ISA_SETP);
  writeExecSizeFromWrPredRegion(Size, Inst, true);
  writePredicateOperand(Inst);
  unsigned IntVal = getPredicateConstantAsInt(C);
  C = ConstantInt::get(Type::getIntNTy(Inst->getContext(), std::max(Size, 8U)), IntVal);
  writeImmediateOperand(C, UNSIGNED);
}

/***********************************************************************
 * buildLoneOperand : build a rdregion or modifier that is not baled in to
 *                    a main instruction
 *
 * Enter:   Inst = the rdregion or modifier instruction
 *          BI = BaleInfo for Inst
 *          Mod = modifier for destination
 *          WrRegion = 0 else wrregion for destination
 *          WrRegionBI = BaleInfo for WrRegion (possibly baling in
 *              variable index add)
 */
void VisaFuncWriter::buildLoneOperand(Instruction *Inst, BaleInfo BI,
                                      unsigned Mod, const DstOpndDesc &DstDesc) {
  Instruction *WrRegion = DstDesc.WrRegion;
  BaleInfo WrRegionBI = DstDesc.WrRegionBI;

  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(Inst->getType()))
    ExecSize = VT->getNumElements();
  unsigned Opcode = ISA_MOV;
  bool Baled = true;
  // Default source from Inst
  Value *Src = Inst;
  if (BI.Type == BaleInfo::NOTMOD) {
    // A lone "not" is implemented as a not instruction, rather than a mov with
    // a not modifier. A mov only allows an arithmetic modifier.
    Opcode = ISA_NOT;
    Baled = BI.isOperandBaled(0);
    // In this case the src is actually operand 0 of the noti intrinsic
    Src = Inst->getOperand(0);
  }
  else if (BI.Type == BaleInfo::RDREGION && !WrRegion && !Mod) {
    GenXVisaRegAlloc::RegNum DstReg = RegAlloc->getRegNumForValue(Inst, DONTCARESIGNED);
    if (DstReg.Category == RegCategory::SURFACE
        || DstReg.Category == RegCategory::SAMPLER
        || DstReg.Category == RegCategory::VME) {
      Opcode = ISA_MOVS;
    }
  }
  writeOpcode(Opcode); // opcode
  writeExecSizeFromWrRegion(ExecSize, DstDesc); // execution size
  if (Opcode != ISA_MOVS) {
    writePredFromWrRegion(DstDesc); // predication
  }
  // Give dest and source the same signedness for byte mov.
  auto Signed = DONTCARESIGNED;
  // destination
  Signed = writeDestination(Inst, Signed, Mod, DstDesc);

  // source
  if ((Mod & MOD_SAT) != 0 && Inst->getType()->getScalarType()->isIntegerTy()
      && isIntegerSat(Inst->user_back()))
    Signed = getISatSrcSign(Inst->user_back());
  writeSource(Src, Signed, Baled, 0); // source
}

/***********************************************************************
 * buildMainInst : build a main instruction
 *
 * Enter:   Inst = the main instruction
 *          BI = BaleInfo for Inst
 *          Mod = modifier bits for destination
 *          WrRegion = 0 else wrregion for destination
 *          WrRegionBI = BaleInfo for WrRegion (possibly baling in
 *              variable index add)
 *
 * Return:  true if terminator inst that falls through to following block
 */
bool VisaFuncWriter::buildMainInst(Instruction *Inst, BaleInfo BI, unsigned Mod,
                                   const DstOpndDesc &DstDesc) {
  if (PHINode *Phi = dyn_cast<PHINode>(Inst))
    buildPhiNode(Phi);
  else if (ReturnInst *RI = dyn_cast<ReturnInst>(Inst))
    buildRet(RI);
  else if (BranchInst *BR = dyn_cast<BranchInst>(Inst))
    return buildBranch(BR);
  else if (CmpInst *Cmp = dyn_cast<CmpInst>(Inst))
    buildCmp(Cmp, BI, DstDesc);
  else if (BinaryOperator *BO = dyn_cast<BinaryOperator>(Inst)) {
    if (!BO->getType()->getScalarType()->isIntegerTy(1))
      buildBinaryOperator(BO, BI, Mod, DstDesc);
    else {
      assert(!Mod && !DstDesc.WrRegion && !BI.isOperandBaled(0) &&
             !BI.isOperandBaled(1));
      buildBoolBinaryOperator(BO);
    }
  } else if (isa<ExtractValueInst>(Inst))
    ; // no code generated
  else if (isa<InsertValueInst>(Inst))
    ; // no code generated
  else if (BitCastInst *BCI = dyn_cast<BitCastInst>(Inst))
    buildBitCast(BCI, BI, Mod, DstDesc);
  else if (CastInst *CI = dyn_cast<CastInst>(Inst))
    buildCastInst(CI, BI, Mod, DstDesc);
  else if (auto SI = dyn_cast<SelectInst>(Inst))
    buildSelectInst(SI, BI, Mod, DstDesc);
  else if (auto LI = dyn_cast<LoadInst>(Inst)) {
    (void)LI; // no code generated
  } else if (CallInst *CI = dyn_cast<CallInst>(Inst)) {
    Function *Callee = CI->getCalledFunction();
    if (!Callee)
      report_fatal_error("Indirect call not supported");
    unsigned IntrinID = Callee->getIntrinsicID();
    switch (IntrinID) {
      case Intrinsic::dbg_value:
      case Intrinsic::dbg_declare:
      case Intrinsic::genx_predefined_surface:
      case Intrinsic::genx_output:
        // ignore
        break;
      case Intrinsic::genx_simdcf_goto:
        // A goto that is not baled into a branch (via an extractvalue)
        buildGoto(CI, nullptr);
        break;
      case Intrinsic::genx_simdcf_join:
        // A join that is not baled into a branch (via an extractvalue)
        buildJoin(CI, nullptr);
        break;
      case Intrinsic::genx_convert:
        buildConvert(CI, BI, Mod, DstDesc);
        break;
      case Intrinsic::genx_convert_addr:
        buildConvertAddr(CI, BI, Mod, DstDesc);
        break;
      case Intrinsic::genx_constanti:
      case Intrinsic::genx_constantf:
      case Intrinsic::genx_constantpred:
        if (isa<UndefValue>(CI->getOperand(0)))
          return false; // Omit llvm.genx.constant with undef operand.
        if (!DstDesc.WrRegion &&
            RegAlloc->getRegNumForValueOrNull(CI).Category == RegCategory::NONE)
          return false; // Omit llvm.genx.constantpred that is EM or RM and so
                        // does not have a register allocated.
        // fall through...
      default:
        buildIntrinsic(CI, IntrinID, BI, Mod, DstDesc);
        break;
      case Intrinsic::not_intrinsic:
        assert(!Mod && !DstDesc.WrRegion &&
               "cannot bale subroutine call into anything");
        buildCall(CI);
        break;
    }
  } else if (isa<UnreachableInst>(Inst))
    ; // no code generated
  else
    assert(0 && "main inst not implemented");

  return false;
}

/***********************************************************************
 * buildPhiNode : build code for a phi node
 *
 * A phi node generates no code because coalescing has ensured that all
 * incomings and the result are in the same register. This function just
 * asserts that that is the case.
 */
void VisaFuncWriter::buildPhiNode(PHINode *Phi)
{
#ifndef NDEBUG
  for (unsigned i = 0, e = Phi->getNumIncomingValues(); i != e; ++i) {
    Value *Incoming = Phi->getIncomingValue(i);
    // This assert has to cope with the case that the phi node has no live range
    // because it is part of an indirected arg/retval in GenXArgIndirection, or
    // it is an EM/RM category.
    if (!isa<UndefValue>(Incoming))
      if (auto LR = Liveness->getLiveRangeOrNull(Incoming))
        if (LR->getCategory() < RegCategory::NUMREALCATEGORIES)
          assert(LR == Liveness->getLiveRangeOrNull(Phi) && "mismatched registers in phi node");
  }
#endif
}

/***********************************************************************
 * buildRet : build a ReturnInst
 */
void VisaFuncWriter::buildRet(ReturnInst *RI)
{
  uint32_t FloatControl = 0;
  auto F = RI->getFunction();
  F->getFnAttribute("CMFloatControl")
      .getValueAsString()
      .getAsInteger(0, FloatControl);
  FloatControl &= CR_Mask;
  if (FloatControl != DefaultFloatControl) {
    buildControlRegUpdate(CR_Mask, true);
    if (DefaultFloatControl)
      buildControlRegUpdate(DefaultFloatControl, false);
  }

  // Ignore non-void return that clang has allowed through.
  writeOpcode(ISA_RET);
  writeByte(0); // execution width
  writeShort(0); // predication
}

/***********************************************************************
 * buildCall : build a call to a subroutine in the same FunctionGroup
 */
void VisaFuncWriter::buildCall(CallInst *CI)
{
  Function *Callee = CI->getCalledFunction();
  unsigned LabelKind = Label::SUBROUTINE;
  if (Callee->hasFnAttribute("CMCallable")) {
    LabelKind = Label::FASTCOMPOSITE;
    HasCallable = true;
  }
  else
    assert(FG == FG->getParent()->getGroup(Callee) &&
           "unexpected call to outside FunctionGroup");

  // Check whether the called function has a predicate arg that is EM.
  int EMOperandNum = -1;
  for (auto ai = Callee->arg_begin(), ae = Callee->arg_end(); ai != ae; ++ai) {
    auto Arg = &*ai;
    if (!Arg->getType()->getScalarType()->isIntegerTy(1))
      continue;
    if (Liveness->getLiveRange(Arg)->getCategory() == RegCategory::EM) {
      EMOperandNum = Arg->getArgNo();
      break;
    }
  }
  if (EMOperandNum < 0) {
    // Non-predicated call.
    writeOpcode(ISA_CALL); // opcode
    writeByte(NoMask); // exec_size is 1 with nomask
    writeShort(0); // predication
    writeShort(getOrCreateLabel(Callee, LabelKind));
  } else {
    // Predicated call.
    writeOpcode(ISA_CALL); // opcode
    writeByte(Log2_32(CI->getArgOperand(EMOperandNum)->getType()
          ->getVectorNumElements())); // exec_size
    writePred(CI, BaleInfo(), EMOperandNum);
    writeShort(getOrCreateLabel(Callee, LabelKind));
  }
}

/***********************************************************************
 * buildBranch : build a conditional or unconditional branch
 *
 * Return:  true if fell through to successor
 */
bool VisaFuncWriter::buildBranch(BranchInst *Branch)
{
  BasicBlock *Next = Branch->getParent()->getNextNode();
  if (Branch->isUnconditional()) {
    // Unconditional branch
    if (Branch->getOperand(0) == Next)
      return true; // fall through to successor
    writeOpcode(ISA_JMP); // opcode
    writeByte(0); // exec_size is 1
    writeShort(0); // predication
    writeShort(getBBRef(Branch->getSuccessor(0)));
    return false;
  }
  // Conditional branch.
  // First check if it is a baled in goto/join, via an extractvalue.
  auto BI = Baling->getBaleInfo(Branch);
  if (BI.isOperandBaled(0/*condition*/)) {
    if (auto Extract = dyn_cast<ExtractValueInst>(Branch->getCondition())) {
      auto GotoJoin = cast<CallInst>(Extract->getAggregateOperand());
      if (getIntrinsicID(GotoJoin) == Intrinsic::genx_simdcf_goto)
        buildGoto(GotoJoin, Branch);
      else {
        assert(GotoJoin::isValidJoin(GotoJoin) && "extra unexpected code in join block");
        buildJoin(GotoJoin, Branch);
      }
      return true;
    }
  }
  // Normal conditional branch.
  unsigned PredField, MaskCtrl;
  Value *Pred = getPredicateOperand(Branch, 0/*OperandNum*/,
      BI, &PredField, &MaskCtrl);
  assert(!isa<VectorType>(Branch->getCondition()->getType()) && "branch must have scalar condition");
  GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValue(Pred, DONTCARESIGNED);
  assert(Reg.Category == RegCategory::PREDICATE);
  BasicBlock *True = Branch->getSuccessor(0);
  BasicBlock *False = Branch->getSuccessor(1);
  if (True == Next) {
    PredField ^= 0x8000; // invert bit in predicate field
    True = False;
    False = Next;
  }
  // Write the conditional branch.
  writeOpcode(ISA_JMP); // opcode
  writeByte(MaskCtrl); // mask ctrl field, and exec size 1
  writeShort(Reg.Num | PredField); // predication
  writeShort(getBBRef(True));
  // If the other successor is not the next block, write an unconditional
  // jmp to that.
  if (False == Next)
    return true; // fall through to successor
  writeOpcode(ISA_JMP); // opcode
  writeByte(0); // exec_size is 1
  writeShort(0); // predication
  writeShort(getBBRef(False));
  return false;
}

/***********************************************************************
 * buildGoto : build a goto
 *
 * Enter:   Goto = goto instruction that is baled into an extractvalue of
 *                 field 2 (the !any(EM) value), that is baled into Branch
 *          Branch = branch instruction, 0 if this is a goto that is not
 *                   baled into a branch, which happens when the goto is
 *                   followed by a join point so the goto's JIP points there,
 *                   and LLVM changes the resulting conditional branch with
 *                   both successors the same into an unconditional branch
 */
void VisaFuncWriter::buildGoto(CallInst *Goto, BranchInst *Branch)
{
  // GenXSimdCFConformance and GenXTidyControlFlow ensure that we have either
  // 1. a forward goto, where the false successor is fallthrough; or
  // 2. a backward goto, where the UIP (the join whose RM the goto updates)
  //    and the true successor are both fallthrough, and the false successor
  //    is the top of the loop.
  // (1) generates a vISA forward goto, but the condition has the wrong sense
  // so we need to invert it.
  // (2) generates a vISA backward goto.
  Value *BranchTarget = nullptr;
  unsigned PredInvert = 0;
  if (!Branch || Branch->getSuccessor(1) == Branch->getParent()->getNextNode()) {
    // Forward goto.  Find the join.
    auto Join = GotoJoin::findJoin(Goto);
    assert(Join && "join not found");
    BranchTarget = Join;
    PredInvert = 0x8000;
  } else {
    assert(Branch->getSuccessor(0) == Branch->getParent()->getNextNode() && "bad goto structure");
    // Backward branch.
    BranchTarget = Branch->getSuccessor(1);
  }
  // Get the condition.
  unsigned PredField, Mask;
  Value *Pred = getPredicateOperand(Goto, 2/*OperandNum*/,
      Baling->getBaleInfo(Goto), &PredField, &Mask);
  assert(!Mask && "cannot have rdpredregion baled into goto");
  if (auto C = dyn_cast<Constant>(Pred)) {
    (void)C;
    if (PredInvert)
      assert(C->isNullValue() &&
             "predication operand must be constant 0 or not constant");
    else
      assert(C->isAllOnesValue() &&
             "predication operand must be constant 1 or not constant");
  } else {
    PredField ^= PredInvert;
    GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValueOrNull(Pred);
    assert(Reg.Category == RegCategory::PREDICATE);
    PredField |= Reg.Num;
  }
  // Generate the vISA instruction.
  writeOpcode(ISA_GOTO); // opcode
  writeByte(llvm::log2(Pred->getType()->getVectorNumElements())); // exec size
  writeShort(PredField); // predication
  writeShort(getBBRef(BranchTarget));
}

/***********************************************************************
 * buildJoin : build a join
 *
 * Enter:   Join = join instruction that is baled into an extractvalue of
 *                 field 1 (the !any(EM) value), that is baled into Branch,
 *                 if Branch is non-zero
 *          Branch = branch instruction, or 0 for a join that is not baled
 *                   in to a branch because it always ends up with at least
 *                   one channel enabled
 */
void VisaFuncWriter::buildJoin(CallInst *Join, BranchInst *Branch)
{
  // A join needs a label. (If the join is at the start of its block, then this
  // gets merged into the block label.)
  writeLabel(Join);
  // There is no join instruction in vISA -- the finalizer derives it by
  // looking for gotos targeting the basic block's label.
}

/***********************************************************************
 * buildCmp : build code for a compare
 *
 * Enter:   Cmp = the compare instruction
 *          BI = BaleInfo for Cmp
 *          WrRegion = 0 else wrpredregion, wrpredpredregion, or wrregion for
 *          destination
 */
void VisaFuncWriter::buildCmp(CmpInst *Cmp, BaleInfo BI,
                              const DstOpndDesc &DstDesc) {
  assert((!DstDesc.WrRegion || Cmp->getType()->getPrimitiveSizeInBits() != 4 ||
          Cmp->getOperand(0)
                  ->getType()
                  ->getScalarType()
                  ->getPrimitiveSizeInBits() == 64) &&
         "write predicate size 4 only allowed for double/longlong type");
  Signedness Signed = DONTCARESIGNED;
  unsigned RelOp;
  switch (Cmp->getPredicate()) {
    case CmpInst::FCMP_OEQ:
    case CmpInst::FCMP_UEQ:
    case CmpInst::ICMP_EQ:
      RelOp = EQ;
      break;
    case CmpInst::FCMP_ONE:
    case CmpInst::FCMP_UNE:
    case CmpInst::ICMP_NE:
      RelOp = NE;
      break;
    case CmpInst::FCMP_OGT:
    case CmpInst::FCMP_UGT:
      RelOp = GT;
      break;
    case CmpInst::ICMP_UGT:
      RelOp = GT;
      Signed = UNSIGNED;
      break;
    case CmpInst::ICMP_SGT:
      RelOp = GT;
      Signed = SIGNED;
      break;
    case CmpInst::FCMP_OGE:
    case CmpInst::FCMP_UGE:
      RelOp = GE;
      break;
    case CmpInst::ICMP_UGE:
      RelOp = GE;
      Signed = UNSIGNED;
      break;
    case CmpInst::ICMP_SGE:
      RelOp = GE;
      Signed = SIGNED;
      break;
    case CmpInst::FCMP_OLT:
    case CmpInst::FCMP_ULT:
      RelOp = LT;
      break;
    case CmpInst::ICMP_ULT:
      RelOp = LT;
      Signed = UNSIGNED;
      break;
    case CmpInst::ICMP_SLT:
      RelOp = LT;
      Signed = SIGNED;
      break;
    case CmpInst::FCMP_OLE:
    case CmpInst::FCMP_ULE:
      RelOp = LE;
      break;
    case CmpInst::ICMP_ULE:
      RelOp = LE;
      Signed = UNSIGNED;
      break;
    case CmpInst::ICMP_SLE:
      RelOp = LE;
      Signed = SIGNED;
      break;
    default:
      assert(0 && "unknown predicate");
      RelOp = EQ;
      break;
  }

  // Check if this is to write to a predicate desination or a GRF desination.
  bool WriteToPred = true;
  if (Cmp->hasOneUse()) {
    Instruction *UI = Cmp->user_back();
    BaleInfo UserBI = Baling->getBaleInfo(UI);
    if (UserBI.Type == BaleInfo::CMPDST)
      WriteToPred = false;
  }

  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(Cmp->getType()))
    ExecSize = VT->getNumElements();
  writeOpcode(ISA_CMP); // opcode
  if (WriteToPred)
    writeExecSizeFromWrPredRegion(ExecSize, DstDesc.WrRegion, false); // execution size
  else
    writeExecSizeFromWrRegion(ExecSize, DstDesc);
  writeByte(RelOp); // rel_op

  // Write to destionation.
  if (WriteToPred) {
    // destination is a predicate opnd.
    writePredicateOperand(DstDesc.WrRegion ? DstDesc.WrRegion : Cmp);
  } else {
    // destination is a general opnd.
    Value *Val = DstDesc.WrRegion ? DstDesc.WrRegion : Cmp->user_back();
    writeDestination(Val, Signed, 0, DstDesc);
  }
  Signedness Src0Signed = writeSourceOperand(Cmp, Signed, 0, BI); // source 0
  writeSourceOperand(Cmp, Src0Signed, 1, BI);                     // source 1
}

/***********************************************************************
 * buildBinaryOperator : build code for a binary operator
 *
 * Enter:   BO = the BinaryOperator
 *          BI = BaleInfo for BO
 *          Mod = modifier bits for destination
 *          WrRegion = 0 else wrregion for destination
 *          WrRegionBI = BaleInfo for WrRegion
 */
void VisaFuncWriter::buildBinaryOperator(BinaryOperator *BO, BaleInfo BI,
                                         unsigned Mod,
                                         const DstOpndDesc &DstDesc) {
  int Opcode = 0;
  Signedness DstSigned = SIGNED;
  Signedness SrcSigned = SIGNED;
  unsigned Mod1 = 0;
  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(BO->getType()))
    ExecSize = VT->getNumElements();
  switch (BO->getOpcode()) {
    case Instruction::Add: case Instruction::FAdd: Opcode = ISA_ADD; break;
    case Instruction::Sub: case Instruction::FSub:
      Opcode = ISA_ADD;
      Mod1 ^= MOD_NEG;
      break;
    case Instruction::Mul: case Instruction::FMul: Opcode = ISA_MUL; break;
    case Instruction::Shl: Opcode = ISA_SHL; break;
    case Instruction::AShr: Opcode = ISA_ASR; break;
    case Instruction::LShr:
      Opcode = ISA_SHR; DstSigned = SrcSigned = UNSIGNED; break;
    case Instruction::UDiv:
      Opcode = ISA_DIV; DstSigned = SrcSigned = UNSIGNED; break;
    case Instruction::SDiv: Opcode = ISA_DIV; break;
    case Instruction::FDiv: {
      Opcode = ISA_DIV;
      if (Constant *Op0 = dyn_cast<Constant>(BO->getOperand(0))) {
        if (Op0->getType()->isVectorTy())
          Op0 = Op0->getSplatValue();
        ConstantFP *CFP = dyn_cast_or_null<ConstantFP>(Op0);
        if (CFP && CFP->isExactlyValue(1.0))
          Opcode = ISA_INV;
      }
    } break;
    case Instruction::URem:
      Opcode = ISA_MOD; DstSigned = SrcSigned = UNSIGNED; break;
    case Instruction::SRem: case Instruction::FRem: Opcode = ISA_MOD; break;
    case Instruction::And: Opcode = ISA_AND; break;
    case Instruction::Or: Opcode = ISA_OR; break;
    case Instruction::Xor:
      Opcode = ISA_XOR;
      break;
    default:
      assert(0 && "buildBinaryOperator: unimplemented binary operator");
      break;
  }

  writeOpcode(Opcode); // opcode
  writeExecSizeFromWrRegion(ExecSize, DstDesc); // execution size
  writePredFromWrRegion(DstDesc); // predication
  writeDestination(BO, DstSigned, Mod, DstDesc); // destination

  if (Opcode == ISA_INV)
    writeSourceOperand(BO, SrcSigned, 1, BI, Mod1); // source 0
  else {
    writeSourceOperand(BO, SrcSigned, 0, BI); // source 0
    writeSourceOperand(BO, SrcSigned, 1, BI, Mod1); // source 1
  }
}

/***********************************************************************
 * buildSelectInst : build code for a select inst
 *
 * Enter:   SI = the SelectInst
 *          BI = BaleInfo for SI
 *          Mod = modifier bits for destination
 *          WrRegion = 0 else wrregion for destination (which is unpredicated)
 *          WrRegionBI = BaleInfo for WrRegion
 */
void VisaFuncWriter::buildSelectInst(SelectInst *SI, BaleInfo BI, unsigned Mod,
                                     const DstOpndDesc &DstDesc) {
  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(SI->getType()))
    ExecSize = VT->getNumElements();
  writeOpcode(ISA_SEL); // opcode
  writeExecSizeFromSelect(ExecSize, SI, BI); // execution size
  writePredFromSelect(SI, BI); // predication
  writeDestination(SI, DONTCARESIGNED, Mod, DstDesc); // destination
  writeSourceOperand(SI, DONTCARESIGNED, 1, BI); // source 0
  writeSourceOperand(SI, DONTCARESIGNED, 2, BI); // source 1
}

/***********************************************************************
 * buildBoolBinaryOperator : build code for a binary operator acting on
 *                           i1 or vector of i1
 *
 * Enter:   BO = the BinaryOperator
 */
void VisaFuncWriter::buildBoolBinaryOperator(BinaryOperator *BO)
{
  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(BO->getType()))
    ExecSize = VT->getNumElements();
  int Opcode = 0;
  switch (BO->getOpcode()) {
    case Instruction::And: Opcode = ISA_AND; break;
    case Instruction::Or: Opcode = ISA_OR; break;
    case Instruction::Xor:
      Opcode = ISA_XOR;
      if (isNot(BO))
        Opcode = ISA_NOT;
      break;
    default:
      assert(0 && "buildBoolBinaryOperator: unimplemented binary operator");
      break;
  }
  writeOpcode(Opcode); // opcode
  writeByte(llvm::log2(ExecSize) | NoMask); // execution size
  writeShort(0); // predication
  writePredicateOperand(BO); // destination
  writePredicateOperand(BO->getOperand(0)); // source 0
  if (Opcode != ISA_NOT)
    writePredicateOperand(BO->getOperand(1)); // source 1
}

/***********************************************************************
 * buildCastInst : build code for a cast (other than a bitcast)
 *
 * Enter:   CI = the CastInst
 *          BI = BaleInfo for CI
 *          Mod = modifier bits for destination
 *          WrRegion = 0 else wrregion for destination
 *          WrRegionBI = BaleInfo for WrRegion
 */
void VisaFuncWriter::buildCastInst(CastInst *CI, BaleInfo BI, unsigned Mod,
                                   const DstOpndDesc &DstDesc) {
  Signedness InSigned = DONTCARESIGNED;
  Signedness OutSigned = DONTCARESIGNED;
  switch (CI->getOpcode()) {
    case Instruction::UIToFP:
      InSigned = UNSIGNED;
      break;
    case Instruction::SIToFP:
      InSigned = SIGNED;
      break;
    case Instruction::FPToUI:
      OutSigned = UNSIGNED;
      break;
    case Instruction::FPToSI:
      OutSigned = SIGNED;
      break;
    case Instruction::ZExt:
      InSigned = UNSIGNED;
      break;
    case Instruction::SExt:
      InSigned = SIGNED;
      break;
    case Instruction::FPTrunc:
    case Instruction::FPExt:
      break;
    case Instruction::PtrToInt:
    case Instruction::IntToPtr:
      break;
    default:
      assert(0 && "buildCastInst: unimplemented cast");
      break;
  }

  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(CI->getType()))
    ExecSize = VT->getNumElements();
  writeOpcode(ISA_MOV); // opcode
  writeExecSizeFromWrRegion(ExecSize, DstDesc); // execution size
  writePredFromWrRegion(DstDesc); // predication
  // Give dest and source the same signedness for byte mov.
  auto Signed = writeDestination(CI, OutSigned, Mod, DstDesc); // destination
  if (InSigned == DONTCARESIGNED)
    InSigned = Signed;
  writeSourceOperand(CI, InSigned, 0, BI); // source 0
}

/***********************************************************************
 * buildBitCast : build a bitcast instruction
 *
 * A bitcast is usually not baled with anything. The only exception is a mask
 * packing may be baled into a region write. For a real bitcast, its source and
 * destination element types are typically different, so we use the bigger of
 * the two.
 *
 * For a bitcast inserted by GenXCoalescing to implement a phi node or
 * materialize a constant input to rdregion, source might be constant.
 * When materializing a constant input to rdregion, source might be undef.
 * (This will stop happening once we are able to constant propagate
 * through rdregion.)
 */
void VisaFuncWriter::buildBitCast(CastInst *CI, BaleInfo BI, unsigned Mod,
                                  const DstOpndDesc &DstDesc) {
  if (!isMaskPacking(CI))
    assert(!BI.Bits && !Mod && !DstDesc.WrRegion &&
           "non predicate bitcast should not be baled with anything");

  if (CI->getType()->getScalarType()->isIntegerTy(1)) {
    if (CI->getOperand(0)->getType()->getScalarType()->isIntegerTy(1)) {
      if (auto C = dyn_cast<Constant>(CI->getOperand(0))) {
        auto Reg = RegAlloc->getRegNumForValueOrNull(CI, DONTCARESIGNED);
        if (Reg.Category == RegCategory::NONE)
          return; // write to EM/RM value, ignore
        // We can move a constant predicate to a predicate register
        // using setp, if we get the constant predicate as a single int.
        unsigned IntVal = getPredicateConstantAsInt(C);
        unsigned Size = C->getType()->getPrimitiveSizeInBits();
        C = ConstantInt::get(Type::getIntNTy(CI->getContext(), std::max(Size, 8U)), IntVal);
        writeOpcode(ISA_SETP); // opcode
        writeByte(llvm::log2(Size) | 0x80); // exec size = number of bits, M1_NM
        writePredicateOperand(Reg); // destination
        writeImmediateOperand(C, UNSIGNED); // source
        return;
      }
      // There does not appear to be a vISA instruction to move predicate
      // to predicate. GenXCoalescing avoids this by moving in two steps
      // via a general register. So the only pred->pred bitcast that arrives here
      // should be one from GenXLowering, and it should have been copy coalesced
      // in GenXCoalescing.
      assert(RegAlloc->getRegNumForValue(CI, DONTCARESIGNED) ==
            RegAlloc->getRegNumForValue(CI->getOperand(0), DONTCARESIGNED)
          && "uncoalesced phi move of predicate");
      return;
    }
    // bitcast from scalar int to predicate
    writeOpcode(ISA_SETP); // opcode
    // exec size = number of bits, M1_NM
    writeByte(llvm::log2(CI->getType()->getPrimitiveSizeInBits()) | 0x80);
    writePredicateOperand(CI); // destination
    writeSourceOperand(CI, UNSIGNED, 0, BI); // source
    return;
  }
  if (isa<Constant>(CI->getOperand(0))) {
    if (isa<UndefValue>(CI->getOperand(0)))
      return; // undef source, generate no code
    // Source is constant.
    int ExecSize = 1;
    if (VectorType *VT = dyn_cast<VectorType>(CI->getType()))
      ExecSize = VT->getNumElements();
    writeOpcode(ISA_MOV); // opcode
    writeExecSizeFromWrRegion(ExecSize, DstDesc); // execution size
    writePredFromWrRegion(DstDesc); // predication
    // Give dest and source the same signedness for byte mov.
    auto Signed = writeDestination(CI, DONTCARESIGNED, Mod,
                                   DstDesc); // destination
    writeSourceOperand(CI, Signed, 0, BI); // source 0
    return;
  }
  if (CI->getOperand(0)->getType()->getScalarType()->isIntegerTy(1)) {
    // Bitcast from predicate to scalar int
    writeOpcode(ISA_MOV); // opcode
    writeByte(0); // exec size 1
    writeShort(0); // no predication
    writeDestination(CI, UNSIGNED, 0, DstDesc); // destination
    writePredicateOperand(CI->getOperand(0)); // source
    return;
  }

  // Real bitcast with possibly different types. Use whichever type has the
  // largest element size, so we minimize the number of channels used in the
  // move.
  Type *Ty = CI->getOperand(0)->getType();
  if (Ty->getScalarType()->getPrimitiveSizeInBits()
      < CI->getType()->getScalarType()->getPrimitiveSizeInBits())
    Ty = CI->getType();
  if (Liveness->isBitCastCoalesced(cast<BitCastInst>(CI)))
    return; // bitcast was coalesced away
  GenXVisaRegAlloc::RegNum DstReg = RegAlloc->getRegNumForValue(
      CI, DONTCARESIGNED, Ty);
  // Give dest and source the same signedness for byte mov.
  auto Signed = RegAlloc->getSigned(DstReg);
  GenXVisaRegAlloc::RegNum SrcReg = RegAlloc->getRegNumForValue(
      CI->getOperand(0), Signed, Ty);
  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(Ty))
    ExecSize = VT->getNumElements();
  assert(llvm::exactLog2(ExecSize) >= 0 && ExecSize <= 32 &&
         "illegal exec size in bitcast: should have been coalesced away");
  writeOpcode(ISA_MOV); // opcode
  writeByte(llvm::log2(ExecSize) | NoMask); // execution size
  writeShort(0); // predication
  // destination
  Region DestR(CI);
  writeRegion(&DestR, DstReg.Num, DONTCARESIGNED, 0/*Mod*/, true/*IsDest*/);
  // source
  Region SourceR(CI->getOperand(0));
  writeRegion(&SourceR, SrcReg.Num, Signed, 0/*Mod*/, false/*IsDest*/);
}

/***********************************************************************
 * getOriginalInstructionForSource : trace a source operand back through
 *     its bale (if any), given a starting instruction.
 *
 * Enter:   Inst = The instruction to start tracing from.
 *          BI = BaleInfo for Inst
 */
Instruction *VisaFuncWriter::getOriginalInstructionForSource(Instruction *Inst, BaleInfo BI)
{
  while (!isa<Constant>(Inst->getOperand(0)) && BI.isOperandBaled(0)) {
    Inst = cast<Instruction>(Inst->getOperand(0));
    BI = Baling->getBaleInfo(Inst);
  }

  return Inst;
}

/***********************************************************************
 * buildConvert : build code for a category conversion
 *
 * Enter:   CI = the CallInst
 *          BI = BaleInfo for CI
 *          Mod = modifier bits for destination
 *          WrRegion = 0 else wrregion for destination
 *          WrRegionBI = BaleInfo for WrRegion
 */
void VisaFuncWriter::buildConvert(CallInst *CI, BaleInfo BI, unsigned Mod,
                                  const DstOpndDesc &DstDesc) {
  GenXVisaRegAlloc::RegNum DstReg = RegAlloc->getRegNumForValue(
      CI, UNSIGNED);
  if (!isa<Constant>(CI->getOperand(0))) {
    Instruction *OrigInst = getOriginalInstructionForSource(CI, BI);
    GenXVisaRegAlloc::RegNum SrcReg =
        RegAlloc->getRegNumForValue(OrigInst->getOperand(0));
    (void)SrcReg;
    assert((SrcReg.Category != RegCategory::GENERAL ||
            DstReg.Category != RegCategory::GENERAL) &&
           "expected a category conversion");
  }
  if (DstReg.Category != RegCategory::ADDRESS) {
    // State copy.
    int ExecSize = 1;
    if (VectorType *VT = dyn_cast<VectorType>(CI->getType())) {
      ExecSize = VT->getNumElements();
    }
    writeOpcode(ISA_MOVS); // opcode
    writeByte(llvm::log2(ExecSize) | NoMask); // execution size
    writeDestination(CI, UNSIGNED, 0, DstDesc);
    writeSourceOperand(CI, UNSIGNED, 0, BI);
    return;
  }

  // Destination is address register.
  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(CI->getType())) {
    assert(0 && "vector of addresses not implemented");
    ExecSize = VT->getNumElements();
  }
  GenXVisaRegAlloc::RegNum SrcReg = RegAlloc->getRegNumForValue(CI->getOperand(0));
  assert(SrcReg.Category == RegCategory::ADDRESS); (void)SrcReg;
  // This is an address->address copy, inserted due to coalescing failure of
  // the address for an indirected arg in GenXArgIndirection.
  // (A conversion to address is handled in buildConvertAddr below.)
  // Write the addr_add instruction.
  writeOpcode(ISA_ADDR_ADD); // opcode
  writeByte(llvm::log2(ExecSize) | NoMask); // execution size
  writeAddressOperand(CI); // destination
  writeAddressOperand(CI->getOperand(0)); // source 0
  writeImmediateOperand(Constant::getNullValue(CI->getType()), UNSIGNED); // source 1
}

/***********************************************************************
 * buildConvertAddr : build code for conversion to address
 *
 * Enter:   CI = the CallInst
 *          BI = BaleInfo for CI
 *          Mod = modifier bits for destination
 *          WrRegion = 0 else wrregion for destination
 *          WrRegionBI = BaleInfo for WrRegion
 */
void VisaFuncWriter::buildConvertAddr(CallInst *CI, BaleInfo BI, unsigned Mod,
                                      const DstOpndDesc &DstDesc) {
  assert(!DstDesc.WrRegion);
  Value *Base = Liveness->getAddressBase(CI);
  int ExecSize = 1;
  if (VectorType *VT = dyn_cast<VectorType>(CI->getType()))
    ExecSize = VT->getNumElements();
  // If the offset is less aligned than the base register element type, then we
  // need a different type.
  Type *OverrideTy = nullptr;
  Type *BaseTy = Base->getType();
  if (BaseTy->isPointerTy())
    BaseTy = BaseTy->getPointerElementType();
  unsigned ElementBytes =
      BaseTy->getScalarType()->getPrimitiveSizeInBits() >> 3;
  int Offset = cast<ConstantInt>(CI->getArgOperand(1))->getSExtValue();
  if ((ElementBytes - 1) & Offset) {
    OverrideTy = VectorType::get(Type::getInt8Ty(CI->getContext()),
                                 BaseTy->getVectorNumElements() * ElementBytes);
    ElementBytes = 1;
  }
  GenXVisaRegAlloc::RegNum BaseReg = RegAlloc->getRegNumForValue(Base,
      DONTCARESIGNED, OverrideTy);
  // Write the addr_add instruction.
  writeOpcode(ISA_ADDR_ADD); // opcode
  writeByte(llvm::log2(ExecSize) | NoMask); // execution size
  writeAddressOperand(CI); // destination
  // Write the src0 operand, which is the base register, with an offset from
  // the offset in the convert_address intrinsic.
  if (BaseReg.Category == RegCategory::SURFACE ||
      BaseReg.Category == RegCategory::SAMPLER) {
    writeByte(CLASS_STATE);
    writeByte(BaseReg.Category == RegCategory::SURFACE ? 0
            : BaseReg.Category == RegCategory::SAMPLER ? 1 : 2);
    writeShort(BaseReg.Num);
    writeByte(Offset >> 2);  // convert offset from bytes to dwords
  } else {
    writeByte(CLASS_GENERAL); // tag+modifiers
    writeInt(BaseReg.Num); // id (register number)
    writeByte(Offset >> llvm::log2(GrfByteSize)); // row offset
    writeByte((Offset & (GrfByteSize - 1)) >> Log2_32(ElementBytes)); // col offset (in elements)
    writeByte(0x21); // region (vstride and width)
    writeByte(0x01); // region (stride)
  }
  // Write the src1 operand, which is the input to the bitcast.
  writeSourceOperand(CI, UNSIGNED, 0/*OperandNum*/, BI); // source 1
}

/***********************************************************************
 * buildIntrinsic : build code for an intrinsic
 *
 * Enter:   CI = the CallInst
 *          IntrinID = intrinsic ID
 *          BI = BaleInfo for the instruction
 *          Mod = modifier bits for destination
 *          WrRegion = 0 else wrregion for destination
 *          WrRegionBI = BaleInfo for WrRegion
 */
void VisaFuncWriter::buildIntrinsic(CallInst *CI, unsigned IntrinID,
                                    BaleInfo BI, unsigned Mod,
                                    const DstOpndDesc &DstDesc) {
  GenXIntrinsicInfo Info(IntrinID);
  assert(Info.isNotNull() && "intrinsic not found");
  unsigned MaxRawOperands = Info.getTrailingNullZoneStart(CI);
  unsigned ExecSize = 0;
  // See if there is a twoaddr operand that is undef, and the result has been
  // allocated a register. If so, we need a lifetime start instruction if this
  // is in a loop.
  const auto *descp = Info.getInstDesc();
  for (const auto *p = descp; *p; ++p) {
    GenXIntrinsicInfo::ArgInfo AI(*p);
    if (AI.getCategory() != GenXIntrinsicInfo::TWOADDR)
      continue;
    if (isa<UndefValue>(CI->getArgOperand(AI.getArgIdx()))
        && RegAlloc->getRegNumForValueOrNull(CI, DONTCARESIGNED).Category)
      writeSendLifetimeStart(CI);
    break;
  }
  // Iterate through the instruction description.
  for (const auto *p = descp; *p; ++p) {
    GenXIntrinsicInfo::ArgInfo AI(*p);
    unsigned Cat = AI.getCategory();
    if (Cat == GenXIntrinsicInfo::LITERAL) {
      // Opcode (or other literal byte)
      if (p == Info.getInstDesc())
        writeOpcode(AI.getLiteral());
      else
        Code.push_back((uint8_t)AI.getLiteral());
      continue;
    }
    // Get the arg or return value.
    Signedness Signed;
    switch (Cat) {
      case GenXIntrinsicInfo::BYTE: // constant operand written as byte
        {
          ConstantInt *Const = dyn_cast<ConstantInt>(CI->getArgOperand(AI.getArgIdx()));
          if (!Const) report_fatal_error("Incorrect args to intrinsic call");
          Code.push_back((uint8_t)Const->getSExtValue());
        }
        break;
      case GenXIntrinsicInfo::SHORT: // constant operand written as short
        {
          ConstantInt *Const = dyn_cast<ConstantInt>(CI->getArgOperand(AI.getArgIdx()));
          if (!Const) report_fatal_error("Incorrect args to intrinsic call");
          Code.push_back((uint16_t)Const->getSExtValue());
        }
        break;
      case GenXIntrinsicInfo::INT: // constant operand written as int
        {
          ConstantInt *Const = dyn_cast<ConstantInt>(CI->getArgOperand(AI.getArgIdx()));
          if (!Const) report_fatal_error("Incorrect args to intrinsic call");
          Code.push_back((uint32_t)Const->getSExtValue());
        }
        break;
      case GenXIntrinsicInfo::MEDIAHEIGHT: {
          // constant byte for media height that we need to infer from the
          // media width and the return type or final arg
          ConstantInt *Const = dyn_cast<ConstantInt>(CI->getArgOperand(AI.getArgIdx()));
          if (!Const) report_fatal_error("Incorrect args to intrinsic call");
          unsigned Width = Const->getZExtValue();
          if (Width == 0 || Width > 64) report_fatal_error("Invalid media width");
          unsigned RoundedWidth = 1 << llvm::log2(Width);
          if (RoundedWidth < Width)
            RoundedWidth *= 2;
          if (RoundedWidth < 4)
            RoundedWidth = 4;
          Type *DataType = CI->getType();
          if (DataType->isVoidTy())
            DataType = CI->getOperand(CI->getNumArgOperands() - 1)->getType();
          unsigned DataSize;
          if (VectorType *VT = dyn_cast<VectorType>(DataType))
            DataSize = VT->getElementType()->getPrimitiveSizeInBits() / 8
              * VT->getNumElements();
          else
            DataSize = DataType->getPrimitiveSizeInBits() / 8;
          if (DataSize <= RoundedWidth && DataSize >= Width)
            Code.push_back((uint8_t)1);
          else {
            if (DataSize % RoundedWidth) report_fatal_error("Invalid media width");
            Code.push_back((uint8_t)(DataSize / RoundedWidth));
          }
        }
        break;
      case GenXIntrinsicInfo::LOG2ELTSIZE: {
          // constant byte for log2 element size
          Value *Arg = CI;
          if (!AI.isRet())
            Arg = CI->getOperand(AI.getArgIdx());
          unsigned EltSize = Arg->getType()->getScalarType()
            ->getPrimitiveSizeInBits() / 8;
          Code.push_back((uint8_t)llvm::log2(EltSize));
        }
        break;
      case GenXIntrinsicInfo::LOG2OWORDS: {
          // constant byte for log2 number of owords
          Value *Arg = CI;
          if (!AI.isRet())
            Arg = CI->getOperand(AI.getArgIdx());
          VectorType *VT = dyn_cast<VectorType>(Arg->getType());
          if (!VT) report_fatal_error("Invalid number of owords");
          int DataSize = VT->getNumElements()
            * VT->getElementType()->getPrimitiveSizeInBits() / 8;
          DataSize = llvm::exactLog2(DataSize) - 4;
          if (DataSize < 0 || DataSize > 4)
            report_fatal_error("Invalid number of words");
          Code.push_back((uint8_t)DataSize);
        }
        break;
      case GenXIntrinsicInfo::LOG2OWORDS_PLUS_8: {
          // constant byte for log2 number of owords, plus 8
          Value *Arg = CI;
          if (!AI.isRet())
            Arg = CI->getOperand(AI.getArgIdx());
          VectorType *VT = dyn_cast<VectorType>(Arg->getType());
          if (!VT) report_fatal_error("Invalid number of owords");
          int DataSize = VT->getNumElements()
            * VT->getElementType()->getPrimitiveSizeInBits() / 8;
          DataSize = llvm::exactLog2(DataSize) - 4;
          if (DataSize < 0 || DataSize > 4)
            report_fatal_error("Invalid number of words");
          Code.push_back((uint8_t)(DataSize + 8));
        }
        break;
      case GenXIntrinsicInfo::NUMGRFS: {
          // constant byte for number of GRFs
          Value *Arg = CI;
          if (!AI.isRet())
            Arg = CI->getOperand(AI.getArgIdx());
          VectorType *VT = dyn_cast<VectorType>(Arg->getType());
          if (!VT) report_fatal_error("Invalid number of GRFs");
          int DataSize = VT->getNumElements()
            * VT->getElementType()->getPrimitiveSizeInBits() / 8;
          DataSize = (DataSize + (GrfByteSize - 1)) / GrfByteSize;
          Code.push_back((uint8_t)DataSize);
        }
        break;
      case GenXIntrinsicInfo::RAW:
        Signed = DONTCARESIGNED;
        if (AI.needsSigned())
          Signed = SIGNED;
        else if (AI.needsUnsigned())
          Signed = UNSIGNED;
        if (AI.isRet()) {
          assert(!Mod);
          writeRawDestination(CI, DstDesc, Signed);
        } else if ((unsigned)AI.getArgIdx() < MaxRawOperands)
          writeRawSourceOperand(CI, AI.getArgIdx(), BI, Signed);
        break;
      case GenXIntrinsicInfo::NULLRAW: // null raw operand
        Code.push_back((uint32_t)0);
        Code.push_back((uint16_t)0);
        break;
      case GenXIntrinsicInfo::GENERAL: {
          Signedness Signed = DONTCARESIGNED;
          if (AI.needsSigned())
            Signed = SIGNED;
          else if (AI.needsUnsigned())
            Signed = UNSIGNED;
          if (AI.isRet()) {
            if (AI.getSaturation() == GenXIntrinsicInfo::SATURATION_SATURATE)
              Mod |= MOD_SAT;
            writeDestination(CI, Signed, Mod, DstDesc);
          } else {
            unsigned MaxWidth = 16;
            if (AI.getRestriction() == GenXIntrinsicInfo::TWICEWIDTH) {
              // For a TWICEWIDTH operand, do not allow width bigger than the
              // execution size.
              MaxWidth = CI->getType()->getVectorNumElements();
            }
            writeSourceOperand(CI, Signed, AI.getArgIdx(), BI, 0, MaxWidth);
          }
        }
        break;
      case GenXIntrinsicInfo::ADDRESS:
        if (AI.isRet())
          writeAddressOperand(CI);
        else
          writeAddressOperand(CI->getArgOperand(AI.getArgIdx()));
        break;
      case GenXIntrinsicInfo::PREDICATE:
        if (AI.isRet())
          writePredicateOperand(CI);
        else
          writePredicateOperand(CI->getArgOperand(AI.getArgIdx()));
        break;
      case GenXIntrinsicInfo::SAMPLER: {
          GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValue(CI->getArgOperand(AI.getArgIdx()));
          assert(Reg.Category == RegCategory::SAMPLER && "Expected sampler register");
          Code.push_back((uint8_t)Reg.Num);
        }
        break;
      case GenXIntrinsicInfo::SURFACE:
        {
          llvm::Value *Arg = CI->getArgOperand(AI.getArgIdx());
          int Index = getReservedSurfaceIndex(Arg);
          if (Index < 0) {
            GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValue(Arg);
            assert(Reg.Category == RegCategory::SURFACE && "Expected surface register");
            Index = Reg.Num;
          }
          Code.push_back((uint8_t)Index);
        }
        break;
      case GenXIntrinsicInfo::EXECSIZE:
      case GenXIntrinsicInfo::EXECSIZE_GE2:
      case GenXIntrinsicInfo::EXECSIZE_GE4:
      case GenXIntrinsicInfo::EXECSIZE_GE8:
      case GenXIntrinsicInfo::EXECSIZE_NOT2:
      case GenXIntrinsicInfo::EXECSIZE_NOMASK:
        {
          // Execution size
          ExecSize = GenXIntrinsicInfo::getOverridedExecSize(CI, ST);
          if (ExecSize == 0) {
            if (VectorType *VT = dyn_cast<VectorType>(CI->getType()))
              ExecSize = VT->getNumElements();
            else
              ExecSize = 1;
          }
          bool IsNoMask = Cat == GenXIntrinsicInfo::EXECSIZE_NOMASK;
          writeExecSizeFromWrRegion(ExecSize, DstDesc, IsNoMask);
        }
        break;
      case GenXIntrinsicInfo::EXECSIZE_FROM_ARG: {
          // exec_size inferred from width of predicate arg, defaulting to 16 if
          // it is scalar i1 (as can happen in raw send). Also get M3 etc flag
          // if the predicate has a baled in rdpredregion, and mark as nomask if
          // the predicate is not EM.
          unsigned MaskCtrl = NoMask;
          // Get the predicate (mask) operand, scanning through baled in
          // all/any/not/rdpredregion and setting PredField and MaskCtrl
          // appropriately.
          unsigned PredField;
          Value *Mask = getPredicateOperand(CI, AI.getArgIdx(),
              BI, &PredField, &MaskCtrl);
          if (isa<Constant>(Mask)
              || RegAlloc->getRegNumForValueOrNull(Mask).Category != RegCategory::NONE)
            MaskCtrl |= NoMask;
          if (auto VT = dyn_cast<VectorType>(
                  CI->getOperand(AI.getArgIdx())->getType()))
            ExecSize = VT->getNumElements();
          else
            ExecSize = GenXIntrinsicInfo::getOverridedExecSize(CI, ST);
          assert(ExecSize <= 32 && ExecSize >= 1);
          ExecSize = llvm::log2(ExecSize) | MaskCtrl;
          writeByte(ExecSize);
        }
        break;
      case GenXIntrinsicInfo::GATHERNUMELTS: {
          // gather/scatter "num elements" field, with exec mask in top nibble
          // Works the same as EXECSIZE_FROM_ARG, but the encoding is different
          // (0 for 8 elements, 1 for 16 elements, 2 for 1 element)
          ExecSize = 1;
          Type *Ty = CI->getOperand(AI.getArgIdx())->getType();
          if (Ty->isIntegerTy(1))
            ExecSize = 8; // default for raw send
          else if (VectorType *VT = dyn_cast<VectorType>(Ty))
            ExecSize = VT->getNumElements();
          switch (ExecSize) {
            case 8: ExecSize = 0; break;
            case 16: ExecSize = 1; break;
            default: ExecSize = 2; break;
          }
          writeByte(ExecSize);
        }
        break;
      case GenXIntrinsicInfo::IMPLICITPRED:
        writePredFromWrRegion(DstDesc); // predication
        break;
      case GenXIntrinsicInfo::PREDICATION:
        // Predication from an explicit arg.
        writePred(CI, BI, AI.getArgIdx());
        break;
      case GenXIntrinsicInfo::SAMPLECHMASK:
        // chmask field in load/sample, with exec size bit inferred from
        // the vector width of the U_offset arg
        {
          ConstantInt *Const = dyn_cast<ConstantInt>(CI->getArgOperand(AI.getArgIdx()));
          if (!Const) report_fatal_error("Incorrect args to intrinsic call");
          unsigned Byte = Const->getSExtValue() & 15;
          // Find the U_offset arg. It is the first vector arg after this one.
          VectorType *VT;
          for (unsigned Idx = AI.getArgIdx() + 1;
              !(VT = dyn_cast<VectorType>(CI->getOperand(Idx)->getType())); ++Idx)
            ;
          unsigned Width = VT->getNumElements();
          if (Width != 8 && Width != 16)
            report_fatal_error("Invalid execution size for load/sample");
          Byte |= Width & 16;
          Code.push_back((uint8_t)Byte);
        }
        break;
      case GenXIntrinsicInfo::SVMGATHERBLOCKSIZE: {
          // svm gather/scatter "block size" field, set to reflect the element
          // type of the data
          Value *V = CI;
          if (!AI.isRet())
            V = CI->getArgOperand(AI.getArgIdx());
          unsigned ElBytes = V->getType()->getScalarType()
              ->getPrimitiveSizeInBits() / 8;
          switch (ElBytes) {
            // For N = 2 byte data type, use block size 1 and block count 2.
            // Otherwise, use block size N and block count 1.
            case 2:
            case 1: ElBytes = 0; break;
            case 4: ElBytes = 1; break;
            case 8: ElBytes = 2; break;
            default:
              report_fatal_error("Bad element type for SVM scatter/gather");
          }
          Code.push_back((uint8_t)ElBytes);
        }
        break;
      case GenXIntrinsicInfo::TRANSPOSEHEIGHT: {
          // log2 block_height field in transpose, derived from block_width
          // (that this arg points at) and the vector width of the return type
          unsigned BlockWidth = cast<ConstantInt>(CI->getArgOperand(
                AI.getArgIdx()))->getSExtValue();
          unsigned Width = cast<VectorType>(CI->getType())->getNumElements();
          Code.push_back((uint8_t)(llvm::log2(Width) - BlockWidth));
        }
        break;
      case GenXIntrinsicInfo::CONSTVI1ASI32: {
          // constant vector of i1 (or just scalar i1) as i32 (used in setp)
          auto C = cast<Constant>(CI->getArgOperand(AI.getArgIdx()));
          // Get the bit value of the vXi1 constant.
          unsigned IntVal = getPredicateConstantAsInt(C);
          // unsigned i32 constant source operand
          Code.push_back((uint8_t)CLASS_IMMEDIATE);
          Code.push_back((uint8_t)ISA_TYPE_UD);
          Code.push_back((uint32_t)IntVal);
        }
        break;
      case GenXIntrinsicInfo::TWOADDR:
        break; // no vISA code for this operand
      case GenXIntrinsicInfo::ARGCOUNT:
        Code.push_back((uint8_t)(MaxRawOperands - AI.getArgIdx()));
        break;
      case GenXIntrinsicInfo::ISBARRIER:
        HasBarrier = true;
        break;
      default:
        assert(0);
    }
  }
}

/***********************************************************************
 * buildControlRegUpdate : generate an instruction to apply a mask to
 *                         the control register (V14).
 *
 * Enter:   Mask = the mask to apply
 *          Clear = false if bits set in Mask should be set in V14,
 *                  true if bits set in Mask should be cleared in V14.
 */
void VisaFuncWriter::buildControlRegUpdate(unsigned Mask, bool Clear)
{
  // write opcode
  if (Clear) {
    writeOpcode(ISA_AND);
    Mask = ~Mask;
  } else
    writeOpcode(ISA_OR);

  // Write the execution size and predicate fields
  writeByte(llvm::log2(1));
  writeShort(0);

  Region Single = Region(1, 4);

  // write CR0 as dest
  writeRegion(&Single, PREDEFINED_CR0, DONTCARESIGNED, 0, true);
  // write CR0 as first source
  writeRegion(&Single, PREDEFINED_CR0, DONTCARESIGNED, 0, false);

  // write Mask as an immediate operand.
  Code.push_back((uint8_t)CLASS_IMMEDIATE);
  Code.push_back((uint8_t)ISA_TYPE_UD);
  Code.push_back((uint32_t)Mask);
}

/***********************************************************************
 * writeWrLifetimeStart : write a lifetime.start instruction if appropriate
 *                        for the result of a wrregion
 *
 * Enter:   WrRegion = wrregion instruction whose "old value of vector"
 *                     operand is undef
 *
 * It is believed that a lifetime.start acts somewhat as an optimization
 * barrier in the jitter, so we only want to write one if it is actually
 * beneficial. It is not beneficial if the wrregion is the first of a chain
 * of wrregions in the same basic block that together write the whole
 * vector, and it is not beneficial if the instruction is not inside a loop.
 */
void VisaFuncWriter::writeWrLifetimeStart(Instruction *WrRegion)
{
  if (!isWrRegion(getIntrinsicID(WrRegion)))
    return; // No lifetime start for wrpredregion.
  // See if the wrregion is in a loop.
  auto BB = WrRegion->getParent();
  if (!isInLoop(BB))
    return; // not in loop
  // See if the wrregion is the first of a sequence in the same basic block
  // that together write the whole register. We assume that each region is
  // contiguous, and the regions are written in ascending offset order, as that
  // is what legalization does if the original write was to the whole register.
  unsigned NumElementsSoFar = 0;
  unsigned TotalNumElements = WrRegion->getType()->getVectorNumElements();
  Instruction *ThisWr = WrRegion;
  for (;;) {
    Region R(ThisWr, BaleInfo());
    if (R.Indirect)
      break;
    if ((unsigned)R.Offset != NumElementsSoFar * R.ElementBytes)
      break;
    if (R.Stride != 1 && R.Width != 1)
      break;
    if (R.Width != R.NumElements)
      break;
    NumElementsSoFar += R.NumElements;
    if (NumElementsSoFar == TotalNumElements)
      return; // whole register is written
    // Go on to next wrregion in the same basic block if any.
    if (!ThisWr->hasOneUse())
      break;
    ThisWr = cast<Instruction>(ThisWr->use_begin()->getUser());
    if (!isWrRegion(getIntrinsicID(ThisWr)))
      break;
    if (ThisWr->getParent() != BB)
      break;
  }
  // The wrregion is in a loop and is not the first in a sequence in the same
  // basic block that writes the whole register. Write a lifetime start.
  writeLifetimeStart(WrRegion);
}

/***********************************************************************
 * writeSendLifetimeStart : write a lifetime.start instruction if appropriate
 *      for a send with a TWOADDR arg that is undef
 *
 * Enter:   Inst = call inst for the send
 *
 * It is believed that a lifetime.start acts somewhat as an optimization
 * barrier in the jitter, so we only want to write one if it is actually
 * beneficial. It is not beneficial if the instruction is not inside a loop.
 */
void VisaFuncWriter::writeSendLifetimeStart(Instruction *Inst)
{
  // See if the instruction is in a loop.
  auto BB = Inst->getParent();
  if (!isInLoop(BB))
    return; // not in loop
  writeLifetimeStart(Inst);
}

/***********************************************************************
 * isInLoop : see if basic block is in a loop
 *
 * This is for the purposes of deciding whether to generate a lifetime start
 * instruction. Thus we also need to check whether the function is called
 * from within a loop, anywhere up the call graph to the kernel.
 */
bool VisaFuncWriter::isInLoop(BasicBlock *BB)
{
  if (getLoops(BB->getParent())->getLoopFor(BB))
    return true; // inside loop in this function
  // Now we need to see if this function is called from inside a loop.
  // First check the cache.
  auto i = IsInLoopCache.find(BB->getParent());
  if (i != IsInLoopCache.end())
    return i->second;
  // Now check all call sites. This recurses as deep as the depth of the call
  // graph, which must be acyclic as GenX does not allow recursion.
  bool InLoop = false;
  for (auto ui = BB->getParent()->use_begin(), ue = BB->getParent()->use_end();
      ui != ue; ++ui) {
    auto CI = cast<CallInst>(ui->getUser());
    assert(ui->getOperandNo() == CI->getNumArgOperands());
    if (isInLoop(CI->getParent())) {
      InLoop = true;
      break;
    }
  }
  IsInLoopCache[BB->getParent()] = InLoop;
  return InLoop;
}

/***********************************************************************
 * writeLifetimeStart : write a lifetime.start instruction
 *
 * Enter:   Inst = value to use in lifetime.start
 */
void VisaFuncWriter::writeLifetimeStart(Instruction *Inst)
{
  auto RN = RegAlloc->getRegNumForValueOrNull(Inst);
  if (RN.isNull())
    return; // no register allocated such as being indirected.

  writeOpcode(ISA_LIFETIME);
  unsigned Properties = 0; // lifetime start
  switch (RN.Category) {
    case RegCategory::GENERAL:
      break;
    case RegCategory::ADDRESS:
      Properties |= 1 << 4;
      break;
#if 0 // Not currently used.
    case RegCategory::PREDICATE:
      Properties |= 2 << 4;
      break;
#endif // 0
    default:
      assert(0);
      break;
  }
  writeByte(Properties);
  writeInt(RN.Num);
}

/***********************************************************************
 * writeExecSizeFromWrPredRegion : write exec size field from wrpredregion
 *        or wrpredpredregion instruction
 *
 * Enter:   ExecSize = execution size
 *          WrPredRegion = 0 else wrpredregion instruction
 *
 * The exec size byte includes the mask control field, which we need to set
 * up from the wrpredregion/wrpredpredregion.
 */
void VisaFuncWriter::writeExecSizeFromWrPredRegion(unsigned ExecSize,
    Instruction *WrPredRegion, bool IsNoMask)
{
  unsigned MaskCtrl = 0;
  if (WrPredRegion) {
#if _DEBUG
    unsigned Size =
        WrPredRegion->getOperand(1)->getType()->getPrimitiveSizeInBits();
    assert(exactLog2(Size) >= 2 &&
           "wrpredregion input size must be power of 2 no less than 4");
#endif
    // Get the mask control field from the offset in the wrpredregion.
    unsigned MaskOffset =
        cast<ConstantInt>(WrPredRegion->getOperand(2))->getSExtValue();
    assert(!(MaskOffset & (Size - 1)) && MaskOffset < 32 && "unexpected mask offset");
    MaskCtrl = MaskOffset << 2;
  }

  // Set to NoMask if requested. Otherwise use the default NM mode
  // when WrPredRegion is null.
  if (IsNoMask)
    MaskCtrl |= 0x80;
  else if (!WrPredRegion)
    MaskCtrl |= NoMask;

  ExecSize = llvm::log2(ExecSize) | MaskCtrl;
  writeByte(ExecSize);
}

/***********************************************************************
 * writeExecSizeFromWrRegion : write exec size field from wrregion instruction
 *
 * Enter:   ExecSize = execution size
 *          WrRegion = 0 else wrregion instruction
 *          WrRegionBI = BaleInfo for wrregion, so we can see if there is a
 *                rdpredregion baled in to the mask
 *
 * If WrRegion != 0, and it has a mask that is not constant 1, then the
 * mask must be a predicate register.
 *
 * The exec size byte includes the mask control field, which we need to set
 * up from any rdpredregion baled in to a predicated wrregion.
 *
 * If the predicate has no register allocated, it must be EM, and we set the
 * instruction to be masked. Otherwise we set nomask.
 */
void VisaFuncWriter::writeExecSizeFromWrRegion(unsigned ExecSize,
                                               const DstOpndDesc &DstDesc,
                                               bool IsNoMask) {
  unsigned MaskCtrl = NoMask;
  // Override mask control if requested.
  if (IsNoMask)
    MaskCtrl = 0x80;
  if (DstDesc.WrRegion) {
    // Get the predicate (mask) operand, scanning through baled in
    // all/any/not/rdpredregion and setting PredField and MaskCtrl
    // appropriately.
    unsigned PredField;
    Value *Mask =
        getPredicateOperand(DstDesc.WrRegion, 7 /*mask operand in wrregion*/,
                            DstDesc.WrRegionBI, &PredField, &MaskCtrl);
    if (isa<Constant>(Mask)
        || RegAlloc->getRegNumForValueOrNull(Mask).Category != RegCategory::NONE)
      MaskCtrl |= NoMask;
  }
  assert(ExecSize <= 32);
  ExecSize = llvm::log2(ExecSize) | MaskCtrl;
  writeByte(ExecSize);
}

/***********************************************************************
 * writeExecSizeFromSelect : write exec size field from select inst
 *
 * Enter:   ExecSize = execution size
 *          SI = select inst
 *          BI = BaleInfo for select, so we can see if there is a
 *                rdpredregion baled in to the mask
 *
 * The exec size byte includes the mask control field, which we need to set
 * up from any rdpredregion baled in to the select
 */
void VisaFuncWriter::writeExecSizeFromSelect(unsigned ExecSize,
    Instruction *SI, BaleInfo BI)
{
  unsigned MaskCtrl = NoMask;
  // Get the predicate (mask) operand, scanning through baled in
  // all/any/not/rdpredregion and setting PredField and MaskCtrl
  // appropriately.
  unsigned PredField;
  Value *Mask = getPredicateOperand(SI, 0/*selector operand in select*/,
      BI, &PredField, &MaskCtrl);
  if (isa<Constant>(Mask)
      || RegAlloc->getRegNumForValueOrNull(Mask).Category != RegCategory::NONE)
    MaskCtrl |= NoMask;
  ExecSize = llvm::log2(ExecSize) | MaskCtrl;
  writeByte(ExecSize);
}

/***********************************************************************
 * writePredFromWrRegion : write predication field from wrregion instruction
 *
 * Enter:   WrRegion = 0 else wrregion instruction
 *          WrRegionBI = BaleInfo for wrregion, so we can see if there is a
 *                rdpredregion baled in to the mask
 *
 * If WrRegion != 0, and it has a mask that is not constant 1, then the
 * mask must be a predicate register (or a baled in rdpredregion or all/any).
 */
void VisaFuncWriter::writePredFromWrRegion(const DstOpndDesc &DstDesc)
{
  unsigned PredField = 0;
  Instruction *WrRegion = DstDesc.WrRegion;
  if (WrRegion) {
    // Get the predicate (mask) operand, scanning through baled in
    // all/any/not/rdpredregion and setting PredField and MaskCtrl
    // appropriately.
    unsigned MaskCtrl;
    Value *Mask =
        getPredicateOperand(WrRegion, 7 /*mask operand in wrregion*/,
                            DstDesc.WrRegionBI, &PredField, &MaskCtrl);
    if (auto C = dyn_cast<Constant>(Mask)) {
      (void)C;
      assert(C->isAllOnesValue() && "wrregion mask or predication operand must "
                                    "be constant 1 or not constant");
    } else {
      // Variable predicate. Derive the predication field from any baled in
      // all/any/not and the predicate register number. If the predicate has not
      // has a register allocated, it must be EM.
      GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValueOrNull(Mask);
      if (Reg.Category != RegCategory::NONE) {
        assert(Reg.Category == RegCategory::PREDICATE);
        PredField |= Reg.Num;
      }
    }
  }
  writeShort(PredField);
}

/***********************************************************************
 * writePredFromSelect : write predication field from select inst
 *
 * Enter:   SI = select inst
 *          BI = BaleInfo for select, so we can see if there is a
 *                rdpredregion baled in to the mask
 */
void VisaFuncWriter::writePredFromSelect(Instruction *SI, BaleInfo BI)
{
  unsigned PredField = 0;
  // Get the predicate (mask) operand, scanning through baled in
  // all/any/not/rdpredregion and setting PredField and MaskCtrl
  // appropriately.
  unsigned MaskCtrl;
  Value *Mask = getPredicateOperand(SI, 0/*selector operand in select*/,
      BI, &PredField, &MaskCtrl);
  assert(!isa<Constant>(Mask));
  // Variable predicate. Derive the predication field from any baled in
  // all/any/not and the predicate register number.
  GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValue(Mask);
  assert(Reg.Category == RegCategory::PREDICATE);
  PredField |= Reg.Num;
  writeShort(PredField);
}

/***********************************************************************
 * getPredicateOperand : get predicate operand, scanning through any baled
 *    in rdpredregion, all, any, not instructions to derive the mask control
 *    field and the predication field
 *
 * Enter:   Inst = instruction to get predicate operand from
 *          OperandNum = operand number in Inst
 *          BI = bale info for Inst
 *          *PredField = where to write predication field modifier bits
 *          *MaskCtrl = where to write mask control field (bits 7..4)
 *
 * Return:  Value of mask after scanning through baled in instructions
 *          *PredField and *MaskCtrl set
 */
Value *VisaFuncWriter::getPredicateOperand(Instruction *Inst, unsigned OperandNum,
    BaleInfo BI, unsigned *PredField, unsigned *MaskCtrl)
{
  *PredField = 0;
  *MaskCtrl = 0;
  Value *Mask = Inst->getOperand(OperandNum);
  // Check for baled in all/any/notp/rdpredregion.
  while (BI.isOperandBaled(OperandNum)) {
    Instruction *Inst = dyn_cast<Instruction>(Mask);
    if (isNot(Inst)) {
      if (*PredField & 0x6000)
        *PredField ^= 0xe000; // switch any<->all as well as invert bit
      else
        *PredField ^= 0x8000; // all/any not set, just invert invert bit
      OperandNum = 0;
      Mask = Inst->getOperand(OperandNum);
      BI = Baling->getBaleInfo(Inst);
      continue;
    }
    switch (getIntrinsicID(Inst)) {
      case Intrinsic::genx_all:
        *PredField |= 0x4000; // predicate combine field = "all"
        OperandNum = 0;
        Mask = Inst->getOperand(OperandNum);
        BI = Baling->getBaleInfo(Inst);
        continue;
      case Intrinsic::genx_any:
        *PredField |= 0x2000; // predicate combine field = "any"
        OperandNum = 0;
        Mask = Inst->getOperand(OperandNum);
        BI = Baling->getBaleInfo(Inst);
        continue;
      case Intrinsic::genx_rdpredregion: {
          // Baled in rdpredregion. Use its constant offset for the mask control
          // field.
          unsigned MaskOffset = cast<ConstantInt>(Inst->getOperand(1))
              ->getSExtValue();
          assert(!(MaskOffset & 3) && MaskOffset < 32 && "unexpected mask offset");
          *MaskCtrl = MaskOffset << 2;
          Mask = Inst->getOperand(0);
          break;
        }
    }
    break;
  }
  return Mask;
}

/***********************************************************************
 * writePred : write predication field from an instruction operand
 *
 * Enter:   Inst = the instruction (0 to write an "always true" pred field)
 *          BI = BaleInfo for the instruction, so we can see if there is a
 *                rdpredregion baled in to the mask
 *          OperandNum = operand number in the instruction
 *
 * If the operand is not constant 1, then it must be a predicate register.
 */
void VisaFuncWriter::writePred(Instruction *Inst, BaleInfo BI, unsigned OperandNum)
{
  unsigned PredField = 0, MaskCtrl = 0;
  Value *Mask = getPredicateOperand(Inst, OperandNum, BI, &PredField, &MaskCtrl);
  if (auto C = dyn_cast<Constant>(Mask)) {
    (void)C;
    assert(C->isAllOnesValue() && "wrregion mask or predication operand must "
                                  "be constant 1 or not constant");
  } else {
    // Variable predicate. Derive the predication field from any baled in
    // all/any/not and the predicate register number. If the predicate has not
    // has a register allocated, it must be EM.
    GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValueOrNull(Mask);
    if (Reg.Category != RegCategory::NONE) {
      assert(Reg.Category == RegCategory::PREDICATE);
      PredField |= Reg.Num;
    }
  }
  writeShort(PredField);
}

/***********************************************************************
 * writeRawDestination : write raw destination operand
 *
 * Enter:   Inst = destination value
 *          WrRegion = 0 else wrregion that destination is baled into
 *
 * A raw destination can be baled into a wrregion, but only if the region
 * is direct and its start index is GRF aligned.
 */
void VisaFuncWriter::writeRawDestination(Value *V, const DstOpndDesc &DstDesc,
                                         Signedness Signed) {
  unsigned ByteOffset = 0;
  if (DstDesc.WrRegion) {
    V = DstDesc.WrRegion;
    Region R(DstDesc.WrRegion, BaleInfo());
    ByteOffset = R.Offset;
  }
  Type *OverrideType = nullptr;
  if (DstDesc.GStore) {
    V = getUnderlyingGlobalVariable(DstDesc.GStore->getOperand(1));
    assert(V && "out of sync");
    OverrideType = DstDesc.GStore->getOperand(0)->getType();
  }
  GenXVisaRegAlloc::RegNum Reg =
      RegAlloc->getRegNumForValueOrNull(V, Signed, OverrideType);
  if (!Reg.Category) {
    // No register assigned. This happens to an unused raw result where the
    // result is marked as RAW_NULLALLOWED in GenXIntrinsics.
    Code.push_back((uint32_t)0);
    Code.push_back((uint16_t)0);
  } else {
    assert(Reg.Category == RegCategory::GENERAL);
    Code.push_back((uint32_t)Reg.Num); // id
    Code.push_back((uint16_t)ByteOffset); // byte offset
  }
}

/***********************************************************************
 * writeRawSourceOperand : write raw source operand of instruction
 *
 * Enter:   Inst = instruction to get source operand from
 *          OperandNum = operand number
 *          BI = BaleInfo for Inst (so we can tell whether a rdregion
 *                  or modifier is bundled in)
 */
void VisaFuncWriter::writeRawSourceOperand(Instruction *Inst,
    unsigned OperandNum, BaleInfo BI, Signedness Signed)
{
  Value *V = Inst->getOperand(OperandNum);
  if (isa<UndefValue>(V)) {
    // Use v0 for undef value.
    Code.push_back((uint32_t)0); // v0
    Code.push_back((uint16_t)0); // byte offset
  } else {
    unsigned ByteOffset = 0;
    if (Baling->getBaleInfo(Inst).isOperandBaled(OperandNum)) {
      Instruction *RdRegion = cast<Instruction>(V);
      Region R(RdRegion, BaleInfo());
      ByteOffset = R.Offset;
      V = RdRegion->getOperand(0);
    }
    GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValue(V, Signed);
    assert(Reg.Category == RegCategory::GENERAL);
    // Write the vISA raw operand:
    Code.push_back((uint32_t)Reg.Num); // id (register number)
    Code.push_back((uint16_t)ByteOffset); // byte offset
  }
}

/***********************************************************************
 * writeDestination : write destination operand
 *
 * Enter:   Dest = destination value
 *          Signed = whether signed/unsigned required
 *          Mod = modifiers
 *          WrRegion = 0 else wrregion that destination is baled into
 *          WrRegionBI = BaleInfo for WrRegion
 *
 * Return:  signedness of destination register (so caller can make the source
 *          the same signedness if it wants)
 */
genx::Signedness VisaFuncWriter::writeDestination(Value *Dest,
                                                  Signedness Signed,
                                                  unsigned Mod,
                                                  const DstOpndDesc &DstDesc) {
  Type *OverrideType = nullptr;
  if (BitCastInst *BCI = dyn_cast<BitCastInst>(Dest)) {
    if (!(isa<Constant>(BCI->getOperand(0))) &&
      !(BCI->getType()->getScalarType()->isIntegerTy(1)) &&
      (BCI->getOperand(0)->getType()->getScalarType()->isIntegerTy(1))) {
      if (VectorType *VT = dyn_cast<VectorType>(Dest->getType())) {
        unsigned int NumBits = VT->getNumElements() * VT->getElementType()->getPrimitiveSizeInBits();
        OverrideType = IntegerType::get(BCI->getContext(), NumBits);
      }
    }
  }

  // Saturation can also change signedness.
  if (!Dest->user_empty() && isIntegerSat(Dest->user_back())) {
    Signed = getISatDstSign(Dest->user_back());
  }

  if (!DstDesc.WrRegion) {
    if (Mod) {
      // There is a sat modifier. Either it is an fp saturate, which is
      // represented by its own intrinsic which this instruction is baled
      // into, or it is an int saturate which always comes from this
      // instruction's semantics. In the former case, use the value
      // that is the result of the saturate. But only if this instruction
      // itself is not the sat intrinsic.
      if (Dest->getType()->getScalarType()->isFloatingPointTy()
          && getIntrinsicID(Dest) != Intrinsic::genx_sat)
        Dest = cast<Instruction>(Dest->use_begin()->getUser());
    }
    if ((Mod & MOD_SAT) != 0) {
      // Similar for integer saturation.
      if (Dest->getType()->getScalarType()->isIntegerTy() &&
          !isIntegerSat(Dest) && isIntegerSat(Dest->user_back()))
        Dest = cast<Instruction>(Dest->user_back());
    }
    GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValue(Dest, Signed, OverrideType);
    // Write the vISA general operand:
    if (Reg.Category == RegCategory::GENERAL) {
      Region DestR(Dest);
      writeRegion(&DestR, Reg.Num, DONTCARESIGNED, Mod/*Mod*/, true/*IsDest*/);
    } else {
      assert(Reg.Category == RegCategory::SURFACE
             || Reg.Category == RegCategory::VME
             || Reg.Category == RegCategory::SAMPLER);
      writeByte(CLASS_STATE | Mod); // tag+modifiers
      writeByte(Reg.Category == RegCategory::SURFACE ? 0 :
                Reg.Category == RegCategory::SAMPLER ? 1 : 2); // class
      writeShort(Reg.Num); // id (register number)
      writeByte(0); // offset
    }
    return RegAlloc->getSigned(Reg);
  }
  // We need to allow for the case that there is no register allocated if it is
  // an indirected arg, and that is OK because the region is indirect so the
  // vISA does not contain the base register.
  GenXVisaRegAlloc::RegNum Reg;

  if (DstDesc.GStore) {
    auto GV = getUnderlyingGlobalVariable(DstDesc.GStore->getOperand(1));
    assert(GV && "out of sync");
    if (OverrideType == nullptr)
      OverrideType = DstDesc.GStore->getOperand(0)->getType();
    Reg = RegAlloc->getRegNumForValue(GV, Signed, OverrideType);
  } else
    Reg = RegAlloc->getRegNumForValueOrNull(DstDesc.WrRegion, Signed, OverrideType);

  assert(Reg.Category == RegCategory::GENERAL ||
         Reg.Category == RegCategory::NONE);

  // Write the vISA general operand with region:
  Region R(DstDesc.WrRegion, DstDesc.WrRegionBI);
  writeRegion(&R, Reg.Num, Signed, Mod, true /*IsDest*/);
  return RegAlloc->getSigned(Reg);
}

/***********************************************************************
 * writeSourceOperand : write source operand of instruction
 *
 * Enter:   Inst = instruction to get source operand from
 *          Signed = whether signed/unsigned required
 *          OperandNum = operand number
 *          BI = BaleInfo for Inst (so we can tell whether a rdregion
 *                  or modifier is bundled in)
 *          Mod = 0 else MOD_NEG from sub instruction
 *          MaxWidth = maximum width (used to stop TWICEWIDTH operand
 *                     getting a width bigger than the execution size, but
 *                     for other uses defaults to 16)
 *
 * Return:  signedness of the operand in case the caller wants to adjust 
 *          the signedness of other operands
 */
Signedness VisaFuncWriter::writeSourceOperand(Instruction *Inst, Signedness Signed,
    unsigned OperandNum, BaleInfo BI, unsigned Mod, unsigned MaxWidth)
{
  Value *V = Inst->getOperand(OperandNum);
  return writeSource(V, Signed, BI.isOperandBaled(OperandNum), Mod, MaxWidth);
}

/***********************************************************************
 * writeSource : write source operand
 *
 * Enter:   V = source value
 *          Signed = whether signed/unsigned required
 *          Baled = whether V is a rdregion/modifier instruction that
 *                  is baled in
 *          Mod = 0 else MOD_NEG modifier from sub instruction. No other
 *                modifier is possible because GenXBaling simplifies away
 *                a modifier with a constant operand.
 *          MaxWidth = maximum width (used to stop TWICEWIDTH operand
 *                     getting a width bigger than the execution size, but
 *                     for other uses defaults to 16)
 *
 * Return:  signedness of the operand in case the caller wants to adjust 
 *          the signedness of other operands
 */
Signedness VisaFuncWriter::writeSource(Value *V, Signedness Signed, bool Baled,
    unsigned Mod, unsigned MaxWidth)
{
  if (auto C = dyn_cast<Constant>(V)) {
    if (Mod) {
      // Need to negate constant.
      assert(Mod == MOD_NEG && "unexpected modifier");
      if (C->getType()->isIntOrIntVectorTy())
        C = ConstantExpr::getNeg(C);
      else
        C = ConstantExpr::getFNeg(C);
    }
    writeImmediateOperand(C, Signed);
    return Signed;
  }

  if (!Baled) {
    GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValue(V, Signed);
    assert(Reg.Category == RegCategory::GENERAL || Reg.Category == RegCategory::SURFACE
        || Reg.Category == RegCategory::SAMPLER || Reg.Category == RegCategory::VME);
    // Write the vISA general operand.
    Region R(V);
    if (R.NumElements == 1)
      R.VStride = R.Stride = 0;
    if (Reg.Category == RegCategory::GENERAL)
      writeRegion(&R, Reg.Num, Signed, Mod, false/*IsDest*/, MaxWidth);
    else
      writeState(&R, Reg.Num, Reg.Category);
    return RegAlloc->getSigned(Reg);
  }
  Instruction *Inst = cast<Instruction>(V);
  BaleInfo BI(Baling->getBaleInfo(Inst));
  unsigned Idx = 0;
  switch (BI.Type) {
    case BaleInfo::RDREGION: {
        // The source operand has a rdregion baled in. We need to allow for the
        // case that there is no register allocated if it is an indirected arg,
        // and that is OK because the region is indirect so the vISA does not
        // contain the base register.
        GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValueOrNull(
            Inst->getOperand(0), Signed);
        // Ensure we pick a non-DONTCARESIGNED signedness here, as, for an
        // indirect region and DONTCARESIGNED, writeRegion arbitrarily picks a
        // signedness as it is attached to the operand, unlike a direct region
        // where it is attached to the vISA register.
        if (Reg.Category != RegCategory::NONE)
          Signed = RegAlloc->getSigned(Reg);
        else if (Signed == DONTCARESIGNED)
          Signed = SIGNED;
        // Write the vISA general operand with region.
        Region R(Inst, Baling->getBaleInfo(Inst));
        if (R.NumElements == 1)
          R.VStride = 0;
        if (R.Width == 1)
          R.Stride = 0;
        if (Reg.Category == RegCategory::GENERAL
            || Reg.Category == RegCategory::NONE || R.Indirect)
          writeRegion(&R, Reg.Num, Signed, Mod, false/*IsDest*/, MaxWidth);
        else
          writeState(&R, Reg.Num, Reg.Category);
        return Signed;
      }
    case BaleInfo::ABSMOD:
      Signed = SIGNED;
      Mod |= MOD_ABS;
      break;
    case BaleInfo::NEGMOD:
      if (!(Mod & MOD_ABS))
        Mod ^= MOD_NEG;
      Idx = 1; // the input we want in "0-x" is x, not 0.
      break;
    case BaleInfo::NOTMOD:
      Mod ^= MOD_NOT;
      break;
    case BaleInfo::ZEXT:
      Signed = UNSIGNED;
      break;
    case BaleInfo::SEXT:
      Signed = SIGNED;
      break;
    default:
      assert(0);
      break;
  }
  return writeSource(Inst->getOperand(Idx), Signed, BI.isOperandBaled(Idx),
                     Mod, MaxWidth);
}

/***********************************************************************
 * writeState : write a vISA state operand
 *
 * Enter:   R = Region
 *          RegNum = state register number
 *          Kind = RegCategory of the state (SURFACE, SAMPLER, VME)
 */
void VisaFuncWriter::writeState(Region *R, unsigned RegNum, unsigned Kind)
{
  enum { SK_SURFACE=0, SK_SAMPLER=1, SK_VME=2 };
  assert(Kind == RegCategory::SURFACE || Kind == RegCategory::SAMPLER || Kind == RegCategory::VME);

  // In the case where we are dealing with state operands there are a number of
  // restrictions on the region.  State can only use an offset from the base
  // address. The rest of the region information is not used - however, for
  // convenience and to reduce the amount of duplication elsewhere we still
  // represent state operands using regions.

  writeByte(CLASS_STATE);

  switch(Kind) {
  case RegCategory::SURFACE:
    writeByte(0);
    break;
  case RegCategory::SAMPLER:
    writeByte(1);
    break;
  case RegCategory::VME:
    writeByte(2);
    break;
  default:
    assert(0 && "Unknown register category in writeState");
  }

  writeShort(RegNum);
  // state-element is in dwords. Region offset is in bytes
  writeByte(R->Offset>>2);
}

/***********************************************************************
 * writeRegion : write a vISA region operand
 *
 * Enter:   R = Region
 *          RegNum = vISA register number (ignored if region is indirect)
 *          Signed = whether signed or unsigned required (only used for
 *                   indirect operand)
 *          Mod = modifiers
 *          IsDest = true if destination operand
 *          MaxWidth = maximum width (used to stop TWICEWIDTH operand
 *                     getting a width bigger than the execution size, but
 *                     for other uses defaults to 16)
 */
void VisaFuncWriter::writeRegion(Region *R, unsigned RegNum,
    Signedness Signed, unsigned Mod, bool IsDest, unsigned MaxWidth)
{
  if (!IsDest && !R->is2D() && R->Indirect && ST->hasIndirectGRFCrossing()) {
    // For a source 1D indirect region that might possibly cross a GRF (because
    // we are on SKL+ so a single GRF crossing is allowed), make it Nx1 instead
    // of 1xN to avoid crossing a GRF within a row.
    R->VStride = R->Stride;
    R->Width = 1;
    R->Stride = 0;
  }
  // another case of converting to <N;1,0> region format
  if (!IsDest && 
    (R->VStride == (int)R->Width * R->Stride || R->Width == R->NumElements)) {
    R->Width = 1;
    R->VStride = R->Stride;
    R->Stride = 0;
  }
  else if (R->Width > MaxWidth) {
    // A Width of more than 16 (or whatever MaxWidth is) is not allowed. If it is
    // more than 16, then legalization has ensured that either there is one row
    // or the rows are contiguous (VStride == Width * Stride) and we can increase
    // the number of rows.  (Note that Width and VStride are ignored in a
    // destination operand; legalization ensures that there is only one row.)
    R->Width = MaxWidth;
    R->VStride = R->Width * R->Stride;
  }

  if (R->Width == R->NumElements) {
    // Use VStride 0 on a 1D region. This is necessary for src0 in line or
    // pln, so we may as well do it for everything.
    R->VStride = 0;
  }
  if (!R->Indirect) {
    // Direct operand. Write the vISA general operand, canonicalizing the
    // region parameters where applicable.
    assert(RegNum && "no register allocated for this value");
    writeByte(CLASS_GENERAL | Mod); // tag+modifiers
    writeInt(RegNum); // id (register number)
    writeByte(R->Offset >> llvm::log2(GrfByteSize)); // row offset
    writeByte((R->Offset & (GrfByteSize-1)) / R->ElementBytes); // col offset
    if (!IsDest) {
      if (R->NumElements == 1)
        writeShort(0x0121); // <0;1,0>
      else {
        writeByte((llvm::log2(R->VStride) + 2) | (llvm::log2(R->Width) + 2) << 4); // region (vstride and width)
        writeByte(llvm::log2(R->Stride) + 2); // region (stride)
      }
    } else {
      if (R->NumElements == 1)
        writeShort(0x0200); // <1>
      else {
        writeByte(0); // region (vstride=null and width=null)
        writeByte(llvm::log2(R->Stride) + 2); // region (stride)
      }
    }
  } else {
    // Check if the indirect operand is a baled in rdregion.
    Value *Indirect = R->Indirect;
    unsigned AddrOffset = 0;
    if (isRdRegion(getIntrinsicID(Indirect))) {
      auto AddrRdR = cast<Instruction>(Indirect);
      Region AddrR(AddrRdR, BaleInfo());
      assert(!AddrR.Indirect && "cannot have address rdregion that is indirect");
      Indirect = AddrRdR->getOperand(0);
      AddrOffset = AddrR.Offset / 2; // address element is always 2 byte
    }
    // Write the vISA indirect operand.
    GenXVisaRegAlloc::RegNum IdxReg = RegAlloc->getRegNumForValue(Indirect,
        DONTCARESIGNED);
    assert(IdxReg.Category == RegCategory::ADDRESS);
    writeByte(CLASS_INDIRECT | Mod); // tag+modifiers
    writeShort(IdxReg.Num); // id (register number)
    writeByte(AddrOffset); // addr_offset
    writeShort(R->Offset); // indirect_offset
    bool NotCrossGrf = !(R->Offset & (GrfByteSize - 1));
    if (!NotCrossGrf) {
      // Determine the NotCrossGrf bit setting (whether we can guarantee
      // that adding an indirect region's constant offset does not cause
      // a carry out of bit 4)
      // by looking at the partial constant for the index
      // before the constant is added on.
      // This only works for a scalar index.
      if (auto IndirInst = dyn_cast<Instruction>(R->Indirect)) {
        auto A = AI.get(IndirInst);
        unsigned Mask = (1U << std::min(5U, A.getLogAlign())) - 1;
        if (Mask) {
          if ((A.getExtraBits() & Mask) + (R->Offset & Mask) <= Mask
              && (unsigned)(R->Offset & (GrfByteSize - 1)) <= Mask) {
            // The alignment and extrabits are such that adding R->Offset
            // cannot cause a carry from bit 4 to bit 5.
            NotCrossGrf = true;
          }
        }
      }
    }
    TypeDetails TD(Func->getParent()->getDataLayout(), R->ElementTy, Signed);
    writeByte(TD.VisaType | NotCrossGrf << 4); // bit_properties
    unsigned VStride = llvm::log2(R->VStride) + 2;
    if (isa<VectorType>(R->Indirect->getType()))
      VStride = 0; // multi indirect (vector index), set vstride field to null
    writeByte(VStride | (llvm::log2(R->Width) + 2) << 4); // region (vstride and width)
    writeByte(llvm::log2(R->Stride) + 2); // region (stride)
  }
}

static bool isDerivedFromUndef(Constant *C) {
  if (isa<UndefValue>(C))
    return true;
  if (!isa<ConstantExpr>(C))
    return false;
  ConstantExpr *CE = cast<ConstantExpr>(C);
  for (auto &Opnd : CE->operands())
    if (isDerivedFromUndef(cast<Constant>(Opnd)))
      return true;
  return false;
}

static unsigned get8bitPackedFloat(float f) {
  union {
    float f;
    unsigned u;
  } u;

  u.f = f;
  unsigned char Sign = (u.u >> 31) << 7;
  unsigned Exp = (u.u >> 23) & 0xFF;
  unsigned Frac = u.u & 0x7FFFFF;
  if (Exp == 0 && Frac == 0)
    return Sign;

  assert(Exp >= 124 && Exp <= 131);
  Exp -= 124;
  assert((Frac & 0x780000) == Frac);
  Frac >>= 19;
  assert(!(Exp == 124 && Frac == 0));

  Sign |= (Exp << 4);
  Sign |= Frac;

  return Sign;
}

/***********************************************************************
 * writeImmediateOperand : write an immediate operand
 *
 * Enter:   V = operand value
 *          Signed = whether we want signed or unsigned
 */
void VisaFuncWriter::writeImmediateOperand(Constant *V, Signedness Signed)
{
  Code.push_back((uint8_t)CLASS_IMMEDIATE);

  if (isDerivedFromUndef(V))
    V = Constant::getNullValue(V->getType());

  Type *T = V->getType();
  if (VectorType *VT = dyn_cast<VectorType>(T)) {
    // Vector constant.
    auto Splat = V->getSplatValue();
    if (!Splat) {
      // Non-splatted vector constant. Must be a packed vector.
      unsigned NumElements = VT->getNumElements();
      if (VT->getElementType()->isIntegerTy()) {
        // Packed int vector.
        assert(NumElements <= 8);
        unsigned Packed = 0;
        for (unsigned i = 0; i != NumElements; ++i) {
          auto El = dyn_cast<ConstantInt>(V->getAggregateElement(i));
          if (!El)
            continue; // undef element
          int This = El->getSExtValue();
          if (This < 0)
            Signed = SIGNED;
          else if (This >= 8)
            Signed = UNSIGNED;
          Packed |= (This & 0xF) << (4 * i);
        }
        // For a 2- or 4-wide operand, we need to repeat the vector elements
        // as which ones are used depends on the position of the other
        // operand in its oword.
        switch (NumElements) {
          case 2:
            Packed = Packed * 0x01010101;
            break;
          case 4:
            Packed = Packed * 0x00010001;
            break;
        }
        Code.push_back((uint8_t)(Signed == UNSIGNED ? ISA_TYPE_UV : ISA_TYPE_V));
        Code.push_back((uint32_t)Packed);
        return;
      }
      // Packed float vector.
      assert(VT->getElementType()->isFloatTy() &&
             (NumElements == 1 || NumElements == 2 || NumElements == 4));
      unsigned Packed = 0;
      for (unsigned i = 0; i != 4; ++i) {
        auto CFP =
            dyn_cast<ConstantFP>(V->getAggregateElement(i % NumElements));
        if (!CFP) // Undef
          continue;
        const APFloat &FP = CFP->getValueAPF();
        Packed |= get8bitPackedFloat(FP.convertToFloat()) << (i * 8);
      }
      Code.push_back((uint8_t)ISA_TYPE_VF);
      Code.push_back((uint32_t)Packed);
      return;
    }
    // Splatted (or single element) vector. Use the scalar value.
    T = VT->getElementType();
    V = Splat;
  }

  if (isDerivedFromUndef(V))
    V = Constant::getNullValue(V->getType());

  // We have a scalar constant.
  if (IntegerType *IT = dyn_cast<IntegerType>(T)) {
    ConstantInt *CI = cast<ConstantInt>(V);
    // I think we need to use the appropriate one of getZExtValue or
    // getSExtValue to avoid an assert on very large 64 bit values...
    int64_t Val = Signed == UNSIGNED ? CI->getZExtValue() : CI->getSExtValue();
    TypeDetails TD(Func->getParent()->getDataLayout(), IT, Signed);
    Code.push_back((uint8_t)TD.VisaType);
    Code.push_back((uint32_t)Val);
    if (IT->getPrimitiveSizeInBits() == 64)
      Code.push_back((uint32_t)(Val >> 32));
  } else {
    ConstantFP *CF = cast<ConstantFP>(V);
    if (T->isFloatTy()) {
      union { float f; uint32_t i; } Val;
      Val.f = CF->getValueAPF().convertToFloat();
      Code.push_back((uint8_t)ISA_TYPE_F);
      Code.push_back(Val.i);
    } else if (T->isHalfTy()) {
      uint16_t Val((uint16_t)(CF->getValueAPF().bitcastToAPInt().getZExtValue()));
      Code.push_back((uint8_t)ISA_TYPE_HF);
      Code.push_back((uint32_t)Val);
    } else {
      assert(T->isDoubleTy());
      union { double f; uint64_t i; } Val;
      Val.f = CF->getValueAPF().convertToDouble();
      Code.push_back((uint8_t)ISA_TYPE_DF);
      Code.push_back((uint32_t)Val.i);
      Code.push_back((uint32_t)(Val.i >> 32));
    }
  }
}

/***********************************************************************
 * writeAddressOperand : write an address register operand
 */
void VisaFuncWriter::writeAddressOperand(Value *V)
{
  GenXVisaRegAlloc::RegNum Reg = RegAlloc->getRegNumForValue(V, DONTCARESIGNED);
  assert(Reg.Category == RegCategory::ADDRESS);
  unsigned Width = 1;
  if (VectorType *VT = dyn_cast<VectorType>(V->getType()))
    Width = VT->getNumElements();
  writeByte(CLASS_ADDRESS); // tag+modifiers
  writeShort(Reg.Num); // id (register number)
  writeByte(0); // offset
  writeByte(llvm::log2(Width)); // width
}

/***********************************************************************
 * writePredicateOperand : write a predicate register operand
 */
void VisaFuncWriter::writePredicateOperand(Value *V)
{
  auto Reg = RegAlloc->getRegNumForValue(V, DONTCARESIGNED);
  writePredicateOperand(Reg);
}

/***********************************************************************
 * writePredicateOperand : write a predicate register operand
 */
void VisaFuncWriter::writePredicateOperand(GenXVisaRegAlloc::RegNum Reg)
{
  assert(Reg.Category == RegCategory::PREDICATE);
  writeByte(CLASS_PREDICATE); // tag+modifiers
  writeShort(Reg.Num); // id (register number)
}

/***********************************************************************
 * writeOpcode : write a vISA opcode byte
 */
void VisaFuncWriter::writeOpcode(int Val)
{
  // Ensure that the last label does not get merged with the next one now we
  // know that there is code in between.
  LastLabel = -1;
  // Check if we have a pending debug location.
  if (PendingLine) {
    // Do the source location debug info with vISA FILE and LOC instructions.
    if (PendingFilename != "" && (PendingFilename != LastFilename
          || PendingDirectory != LastDirectory)) {
      SmallString<256> Filename;
      // Bodge here to detect Windows absolute path even when built on cygwin.
      if (sys::path::is_absolute(PendingFilename)
          || (PendingFilename.size() > 2 && PendingFilename[1] == ':'))
        Filename = PendingFilename;
      else {
        Filename = PendingDirectory;
        sys::path::append(Filename, PendingFilename);
      }
      writeByte(ISA_FILE);
      std::string FilenameStr = Filename.str();
      writeInt(getStringIdx(FilenameStr, /*Limit64=*/false));
      LastDirectory = PendingDirectory;
      LastFilename = PendingFilename;
    }
    if (PendingLine != LastLine) {
      writeByte(ISA_LOC);
      writeInt(PendingLine);
      LastLine = PendingLine;
      PendingLine = 0;
    }
  }
  // Write the opcode.
  writeByte(Val);
}

void VisaFuncWriter::emitOptimizationHints() {
  if (skipOptWithLargeBlock(*FG))
    return;

  // Track rp considering byte variable widening.
  PressureTracker RP(*FG, Liveness, /*ByteWidening*/ true);
  const std::vector<LiveRange *> &WidenLRs = RP.getWidenVariables();

  for (auto LR : WidenLRs) {
    SimpleValue SV = *LR->value_begin();
    auto RN = RegAlloc->getRegNumForValueOrNull(SV);
    // This variable is being used in or crossing a high register pressure
    // region. Set an optimization hint not to widen it.
    if (!RN.isNull() && RP.intersectWithRedRegion(LR)) {
      RegAlloc->addAttr(RN, getStringIdx("NoWidening"), "");
      RP.decreasePressure(LR);
    }
  }
}

/***********************************************************************
 * VisaFuncWriter::getStringIdx : add/find string in string table and return index
 *
 * Enter:   Str = the string
 *          Limit64 = whether to limit to 64 bytes
 */
unsigned VisaFuncWriter::getStringIdx(std::string Str, bool Limit64)
{
  // vISA is limited to 64 byte strings. But old fe-compiler seems to ignore
  // that for source filenames.
  if (Limit64 && Str.size() > 64)
    Str.erase(64);
  std::pair<std::string, unsigned> Val(Str, Strings.size());
  std::pair<Strings_t::iterator, bool> Found = Strings.insert(Val);
  return Found.first->second;
}

/***********************************************************************
 * VisaFuncWriter::getLoops : get loop info for given function, cacheing in
 *      Loops map
 */
LoopInfoBase<BasicBlock, Loop> *VisaFuncWriter::getLoops(Function *F)
{
  auto LoopsEntry = &Loops[F];
  if (!*LoopsEntry) {
    auto DT = DTs->getDomTree(F);
    *LoopsEntry = new LoopInfoBase<BasicBlock, Loop>;
    (*LoopsEntry)->analyze(*DT);
  }
  return *LoopsEntry;
}

