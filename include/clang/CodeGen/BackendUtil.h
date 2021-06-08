/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

SPDX-License-Identifier: MIT
============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

#ifndef LLVM_CLANG_CODEGEN_BACKENDUTIL_H
#define LLVM_CLANG_CODEGEN_BACKENDUTIL_H

#include "clang/Basic/LLVM.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include <memory>

namespace llvm {
  class BitcodeModule;
  template <typename T> class Expected;
  class Module;
  class MemoryBufferRef;
}

namespace clang {
  class DiagnosticsEngine;
  class HeaderSearchOptions;
  class CodeGenOptions;
  class TargetOptions;
  class LangOptions;

  enum BackendAction {
    Backend_EmitAssembly,  ///< Emit native assembly files
    Backend_EmitBC,        ///< Emit LLVM bitcode files
    Backend_EmitLL,        ///< Emit human-readable LLVM assembly
    Backend_EmitNothing,   ///< Don't emit anything (benchmarking mode)
    Backend_EmitMCNull,    ///< Run CodeGen, but don't emit anything
    Backend_EmitObj,       ///< Emit native object files
    Backend_EmitSPIRV      ///< Emit spirv-binary files
  };

  void EmitBackendOutput(DiagnosticsEngine &Diags, const HeaderSearchOptions &,
                         const CodeGenOptions &CGOpts,
                         const TargetOptions &TOpts, const LangOptions &LOpts,
                         const llvm::DataLayout &TDesc, llvm::Module *M,
                         BackendAction Action,
                         std::unique_ptr<raw_pwrite_stream> OS);

  void EmbedBitcode(llvm::Module *M, const CodeGenOptions &CGOpts,
                    llvm::MemoryBufferRef Buf);

  llvm::Expected<llvm::BitcodeModule>
  FindThinLTOModule(llvm::MemoryBufferRef MBRef);
  llvm::BitcodeModule *
  FindThinLTOModule(llvm::MutableArrayRef<llvm::BitcodeModule> BMs);
}

#endif
