//===-- igcmc.cpp - Implement the LLVM INTEL Graphics GEN Code Generator --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This implements a c-interface for generating vISA or native GEN assembly
// language code, given LLVM bitcode or spir-v input.
//
//===----------------------------------------------------------------------===//
#include "igcmc.h"

#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/IntrinsicsGenX.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"
#include "llvm/CodeGen/CommandFlags.def"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SPIRV.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Scalar.h"

#include <memory>
#include <sstream>
#include <vector>
#include <string>

using namespace llvm;

namespace {

class output_stream {
  raw_svector_ostream os;
  SmallVector<char, 1024> s;
public:
  output_stream() : os(s) {}
  ~output_stream() = default;

  raw_pwrite_stream &get() { return os; }
  const char *data() { return s.data(); }
  size_t size() const { return s.size(); }
};

/// The compilation context implementation.
struct JITContext {
  BumpPtrAllocator Allocator;

  // The string pool within the context.
  std::vector<std::string *> strings;

  JITContext() {}

  ~JITContext() {
    // Release non-primitive allocations within the context.
    for (auto p : strings)
      p->~basic_string();
  }

  // get a null ended c-string.
  const char *get_string(StringRef s) {
    std::string *p = new (Allocator) std::string(s);
    strings.push_back(p);
    return p->c_str();
  }

  // get a plain byte array.
  char *get_binary(size_t n) {
    char *p = new (Allocator) char[n];
    return p;
  }

  // get an empty cm_jit_info object.
  cmc_jit_info *get_jit_info() {
    cmc_jit_info *p = new (Allocator) cmc_jit_info;
    p->context = this;
    return p;
  }

  // get an array of kernel info objects.
  cmc_kernel_info *
  get_kernel_info(const std::vector<StringRef> &kernel_names,
                  const std::vector<std::vector<StringRef>> &arg_descs) {
    size_t num_kernels = kernel_names.size();
    cmc_kernel_info *p = new (Allocator) cmc_kernel_info[num_kernels];
    for (size_t i = 0; i < num_kernels; ++i) {
      p[i].name = get_string(kernel_names[i]);
      p[i].num_arg_desc = arg_descs[i].size();
      p[i].arg_desc = new (Allocator) const char *[arg_descs[i].size()];
      for (size_t j = 0; j < arg_descs[i].size(); ++j)
        p[i].arg_desc[j] = get_string(arg_descs[i][j]);
    }
    return p;
  }
};

} // namespace

// Utility function to tell whether a Function is a kernel.
static bool isKernel(Function *F) {
  // We use DLLExport to represent a kernel in LLVM IR.
  return F->hasDLLExportStorageClass();
}

cmc_error_t cmc_load_and_compile(const char *input, size_t input_size,
                                 const char *const compile_options,
                                 cmc_jit_info **output) {
  // Initialize llvm
  LLVMContext Context;
  LLVMInitializeGenXTarget();
  LLVMInitializeGenXTargetInfo();

  // Parse options

  // Parse the input stream
  std::unique_ptr<Module> M;
  {
    StringRef spirv_input = StringRef(input, input_size);
    std::istringstream IS(spirv_input);
    std::string ErrMsg;
    Module *SpirM = nullptr;
    if (!readSPIRV(Context, IS, SpirM, ErrMsg))
      return cmc_error_t::CMC_ERROR_READING_SPIRV;
    if (verifyModule(*SpirM))
      return cmc_error_t::CMC_ERROR_BROKEN_INPUT_IR;
    // Mark all kernels with attribute oclrt.
    for (auto &F : SpirM->getFunctionList())
      if (F.hasDLLExportStorageClass())
        F.addFnAttr("oclrt", "true");
    M.reset(SpirM);
  }

  // Setup the target machine to compile the input IR.
  output_stream os;
  {
    std::string TargetTriple = M.get()->getTargetTriple();
    if (!TargetTriple.empty())
      M->setTargetTriple(Triple::normalize(TargetTriple));
    Triple TheTriple = Triple(M->getTargetTriple());
    if (TheTriple.getTriple().empty())
      TheTriple.setTriple(sys::getDefaultTargetTriple());

    // Update to gen32 or gen64 arch.
    TheTriple.isArch32Bit() ? TheTriple.setArch(Triple::genx32)
                            : TheTriple.setArch(Triple::genx64);
    M->setTargetTriple(TheTriple.getTriple());

    // Get the target specific parser.
    std::string Error;
    auto TheTarget = TargetRegistry::lookupTarget(MArch, TheTriple, Error);
    if (!TheTarget)
      return CMC_ERROR_IN_LOADING_TARGET;

    // TODO: setup the following options from cmd line options parsed.
    std::string CPUStr = getCPUStr();
    std::string FeaturesStr = getFeaturesStr();
    CodeGenOpt::Level OLvl = CodeGenOpt::Default;
    TargetOptions Options = InitTargetOptionsFromCodeGenFlags();

    std::unique_ptr<TargetMachine> TM(TheTarget->createTargetMachine(
        TheTriple.getTriple(), CPUStr, FeaturesStr, Options, getRelocModel(),
        getCodeModel(), OLvl));
    assert(TM && "Could not allocate target machine!");

    // Add the target data from the target machine, if it exists, or the module.
    M->setDataLayout(TM->createDataLayout());

    // Build up all of the passes that we want to do to the module.
    legacy::PassManager PM;

    // Add an appropriate TargetLibraryInfo pass for the module's triple.
    TargetLibraryInfoImpl TLII(Triple(M->getTargetTriple()));
    PM.add(new TargetLibraryInfoWrapperPass(TLII));

    // Recompute argument offset.
    unsigned Width = 32;
    PM.add(createCMKernelArgOffsetPass(Width, /* OCLCodeGen*/true));

    auto FileType = TargetMachine::CodeGenFileType::CGFT_AssemblyFile;
    if (TM->addPassesToEmitFile(PM, os.get(), FileType, /*NoVerify*/ true))
      return cmc_error_t::CMC_ERROR_IN_COMPILING_IR;

    PM.run(*M);
  }

  // Output the result.
  {
    JITContext *context = new JITContext;
    cmc_jit_info *info = context->get_jit_info();
    char *bin = context->get_binary(os.size());

    // vISA binary
    std::copy(os.data(), os.data() + os.size(), bin);
    info->binary = (void *) bin;
    info->binary_size = os.size();
    info->visa_major_version = genx::VISA_MAJOR_VERSION;
    info->visa_minor_version = genx::VISA_MINOR_VERSION;

    // kernels
    std::vector<StringRef> kernel_names;
    std::vector<std::vector<StringRef>> arg_descs;

    // Collect all kernel names and argument attributes.
    for (auto &F : M.get()->getFunctionList()) {
      if (!F.empty() && genx::isKernel(&F)) {
        genx::KernelMetadata KM(&F);
        kernel_names.push_back(KM.getName());

        std::vector<StringRef> descs;
        // TODO: replace with OCL argument attributes.
        for (auto AI = F.arg_begin(); AI != F.arg_end(); ++AI)
          descs.push_back(AI->getName());
        arg_descs.push_back(descs);
      }
    }

    info->num_kernels = kernel_names.size();
    info->kernel_info = context->get_kernel_info(kernel_names, arg_descs);

    *output = info;
  }

  return CMC_SUCCESS;
}

const char *cmc_get_error_string(cmc_error_t err) {
  switch (err) {
  case CMC_SUCCESS:
    return "success";
  case CMC_ERROR:
    return "unspecified error";
  case CMC_ERROR_READING_SPIRV:
    return "error in reading SPIR-V stream";
  case CMC_ERROR_BROKEN_INPUT_IR:
    return "input IR is broken";
  case CMC_ERROR_IN_LOADING_TARGET:
    return "error in loading GenX target";
  case CMC_ERROR_IN_COMPILING_IR:
    return "error in compiling input IR";
  default:
    break;
  }
  return "unknown error";
}

cmc_error_t cmc_free_jit_info(cmc_jit_info *output) {
  if (output)
    delete reinterpret_cast<JITContext *>(output->context);
  return CMC_SUCCESS;
}
