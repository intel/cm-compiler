/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/


#include "Common.h"

#ifdef USE_OCLOC_API_HEADER
#include "ocloc_api.h"
#endif

#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/raw_ostream.h"

#include <fstream>
#include <string>
#include <unordered_map>

#include <cassert>

namespace {
struct LibOclocWrapper {
#ifdef USE_OCLOC_API_HEADER
  using invokePtr = decltype(::oclocInvoke) *;
  using freeOutputPtr = decltype(::oclocFreeOutput) *;
#else
  using invokePtr = int (*)(unsigned, const char **, const uint32_t,
                            const uint8_t **, const uint64_t *, const char **,
                            const uint32_t, const uint8_t **, const uint64_t *,
                            const char **, uint32_t *, uint8_t ***, uint64_t **,
                            char ***);
  using freeOutputPtr = int (*)(uint32_t *, uint8_t ***, uint64_t **, char ***);
#endif

  invokePtr invoke;
  freeOutputPtr freeOutput;

  LibOclocWrapper() {
    const std::string LibPath = getLibOclocName();
    std::string Err;
    auto Lib =
        llvm::sys::DynamicLibrary::getPermanentLibrary(LibPath.c_str(), &Err);
    if (!Lib.isValid())
      FatalError(Err);

    invoke = reinterpret_cast<invokePtr>(Lib.getAddressOfSymbol("oclocInvoke"));
    if (!invoke)
      FatalError("oclocInvoke symbol is missing");
    freeOutput = reinterpret_cast<freeOutputPtr>(
        Lib.getAddressOfSymbol("oclocFreeOutput"));
    if (!freeOutput)
      FatalError("oclocFreeOutput symbol is missing");
  }

private:
  static std::string getLibOclocName() {
    std::string Dir;
    if (auto EnvDir = llvm::sys::Process::GetEnv("CMOC_OCLOC_DIR"))
      Dir = EnvDir.getValue();
    llvm::SmallString<32> Path;
    llvm::sys::path::append(Path, Dir, LIBOCLOC_NAME);
    return Path.str().str();
  }
};
} // namespace

// clang-format off
const std::unordered_map<std::string, std::string> CmToNeoCPU{
    {"BDW", "bdw"},
    {"BXT", "bxt"},
    {"GLK", "glk"},
    {"KBL", "kbl"},
    {"SKL", "skl"},
    {"ICLLP", "icllp"},
    {"TGLLP", "tgllp"},
};
// clang-format on

static std::string translateCPU(const std::string &CPU) {
  auto It = CmToNeoCPU.find(CPU);
  if (It == CmToNeoCPU.end())
    FatalError("Unexpected CPU model: " + CPU);
  return It->second;
}

// Try to guess revision id for given stepping.
// Relies on enumeration values inside driver since
// ocloc accepts these numeric values instead of letter codes
static std::string translateStepping(const std::string &CPU) {
  return "";
}

// Get correct file extension to search for in ocloc outputs.
// CM requires plain binary from IGC (isa binary) -- .gen.
// OCL and ZE should work with binary prepared by NEO -- .bin.
// .bin wraps things like .gen, .dbg and others.
static std::string translateRequiredExtension(const std::string &BinaryFormat) {
  return llvm::StringSwitch<std::string>(BinaryFormat)
      .Case("cm", ".gen")
      .Cases("ocl", "ze", ".bin");
}

template <typename OsT>
static void printEscapedString(OsT &OS, const char *Str, const char Escape) {
  OS << Escape;
  while (char C = *Str++)
    (C == Escape ? OS << '\\' : OS) << C;
  OS << Escape;
}

// Print strings wrapping each with Escape characters and prepending
// '\' to each of escape charaters inside strings.
template <typename OsT>
static void printEscapedArgs(OsT &OS, const std::vector<const char *> &Args,
                             const char Escape) {
  for (auto *Arg : Args) {
    printEscapedString(OS, Arg, Escape);
    OS << ' ';
  }
  OS << '\n';
}

// Copy outputs. Required file should have provided extension (it is
// different for different binary kinds).
static void saveOutputs(uint32_t NumOutputs, uint8_t **DataOutputs,
                        uint64_t *LenOutputs, char **NameOutputs,
                        const std::string &RequiredExtension,
                        ILTranslationResult &Result) {
  llvm::ArrayRef<const uint8_t *> OutBins{DataOutputs, NumOutputs};
  llvm::ArrayRef<uint64_t> OutLens{LenOutputs, NumOutputs};
  llvm::ArrayRef<const char *> OutNames{NameOutputs, NumOutputs};
  auto Zip = llvm::zip(OutBins, OutLens, OutNames);
  using ZipTy = typename decltype(Zip)::value_type;
  auto BinIt =
      std::find_if(Zip.begin(), Zip.end(), [&RequiredExtension](ZipTy File) {
        llvm::StringRef Name{std::get<2>(File)};
        return Name.endswith(RequiredExtension);
      });
  assert(BinIt != Zip.end() && "Output binary is missing");

  llvm::ArrayRef<uint8_t> BinRef{std::get<0>(*BinIt),
                                 static_cast<std::size_t>(std::get<1>(*BinIt))};
  Result.KernelBinary.assign(BinRef.begin(), BinRef.end());
}

static void invokeBE(const std::vector<char> &SPIRV, const std::string &NeoCPU,
                     const std::string &RevId,
                     const std::string &RequiredExtension,
                     const std::string &Options,
                     const std::string &InternalOptions,
                     ILTranslationResult &Result) {
  const LibOclocWrapper LibOcloc;

  const char *SpvFileName = "cmoc_spirv";

  std::vector<const char *> OclocArgs;
  OclocArgs.push_back("ocloc");
  OclocArgs.push_back("compile");
  OclocArgs.push_back("-device");
  OclocArgs.push_back(NeoCPU.c_str());
  if (!RevId.empty()) {
    OclocArgs.push_back("-revision_id");
    OclocArgs.push_back(RevId.c_str());
  }
  OclocArgs.push_back("-spirv_input");
  OclocArgs.push_back("-file");
  OclocArgs.push_back(SpvFileName);
  OclocArgs.push_back("-options");
  OclocArgs.push_back(Options.c_str());
  OclocArgs.push_back("-internal_options");
  OclocArgs.push_back(InternalOptions.c_str());

  if (isCmocDebugEnabled()) {
    llvm::errs() << "oclocInvoke options: ";
    printEscapedArgs(llvm::errs(), OclocArgs, '"');
  }

  uint32_t NumOutputs = 0;
  uint8_t **DataOutputs = nullptr;
  uint64_t *LenOutputs = nullptr;
  char **NameOutputs = nullptr;

  static_assert(alignof(uint8_t) == alignof(char), "Possible unaligned access");
  auto *SpvSource = reinterpret_cast<const uint8_t *>(SPIRV.data());
  const uint64_t SpvLen = SPIRV.size();
  if (LibOcloc.invoke(OclocArgs.size(), OclocArgs.data(), 1, &SpvSource,
                      &SpvLen, &SpvFileName, 0, nullptr, nullptr, nullptr,
                      &NumOutputs, &DataOutputs, &LenOutputs, &NameOutputs)) {
    // no need to print build log here if -q option is not passed
    // and now it is not
    FatalError("Call to oclocInvoke failed");
  }
  saveOutputs(NumOutputs, DataOutputs, LenOutputs, NameOutputs,
              RequiredExtension, Result);
  if (LibOcloc.freeOutput(&NumOutputs, &DataOutputs, &LenOutputs, &NameOutputs))
    FatalError("Call to oclocFreeOutput failed");
}

static std::string composeOptions(const std::string &APIOptions) {
  std::string Options{"-vc-codegen "};

  Options.append(APIOptions);

  auto AuxVCOptions = llvm::sys::Process::GetEnv("CM_VC_API_OPTIONS");
  if (AuxVCOptions) {
    Options.append(" ").append(AuxVCOptions.getValue());
  }

  return Options;
}

static std::string
composeInternalOptions(const std::string &BinFormat,
                       const std::vector<std::string> &BackendOptions,
                       const std::string &Features, bool TimePasses) {
  std::string InternalOptions;
  InternalOptions.append(" -binary-format=").append(BinFormat);

  if (!BackendOptions.empty())
    InternalOptions +=
        " -llvm-options='" + llvm::join(BackendOptions, " ") + "'";

  if (!Features.empty())
    InternalOptions.append(" -target-features=").append(Features);

  if (auto EnvInternalOptions =
          llvm::sys::Process::GetEnv("CM_INTERNAL_OPTIONS"))
    InternalOptions.append(" ").append(EnvInternalOptions.getValue());

  if (TimePasses)
    InternalOptions.append(" -ftime-report");

  return InternalOptions;
}

void translateIL(const std::string &CPUName, int RevId,
                 const std::string &BinaryFormat, const std::string &Features,
                 const std::string &APIOptions,
                 const std::vector<std::string> &BackendOptions,
                 const std::vector<char> &SPIRV_IR, InputKind IK,
                 bool TimePasses, ILTranslationResult &Result) {

  if (isCmocDebugEnabled()) {
    llvm::errs() << "requested platform for translateIL: " << CPUName << "\n";
    llvm::errs() << "requested runtime for translateIL: " << BinaryFormat
                 << "\n";
  }

  const std::string Options = composeOptions(APIOptions);
  const std::string InternalOptions = composeInternalOptions(
      BinaryFormat, BackendOptions, Features, TimePasses);

  if (isCmocDebugEnabled()) {
    llvm::errs() << "IGC Translation Options: " << Options << "\n";
    llvm::errs() << "IGC Translation Internal: " << InternalOptions << "\n";
  }

  const std::string RequiredExtension =
      translateRequiredExtension(BinaryFormat);
  const std::string NeoCPU = translateCPU(CPUName);

  // translate revid from CPU name if available
  std::string RevIdStr = translateStepping(CPUName);

  // if revid passed as option, prefer it
  if (RevId > 0)
    RevIdStr = std::to_string(RevId);

  invokeBE(SPIRV_IR, NeoCPU, RevIdStr, RequiredExtension, Options,
           InternalOptions, Result);
}
