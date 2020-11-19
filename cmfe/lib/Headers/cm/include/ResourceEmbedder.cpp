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

//
// ResourceEmbedder -- tool for conversion of arbitrary data into C
// character data.
//
// Tool receives a list of input files and produces output file with
// array of characters (actually, array of arrays of characters) and
// auxiliary structures like positions of individual files and mapping
// from name to index.
//
// Conversion is performed by creating array of fixed chunks. Each
// chunk is represented as array of characters. Each chunk is
// initialized with C-string exploiting the fact that C allows to drop
// null terminator if C-string fits into array without it.
//
// char foo[3] = "foo"; // That's it.
//
// Using this, data can be split into such chunks and then compiled
// very quickly. Way more faster than splitting into simple array of
// separate characters.
//
// Division into chunks is required to overcome limitation of standard
// on maximum supported length of string literal. Additionally, MSVC
// compiler does not support even guaranteed by standard lengths.
//
//===----------------------------------------------------------------------===//

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#include <numeric>
#include <vector>

#include <cstdio>

using namespace llvm;

static cl::list<std::string> InputFiles(cl::Positional, cl::OneOrMore,
                                        cl::desc("[file]..."));

static cl::opt<std::string>
    OutputFile("output",
               cl::desc("Output file containing C code for embedding"));

static cl::opt<unsigned>
    MaxChunkSize("chunk-size", cl::init(1024),
                 cl::desc("Maximum size of string chunks"));

static void writeAsOctalSequence(unsigned char C, raw_ostream &Out) {
  char Buf[4];
  int Sz = sprintf(Buf, "%.3hho", C);
  assert(Sz == 3 && "Unexpected number of octal characters");
  assert(Buf[3] == '\0' && "Buf is not null-terminated");
  Out << '\\';
  Out << Buf;
}

// Write arbitrary string as a valid C string literal.
// This function escapes '"', '\' and '?' (to avoid trigraphs)
// printable symbols. Non-printable symbols are written as octal
// escape sequences.
static void writeEscapedCString(StringRef Str, raw_ostream &Out) {
  for (char C : Str) {
    switch (C) {
    case '?':
    case '"':
    case '\\':
      Out << '\\' << C;
      continue;
    }
    if (std::isprint(C)) {
      Out << C;
      continue;
    }
    writeAsOctalSequence(C, Out);
  }
}

namespace {
// Helper class that prints string in chunks accounting for previous
// output. It minimizes number of required chunks.
class ChunkPrinter {
  size_t ChunkSpaceLeft = MaxChunkSize;
  raw_ostream &Out;

public:
  ChunkPrinter(raw_ostream &OutP) : Out(OutP) {}

  void printInChunks(StringRef Str) {
    while (!Str.empty()) {
      if (ChunkSpaceLeft == MaxChunkSize)
        Out << '"';

      const StringRef CurrentChunk = Str.take_front(ChunkSpaceLeft);
      const size_t CurrentChunkSize = CurrentChunk.size();
      writeEscapedCString(CurrentChunk, Out);
      ChunkSpaceLeft -= CurrentChunkSize;
      Str = Str.drop_front(CurrentChunkSize);

      if (ChunkSpaceLeft == 0) {
        Out << "\",\n";
        ChunkSpaceLeft = MaxChunkSize;
      }
    }
  }

  void finalize() {
    if (ChunkSpaceLeft)
      Out << "\",\n";
  }
};
} // namespace

// Print array of arrays of chars initialized with strings.
// Each internal array represents a chunk. Splitting strings in chunks
// allows to overcome limitations of MSVC that fails to compile strings
// that are longer than 65535 characters (including null terminator).
// Implemented technique uses C feature that allows to drop null terminator
// during initialization of array with string if it does not fits.
static std::vector<size_t> printStrings(raw_ostream &Out) {
  Out << "#ifdef CM_GET_RESOURCE_STORAGE\n";

  Out << "static const char ResourceStorage[][" << MaxChunkSize << "] = {\n";
  ChunkPrinter Printer(Out);
  std::vector<size_t> Sizes;
  for (auto &&InputFile : InputFiles) {
    auto InErr = MemoryBuffer::getFile(InputFile);
    if (auto EC = InErr.getError())
      report_fatal_error(EC.message());
    StringRef Buf = InErr.get()->getBuffer();
    Printer.printInChunks(Buf);
    Sizes.push_back(Buf.size());
  }
  Printer.finalize();
  Out << "};\n\n";

  Out << "#endif // CM_GET_RESOURCE_STORAGE\n\n";
  return Sizes;
}

// Print initializer for array of resource descriptors.
static void printDescriptors(const std::vector<size_t> &Sizes,
                             raw_ostream &Out) {
  Out << "#ifdef CM_GET_RESOURCE_DESCS\n";

  std::vector<size_t> Positions;
  Positions.push_back(0);
  std::partial_sum(Sizes.begin(), std::prev(Sizes.end()),
                   std::back_inserter(Positions));
  for (auto &&Desc : llvm::zip(InputFiles, Positions, Sizes)) {
    const std::string &Header = std::get<0>(Desc);
    const std::size_t Pos = std::get<1>(Desc);
    const std::size_t Size = std::get<2>(Desc);
    Out << "{\"";
    writeEscapedCString(Header, Out);
    Out << "\", (const char *)ResourceStorage + " << Pos << ", " << Size
        << "},\n";
  }

  Out << "#endif // CM_GET_RESOURCE_DESCS\n\n";
}

// Print string storage and descriptors in form of "Name, Begin pointer, Size".
static void printStoragePart(raw_ostream &Out) {
  std::vector<size_t> Sizes = printStrings(Out);
  printDescriptors(Sizes, Out);
}

int main(int argc, const char *const *argv) {
  cl::ParseCommandLineOptions(argc, argv);

  if (InputFiles.getNumOccurrences() == 0)
    report_fatal_error("No input files");

  std::error_code EC;
  StringRef Dir = llvm::sys::path::parent_path(OutputFile);
  if (!Dir.empty()) {
    EC = llvm::sys::fs::create_directories(Dir);
    if (EC)
      report_fatal_error(EC.message());
  }
  raw_fd_ostream Out(OutputFile, EC);
  if (EC)
    report_fatal_error(EC.message());

  Out << "// WARNING: this file is autogenerated. Modify sources if changes "
         "required here.\n\n";
  printStoragePart(Out);

  return 0;
}
