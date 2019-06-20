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

//===-- GenXVisa.h - vISA defines -----------------------------------------===//
//===----------------------------------------------------------------------===//
//
// This file contains defines for vISA and the vISA writer.
//
//===----------------------------------------------------------------------===//
#ifndef GENXVISA_H
#define GENXVISA_H

#include "GenX.h"
#include "GenXBaling.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include <map>
#include <string>
#include <vector>
#include "GenXModule.h"

namespace llvm {
  namespace visa {

    // vISA relational operators
    enum { EQ, NE, GT, GE, LT, LE };


    enum {
      CLASS_GENERAL, CLASS_ADDRESS, CLASS_PREDICATE, CLASS_INDIRECT,
      CLASS_IMMEDIATE = 5, CLASS_STATE };

    // vISA vector operand modifiers
    enum { MOD_ABS = 0x8, MOD_NEG = 0x10, MOD_NEGABS = 0x18,
      MOD_SAT = 0x20, MOD_NOT = 0x28 };

    enum { VISA_NUM_RESERVED_REGS = 32,
           VISA_NUM_RESERVED_PREDICATES = 1,
           VISA_NUM_RESERVED_SURFACES = 6 };

    enum {
      VISA_RESERVED_SURFACE_T0 = 0, // Shared local memory access
      VISA_RESERVED_SURFACE_T1,
      VISA_RESERVED_SURFACE_T2,
      VISA_RESERVED_SURFACE_T3,
      VISA_RESERVED_SURFACE_T4,
      VISA_RESERVED_SURFACE_T5
    };

    /// For a fixed surface index, return its vISA equivalent, or -1 if none.
    inline int getReservedSurfaceIndex(Value *Idx) {
      if (ConstantInt *CI = dyn_cast_or_null<ConstantInt>(Idx)) {
        switch ((uint32_t)CI->getZExtValue()) {
          case 254: // 254 is SLM, which is T0 in vISA
            return VISA_RESERVED_SURFACE_T0;
          case 255: // 255 is stateless, which is T5 in vISA
            return VISA_RESERVED_SURFACE_T5;
        }
      }
      return -1;
    }

    inline int getT5() { return 255; }

    enum { VISA_MAX_GENERAL_REGS = 65536 * 256 - 1,
           VISA_MAX_ADDRESS_REGS = 4096,
           VISA_MAX_PREDICATE_REGS = 4096,
           VISA_MAX_SAMPLER_REGS = 32 - 1,
           VISA_MAX_SURFACE_REGS = 256,
           VISA_MAX_VME_REGS = 16 };

    enum { VISA_WIDTH_GENERAL_REG = 32 };

    enum { VISA_ABI_INPUT_REGS_RESERVED = 1,
           VISA_ABI_INPUT_REGS_MAX = 128 };

  } // end namespace Visa

} // end namespace llvm
#endif // ndef GENXVISA_H
