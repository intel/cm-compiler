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

//===-- llvm/InstrinsicsGenX.h - defines for GenX Intrinsics ----*- C++ -*-===//
//===----------------------------------------------------------------------===//
//
// This file defines enums used when manipulating GenX rdregion and wrregion
// intrinsics.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTRINSICSGENX_H
#define LLVM_IR_INTRINSICSGENX_H

namespace llvm {
namespace Intrinsic {
namespace GenXRegion {

enum {
  // Operands in both rdregion and wrregion:
  OldValueOperandNum = 0,
  // Operands in rdregion:
  RdVStrideOperandNum = 1,
  RdWidthOperandNum = 2,
  RdStrideOperandNum = 3,
  RdIndexOperandNum = 4,
  // Operands in wrregion:
  NewValueOperandNum = 1,
  WrVStrideOperandNum = 2,
  WrWidthOperandNum = 3,
  WrStrideOperandNum = 4,
  WrIndexOperandNum = 5,
  PredicateOperandNum = 7
};

} // End GenXRegion namespace
} // End Intrinsic namespace
} // End llvm namespace

#endif
