/*
 * Copyright (c) 2017, Intel Corporation
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

#pragma once
#include "iostream"

class CmExampleTest
{
public:
  virtual ~CmExampleTest() = 0;

public:
  bool Test()
  {
    bool result = true;

    Setup();

    RunAndTimeFor(*this, &CmExampleTest::RunExpect, "Expect ");

    RunAndTimeFor(*this, &CmExampleTest::RunActual, "Actual ");

    return Validate();
  }

protected:
  virtual void Setup() = 0;
  virtual void Teardown() = 0;
  virtual void RunActual() = 0;
  virtual void RunExpect() = 0;
  virtual bool Validate() = 0;

protected:
  template<typename objectTy, typename funcTy>
  static void RunAndTimeFor(objectTy & object, funcTy func, const char * description)
  {

    (object.*func) ();
  }
};

inline CmExampleTest::~CmExampleTest() {}

class BitonicSort : public CmExampleTest
{
protected:
  enum
  {
    base_sort_size_  = 256,
    base_split_size_ = base_sort_size_ << 1,
    base_merge_size_ = 256
  };

public:
  BitonicSort (unsigned int size);

protected:
  virtual void Setup ();
  virtual void RunActual ();
  virtual void RunExpect ();
  virtual bool Validate ();
  virtual void Teardown ();

protected:
  int Solve (const unsigned int * pInputs, unsigned int * pOutputs, unsigned int size);

protected:
  unsigned int size_;
  unsigned int * pInputs_;
  unsigned int * pActualOutputs_;
  unsigned int * pExpectOutputs_;
  unsigned int * pSortDirections_;
};

#define MAX_TS_WIDTH 512
