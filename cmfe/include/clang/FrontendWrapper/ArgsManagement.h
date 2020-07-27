#ifndef LLVM_FRONTEND_WRAPPER_ARGS_MANAGEMENT_H
#define LLVM_FRONTEND_WRAPPER_ARGS_MANAGEMENT_H

#include "clang/FrontendWrapper/Interface.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <unordered_set>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace wrapper {

using IInputArgs = Intel::CM::ClangFE::IInputArgs;
using IOutputArgs = Intel::CM::ClangFE::IOutputArgs;
using IDriverInvocation = Intel::CM::ClangFE::IDriverInvocation;

class OutputArgsBuilder final {
public:
  using ErrT = IOutputArgs::ErrT;

private:
  llvm::SmallString<0> IR;
  std::string Log;
  ErrT Status;

public:

  const std::string &getLog() const { return Log; }
  const llvm::SmallString<0> &getIR() const { return IR; }
  ErrT getStatus() const { return Status; }
  // unfortunately llvm::raw_string_ostream has no move constructor
  std::unique_ptr<llvm::raw_string_ostream> getLogStream() {
    return llvm::make_unique<llvm::raw_string_ostream>(Log);
  }
  std::unique_ptr<llvm::raw_svector_ostream> getIRStream() {
    return llvm::make_unique<llvm::raw_svector_ostream>(IR);
  }
  void setStatus(bool ClangSuccess) {
    Status = ClangSuccess ? ErrT::SUCCESS : ErrT::COMPILE_PROGRAM_FAILURE;
  }

  ~OutputArgsBuilder() {}
};

class OutputArgsImpl final : public IOutputArgs {
  BufferT IR;
  StrT Log;
  ErrT Status;

  // NOTE: this .ctr is expected to be private,
  // this emphasises that creating such object on the stack is prohibited
  OutputArgsImpl(const OutputArgsBuilder &OutArgsBuilder)
      : IR(OutArgsBuilder.getIR().begin(), OutArgsBuilder.getIR().end()),
        Log(OutArgsBuilder.getLog()), Status(OutArgsBuilder.getStatus()) {}

public:
  static OutputArgsImpl* create(const OutputArgsBuilder &OutArgsBuilder) {
    return new OutputArgsImpl(OutArgsBuilder);
  }

  ErrT getStatus() const override { return Status; }
  const BufferT &getIR() const override { return IR; }
  const StrT &getLog() const override { return Log; }
  void discard() override { delete this; }

  ~OutputArgsImpl() {}
};

class IDriverInvocationImpl final : public IDriverInvocation {

  SeqStrT FrontendArgs;
  SeqStrT BackendArgs;

  InputTypeT InputType = InputTypeT::OTHER;
  OutputTypeT OutputType = OutputTypeT::OTHER;

  TargetRuntimeT TargetRuntime = TargetRuntimeT::CM;
  StrT TargetArch;

  StrT InputFilename;
  StrT OutputFilename;

  StrT TargetFeaturesStr;

  bool   IsHelpInvocation = false;

  IDriverInvocationImpl() {};

  IDriverInvocationImpl(SeqStrT &&FeArgs, SeqStrT &&BeArgs,
                        InputTypeT InputType, OutputTypeT OutputType,
                        StrT&& InputFilename, StrT&& OutputFilename,
                        TargetRuntimeT TargetRuntime, StrT&& TargetArch,
                        StrT&& TargetFeaturesStr)
    : FrontendArgs(std::move(FeArgs)), BackendArgs(std::move(BeArgs)),
      InputType(std::move(InputType)), OutputType(std::move(OutputType)),
      TargetRuntime(TargetRuntime), TargetArch(std::move(TargetArch)),
      InputFilename(std::move(InputFilename)),
      OutputFilename(std::move(OutputFilename)),
      TargetFeaturesStr(std::move(TargetFeaturesStr)) {}

public:


  static IDriverInvocationImpl* createHelp() {
    auto p = new IDriverInvocationImpl();
    p->IsHelpInvocation = true;
    return p;
  }

  static IDriverInvocationImpl* createInvocation(
          SeqStrT &&FeArgs, SeqStrT &&BeArgs,
          InputTypeT InputType, OutputTypeT OutputType,
          StrT&& InputFilename, StrT&& OutputFilename,
          TargetRuntimeT TargetRuntime, StrT&& TargetArch,
          StrT&& TargetFeaturesStr) {

    auto p = new IDriverInvocationImpl(
        std::move(FeArgs), std::move(BeArgs), InputType, OutputType,
        std::move(InputFilename), std::move(OutputFilename),
        TargetRuntime, std::move(TargetArch), std::move(TargetFeaturesStr));
    return p;
  }

  void discard() override {
    delete this;
  }

  const TargetRuntimeT& getTargetRuntime() const override { return TargetRuntime; }

  const StrT& getTargetArch() const override { return TargetArch; }

  const InputTypeT& getInputType() const override { return InputType; }
  const OutputTypeT& getOutputType() const override { return OutputType; }

  const SeqStrT& getFEArgs() const override { return FrontendArgs; }
  const SeqStrT& getBEArgs() const override { return BackendArgs; }

  const StrT& getInputFilename() const override { return InputFilename; }
  const StrT& getOutputFilename() const override { return OutputFilename; }
  const StrT& getTargetFeaturesStr() const override { return TargetFeaturesStr; }

  bool isHelp() const override { return IsHelpInvocation; }
};

template <typename T> using SeqT = std::vector<T>;

template <typename T> T getSrc(const IInputArgs *);

template <> llvm::StringRef getSrc<llvm::StringRef>(const IInputArgs *Input) {
  const auto &Src = Input->getSrc();
  return llvm::StringRef(Src.c_str(), Src.size());
}

template <typename T> SeqT<T> getCompOpts(const IInputArgs *);

template <>
SeqT<const char *> getCompOpts<const char *>(const IInputArgs *Input) {
  const auto &CompOpts = Input->getCompOpts();
  SeqT<const char *> CStrCompOpts(CompOpts.size());
  std::transform(CompOpts.begin(), CompOpts.end(), CStrCompOpts.begin(),
                 [](const IInputArgs::StrT &str) { return str.c_str(); });
  return CStrCompOpts;
}

template <typename T>
SeqT<IInputArgs::FileT<T>> getExtraFiles(const IInputArgs *);

template <>
SeqT<IInputArgs::FileT<llvm::StringRef>>
getExtraFiles<llvm::StringRef>(const IInputArgs *Input) {
  const auto &Libs = Input->getExtraFiles();
  SeqT<IInputArgs::FileT<llvm::StringRef>> SRLibs;
  std::transform(Libs.begin(), Libs.end(), std::back_inserter(SRLibs),
                 [](const IInputArgs::FileT<IInputArgs::StrT> &Lib) {
                   return IInputArgs::FileT<llvm::StringRef>{
                       llvm::StringRef(Lib.Name.c_str(), Lib.Name.size()),
                       llvm::StringRef(Lib.Src.c_str(), Lib.Src.size())};
                 });
  return SRLibs;
}

} // namespace wrapper

#endif // LLVM_FRONTEND_WRAPPER_ARGS_MANAGEMENT_H
