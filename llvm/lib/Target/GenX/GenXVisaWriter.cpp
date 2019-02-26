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
/// GenXVisaWriter
/// --------------
///
/// This pass implements the final writing of the vISA file.
///
/// This is a module pass, so it runs once for the whole vISA file. It picks up
/// the VisaFuncWriters (one per kernel) from GenXModule, and constructs the
/// overall vISA file.
///
//===----------------------------------------------------------------------===//

#include <vector>
#include "GenX.h"
#include "GenXModule.h"
#include "GenXVisa.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
using namespace llvm;
using namespace visa;

namespace {
  class GenXVisaWriter : public ModulePass {
    raw_pwrite_stream &Out;
  public:
    static char ID;
    explicit GenXVisaWriter(raw_pwrite_stream &o) :
      ModulePass(ID), Out(o) {}

    virtual StringRef getPassName() const { return "GenX vISA writer"; }

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<GenXModule>();
      AU.setPreservesAll();
    }

    bool runOnModule(Module &M) { writeModule(&M); return false; }

  private:
    void writeModule(Module *M);

  };
} // end anonymous namespace.

char GenXVisaWriter::ID = 0;

ModulePass *llvm::createGenXVisaWriterPass(raw_pwrite_stream &o) {
  return new GenXVisaWriter(o);
}

/***********************************************************************
 * writeModule : write whole module
 */
void GenXVisaWriter::writeModule(Module *M)
{
  GenXModule *GM = getAnalysisIfAvailable<GenXModule>();
  assert(GM && "GenXModule not run");
  // Count the kernels and functions.
  uint16_t NumKernels = 0, NumFuncs = 0;
  // Create the kernels and functions.
  for (GenXModule::iterator i = GM->begin(), e = GM->end(); i != e; i++)
    if ((*i)->isKernel())
      NumKernels++;
    else
      NumFuncs++;
  // Count size of header then kernel and function bodies, so we can assign an
  // offset to each.
  unsigned Pos = 6; // size of magic, major version and minor version
  // Func/kernel headers
  Pos += 2; // for the num_kernels field
  Pos += 2; // for the num_functions field
  for (GenXModule::iterator i = GM->begin(), e = GM->end(); i != e; i++)
    Pos += (*i)->getHeaderSize();
  // Variables (only in header).
  Pos += 2; // for the num_variables field
  // Func/kernel bodies
  for (GenXModule::iterator i = GM->begin(), e = GM->end(); i != e; i++) {
    (*i)->setOffset(Pos);
    Pos += (*i)->getBodySize();
  }

  // Now write the vISA file.
  enum {
    VISA_MAJOR_VERSION = 3,
    VISA_MINOR_VERSION = 6
  };
  // Write the start of the header.
  Out << "CISA";
  Out << (char)VISA_MAJOR_VERSION;
  Out << (char)VISA_MINOR_VERSION;
  // Write the header's kernel array.
  Out.write((const char *)&NumKernels, sizeof(NumKernels));
  for (GenXModule::iterator i = GM->begin(), e = GM->end(); i != e; i++)
    if ((*i)->isKernel())
      (*i)->writeHeader(Out);
  // Write the header's variable array.
  uint16_t NumVariables = 0;
  Out.write((const char *)&NumVariables, sizeof(NumVariables));
  // Write the header's function array.
  Out.write((const char *)&NumFuncs, sizeof(NumFuncs));
  for (GenXModule::iterator i = GM->begin(), e = GM->end(); i != e; i++)
    if (!(*i)->isKernel())
      (*i)->writeHeader(Out);
  // Write the func/kernel bodies.
  for (GenXModule::iterator i = GM->begin(), e = GM->end(); i != e; i++)
    (*i)->writeBody(Out);
}

/***********************************************************************
 * Stream::write : write the stream contents to a formatted_output_stream
 */
void genx::Stream::write(raw_pwrite_stream &Out)
{
  if (V.size())
    Out.write((const char *)&V[0], V.size());
}

