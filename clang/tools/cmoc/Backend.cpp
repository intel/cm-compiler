/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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

static LibOclocWrapper const& getLibOclocWrapper(){
  static std::unique_ptr<const LibOclocWrapper> LibOcloc = nullptr;
  if (!LibOcloc)
    LibOcloc.reset(new LibOclocWrapper());

  return *LibOcloc;
}

static void invokeBE(const std::vector<char> &SPIRV, const std::string &NeoCPU,
                     const std::string &RequiredExtension,
                     const std::string &Options,
                     const std::string &InternalOptions,
                     ILTranslationResult &Result) {
  auto& LibOcloc = getLibOclocWrapper();

  const char *SpvFileName = "cmoc_spirv";

  std::vector<const char *> OclocArgs = {
      "ocloc",
      "compile",
      "-device",
      NeoCPU.c_str(),
      "-spirv_input",
      "-file",
      SpvFileName,
      "-options",
      Options.c_str(),
      "-internal_options",
      InternalOptions.c_str(),
  };

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

  auto Status =
      LibOcloc.invoke(OclocArgs.size(), OclocArgs.data(), 1, &SpvSource,
                      &SpvLen, &SpvFileName, 0, nullptr, nullptr, nullptr,
                      &NumOutputs, &DataOutputs, &LenOutputs, &NameOutputs);

  auto *NameOutputsEnd = NameOutputs + NumOutputs;
  auto LogP =
      std::find(NameOutputs, NameOutputsEnd, llvm::StringRef("stdout.log"));
  if (LogP != NameOutputsEnd) {
    auto LogIndex = LogP - NameOutputs;
    llvm::StringRef Log{reinterpret_cast<char *>(DataOutputs[LogIndex]),
                        static_cast<size_t>(LenOutputs[LogIndex])};
    llvm::errs() << Log;
  }

  if (Status != 0) {
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
composeInternalOptions(const llvm::Optional<std::string> &BinFormat,
                       const std::vector<std::string> &BackendOptions,
                       const std::string &Features, bool TimePasses,
                       bool PrintStats, const std::string &StatsFile) {
  std::string InternalOptions;

  if (BinFormat)
    InternalOptions.append(" -binary-format=").append(BinFormat.getValue());

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

  if (PrintStats)
    InternalOptions.append(" -print-stats");

  if (!StatsFile.empty())
    InternalOptions.append(" -stats-file=" + StatsFile);

  return InternalOptions;
}

void translateIL(const std::string &CPUName, int RevId,
                 const llvm::Optional<std::string> &BinaryFormat,
                 const std::string &Features, const std::string &APIOptions,
                 const std::vector<std::string> &BackendOptions,
                 const std::vector<char> &SPIRV_IR, InputKind IK,
                 bool TimePasses, bool PrintStats, const std::string &StatsFile,
                 ILTranslationResult &Result) {
  if (isCmocDebugEnabled()) {
    llvm::errs() << "requested platform for translateIL: " << CPUName << "\n";
    llvm::errs() << "requested runtime for translateIL: "
                 << BinaryFormat.getValueOr("default") << "\n";
  }

  const std::string Options = composeOptions(APIOptions);
  const std::string InternalOptions = composeInternalOptions(
      BinaryFormat, BackendOptions, Features, TimePasses, PrintStats, StatsFile);

  if (isCmocDebugEnabled()) {
    llvm::errs() << "IGC Translation Options: " << Options << "\n";
    llvm::errs() << "IGC Translation Internal: " << InternalOptions << "\n";
  }

  const std::string RequiredExtension = ".bin";

  invokeBE(SPIRV_IR, CPUName, RequiredExtension, Options, InternalOptions,
           Result);
}

std::string oclocQuery(const char *request) {
  auto& LibOcloc = getLibOclocWrapper();

  std::vector<const char *> OclocArgs;
  OclocArgs.push_back("ocloc");
  OclocArgs.push_back("query");
  OclocArgs.push_back(request);

  uint32_t NumOutputs = 0;
  uint8_t **DataOutputs = nullptr;
  uint64_t *LenOutputs = nullptr;
  char **NameOutputs = nullptr;

  if (LibOcloc.invoke(OclocArgs.size(), OclocArgs.data(), 0, nullptr, nullptr,
                      nullptr, 0, nullptr, nullptr, nullptr, &NumOutputs,
                      &DataOutputs, &LenOutputs, &NameOutputs)) {
    FatalError("Call to oclocInvoke failed");
  }

  assert(NumOutputs == 1);
  std::string ret(*DataOutputs, *DataOutputs + *LenOutputs);

  if (LibOcloc.freeOutput(&NumOutputs, &DataOutputs, &LenOutputs, &NameOutputs))
    FatalError("Call to oclocFreeOutput failed");

  return ret;
}
std::string getOclocDriverVersion() { return oclocQuery("OCL_DRIVER_VERSION"); }
std::string getOclocRevision() { return oclocQuery("NEO_REVISION"); }
