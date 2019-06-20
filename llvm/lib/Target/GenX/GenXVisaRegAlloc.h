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
/// GenXVisaRegAlloc
/// ----------------
///
/// GenXVisaRegAlloc is a function group pass that allocates vISA registers to
/// LLVM IR values.
///
/// Before allocating registers, this pass does "extra coalescing", over and above
/// what GenXCoalescing does. Two otherwise independent live ranges that are
/// related by being an operand and the result of the same instruction (and are
/// the same type) get coalesced and thus allocated into the same register.
///
/// However, extra coalescing is not performed when the result of the instruction
/// is used in a non-alu intrinsic, to try and avoid the danger of the jitter
/// needing to add an extra move in the send.
///
/// Other than that, all this pass does is allocate a different vISA register to
/// each LiveRange.
///
/// The pass is also an analysis for GenXVisaFuncWriter to query to find out
/// what vISA register is allocated to a particular Value. In fact, the query
/// from GenXVisaFuncWriter can specify what type it wants the register to be,
/// and it is at that point that an alias is allocated if there is no existing
/// alias of the requested type.
///
/// Finally, there are callbacks in the analysis to generate the vISA variable
/// tables to put into the vISA file.
///
//===----------------------------------------------------------------------===//
#ifndef GENXVISAREGALLOC_H
#define GENXVISAREGALLOC_H

#include "FunctionGroup.h"
#include "GenX.h"
#include "GenXLiveness.h"
#include "GenXModule.h"
#include "llvm/Pass.h"
#include "llvm/IR/IntrinsicsGenX.h"
#include <map>
#include <string>
#include <vector>

namespace llvm {
  class Function;
  namespace genx {
    class LiveRange;

    // Attr : a vISA attribute
    struct Attr {
      unsigned Name;
      std::string Value;
      // Gven the string index of the name, and a 0 terminated value.
      Attr(unsigned Name, Twine Value) : Name(Name), Value(Value.str()) {}
    };

    // Attrs : a load of vISA attributes
    class Attrs {
    public:
      std::vector<Attr> AttrsVec;

      // push_back : add a new attribute
      void push_back(unsigned Name, Twine Value) {
        AttrsVec.push_back(Attr(Name, Value));
      }
      // write : write the attributes into a Stream
      template <typename T>
      void write(genx::Stream *S) const {
        S->push_back((T)AttrsVec.size());
        for (unsigned i = 0; i != AttrsVec.size(); ++i) {
          S->push_back((uint32_t)AttrsVec[i].Name);
          assert(AttrsVec[i].Value.size() <= 255 && "attribute value too long");
          S->push_back((uint8_t)AttrsVec[i].Value.size());
          if (AttrsVec[i].Value.size() > 0)
            S->push_back(AttrsVec[i].Value.data(), AttrsVec[i].Value.size());
        }
      }
    };
  } // genx namespace

  class FunctionPass;
  class raw_ostream;
  class Type;
  class Value;

  FunctionGroupPass *createGenXGroupPrinterPass(raw_ostream &O, const std::string &Banner);

  // GenXVisaRegAlloc : vISA virtual register allocator pass
  class GenXVisaRegAlloc : public FunctionGroupPass {
    FunctionGroup *FG;
    GenXLiveness *Liveness;
    GenXNumbering *Numbering;
    // Reg : a virtual register
    class Reg {
    public:
      unsigned AliasTo; // 0 else link back to what it aliases
      unsigned NextAlias; // 0 else singly linked list of aliases
      genx::Signedness Signed;
      Type *Ty;
      unsigned Alignment; // log2 min alignment requested by user of register
      unsigned Name; // name index
      genx::Attrs Attributes; // attributes
      Reg(Type *Ty = 0, genx::Signedness Signed = genx::DONTCARESIGNED,
          unsigned LogAlignment = 0)
          : AliasTo(0), NextAlias(0), Signed(Signed), Ty(Ty),
            Alignment(LogAlignment), Name(0) {}

      // The number of attributes on this declaration.
      unsigned getNumAttrs() const {
        return static_cast<unsigned>(Attributes.AttrsVec.size());
      }
    };
  private:
    unsigned NumReserved[genx::RegCategory::NUMREALCATEGORIES];
    std::vector<Reg> Regs[genx::RegCategory::NUMREALCATEGORIES];
  public:
    struct RegNum {
      unsigned short Category;
      unsigned short Num;
      RegNum(unsigned Category = 0, unsigned Num = 0) : Category(Category), Num(Num) {}
      bool operator==(const RegNum &Rhs) { return Category == Rhs.Category && Num == Rhs.Num; }
      bool operator!=(const RegNum &Rhs) { return !(*this == Rhs); }
      void print(raw_ostream &OS);
      bool isNull() const { return Category == 0 && Num == 0; }
    };
  private:
    typedef std::map<genx::SimpleValue, RegNum> RegMap_t;
    RegMap_t RegMap;
  public:
    static char ID;
    explicit GenXVisaRegAlloc() : FunctionGroupPass(ID) { }
    virtual StringRef getPassName() const { return "GenX vISA virtual register allocator"; }
    void getAnalysisUsage(AnalysisUsage &AU) const;
    bool runOnFunctionGroup(FunctionGroup &FG);
    // Get the vISA virtual reg number for a value (assert if none)
    RegNum getRegNumForValue(genx::SimpleValue V,
        genx::Signedness Signed = genx::DONTCARESIGNED, Type *OverrideType = 0)
    {
      RegNum RN = getRegNumForValueOrNull(V, Signed, OverrideType);
      assert(RN.Category && "no register allocated for this value");
      return RN;
    }
    // Get the vISA virtual reg number for a value (0 if none)
    RegNum getRegNumForValueOrNull(genx::SimpleValue V,
        genx::Signedness Signed = genx::DONTCARESIGNED, Type *OverrideType = 0);
    // Get the vISA virtual reg number for a value (0 if none), ignoring type
    // and signedness so it can be a const method usable from print().
    RegNum getRegNumForValueUntyped(genx::SimpleValue V) const;
    // Get the signedness of a register.
    genx::Signedness getSigned(RegNum RN);
    // Check if two RegNum values returned by getRegNumForValue are aliased.
    bool areAliased(RegNum R1, RegNum R2);
    // Get the log2 alignment of a register
    unsigned getLogAlignment(RegNum R);
    // Set the name index of a register
    void setName(RegNum R, unsigned Name) {
      Regs[R.Category][R.Num].Name = Name;
    }
    void addAttr(RegNum R, unsigned AttrName, Twine AttrVal) {
      Regs[R.Category][R.Num].Attributes.push_back(AttrName, AttrVal);
    }
    // Build the variable lists in the kernel/function header (parts 1 and 2)
    void buildHeader1(genx::Stream *S);
    void buildHeader2(genx::Stream *S);
    // createPrinterPass : get a pass to print the IR, together with the GenX
    // specific analyses
    virtual Pass *createPrinterPass(raw_ostream &O, const std::string &Banner) const
    { return createGenXGroupPrinterPass(O, Banner); }
    // print : dump the state of the pass. This is used by -genx-dump-regalloc
    virtual void print(raw_ostream &O, const Module *M) const;
  private:
    Type *BoolTy;
    void getLiveRanges(std::vector<genx::LiveRange *> *LRs) const;
    void getLiveRangesForValue(Value *V, std::vector<genx::LiveRange *> *LRs) const;
    void extraCoalescing();
    void allocReg(genx::LiveRange *LR);
  public:
    // Add special RetIP argument.
    RegNum getRetIPArgument() const { return RetIP; }
    void addRetIPArgument();
  private:
    unsigned CoalescingCount = 0;
    RegNum RetIP;
  };

  namespace visa {
    // Details of a type required for a vISA general register declaration
    // or an indirect operand.
    struct TypeDetails {
      const DataLayout &DL;
      unsigned NumElements;
      unsigned BytesPerElement;
      unsigned VisaType;
      TypeDetails(const DataLayout &DL, Type *Ty, genx::Signedness Signed);
    };
  } // end namespace visa

  void initializeGenXVisaRegAllocPass(PassRegistry &);

} // end namespace llvm
#endif //ndef GENXVISAREGALLOC_H

